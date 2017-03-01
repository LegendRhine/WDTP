/*
  ==============================================================================

    HtmlProcessor.cpp
    Created: 4 Jan 2017 3:34:15pm
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

//=================================================================================================
void HtmlProcessor::renderHtmlContent (const ValueTree& docTree,
                                       const File& tplFile,
                                       const File& htmlFile)
{
    String tplStr (tplFile.existsAsFile() ? tplFile.loadFileAsString()
                   : TRANS ("Please specify a template file. "));

    // md to html
    const File mdDoc (DocTreeViewItem::getMdFileOrDir (docTree));

    if (!mdDoc.existsAsFile())
        return;

    String mdStrWithoutAbbrev (processAbbrev (docTree, mdDoc.loadFileAsString()));
    const String& keywords (docTree.getProperty ("keywords").toString());

    // here need insert this doc's keywords below the title, 
    // this setp bases on this doc's property 'showKeywords'
    if ((bool)docTree.getProperty ("showKeywords"))
    {
        const String keywordsToInsert (newLine + "> " + TRANS ("Keywords: ") + keywords);
        const int insertIndex = mdStrWithoutAbbrev.indexOf (0, "\n");
        mdStrWithoutAbbrev = mdStrWithoutAbbrev.replaceSection (insertIndex, 0, keywordsToInsert);
    }

    // parse mdString to html string
    const String htmlContentStr (Md2Html::mdStringToHtml (mdStrWithoutAbbrev));

    if (htmlContentStr.isEmpty())
        return;

    // get the path which relative the site root-dir            
    const String& rootRelativePath (getRelativePathToRoot (htmlFile));

    // process code
    if (htmlContentStr.contains ("<pre><code>"))
    {
        tplStr = tplStr.replace ("\n  <title>",
                                 "\n  <script src = \""
                                 + rootRelativePath + "add-in/hl.js\"></script>\n"
                                 "  <script>hljs.initHighlightingOnLoad(); </script>\n"
                                 "  <title>");
    }

    processTags (docTree, htmlFile, tplStr);
    const String& siteName (" - " + FileTreeContainer::projectTree.getProperty ("title").toString());

    // generate the html file
    htmlFile.appendText (tplStr.replace ("{{keywords}}", keywords)
                         .replace ("{{author}}", FileTreeContainer::projectTree.getProperty ("owner").toString())
                         .replace ("{{description}}", docTree.getProperty ("description").toString())
                         .replace ("{{title}}", docTree.getProperty ("title").toString() + siteName)
                         .replace ("{{siteRelativeRootPath}}", rootRelativePath)
                         .replace ("{{content}}", htmlContentStr));

    copyDocMediasToSite (mdDoc, htmlFile, htmlContentStr);
}

//=================================================================================================
const String HtmlProcessor::getRelativePathToRoot (const File &htmlFile)
{
    const String htmlFilePath (htmlFile.getFullPathName());
    const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory().getFullPathName()
                                 + File::separator + "site");
    const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
    String cssRelativePath;

    for (int i = tempStr.length(); --i >= 0;)
    {
        if (tempStr[i] == File::separator)
            cssRelativePath << String ("../");
    }

    return cssRelativePath;
}

//=================================================================================================
const String HtmlProcessor::getSiteLink (const File &htmlFile)
{
    const String& rootPathLink (getRelativePathToRoot (htmlFile) + "index.html");
    const String& siteTitle (FileTreeContainer::projectTree.getProperty ("title").toString());

    return "<a href=\"" + rootPathLink + "\">" + siteTitle.upToFirstOccurrenceOf (" ", false, true) + "</a>";
}

//=================================================================================================
const bool HtmlProcessor::atLeastHasOneMenu (const ValueTree& tree)
{
    for (int i = tree.getNumChildren(); --i >= 0; )
    {
        if ((bool)tree.getChild (i).getProperty ("isMenu"))
            return true;
    }

    return false;
}

//=================================================================================================
const File HtmlProcessor::createArticleHtml (ValueTree& docTree, bool saveProject)
{
    jassert (FileTreeContainer::projectTree.isValid());

    const String docPath (DocTreeViewItem::getMdFileOrDir (docTree).getFullPathName());
    const File htmlFile (File (docPath.replace ("docs", "site")).withFileExtension ("html"));

    if ((bool)docTree.getProperty ("needCreateHtml") || !htmlFile.existsAsFile())
    {
        if (htmlFile.deleteFile())
        {
            const String tplPath (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                  .getFullPathName() + File::separator
                                  + FileTreeContainer::projectTree.getProperty ("render").toString()
                                  + File::separator);

            const File tplFile (tplPath + docTree.getProperty ("tplFile").toString());

            // generate the doc's html
            htmlFile.create();
            renderHtmlContent (docTree, tplFile, htmlFile);

            docTree.setProperty ("needCreateHtml", false, nullptr);

            if (saveProject)
                FileTreeContainer::saveProject();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this document's html file."));
        }
    }

    return htmlFile;
}

//=================================================================================================
void HtmlProcessor::copyDocMediasToSite (const File& mdFile,
                                         const File& htmlFile,
                                         const String& htmlStr)
{
    const String docMediaDirStr (mdFile.getSiblingFile ("media").getFullPathName());
    const String htmlMediaDirStr (htmlFile.getSiblingFile ("media").getFullPathName());
    Array<File> docMedias;
    Array<File> htmlMedias;

    int indexStart = htmlStr.indexOfIgnoreCase (0, "src=\"");
    int indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");

    while (indexStart != -1 && indexEnd != -1)
    {
        const String fileName (htmlStr.substring (indexStart + 11, indexEnd));

        if (!fileName.contains (String (File::separator)))
        {
            docMedias.add (File (docMediaDirStr + File::separator + fileName));
            htmlMedias.add (File (htmlMediaDirStr + File::separator + fileName));
        }

        indexStart = htmlStr.indexOfIgnoreCase (indexEnd + 2, "src=\"");

        if (indexStart != -1)
            indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");
    }

    jassert (docMedias.size() == htmlMedias.size());
    String errorStr;

    for (int i = docMedias.size(); --i >= 0; )
    {
        if (docMedias[i].existsAsFile())
        {
            htmlMedias[i].create();

            if ((docMedias[i].getLastModificationTime() == htmlMedias[i].getLastModificationTime())
                && (docMedias[i].getSize() == htmlMedias[i].getSize()))
                continue;

            if (!docMedias[i].copyFileTo (htmlMedias[i]))
                errorStr << docMedias[i].getFullPathName() << newLine;
        }
    }

    if (errorStr.isNotEmpty())
    {
        SHOW_MESSAGE (TRANS ("Can't generate these media-files:")
                      + newLine + newLine + errorStr + newLine);
    }
}

//=================================================================================================
const File HtmlProcessor::createIndexHtml (ValueTree& dirTree, bool saveProject)
{
    const File& indexHtml (DocTreeViewItem::getHtmlFileOrDir (dirTree));
    jassert (indexHtml.getFileName() == "index.html");

    if ((bool)dirTree.getProperty ("needCreateHtml") || !indexHtml.existsAsFile())
    {
        if (indexHtml.deleteFile())
        {
            const File tplFile (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                .getFullPathName() + File::separator
                                + FileTreeContainer::projectTree.getProperty ("render").toString()
                                + File::separator
                                + dirTree.getProperty ("tplFile").toString());

            // get the path which relative the site root-dir, for css path            
            const String htmlFilePath (indexHtml.getFullPathName());
            const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory().getFullPathName()
                                         + File::separator + "site");
            const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
            String cssRelativePath;

            for (int i = tempStr.length(); --i >= 0;)
            {
                if (tempStr[i] == File::separator)
                    cssRelativePath << String ("../");
            }

            String tplStr (tplFile.existsAsFile() ? tplFile.loadFileAsString() : String());

            // when missing render dir (no tpl)
            if (tplStr.isEmpty())
            {
                indexHtml.create();
                indexHtml.appendText (TRANS ("Please specify a template file. "));

                return indexHtml;
            }

            const String indexTileStr (dirTree.getProperty ("title").toString());
            const String indexAuthorStr (FileTreeContainer::projectTree.getProperty ("owner").toString());            
            const String indexKeywordsStr (dirTree.getProperty ("keywords").toString());
            const String indexDescStr (dirTree.getProperty ("description").toString());
            const String siteName (dirTree.getType().toString() == "wdtpProject"
                                   ? String() : " - " + FileTreeContainer::projectTree.getProperty ("title").toString());

            tplStr = tplStr.replace ("{{siteRelativeRootPath}}", cssRelativePath)
                .replace ("{{author}}", indexAuthorStr)
                .replace ("{{title}}", indexTileStr + siteName)
                .replace ("{{keywords}}", indexKeywordsStr)
                .replace ("{{description}}", indexDescStr);

            processTags (dirTree, indexHtml, tplStr);

            // list for book
            if (tplStr.contains ("{{bookList}}"))
            {
                tplStr = tplStr.replace ("{{bookList}}", getBookList (dirTree));
                indexHtml.create();
                indexHtml.appendText (tplStr);
            }

            // list for blog
            if (tplStr.contains ("{{blogList}}"))
            {
                const StringArray fileLinks (getBlogList (dirTree));
                const int howManyFiles = fileLinks.size() / 3;
                const int howManyPages = howManyFiles / 10 + (howManyFiles % 10 == 0 ? 0 : 1);

                if (howManyFiles < 1)
                {
                    indexHtml.deleteFile();
                    indexHtml.create();

                    const String listHtmlStr = tplStr.replace ("{{blogList}}", String());
                    indexHtml.appendText (listHtmlStr);
                }
                else  // devide to many pages
                {
                    for (int i = 0; i < howManyPages; ++i)
                    {
                        StringArray pageLinks;
                        pageLinks.addArray (fileLinks, i * 30, 30);
                        pageLinks.add (getPageNavi (howManyPages, i + 1));

                        const String listHtmlStr = tplStr.replace ("{{blogList}}",
                                                                   "<div>" + pageLinks.joinIntoString (newLine) + "</div>");

                        const File& indexFile (indexHtml.getSiblingFile ("index-" + String (i + 1) + ".html"));
                        indexFile.deleteFile();
                        indexFile.create();
                        indexFile.appendText (listHtmlStr);
                    }

                    indexHtml.getSiblingFile ("index-1.html").moveFileTo (indexHtml);
                }
            }

            dirTree.setProperty ("needCreateHtml", false, nullptr);

            if (saveProject)
                FileTreeContainer::saveProject();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this folder's index.html."));
        }
    }

    return indexHtml;
}

void HtmlProcessor::rebuildAllKeywords (const bool saveProjectAndPopupMessage)
{
    // extract all keywords of each doc of this project
    StringArray keywordsArray;
    ValueTree pTree (FileTreeContainer::projectTree);
    extractKeywords (pTree, keywordsArray);

    keywordsArray.appendNumbersToDuplicates (true, false, CharPointer_UTF8 ("--"), CharPointer_UTF8 (""));
    keywordsArray.sortNatural ();

    // remove duplicates and remain the last which include "-x (times)"
    for (int i = keywordsArray.size (); --i >= 1; )
    {
        if (keywordsArray[i].upToLastOccurrenceOf ("--", false, true).compareIgnoreCase (
            keywordsArray[i - 1].upToLastOccurrenceOf ("--", false, true)) == 0)
        {
            keywordsArray.remove (i - 1);
        }
    }

    // sort by duplicate-times
    for (int i = 0; i < keywordsArray.size (); ++i)
    {
        for (int j = 0; j < keywordsArray.size () - 1; ++j)
        {
            if (keywordsArray[j].fromLastOccurrenceOf ("--", false, true).getIntValue () <
                keywordsArray[j + 1].fromLastOccurrenceOf ("--", false, true).getIntValue ())
            {
                const String str (keywordsArray[j]);
                keywordsArray.getReference (j) = keywordsArray[j + 1];
                keywordsArray.getReference (j + 1) = str;
            }
        }
    }

    // move the '123XX' to the end
    StringArray tempStrs;

    for (int i = 0; i < keywordsArray.size (); ++i)
    {
        if (!keywordsArray[i].contains ("--"))
        {
            tempStrs.add (keywordsArray[i]);
            keywordsArray.remove (i);
            --i;
        }
    }

    keywordsArray.addArray (tempStrs);
    String keywords (keywordsArray.joinIntoString (","));

    if (keywords.substring (0, 1) == ",")
        keywords = keywords.substring (1);

    //DBGX (keywords);
    pTree.setProperty ("allKeywords", keywords, nullptr);

    if (saveProjectAndPopupMessage)
    {
        FileTreeContainer::saveProject ();
        SHOW_MESSAGE (TRANS ("All keywords in this project have been rebuilt successfully."));
    }
}

//=================================================================================================
void HtmlProcessor::extractKeywords (const ValueTree& tree,
                                  StringArray& arrayToAdd)
{
    const String& keywords (tree.getProperty ("keywords").toString ()
                            .replace (CharPointer_UTF8 ("\xef\xbc\x8c"), ",")); // Chinese ','
    StringArray thisArray;

    thisArray.addTokens (keywords, ",", String ());
    thisArray.trim ();
    thisArray.removeEmptyStrings ();
    thisArray.removeDuplicates (true);
    arrayToAdd.addArray (thisArray);

    for (int i = tree.getNumChildren (); --i >= 0; )
        extractKeywords (tree.getChild (i), arrayToAdd);
}

//=================================================================================================
const String HtmlProcessor::getPageNavi (const int howManyPages, const int thisIsNoX)
{
    if (howManyPages < 2)
        return String();

    StringArray naviStr;

    for (int i = 0; i < howManyPages; ++i)
    {
        const String currentDivClass ((thisIsNoX - 1 == i) ? " class=current" : String());

        if (i == 0)
            naviStr.add ("<a href=index.html" + currentDivClass + ">1</a>");
        else
            naviStr.add ("<a href=index-" + String (i + 1) + ".html" + currentDivClass + ">" + String (i + 1) + "</a>");
    }

    return "<div class=page_navi>" + naviStr.joinIntoString (newLine) + "</div>";
}

//=================================================================================================
const String HtmlProcessor::getToTop()
{
    const String& text (TRANS ("Back To Top"));
    return "<div class=page_navi><a href=\"#top\">" + text + "</a></div>";
}

//=================================================================================================
const String HtmlProcessor::getBackPrevLevel()
{
    return "<div class=siteNavi><a href=\"../index.html\">" + TRANS ("Upper Level") + "</a></div>";
}

//=================================================================================================
const int HtmlProcessor::compareElements (const ValueTree& ft, const ValueTree& st)
{
    // doc vs dir
    if (ft.getType().toString() == "dir" && st.getType().toString() == "doc")
    {
        return sortByReverse ? 1 : -1;
    }
    else if (ft.getType().toString() == "doc" && st.getType().toString() == "dir")
    {
        return sortByReverse ? -1 : 1;
    }
    else  // doc vs doc and dir vs dir..
    {
        if (sortByReverse)
            return ft.getProperty ("createDate").toString().compareIgnoreCase (st.getProperty ("createDate").toString());
        else
            return st.getProperty ("createDate").toString().compareIgnoreCase (ft.getProperty ("createDate").toString());
    }
}

//=================================================================================================
void HtmlProcessor::processTags (const ValueTree& docOrDirTree,
                                 const File& htmlFile,
                                 String& tplStr)
{
    const String& rootRelativePath (getRelativePathToRoot (htmlFile));

    // title of this index.html
    if (tplStr.contains ("{{titleOfDir}}"))
    {
        tplStr = tplStr.replace ("{{titleOfDir}}", "<div align=center><h1>"
                                 + docOrDirTree.getProperty ("title").toString()
                                 + "</h1></div>" + newLine);
    }

    // js
    if (docOrDirTree.getProperty ("js").toString().trim().isNotEmpty())
    {
        tplStr = tplStr.replace ("\n  <title>",
                                 "\n" + docOrDirTree.getProperty ("js").toString().trim() + "\n\n"
                                 "  <title>");
    }

    // site logo
    if (tplStr.contains ("{{siteLogo}}"))
    {
        tplStr = tplStr.replace ("{{siteLogo}}",
                                 "<div class=\"siteLogo\"><a href = \"" + rootRelativePath + "index.html\"><img src = \""
                                 + rootRelativePath + "add-in/logo.png\" /></a></div>");
    }

    // site menu
    if (tplStr.contains ("{{siteMenu}}"))
    {
        if (docOrDirTree.getType().toString() == "doc")
            tplStr = tplStr.replace ("{{siteMenu}}", getSiteMenu (docOrDirTree.getParent()));
        else
            tplStr = tplStr.replace ("{{siteMenu}}", getSiteMenu (docOrDirTree));
    }

    // site navi
    if (tplStr.contains ("{{siteNavi}}"))
    {
        tplStr = tplStr.replace ("{{siteNavi}}", getSiteNavi (docOrDirTree));
    }

    // site link
    if (tplStr.contains ("{{siteLink}}"))
    {
        tplStr = tplStr.replace ("{{siteLink}}", getSiteLink (htmlFile));
    }

    // back to previous level
    if (tplStr.contains ("{{backPrevious}}"))
    {
        tplStr = tplStr.replace ("{{backPrevious}}", getBackPrevLevel());
    }

    // content tile
    if (tplStr.contains ("{{contentTitle}}"))
    {
        tplStr = tplStr.replace ("{{contentTitle}}", getContentTitle (docOrDirTree));
    }

    // content decription
    if (tplStr.contains ("{{contentDesc}}"))
    {
        String descStr (docOrDirTree.getProperty ("description").toString());
        descStr = "<div  align=\"center\"><blockquote>" + descStr + "</blockquote></div>";
        tplStr = tplStr.replace ("{{contentDesc}}", descStr);
    }

    // create and modified time
    if (tplStr.contains ("{{createAndModifyTime}}"))
    {
        tplStr = tplStr.replace ("{{createAndModifyTime}}", getCreateAndModifyTime (docOrDirTree));
    }

    // prev and next
    if (tplStr.contains ("{{previousAndNext}}"))
    {
        tplStr = tplStr.replace ("{{previousAndNext}}", getPrevAndNextArticel (docOrDirTree));
    }

    // ad
    if (tplStr.contains ("{{ad}}"))
    {
        tplStr = tplStr.replace ("{{ad}}", getAdStr (FileTreeContainer::projectTree.getProperty ("ad").toString(), htmlFile));
    }

    // random 5
    if (tplStr.contains ("{{random}}"))
    {
        tplStr = tplStr.replace ("{{random}}", getRandomArticels (docOrDirTree, 5));
    }

    // contact
    if (tplStr.contains ("{{contact}}"))
    {
        tplStr = tplStr.replace ("{{contact}}", getContactInfo());
    }

    // click to top
    if (tplStr.contains ("{{toTop}}"))
    {
        tplStr = tplStr.replace ("{{toTop}}", getToTop());
    }

    // copyright on the bottom
    if (tplStr.contains ("{{bottomCopyright}}"))
    {
        tplStr = tplStr.replace ("{{bottomCopyright}}", getCopyrightInfo());
    }
}

//=================================================================================================
const String HtmlProcessor::processAbbrev (const ValueTree& docTree, const String& originalStr)
{
    // want to process dir or project?!?
    jassert (docTree.getType().toString() == "doc");

    String resultStr (originalStr);

    // get the abbrevs by line
    StringArray abbrevAndOriginal;
    abbrevAndOriginal.addLines (docTree.getProperty ("abbrev").toString().trim());
    abbrevAndOriginal.removeDuplicates (false);
    abbrevAndOriginal.removeEmptyStrings (true);

    if (abbrevAndOriginal.size() != 0)
    {
        // for replace the abbrevs
        StringArray abbrevs, originals;

        for (int i = abbrevAndOriginal.size(); --i >= 0; )
        {
            abbrevAndOriginal.getReference (i) = abbrevAndOriginal[i].trim();

            const String abbrev (abbrevAndOriginal[i].upToFirstOccurrenceOf (" ", false, false));
            const String original (abbrevAndOriginal[i].fromFirstOccurrenceOf (" ", false, false).trimStart());

            if (abbrev.isNotEmpty() && original.isNotEmpty())
            {
                abbrevs.add (abbrev);
                originals.add (original);
            }
        }
        
        // replace
        for (int i = abbrevs.size(); --i >= 0; )
            resultStr = resultStr.replace (abbrevs[i], originals[i], false);
    }

    return resultStr;
}

//=================================================================================================
const String HtmlProcessor::getSiteMenu (const ValueTree& tree)
{
    ValueTree pTree (FileTreeContainer::projectTree.createCopy());
    HtmlProcessor sorter (false);
    pTree.sort (sorter, nullptr, false);

    StringArray menuHtmlStr;

    if (atLeastHasOneMenu (pTree))
        menuHtmlStr.add ("<div class=\"siteMenu\"><ul>");
    else
        return String();

    for (int i = 0; i < pTree.getNumChildren(); ++i)
    {
        ValueTree fd (pTree.getChild (i));

        if ((bool)fd.getProperty ("isMenu") && DocTreeViewItem::getMdFileOrDir (fd).exists())
        {
            const File& dirIndex (DocTreeViewItem::getHtmlFileOrDir (fd));
            const String& menuName (fd.getProperty ("title").toString());
            String path;

            if (fd.getType().toString() == "doc")
            {
                path = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree))
                    + fd.getProperty ("name").toString() + ".html";
            }
            else
            {
                path = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree))
                    + dirIndex.getParentDirectory().getFileName() + "/index.html";
            }

            menuHtmlStr.add ("<li><a href=\"" + path + "\">" + menuName + "</a>");

            if (atLeastHasOneMenu (fd))
            {
                menuHtmlStr.add ("<ul>");
                fd.sort (sorter, nullptr, false);

                for (int j = 0; j < fd.getNumChildren(); ++j)
                {
                    const ValueTree& sd (fd.getChild (j));

                    if (DocTreeViewItem::getMdFileOrDir (sd).exists()
                        && (bool)sd.getProperty ("isMenu"))
                    {
                        const File& sDirIndex (DocTreeViewItem::getHtmlFileOrDir (sd));
                        const String& sMenuName (sd.getProperty ("title").toString());
                        String sPath;

                        if (sd.getType().toString() == "doc")
                        {
                            sPath = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree))
                                + dirIndex.getParentDirectory().getFileName() + "/"
                                + sd.getProperty ("name").toString() + ".html";
                        }
                        else
                        {
                            sPath = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree))
                                + dirIndex.getParentDirectory().getFileName() + "/"
                                + sDirIndex.getParentDirectory().getFileName() + "/index.html";
                        }

                        menuHtmlStr.add ("<li><a href=\"" + sPath + "\">" + sMenuName + "</a></li>");
                    }
                }

                menuHtmlStr.add ("</li></ul>");
            }
        }
    }

    menuHtmlStr.add ("</ul></div>");
    return menuHtmlStr.joinIntoString (newLine);
}

//=================================================================================================
const String HtmlProcessor::getSiteNavi (const ValueTree& docTree)
{
    String navi;
    ValueTree parent (docTree.getParent());
    String path;

    while (parent.isValid())
    {
        String text (parent.getProperty ("title").toString());

        if (parent.getType().toString() == "wdtpProject")
            text = text.upToFirstOccurrenceOf (" ", false, true);

        navi = "<a href=\"" + path + "index.html\">" + text + "</a>/" + navi;
        path += "../";

        parent = parent.getParent();
    }

    return "<div class=\"siteNavi\">" + navi + "</div>";
}

//=================================================================================================
const String HtmlProcessor::getContentTitle (const ValueTree& tree)
{
    return tree.getProperty ("title").toString();
}

//=================================================================================================
const String HtmlProcessor::getCreateAndModifyTime (const ValueTree& tree)
{
    const String& createStr (TRANS ("Create Time: ") + tree.getProperty ("createDate").toString().dropLastCharacters (3));
    const String& modifyStr (TRANS ("Last Modified: ") + tree.getProperty ("modifyDate").toString().dropLastCharacters (3));

    return "<div class=timeStr>" + createStr + "<br>" + modifyStr + "</div>";
}

//=================================================================================================
const String HtmlProcessor::getPrevAndNextArticel (const ValueTree& tree)
{
    String prevStr, nextStr;

    ValueTree prevTree ("doc");
    getPreviousTree (FileTreeContainer::projectTree, tree, prevTree);
    const String prevName = prevTree.getProperty ("title").toString();

    if (prevName.isNotEmpty())
    {
        String prevPath = DocTreeViewItem::getHtmlFileOrDir (prevTree).getFullPathName();
        prevPath = prevPath.replace (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName(), String());
        prevPath = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree)) + prevPath.substring (1);
        prevPath = prevPath.replace ("\\", "/");
        prevStr = TRANS ("Prev: ") + "<a href=\"" + prevPath + "\">" + prevName + "</a><br>";
    }

    ValueTree nextTree ("doc");
    getNextTree (FileTreeContainer::projectTree, tree, nextTree);
    const String nextName = nextTree.getProperty ("title").toString();

    if (nextName.isNotEmpty())
    {
        String nextPath = DocTreeViewItem::getHtmlFileOrDir (nextTree).getFullPathName();
        nextPath = nextPath.replace (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName(), String());
        nextPath = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (tree)) + nextPath.substring (1);
        nextPath = nextPath.replace ("\\", "/");
        nextStr = TRANS ("Next: ") + "<a href=\"" + nextPath + "\">" + nextName + "</a>";
    }

    return "<div class=prevAndNext>" + prevStr + nextStr + "</div>";
}

//=================================================================================================
const String HtmlProcessor::getRandomArticels (const ValueTree& notIncludeThisTree,
                                               const int howMany)
{
    StringArray links;
    getLinkStrOfAlllDocTrees (FileTreeContainer::projectTree, notIncludeThisTree, links);

    // + 1: prevent a articel is the current, make sure enough
    Array<int> randoms = getRandomInts (howMany + 1);
    StringArray randomLinks;

    for (int i = 0; i < randoms.size(); ++i)
    {
        const int order = randoms[i];
        randomLinks.add (links[order]);
    }

    randomLinks.removeEmptyStrings (true);

    if (randomLinks.size() > howMany)
        randomLinks.remove (0);  // remove the newest one

    randomLinks.insert (0, "<div class=randomArticels><ul>");
    randomLinks.insert (0, "<b>" + TRANS ("Random Posts:") + "</b>");

    for (int j = randomLinks.size(); --j >= 2; )
        randomLinks.getReference (j) = "<li>" + randomLinks[j] + "</li>";

    randomLinks.add ("</ul></div>");

    return randomLinks.joinIntoString (newLine);
}

//=================================================================================================
void HtmlProcessor::getPreviousTree (const ValueTree& oTree,
                                     const ValueTree& tree,
                                     ValueTree& result)
{
    // prevent createDate is empty
    if (result.getProperty ("createDate").toString() == String())
        result.setProperty ("createDate", 1, nullptr);

    if (oTree.getType().toString() == "doc" && !(bool)oTree.getProperty ("isMenu"))
    {
        if ((oTree.getProperty ("createDate").toString() < tree.getProperty ("createDate").toString())
            && (oTree.getProperty ("createDate").toString() > result.getProperty ("createDate").toString()))
            result = oTree;
    }

    for (int i = oTree.getNumChildren(); --i >= 0; )
        getPreviousTree (oTree.getChild (i), tree, result);
}

//=================================================================================================
void HtmlProcessor::getNextTree (const ValueTree& oTree,
                                 const ValueTree& tree,
                                 ValueTree& result)
{
    // prevent createDate is empty
    if (result.getProperty ("createDate").toString() == String())
        result.setProperty ("createDate", 3, nullptr);

    if (oTree.getType().toString() == "doc" && !(bool)oTree.getProperty ("isMenu"))
    {
        if ((oTree.getProperty ("createDate").toString() > tree.getProperty ("createDate").toString())
            && (oTree.getProperty ("createDate").toString() < result.getProperty ("createDate").toString()))
            result = oTree;
    }

    for (int i = oTree.getNumChildren(); --i >= 0; )
        getNextTree (oTree.getChild (i), tree, result);
}

//=================================================================================================
const String HtmlProcessor::getAdStr (const String& text, const File& htmlFile)
{
    StringArray orignalText;
    orignalText.addLines (text.trim());
    orignalText.removeEmptyStrings (true);

    StringArray links;

    for (int i = 0; i < orignalText.size(); ++i)
    {
        const String& imgName (getRelativePathToRoot (htmlFile) + "add-in/"
                               + orignalText[i].trim().upToFirstOccurrenceOf (" ", false, true));
        const String& link (orignalText[i].trim().fromFirstOccurrenceOf (" ", false, true));
        links.add ("<a href=\"" + link + "\" target=\"_blank\"><img src=\"" + imgName + "\"></a><br>");
    }

    if (links.size() > 0)  // remove the last '<br>'
        links.getReference (links.size() - 1) = links[links.size() - 1].dropLastCharacters (4);

    links.insert (0, "<div class=ad>");
    links.add ("</div>");

    return links.joinIntoString (newLine);
}

//=================================================================================================
void HtmlProcessor::getDocNumbersOfTheDir (const ValueTree& dirTree, int& num)
{
    if (dirTree.getType().toString() == "doc")
        ++num;
    else
        for (int i = dirTree.getNumChildren(); --i >= 0; )
            getDocNumbersOfTheDir (dirTree.getChild (i), num);
}

//=================================================================================================
const Array<int> HtmlProcessor::getRandomInts (const int howMany)
{
    Array<int> values;
    int maxValue = 0;
    getDocNumbersOfTheDir (FileTreeContainer::projectTree, maxValue);
    Random r (Time::currentTimeMillis());

    for (int i = jmin (maxValue, howMany); --i >= 0; )
    {
        int randomValue = r.nextInt (maxValue);

        for (int j = 0; j < values.size(); ++j)
        {
            if (values[j] == randomValue)
            {
                randomValue = r.nextInt (maxValue);
                j = -1;  // here must be -1, because it'll ++j in for()
            }
        }

        values.addUsingDefaultSort (randomValue);
    }

    return values;
}

//=================================================================================================
void HtmlProcessor::getLinkStrOfAlllDocTrees (const ValueTree& fromThisTree,
                                              const ValueTree& baseOnThisTree,
                                              StringArray& linkStr)
{
    if (fromThisTree.getType().toString() == "doc" && fromThisTree != baseOnThisTree)
    {
        if (!(bool)fromThisTree.getProperty ("isMenu"))
        {
            const String text = fromThisTree.getProperty ("title").toString();

            String path = DocTreeViewItem::getHtmlFileOrDir (fromThisTree).getFullPathName();
            path = path.replace (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName(), String());
            path = getRelativePathToRoot (DocTreeViewItem::getHtmlFileOrDir (baseOnThisTree)) + path.substring (1);
            path = path.replace ("\\", "/");

            linkStr.add ("<a href=\"" + path + "\">" + text + "</a>");
        }
    }
    else
    {
        for (int i = fromThisTree.getNumChildren(); --i >= 0; )
            getLinkStrOfAlllDocTrees (fromThisTree.getChild (i), baseOnThisTree, linkStr);
    }
}

//=================================================================================================
void HtmlProcessor::getBookListLinks (const ValueTree& tree,
                                      const bool isRootTree,
                                      StringArray& linkStr)
{
    const String filePath (DocTreeViewItem::getHtmlFileOrDir (tree).getFullPathName());
    const String rootPath (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName());
    String path (filePath.fromFirstOccurrenceOf (rootPath, false, false).substring (1));
    path = path.replace ("\\", "/");

    if (!isRootTree)
        path = "../" + path;

    //DBGX(path);

    String text (tree.getProperty ("title").toString());

    if (tree.getType().toString() != "doc")
        text = "<b>" + text + "</b> ";

    // title and its link
    String str = "<li><a href=\"" + path + "\">" + text + "</a></li>";
    linkStr.add (str);
    linkStr.add ("<ul>");

    for (int i = tree.getNumChildren(); --i >= 0; )
        getBookListLinks (tree.getChild (i), isRootTree, linkStr);

    linkStr.add ("</ul>");
}

//=================================================================================================
void HtmlProcessor::getBlogListHtmlStr (const ValueTree& tree,
                                        const File& baseOnthisFile,
                                        StringArray& linkStr)
{
    const String& rootPath (getRelativePathToRoot (baseOnthisFile));
    String path = DocTreeViewItem::getHtmlFileOrDir (tree).getFullPathName();
    path = path.replace (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName(), String());
    path = rootPath + path.substring (1);
    path = path.replace ("\\", "/");

    if (DocTreeViewItem::getHtmlFileOrDir (tree) != baseOnthisFile)
    {
        if (!(bool)tree.getProperty ("isMenu"))
        {
            const String& text (tree.getProperty ("title").toString());
            const String& imgName (tree.getProperty ("thumbName").toString());

            String str (tree.getType().toString() == "doc" ? "doc" : "dir");

            // create and last modified date
            str += "@_^_#_%_@<img src=" + rootPath
                + "add-in/createDate.png style=\"vertical-align:middle; display:inline-block\"> "
                + tree.getProperty ("createDate").toString().dropLastCharacters (3) // drop seconds
                + " &nbsp;&nbsp;<img src=" + rootPath
                + "add-in/modifiedDate.png style=\"vertical-align:middle; display:inline-block\"> " +
                // + " - " +
                tree.getProperty ("modifyDate").toString().dropLastCharacters (3); // drop seconds 

            // 2 level dir and their link
            const ValueTree parentTree (tree.getParent());
            const ValueTree grandTree (parentTree.getParent());

            // dir icon
            if ((grandTree.isValid() && grandTree.getType().toString() != "wdtpProject")
                || (parentTree.isValid() && parentTree.getType().toString() != "wdtpProject"))
            {
                str += " &nbsp;&nbsp;<img src=" + rootPath
                    + "add-in/dir.png style=\"vertical-align:middle; display:inline-block\"> ";
            }

            if (grandTree.isValid() && grandTree.getType().toString() != "wdtpProject")
            {
                const String parentPath (path.upToLastOccurrenceOf ("/", false, false)
                                         .upToLastOccurrenceOf ("/", true, false) + "index.html");

                str += "<a href=\"" + parentPath + "\">" +
                    grandTree.getProperty ("title").toString() + "</a>/";
            }

            if (parentTree.isValid() && parentTree.getType().toString() != "wdtpProject")
            {
                const String parentPath (path.upToLastOccurrenceOf ("/", true, false) + "index.html");
                str += "<a href=\"" + parentPath + "\">" +
                    parentTree.getProperty ("title").toString() + "</a>";
            }

            // title and its link
            str = str + "@_^_#_%_@" + "<a href=\"" + path + "\">" + text + "</a>@_^_#_%_@";

            if (imgName.isNotEmpty() && (bool)tree.getProperty ("thumb"))
            {
                const String& imgPath (imgName.substring (0, 4) == "http" ? imgName
                                       : path.upToLastOccurrenceOf ("/", true, false) + imgName); // remove 'xxxx.html'
                str += "<div><img src=\"" + imgPath + "\"></div><p>";
            }

            // description
            str += tree.getProperty ("description").toString() + "<div class=readMore align=right>"
                + "<a href=\"" + path + "\">" + TRANS ("Read More") + "</a></div>";

            linkStr.add (str);
        }
    }

    for (int i = tree.getNumChildren(); --i >= 0; )
        getBlogListHtmlStr (tree.getChild (i), baseOnthisFile, linkStr);
}

//=================================================================================================
const StringArray HtmlProcessor::getBlogList (const ValueTree& dirTree)
{
    jassert (dirTree.getType().toString() != "doc");
    const File& indexFile (DocTreeViewItem::getHtmlFileOrDir (dirTree));
    StringArray filesLinkStr;

    getBlogListHtmlStr (dirTree, indexFile, filesLinkStr);
    filesLinkStr.sort (true);

    for (int i = filesLinkStr.size(); --i >= 0; )
    {
        if (filesLinkStr[i].substring (0, 3) == "dir")
            filesLinkStr.remove (i);
    }

    StringArray linkStr;

    for (int i = filesLinkStr.size(); --i >= 0; )
    {
        filesLinkStr.getReference (i) = filesLinkStr[i].substring (12); // remove "dir/doc @_^_#_%_@"
        const String dateStr (filesLinkStr[i].upToFirstOccurrenceOf ("@_^_#_%_@", false, true));        
        const String descStr (filesLinkStr[i].fromLastOccurrenceOf ("@_^_#_%_@", false, true));
        const String titleStr (filesLinkStr[i].substring (dateStr.length() + 9).dropLastCharacters (descStr.length() + 9));

        linkStr.add (titleStr);
        linkStr.add (dateStr);
        linkStr.add (Md2Html::mdStringToHtml (descStr));
    }

    for (int i = 0; i < linkStr.size(); ++i)
    {
        if (0 == i % 3)
            linkStr.getReference (i) = "<div class=listTitle>" + linkStr[i] + "</div>";
        else if (1 == i % 3)
            linkStr.getReference (i) = "<div class=listDate>" + linkStr[i] + "</div>";
        else
            linkStr.getReference (i) = "<div class=listDesc>" + linkStr[i] + "</div><hr>";
    }

    //DBGX(linkStr.joinIntoString(newLine));
    return linkStr;
}

//=================================================================================================
const String HtmlProcessor::getBookList (const ValueTree& dirTree)
{
    struct Sorter  // by create date, but doc before dir
    {
        const int compareElements (const ValueTree ft, const ValueTree st) const
        {
            if (ft.getType().toString() == "dir" && st.getType().toString() == "doc")
                return -1;
            else if (ft.getType().toString() == "doc" && st.getType().toString() == "dir")
                return 1;
            else  // doc vs doc and dir vs dir..
                return st.getProperty ("createDate").toString().compareIgnoreCase (ft.getProperty ("createDate").toString());
        }
    };

    ValueTree tree (dirTree.createCopy());
    Sorter s;
    tree.sort (s, nullptr, false);
    StringArray links;

    getBookListLinks (tree, tree.getType().toString() == "wdtpProject", links);
    links.remove (0);  // itself

    return "<div class=catalogue>" + links.joinIntoString (newLine) + "</div>";
}

//=========================================================================
const String HtmlProcessor::getCopyrightInfo()
{
    return "<p><hr>\n"
        "<table id=\"copyright\"><tr><td id=\"copyright\">" +
        FileTreeContainer::projectTree.getProperty ("copyright").toString() +
        "</td><td id=\"copyright\" align=\"right\">Powered by "
        "<a href=\"http://underwaySoft.com/works/wdtp/index.html\""
        " target=\"_blank\">WDTP</a> </td></tr></table>";
}

//=================================================================================================
const String HtmlProcessor::getContactInfo()
{
    const String& contactStr (FileTreeContainer::projectTree.getProperty ("contact").toString());
    return "<div class=contact>" + contactStr + "</div>";
}


/*
  ==============================================================================

    HtmlProcessor.cpp
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#include "../WdtpHeader.h"

//=================================================================================================
const String HtmlProcessor::getFileList (const ValueTree& dirTree_,
                                           const bool reverse,
                                           const bool includeDir,
                                           const bool extrctIntro,
                                           const int itemsPrePage/* = 0*/)
{   
    jassert (dirTree_.getType ().toString () != "doc");

    StringArray filesLinkStr;
    const File thisDir (DocTreeViewItem::getHtmlFileOrDir (dirTree_).getParentDirectory());

    ValueTree dirTree (dirTree_.createCopy ());
    HtmlProcessor sorter;
    dirTree.sort (sorter, nullptr, false);

    for (int i = dirTree.getNumChildren(); --i >= 0; )
    {
        const ValueTree& tree (dirTree.getChild (i));
        const String titleStr (tree.getProperty ("title").toString ());
        const String fileName (tree.getProperty ("name").toString ());

        File html;

        if (tree.getType ().toString () == "dir")
            html = thisDir.getChildFile (fileName + "/index.html");
        else  // doc
            html = thisDir.getChildFile (fileName + ".html");

        String path (html.getFullPathName().fromFirstOccurrenceOf (thisDir.getFullPathName(), false, false));

        if (!includeDir && path == "index.html")
        {
            path = String ();
        }
        else
        {
            path = "<a href=\"." + path + "\">" + titleStr + "</a>";

            if (extrctIntro)
            {
                path = "<h2>" + path + "</h2>" +
                    +"<h4>" + tree.getProperty ("date").toString () + "</h4><p>"
                    + tree.getProperty ("description").toString () + "<hr>";

            }
            else
            {
                path = "<h4>" + path + "</h4>";
            }
        }

        if (reverse)
            filesLinkStr.add (path);
        else
            filesLinkStr.insert (0, path);
    }

    filesLinkStr.removeEmptyStrings (false);
    return filesLinkStr.joinIntoString (newLine) + Md2Html::copyrightInfo;
}

//=================================================================================================
const File HtmlProcessor::createArticleHtml (ValueTree& docTree, bool saveProject)
{
    jassert (FileTreeContainer::projectTree.isValid ());

    const File mdDoc (DocTreeViewItem::getMdFileOrDir (docTree));
    jassert (mdDoc.existsAsFile ());  // selected a dir currently??    

    const String docPath (mdDoc.getFullPathName ());
    const File htmlFile (File (docPath.replace ("docs", "site")).withFileExtension ("html"));

    if ((bool) docTree.getProperty ("needCreateHtml") || !htmlFile.existsAsFile ())
    {
        if (htmlFile.deleteFile ())
        {
            const String tplPath (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                  .getFullPathName () + File::separator
                                  + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                  + File::separator);

            const File tplFile = tplPath + ((bool (docTree.getProperty ("isPage")))
                                            ? "page.html" : "article.html");

            // get the path which relative the site root-dir, for css path            
            const String htmlFilePath (htmlFile.getFullPathName ());
            const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory ().getFullPathName ()
                                         + File::separator + "site");
            const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
            String cssRelativePath;

            for (int i = tempStr.length (); --i >= 0;)
            {
                if (tempStr[i] == File::separator)
                    cssRelativePath << String ("../");
            }

            // generate the doc's html
            htmlFile.create ();
            const String htmlStr (Md2Html::mdStringToHtml (mdDoc.loadFileAsString ()));
            htmlFile.appendText (Md2Html::renderHtmlContent (htmlStr,
                                                             tplFile,
                                                             docTree.getProperty ("keywords").toString (),
                                                             docTree.getProperty ("description").toString (),
                                                             docTree.getProperty ("title").toString (),
                                                             cssRelativePath));

            // here, we copy this doc's media file to the site's
            const String docMediaDirStr (mdDoc.getSiblingFile ("media").getFullPathName ());
            const String htmlMediaDirStr (htmlFile.getSiblingFile ("media").getFullPathName ());
            Array<File> docMedias;
            Array<File> htmlMedias;

            int indexStart = htmlStr.indexOfIgnoreCase (0, "src=\"");
            int indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");

            while (indexStart != -1 && indexEnd != -1)
            {
                docMedias.add (File (docMediaDirStr + File::separator + htmlStr.substring (indexStart + 11, indexEnd)));
                htmlMedias.add (File (htmlMediaDirStr + File::separator + htmlStr.substring (indexStart + 11, indexEnd)));

                indexStart = htmlStr.indexOfIgnoreCase (indexEnd + 2, "src=\"");

                if (indexStart != -1)
                    indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");
            }

            jassert (docMedias.size () == htmlMedias.size ());
            String errorStr;

            for (int i = docMedias.size (); --i >= 0; )
            {
                htmlMedias[i].create ();

                if (!docMedias[i].copyFileTo (htmlMedias[i]))
                    errorStr << docMedias[i].getFullPathName () << newLine;
            }

            if (errorStr.isNotEmpty ())
            {
                SHOW_MESSAGE (TRANS ("Can't generate these media-files:")
                              + newLine + newLine + errorStr + newLine);
            }

            docTree.setProperty ("needCreateHtml", false, nullptr);

            if (saveProject)
                FileTreeContainer::saveProject ();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this document's html file."));
        }
    }

    return htmlFile;
}

//=================================================================================================
const File HtmlProcessor::createIndexHtml (ValueTree& dirTree, bool saveProject)
{
    jassert (FileTreeContainer::projectTree.isValid ());

    const File dirFile (DocTreeViewItem::getMdFileOrDir (dirTree));
    jassert (dirFile.isDirectory ());  // selected a article currently?? 

    const String dirPath (dirFile.getFullPathName ());
    const File siteDir (dirPath.replace ("docs", "site"));
    File indexHtml (siteDir.getChildFile ("index.html"));

    if ((bool) dirTree.getProperty ("needCreateHtml")
        || !indexHtml.existsAsFile ())
    {
        if (indexHtml.deleteFile ())
        {
            const bool isWebIndex = (siteDir.getFileName () == "site");

            const File tplFile (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                .getFullPathName () + File::separator
                                + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                + File::separator
                                + (isWebIndex ? "index.html" : "category.html"));

            // get the path which relative the site root-dir, for css path            
            const String htmlFilePath (indexHtml.getFullPathName ());
            const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory ().getFullPathName ()
                                         + File::separator + "site");
            const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
            String cssRelativePath;

            for (int i = tempStr.length (); --i >= 0;)
            {
                if (tempStr[i] == File::separator)
                    cssRelativePath << String ("../");
            }

            const String tplStr (tplFile.loadFileAsString ());
            const String indexTileStr (dirTree.getProperty ("title").toString ());
            const String indexDescStr (dirTree.getProperty ("description").toString ());

            String indexContent (tplStr.replace ("{{siteRelativeRootPath}}", cssRelativePath)
                                 .replace ("{{title}}", indexTileStr)
                                 .replace ("{{keywords}}", indexTileStr)
                                 .replace ("{{description}}", indexDescStr));

            // title of this index.html
            if (tplStr.contains ("{{titleOfDir}}"))
                indexContent = indexContent.replace ("{{titleOfDir}}", "<div align=center><h1>"
                                                     + indexTileStr + "</h1></div>" + newLine);

            // process TAGs, it's nothing about the content but the dir or something else
            if (tplStr.contains ("{{fileAndDirList_N_Y_N_0}}"))
            {
                indexContent = indexContent.replace ("{{fileAndDirList_N_Y_N_0}}", "<div>"
                                                     + getFileList (dirTree, false, true, false)
                                                     + "</div>");
            }

            indexHtml.create ();
            indexHtml.appendText (indexContent);
            dirTree.setProperty ("needCreateHtml", false, nullptr);

            if (saveProject)
                FileTreeContainer::saveProject ();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this folder's index.html."));
        }
    }

    return indexHtml;
}

//=================================================================================================
const int HtmlProcessor::compareElements (const ValueTree& ft, const ValueTree& st)
{
    if (ft.getType ().toString () == "dir" && st.getType ().toString () == "doc")
        return -1;
    else if (ft.getType ().toString () == "doc" && st.getType ().toString () == "dir")
        return 1;
    else  // doc vs doc and dir vs dir..
        return ft.getProperty ("date").toString ().compareIgnoreCase (st.getProperty ("date").toString ());
}


/*
  ==============================================================================

    HtmlProcessor.cpp
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#include "WdtpHeader.h"

//=================================================================================================
void HtmlProcessor::renderHtmlContent (const ValueTree& docTree, 
	const File& tplFile,
	const File& htmlFile)
{
	String tplStr (tplFile.existsAsFile () ? tplFile.loadFileAsString ()
		: TRANS ("Please specify a template file. "));
		
	// md to html
	const File mdDoc (DocTreeViewItem::getMdFileOrDir (docTree));
	jassert (mdDoc.existsAsFile ());  // selected a dir currently?? 
	const String htmlContentStr (Md2Html::mdStringToHtml (mdDoc.loadFileAsString ()));

	if (htmlContentStr.isEmpty ())
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

	// process doc's js
	if (docTree.getProperty ("js").toString().trim ().isNotEmpty ())
	{
		tplStr = tplStr.replace ("\n  <title>",
			"\n  <script type=\"text/javascript\">\n"
			+ docTree.getProperty ("js").toString().trim () + "\n  </script>\n"
			"  <title>");
	}

	/* process tags...
	- {{siteLogo}} 显示网站LOGO图片，图片位于网站根目录add-in文件夹下，文件名为logo.png
	- {{siteMenu}} 网站主菜单
	- {{siteNavi}} 当前页面的导航菜单
	- {{previousAndNext}} 上一篇，下一篇
	- {{random-5}} 随机推荐5篇本站文章
	*/
	if (tplStr.contains ("{{siteLogo}}"))
	{
		tplStr = tplStr.replace ("{{siteLogo}}",
			"<div class=\"siteLogo\"><a href = \"" + rootRelativePath + "index.html\"><img src = \"" 
			+ rootRelativePath + "add-in/logo.png\" /></a></div>");
	}

	if (tplStr.contains ("{{siteMenu}}"))
	{
		tplStr = tplStr.replace ("{{siteMenu}}", getSiteMenu (docTree.getParent()));
	}

	if (tplStr.contains ("{{siteNavi}}"))
	{
		tplStr = tplStr.replace ("{{siteNavi}}", getSiteNavi (docTree));
	}

	// generate the html file
	htmlFile.appendText (tplStr.replace ("{{keywords}}", docTree.getProperty ("keywords").toString ())
		.replace ("{{author}}", FileTreeContainer::projectTree.getProperty ("owner").toString ())
		.replace ("{{description}}", docTree.getProperty ("description").toString ())
		.replace ("{{title}}", docTree.getProperty ("title").toString ())
		.replace ("{{siteRelativeRootPath}}", rootRelativePath)
		.replace ("{{content}}", htmlContentStr));

	copyDocMediasToSite (mdDoc, htmlFile, htmlContentStr);
}

//=================================================================================================
const String HtmlProcessor::getRelativePathToRoot (const File &htmlFile)
{
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

	return cssRelativePath;
}

//=================================================================================================
const bool HtmlProcessor::hasDirAndAtLeadOneIsMenu(const ValueTree& tree)
{
	for (int i = tree.getNumChildren(); --i >= 0; )
	{
		if (tree.getChild(i).getType().toString() == "dir" && (bool)tree.getChild(i).getProperty("isMenu"))
			return true;
	}

	return false;
}

//=================================================================================================
const String HtmlProcessor::getFileList (const ValueTree& dirTree_,
                                         const bool reverse,
                                         const bool includeDir,
                                         const bool extrctIntro,
                                         const int /*itemsPerPage*//* = 0*/)
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
                    +"<h4>" + tree.getProperty ("createDate").toString () + "</h4><p>"
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
    return filesLinkStr.joinIntoString (newLine) + getCopyrightInfo();
}

//=================================================================================================
const File HtmlProcessor::createArticleHtml (ValueTree& docTree, bool saveProject)
{
    jassert (FileTreeContainer::projectTree.isValid ());   

    const String docPath (DocTreeViewItem::getMdFileOrDir (docTree).getFullPathName ());
    const File htmlFile (File (docPath.replace ("docs", "site")).withFileExtension ("html"));

    if ((bool) docTree.getProperty ("needCreateHtml") || !htmlFile.existsAsFile ())
    {
        if (htmlFile.deleteFile ())
        {
            const String tplPath (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                  .getFullPathName () + File::separator
                                  + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                  + File::separator);

            const File tplFile (tplPath + docTree.getProperty("tplFile").toString());            

            // generate the doc's html
            htmlFile.create ();
			renderHtmlContent (docTree, tplFile, htmlFile);

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
void HtmlProcessor::copyDocMediasToSite (const File& mdFile, 
	const File& htmlFile, 
	const String& htmlStr)
{
	const String docMediaDirStr (mdFile.getSiblingFile ("media").getFullPathName ());
	const String htmlMediaDirStr (htmlFile.getSiblingFile ("media").getFullPathName ());
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

	jassert (docMedias.size () == htmlMedias.size ());
	String errorStr;

	for (int i = docMedias.size (); --i >= 0; )
	{
		if (docMedias[i].existsAsFile ())
		{
			htmlMedias[i].create ();

			if (!docMedias[i].copyFileTo (htmlMedias[i]))
				errorStr << docMedias[i].getFullPathName () << newLine;
		}
	}

	if (errorStr.isNotEmpty ())
	{
		SHOW_MESSAGE (TRANS ("Can't generate these media-files:")
			+ newLine + newLine + errorStr + newLine);
	}
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
            const File tplFile (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                .getFullPathName () + File::separator
                                + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                + File::separator
                                + dirTree.getProperty("tplFile").toString());

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

            const String tplStr (tplFile.existsAsFile() ? tplFile.loadFileAsString () : String());
            const String indexTileStr (dirTree.getProperty ("title").toString ());
            const String indexAuthorStr (FileTreeContainer::projectTree.getProperty ("owner").toString ());
            const String indexKeywordsStr (dirTree.getProperty ("keywords").toString ());
            const String indexDescStr (dirTree.getProperty ("description").toString ());

            String indexContent (tplStr.replace ("{{siteRelativeRootPath}}", cssRelativePath)
                                 .replace ("{{author}}", indexAuthorStr)
                                 .replace ("{{title}}", indexTileStr)
                                 .replace ("{{keywords}}", indexKeywordsStr)
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

            // java script
            if (dirTree.getProperty("js").toString().trim().isNotEmpty())
            {
                indexContent = indexContent.replace (newLine + "  <title>",
                                                     "\n  <script type=\"text/javascript\">\n"
                                                     + dirTree.getProperty ("js").toString ().trim ()
                                                     + "\n  </script>\n  <title>");
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
    if (ft.getType().toString() == "dir" && st.getType().toString() == "doc")
        return -1;
    else if (ft.getType().toString() == "doc" && st.getType().toString() == "dir")
        return 1;
    else  // doc vs doc and dir vs dir..
        return ft.getProperty("createDate").toString().compareIgnoreCase
        (st.getProperty("createDate").toString());
}

//=================================================================================================
String HtmlProcessor::getSiteMenu (const ValueTree& tree)
{
	const ValueTree& pTree(FileTreeContainer::projectTree);
	StringArray menuHtmlStr;

	if (hasDirAndAtLeadOneIsMenu(pTree))
		menuHtmlStr.add("<div class=\"siteMenu\"><ul>");
	else
		return String();

	for (int i = 0; i < pTree.getNumChildren(); ++i)
	{
		const ValueTree& fd(pTree.getChild(i));

		if (fd.getType().toString() == "dir" && (bool)fd.getProperty("isMenu"))
		{
			const File& dirIndex(DocTreeViewItem::getHtmlFileOrDir(fd));
			const String& menuName(fd.getProperty("title").toString());
			const String& path(getRelativePathToRoot(DocTreeViewItem::getHtmlFileOrDir(tree)) 
							   + dirIndex.getParentDirectory().getFileName() + "/index.html");
			const String& linkStr("<li><a href=\"" + path + "\">" + menuName + "</a>");
			menuHtmlStr.add(linkStr);

			if (hasDirAndAtLeadOneIsMenu(fd))
			{
				menuHtmlStr.add("<ul>");

				for (int j = 0; j < fd.getNumChildren(); ++j)
				{
					const ValueTree& sd(fd.getChild(j));

					if (sd.getType().toString() == "dir" && (bool)sd.getProperty("isMenu"))
					{
						const File& sDirIndex(DocTreeViewItem::getHtmlFileOrDir(sd));
						const String& sMenuName(sd.getProperty("title").toString());
						const String& sPath(getRelativePathToRoot(DocTreeViewItem::getHtmlFileOrDir(tree))
											+ dirIndex.getParentDirectory().getFileName() + "/"
											+ sDirIndex.getParentDirectory().getFileName() + "/index.html");
						const String& sLinkStr("<li><a href=\"" + sPath + "\">" + sMenuName + "</a></li>");
						menuHtmlStr.add(sLinkStr);
					}
				}

				menuHtmlStr.add("</li></ul>");
			}
		}
	}

	menuHtmlStr.add("</ul></div>");
	return menuHtmlStr.joinIntoString(newLine);
}

//=================================================================================================
String HtmlProcessor::getSiteNavi (const ValueTree& docTree)
{
	String navi;
	ValueTree parent (docTree.getParent());
	String path;

	while (parent.isValid())
    {
		const String& text (parent.getProperty("title").toString());
		navi = "<a href=\"" + path + "index.html\">" + text + "</a>/" + navi;
		path += "../";

		parent = parent.getParent();
	}

	return "<div class=\"siteNavi\">" + navi + "</div>";
}

//=========================================================================
const String HtmlProcessor::getCopyrightInfo()
{
    return "<p><hr>\n"
        "<table id=\"copyright\"><tr><td id=\"copyright\">" +
        FileTreeContainer::projectTree.getProperty ("copyright").toString () +
        "</td><td id=\"copyright\" align=\"right\">Powered by "
        "<a href=\"http://www.underwaySoft.com/wdtp\""
        " target=\"_blank\">WDTP</a> </td></tr></table>";
}
    


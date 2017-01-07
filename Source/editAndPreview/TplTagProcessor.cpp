/*
  ==============================================================================

    TplTagProcessor.cpp
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#include "../WdtpHeader.h"

//=================================================================================================
const String TplTagProcessor::fileAndDirList (const ValueTree& dirTree_,
                                              const bool reverse,
                                              const bool includeDir,
                                              const bool extrctIntro,
                                              const int itemsPrePage/* = 0*/)
{   
    jassert (dirTree_.getType ().toString () != "doc");

    StringArray filesLinkStr;
    const File thisDir (DocTreeViewItem::getHtmlFileOrDir (dirTree_).getParentDirectory());

    ValueTree dirTree (dirTree_.createCopy ());
    TplTagProcessor sorter;
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
                path = "<h2>" + path + "</h2><p>" + tree.getProperty ("description").toString () + "<hr>";
            else
                path += "<br>";
        }

        if (reverse)
            filesLinkStr.add (path);
        else
            filesLinkStr.insert (0, path);
    }

    filesLinkStr.removeEmptyStrings (false);
    return filesLinkStr.joinIntoString (newLine) + Md2Html::copyrightInfo;
}


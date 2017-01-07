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
    String parentPath (DocTreeViewItem::getHtmlFileOrDir (dirTree_).getFullPathName());
    parentPath = parentPath.dropLastCharacters (11); // remove "/index.html"

    ValueTree dirTree (dirTree_.createCopy ());
    TplTagProcessor sorter;
    dirTree.sort (sorter, nullptr, false);

    for (int i = dirTree.getNumChildren(); --i >= 0; )
    {
        const ValueTree& tree (dirTree.getChild (i));
        const String titleStr (tree.getProperty ("title").toString ());

        const File& html (DocTreeViewItem::getHtmlFileOrDir (tree));
        String path (html.getFullPathName().fromFirstOccurrenceOf (parentPath, false, false));
        //DBG (path);

        if (!includeDir && html.getFileNameWithoutExtension () == "index")
        {
            path = String();
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


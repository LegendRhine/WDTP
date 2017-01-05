/*
  ==============================================================================

    TplTagProcessor.cpp
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#include "../WdtpHeader.h"

//=================================================================================================
const String TplTagProcessor::fileAndDirList (const ValueTree& dirTree)
{   
    StringArray filesLinkStr;
    const String parentPath (DocTreeViewItem::getHtmlFileOrDir (dirTree).getFullPathName());

    for (int i = 0; i < dirTree.getNumChildren(); ++i)
    {
        const ValueTree& tree (dirTree.getChild (i));
        const String titleStr (tree.getProperty ("title").toString ());

        const File& html (DocTreeViewItem::getHtmlFileOrDir (tree));
        String path (html.getFullPathName().fromFirstOccurrenceOf (parentPath, false, false));
        path = "<a href=\"" + path + "\">" + titleStr + "</a><br>";

        filesLinkStr.add (path);
    }

    return filesLinkStr.joinIntoString (newLine);
}

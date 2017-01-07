/*
  ==============================================================================

    HtmlProcessor.h
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#ifndef HTMLPROCESSOR_H_INCLUDED
#define HTMLPROCESSOR_H_INCLUDED

struct HtmlProcessor
{
    /** process {{fileAndDirList_xx_xx}}, return its html-code
    
        @pagram dirTree         process this dir's html and dir
        @pagram reverse         true for reverse-date display the items.
        @pagram includeDir      true will display its sub-dir's title
        @pagram extrctIntro     true will display the item's description under its title (like blog)
        @pagram itemsPrePage    0 for unlimit    
    */
    static const String getFileList (const ValueTree& dirTree,
                                     const bool reverse,
                                     const bool includeDir,
                                     const bool extrctIntro,
                                     const int itemsPrePage = 0);

    static const File createArticleHtml (ValueTree& docTree, bool saveProjectAfterCreated);
    static const File createIndexHtml (ValueTree& dirTree, bool saveProjectAfterCreated);

    //=========================================================================
    /** Use for file/dir list sort. Base on create-date */
    const int compareElements (const ValueTree& ft, const ValueTree& st);

};



#endif  // HTMLPROCESSOR_H_INCLUDED

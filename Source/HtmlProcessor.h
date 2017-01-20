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
	/* 1 + 2 => 3 */
	static void renderHtmlContent (const ValueTree& docTree,
		const File& tplFile,
		const File& htmlFile);

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
                                     const int itemsPerPage = 0);

    static const File createArticleHtml (ValueTree& docTree, bool saveProjectAfterCreated);
	static const File createIndexHtml (ValueTree& dirTree, bool saveProjectAfterCreated);

    /** The return result could be showed on the very bottom of every web-page */
    static const String getCopyrightInfo();

    //=========================================================================
    /** Use for file/dir list sort. Base on create-date */
    const int compareElements (const ValueTree& ft, const ValueTree& st);

private:
	/** get dirs of the arg dir */
	static String getSiteMenu (const ValueTree& parent);

	/** get dirs-chain of the arg doc */
	static String getSiteNavi (const ValueTree& docTree);

	static void copyDocMediasToSite (const File& mdFile, const File& htmlFile, const String& htmlStr);
	static const String getRelativePathToRoot (const File &htmlFile);
};



#endif  // HTMLPROCESSOR_H_INCLUDED

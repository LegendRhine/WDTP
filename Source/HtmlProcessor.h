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

   

    //=========================================================================
    /** Use for file/dir list sort. Base on create-date */
    const int compareElements (const ValueTree& ft, const ValueTree& st);

private:
	/** generate site menu. 2 level. */
	static const String getSiteMenu (const ValueTree& tree);
	static const String getSiteNavi (const ValueTree& docTree);
    static const String getContentTitle(const ValueTree& tree);
    static const String getCreateAndModifyTime(const ValueTree& tree);
    static const String getPrevAndNextArticel(const ValueTree& tree);
    
    /** The return result could be showed on the very bottom of every web-page */
    static const String getCopyrightInfo();

	static void copyDocMediasToSite (const File& mdFile, const File& htmlFile, const String& htmlStr);
	static const String getRelativePathToRoot (const File &htmlFile);
    static void getAllArticels(ValueTree fromThisTree, ValueTree& allArticels);

	static const bool hasDirAndAtLeadOneIsMenu(const ValueTree& tree);
};



#endif  // HTMLPROCESSOR_H_INCLUDED

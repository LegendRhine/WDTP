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

    static void getDocNumbersOfTheDir(const ValueTree& dirTree, int& num);

private:
	/** generate site menu. 2 level. */
	static const String getSiteMenu (const ValueTree& tree);
	static const String getSiteNavi (const ValueTree& docTree);
    static const String getContentTitle(const ValueTree& tree);
    static const String getCreateAndModifyTime(const ValueTree& tree);
    static const String getPrevAndNextArticel(const ValueTree& tree);
    static const String getRandomArticels(const ValueTree& notIncludeThisTree, const int howMany);

    /** The return result could be showed on the very bottom of every web-page */
    static const String getCopyrightInfo();

	static void copyDocMediasToSite (const File& mdFile, const File& htmlFile, const String& htmlStr);
	static const String getRelativePathToRoot (const File &htmlFile);

	static const bool hasDirAndAtLeadOneIsMenu(const ValueTree& tree);
    
    //=================================================================================================
    /** get a tree that create time previous/next the arg tree */
    static void getPreviousTree(const ValueTree& oTree, const ValueTree& tree, ValueTree& result);
    static void getNextTree(const ValueTree& oTree, const ValueTree& tree, ValueTree& result);

    /** ad text from the project setup. its formmat should be "imgName link" 
        the img should place in site's add-in folder. */
    static const String getAdStr(const String& text, const File& htmlFile);

    /** return 'howMany' ints, range: 0 ~ getDocNumbersOfTheDir (projectTree). */
    static const Array<int> getRandomInts (const int howMany);

    /** get the arg tree's all child tree's link string. their path base on the arg 2 tree.
        Note: not include the baseOnThisTree's link */
    static void getLinkStrOfAlllDocTrees(const ValueTree& fromThisTree, 
                                         const ValueTree& baseOnThisTree, 
                                         StringArray& linkStr);

};



#endif  // HTMLPROCESSOR_H_INCLUDED

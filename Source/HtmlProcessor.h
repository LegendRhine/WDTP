/*
  ==============================================================================

    HtmlProcessor.h
    Created: 4 Jan 2017 3:34:15pm
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef HTMLPROCESSOR_H_INCLUDED
#define HTMLPROCESSOR_H_INCLUDED

struct HtmlProcessor
{
    HtmlProcessor(const bool sortByReverse_) : sortByReverse(sortByReverse_) { }

	/* 1 + 2 => 3 */
    static void renderHtmlContent(const ValueTree& docTree,
                                  const File& tplFile,
                                  const File& htmlFile);

    static const File createArticleHtml (ValueTree& docTree, bool saveProjectAfterCreated);
	static const File createIndexHtml (ValueTree& dirTree, bool saveProjectAfterCreated);

    //=========================================================================
    /** Use for file/dir list sort. Base on create-date */
    const int compareElements (const ValueTree& ft, const ValueTree& st);

    static void getDocNumbersOfTheDir(const ValueTree& dirTree, int& num);

private:
    /** Process tpl-file's tags */
    static void processTags(const ValueTree& docOrDirTree, const File& htmlFile, String& tplStr);

    static const StringArray getBlogList(const ValueTree& dirTree);
    static const String getBookList(const ValueTree& dirTree);

	/** generate site menu. 2 level dir and 1 level doc.
        that is: if a doc tend for a site menu, it must under the root dir. */
	static const String getSiteMenu (const ValueTree& tree);

	static const String getSiteNavi (const ValueTree& docTree);
    static const String getContentTitle(const ValueTree& tree);
    static const String getCreateAndModifyTime(const ValueTree& tree);
    static const String getPrevAndNextArticel(const ValueTree& tree);
    static const String getRandomArticels(const ValueTree& notIncludeThisTree, const int howMany);

    /** The return result could be showed on the very bottom of every web-page */
    static const String getCopyrightInfo();
    static const String getContactInfo();

	static void copyDocMediasToSite (const File& mdFile, const File& htmlFile, const String& htmlStr);
	static const String getRelativePathToRoot (const File &htmlFile);

	static const bool atLeastHasOneIsMenu(const ValueTree& tree);
    
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
        Note: not include the baseOnThisTree's link and not include create-date, desc, etc. 
        This method is for random articel. */
    static void getLinkStrOfAlllDocTrees(const ValueTree& fromThisTree, 
                                         const ValueTree& baseOnThisTree, 
                                         StringArray& linkStr);

    /** this method is for file-list of index.html. it'll include create date and extra info */
    static void getBlogListHtmlStr(const ValueTree& tree,
                               const File& baseOnthisFile,
                               StringArray& linkStr);

    static void getBookListLinks(const ValueTree& tree, 
                                 const bool isRootTree,
                               StringArray& linkStr);

    /** arg 2: the caller's page number. 1: No.1 (index.html), 2: No.2 (index-2.html)...*/
    static const String getPageNavi(const int howManyPages, const int thisIsNoX);
    static const String getBackPrevLevel();
    static const String getToTop();

    //=================================================================================================
    bool sortByReverse = true;

};



#endif  // HTMLPROCESSOR_H_INCLUDED

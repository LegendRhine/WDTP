/*
  ==============================================================================

    DocTreeViewItem.h
    Created: 16 Sep 2016 5:20:49am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef DOCTREEVIEWITEM_H_INCLUDED
#define DOCTREEVIEWITEM_H_INCLUDED

/** Repsent a doc, a dir or the project-root which showed in treeView. */
class DocTreeViewItem : public TreeViewItem,
                        private ValueTree::Listener
{
public:
    DocTreeViewItem (const ValueTree& tree,
                     FileTreeContainer* container,
                     ItemSorter* itemSorter);
    ~DocTreeViewItem();

    /** Note 1: when this item is the root (project item of the top),
    this method will return this project's 'docs' dir instead of the project file!

    Note 2: this method will return a nonexists file when the arg is invalid. */
    static const File getMdFileOrDir (const ValueTree& tree);

    /** these two methods will return index.html when it's a dir. */
    static const File getHtmlFileOrDir (const File& mdFileOrDir);
    static const File getHtmlFileOrDir (const ValueTree& tree);

    /** get a html-file's all local media files. the result would be stored in arg-2.
        return: media-files' number of this html-file included. */
    static const int getHtmlMediaFiles (const File& htmlFile, Array<File>& files);

    /** get a doc-file's all local media files. the result would be stored in arg-2.
        return: media-files' number of this doc-file included.	*/
    static const int getMdMediaFiles (const File& doc, Array<File>& files);

    /** let the arg tree and all its parents set to needCreateHtml. */
    static void needCreate (ValueTree tree);
    static void allChildrenNeedCreate (ValueTree tree);

    /** for drag-drop/moving items */
    static void moveItems (const OwnedArray<ValueTree>& items,
                           ValueTree newParent);

    /** callback method whnever the project-tree has some changed */
    void refreshDisplay();
    const ValueTree& getTree() const       { return tree; }

    /** override the parent class... */
    virtual bool mightContainSubItems() override;
    virtual String getUniqueName() const override;
    virtual void itemOpennessChanged (bool isNowOpen) override;

    virtual String getTooltip() override;
    virtual void paintItem (Graphics& g, int width, int height) override;

    /** core method, it'll call the edit/preview area's startWork()*/
    virtual void itemSelectionChanged (bool isNowSelected) override;

    /** right-click popup-menu is here */
    virtual void itemClicked (const MouseEvent& e) override;

    /** double click for rename it */
    virtual void itemDoubleClicked (const MouseEvent& e) override;

    /** for move items.. */
    virtual var getDragSourceDescription() override;
    virtual bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& details) override;
    virtual void itemDropped (const DragAndDropTarget::SourceDetails& details,
                              int insertIndex) override;

    /** files drag to import */
    virtual bool isInterestedInFileDrag (const StringArray&	files) override;
    virtual void filesDropped (const StringArray& files, int insertIndex) override;

    void importExternalDocs (const Array<File>& docs);

    /** draw lines from within the file-tree panel.. */
    virtual void paintHorizontalConnectingLine (Graphics&, const Line<float>& line) override;
    virtual void paintVerticalConnectingLine (Graphics&, const Line<float>& line) override;
    virtual void paintOpenCloseButton (Graphics&, const Rectangle<float>&, Colour, bool) override;

private:
    //=========================================================================
    /** for exportAsHtml. Note: the arg item must be a dir. */
    static const bool getDirDocsAndAllMedias (DocTreeViewItem* item,
                                              const File& mdFile,
                                              Array<File>& medias);

    static DocTreeViewItem* getRootItem (DocTreeViewItem* subItem);

    static void statis (const ValueTree& tree, int& dirNums,
                        int& totalWords, int& totalImgs);

    static void getWordsAndImgNumsInDoc (const ValueTree& tree, int& words, int& imgNums);

    //=========================================================================
    enum MenuIndex
    {
        newDir = 1, newDoc, 
        importTextDocs,
        packWholeSite, packHtmls, packMedias,
        exportTextDoc, exportDocs, dataStatis, getItemPath,
        replaceIn, rename, deleteThis, 
        vewInFinder, openInExEditor, browseInEx
    };

    void menuPerform (const int menuIndex);
    
    void importExternalDocs();
    void packSiteData (const bool includeHtmls, const bool includeMedias);
    void exportAsTextDoc();
    void exportAsHtml(); 
    void renameSelectedItem();
    void createNewDocument();
    const File createDoc (const String &docName, const bool selectAfterCreated);

    void createNewFolder();
    void deleteSelected();
    void statistics();
    void getPath();
    void replaceContent();

    //=========================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override;
    void valueTreeParentChanged (ValueTree&) override { }
    void treeChildrenChanged (const ValueTree& parentTree);

    //=========================================================================
    ValueTree tree; // no need and must NOT be refernce!!
    FileTreeContainer* treeContainer;
    ItemSorter* sorter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocTreeViewItem)
};


#endif  // HEADERGUA

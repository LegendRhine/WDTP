/*
  ==============================================================================

    DocTreeViewItem.h
    Created: 16 Sep 2016 5:20:49am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef DOCTREEVIEWITEM_H_INCLUDED
#define DOCTREEVIEWITEM_H_INCLUDED

/** Repsent a doc, a dir or the project which showed in treeView. */
class DocTreeViewItem : public TreeViewItem,
                        private ValueTree::Listener
{
public:
    DocTreeViewItem (const ValueTree& tree,
                     FileTreeContainer* container,
                     ItemSorter* itemSorter);
    ~DocTreeViewItem ();

    /** Note 1: when this item is the root (project item of the top),
    this method will return this project's 'docs' dir,
    instead of the project file!

    Note 2: this method always return a valid file object however it exsits or not,
    if to check the file exists or not, use this sentence: getFileOfThisItem().exists();
    or: getFileOfThisItem().existsAsFile(); or: getFileOfThisItem().isDirectory()
    */
    static const File getFileOrDir (const ValueTree& tree);

    // static public methods for drag-drop moving items..
    static void moveItems (const OwnedArray<ValueTree>& items, 
                           ValueTree newParent);

    void refreshDisplay ();
    const ValueTree& getTree () const    { return tree; }

    // override...
    virtual bool mightContainSubItems () override;
    virtual String getUniqueName () const override;
    virtual void itemOpennessChanged (bool isNowOpen) override;

    virtual String getTooltip () override;
    virtual void paintItem (Graphics& g, int width, int height) override;
    virtual void itemSelectionChanged (bool isNowSelected) override;
    virtual void itemClicked (const MouseEvent& e) override;

    // for move items..
    virtual var getDragSourceDescription () override;
    virtual bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& details) override;
    virtual void itemDropped (const DragAndDropTarget::SourceDetails& details,
                              int insertIndex) override;

    // draw line..
    virtual void paintHorizontalConnectingLine (Graphics&, const Line<float>& line) override;
    virtual void paintVerticalConnectingLine (Graphics&, const Line<float>& line) override;

private:
    //=========================================================================
    /** export the selected item (all project-docs, a dir-docs or a doc) as a single md file. */
    static const bool exportDocsAsMd (DocTreeViewItem* item,
                                      const ValueTree& tree,
                                      const File& fileAppendTo);

    static DocTreeViewItem* getRootItem (DocTreeViewItem* subItem);

    //=========================================================================
    void menuPerform (const int menuIndex);

    // internal call the static method exportDocsAsMd()
    void exportAsMdFile ();
    void renameSelectedItem ();
    void importDocuments ();
    void createNewDocument ();
    void createNewFolder ();
    void deleteSelected ();

    //=========================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override;
    void valueTreeParentChanged (ValueTree&) override    { }
    void treeChildrenChanged (const ValueTree& parentTree);

    //=========================================================================
    ValueTree tree; // must NOT be refernce!!
    FileTreeContainer* treeContainer;
    ItemSorter* sorter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocTreeViewItem)

};


#endif  // HEADERGUA
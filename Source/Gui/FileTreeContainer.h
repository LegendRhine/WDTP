/*
  ==============================================================================

    FileTreeContainer.h
    Created: 7 Sep 2016 7:37:28am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef FILETREECONTAINER_H_INCLUDED
#define FILETREECONTAINER_H_INCLUDED

class FileTreeContainer;
class DocTreeViewItem;

//=========================================================================
/** For sort items of class DocTreeViewItem. 
    Usage: create object, then call setTreeViewItem().
*/
class ItemSorter : public Value::Listener
{
public:
    ItemSorter (ValueTree& tree);
    ~ItemSorter();

    void setTreeViewItem (DocTreeViewItem* item) { rootItem = item; }

    const int getOrder() const              { return var (order);     }
    const int getShowWhat() const           { return var (showWhat);  }
    const int getTooltipToShow() const      { return var (tooltip);   }
    const int getAscending()  const         { return var (ascending); }
    const int getWhichFirst()  const        { return var (dirFirst);  }

    void setOrder (const int value)         { order = value;     }
    void setShowWhat(const int value)       { showWhat = value;  }
    void setTooltipToShow (const int value) { tooltip = value;   }
    void setAscending (const int value)     { ascending = value; }
    void setWhichFirst (const int value)    { dirFirst = value;  }

    virtual void valueChanged (Value& value) override;

private:
    //=========================================================================
    ValueTree& tree;
    DocTreeViewItem* rootItem;

    Value order;
    Value showWhat;
    Value tooltip;
    Value ascending;
    Value dirFirst;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemSorter)
};

//=========================================================================
/** Repsent a doc, a dir or the project which showed in treeView. */
class DocTreeViewItem : public TreeViewItem,
                        private ValueTree::Listener,
                        private Value::Listener
{
public:
    DocTreeViewItem (const ValueTree& tree, 
                     FileTreeContainer* container, 
                     ItemSorter* itemSorter);
    ~DocTreeViewItem ();

    // static public methods..
    static void moveItems (const OwnedArray<ValueTree>& items, ValueTree newParent);

    void refreshDisplay ();

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
    /** Note 1: when this item is the root (project item of the top),
    this method will return this project's 'docs' dir,
    instead of the project file!

    Note 2: this method always return a valid file object however it exsits or not,
    if to check the file exists or not, use this sentence: getFileOfThisItem().exists();
    or: getFileOfThisItem().existsAsFile(); or: getFileOfThisItem().isDirectory()*/
    static const File getFileOrDir (const ValueTree& tree);

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
    void importDocuments();
    void createNewDocument();
    void createNewFolder();
    void delSelected ();

    //=========================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override;
    void valueTreeParentChanged (ValueTree&) override    { }
    void treeChildrenChanged (const ValueTree& parentTree);

    // for sort
    virtual void valueChanged (Value& value) override;

    //=========================================================================
    ValueTree tree; // must NOT be refernce!!
    FileTreeContainer* treeContainer;    
    ItemSorter* sorter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocTreeViewItem)

};

//==============================================================================
/**  */
class FileTreeContainer    : public Component,
                             public DragAndDropContainer
{
public:
    FileTreeContainer (EditAndPreview* editAndPreview);
    ~FileTreeContainer();

    void resized() override;

    void openProject (const File& projectFile);
    void closeProject ();
    const bool hasLoadedProject () const;
    const bool saveDocAndProject () const;

    EditAndPreview* getEditAndPreview () const    { return editAndPreview; }

    static File projectFile;

private:
    //=========================================================================
    ScopedPointer<DocTreeViewItem> docTreeItem;
    ScopedPointer<ItemSorter> sorter;
    TreeView fileTree;
    ValueTree projectTree;
    EditAndPreview* editAndPreview;

    bool projectloaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeContainer)

};


#endif  // FILETREECONTAINER_H_INCLUDED

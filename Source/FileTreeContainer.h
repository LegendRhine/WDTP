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

    const int compareElements (TreeViewItem* first,
                               TreeViewItem* second) const;

    //============================================================================
    const int getOrder() const             { return var (order); }
    const int getShowWhat() const          { return var (showWhat); }
    const int getTooltipToShow() const     { return var (tooltip); }
    const int getAscending()  const        { return var (ascending); }
    const int getWhichFirst()  const       { return var (dirFirst); }

    void setOrder (const int value)         { order.setValue (value); }
    void setShowWhat (const int value)      { showWhat.setValue (value); }
    void setTooltipToShow (const int value) { tooltip.setValue (value); }
    void setAscending (const int value)     { ascending.setValue (value); }
    void setWhichFirst (const int value)    { dirFirst.setValue (value); }

    /** update the file-tree ui and save the project */
    virtual void valueChanged (Value& value) override;

private:
    //=========================================================================
    ValueTree& projectTree;
    DocTreeViewItem* rootItem;

    Value order, showWhat, tooltip, ascending, dirFirst;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemSorter)
};

//==============================================================================
/** Showed in main interface's left.
*/
class FileTreeContainer : public Component,
                          public DragAndDropContainer
{
public:
    FileTreeContainer (EditAndPreview* editAndPreview);
    ~FileTreeContainer();

    void resized() override;
    void paint (Graphics& g) override;

    void openProject (const File& projectFile);
    void closeProject();

    const Array<TreeViewItem*> getSelectedItems() const;
    const bool saveOpenSateAndSelect (const bool alsoSaveProject);
    const bool saveDocAndProject();

    const bool hasLoadedProject() const                    { return projectTree.isValid(); }
    TreeView& getTreeView()                                { return fileTree; }
    EditAndPreview* getEditAndPreview() const              { return editAndPreview; }

    const bool aDocSelectedCurrently() const;
    void reloadCurrentDoc();

    static bool saveProject();
    const bool selectItemFromHtmlFile (const File& html);

    // core static objects. this's a BAD design I totally know that but it's handy :)
    static File projectFile;
    static ValueTree projectTree;

private:
    //=========================================================================
    ScopedPointer<DocTreeViewItem> docTreeItem;
    ScopedPointer<ItemSorter> sorter;

    TreeView fileTree;
    EditAndPreview* editAndPreview;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeContainer)

};


#endif  // FILETREECONTAINER_H_INCLUDED

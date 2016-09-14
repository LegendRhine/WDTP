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

//=========================================================================
/** Repsent a doc, a dir or the project which showed in treeView. */
class DocTreeViewItem : public TreeViewItem
{
public:
    DocTreeViewItem (const ValueTree& tree,
                     FileTreeContainer* container);
    ~DocTreeViewItem () { }

    virtual bool mightContainSubItems () override;
    virtual String getUniqueName () const override;
    virtual void itemOpennessChanged (bool isNowOpen) override;

    virtual void paintItem (Graphics& g, int width, int height) override;
    virtual void itemSelectionChanged (bool isNowSelected) override;
    virtual void itemClicked (const MouseEvent& e) override;

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

    //=========================================================================
    void menuPerform (const int menuIndex);

    void renameSelectedItem();
    void exportAsMdFile();
    void importDocuments();
    void createNewDocument();
    void createNewFolder();
    void moveSelectedTo();

    ValueTree tree; // must NOT be refernce!!
    FileTreeContainer* treeContainer;
    bool isAscendingOrder = true;

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
    TreeView fileTree;
    ValueTree projectTree;
    EditAndPreview* editAndPreview;

    bool projectloaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeContainer)

};


#endif  // FILETREECONTAINER_H_INCLUDED

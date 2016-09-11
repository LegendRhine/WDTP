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
/**  */
class DocTreeViewItem : public TreeViewItem
{
public:
    DocTreeViewItem (const ValueTree& tree, 
                     const File& projectFile,
                     FileTreeContainer* container);
    ~DocTreeViewItem () { }

    virtual bool mightContainSubItems () override;
    virtual String getUniqueName () const override;
    virtual void itemOpennessChanged (bool isNowOpen) override;

    virtual void paintItem (Graphics& g, int width, int height) override;
    virtual void itemClicked (const MouseEvent& e) override;

private:
    const ValueTree tree; // must NOT be refernce!!
    const File& projectFile;
    FileTreeContainer* treeContainer;

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

private:
    File projectFile;
    ScopedPointer<DocTreeViewItem> docTreeItem;
    TreeView fileTree;
    ValueTree projectTree;
    EditAndPreview* editAndPreview;

    bool projectloaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeContainer)

};


#endif  // FILETREECONTAINER_H_INCLUDED

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

    const int getOrder() const              { return var (order);     }
    const int getShowWhat() const           { return var (showWhat);  }
    const int getTooltipToShow() const      { return var (tooltip);   }
    const int getAscending()  const         { return var (ascending); }
    const int getWhichFirst()  const        { return var (dirFirst);  }

    void setOrder (const int value)         { order.setValue (value);     }
    void setShowWhat(const int value)       { showWhat.setValue (value);  }
    void setTooltipToShow (const int value) { tooltip.setValue (value);   }
    void setAscending (const int value)     { ascending.setValue (value); }
    void setWhichFirst (const int value)    { dirFirst.setValue (value);  }

    virtual void valueChanged (Value& value) override;

private:
    //=========================================================================
    ValueTree& projectTree;
    DocTreeViewItem* rootItem = nullptr;

    Value order;
    Value showWhat;
    Value tooltip;
    Value ascending;
    Value dirFirst;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemSorter)
};

//==============================================================================
/** Showed in main interface's left.
*/
class FileTreeContainer    : public Component,
                             public DragAndDropContainer
{
public:
    FileTreeContainer (EditAndPreview* editAndPreview);
    ~FileTreeContainer();

    void resized () override;
    void paint (Graphics& g) override;

    void openProject (const File& projectFile);
    void closeProject();
    const bool saveDocAndProject ();

    const bool hasLoadedProject () const                      { return projectTree.isValid(); }
    TreeView& getTreeView()                                   { return fileTree; }

    EditAndPreview* getEditAndPreview () const                { return editAndPreview; }
    void setIdentityOfLastSelectedItem (const String& str)    { lastItem = str; }
    void selectIdentityItem();

    static bool saveProject ();
    static File projectFile;
    static ValueTree projectTree;

private:
    //=========================================================================
    ScopedPointer<DocTreeViewItem> docTreeItem;
    ScopedPointer<ItemSorter> sorter;

    TreeView fileTree;
    EditAndPreview* editAndPreview;

    String lastItem;  // identity of last selected item

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeContainer)

};


#endif  // FILETREECONTAINER_H_INCLUDED

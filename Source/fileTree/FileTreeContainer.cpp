/*
  ==============================================================================

    FileTreeContainer.cpp
    Created: 7 Sep 2016 7:37:28am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

extern PropertiesFile* systemFile;
File FileTreeContainer::projectFile;

//==============================================================================
FileTreeContainer::FileTreeContainer (EditAndPreview* rightArea) :
    editAndPreview (rightArea)
{
    jassert (editAndPreview != nullptr);

    // fileTree...
    fileTree.setRootItem (nullptr);
    fileTree.setRootItemVisible (true);
    fileTree.setDefaultOpenness (true);
    fileTree.setMultiSelectEnabled (true);
    fileTree.setOpenCloseButtonsVisible (true);
    fileTree.setIndentSize (15);   
    fileTree.getViewport()->setScrollBarThickness (10);

    addAndMakeVisible (fileTree);
}

//=========================================================================
FileTreeContainer::~FileTreeContainer()
{
    fileTree.setRootItem (nullptr);
}

//=========================================================================
void FileTreeContainer::resized()
{
    fileTree.setVisible (getWidth () > 50);
    fileTree.setBounds (10, 0, getWidth () - 10, getHeight ());
}

//=================================================================================================
void FileTreeContainer::openProject (const File& project)
{
    // check if the file exists and could write
    if (!(project.existsAsFile() && project.hasWriteAccess()))
    {
        AlertWindow::showMessageBox (AlertWindow::InfoIcon, TRANS ("Message"),
                                     TRANS ("Project file \"") + project.getFullPathName() + 
                                     "\" " + TRANS ("is nonexistent or cannot be written to."));
        return;
    }

    // check if this project alreay opened, if not, initial the projectTree
    if (project == projectFile)
    {
        return;
    }
    else
    {
        closeProject ();
        projectTree = SwingUtilities::readValueTreeFromFile (project);
    }

    // check if this is an vaild project file
    if (projectTree.getType().toString() != "wdtpProject")
    {
        AlertWindow::showMessageBox (AlertWindow::InfoIcon, TRANS ("Message"),
                                     TRANS ("An invalid project file."));
        return;
    }

    // if there's a project alreay opened, save it and clear the treeView    
    if (hasLoadedProject() && !saveDocAndProject())
    {
        AlertWindow::showMessageBox (AlertWindow::InfoIcon, TRANS ("Message"),
                                     TRANS ("Something wrong when saving the current document.") + newLine +
                                     TRANS ("Please check it out, then open the new project again."));
        return;
    }

    // load the project
    projectFile = project;
    
    sorter = new ItemSorter (projectTree);
    docTreeItem = new DocTreeViewItem (projectTree, this, sorter);
    sorter->setTreeViewItem (docTreeItem);

    fileTree.setRootItem (docTreeItem);
    projectloaded = true;

    // change the text of main window's title-bar
    MainWindow* mainWindow = dynamic_cast<MainWindow*>(getTopLevelComponent ());
    jassert (mainWindow != nullptr);
    mainWindow->setName (JUCEApplication::getInstance ()->getApplicationName () + " - " +
                         project.getFileNameWithoutExtension ());

    // add the project to recent opened file list
    RecentlyOpenedFilesList  recentFiles;
    recentFiles.setMaxNumberOfItems (10);
    recentFiles.removeNonExistentFiles ();
    recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
    recentFiles.addFile (project);

    systemFile->setValue ("recentFiles", recentFiles.toString ());
}

//=================================================================================================
void FileTreeContainer::closeProject ()
{
    if (hasLoadedProject () && saveDocAndProject ())
    {
        fileTree.setRootItem (nullptr);
        docTreeItem = nullptr;
        sorter = nullptr;
        projectTree = ValueTree::invalid;
        projectFile = File::nonexistent;
        projectloaded = false;
        editAndPreview->projectClosed();

        // change the text of main window's title-bar
        MainWindow* mainWindow = dynamic_cast<MainWindow*>(getTopLevelComponent ());
        jassert (mainWindow != nullptr);
        mainWindow->setName (JUCEApplication::getInstance ()->getApplicationName ());
    }
}

//=================================================================================================
const bool FileTreeContainer::hasLoadedProject () const
{
    return projectloaded;
}

//=================================================================================================
const bool FileTreeContainer::saveDocAndProject () const
{
    //NEED_TO_DO ("saveDocAndProject");

    return true;
}

//=================================================================================================
ItemSorter::ItemSorter (ValueTree& tree_) 
    : tree(tree_)
{
    jassert (tree.isValid());

    order.setValue (tree.getProperty ("order"));
    showWhat.setValue (tree.getProperty ("showWhat"));
    tooltip.setValue (tree.getProperty ("tooltip"));
    ascending.setValue (tree.getProperty ("ascending"));
    dirFirst.setValue (tree.getProperty ("dirFirst"));

    order.addListener (this);
    showWhat.addListener (this);
    ascending.addListener (this);
    tooltip.addListener (this);
    dirFirst.addListener (this);
}

//=========================================================================
ItemSorter::~ItemSorter ()
{
    order.removeListener (this);
    showWhat.removeListener (this);
    tooltip.removeListener (this);
    ascending.removeListener (this);
    dirFirst.removeListener (this);
}

//=================================================================================================
const int ItemSorter::compareElements (TreeViewItem* first, TreeViewItem* second) const
{
    DocTreeViewItem* f = dynamic_cast<DocTreeViewItem*> (first);
    DocTreeViewItem* s = dynamic_cast<DocTreeViewItem*> (second);

    if (f == nullptr || s == nullptr)
        return 0;

    const ValueTree& ft (f->getTree());
    const ValueTree& st (s->getTree());

    // root tree
    if (ft.getType().toString() == "wdtpProject")
        return -1;

    if (st.getType().toString() == "wdtpProject")
        return 1;

    const File& ff (DocTreeViewItem::getFileOrDir (ft));
    const File& sf (DocTreeViewItem::getFileOrDir (st));
    const bool isAscending = (ascending.getValue() == var(0));
    const bool isDirFirst = (dirFirst.getValue() == var(0));

    // one is dir or both are dir, or both are doc. here must use ValueTree rather than file
    // bacause the file maybe nonexists..
    if (ft.getType().toString() == "dir" && st.getType().toString() == "doc")
    {
        return isDirFirst ? -1 : 1;
    }
    else if (ft.getType().toString() == "doc" && st.getType().toString() == "dir")
    {
        return isDirFirst ? 1 : -1;
    }
    else  // doc vs doc and dir vs dir..
    {        
        if (0 == order) // file name
        {
            const int r = ft.getProperty ("name").toString ().compareIgnoreCase (ft.getProperty ("name").toString ());
            return isAscending ? r : -r;
        }
        else if (1 == order) // title or descrition
        {
            const int r = ft.getProperty ("title").toString ().compareIgnoreCase (ft.getProperty ("title").toString ());
            return isAscending ? r : -r;
        }
        else if (2 == order) // webname
        {
            const int r = ft.getProperty ("webName").toString ().compareIgnoreCase (ft.getProperty ("webName").toString ());
            return isAscending ? r : -r;
        }
        else if (3 == order) // file size
        {
            const int r = int (ff.getSize () - sf.getSize ());
            return isAscending ? r : -r;
        }
        else if (4 == order) // create time
        {
            if (!(ff.exists() && sf.exists()))
                return 0;

            const bool b = (ff.getCreationTime () < sf.getCreationTime ());
            const int r = b ? -1 : 1;
            return isAscending ? -r : r;
        }
        else if (5 == order) // modified time
        {
            if (!(ff.exists() && sf.exists()))
                return 0;

            const bool b = (ff.getLastModificationTime () < sf.getLastModificationTime ());
            const int r = b ? -1 : 1;
            return isAscending ? -r : r;
        }
    }

    jassertfalse;  // should never run here..
    return 0;  
}

//=================================================================================================
void ItemSorter::valueChanged (Value& value)
{
    // haven't called setTreeViewItem() yet? See this class' description..
    jassert (rootItem != nullptr);    
    rootItem->refreshDisplay();

    // update project-tree
    if (value == order)           tree.setProperty ("order", order, nullptr);
    else if (value == showWhat)   tree.setProperty ("showWhat", showWhat, nullptr);
    else if (value == ascending)  tree.setProperty ("ascending", ascending, nullptr);
    else if (value == tooltip)    tree.setProperty ("tooltip", tooltip, nullptr);
    else if (value == dirFirst)   tree.setProperty ("dirFirst", dirFirst, nullptr);    

    // save the project file
    if (!SwingUtilities::writeValueTreeToFile (tree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
}



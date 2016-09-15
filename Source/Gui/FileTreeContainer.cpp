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
DocTreeViewItem::DocTreeViewItem (const ValueTree& tree_, 
                                  FileTreeContainer* container,
                                  ItemSorter* itemSorter) :
    tree (tree_), 
    treeContainer (container),
    sorter (itemSorter)
{
    jassert (treeContainer != nullptr);
    jassert (sorter != nullptr);

    // highlight for the whole line
    //setDrawsInLeftMargin (true); 
    setLinesDrawnForSubItems (true);
    tree.addListener (this);
}

//=================================================================================================
DocTreeViewItem::~DocTreeViewItem ()
{
    tree.removeListener (this);
}

//=================================================================================================
bool DocTreeViewItem::mightContainSubItems ()
{
    return tree.getNumChildren() > 0;
}

//=================================================================================================
String DocTreeViewItem::getUniqueName () const
{
    return getFileOrDir (tree).getFullPathName();
}

//=================================================================================================
void DocTreeViewItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen && getNumSubItems() == 0)
        refreshDisplay();
    else
        clearSubItems();
}

//=================================================================================================
void DocTreeViewItem::paintItem (Graphics& g, int width, int height)
{
    g.setFont (SwingUtilities::getFontSize () - 2.f);
    int leftGap = 4;
    Colour c (0xff303030);

    // doc and dir item
    if (tree.getType ().toString () != "wdtpProject") 
    {
        leftGap = 22;
        Image icon;

        if (tree.getType ().toString () == "doc")
        {
            icon = ImageCache::getFromMemory (BinaryData::doc_png, BinaryData::doc_pngSize);
        }
        else
        {
            if (isOpen ())
                icon = ImageCache::getFromMemory (BinaryData::diro_png, BinaryData::diro_pngSize);
            else
                icon = ImageCache::getFromMemory (BinaryData::dir_png, BinaryData::dir_pngSize);
        }

        g.drawImageAt (icon, 2, getItemHeight () / 2 - 8);
    }
    
    if (!getFileOrDir (tree).exists())
        c = Colours::red;

    g.setColour (c);
    
    String itemName;
    
    if (sorter->getShowWhat() == 0)  // file name
        itemName = tree.getProperty ("name").toString();

    else if (sorter->getShowWhat() == 1) // title or intro
        itemName = tree.getProperty ("title").toString ();
    
    else if (sorter->getShowWhat() == 2)  // webpage name
        itemName = tree.getProperty ("webName").toString ();

    g.drawText (itemName, leftGap, 0, width - 4, height, Justification::centredLeft, true);
}

//=================================================================================================
const File DocTreeViewItem::getFileOrDir (const ValueTree& tree)
{
    const File& root (FileTreeContainer::projectFile.getSiblingFile ("docs"));

    if (tree.getType().toString() == "wdtpProject")
    {
        return root;
    }
    else
    {
        String path = tree.getProperty ("name").toString();
        ValueTree t (tree);

        while (t.getParent().isValid() && t.getParent().getType().toString() != "wdtpProject")
        {
            t = t.getParent();
            path = t.getProperty ("name").toString() + "/" + path;
        }

        if (tree.getType().toString() == "doc")
            path += ".md";

        return root.getChildFile (path);
    }
}

//=================================================================================================
// left click
void DocTreeViewItem::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        EditAndPreview* editArea = treeContainer->getEditAndPreview ();

        // doc
        if (tree.getType ().toString () == "doc")
        {
            if (getFileOrDir (tree).existsAsFile ())
            {
                if (0 == systemFile->getIntValue ("clickForEdit"))
                    editArea->editDoc (getFileOrDir (tree));
                else
                    editArea->previewDoc (getFileOrDir (tree));

                editArea->setDocProperties (tree);
            }
            else
            {
                //editArea->whenFileOrDirNonexists ();
                SHOW_MESSAGE (getFileOrDir (tree).getFullPathName ());
            }
        }
        // dir
        else if (tree.getType ().toString () == "dir")
        {
            if (getFileOrDir (tree).isDirectory ())
                editArea->setDirProperties (tree);
            else
                //editArea->whenFileOrDirNonexists ();
                SHOW_MESSAGE (getFileOrDir (tree).getFullPathName ());
        }
        else // root
        {
            editArea->setProjectProperties ();
        }
    }
}

//=================================================================================================
// right click
void DocTreeViewItem::itemClicked (const MouseEvent& e)
{   
    const bool exist = getFileOrDir (tree).exists ();
    const bool isDoc = (tree.getType ().toString () == "doc");
    const bool isDir = (tree.getType ().toString () == "dir");
    const bool isRoot = (tree.getType ().toString () == "wdtpProject");
    const bool onlyOneSelected = getOwnerView()->getNumSelectedItems () == 1;

    // right click
    if (e.mods.isPopupMenu())
    {
        PopupMenu m;
        m.addItem (1, TRANS ("New Folder..."), exist && !isDoc && onlyOneSelected);
        m.addItem (2, TRANS ("New Document..."), exist && !isDoc && onlyOneSelected);
        m.addSeparator ();

        m.addItem (3, TRANS ("Import..."), exist && !isDoc && onlyOneSelected);
        m.addItem (4, TRANS ("Export..."), exist && onlyOneSelected);
        m.addSeparator ();

        PopupMenu sortMenu;
        sortMenu.addItem (100, TRANS ("File Name"), true, sorter->getOrder() == 0);
        sortMenu.addItem (101, TRANS ("Title / Intro"), true, sorter->getOrder () == 1);
        sortMenu.addItem (102, TRANS ("Web Name"), true, sorter->getOrder () == 2);
        sortMenu.addItem (103, TRANS ("File Size"), true, sorter->getOrder () == 3);
        sortMenu.addItem (104, TRANS ("Create Time"), true, sorter->getOrder () == 4);
        sortMenu.addItem (105, TRANS ("Modified Time"), true, sorter->getOrder () == 5);
        sortMenu.addSeparator ();
        sortMenu.addItem (106, TRANS ("Ascending Order"), true, sorter->getAscending() == 0);
        sortMenu.addItem (107, TRANS ("Folder First"), true, sorter->getWhichFirst() == 0);

        m.addSubMenu (TRANS ("Sort by"), sortMenu, exist && !isDoc);

        PopupMenu showedAsMenu;
        showedAsMenu.addItem (200, TRANS ("File Name"), true, sorter->getShowWhat() == 0);
        showedAsMenu.addItem (201, TRANS ("Title / Intro"), true, sorter->getShowWhat() == 1);
        showedAsMenu.addItem (202, TRANS ("Web Name"), true, sorter->getShowWhat() == 2);

        m.addSubMenu (TRANS ("Showed as"), showedAsMenu, exist && !isRoot);

        PopupMenu tooltipAsMenu;
        tooltipAsMenu.addItem (300, TRANS ("File Path"), true, sorter->getTooltipToShow() == 0);
        tooltipAsMenu.addItem (301, TRANS ("Title / Intro"), true, sorter->getTooltipToShow() == 1);
        tooltipAsMenu.addItem (302, TRANS ("Web Name"), true, sorter->getTooltipToShow() == 2);

        m.addSubMenu (TRANS ("Tooltip for"), tooltipAsMenu, exist && !isRoot);
        m.addSeparator ();

        m.addItem (10, TRANS ("Rename..."), !isRoot && onlyOneSelected);
        m.addItem (12, TRANS ("Delete..."), !isRoot);
        m.addSeparator ();

        m.addItem (14, TRANS ("Open in Explorer / Finder..."), exist && onlyOneSelected);
        m.addItem (15, TRANS ("Open in External Editor..."), exist && isDoc && onlyOneSelected);

        menuPerform (m.show());
    }
}

//=================================================================================================
void DocTreeViewItem::menuPerform (const int index)
{
    if (index == 1)
        createNewFolder ();
    else if (index == 2)
        createNewDocument ();
    else if (index == 3)
        importDocuments ();
    else if (index == 4)
        exportAsMdFile ();
    else if (index == 10)
        renameSelectedItem ();
    else if (index == 12)
        delSelected ();

    // open in external app..
    else if (index == 14)
        getFileOrDir (tree).revealToUser ();
    else if (index == 15)
        getFileOrDir (tree).startAsProcess ();

    // sort...
    else if (index >= 100 && index <= 105)
        sorter->setOrder (index - 100);
    else if (index == 106)
        sorter->setAscending ((sorter->getAscending() == 0) ? 1 : 0);
    else if (index == 107)
        sorter->setWhichFirst ((sorter->getWhichFirst() == 0) ? 1 : 0);
    else if (index >= 200 && index <= 202)
        sorter->setShowWhat (index - 200);
    else if (index >= 300 && index <= 302)
        sorter->setTooltipToShow (index - 300);

}

//=================================================================================================
void DocTreeViewItem::renameSelectedItem ()
{
    const File& fileOrDir (getFileOrDir (tree));
    jassert (tree.getType ().toString () != "wdtpProject");

    AlertWindow dialog (TRANS ("Rename the selected item"), TRANS ("Please input the new name."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", fileOrDir.getFileNameWithoutExtension ());
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop ())
    {
        const String inputStr (dialog.getTextEditor ("name")->getText());

        if (inputStr == tree.getProperty ("name").toString())
            return;

        String newName (SwingUtilities::getValidFileName (inputStr));

        if (newName.isEmpty ())
            newName = TRANS ("Untitled");

        File newFile (getFileOrDir (tree).getSiblingFile (newName + (fileOrDir.isDirectory () ? String () 
                                                                     : ".md")));
        newFile = newFile.getNonexistentSibling (true);
        fileOrDir.moveFileTo (newFile);

        // save the project file
        tree.setProperty ("name", newFile.getFileNameWithoutExtension (), nullptr);

        ValueTree rootTree = tree;

        while (rootTree.getParent ().isValid ())
            rootTree = rootTree.getParent ();

        if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
    }
}

//=================================================================================================
void DocTreeViewItem::exportAsMdFile ()
{
    FileChooser fc (TRANS ("Export document(s)..."), File::getSpecialLocation (
        File::userDocumentsDirectory).getChildFile (tree.getProperty ("name").toString () + ".md"),
        "*.md", false);

    if (!fc.browseForFileToSave (false))
        return;

    File mdFile (fc.getResult());
    mdFile = mdFile.getSiblingFile (SwingUtilities::getValidFileName (mdFile.getFileNameWithoutExtension()));

    if (!mdFile.hasFileExtension (".md"))
        mdFile = mdFile.withFileExtension ("md");

    // overwrite or not if it has been there
    if (mdFile.existsAsFile () &&
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                       TRANS ("This file already exists, want to overwrite it?")))
    {
        return;
    }

    if (!mdFile.deleteFile())
    {
        SHOW_MESSAGE (TRANS ("Can't write to this file! "));
        return;
    }

    mdFile.create ();

    // single doc or dir-docs
    if ((tree.getType ().toString () == "doc") ? getFileOrDir (tree).copyFileTo (mdFile)
                                              : exportDocsAsMd (this, tree, mdFile))
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::InfoIcon, TRANS ("Message"),
                                          TRANS ("File: \"") + mdFile.getFullPathName () + "\"" 
                                          + newLine + TRANS ("Exported Successful! "),
                                          TRANS ("Open")))
            mdFile.startAsProcess ();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Export Failed."));
    }
}

//=================================================================================================
void DocTreeViewItem::importDocuments ()
{
    // can't import any doc under a doc!
    jassert (tree.getType ().toString () != "doc");

    FileChooser fc (TRANS ("Import document(s)..."), File::nonexistent,
                    "*.md;*.markdown;*.txt;*.html;*.htm", false);

    if (!fc.browseForMultipleFilesToOpen ())
        return;

    const Array<File> docFiles (fc.getResults ());
    const File thisDir (getFileOrDir (tree));
    jassert (thisDir.isDirectory ()); // can't copy external docs to a nonexists dir

                                     // get the root, here we need some info from it
    ValueTree rootTree = tree;

    while (rootTree.getParent ().isValid ())
        rootTree = rootTree.getParent ();

    for (int i = docFiles.size (); --i >= 0; )
    {
        // copy md-file to current dir
        const String& docName (docFiles[i].getFileNameWithoutExtension ());
        const File& targetFile (thisDir.getChildFile (docName + ".md").getNonexistentSibling (true));
        docFiles[i].copyFileTo (targetFile);

        ValueTree docTree ("doc");
        docTree.setProperty ("name", targetFile.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("title", targetFile.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("author", rootTree.getProperty ("owner").toString (), nullptr);
        docTree.setProperty ("publish", true, nullptr);
        docTree.setProperty ("webName", targetFile.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("tplFile", rootTree.getProperty ("render").toString () + "/article.html", nullptr);
        docTree.setProperty ("js", String (), nullptr);

        tree.addChild (docTree, 0, nullptr);
    }

    if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
}

//=================================================================================================
void DocTreeViewItem::createNewDocument ()
{
    // can't create any doc under a doc!
    jassert (tree.getType ().toString () != "doc");

    AlertWindow dialog (TRANS ("Create a new document"), TRANS ("Please input the new doc's name."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", TRANS ("Untitled"));
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop ())
    {
        String docName (SwingUtilities::getValidFileName (dialog.getTextEditor ("name")->getText ()));

        if (docName.isEmpty ())
            docName = TRANS ("Untitled");

        // create this doc on disk
        const File& thisDoc (getFileOrDir (tree).getChildFile (docName + ".md")
                             .getNonexistentSibling (true));
        thisDoc.create ();
        thisDoc.appendText (String ("Recording and Sharing...")
                            + newLine + newLine +
                            SwingUtilities::getTimeStringWithSeparator
                            (SwingUtilities::getCurrentTimeString ()));

        // get the root for get some its properties
        ValueTree rootTree = tree;

        while (rootTree.getParent ().isValid ())
            rootTree = rootTree.getParent ();

        // valueTree of this doc
        ValueTree docTree ("doc");
        docTree.setProperty ("name", thisDoc.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("title", thisDoc.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("author", rootTree.getProperty ("owner").toString (), nullptr);
        docTree.setProperty ("publish", true, nullptr);
        docTree.setProperty ("webName", docName, nullptr);
        docTree.setProperty ("tplFile", tree.getProperty ("render").toString () + "/article.html", nullptr);
        docTree.setProperty ("js", String (), nullptr);

        // must update this tree before show this new item
        tree.removeListener (this);
        tree.addChild (docTree, 0, nullptr);
        tree.addListener (this);

        // select the new item 
        setOpen (true);
        DocTreeViewItem* docItem = new DocTreeViewItem (docTree, treeContainer, sorter);
        addSubItemSorted (*sorter, docItem);
        docItem->setSelected (true, true);

        // save the data to project file
        if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
    }
}

//=================================================================================================
void DocTreeViewItem::createNewFolder ()
{
    // can't create any dir under a doc!
    jassert (tree.getType ().toString () != "doc");

    AlertWindow dialog (TRANS ("Create a new folder"), TRANS ("Please input the new folder's name."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", TRANS ("New Folder"));
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop ())
    {
        String dirName (SwingUtilities::getValidFileName (dialog.getTextEditor ("name")->getText ()));

        if (dirName.isEmpty ())
            dirName = TRANS ("New folder");

        // create this dir on disk
        File thisDir (getFileOrDir (tree).getChildFile (dirName));
        thisDir = thisDir.getNonexistentSibling (true);
        thisDir.createDirectory ();

        // update tree
        ValueTree rootTree = tree;

        while (rootTree.getParent ().isValid ())
            rootTree = rootTree.getParent ();

        ValueTree dirTree ("dir");
        dirTree.setProperty ("name", dirName, nullptr);
        dirTree.setProperty ("title", dirName + TRANS ("\'s description"), nullptr);
        dirTree.setProperty ("isMenu", false, nullptr);
        dirTree.setProperty ("render", rootTree.getProperty ("render").toString (), nullptr);
        dirTree.setProperty ("webName", dirName, nullptr);

        // must update this tree before show this new item
        tree.removeListener (this);
        tree.addChild (dirTree, 0, nullptr);
        tree.addListener (this);

        // select the new item 
        setOpen (true);
        DocTreeViewItem* dirItem = new DocTreeViewItem (dirTree, treeContainer, sorter);
        addSubItemSorted (*sorter, dirItem);
        dirItem->setSelected (true, true);

        // save the data to project file
        if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
    }
}

//=================================================================================================
void DocTreeViewItem::delSelected ()
{
    // get all selected items
    OwnedArray<ValueTree> selectedTrees;
    TreeView* treeView = getOwnerView ();

    for (int i = 0; i < treeView->getNumSelectedItems (); ++i)
    {
        const DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView->getSelectedItem (i));
        jassert (item != nullptr);

        selectedTrees.add (new ValueTree (item->tree));
    }

    if (!AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                       TRANS ("Do you really want to delete ") +
                                       String (selectedTrees.size()) +  
                                       TRANS (" selected item(s)? ") + newLine + 
                                       TRANS("This operation CANNOT be undone! ") + newLine + newLine +
                                       TRANS ("Tips: The deleted items could be found in OS's Recycle Bin. ")))
        return;

    // MUST get the root before remove the tree!
    ValueTree rootTree = tree;

    while (rootTree.getParent ().isValid ())
        rootTree = rootTree.getParent ();

    // delete one by one
    for (int i = selectedTrees.size(); --i >=0; )
    {
        ValueTree& v = *selectedTrees.getUnchecked (i);

        if (v.getParent().isValid())
        {
            getFileOrDir (v).moveToTrash();
            v.getParent().removeChild (v, nullptr);
        }        
    }

    // save the data to project file
    if (!SwingUtilities::writeValueTreeToFile (rootTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
}

//=================================================================================================
void DocTreeViewItem::refreshDisplay()
{
    clearSubItems ();

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        DocTreeViewItem* itm = new DocTreeViewItem (tree.getChild (i), treeContainer, sorter);
        addSubItemSorted (*sorter, itm);
    }
}

//=================================================================================================
void DocTreeViewItem::valueTreePropertyChanged (ValueTree&, const Identifier&)
{
    repaintItem();
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildAdded (ValueTree& parentTree, ValueTree&)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildOrderChanged (ValueTree& parentTree, int, int)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::treeChildrenChanged (const ValueTree& parentTree)
{
    if (parentTree == tree)
    {
        refreshDisplay();
        treeHasChanged();
        setOpen (true);
    }
}

//=================================================================================================
const bool DocTreeViewItem::exportDocsAsMd (DocTreeViewItem* item,
                                      const ValueTree& tree,
                                      const File& fileAppendTo)
{
    item->setOpen (true);

    if (item->getNumSubItems() > 0)
    {
        jassert (item->getNumSubItems () == tree.getNumChildren ());

        for (int i = 0; i < item->getNumSubItems (); ++i)
        {
            DocTreeViewItem* currentItem = dynamic_cast<DocTreeViewItem*> (item->getSubItem (i));
            jassert (currentItem != nullptr);
            
            // recursive traversal
            if (!exportDocsAsMd (currentItem, tree.getChild (i), fileAppendTo))
                return false;
        }
    }
    else
    {
        const File& currentFile (getFileOrDir (tree));

        if (currentFile.existsAsFile() && currentFile.getSize() > 0)
            return fileAppendTo.appendText (currentFile.loadFileAsString().trimEnd() + newLine + newLine);

    }    

    return true;
}

//=================================================================================================
DocTreeViewItem* DocTreeViewItem::getRootItem (DocTreeViewItem* subItem)
{
    DocTreeViewItem* item = subItem;

    while (item->getParentItem() != nullptr)
        item = dynamic_cast<DocTreeViewItem*>(item->getParentItem());

    return item;
}

//=================================================================================================
var DocTreeViewItem::getDragSourceDescription ()
{
    if (tree.getType ().toString () != "wdtpProject" && getFileOrDir (tree).exists ())
        return "dragFileOrDir";

    return String ();
}

//=================================================================================================
bool DocTreeViewItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& details)
{
    if (tree.getType ().toString () != "doc")
        return details.description == "dragFileOrDir";

    return false;
}

//=================================================================================================
void DocTreeViewItem::itemDropped (const DragAndDropTarget::SourceDetails& details, 
                                   int /*insertIndex*/)
{
    if (details.description != "dragFileOrDir")
        return;

    jassert (getFileOrDir (tree).isDirectory());

    OwnedArray<ValueTree> selectedTrees;
    TreeView* treeView = getOwnerView();

    for (int i = 0; i < treeView->getNumSelectedItems(); ++i)
    {
        const DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView->getSelectedItem (i));
        jassert (item != nullptr);
        
        selectedTrees.add (new ValueTree (item->tree));
    }

    moveItems (selectedTrees, tree);
}

//=================================================================================================
void DocTreeViewItem::moveItems (const OwnedArray<ValueTree>& items, ValueTree thisTree)
{
    if (items.size () < 1)
        return;

#define CHOICE_BOX AlertWindow::showYesNoCancelBox

    for (int i = items.size (); --i >= 0;)
    {
        ValueTree& v = *items.getUnchecked (i);

        if (v.getParent().isValid() && thisTree != v && !thisTree.isAChildOf (v))
        {
            const File& thisFile (getFileOrDir (v));
            File targetFile (getFileOrDir (thisTree).getChildFile (thisFile.getFileName()));

            // prevent same-location move. but this will also prevent sort by mannul-mode
            if (thisFile.exists() && targetFile.exists() && thisFile == targetFile)
                continue;

            if (targetFile.exists())
            {
                const int choice = CHOICE_BOX (AlertWindow::QuestionIcon, TRANS ("Message"),
                                               "\"" + targetFile.getFullPathName() + "\"\n" +
                                               TRANS ("already exists. So what do you want? "),
                                               TRANS ("Keep Both"), TRANS ("Overwrite"));

                if (choice == 0)
                    continue;
                else if (choice == 1)
                    targetFile = targetFile.getNonexistentSibling (true);
            }

            if (thisFile.moveFileTo (targetFile))
            {
                v.setProperty ("name", targetFile.getFileNameWithoutExtension (), nullptr);
                v.getParent().removeChild (v, nullptr);
                thisTree.addChild (v, 0, nullptr);
            }
            else
            {
                SHOW_MESSAGE ("\"" + thisFile.getFullPathName() + "\"\n" + 
                              TRANS("move failed. "));
            }
        }
    }

#undef CHOICE_BOX

    // save the data to project file
    ValueTree rootTree = thisTree;

    while (rootTree.getParent().isValid())
        rootTree = rootTree.getParent();

    if (!SwingUtilities::writeValueTreeToFile (rootTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));
}

//=================================================================================================
void DocTreeViewItem::paintHorizontalConnectingLine (Graphics& g, const Line<float>& line)
{
    g.setColour (Colours::skyblue);
    g.drawLine (line);
}

//=================================================================================================
void DocTreeViewItem::paintVerticalConnectingLine (Graphics& g, const Line<float>& line)
{
    g.setColour (Colours::skyblue);
    g.drawLine (line);
}

//=================================================================================================
void DocTreeViewItem::valueChanged (Value& /*value*/)
{
    getRootItem (this)->refreshDisplay();
}

//=================================================================================================
String DocTreeViewItem::getTooltip ()
{
    // full path of file name
    if (sorter->getTooltipToShow() == 0)  
        return getFileOrDir (tree).getFullPathName();

    // title or intro
    else if (sorter->getTooltipToShow() == 1)  
        return tree.getProperty ("title").toString();
    
    // 2 for webpage name
    else if (sorter->getTooltipToShow() == 2)
        return tree.getProperty ("webName").toString ();

    return String();
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



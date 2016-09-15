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
    fileTree.getViewport ()->setScrollBarThickness (10);

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
    docTreeItem = new DocTreeViewItem (projectTree, this);
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
        projectTree = ValueTree::invalid;
        projectFile = File::nonexistent;
        projectloaded = false;

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
                                  FileTreeContainer* container) :
    tree (tree_), 
    treeContainer (container)
{
    jassert (treeContainer != nullptr);

    // highlight for the whole line
    setDrawsInLeftMargin (true); 
    setLinesDrawnForSubItems (true);
    tree.addListener (this);
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
        refreshSubItems ();
    else
        clearSubItems ();
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
    const String& itemName (tree.getProperty ("name").toString ());
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
        sortMenu.addItem (100, TRANS ("File Name"), true);
        sortMenu.addItem (101, TRANS ("Title"), true);
        sortMenu.addItem (102, TRANS ("Web Name"), true);
        sortMenu.addItem (103, TRANS ("File Size"), true);
        sortMenu.addItem (104, TRANS ("Create Time"), true);
        sortMenu.addItem (105, TRANS ("Modified Time"), true);
        sortMenu.addSeparator ();
        sortMenu.addItem (5, TRANS ("Ascending Order"), true, isAscendingOrder);

        m.addSubMenu (TRANS ("Sort by"), sortMenu, exist && !isDoc);

        PopupMenu showedAsMenu;
        showedAsMenu.addItem (200, TRANS ("File Name"), true);
        showedAsMenu.addItem (201, TRANS ("Title/Intro"), true);
        showedAsMenu.addItem (202, TRANS ("Web Name"), true);

        m.addSubMenu (TRANS ("Showed as"), showedAsMenu, exist && !isRoot);

        PopupMenu tooltipAsMenu;
        tooltipAsMenu.addItem (300, TRANS ("File Name"), true);
        tooltipAsMenu.addItem (301, TRANS ("Title/Intro"), true);
        tooltipAsMenu.addItem (302, TRANS ("Web Name"), true);

        m.addSubMenu (TRANS ("Tooltip for"), tooltipAsMenu, exist && !isRoot);
        m.addSeparator ();

        m.addItem (10, TRANS ("Rename..."), !isRoot && onlyOneSelected);
        m.addItem (12, TRANS ("Delete..."), !isRoot);
        m.addSeparator ();

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
    else if (index == 15)
        getFileOrDir (tree).startAsProcess();
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
            SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
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
        SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
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
        DocTreeViewItem* docItem = new DocTreeViewItem (docTree, treeContainer);
        addSubItem (docItem, 0);
        docItem->setSelected (true, true);

        // save the data to project file
        if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
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
        dirTree.setProperty ("desc", TRANS ("Description of ") + dirName, nullptr);
        dirTree.setProperty ("isMenu", false, nullptr);
        dirTree.setProperty ("render", rootTree.getProperty ("render").toString (), nullptr);
        dirTree.setProperty ("webName", dirName, nullptr);

        // must update this tree before show this new item
        tree.removeListener (this);
        tree.addChild (dirTree, 0, nullptr);
        tree.addListener (this);

        // select the new item 
        setOpen (true);
        DocTreeViewItem* dirItem = new DocTreeViewItem (dirTree, treeContainer);
        addSubItem (dirItem, 0);
        dirItem->setSelected (true, true);

        // save the data to project file
        if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
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
                                       TRANS (" selected item(s)? ") + newLine + newLine +
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
        SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
}

//=================================================================================================
void DocTreeViewItem::refreshSubItems ()
{
    clearSubItems ();

    for (int i = 0; i < tree.getNumChildren (); ++i)
    {
        // TODO: sort...
        addSubItem (new DocTreeViewItem (tree.getChild (i), treeContainer));
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
        refreshSubItems();
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
        SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
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


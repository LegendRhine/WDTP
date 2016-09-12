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
    MainWindow* mainWindow = static_cast<MainWindow*>(getTopLevelComponent ());
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
        MainWindow* mainWindow = static_cast<MainWindow*>(getTopLevelComponent ());
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
}

//=================================================================================================
bool DocTreeViewItem::mightContainSubItems ()
{
    return tree.getNumChildren() > 0;
}

//=================================================================================================
String DocTreeViewItem::getUniqueName () const
{
    return tree.getProperty ("name").toString ();
}

//=================================================================================================
void DocTreeViewItem::itemOpennessChanged (bool isNowOpen)
{
    clearSubItems ();

    if (isNowOpen && getNumSubItems () == 0)
    {
        for (int i = 0; i < tree.getNumChildren (); ++i)
            addSubItem (new DocTreeViewItem (tree.getChild (i), treeContainer));
    }
}

//=================================================================================================
void DocTreeViewItem::paintItem (Graphics& g, int width, int height)
{
    g.setFont (SwingUtilities::getFontSize () - 2.f);
    int leftGap = 4;
    const String itemPath (tree.getProperty ("name").toString ());
    Colour c (0xff303030);

    // draw doc item
    if (tree.getType ().toString () == "doc") 
    {
        leftGap = 24;
        g.drawImageAt (ImageCache::getFromMemory (BinaryData::doc_png, BinaryData::doc_pngSize), 
                       4, getItemHeight () / 2 - 8);        
    }
    // draw dir item
    else if (tree.getType ().toString () == "dir") 
    {
        leftGap = 24;
        Image icon;

        if (isOpen ())
            icon = ImageCache::getFromMemory (BinaryData::diro_png, BinaryData::diro_pngSize);
        else
            icon = ImageCache::getFromMemory (BinaryData::dir_png, BinaryData::dir_pngSize);

        g.drawImageAt (icon, 4, getItemHeight () / 2 - 8);
    }  

    if (!getFileOfThisItem().exists())
        c = Colours::red;

    g.setColour (c);
    g.drawText (itemPath.fromLastOccurrenceOf ("/", false, true),
                leftGap, 0, width - 4, height, Justification::centredLeft, true);
}

//=================================================================================================
void DocTreeViewItem::itemClicked (const MouseEvent& e)
{    
    EditAndPreview* editArea = treeContainer->getEditAndPreview ();

    // doc
    if (tree.getType ().toString () == "doc")
    {
        if (getFileOfThisItem().existsAsFile())
        {
            if (0 == systemFile->getIntValue ("clickForEdit"))
                editArea->editDoc (getFileOfThisItem ());
            else
                editArea->previewDoc (getFileOfThisItem ());

            editArea->setDocProperties (tree);
        }
        else
        {
            editArea->whenFileOrDirNonexists ();
        }
    }
    // dir
    else if (tree.getType ().toString () == "dir")
    {
        if (getFileOfThisItem().isDirectory())
            editArea->setDirProperties (tree);
        else
            editArea->whenFileOrDirNonexists ();
    }
    else // root
    {
        editArea->setProjectProperties ();
    }

    // right click
    if (getFileOfThisItem().exists() && e.mods.isPopupMenu())
    {
        PopupMenu m;
        m.addItem (1, TRANS ("New Folder..."), tree.getType ().toString () != "doc");
        m.addItem (2, TRANS ("New Document..."), tree.getType ().toString () != "doc");
        m.addSeparator ();

        m.addItem (3, TRANS ("Import..."), tree.getType ().toString () != "doc");
        m.addItem (4, TRANS ("Export...")); // export as a single markdown file
        m.addSeparator ();

        PopupMenu sortMenu;
        sortMenu.addItem (5, TRANS ("File Name"), true);
        sortMenu.addItem (6, TRANS ("Title"), true);
        sortMenu.addItem (7, TRANS ("Web Name"), true);
        sortMenu.addItem (8, TRANS ("Words Number"), true);
        sortMenu.addItem (9, TRANS ("Create Time"), true);
        sortMenu.addItem (10, TRANS ("Modified Time"), true);
        sortMenu.addSeparator ();
        sortMenu.addItem (11, TRANS ("Ascending Order"), true, isAscendingOrder);

        m.addSubMenu (TRANS ("Sort by"), sortMenu, tree.getType ().toString () != "doc");
        m.addSeparator ();

        m.addItem (12, TRANS ("Rename..."), tree.getType ().toString () != "wdtpProject");
        m.addItem (13, TRANS ("Move To..."), tree.getType ().toString () != "wdtpProject");
        m.addItem (14, TRANS ("Copy To..."), tree.getType ().toString () == "doc");
        m.addItem (15, TRANS ("Remove..."), tree.getType ().toString () != "wdtpProject");
        m.addSeparator ();

        m.addItem (16, TRANS ("Open In External Editor..."), tree.getType ().toString () == "doc");

        menuPerform (m.show());
    }
}

//=================================================================================================
const File DocTreeViewItem::getFileOfThisItem () const
{
    const File& root (treeContainer->projectFile.getSiblingFile ("docs"));

    if (tree.getType ().toString () == "dir")
        return root.getChildFile (tree.getProperty ("name").toString ());

    else if (tree.getType ().toString () == "doc")
        return root.getChildFile (tree.getProperty ("name").toString () + ".md");

    else if (tree.getType ().toString () == "wdtpProject")
        return treeContainer->projectFile;

    return File::nonexistent;
}

//=================================================================================================
void DocTreeViewItem::menuPerform (const int index)
{
    if (index == 1)  // create a new folder
    {
        jassert (tree.getType ().toString () != "doc");

        AlertWindow dialog (TRANS ("Create a new folder"), TRANS ("Please input the new folder's name."),
                           AlertWindow::InfoIcon);
        
        dialog.addTextEditor ("name", TRANS ("New Folder"));
        dialog.addButton (TRANS ("OK"), 0, KeyPress(KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));
        
        if (0 == dialog.runModalLoop ())
        {
            String dirName (dialog.getTextEditor ("name")->getText ().trim ()
                            .replaceCharacter ('.', '-').replaceCharacter ('?', '-')
                            .replaceCharacter ('*', '-').replaceCharacter ('/', '-')
                            .replaceCharacter ('~', '-').replaceCharacter (':', '-')
                            .replaceCharacter ('|', '-').replaceCharacter ('<', '-')
                            .replaceCharacter ('>', '-').replaceCharacter ('\"', '-')
                            .replaceCharacter ('\\', '-').replaceCharacter ('\'', '-'));
            
            if (dirName.isEmpty ())
                dirName = TRANS ("New folder");

            // create this dir on disk
            File thisDir (getFileOfThisItem ().getChildFile (dirName));
            thisDir = thisDir.getNonexistentSibling (true);
            thisDir.createDirectory ();

            // update view
            if (tree.getType ().toString () == "dir")
                dirName = tree.getProperty ("name").toString () + "/" + thisDir.getFileName();
            else
                dirName = thisDir.getFileName ();

            ValueTree dirTree ("dir");
            dirTree.setProperty ("name", dirName, nullptr);
            dirTree.setProperty ("desc", String(), nullptr);
            dirTree.setProperty ("isMenu", false, nullptr);
            dirTree.setProperty ("webName", dirName.fromLastOccurrenceOf ("/", false, true), nullptr);
                          
            tree.addChild (dirTree, 0, nullptr);
            itemOpennessChanged (true);

            // save the data to project file
            ValueTree rootTree = tree;

            while (rootTree.getParent ().isValid ())
                rootTree = rootTree.getParent ();

            if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
                SHOW_MESSAGE (TRANS ("Something wrong during this operation."));
        }
    }
}


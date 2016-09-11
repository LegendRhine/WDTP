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

        const File itemFile (treeContainer->projectFile.getSiblingFile ("docs").getChildFile (itemPath + ".md"));

        if (!itemFile.existsAsFile ())
            c = Colours::red;
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

        const File itemDir (treeContainer->projectFile.getSiblingFile ("docs").getChildFile (itemPath));

        if (! (itemDir.exists () && itemDir.isDirectory ()))
            c = Colours::red;
    }  

    g.setColour (c);
    g.drawText (itemPath.fromLastOccurrenceOf ("/", false, true),
                leftGap, 0, width - 4, height, Justification::centredLeft, true);
}

//=================================================================================================
void DocTreeViewItem::itemClicked (const MouseEvent& e)
{
    // left click
    if (e.mods.isLeftButtonDown())
    {
        const String itemPath (tree.getProperty ("name").toString ());
        EditAndPreview* editArea = treeContainer->getEditAndPreview ();

        // doc
        if (tree.getType ().toString () == "doc")
        {
            const File itemFile (treeContainer->projectFile.getSiblingFile ("docs").getChildFile (itemPath + ".md"));

            if (itemFile.existsAsFile ())
            {
                if (0 == systemFile->getIntValue ("clickForEdit"))
                    editArea->editDoc (itemFile);
                else
                    editArea->previewDoc (itemFile);

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
            editArea->setDirProperties (tree);
        }
        else // root
        {
            editArea->setProjectProperties ();
        }
    }
    // right click
    else if (e.mods.isPopupMenu())
    {
        if (tree.getType ().toString () == "doc")
        {

        }
        else if (tree.getType ().toString () == "dir")
        {

        }
        else // root
        {

        }
    }
}


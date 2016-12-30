/*
  ==============================================================================

    DocTreeViewItem.cpp
    Created: 16 Sep 2016 5:20:49am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

extern PropertiesFile* systemFile;

//=================================================================================================
DocTreeViewItem::DocTreeViewItem (const ValueTree& tree_,
                                  FileTreeContainer* container,
                                  ItemSorter* itemSorter) :
    tree (tree_),
    treeContainer (container),
    sorter (itemSorter)
{
    jassert (treeContainer != nullptr);

    // highlight for the whole line
    //setDrawsInLeftMargin (true); 
    setLinesDrawnForSubItems (true);
    tree.addListener (this);
}

//=================================================================================================
DocTreeViewItem::~DocTreeViewItem ()
{
    tree.removeListener (this);
    treeContainer = nullptr;
}

//=================================================================================================
bool DocTreeViewItem::mightContainSubItems ()
{
    return tree.getNumChildren () > 0;
}

//=================================================================================================
String DocTreeViewItem::getUniqueName () const
{
    return tree.getProperty("name").toString();
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

    if (!getFileOrDir (tree).exists ())
        c = Colours::red;

    g.setColour (c);

    String itemName;
    jassert (sorter != nullptr);

    if (sorter->getShowWhat () == 0)  // file name
        itemName = tree.getProperty ("name").toString ();

    else if (sorter->getShowWhat () == 1) // title or intro
        itemName = tree.getProperty ("title").toString ();

    g.drawText (itemName, leftGap, 0, width - 4, height, Justification::centredLeft, true);
}

//=================================================================================================
const File DocTreeViewItem::getFileOrDir (const ValueTree& tree)
{
    if (!tree.isValid())
        return File::nonexistent;
    
    const File& root (FileTreeContainer::projectFile.getSiblingFile ("docs"));

    if (tree.getType ().toString () == "wdtpProject")
    {
        return root;
    }
    else
    {
        String path = tree.getProperty ("name").toString ();
        ValueTree t (tree);

        while (t.getParent ().isValid () && t.getParent ().getType ().toString () != "wdtpProject")
        {
            t = t.getParent ();
            path = t.getProperty ("name").toString () + "/" + path;
        }

        if (tree.getType ().toString () == "doc")
            path += ".md";

        return root.getChildFile (path);
    }
}

//=================================================================================================
// mouse click
void DocTreeViewItem::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        EditAndPreview* editArea = treeContainer->getEditAndPreview ();
        MainContentComponent* mainComp = editArea->findParentComponentOfClass<MainContentComponent> ();
        TopToolBar* toolbar = mainComp->getToolbar ();

        // edit or preview
        if (toolbar != nullptr && toolbar->getStateOfViewButton ())
            editArea->previewDoc (tree);
        else
            editArea->editNewDoc (tree);

        // set properties on the right side
        if (tree.getType ().toString () == "doc")
        {
            editArea->setDocProperties (tree);
        }
        else if (tree.getType ().toString () == "dir")
        {
            editArea->setDirProperties (tree);
        }
        else // root
        {
            editArea->setProjectProperties (tree);
        }

        treeContainer->setIdentityOfLastSelectedItem (getItemIdentifierString ());
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
    const bool onlyOneSelected = getOwnerView ()->getNumSelectedItems () == 1;

    jassert (sorter != nullptr);

    // right click
    if (e.mods.isPopupMenu ())
    {
        PopupMenu m;
        m.addItem (1, TRANS ("New Folder..."), exist && !isDoc && onlyOneSelected);
        m.addItem (2, TRANS ("New Document..."), exist && !isDoc && onlyOneSelected);
        m.addSeparator ();

        m.addItem (3, TRANS ("Import..."), exist && !isDoc && onlyOneSelected);
        m.addItem (4, TRANS ("Export..."), exist && onlyOneSelected);
        m.addSeparator ();

        PopupMenu sortMenu;
        sortMenu.addItem (100, TRANS ("File Name"), true, sorter->getOrder () == 0);
        sortMenu.addItem (101, TRANS ("Title / Intro"), true, sorter->getOrder () == 1);
        sortMenu.addItem (103, TRANS ("File Size"), true, sorter->getOrder () == 3);
        sortMenu.addItem (104, TRANS ("Create Time"), true, sorter->getOrder () == 4);
        sortMenu.addItem (105, TRANS ("Modified Time"), true, sorter->getOrder () == 5);
        sortMenu.addSeparator ();
        sortMenu.addItem (106, TRANS ("Ascending Order"), true, sorter->getAscending () == 0);
        sortMenu.addItem (107, TRANS ("Folder First"), true, sorter->getWhichFirst () == 0);

        m.addSubMenu (TRANS ("Sort by"), sortMenu);

        PopupMenu showedAsMenu;
        showedAsMenu.addItem (200, TRANS ("File Name"), true, sorter->getShowWhat () == 0);
        showedAsMenu.addItem (201, TRANS ("Title / Intro"), true, sorter->getShowWhat () == 1);

        m.addSubMenu (TRANS ("Showed as"), showedAsMenu);

        PopupMenu tooltipAsMenu;
        tooltipAsMenu.addItem (300, TRANS ("File Path"), true, sorter->getTooltipToShow () == 0);
        tooltipAsMenu.addItem (301, TRANS ("Title / Intro"), true, sorter->getTooltipToShow () == 1);

        m.addSubMenu (TRANS ("Tooltip for"), tooltipAsMenu);
        m.addSeparator ();

        m.addItem (10, TRANS ("Rename..."), !isRoot && onlyOneSelected);
        m.addItem (12, TRANS ("Delete..."), !isRoot);
        m.addSeparator ();

        m.addItem (14, TRANS ("Open in Explorer / Finder..."), exist && onlyOneSelected);
        m.addItem (15, TRANS ("Open in External Editor..."), exist && isDoc && onlyOneSelected);

        menuPerform (m.show ());
    }
}

//=================================================================================================
void DocTreeViewItem::menuPerform (const int index)
{
    jassert (sorter != nullptr);

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
        deleteSelected ();

    // open in external app..
    else if (index == 14)
        getFileOrDir (tree).revealToUser ();
    else if (index == 15)
        getFileOrDir (tree).startAsProcess ();

    // sort and show what...
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
        const String inputStr (dialog.getTextEditor ("name")->getText ());
        String newName (SwingUtilities::getValidFileName (inputStr));

        if (newName == tree.getProperty ("name").toString ())
            return;

        if (newName.isEmpty ())
            newName = TRANS ("Untitled");

        File newFile (getFileOrDir (tree).getSiblingFile (newName + (fileOrDir.isDirectory () ? String ()
                                                                     : ".md")));
        newFile = newFile.getNonexistentSibling (true);

        if (fileOrDir.moveFileTo (newFile))
        {
            // save the project file
            tree.setProperty ("name", newFile.getFileNameWithoutExtension (), nullptr);

            ValueTree rootTree = tree;

            while (rootTree.getParent ().isValid ())
                rootTree = rootTree.getParent ();

            if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
                SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't rename this item. "));
        }
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

    File mdFile (fc.getResult ());
    mdFile = mdFile.getSiblingFile (SwingUtilities::getValidFileName (mdFile.getFileNameWithoutExtension ()));

    if (!mdFile.hasFileExtension ("md"))
        mdFile = mdFile.withFileExtension ("md");

    // overwrite or not if it has been there
    if (mdFile.existsAsFile () &&
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                       TRANS ("This file already exists, want to overwrite it?")))
    {
        return;
    }

    if (!mdFile.deleteFile ())
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

    FileChooser fc (TRANS ("Import document(s)..."), File::nonexistent, String(), false);

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
        
        if (docFiles[i].copyFileTo (targetFile))
        {
            ValueTree docTree ("doc");
            docTree.setProperty ("name", targetFile.getFileNameWithoutExtension (), nullptr);
            docTree.setProperty ("title", targetFile.getFileNameWithoutExtension (), nullptr);
            docTree.setProperty ("tplFile", rootTree.getProperty ("render").toString () + "/article.html", nullptr);
            docTree.setProperty ("js", String (), nullptr);

            tree.addChild (docTree, 0, nullptr);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't import this document: ") + newLine +
                          docFiles[i].getFullPathName());
        }
    }

    if (!SwingUtilities::writeValueTreeToFile (rootTree, treeContainer->projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
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
        String docName (SwingUtilities::getValidFileName (dialog.getTextEditor ("name")->getText()));

        if (docName.isEmpty ())
            docName = TRANS ("Untitled");

        // create this doc on disk
        const File& thisDoc (getFileOrDir (tree).getChildFile (docName + ".md")
                             .getNonexistentSibling (true));
        thisDoc.create();
        thisDoc.appendText (TRANS ("# Title of this article")
                            + newLine + newLine);

        // get the root for get some its properties
        ValueTree rootTree = tree;

        while (rootTree.getParent ().isValid ())
            rootTree = rootTree.getParent ();

        // valueTree of this doc
        ValueTree docTree ("doc");
        docTree.setProperty ("name", thisDoc.getFileNameWithoutExtension (), nullptr);
        docTree.setProperty ("title", "Title of this article", nullptr);
        docTree.setProperty ("keywords", String (), nullptr);
        docTree.setProperty ("tplFile", tree.getProperty ("render").toString() + "/article.html", nullptr);
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
            SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
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
        dirTree.setProperty ("name", thisDir.getFileNameWithoutExtension(), nullptr);
        dirTree.setProperty ("title", thisDir.getFileNameWithoutExtension (), nullptr);
        dirTree.setProperty ("isMenu", false, nullptr);
        dirTree.setProperty ("render", rootTree.getProperty ("render").toString (), nullptr);

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
            SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
    }
}

//=================================================================================================
void DocTreeViewItem::deleteSelected ()
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
                                       String (selectedTrees.size ()) +
                                       TRANS (" selected item(s)? ") + newLine +
                                       TRANS ("This operation CANNOT be undone! ") + newLine + newLine +
                                       TRANS ("Tips: The deleted items could be found in OS's Recycle Bin. ")))
        return;

    // MUST get the root before remove the tree!
    ValueTree rootTree = tree;

    while (rootTree.getParent().isValid())
        rootTree = rootTree.getParent();

    // here get the EditAndPreview, it must before the delete, otherwise it'll be wild-pointer!
    EditAndPreview* editor = treeContainer->getEditAndPreview();

    // delete one by one
    for (int i = selectedTrees.size (); --i >= 0; )
    {
        ValueTree& v = *selectedTrees.getUnchecked (i);

        if (v.getParent ().isValid ())
        {
            getFileOrDir (v).moveToTrash ();
            v.getParent ().removeChild (v, nullptr);
        }
    }

    // save the data to project file
    if (!SwingUtilities::writeValueTreeToFile (rootTree, FileTreeContainer::projectFile))
    {
        SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
    }
    else
    {
        jassert (editor != nullptr);
        editor->editNewDoc (ValueTree::invalid);
    }
}

//=================================================================================================
void DocTreeViewItem::refreshDisplay ()
{
    clearSubItems();

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        DocTreeViewItem* itm = new DocTreeViewItem (tree.getChild (i), treeContainer, sorter);
        addSubItemSorted (*sorter, itm);
    }
}

//=================================================================================================
void DocTreeViewItem::valueTreePropertyChanged (ValueTree&, const Identifier&)
{
    repaintItem ();
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildAdded (ValueTree& parentTree, ValueTree& /*childTree*/)
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
        refreshDisplay ();
        treeHasChanged ();
        setOpen (true);
    }
}

//=================================================================================================
const bool DocTreeViewItem::exportDocsAsMd (DocTreeViewItem* item,
                                            const ValueTree& tree,
                                            const File& fileAppendTo)
{
    item->setOpen (true);

    if (item->getNumSubItems () > 0)
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

        if (currentFile.existsAsFile () && currentFile.getSize () > 0)
            return fileAppendTo.appendText (currentFile.loadFileAsString ().trimEnd () + newLine + newLine);

    }

    return true;
}

//=================================================================================================
DocTreeViewItem* DocTreeViewItem::getRootItem (DocTreeViewItem* subItem)
{
    DocTreeViewItem* item = subItem;

    while (item->getParentItem () != nullptr)
        item = dynamic_cast<DocTreeViewItem*>(item->getParentItem ());

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
void DocTreeViewItem::moveItems (const OwnedArray<ValueTree>& items, 
                                 ValueTree thisTree)
{
    if (items.size () < 1)
        return;

#define CHOICE_BOX AlertWindow::showYesNoCancelBox

    for (int i = items.size (); --i >= 0;)
    {
        ValueTree& v = *items.getUnchecked (i);

        if (v.getParent ().isValid () && thisTree != v && !thisTree.isAChildOf (v))
        {
            const File& thisFile (getFileOrDir (v));
            File targetFile (getFileOrDir (thisTree).getChildFile (thisFile.getFileName ()));

            // prevent same-location move. but this will also prevent sort by mannul-mode
            if (thisFile.exists () && targetFile.exists () && thisFile == targetFile)
                continue;

            if (targetFile.exists ())
            {
                const int choice = CHOICE_BOX (AlertWindow::QuestionIcon, TRANS ("Message"),
                                               "\"" + targetFile.getFullPathName () + "\"\n" +
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
                v.getParent ().removeChild (v, nullptr);
                thisTree.addChild (v, 0, nullptr);
            }
            else
            {
                SHOW_MESSAGE ("\"" + thisFile.getFullPathName () + "\"\n" +
                              TRANS ("move failed. "));
            }
        }
    }

#undef CHOICE_BOX

    // save the data to project file
    ValueTree rootTree = thisTree;

    while (rootTree.getParent ().isValid ())
        rootTree = rootTree.getParent ();

    if (!SwingUtilities::writeValueTreeToFile (rootTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));
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
    {
        if (tree.getType ().toString () == "wdtpProject")
        {
            return tree.getProperty ("domain").toString();
        }
        else
        {
            const String docPath (getFileOrDir(tree).getFullPathName());
            const String& htmlPath (docPath.replace ("docs","site"));
            //DBGX (htmlPath.replace (".md", ".html"));

            return htmlPath.replace (".md", ".html");
        }
    }

    return String ();
}


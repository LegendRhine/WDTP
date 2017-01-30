﻿/*
  ==============================================================================

    TopToolBar.cpp
    Created: 4 Sep 2016 12:25:18am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;
extern ApplicationCommandManager* cmdManager;

const float imageTrans = 1.f;

//==============================================================================
TopToolBar::TopToolBar (FileTreeContainer* f, EditAndPreview* e) :
    fileTreeContainer (f),
    editAndPreview (e)
{
    jassert (fileTreeContainer != nullptr);
    jassert (editAndPreview != nullptr);

    // 2 search textEditors..
    addAndMakeVisible (searchInProject = new TextEditor ());
    searchInProject->addListener (this);

    searchInProject->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInProject->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInProject->setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha(0.6f));
    searchInProject->setScrollBarThickness (10);
    searchInProject->setFont (SwingUtilities::getFontSize () - 2.f);
    searchInProject->setTextToShowWhenEmpty (TRANS ("Seach in this project..."), Colour (0xff303030).withAlpha(0.6f));
    //searchEditor->setTabKeyUsedAsCharacter(true);

    addAndMakeVisible (searchInDoc = new TextEditor ());
    searchInDoc->addListener (this);

    searchInDoc->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInDoc->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInDoc->setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha(0.6f));
    searchInDoc->setScrollBarThickness (10);
    searchInDoc->setFont (SwingUtilities::getFontSize () - 2.f);
    searchInDoc->setTextToShowWhenEmpty (TRANS ("Seach in current document..."), Colour (0xff303030).withAlpha(0.6f));
    //searchEditor->setTabKeyUsedAsCharacter(true);

    // image buttons...
    for (int i = totalBts; --i >= 0; )
    {
        MyImageButton* bt = new MyImageButton ();
        bt->setSize (20, 20);
        bt->addListener (this);

        bts.add (bt);
        addAndMakeVisible (bt);
    }

    bts[prevAll]->setTooltip (TRANS ("Find Previous"));
    bts[prevAll]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::prev_png,
                                                        BinaryData::prev_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    bts[nextAll]->setTooltip (TRANS ("Find Next"));
    bts[nextAll]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::next_png,
                                                        BinaryData::next_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    bts[prevPjt]->setTooltip (TRANS ("Find Previous"));
    bts[prevPjt]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::prev_png,
                                                        BinaryData::prev_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    bts[nextPjt]->setTooltip (TRANS ("Find Next"));
    bts[nextPjt]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::next_png,
                                                        BinaryData::next_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    String cmdStr("cmd");

#if JUCE_WINDOWS
    cmdStr = "Ctrl";
#endif

    bts[view]->setTooltip (TRANS ("Switch Preview / Edit Mode") + "  (" + cmdStr + " + TAB)");
    bts[view]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::view_png,
                                                     BinaryData::view_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.0f, Colour (0x00),
                          Image::null, 1.0f, Colours::darkcyan);
    bts[view]->setToggleState (true, dontSendNotification);

    bts[system]->setTooltip (TRANS ("Popup System Menu"));
    bts[system]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::system_png,
                                                       BinaryData::system_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colours::darkcyan,
                            Image::null, 1.000f, Colours::darkcyan);
    
    bts[width]->setTooltip (TRANS ("Switch Simply / Full Mode") + "  (" + cmdStr + " + ~)");
    bts[width]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::width_png,
                                                     BinaryData::width_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.0f, Colour (0x00),
                          Image::null, 1.0f, Colours::darkcyan);
    bts[width]->setToggleState (true, dontSendNotification);
}

//=======================================================================
TopToolBar::~TopToolBar ()
{
}

//=======================================================================
void TopToolBar::paint (Graphics& g)
{
    g.setColour (Colour::fromString (systemFile->getValue("uiTextColour")).withAlpha (0.6f));
    g.drawLine (1.0f, getHeight () - 0.5f, getWidth () - 2.0f, getHeight () - 0.5f, 0.6f);
    //g.drawVerticalLine (getWidth () / 2, 0.5f, getHeight () - 1.0f);
}

//=======================================================================
void TopToolBar::resized ()
{
    // search textEditors and find buttons
    if (getWidth() >= 800)
    {
        bts[prevAll]->setVisible(true);
        searchInProject->setVisible(true);
        bts[nextAll]->setVisible(true);
        bts[nextPjt]->setVisible(true);
        searchInDoc->setVisible(true);
        bts[prevPjt]->setVisible(true);
        
        bts[prevAll]->setBounds (12, 14, 16, 16);
        searchInProject->setBounds (bts[prevAll]->getRight () + 10, 10, 200, 25);
        bts[nextAll]->setBounds (searchInProject->getRight () + 10, 14, 16, 16);
        
        bts[nextPjt]->setBounds (getWidth () - 24, 14, 16, 16);
        searchInDoc->setBounds (bts[nextPjt]->getX () - 230, 10, 220, 25);
        bts[prevPjt]->setBounds (searchInDoc->getX () - 25, 14, 16, 16);
    }
    else
    {
        bts[prevAll]->setVisible(false);
        searchInProject->setVisible(false);
        bts[nextAll]->setVisible(false);
        bts[nextPjt]->setVisible(false);
        searchInDoc->setVisible(false);
        bts[prevPjt]->setVisible(false);
    }

    // image buttons
    bts[system]->setTopLeftPosition(getWidth() / 2 - 9, 12);
    bts[view]->setTopRightPosition (bts[system]->getX() - 40, 12);
    bts[width]->setTopLeftPosition (bts[system]->getRight() + 40, 12);
}

//=================================================================================================
void TopToolBar::enableEditPreviewBt (const bool enableIt,
                                      const bool toggleState)
{
    bts[view]->setToggleState (toggleState, dontSendNotification);
    bts[view]->setVisible (enableIt);
}

//=========================================================================
void TopToolBar::textEditorReturnKeyPressed (TextEditor& te)
{
    if (&te == searchInProject)
        findInProject (true);

    else if (&te == searchInDoc)
        findInDoc (true);
}

//=========================================================================
void TopToolBar::textEditorEscapeKeyPressed (TextEditor& te)
{
    if (&te == searchInProject)
        searchInProject->setText(String(), false);

    else if (&te == searchInDoc)
        searchInDoc->setText (String(), false);
}

//=================================================================================================
void TopToolBar::findInProject (const bool next)
{
    const String& keyword (searchInProject->getText());

    if (keyword.isEmpty ())
        return;

    TreeView& treeView (fileTreeContainer->getTreeView ());
    treeView.setDefaultOpenness (true);

    // get start (selected) row-number
    int startIndex = 0;

    for (int i = startIndex; i < treeView.getNumRowsInTree (); ++i)
    {
        if (treeView.getItemOnRow (i)->isSelected ())
        {
            startIndex = i;
            break;
        }
    }

    // find and select
    for (int i = next ? startIndex + 1 : startIndex - 1; 
         next ? (i < treeView.getNumRowsInTree ()) : (i >= 0); 
         next ? ++i : --i)
    {
        DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView.getItemOnRow (i));

        if (item == nullptr)
            continue;

        const File& docFile (DocTreeViewItem::getMdFileOrDir (item->getTree()));
        const String& docContent (docFile.loadFileAsString());

        if (docContent.containsIgnoreCase (keyword))
        {
            item->setSelected (true, true);
            searchInDoc->setText (keyword, false);
            treeView.scrollToKeepItemVisible(item);

            editAndPreview->getEditor()->moveCaretToTop(false);
            findInDoc (true);

            return;
        }
    }

    LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
}

//=================================================================================================
void TopToolBar::findInDoc (const bool next)
{
    const String& keyword (searchInDoc->getText());

    if (keyword.isEmpty ())
        return;

    editAndPreview->switchMode(false);
    TextEditor* editor = editAndPreview->getEditor();
    const String& content = editor->getText();

    int startIndex = 0;
    int caretIndex = editor->getCaretPosition ();

    // find the start index of the keyword
    if (next)
        startIndex = content.indexOfIgnoreCase (caretIndex, keyword);
    else
        startIndex = content.substring (0, caretIndex - 1).lastIndexOfIgnoreCase (keyword);

    // select the keyword
    if (startIndex != -1)
    {
        Array<Range<int>> rangeArray;
        rangeArray.add(Range<int>(startIndex, startIndex + keyword.length()));

        editor->setTemporaryUnderlining(rangeArray);
        editor->setHighlightedRegion(rangeArray[0]);
    }
    else
    {
        LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    }
}

//=========================================================================
void TopToolBar::buttonClicked (Button* bt)
{
	if (bt == bts[view])
	{
		bts[view]->setToggleState (!bts[view]->getToggleState (), dontSendNotification);
		editAndPreview->startWork (editAndPreview->getCurrentTree ());
	}
	else if (bt == bts[width])
	{
		bts[width]->setToggleState (!bts[width]->getToggleState (), dontSendNotification);
		getParentComponent ()->setSize ((bts[width]->getToggleState () ? 1200 : 660), 780);
		getTopLevelComponent ()->setCentreRelative (0.5f, 0.53f);
	}
	else if (bt == bts[system])
		popupSystemMenu ();
	else if (bt == bts[prevAll])
		findInProject (false);
	else if (bt == bts[nextAll])
		findInProject (true);
	else if (bt == bts[prevPjt])
		findInDoc (false);
	else if (bt == bts[nextPjt])
		findInDoc (true);
}

//=================================================================================================
void TopToolBar::popupSystemMenu ()
{
    PopupMenu m;
    m.addItem(1, TRANS("New Project..."), true);
    m.addSeparator();

    m.addItem(3, TRANS("Open Project..."), true);

    // recent files
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
    PopupMenu recentFilesMenu;
    recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);

    m.addSubMenu (TRANS ("Open Rcent"), recentFilesMenu);
    m.addSeparator();    

    m.addItem(2, TRANS("Close Project"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    m.addCommandItem(cmdManager, 12);
    m.addCommandItem(cmdManager, 13);
    m.addSeparator();

    m.addItem(5, TRANS("Regenerate Whole Site..."), fileTreeContainer->hasLoadedProject());
	m.addItem (6, TRANS ("Cleanup Local Medias..."), fileTreeContainer->hasLoadedProject ());
    m.addSeparator ();

    m.addItem(7, TRANS("Export Current Templates"), fileTreeContainer->hasLoadedProject());
    m.addItem(8, TRANS("Import External Templates..."), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    PopupMenu lanMenu;
    lanMenu.addItem (30, "English", true, systemFile->getValue ("language") == "English");
    lanMenu.addItem (31, CharPointer_UTF8 ("\xe4\xb8\xad\xe6\x96\x87"), true, systemFile->getValue ("language") == "Chinese");
    m.addSubMenu (TRANS ("UI Language"), lanMenu);

    PopupMenu uiMenu;
    uiMenu.addItem(15, TRANS("Set UI Color..."));
    uiMenu.addItem(16, TRANS("Reset to Default"));
    m.addSubMenu(TRANS("UI Color"), uiMenu);
    m.addSeparator();

    m.addItem (18, TRANS ("Getting Started..."), true);
    m.addItem (19, TRANS ("Check New Version..."), true);
    m.addItem (20, TRANS ("About..."), true);

    // display the menu
    const int index = m.show ();

    if (index >= 100 && index < 200)   // recently opened files..
    {
        const File& project(recentFiles.getFile(index - 100));

        if (fileTreeContainer->projectTree.isValid())
        {
            if (project != FileTreeContainer::projectFile)
                Process::openDocument(File::getSpecialLocation(File::currentApplicationFile).getFullPathName(),
                                      project.getFullPathName());
        }
        else
        {
            fileTreeContainer->openProject(project);
        }        
    }
    else
    {
        menuPerform(index);
    }
}

//=================================================================================================
void TopToolBar::menuPerform (const int index)
{
    if (index == 1)         createNewProject ();
    else if (index == 2)    closeProject();
    else if (index == 3)    openProject ();
    else if (index == 4)    generateHtmlsIfNeeded();
    else if (index == 5)    cleanAndGenerateAll();
    else if (index == 6)    cleanLocalMedias();
    else if (index == 7)    exportCurrentTpls();
    else if (index == 8)    importExternalTpls();
    else if (index == 15)   setUiColour();
    else if (index == 16)   resetUiColour();
    else if (index == 18)   NEED_TO_DO ("Getting started...");
    else if (index == 19)   URL ("http://underwaySoft.com/works/wdtp").launchInDefaultBrowser (); // check new version
    else if (index == 20)   SwingUtilities::showAbout (TRANS ("Write Down, Then Publish"), "2017");

    // language
    else if (index == 30)
    {
        systemFile->setValue ("language", "English");
    }

    else if (index == 31)
    {
        systemFile->setValue ("language", "Chinese");
    }
}

//=================================================================================================
void TopToolBar::createNewProject ()
{
    // popup file save dialog
    FileChooser fc (TRANS ("New Project..."), File::nonexistent, "*.wdtp", false);

    if (!fc.browseForFileToSave (false))
        return;

    File projectFile (fc.getResult ());

    if (!projectFile.hasFileExtension (".wdtp"))
        projectFile = projectFile.withFileExtension ("wdtp");

    // overwrite or not if it has been there
    if (projectFile.existsAsFile () && 
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                       TRANS ("Message"), 
                                       TRANS ("This project already exists, want to overwrite it?")))
    {
        return;
    }

    // create and initial project file
    if (!projectFile.deleteFile ())
    {
        SHOW_MESSAGE (TRANS ("Can't overwrite this project! "));
        return;
    }

    projectFile.create ();

    ValueTree p("wdtpProject");
    p.setProperty("name", "site", nullptr);
    p.setProperty("title", projectFile.getFileNameWithoutExtension(), nullptr);
    p.setProperty("description", TRANS("Description of this project..."), nullptr);
    p.setProperty("owner", SystemStats::getLogonName(), nullptr);
    p.setProperty("order", 0, nullptr);
    p.setProperty("ascending", 0, nullptr);
    p.setProperty("dirFirst", 0, nullptr);
    p.setProperty("showWhat", 0, nullptr);
    p.setProperty("tooltip", 0, nullptr);
    p.setProperty("render", "blog", nullptr);
    p.setProperty("tplFile", "index.html", nullptr);
    p.setProperty("ad", "ad-1.jpg http://underwaySoft.com", nullptr);
    p.setProperty("contact", "Email: yourEmail-1@xxx.com, yourEmail-2@xxx.com<br>QQ: 123456789 (QQ Name) WeChat: yourWeChat", nullptr);
    p.setProperty("copyright", "&copy; 2017 " + SystemStats::getLogonName() + " All Right Reserved", nullptr);
    p.setProperty("needCreateHtml", true, nullptr);

    // create dirs and default template files
    projectFile.getSiblingFile ("docs").createDirectory();
    
    // release templates in 'themes/..' and css/js, image files in 'site/add-in'
    const File projectRoot(projectFile.getParentDirectory());
    MemoryInputStream inputSteam(BinaryData::SiteData_zip, BinaryData::SiteData_zipSize, false);
    ZipFile zip(inputSteam);
    zip.uncompressTo(projectRoot);
    
    // release logo image to "site/add-in"
    const File imgFile(projectFile.getSiblingFile("site/add-in").getChildFile("logo.png"));
    Image logoImg(ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));

    PNGImageFormat pngFormat;
    ScopedPointer<FileOutputStream> imgOutStram(imgFile.createOutputStream());

    pngFormat.writeImageToStream(logoImg, *imgOutStram);
    imgOutStram->flush();
    imgOutStram = nullptr;

    // save the project file
    if (SwingUtilities::writeValueTreeToFile (p, projectFile))
        fileTreeContainer->openProject (projectFile); // load the new project file
    else
        SHOW_MESSAGE (TRANS ("Something wrong during create this project file."));
}

//=================================================================================================
void TopToolBar::openProject ()
{
    FileChooser fc (TRANS ("Open Project..."), File::nonexistent, "*.wdtp", false);

    if (fc.browseForFileToOpen ())
        fileTreeContainer->openProject (fc.getResult ());
}

//=================================================================================================
void TopToolBar::closeProject()
{
    fileTreeContainer->closeProject();
    bts[view]->setVisible(false);
}

//=================================================================================================
void TopToolBar::cleanAndGenerateAll ()
{
    if (AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, 
                                       TRANS ("Confirm"),
                                       TRANS ("Do you really want to cleanup the whole site\n"
                                              "and then auto-regenerate them all?")))
    {
        // move the add-in dir which inlcude style.css, code-highlight.js
        // prevent it will be deleted
        const File addinDir (FileTreeContainer::projectFile.getSiblingFile ("site").getChildFile("add-in"));
        const File tempDirForAddin (FileTreeContainer::projectFile.getSiblingFile ("add-in"));
        addinDir.copyDirectoryTo (tempDirForAddin);

        // cleanup
        FileTreeContainer::projectFile.getSiblingFile("site").deleteRecursively();
        
        generateHtmlFiles(FileTreeContainer::projectTree);
        FileTreeContainer::saveProject();

        // restore the add-in dir
        tempDirForAddin.moveFileTo(addinDir);
        SHOW_MESSAGE(TRANS("Site clean and regenerate successful!"));

        tempDirForAddin.deleteRecursively();
    }
}

//=================================================================================================
void TopToolBar::generateHtmlFiles (ValueTree tree)
{
    if (!DocTreeViewItem::getMdFileOrDir(tree).exists())
        return;

    tree.setProperty("needCreateHtml", true, nullptr);

	if (tree.getType ().toString () == "doc")
	{
		HtmlProcessor::createArticleHtml (tree, false);
	}
	else
	{
		HtmlProcessor::createIndexHtml (tree, false);

		for (int i = tree.getNumChildren (); --i >= 0; )
			generateHtmlFiles (tree.getChild (i));
	}
}

//=================================================================================================
void TopToolBar::generateHtmlsIfNeeded()
{
    generateHtmlFilesIfNeeded(fileTreeContainer->projectTree);
    FileTreeContainer::saveProject();

    SHOW_MESSAGE(TRANS("All chaned items regenerate successful!"));
}

//=================================================================================================
void TopToolBar::generateHtmlFilesIfNeeded (ValueTree tree)
{
    if ((bool)tree.getProperty("needCreateHtml"))
    {
        if (tree.getType().toString() == "doc")
            HtmlProcessor::createArticleHtml(tree, false);
        else
            HtmlProcessor::createIndexHtml(tree, false);
    }

    for (int i = tree.getNumChildren(); --i >= 0; )
        generateHtmlFilesIfNeeded(tree.getChild(i));
}

//=================================================================================================
void TopToolBar::generateCurrentPage()
{
    editAndPreview->getCurrentTree().setProperty("needCreateHtml", true, nullptr);
    editAndPreview->switchMode(true);
}

//=================================================================================================
void TopToolBar::setUiColour()
{
    bgColourSelector = new ColourSelectorWithPreset();

    //bgColourSelector->setColour(ColourSelector::backgroundColourId, Colour(0xffededed));
    bgColourSelector->setSize(450, 480);
    bgColourSelector->setCurrentColour(Colour::fromString(systemFile->getValue("uiBackground")));
    bgColourSelector->addChangeListener(this);

    CallOutBox callOut(*bgColourSelector, getScreenBounds(), nullptr);
    callOut.runModalLoop();

    systemFile->saveIfNeeded();
}

//=================================================================================================
void TopToolBar::changeListenerCallback(ChangeBroadcaster* source)
{
    if (source == bgColourSelector)
    {
        // itself
        repaint();

        // update ui's background colour
        systemFile->setValue("uiBackground", bgColourSelector->getCurrentColour().toString());
        getParentComponent()->repaint();

        // treeView
        if (bgColourSelector->getCurrentColour().getBrightness() >= 0.70f)
            systemFile->setValue("uiTextColour", Colour(0xff303030).toString());
        else
            systemFile->setValue("uiTextColour", Colour(0xffe9e9e9).toString());

        TreeViewItem* projectTreeItem = fileTreeContainer->getTreeView().getRootItem();
        
        if (projectTreeItem != nullptr)
            projectTreeItem->repaintItem();

        // setup panel

    }
}

//=================================================================================================
ApplicationCommandTarget* TopToolBar::getNextCommandTarget()
{
    //return findFirstTargetParentComponent();
    return nullptr;
}

//=================================================================================================
void TopToolBar::getAllCommands(Array<CommandID>& commands)
{
    commands.add(10); // switch mode (preview / edit)
    commands.add(11); // switch width 
    commands.add(12); // regenerate current page
    commands.add(13); // generate all needed
}

//=================================================================================================
void TopToolBar::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    if (10 == commandID)
    {
        result.setInfo("Switch mode", "Switch to preview/edit", String(), 0);
        result.addDefaultKeypress(KeyPress::tabKey, ModifierKeys::commandModifier);
    } 
    else if (11 == commandID)
    {
        result.setInfo("Switch width", "Switch width", String(), 0);
        result.addDefaultKeypress('`', ModifierKeys::commandModifier);
    }
    else if (12 == commandID)
    {
        result.setInfo("Regenerate Current Page", "Regenerate Current Page", String(), 0);
        result.addDefaultKeypress(KeyPress::F5Key, ModifierKeys::noModifiers);
        result.setActive(bts[view]->getToggleState() && editAndPreview->getCurrentDocFile().exists());
    }
    else if (13 == commandID)
    {
        result.setInfo("Regenerate All Changed", "Regenerate All Changed", String(), 0);
        result.addDefaultKeypress(KeyPress::F6Key, ModifierKeys::noModifiers);
        result.setActive(fileTreeContainer->hasLoadedProject());
    }
}

//=================================================================================================
bool TopToolBar::perform(const InvocationInfo& info)
{
    if (info.commandID == 10)
    {
        bts[view]->triggerClick();
        return true;
    }
    else if (info.commandID == 11)
    {
        bts[width]->triggerClick();
        return true;
    }
    else if (info.commandID == 12)
    {
        generateCurrentPage();
        return true;
    }
    else if (info.commandID == 13)
    {
        generateHtmlsIfNeeded();
        return true;
    }
    else
    {
        return false;
    }
}

//=================================================================================================
void TopToolBar::resetUiColour()
{
    if (AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, TRANS("Confirm"),
                                     TRANS("Do you want to reset the UI's color?")))
    {
        // background colour
        systemFile->setValue("uiBackground", Colour(0xffdcdbdb).toString());
        getParentComponent()->repaint();

        systemFile->setValue("uiTextColour", Colour(0xff303030).toString());

        // treeView
        TreeViewItem* projectTreeItem = fileTreeContainer->getTreeView().getRootItem();

        if (projectTreeItem != nullptr)
            projectTreeItem->repaintItem();

        systemFile->saveIfNeeded();
    }
}

//=================================================================================================
void TopToolBar::exportCurrentTpls()
{
    const File& pFile(FileTreeContainer::projectFile);
    ZipFile::Builder builder;

    // theme
    const String themeStr("themes" + File::separatorString + FileTreeContainer::projectTree.getProperty("render").toString());
    const File& themeDir(pFile.getSiblingFile(themeStr));

    Array<File> themeFiles;
    themeDir.findChildFiles(themeFiles, File::findFiles, false, "*");

    for (int i = themeFiles.size(); --i >= 0; )
    {
        // NEED_TO_DO: here need check if include some OS system-file
        builder.addFile(themeFiles[i], 9, themeStr + File::separatorString + themeFiles[i].getFileName());
    }

    // add-in
    const String addStr("site" + File::separatorString + "add-in");
    const File& addDir(pFile.getSiblingFile(addStr));

    Array<File> addFiles;
    addDir.findChildFiles(addFiles, File::findFiles, false, "*");

    for (int i = addFiles.size(); --i >= 0; )
    {
        // NEED_TO_DO: here need check if include some OS system-file
        if (addFiles[i].getFileName() != "desktop.ini")
            builder.addFile(addFiles[i], 9, addStr + File::separatorString + addFiles[i].getFileName());
    }
    
    // write to zip file
    const File tplZip(pFile.getSiblingFile(pFile.getFileNameWithoutExtension() + ".wtpl"));
    tplZip.deleteFile();
    tplZip.create();

    ScopedPointer<FileOutputStream> out = tplZip.createOutputStream();
    
    if (builder.writeToStream(*out, nullptr))
    {
        out->flush();
        out = nullptr;
        SHOW_MESSAGE(TRANS("Export successful!"));

        tplZip.revealToUser();
    }
    else
        SHOW_MESSAGE(TRANS("Somehow the export failed."));
}

//=================================================================================================
void TopToolBar::importExternalTpls()
{
    FileChooser fc(TRANS("Open Template File..."), File::nonexistent, "*.wtpl", false);

    if (fc.browseForFileToOpen())
    {
        ZipFile zip(fc.getResult());
        const bool notZip = zip.getNumEntries() < 1;

        String message(zip.uncompressTo(FileTreeContainer::projectFile.getParentDirectory()).getErrorMessage());

        if (notZip)
            message = TRANS("Invalid templates file.");

        if (message.isNotEmpty() || notZip)
            SHOW_MESSAGE(TRANS("Import failed:") + newLine + message);
        else
            SHOW_MESSAGE(TRANS("Import successful!\nPlease regenerate the whole site if you want to use it."));
    }

    // here should update the project-setup panel
    fileTreeContainer->getTreeView().getRootItem()->setSelected(true, true);
    editAndPreview->setProjectProperties(FileTreeContainer::projectTree);
}

//=================================================================================================
void TopToolBar::cleanLocalMedias ()
{
	// exist medias
	Array<File> allDirs;
	Array<File> allMediasOnLocal;
	FileTreeContainer::projectFile.getSiblingFile ("docs").findChildFiles (allDirs, File::findDirectories, true);

	for (int i = allDirs.size () ; --i >= 0; )
	{
		if (allDirs[i].getFileName () != "media")
			allDirs.remove (i);
		else
			allDirs[i].findChildFiles (allMediasOnLocal, File::findFiles, false);
	}

	// all medias in docs
	Array<File> allDocs;
	Array<File> allMediasInDocs;
	FileTreeContainer::projectFile.getSiblingFile ("docs").findChildFiles (allDocs, File::findFiles, true);
	
	for (int i = allDocs.size(); --i >= 0; )
		DocTreeViewItem::getMdMediaFiles (allDocs[i], allMediasInDocs);

	allMediasOnLocal.removeValuesIn (allMediasInDocs);

	if (allMediasOnLocal.size () < 1)
	{
		SHOW_MESSAGE (TRANS ("Your project is very neat. \nNo need to clean it up."));
	}
	else
	{
		String extraFilesName;

		for (int i = allMediasOnLocal.size (); --i >= 0; )
			extraFilesName += "  - " + allMediasOnLocal[i].getFullPathName () + newLine;

		if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
			TRANS ("Find ") + String (allMediasOnLocal.size ()) + " " 
			+TRANS ("redundant media-file(s):") + newLine
			+ extraFilesName + newLine
			+ TRANS ("Do you want to clean them up?")))
		{
			for (int i = allMediasOnLocal.size (); --i >= 0; )
				allMediasOnLocal[i].moveToTrash ();

			SHOW_MESSAGE (TRANS ("Local medias cleanup successful!"));
		}
	}
}


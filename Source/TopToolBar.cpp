/*
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
TopToolBar::TopToolBar (FileTreeContainer* f, 
                        EditAndPreview* e) 
    : Thread ("forGenerateHtmls"),
    fileTreeContainer (f),
    editAndPreview (e),
    progressBar (progressValue)
{
    jassert (fileTreeContainer != nullptr);
    jassert (editAndPreview != nullptr);

    // 2 search textEditors..
    addAndMakeVisible (searchInProject = new TextEditor());
    searchInProject->addListener (this);

    searchInProject->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInProject->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInProject->setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha (0.6f));
    searchInProject->setScrollBarThickness (10);
    searchInProject->setFont (SwingUtilities::getFontSize() - 3.f);
    searchInProject->setSelectAllWhenFocused (true);

    addAndMakeVisible (searchInDoc = new TextEditor());
    searchInDoc->addListener (this);

    searchInDoc->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInDoc->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInDoc->setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha (0.6f));
    searchInDoc->setScrollBarThickness (10);
    searchInDoc->setFont (SwingUtilities::getFontSize() - 3.f);
    searchInDoc->setSelectAllWhenFocused (true);

    // ui language
    setUiLanguage ((LanguageID)systemFile->getIntValue ("language"));

    // image buttons...
    for (int i = totalBts; --i >= 0; )
    {
        MyImageButton* bt = new MyImageButton();
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

    String ctrlStr ("Ctrl");

#if JUCE_MAC
    ctrlStr = "Cmd";
#endif

    bts[view]->setTooltip (TRANS ("Switch Preview / Edit Mode") + "  (" + ctrlStr + " + S)");
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

    bts[width]->setTooltip (TRANS ("Switch Simply / Full Mode") + "  (" + ctrlStr + " + D)");
    bts[width]->setImages (false, true, true,
                           ImageCache::getFromMemory (BinaryData::width_png,
                                                      BinaryData::width_pngSize),
                           imageTrans, Colour (0x00),
                           Image::null, 1.0f, Colour (0x00),
                           Image::null, 1.0f, Colours::darkcyan);

    bts[width]->setToggleState (true, dontSendNotification);

    // progressBar
    progressBar.setColour (ProgressBar::backgroundColourId, Colour (0x00));
    progressBar.setColour (ProgressBar::foregroundColourId, Colours::lightskyblue);
    progressBar.setPercentageDisplay (false);
    addAndMakeVisible (progressBar);
}

//=================================================================================================
TopToolBar::~TopToolBar()
{
    if (isThreadRunning())
        stopThread (3000);
}

//=======================================================================
void TopToolBar::paint (Graphics& g)
{
    g.setColour (Colour::fromString (systemFile->getValue ("uiTextColour")).withAlpha (0.6f));
    g.drawLine (1.0f, getHeight() - 0.5f, getWidth() - 2.0f, getHeight() - 0.5f, 0.6f);
    //g.drawVerticalLine (getWidth() / 2, 0.5f, getHeight() - 1.0f);
}

//=======================================================================
void TopToolBar::resized()
{
    // search textEditors and find buttons
    if (getWidth() >= 800)
    {
        bts[prevAll]->setVisible (true);
        searchInProject->setVisible (true);
        bts[nextAll]->setVisible (true);
        bts[nextPjt]->setVisible (true);
        searchInDoc->setVisible (true);
        bts[prevPjt]->setVisible (true);

        bts[prevAll]->setBounds (12, 14, 16, 16);
        searchInProject->setBounds (bts[prevAll]->getRight() + 10, 10, 200, 25);
        bts[nextAll]->setBounds (searchInProject->getRight() + 10, 14, 16, 16);

        bts[nextPjt]->setBounds (getWidth() - 24, 14, 16, 16);
        searchInDoc->setBounds (bts[nextPjt]->getX() - 230, 10, 220, 25);
        bts[prevPjt]->setBounds (searchInDoc->getX() - 25, 14, 16, 16);
    }
    else
    {
        bts[prevAll]->setVisible (false);
        searchInProject->setVisible (false);
        bts[nextAll]->setVisible (false);
        bts[nextPjt]->setVisible (false);
        searchInDoc->setVisible (false);
        bts[prevPjt]->setVisible (false);
    }

    // image buttons
    bts[system]->setTopLeftPosition (getWidth() / 2 - 9, 12);
    bts[view]->setTopRightPosition (bts[system]->getX() - 40, 12);
    bts[width]->setTopLeftPosition (bts[system]->getRight() + 40, 12);

    // progressBar
    progressBar.setBounds (0, getHeight() - 5, getWidth(), 5);
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
        searchInProject->setText (String(), false);

    else if (&te == searchInDoc)
        searchInDoc->setText (String(), false);
}

//=================================================================================================
void TopToolBar::findInProject (const bool next)
{
    const String& keyword (searchInProject->getText());

    if (keyword.isEmpty())
        return;

    TreeView& treeView (fileTreeContainer->getTreeView());
    treeView.setDefaultOpenness (true);

    // get start (selected) row-number
    int startIndex = 0;

    for (int i = startIndex; i < treeView.getNumRowsInTree(); ++i)
    {
        if (treeView.getItemOnRow (i)->isSelected())
        {
            startIndex = i;
            break;
        }
    }

    // find and select
    for (int i = next ? startIndex + 1 : startIndex - 1;
         next ? (i < treeView.getNumRowsInTree()) : (i >= 0);
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
            treeView.scrollToKeepItemVisible (item);

            editAndPreview->getEditor()->moveCaretToTop (false);
            findInDoc (true);

            return;
        }
    }

    LookAndFeel::getDefaultLookAndFeel().playAlertSound();
}

//=================================================================================================
void TopToolBar::findInDoc (const bool next)
{
    const String& keyword (searchInDoc->getText());

    if (keyword.isEmpty())
        return;

    editAndPreview->switchMode (false);
    MarkdownEditor* editor = (MarkdownEditor*)editAndPreview->getEditor();
    const String& content = editor->getText();

    int startIndex = 0;
    int caretIndex = editor->getCaretPosition();

    // find the start index of the keyword
    if (next)
        startIndex = content.indexOfIgnoreCase (caretIndex, keyword);
    else
        startIndex = content.substring (0, caretIndex - 1).lastIndexOfIgnoreCase (keyword);

    // select the keyword
    if (startIndex != -1)
    {
        Array<Range<int>> rangeArray;
        rangeArray.add (Range<int> (startIndex, startIndex + keyword.length()));

        editor->setCaretPosition (startIndex + keyword.length());
        //editor->setHighlightedRegion (rangeArray[0]);
        editor->setTemporaryUnderlining (rangeArray);
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
        bts[view]->setToggleState (!bts[view]->getToggleState(), dontSendNotification);
        editAndPreview->startWork (editAndPreview->getCurrentTree());
    }
    else if (bt == bts[width])
    {
        bts[width]->setToggleState (!bts[width]->getToggleState(), dontSendNotification);
        getParentComponent()->setSize ((bts[width]->getToggleState() ? 1260 : 700), 800);
        getTopLevelComponent()->setCentreRelative (0.5f, 0.53f);
    }
    else if (bt == bts[system])
        popupSystemMenu();
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
void TopToolBar::popupSystemMenu()
{
    PopupMenu m;
    m.addItem (newPjt, TRANS ("New Project..."), true);
    m.addItem (packPjt, TRANS ("Pack Project"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    m.addItem (openPjt, TRANS ("Open Project..."), true);

    // recent files
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
    PopupMenu recentFilesMenu;
    recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);

    m.addSubMenu (TRANS ("Open Rcent"), recentFilesMenu);
    m.addSeparator();

    m.addItem (closePjt, TRANS ("Close Project"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    m.addCommandItem (cmdManager, generateCurrent);
    m.addCommandItem (cmdManager, generateNeeded);
    m.addSeparator();

    m.addItem (generateWhole, TRANS ("Regenerate Whole Site"), fileTreeContainer->hasLoadedProject());
    m.addItem (cleanUpLocal, TRANS ("Cleanup Local Medias"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    m.addItem (rebuildKeywords, TRANS ("Rebuild Keywords Table"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    m.addItem (exportTpl, TRANS ("Export Current Templates"), fileTreeContainer->hasLoadedProject());
    m.addItem (importTpl, TRANS ("Import External Templates..."), fileTreeContainer->hasLoadedProject());
    m.addItem (releaseSystemTpl, TRANS ("Reset/Repair Default Templates"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();

    PopupMenu lanMenu;
    lanMenu.addItem (uiEnglish, "English", true, systemFile->getIntValue ("language") == 0);
    lanMenu.addItem (uiChinese, CharPointer_UTF8 ("\xe4\xb8\xad\xe6\x96\x87"), true, 
                     systemFile->getIntValue ("language") == 1);
    m.addSubMenu (TRANS ("UI Language"), lanMenu);

    PopupMenu uiMenu;
    uiMenu.addItem (setUiColor, TRANS ("Set UI Color..."));
    uiMenu.addItem (resetUiColor, TRANS ("Reset to Default"));
    m.addSubMenu (TRANS ("UI Color"), uiMenu);
    m.addSeparator();

    m.addItem (gettingStarted, TRANS ("Getting Started..."), true);
    m.addItem (checkNewVersion, TRANS ("Check New Version..."), true);
    m.addItem (showAbout, TRANS ("About..."), true);

    // display the menu
    const int index = m.show();
    
    // recently opened files..
    if (index >= 100 && index < 200)   
        fileTreeContainer->openProject (recentFiles.getFile (index - 100));
    else
        menuPerform (index);
}

//=================================================================================================
void TopToolBar::menuPerform (const int index)
{
    if (index == newPjt)                createNewProject();
    else if (index == packPjt)          packProject();
    else if (index == closePjt)         closeProject();
    else if (index == openPjt)          openProject();
    else if (index == generateWhole)    cleanAndGenerateAll();
    else if (index == cleanUpLocal)     cleanLocalMedias();
    else if (index == rebuildKeywords)  rebuildAllKeywords (true);
    else if (index == exportTpl)        exportCurrentTpls();
    else if (index == importTpl)        importExternalTpls();
    else if (index == releaseSystemTpl) releaseSystemTpls (FileTreeContainer::projectFile, true);
    else if (index == setUiColor)       setUiColour();
    else if (index == resetUiColor)     resetUiColour();
    else if (index == gettingStarted)   URL ("http://underwaysoft.com/works/wdtp/gettingStarted.html").launchInDefaultBrowser();
    else if (index == checkNewVersion)  URL ("http://underwaySoft.com/works/wdtp/download.html").launchInDefaultBrowser();
    else if (index == showAbout)        SwingUtilities::showAbout (TRANS ("Write Down, Then Publish"), "2017");

    // switch ui-language in realtime
    else if (index == uiEnglish)
    {
        systemFile->setValue ("language", 0);
        setUiLanguage (English);
    }
    else if (index == uiChinese)
    {
        systemFile->setValue ("language", 1);
        setUiLanguage (Chinese);
    }
}

//=================================================================================================
void TopToolBar::createNewProject()
{
    // popup native file save dialog
    FileChooser fc (TRANS ("New Project..."), File::nonexistent, "*.wdtp", true);

    if (!fc.browseForFileToSave (false))
        return;

    File projectFile (fc.getResult());

    if (!projectFile.hasFileExtension (".wdtp"))
        projectFile = projectFile.withFileExtension ("wdtp");

    // overwrite or not if it has been there
    if (projectFile.existsAsFile() &&
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                       TRANS ("Message"),
                                       TRANS ("This project already exists, want to overwrite it?")))
    {
        return;
    }

    // create and initial project file
    if (!projectFile.deleteFile())
    {
        SHOW_MESSAGE (TRANS ("Can't overwrite this project! "));
        return;
    }

    projectFile.create();

    ValueTree p ("wdtpProject");
    p.setProperty ("name", "site", nullptr);
    p.setProperty ("title", projectFile.getFileNameWithoutExtension(), nullptr);
    p.setProperty ("description", TRANS ("Description of this project..."), nullptr);
    p.setProperty ("owner", SystemStats::getLogonName(), nullptr);
    p.setProperty ("order", 0, nullptr);
    p.setProperty ("ascending", 0, nullptr);
    p.setProperty ("dirFirst", 0, nullptr);
    p.setProperty ("showWhat", 0, nullptr);
    p.setProperty ("tooltip", 0, nullptr);
    p.setProperty ("render", "blog", nullptr);
    p.setProperty ("tplFile", "index.html", nullptr);
    p.setProperty ("ad", "ad-1.jpg http://underwaySoft.com", nullptr);
    p.setProperty ("contact", "Email: yourEmail-1@xxx.com, yourEmail-2@xxx.com<br>QQ: 123456789 (QQ Name) WeChat: yourWeChat", nullptr);
    p.setProperty ("copyright", "&copy; 2017 " + SystemStats::getLogonName() + " All Right Reserved", nullptr);
    p.setProperty ("needCreateHtml", true, nullptr);

    // create 'docs' dir 
    projectFile.getSiblingFile ("docs").createDirectory();

    // release system tpls and add-in files (this also create 'themes' and 'site' dir)
    releaseSystemTpls (projectFile, false);

    // save the new project file and load it
    if (SwingUtilities::writeValueTreeToFile (p, projectFile))
        fileTreeContainer->openProject (projectFile);
    else
        SHOW_MESSAGE (TRANS ("Something wrong during create this project file."));
}

//=================================================================================================
void TopToolBar::openProject()
{
    FileChooser fc (TRANS ("Open Project..."), File::nonexistent, "*.wdtp;*.wpck", true);

    if (fc.browseForFileToOpen())
        fileTreeContainer->openProject (fc.getResult());
}

//=================================================================================================
void TopToolBar::closeProject()
{
    fileTreeContainer->closeProject();
    bts[view]->setVisible (false);
}

//=================================================================================================
void TopToolBar::cleanAndGenerateAll()
{
    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                      TRANS ("Confirm"),
                                      TRANS ("Do you really want to cleanup the whole site\n"
                                             "and then auto-regenerate them all?")))
    {
        // move the add-in dir and favicon.ico prevent they will be deleted
        const File addinDir (FileTreeContainer::projectFile.getSiblingFile ("site").getChildFile ("add-in"));
        const File tempDirForAddin (FileTreeContainer::projectFile.getSiblingFile ("add-in"));
        addinDir.copyDirectoryTo (tempDirForAddin);

        const File iconFile (FileTreeContainer::projectFile.getSiblingFile ("site").getChildFile ("favicon.ico"));
        const File tempIconFile (FileTreeContainer::projectFile.getSiblingFile ("favicon.ico"));
        iconFile.copyFileTo (tempIconFile);

        // cleanup and initial progress value
        FileTreeContainer::projectFile.getSiblingFile ("site").deleteRecursively();

        fileTreeContainer->getTreeView().getRootItem()->setOpen (true);
        totalItems = fileTreeContainer->getTreeView().getNumRowsInTree();
        accumulator = 0;
        progressValue = 0.0;

        progressBar.enterModalState();
        startThread();  // start generate..

        // restore the add-in dir and favicon.ico
        addinDir.createDirectory();
        iconFile.create();

        tempDirForAddin.moveFileTo (addinDir);
        tempIconFile.moveFileTo (iconFile);

        tempDirForAddin.deleteRecursively();
        tempIconFile.deleteFile();
    }
}

//=================================================================================================
double TopToolBar::progressValue = 0.0;
int TopToolBar::totalItems = 0;
int TopToolBar::accumulator = 0;

//=================================================================================================
void TopToolBar::generateHtmlFiles (ValueTree tree)
{
    if (!DocTreeViewItem::getMdFileOrDir (tree).exists())
        return;

    ++accumulator;
    progressValue = (double)accumulator / totalItems;
    const bool isDoc = (tree.getType().toString() == "doc");

    {
        // here must using messageThreadLock for item's repaint
        const MessageManagerLock mmLock;
        tree.setProperty ("needCreateHtml", true, nullptr);

        if (isDoc)
            HtmlProcessor::createArticleHtml (tree, false);
        else
            HtmlProcessor::createIndexHtml (tree, false);
    }

    if (!isDoc)
    {
        for (int i = tree.getNumChildren(); --i >= 0; )
            generateHtmlFiles (tree.getChild (i));
    }
}

//=================================================================================================
void TopToolBar::generateHtmlsIfNeeded()
{
    generateHtmlFilesIfNeeded (fileTreeContainer->projectTree);
    FileTreeContainer::saveProject();

    SHOW_MESSAGE (TRANS ("All changed items regenerate successful!"));
}

//=================================================================================================
void TopToolBar::generateHtmlFilesIfNeeded (ValueTree tree)
{
    if ((bool)tree.getProperty ("needCreateHtml"))
    {
        if (tree.getType().toString() == "doc")
        {
            HtmlProcessor::createArticleHtml (tree, false);
        }
        else
        {
            HtmlProcessor::createIndexHtml (tree, false);

            for (int i = tree.getNumChildren(); --i >= 0; )
                generateHtmlFilesIfNeeded (tree.getChild (i));
        }
    }    
}

//=================================================================================================
void TopToolBar::run()
{
    generateHtmlFiles (FileTreeContainer::projectTree);

    accumulator = 0;
    progressValue = 0.999;

    SHOW_MESSAGE (TRANS ("Site clean and regenerate successful!"));
    FileTreeContainer::saveProject();
    progressValue = 0.0;

    const MessageManagerLock mmLock;
    progressBar.exitModalState (0);
}

//=================================================================================================
void TopToolBar::generateCurrentPage()
{
    editAndPreview->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
    editAndPreview->switchMode (true);
}

//=================================================================================================
void TopToolBar::setUiColour()
{
    bgColourSelector = new ColourSelectorWithPreset();
    bgColourSelector->setSize (450, 480);
    bgColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("uiBackground")));
    bgColourSelector->addChangeListener (this);

    CallOutBox callOut (*bgColourSelector, getScreenBounds(), nullptr);
    callOut.runModalLoop();

    systemFile->saveIfNeeded();
}

//=================================================================================================
void TopToolBar::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == bgColourSelector)
    {
        // itself
        repaint();

        // update ui's background colour
        systemFile->setValue ("uiBackground", bgColourSelector->getCurrentColour().toString());
        getParentComponent()->repaint();

        // treeView
        if (bgColourSelector->getCurrentColour().getBrightness() >= 0.70f)
            systemFile->setValue ("uiTextColour", Colour (0xff303030).toString());
        else
            systemFile->setValue ("uiTextColour", Colour (0xffe9e9e9).toString());

        TreeViewItem* projectTreeItem = fileTreeContainer->getTreeView().getRootItem();

        if (projectTreeItem != nullptr)
            projectTreeItem->repaintItem();

        // change the setup panel's background color, but it seems extremely hard to do it
        // so here need to be done in the future...

    }
}

//=================================================================================================
ApplicationCommandTarget* TopToolBar::getNextCommandTarget()
{
    //return findFirstTargetParentComponent();
    return nullptr;
}

//=================================================================================================
void TopToolBar::getAllCommands (Array<CommandID>& commands)
{
    commands.add (switchEdit);
    commands.add (switchWidth);
    commands.add (generateCurrent);
    commands.add (generateNeeded);
    commands.add (activeSearch);
}

//=================================================================================================
void TopToolBar::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    if (switchEdit == commandID)
    {
        result.setInfo ("Switch mode", "Switch to preview/edit", String(), 0);
        result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
    }
    else if (switchWidth == commandID)
    {
        result.setInfo ("Switch width", "Switch width", String(), 0);
        result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
    }
    else if (generateCurrent == commandID)
    {
        result.setInfo (TRANS ("Update Current Page"), "Update Current Page", String(), 0);
        result.addDefaultKeypress (KeyPress::F5Key, ModifierKeys::noModifiers);
        result.setActive (bts[view]->getToggleState() && editAndPreview->getCurrentDocFile().exists());
    }
    else if (generateNeeded == commandID)
    {
        result.setInfo (TRANS ("Regenerate All Changed"), "Regenerate All Changed", String(), 0);
        result.addDefaultKeypress (KeyPress::F6Key, ModifierKeys::noModifiers);
        result.setActive (fileTreeContainer->hasLoadedProject());
    }
    else if (activeSearch == commandID)
    {
        result.setInfo (TRANS ("Active Search"), "Active Search", String(), 0);
        result.addDefaultKeypress ('f', ModifierKeys::commandModifier);
        result.setActive (fileTreeContainer->hasLoadedProject());
    }
}

//=================================================================================================
bool TopToolBar::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case switchEdit:        bts[view]->triggerClick();         break;
    case switchWidth:       bts[width]->triggerClick();        break;
    case generateCurrent:   generateCurrentPage();             break;
    case generateNeeded:    generateHtmlsIfNeeded();           break;
    case activeSearch:      searchInDoc->grabKeyboardFocus();   break;

    default:                return false; 
    }

    return true;
}

//=================================================================================================
void TopToolBar::resetUiColour()
{
    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                      TRANS ("Do you want to reset the UI's color?")))
    {
        // background colour
        systemFile->setValue ("uiBackground", Colour (0xffdcdbdb).toString());
        getParentComponent()->repaint();

        systemFile->setValue ("uiTextColour", Colour (0xff303030).toString());

        // treeView
        TreeViewItem* projectTreeItem = fileTreeContainer->getTreeView().getRootItem();

        if (projectTreeItem != nullptr)
            projectTreeItem->repaintItem();

        systemFile->saveIfNeeded();
    }
}

//=================================================================================================
void TopToolBar::packProject()
{
    const File& projectFile (FileTreeContainer::projectFile);
    const String rootPath (projectFile.getParentDirectory().getFullPathName() + File::separatorString);
    ZipFile::Builder builder;

    // add project file
    builder.addFile (projectFile, 9, projectFile.getFileName());

    // add all doc files (include all doc-dirs)
    const File& docsDir (projectFile.getSiblingFile ("docs"));
    Array<File> docFiles;
    docsDir.findChildFiles (docFiles, File::findFiles, true, "*"); 
    
    for (int i = docFiles.size(); --i >= 0; )
    {
        if (docFiles[i].getFileName() != "desktop.ini" && docFiles[i].getFileName() != ".DS_Store")
            builder.addFile (docFiles[i], 9, 
                             docFiles[i].getFullPathName().fromFirstOccurrenceOf (rootPath, false, false));
    }
    
    // add current themes
    const String themeStr ("themes" + 
                           File::separatorString + FileTreeContainer::projectTree.getProperty ("render").toString());
    const File& themeDir (projectFile.getSiblingFile (themeStr));

    Array<File> themeFiles;
    themeDir.findChildFiles (themeFiles, File::findFiles, false, "*");

    for (int j = themeFiles.size(); --j >= 0; )
    {
        if (themeFiles[j].getFileName() != "desktop.ini" && themeFiles[j].getFileName() != ".DS_Store")
            builder.addFile (themeFiles[j], 9, themeStr + File::separatorString + themeFiles[j].getFileName());
    }

    // add add-in dir and all its files
    const String addStr ("site" + File::separatorString + "add-in");
    const File& addDir (projectFile.getSiblingFile (addStr));

    Array<File> addFiles;
    addDir.findChildFiles (addFiles, File::findFiles, false, "*");

    for (int m = addFiles.size(); --m >= 0; )
    {
        if (addFiles[m].getFileName() != "desktop.ini" 
            && themeFiles[m].getFileName() != ".DS_Store")
        {
            builder.addFile (addFiles[m], 9, addStr + File::separatorString
                             + addFiles[m].getFileName());
        }
    }

    // add favicon.ico
    builder.addFile (projectFile.getSiblingFile ("site").getChildFile ("favicon.ico"), 
                     9, "site" + File::separatorString + "favicon.ico");

    // to get the date string ("-2017-0209-0508-16") for zip's file name
    String packDate (SwingUtilities::getCurrentTimeString());
    packDate = "-" + packDate.replaceSection (4, 0, "-").replaceSection (9, 0, "-").replaceSection (14, 0, "-");

    // write to zip file
    const File packZipFile (projectFile.getSiblingFile (projectFile.getFileNameWithoutExtension() + packDate + ".wpck"));
    packZipFile.deleteFile();
    packZipFile.create();

    ScopedPointer<FileOutputStream> out = packZipFile.createOutputStream();

    if (builder.writeToStream (*out, nullptr))
    {
        out->flush();
        out = nullptr;
        SHOW_MESSAGE (TRANS ("Pack the project's data successful!"));

        packZipFile.revealToUser();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Somehow the project's data packed failed."));
    }
}

//=================================================================================================
void TopToolBar::exportCurrentTpls()
{
    const File& pFile (FileTreeContainer::projectFile);
    ZipFile::Builder builder;

    // theme
    const String themeStr ("themes" + File::separatorString 
                           + FileTreeContainer::projectTree.getProperty ("render").toString());
    const File& themeDir (pFile.getSiblingFile (themeStr));

    Array<File> themeFiles;
    themeDir.findChildFiles (themeFiles, File::findFiles, false, "*");

    for (int i = themeFiles.size(); --i >= 0; )
    {
        // here need check if include some OS system-file
        if (themeFiles[i].getFileName() != "desktop.ini" && themeFiles[i].getFileName() != ".DS_Store")
            builder.addFile (themeFiles[i], 9, themeStr + File::separatorString + themeFiles[i].getFileName());
    }

    // add-in
    const String addStr ("site" + File::separatorString + "add-in");
    const File& addDir (pFile.getSiblingFile (addStr));

    Array<File> addFiles;
    addDir.findChildFiles (addFiles, File::findFiles, false, "*");

    for (int i = addFiles.size(); --i >= 0; )
    {
        // here need check if include some OS system-file
        if (addFiles[i].getFileName() != "desktop.ini" && themeFiles[i].getFileName() != ".DS_Store")
            builder.addFile (addFiles[i], 9, addStr + File::separatorString + addFiles[i].getFileName());
    }

    // write to zip file
    const File tplZip (pFile.getSiblingFile (pFile.getFileNameWithoutExtension() + ".wtpl"));
    tplZip.deleteFile();
    tplZip.create();

    ScopedPointer<FileOutputStream> out = tplZip.createOutputStream();

    if (builder.writeToStream (*out, nullptr))
    {
        out->flush();
        out = nullptr;
        SHOW_MESSAGE (TRANS ("Export successful!"));

        tplZip.revealToUser();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Somehow the export failed."));
    }
}

//=================================================================================================
void TopToolBar::importExternalTpls()
{
    FileChooser fc (TRANS ("Open Template File..."), File::nonexistent, "*.wtpl", true);

    if (fc.browseForFileToOpen())
    {
        ZipFile zip (fc.getResult());
        const bool notZip = zip.getNumEntries() < 1;

        String message (zip.uncompressTo (FileTreeContainer::projectFile.getParentDirectory()).getErrorMessage());

        if (notZip)
            message = TRANS ("Invalid templates file.");

        if (message.isNotEmpty() || notZip)
        {
            SHOW_MESSAGE (TRANS ("Import failed:") + newLine + message);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Import successful!\nPlease regenerate the whole site if you want to use it."));
         
            // here should update the project-setup panel
            fileTreeContainer->getTreeView().getRootItem()->setSelected (true, true);
        }
    }    
}

//=================================================================================================
void TopToolBar::releaseSystemTpls (const File& projectFile, const bool askAndShowMessage)
{
    if (askAndShowMessage)
    {
        if (!AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, TRANS ("Confirm"),
                                           TRANS ("This operate will overwrite your current templates!\n"
                                                  "Do you really want to do it?")))
            return;
    }

    // release templates in 'themes/..' and css/js, image files in 'site/add-in'
    const File projectRoot (projectFile.getParentDirectory());
    MemoryInputStream inputSteam (BinaryData::SiteData_zip, BinaryData::SiteData_zipSize, false);
    ZipFile zip (inputSteam);
    Result unzip = zip.uncompressTo (projectRoot);

    // release logo image to "site/add-in"
    const File imgFile (projectFile.getSiblingFile ("site/add-in").getChildFile ("logo.png"));
    Image logoImg (ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize));

    PNGImageFormat pngFormat;
    ScopedPointer<FileOutputStream> imgOutStram (imgFile.createOutputStream());

    bool releaseLogo = false;

    if (pngFormat.writeImageToStream (logoImg, *imgOutStram))
    {
        imgOutStram->flush();
        imgOutStram = nullptr;
        releaseLogo = true;
    }

    if (askAndShowMessage)
    {
        if (Result::ok() == unzip && releaseLogo)
            SHOW_MESSAGE (TRANS ("System TPLs reset/repair successful!"));
        else
            SHOW_MESSAGE (TRANS ("System TPLs reset/repair failed!"));
    }
}

//=================================================================================================
void TopToolBar::setUiLanguage (const LanguageID& id)
{
    languageStr = String();
    LocalisedStrings::setCurrentMappings (nullptr);

    if (id == Chinese)
    {
        languageStr = MemoryBlock (BinaryData::transcn_h, BinaryData::transcn_hSize).toString();
        LocalisedStrings::setCurrentMappings (new LocalisedStrings (languageStr, true));
    }

    fileTreeContainer->getTreeView().moveSelectedRow (1);
    fileTreeContainer->getTreeView().moveSelectedRow (-1);

    setEmptyTextOfSearchBox();
}

//=================================================================================================
void TopToolBar::setEmptyTextOfSearchBox()
{
    searchInProject->setTextToShowWhenEmpty (TRANS ("Search in this project"), 
                                             Colour (0xff303030).withAlpha (0.6f));
    searchInDoc->setTextToShowWhenEmpty (TRANS ("Search in current document"), 
                                         Colour (0xff303030).withAlpha (0.6f));

    // these 4 ugly staments for switch ui language without restart this app
    searchInProject->setText (" ");
    searchInDoc->setText (" ");
    searchInProject->setText (String());
    searchInDoc->setText (String());
}

//=================================================================================================
void TopToolBar::cleanLocalMedias()
{
    // exist medias
    Array<File> allDirs;
    Array<File> allMediasOnLocal;
    FileTreeContainer::projectFile.getSiblingFile ("docs").findChildFiles (allDirs, File::findDirectories, true);

    for (int i = allDirs.size(); --i >= 0; )
    {
        if (allDirs[i].getFileName() != "media")
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

    if (allMediasOnLocal.size() < 1)
    {
        SHOW_MESSAGE (TRANS ("Your project is very neat. \nNo need to clean it up."));
    }
    else
    {
        String extraFilesName;

        for (int i = allMediasOnLocal.size(); --i >= 0; )
            extraFilesName += "  - " + allMediasOnLocal[i].getFullPathName() + newLine;

        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                          TRANS ("Find ") + String (allMediasOnLocal.size()) + " "
                                          + TRANS ("redundant media-file(s):") + newLine
                                          + extraFilesName + newLine
                                          + TRANS ("Do you want to clean them up?")))
        {
            for (int i = allMediasOnLocal.size(); --i >= 0; )
                allMediasOnLocal[i].moveToTrash();

            SHOW_MESSAGE (TRANS ("Local medias cleanup successful!"));
        }
    }
}
//=================================================================================================
void TopToolBar::rebuildAllKeywords (const bool saveProjectAndPopupMessage)
{
    // extract all keywords of each doc of this project
    StringArray keywordsArray;
    ValueTree pTree (FileTreeContainer::projectTree);

    extractKeywords (pTree, keywordsArray);
    
    keywordsArray.appendNumbersToDuplicates (true, false, CharPointer_UTF8("--"), CharPointer_UTF8(""));
    keywordsArray.sortNatural();

    // remove duplicates and remain the last which include "-x (times)"
    for (int i = keywordsArray.size(); --i >= 1; )
    {
        if (keywordsArray[i].upToLastOccurrenceOf ("--", false, true).compareIgnoreCase (
            keywordsArray[i - 1].upToLastOccurrenceOf ("--", false, true)) == 0)
        {
            keywordsArray.remove (i - 1);
        }
    }

    // sort by duplicate-times
    for (int i = 0; i < keywordsArray.size(); ++i)
    {
        for (int j = 0; j < keywordsArray.size() - 1; ++j)
        {
            if (keywordsArray[j].fromLastOccurrenceOf ("--", false, true).getIntValue() <
                keywordsArray[j + 1].fromLastOccurrenceOf ("--", false, true).getIntValue())
            {
                const String str (keywordsArray[j]);
                keywordsArray.getReference (j) = keywordsArray[j + 1];
                keywordsArray.getReference (j + 1) = str;
            }
        }    	
    }

    // move the '123XX' to the end
    StringArray tempStrs;

    for (int i = 0; i < keywordsArray.size(); ++i)
    {
        if (!keywordsArray[i].contains ("--"))
        {
            tempStrs.add (keywordsArray[i]);
            keywordsArray.remove (i);
            --i;
        }
    }

    keywordsArray.addArray (tempStrs);
    String keywords (keywordsArray.joinIntoString (","));

    if (keywords.substring (0, 1) == ",")
        keywords = keywords.substring (1);

    //DBGX (keywords);
    pTree.setProperty ("allKeywords", keywords, nullptr);

    if (saveProjectAndPopupMessage)
    {
        FileTreeContainer::saveProject();
        SHOW_MESSAGE (TRANS ("All keywords in this project have been rebuilt successfully."));
    }    
}

//=================================================================================================
void TopToolBar::extractKeywords (const ValueTree& tree,
                                  StringArray& arrayToAdd)
{
    const String& keywords (tree.getProperty ("keywords").toString()
                            .replace (CharPointer_UTF8 ("\xef\xbc\x8c"), ",")); // Chinese ','
    StringArray thisArray;

    thisArray.addTokens (keywords, ",", String());
    thisArray.trim();
    thisArray.removeEmptyStrings();
    thisArray.removeDuplicates (true);
    arrayToAdd.addArray (thisArray);

    for (int i = tree.getNumChildren(); --i >= 0; )
        extractKeywords (tree.getChild (i), arrayToAdd);
}


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
extern AudioDeviceManager* deviceManager;

const float imageTrans = 1.f;

//==============================================================================
TopToolBar::TopToolBar (FileTreeContainer* f, 
                        EditAndPreview* e) 
    : Thread ("forGenerateHtmls"),
    fileTreeContainer (f),
    editAndPreview (e),
    progressBar (progressValue),
    newVersionIsReady (false)
{
    jassert (fileTreeContainer != nullptr);
    jassert (editAndPreview != nullptr);

    // 2 search textEditors..
    addAndMakeVisible (searchInput = new TextEditor());
    searchInput->addListener (this);

    searchInput->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInput->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInput->setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha (0.6f));
    searchInput->setScrollBarThickness (10);
    searchInput->setFont (SwingUtilities::getFontSize() - 3.f);
    searchInput->setSelectAllWhenFocused (true);    

    // image buttons...
    for (int i = totalBts; --i >= 0; )
    {
        MyImageButton* bt = new MyImageButton();
        bt->setSize (20, 20);
        bt->addListener (this);

        bts.add (bt);
        addAndMakeVisible (bt);
    }

    bts[searchPrev]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::prev_png,
                                                        BinaryData::prev_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    bts[searchNext]->setImages (false, true, true,
                             ImageCache::getFromMemory (BinaryData::next_png,
                                                        BinaryData::next_pngSize),
                             imageTrans, Colour (0x00),
                             Image::null, 1.0f, Colours::darkcyan,
                             Image::null, 1.0f, Colours::darkcyan);

    bts[viewBt]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::view_png,
                                                     BinaryData::view_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.0f, Colour (0x00),
                          Image::null, 1.0f, Colours::darkcyan);
    bts[viewBt]->setToggleState (true, dontSendNotification);
    
    bts[systemBt]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::system_png,
                                                       BinaryData::system_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colour (0x00),
                            Image::null, 1.000f, Colours::darkcyan);

    bts[layoutBt]->setImages (false, true, true,
                              ImageCache::getFromMemory (BinaryData::width_png,
                                                         BinaryData::width_pngSize),
                              imageTrans, Colour (0x00),
                              Image::null, 1.0f, Colour (0x00),
                              Image::null, 1.0f, Colours::darkcyan);

    // ui language
    setUiLanguage ((LanguageID)systemFile->getIntValue ("language"));

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
        bts[searchPrev]->setVisible (true);
        searchInput->setVisible (true);
        bts[searchNext]->setVisible (true);

        bts[searchPrev]->setBounds (12, 14, 16, 16);
        searchInput->setBounds (bts[searchPrev]->getRight() + 10, 10, 200, 25);
        bts[searchNext]->setBounds (searchInput->getRight() + 10, 14, 16, 16);
    }
    else
    {
        bts[searchPrev]->setVisible (false);
        searchInput->setVisible (false);
        bts[searchNext]->setVisible (false);
    }

    // image buttons
    bts[systemBt]->setTopLeftPosition (getWidth() / 2 - 9, 12);
    bts[viewBt]->setTopRightPosition (bts[systemBt]->getX() - 40, 12);
    bts[layoutBt]->setTopLeftPosition (bts[systemBt]->getRight() + 40, 12);

    // progressBar
    progressBar.setBounds (0, getHeight() - 5, getWidth(), 5);
}

//=================================================================================================
void TopToolBar::enableEditPreviewBt (const bool enableIt,
                                      const bool toggleState)
{
    bts[viewBt]->setToggleState (toggleState, dontSendNotification);
    bts[viewBt]->setVisible (enableIt);
}

//=========================================================================
void TopToolBar::textEditorReturnKeyPressed (TextEditor& te)
{
    if (&te == searchInput)
        keywordSearch (true);
}

//=========================================================================
void TopToolBar::textEditorEscapeKeyPressed (TextEditor& te)
{
    if (&te == searchInput)
        searchInput->setText (String(), false);
}

//=================================================================================================
void TopToolBar::keywordSearch (const bool next)
{
    const String& keyword (searchInput->getText());

    if (keyword.isEmpty() || !fileTreeContainer->projectTree.isValid())
        return;

    TreeView& treeView (fileTreeContainer->getTreeView());
    int startIndex = 0;
    
    if (treeView.getSelectedItem (0) != nullptr)
        startIndex = treeView.getSelectedItem (0)->getRowNumberInTree();

    for (int i = startIndex;
         next ? (i < treeView.getNumRowsInTree()) : (i >= 0);
         next ? ++i : --i)
    {
        DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView.getItemOnRow (i));
        item->setOpen (true);

        if (item == nullptr || item->getTree().getType().toString() != "doc")
            continue;

        const File& docFile (DocTreeViewItem::getMdFileOrDir (item->getTree()));
        const String& docContent (docFile.loadFileAsString());

        if (docContent.containsIgnoreCase (keyword))
        {
            item->setSelected (true, true);
            treeView.scrollToKeepItemVisible (item);

            editAndPreview->switchMode (false);
            MarkdownEditor* editor = (MarkdownEditor*)editAndPreview->getEditor();
            const String& content = editor->getText();

            int startIndexInDoc = 0;
            int caretIndex = editor->getCaretPosition();

            // find the start index of the keyword
            if (next)
            {
                startIndexInDoc = content.indexOfIgnoreCase (caretIndex, keyword);
            }
            else
            {
                if (caretIndex != 0)
                    startIndexInDoc = content.substring (0, caretIndex - 1).lastIndexOfIgnoreCase (keyword);
                else
                    startIndexInDoc = content.lastIndexOfIgnoreCase (keyword);
            }

            // highlight the keyword
            if (startIndexInDoc != -1)
            {
                Array<Range<int>> rangeArray;
                rangeArray.add (Range<int> (startIndexInDoc, startIndexInDoc + keyword.length()));

                editor->setCaretPosition (startIndexInDoc + keyword.length());
                editor->setTemporaryUnderlining (rangeArray);                
                
                return;
            }             
        }
    }

    SHOW_MESSAGE (TRANS ("Nothing could be found."));
}

//=========================================================================
void TopToolBar::buttonClicked (Button* bt)
{
    if (bt == bts[viewBt])
    {
        bts[viewBt]->setToggleState (!bts[viewBt]->getToggleState(), dontSendNotification);
        editAndPreview->saveCurrentDocIfChanged();
        editAndPreview->switchMode (bts[viewBt]->getToggleState());
    }

    else if (bt == bts[layoutBt])     popupLayoutMenu();
    else if (bt == bts[systemBt])     popupSystemMenu();
    else if (bt == bts[searchPrev])   keywordSearch (false);
    else if (bt == bts[searchNext])   keywordSearch (true);
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

    m.addItem (viewHtmlCode, TRANS ("View Html Code of Current Page"), 
               bts[viewBt]->getToggleState() && editAndPreview->getCurrentDocFile().exists());
    m.addCommandItem (cmdManager, generateCurrent);
    m.addCommandItem (cmdManager, generateNeeded);
    m.addSeparator();

    m.addItem (generateWhole, TRANS ("Regenerate Whole Site"), fileTreeContainer->hasLoadedProject());
    m.addItem (cleanUpLocal, TRANS ("Cleanup Needless Medias"), fileTreeContainer->hasLoadedProject());
    m.addSeparator();
    
    // set/edit theme files
    PopupMenu themeFilesMenu;
    createThemeFilesMenu (themeFilesMenu, 200);
    m.addSubMenu (TRANS ("Modify Current Theme"), themeFilesMenu, fileTreeContainer->hasLoadedProject());

    m.addItem (exportTpl, TRANS ("Export Current Theme"), fileTreeContainer->hasLoadedProject());
    m.addItem (importTpl, TRANS ("Import External Theme..."), fileTreeContainer->hasLoadedProject());
    m.addItem (releaseSystemTpl, TRANS ("Reset/Repair Default Theme"), fileTreeContainer->hasLoadedProject());
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
    m.addItem (setupAudio, TRANS ("Setup Audio Device..."));
    m.addSeparator();

    if (newVersionIsReady)
        m.addItem (checkNewVersion, TRANS ("Download New Version..."), true);

    // site url..
    PopupMenu siteUrls;
    siteUrls.addItem (wdtpUpdateList, TRANS ("Update List..."), true);
    siteUrls.addItem (gettingStarted, TRANS ("Getting Started..."), true);
    siteUrls.addItem (syntax, TRANS ("Text Mark Syntax and Demo..."), true);
    siteUrls.addItem (faq, TRANS ("FAQ..."), true);
    siteUrls.addItem (feedback, TRANS ("Feedback/Discuss/Interflow..."), true);

    m.addSubMenu (TRANS ("Help"), siteUrls);
    m.addItem (showAboutDialog, TRANS ("About..."), true);

    // display the menu
    const int index = m.show();    
    
    if (index >= 100 && index < 200)    // recently opened files..
    {
        fileTreeContainer->openProject (recentFiles.getFile (index - 100));
    }
        
    else if ((index == viewHtmlCode) || (index >= 200 && index < 300))  // edit theme file
    {
        MainContentComponent* main = dynamic_cast<MainContentComponent*>(getParentComponent());
        main->setLayout (false);

        if (index == viewHtmlCode)
        {
            editAndPreview->editThemeFile (editAndPreview->getCurrentUrl());
        }
        else if (index == 200)
        {
            editAndPreview->editThemeFile (fileTreeContainer->projectFile.getSiblingFile ("site")
                                           .getChildFile ("add-in").getChildFile ("style.css"));
        }
        else
        {
            Array<File> files;
            const String currentRender (FileTreeContainer::projectTree.getProperty ("render").toString());
            const File& dirOfRender (FileTreeContainer::projectFile.getSiblingFile ("themes").getChildFile (currentRender));
            dirOfRender.findChildFiles (files, File::findFiles, false, "*.html");

            editAndPreview->editThemeFile (files[index - 200 - 1]);
        }
    }

    else
    {
        systemMenuPerform (index);
    }
}

//=================================================================================================
void TopToolBar::systemMenuPerform (const int index)
{
    if (index == newPjt)                createNewProject();
    else if (index == packPjt)          packProject();
    else if (index == closePjt)         closeProject();
    else if (index == openPjt)          openProject();
    else if (index == generateWhole)    cleanAndGenerateAll();
    else if (index == cleanUpLocal)     cleanNeedlessMedias (true);
    
    else if (index == importIco)        setSiteImgs (0);
    else if (index == importLogo)       setSiteImgs (1);
    else if (index == exportTpl)        exportCurrentTpls();
    else if (index == importTpl)        importExternalTpls();
    else if (index == releaseSystemTpl) releaseSystemTpls (true);
    else if (index == setUiColor)       setUiColour();
    else if (index == resetUiColor)     resetUiColour();
    else if (index == setupAudio)       setupAudioDevice();

    else if (index == checkNewVersion)  URL ("http://underwaySoft.com/works/wdtp/download.html").launchInDefaultBrowser();
    else if (index == wdtpUpdateList)   URL ("http://underwaysoft.com/works/wdtp/updateList.html").launchInDefaultBrowser();
    else if (index == gettingStarted)   URL ("http://underwaysoft.com/works/wdtp/gettingStarted.html").launchInDefaultBrowser();
    else if (index == syntax)           URL ("http://underwaysoft.com/works/wdtp/syntaxMark.html").launchInDefaultBrowser();
    else if (index == faq)              URL ("http://underwaysoft.com/works/wdtp/faq.html").launchInDefaultBrowser();
    else if (index == feedback)         URL ("http://underwaysoft.com/guestBook.html").launchInDefaultBrowser();

    else if (index == showAboutDialog)  SwingUtilities::showAbout ( "Walden Trip (" 
                                            + String (CharPointer_UTF8 ("\xe5\xb1\xb1\xc2\xb7\xe6\xb9\x96\xc2\xb7\xe8\xb7\xaf"))
                                            + ")", "2017");

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
void TopToolBar::popupLayoutMenu()
{
    const bool isSilentMode = (getParentComponent()->getWidth() < 760);
    const bool hasProject = fileTreeContainer->projectTree.isValid();
    String ctrlStr ("  (Ctrl + ");

#if JUCE_MAC
    ctrlStr = "  (Cmd + ";
#endif

    PopupMenu menu;
    menu.addItem (1, TRANS ("Show File Tree Panel"), !isSilentMode && hasProject, 
                  fileTreeContainer->isVisible());
    menu.addItem (2, TRANS ("Show Properties Panel"), !isSilentMode && hasProject, 
                  editAndPreview->setupAreaIsShowing() && hasProject);

    menu.addSeparator();
    menu.addItem (3, TRANS ("Silent Mode") + ctrlStr + "D)", true, isSilentMode);

    const int index = menu.show();
    MainContentComponent* main = dynamic_cast<MainContentComponent*>(getParentComponent());

    if (index == 1 && main != nullptr)
        main->setLayout (!fileTreeContainer->isVisible());

    else if (index == 2)
        editAndPreview->setLayout (!editAndPreview->setupAreaIsShowing(), true);
    
    else if (index == 3)
        switchSilentMode (!isSilentMode);
}

//=================================================================================================
void TopToolBar::switchSilentMode (const bool enterSilent)
{
    DocumentWindow* mainWindow = findParentComponentOfClass<DocumentWindow>();
    jassert (mainWindow != nullptr);

    if (enterSilent)
    {
        const int height = getParentComponent()->getHeight();
        mainWindow->setFullScreen (false);
        getParentComponent()->setSize (710, height);
        getTopLevelComponent()->setCentreRelative (0.5f, 0.f);
    }
    else
    {
        mainWindow->setFullScreen (true);
    }
}

//=================================================================================================
void TopToolBar::createNewProject()
{
    // popup native file save dialog
    FileChooser fc (TRANS ("New Project..."), File::nonexistent, String(), true);

    if (!fc.browseForFileToSave (true))
        return;

    File projectDir (fc.getResult().getNonexistentSibling (true));
    const File projectFile (projectDir.getChildFile (projectDir.getFileNameWithoutExtension() + ".wdtp"));

    if (projectFile.create() != Result::ok())
    {
        SHOW_MESSAGE (TRANS ("Cannot create this project!"));
        return;
    }
    
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
    p.setProperty ("needCreateHtml", true, nullptr);
    p.setProperty ("copyright", "&copy; 2017 " + SystemStats::getLogonName() 
                   + " All Right Reserved", nullptr);
    p.setProperty ("contact", "Email: yourEmail-1@xxx.com, yourEmail-2@xxx.com<br>"
                   "QQ: 123456789 (QQ Name) WeChat: yourWeChat", nullptr);

    // create 'docs' dir 
    projectFile.getSiblingFile ("docs").createDirectory();

    // save the new project file and load it
    if (SwingUtilities::writeValueTreeToFile (p, projectFile))
    {
        // must open it first then release system tpls and add-in files
        // also it'll create 'themes' and 'site' dir
        fileTreeContainer->openProject (projectFile);
        releaseSystemTpls (false);

        // show some tip info
        Label info (String(), TRANS ("Right-click the root item then\n\n'New Folder' or 'New Document'..."));
        info.setColour (Label::textColourId, Colours::greenyellow.withAlpha (0.8f));
        info.setFont (SwingUtilities::getFontSize() - 2.f);
        info.setJustificationType (Justification::centred);
        info.setSize (260, 70);

        CallOutBox callOut (info, fileTreeContainer->getScreenBounds().withHeight (30).withWidth (150), nullptr);
        callOut.runModalLoop();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Something wrong during create this project file."));
    }
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
    bts[viewBt]->setVisible (false);
}

//=================================================================================================
void TopToolBar::cleanAndGenerateAll()
{
    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                      TRANS ("Confirm"),
                                      TRANS ("Do you really want to cleanup the whole site\n"
                                             "and then auto-regenerate them all?")))
    {
        // cleanup needless medias
        cleanNeedlessMedias (false);

        // cleanup all html but medias 
        const File& site (FileTreeContainer::projectFile.getSiblingFile ("site"));
        Array<File> htmls;
        site.findChildFiles (htmls, File::findFiles, true, "*.html");

        for (int i = htmls.size(); --i >= 0; )
            htmls[i].deleteFile();

        // initial progress value
        totalItems = 0;
        SwingUtilities::getAllChildrenNumbers (FileTreeContainer::projectTree, totalItems);

        accumulator = 0;
        progressValue = 0.0;

        progressBar.enterModalState();
        startThread();  // start generate..
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
    FileTreeContainer::projectTree.setProperty ("needCreateHtml", true, nullptr);
    generateHtmlFilesIfNeeded (FileTreeContainer::projectTree);
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
void TopToolBar::setSearchKeyword (const String& kw)
{
    searchInput->setText (kw, false);
}

//=================================================================================================
void TopToolBar::hasNewVersion()
{
    newVersionIsReady = true;

    bts[systemBt]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::systemr_png,
                                                       BinaryData::systemr_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colours::darkcyan,
                            Image::null, 1.000f, Colours::darkcyan);
}

//=================================================================================================
void TopToolBar::run()
{
    //const uint32 startTime = Time::getMillisecondCounter();
    generateHtmlFiles (FileTreeContainer::projectTree);
    accumulator = 0;
    progressValue = 0.999;

    AlertWindow::showMessageBox (AlertWindow::InfoIcon, 
                                 TRANS ("Congratulations"),
                                 TRANS ("Site clean and regenerate successful!"));

    FileTreeContainer::saveProject();
    progressValue = 0.0;

    const MessageManagerLock mmLock;
    progressBar.exitModalState (0);

    //DBGX (int (Time::getMillisecondCounter() - startTime));
}

//=================================================================================================
void TopToolBar::generateCurrentPage()
{
    ValueTree tree (editAndPreview->getCurrentTree());
    tree.setProperty ("needCreateHtml", true, nullptr);

    // link the index doc if this is a dir
    ValueTree indexTree (tree.getChildWithProperty ("name", var ("index")));

    if (indexTree.isValid())
        indexTree.setProperty ("needCreateHtml", true, nullptr);

    // then let the workarea show it
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
void TopToolBar::setTooltips()
{
    String ctrlStr ("Ctrl");

#if JUCE_MAC
    ctrlStr = "Cmd";
#endif

    bts[searchPrev]->setTooltip (TRANS ("Find Previous"));
    bts[searchNext]->setTooltip (TRANS ("Find Next"));
    bts[viewBt]->setTooltip (TRANS ("Switch Preview / Edit Mode") + "  (" + ctrlStr + " + S)");
    bts[systemBt]->setTooltip (TRANS ("Popup System Menu"));
    bts[layoutBt]->setTooltip (TRANS ("Popup Layout Menu"));
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
            systemFile->setValue ("uiTextColour", Colour (0xff181818).toString());
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
        result.setActive (bts[viewBt]->getToggleState() && editAndPreview->getCurrentDocFile().exists());
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
    const bool isSilentMode = (getParentComponent()->getWidth() < 760);

    switch (info.commandID)
    {
    case switchEdit:        bts[viewBt]->triggerClick();       break;
    case switchWidth:       switchSilentMode (!isSilentMode);  break;
    case generateCurrent:   generateCurrentPage();             break;
    case generateNeeded:    generateHtmlsIfNeeded();           break;
    case activeSearch:      searchInput->grabKeyboardFocus();  break;
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
        systemFile->setValue ("uiBackground", Colour (0xff202020).toString());
        getParentComponent()->repaint();

        systemFile->setValue ("uiTextColour", Colour (0xffe8e8e8).toString());

        // treeView
        TreeViewItem* projectTreeItem = fileTreeContainer->getTreeView().getRootItem();

        if (projectTreeItem != nullptr)
            projectTreeItem->repaintItem();

        systemFile->saveIfNeeded();
    }
}

//=================================================================================================
void TopToolBar::setupAudioDevice()
{
    AudioDeviceSelectorComponent deviceComp (*deviceManager, 0, 255, 2, 255, false, false, false, false);
    OptionalScopedPointer<Component> deviceEditor (&deviceComp, false);
    deviceEditor->setSize (500, 320);

    DialogWindow::LaunchOptions dialog;
    dialog.dialogTitle = TRANS ("Setup audio devices");
    dialog.content = deviceEditor;

    dialog.runModal();

    // save it
    ScopedPointer<XmlElement> audioSetup (deviceManager->createStateXml());
    systemFile->setValue ("audioState", audioSetup);
    systemFile->saveIfNeeded();
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
    FileChooser fc (TRANS ("Open Theme File..."), File::nonexistent, "*.wtpl", true);

    if (fc.browseForFileToOpen())
    {
        ZipFile zip (fc.getResult());
        const bool notZip = zip.getNumEntries() < 1;

        String message (zip.uncompressTo (FileTreeContainer::projectFile.getParentDirectory()).getErrorMessage());

        if (notZip)
            message = TRANS ("Invalid theme file.");

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
void TopToolBar::releaseSystemTpls (const bool askAndShowMessage)
{
    if (askAndShowMessage)
    {
        if (!AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, TRANS ("Confirm"),
                                           TRANS ("This operate will overwrite your current theme!\n"
                                                  "Do you really want to do it?")))
            return;
    }

    // doesn't overwrite favicon.ico
    const File& originalIco (FileTreeContainer::projectFile.getSiblingFile ("site").getChildFile ("favicon.ico"));

    if (originalIco.existsAsFile())
        SwingUtilities::renameFile (originalIco, "__favicon.ico__");

    // release default themes in 'themes/..' and css/js, image files in 'site/add-in'
    const File& projectRoot (FileTreeContainer::projectFile.getParentDirectory());
    MemoryInputStream inputSteam (BinaryData::SiteData_zip, BinaryData::SiteData_zipSize, false);
    ZipFile zip (inputSteam);
    Result unzip = zip.uncompressTo (projectRoot);

    // restore the original favicon.ico
    const File& icoFile (FileTreeContainer::projectFile.getSiblingFile ("site").getChildFile ("__favicon.ico__"));

    if (icoFile.existsAsFile())
        SwingUtilities::renameFile (icoFile, "favicon.ico");

    // release logo image to "site/add-in" if it hasn't been there
    const File& logoFile (FileTreeContainer::projectFile.getSiblingFile ("site/add-in").getChildFile ("logo.png"));

    if (!logoFile.existsAsFile())
    {
        logoFile.create();

        Image logoImg (ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize));
        PNGImageFormat pngFormat;
        ScopedPointer<FileOutputStream> imgOutStram (logoFile.createOutputStream());

        if (pngFormat.writeImageToStream (logoImg, *imgOutStram))
        {
            imgOutStram->flush();
            imgOutStram = nullptr;
        }
    }

    if (askAndShowMessage)
    {
        if (Result::ok() == unzip)
            SHOW_MESSAGE (TRANS ("System theme reset/repair successful!"));
        else
            SHOW_MESSAGE (TRANS ("System theme reset/repair failed!"));
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

    setTooltips();
}

//=================================================================================================
void TopToolBar::createThemeFilesMenu (PopupMenu& menu, const int baseId)
{
    if (fileTreeContainer->hasLoadedProject())
    {
        menu.addItem (importIco, TRANS ("Set Site Icon") + "...", true);
        menu.addItem (importLogo, TRANS ("Set Project Logo") + "...", true);
        menu.addSeparator();

        menu.addItem (baseId, TRANS ("Global Stylesheet"), true, 
                      (editAndPreview->themeEditorIsShowing() && 
                      editAndPreview->getEditingThemeFile().getFileName() == "style.css"));
        menu.addSeparator();

        Array<File> files;
        const String currentRender (FileTreeContainer::projectTree.getProperty ("render").toString());
        const File& dirOfRender (FileTreeContainer::projectFile.getSiblingFile ("themes").getChildFile (currentRender));
        dirOfRender.findChildFiles (files, File::findFiles, false, "*.html");

        for (int i = 0; i < files.size(); ++i)
            menu.addItem (baseId + i + 1, files[i].getFileName(), true, 
                          (editAndPreview->themeEditorIsShowing() && editAndPreview->getEditingThemeFile() == files[i]));
    }
}

//=================================================================================================
void TopToolBar::setSiteImgs (const int imgType)
{
    // 0 for site ico, 1 for project logo
    const String& imgExtension ((imgType == 0) ? "*.ico" : "*.png");
    FileChooser fc (TRANS ("Please select an image") + "...", File::nonexistent, imgExtension, true);

    if (fc.browseForFileToOpen())
    {
        const File& selectedImg (fc.getResult());

        if (imgType == 0)
        {
            if (selectedImg.getFileExtension() == ".ico"
                && selectedImg.copyFileTo (fileTreeContainer->projectFile.getSiblingFile ("site")
                                           .getChildFile ("favicon.ico")))
                SHOW_MESSAGE (TRANS ("Site ico has been set successful!"));
            else
                SHOW_MESSAGE (TRANS ("Please select a valid ico image."));
        }

        else if (imgType == 1)
        {
            if (selectedImg.getFileExtension() == ".png"
                && selectedImg.copyFileTo (fileTreeContainer->projectFile.getSiblingFile ("site/add-in")
                                           .getChildFile ("logo.png")))
            {
                SHOW_MESSAGE (TRANS ("Project logo has been set successful!"));
                editAndPreview->restartWebBrowser();
            }
            else
            {
                SHOW_MESSAGE (TRANS ("Please select a valid png image."));
            }
        }
    }
}

//=================================================================================================
void TopToolBar::cleanNeedlessMedias (const bool showMessageWhenNoAnyNeedless)
{
    // exist medias
    Array<File> allDirs;
    Array<File> allMediasOnLocal;
    FileTreeContainer::projectFile.getSiblingFile ("docs").findChildFiles (allDirs, File::findDirectories, true);

    for (int i = allDirs.size(); --i >= 0; )
    {
        // delete extra sub-dir of media (might be backup after external edited medias)
        if (allDirs[i].getParentDirectory().getFileName() == "media")
        {
            allDirs[i].setReadOnly (false, true);
            allDirs[i].deleteRecursively();
        }

        else if (allDirs[i].getFileName() != "media")
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
        if (showMessageWhenNoAnyNeedless)
            SHOW_MESSAGE (TRANS ("Your project is very neat. \nNo need to clean it up."));
    }
    else
    {
        String extraFilesName;
        const String rootPath (FileTreeContainer::projectFile.getSiblingFile ("docs").getFullPathName());

        for (int i = allMediasOnLocal.size(); --i >= 0; )
            extraFilesName += "  - " 
            + allMediasOnLocal[i].getFullPathName().fromFirstOccurrenceOf (rootPath, false, false).substring (1)
            + newLine;

        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                          TRANS ("Find ") + String (allMediasOnLocal.size()) + " "
                                          + TRANS ("redundant media-file(s):") + newLine
                                          + extraFilesName + newLine
                                          + TRANS ("Do you want to clean them up?")))
        {
            for (int i = allMediasOnLocal.size(); --i >= 0; )
            {
                const String& siteMediaPath (allMediasOnLocal[i].getFullPathName().replace ("docs", "site"));

                allMediasOnLocal[i].moveToTrash();
                File (siteMediaPath).deleteFile();
            }

            SHOW_MESSAGE (TRANS ("Needless medias cleanup successful!"));
        }
    }
}



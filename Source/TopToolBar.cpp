/*
  ==============================================================================

    TopToolBar.cpp
    Created: 4 Sep 2016 12:25:18am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

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

    bts[view]->setTooltip (TRANS ("Switch Preview / Edit Mode"));
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
    
    bts[width]->setTooltip (TRANS ("Switch Simply / Full Mode"));
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
            findInDoc (true);

            treeView.scrollToKeepItemVisible (item);
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

    TextEditor* editor = editAndPreview->getEditor();
    const String& content = editor->getText();

    int startIndex = 0;
    int caretIndex = editor->getCaretPosition ();
    //DBGX (String (caretIndex) << " / " << String (editor->getTotalNumChars ()) << " / " << content.length ());

    // find the start index of the keyword
    if (next)
        startIndex = content.indexOfIgnoreCase (caretIndex, keyword);
    else
        startIndex = content.substring (0, caretIndex - 1).lastIndexOfIgnoreCase (keyword);

    // select the keyword
    if (startIndex != -1)
        editor->setHighlightedRegion (Range<int> (startIndex, startIndex + keyword.length ()));
    else
        LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
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
		getParentComponent ()->setSize ((bts[width]->getToggleState () ? 1200 : 600), 780);
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
    m.addItem (1, TRANS ("New Project..."), true);
    m.addItem (2, TRANS ("Open Project..."), true);

    // recent files
    RecentlyOpenedFilesList recentFiles;
    recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
    PopupMenu recentFilesMenu;
    recentFiles.createPopupMenuItems (recentFilesMenu, 100, true, true);

    m.addSubMenu (TRANS ("Open Rcent"), recentFilesMenu);
    m.addItem (3, TRANS ("Close Project"), fileTreeContainer->hasLoadedProject ());
    m.addSeparator ();

	m.addItem (5, TRANS ("Regenerate All..."), fileTreeContainer->hasLoadedProject ());
	m.addItem (6, TRANS ("Clean Local Medias..."), fileTreeContainer->hasLoadedProject ());
    m.addSeparator ();

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
        //fileTreeContainer->openProject(recentFiles.getFile(index - 100));
        const File& project(recentFiles.getFile(index - 100));

        if (project != FileTreeContainer::projectFile)
            Process::openDocument(File::getSpecialLocation(File::currentApplicationFile).getFullPathName(),
                                  project.getFullPathName());
    }
    else
    {
        menuPerform(index);
    }
}

//=================================================================================================
void TopToolBar::menuPerform (const int index)
{
    // create a new project
    if (index == 1)         createNewProject ();

    // open an existed project
    else if (index == 2)    openProject ();

    // close current project
    else if (index == 3)    fileTreeContainer->closeProject ();

    // clean up and re-generate the whole site
	else if (index == 5)    cleanAndGenerateAll ();
	else if (index == 6)    cleanLocalMedias ();

    else if (index == 15)  setUiColour();
    else if (index == 16)  resetUiColour();

    // help
    else if (index == 18)   NEED_TO_DO ("Getting started...");

    // check new version
    else if (index == 19)   URL ("http://underwaySoft.com").launchInDefaultBrowser ();
	   
    // about
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

    ValueTree p ("wdtpProject");
    p.setProperty ("name", "site", nullptr);
    p.setProperty ("title", projectFile.getFileNameWithoutExtension (), nullptr);
    p.setProperty ("description", TRANS ("Description of this project..."), nullptr);
    p.setProperty ("owner", SystemStats::getLogonName() , nullptr);
    //p.setProperty ("skin", "Elegence", nullptr);
    p.setProperty ("order", 0, nullptr);
    p.setProperty ("ascending", 0, nullptr);
    p.setProperty ("dirFirst", 0, nullptr);
    p.setProperty ("showWhat", 0, nullptr);
    p.setProperty ("tooltip", 0, nullptr);
    p.setProperty ("render", "blog", nullptr);
    p.setProperty ("tplFile", "index.html", nullptr);
    p.setProperty("needCreateHtml", true, nullptr);

    // create dirs and default template files
    projectFile.getSiblingFile ("docs").createDirectory();
    projectFile.getSiblingFile ("site").createDirectory();
    projectFile.getSiblingFile ("site").getChildFile("add-in").createDirectory();
    projectFile.getSiblingFile ("themes").createDirectory();

    // TODO: create template in 'themes/..' and css/js files in 'site/add-in'

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
        jassert (addinDir.isDirectory());
        const File tempDirForAddin (FileTreeContainer::projectFile.getSiblingFile ("add-in"));
        addinDir.copyDirectoryTo (tempDirForAddin);

        // cleanup
        if (FileTreeContainer::projectFile.getSiblingFile ("site").deleteRecursively())
        {
            // generate
            generateHtmlFiles (FileTreeContainer::projectTree);
			FileTreeContainer::saveProject ();

            // restore the add-in dir
            tempDirForAddin.moveFileTo (addinDir);
            SHOW_MESSAGE (TRANS ("Site clean and regenerate successful!"));
        }
        else
        {
            tempDirForAddin.deleteRecursively();
        }
    }
}

//=================================================================================================
void TopToolBar::generateHtmlFiles (ValueTree tree)
{
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
void TopToolBar::generateHtmlFilesIfNeeded (ValueTree tree)
{
	if (tree.getType ().toString () == "doc")
	{
		if ((bool)tree.getProperty ("needCreateHtml"))
			HtmlProcessor::createArticleHtml (tree, false);
	}
	else
	{
		if ((bool)tree.getProperty ("needCreateHtml"))
		{
			HtmlProcessor::createIndexHtml (tree, false);

			for (int i = tree.getNumChildren (); --i >= 0; )
				generateHtmlFilesIfNeeded (tree.getChild (i));
		}
	}
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


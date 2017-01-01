/*
  ==============================================================================

    TopToolBar.cpp
    Created: 4 Sep 2016 12:25:18am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

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
    searchInProject->setColour (TextEditor::backgroundColourId, Colour (0xffededed));
    searchInProject->setScrollBarThickness (10);
    searchInProject->setFont (SwingUtilities::getFontSize () - 2.f);
    searchInProject->setTextToShowWhenEmpty (TRANS ("Seach in this project..."), Colour (0xffa0a0a0));
    //searchEditor->setTabKeyUsedAsCharacter(true);

    addAndMakeVisible (searchInDoc = new TextEditor ());
    searchInDoc->addListener (this);

    searchInDoc->setColour (TextEditor::textColourId, Colour (0xff303030));
    searchInDoc->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInDoc->setColour (TextEditor::backgroundColourId, Colour (0xffededed));
    searchInDoc->setScrollBarThickness (10);
    searchInDoc->setFont (SwingUtilities::getFontSize () - 2.f);
    searchInDoc->setTextToShowWhenEmpty (TRANS ("Seach in current document..."), Colour (0xffa0a0a0));
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

    bts[view]->setTooltip (TRANS ("Switch preview / edit mode"));
    bts[view]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::view_png,
                                                     BinaryData::view_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.000f, Colours::darkcyan,
                          Image::null, 1.0f, Colours::darkcyan);

    bts[system]->setTooltip (TRANS ("Popup system menu"));
    bts[system]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::system_png,
                                                       BinaryData::system_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colours::darkcyan,
                            Image::null, 1.000f, Colours::darkcyan);
}

//=======================================================================
TopToolBar::~TopToolBar ()
{
}

//=======================================================================
void TopToolBar::paint (Graphics& g)
{
    g.setColour (Colour (0x00).withAlpha (0.2f));
    g.drawLine (1.0f, getHeight () - 0.5f, getWidth () - 2.0f, getHeight () - 0.5f, 0.6f);
}

//=======================================================================
void TopToolBar::resized ()
{
    // search textEditors and find buttons
    bts[prevAll]->setBounds (12, 16, 12, 12);
    searchInProject->setBounds (bts[prevAll]->getRight () + 10, 10, 220, 25);
    bts[nextAll]->setBounds (searchInProject->getRight () + 10, 16, 12, 12);

    bts[nextPjt]->setBounds (getWidth () - 25, 16, 12, 12);
    searchInDoc->setBounds (bts[nextPjt]->getX () - 240, 10, 230, 25);
    bts[prevPjt]->setBounds (searchInDoc->getX () - 22, 16, 12, 12);

    // image buttons
    bts[view]->setTopRightPosition (getWidth() / 2 - 30, 12);
    bts[system]->setTopLeftPosition (getWidth () / 2 + 30, 12);
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

        const File& docFile (DocTreeViewItem::getFileOrDir (item->getTree()));
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
        editAndPreview->startWork (editAndPreview->getCurrentTree(), bts[view]->getToggleState());
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

    m.addItem (5, TRANS ("Regenerate All"), fileTreeContainer->hasLoadedProject ());
    m.addSeparator ();

    PopupMenu lanMenu;
    lanMenu.addItem (30, TRANS ("English"), true, String ("English") == systemFile->getValue ("language"));
    lanMenu.addItem (31, TRANS ("Chinese"), true, String ("Chinese") == systemFile->getValue ("language"));

    m.addSubMenu (TRANS ("UI Language"), lanMenu);
    m.addItem (18, TRANS ("Getting Started..."), true);
    m.addItem (19, TRANS ("Check New Version..."), true);
    m.addSeparator ();

    m.addItem (20, TRANS ("About..."), true);

    // display the menu
    const int index = m.show ();

    if (index >= 100 && index < 200)   // recently opened files..
        fileTreeContainer->openProject (recentFiles.getFile (index - 100));
    else
        menuPerform (index);
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

    // re-generate the whole site
    else if (index == 5)    NEED_TO_DO ("Regenerate the whole site...");

    // help
    else if (index == 18)   NEED_TO_DO ("Getting started...");

    // check new version
    else if (index == 19)   URL ("http://underwaySoft.com").launchInDefaultBrowser ();

    // about
    else if (index == 20)   SwingUtilities::showAbout (TRANS ("Write Down, Then Publish"), "2016");

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

    // 5-1： create and initial project file
    if (!projectFile.deleteFile ())
    {
        SHOW_MESSAGE (TRANS ("Can't overwrite this project! "));
        return;
    }

    projectFile.create ();

    ValueTree p ("wdtpProject");
    p.setProperty ("name", projectFile.getFileNameWithoutExtension (), nullptr);
    p.setProperty ("title", TRANS ("Description of this project..."), nullptr);
    p.setProperty ("owner", SystemStats::getLogonName() , nullptr);
    p.setProperty ("skin", "Elegence", nullptr);
    p.setProperty ("order", 0, nullptr);
    p.setProperty ("ascending", 0, nullptr);
    p.setProperty ("dirFirst", 0, nullptr);
    p.setProperty ("showWhat", 0, nullptr);
    p.setProperty ("tooltip", 0, nullptr);
    p.setProperty ("render", "themes/theme-1", nullptr);
    p.setProperty ("fontName", SwingUtilities::getFontName (), nullptr);
    p.setProperty ("fontSize", SwingUtilities::getFontSize (), nullptr);

    // 5-2 create dirs and default template files
    projectFile.getSiblingFile ("docs").createDirectory ();
    projectFile.getSiblingFile ("site").createDirectory ();
    projectFile.getSiblingFile ("themes").createDirectory ();

    // TODO: create template files in 'themes/theme-1'..

    // 5-3: create a index.md and initial its properties
    const File& indexFile (projectFile.getSiblingFile ("docs/index.md"));
    indexFile.create ();
    indexFile.appendText ("Hello World! ");

    // index valueTree
    ValueTree d ("doc");
    d.setProperty ("name", indexFile.getFileNameWithoutExtension (), nullptr);
    d.setProperty ("title", "Hello World", nullptr);
    d.setProperty ("keywords", String (), nullptr);
    d.setProperty ("tplFile", p.getProperty ("render").toString () + "/article.html", nullptr);
    d.setProperty ("js", String (), nullptr);

    // 5-4: add the new document's info to project file
    p.addChild (d, 0, nullptr);

    // 5-5: save the project file
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



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
    fileTree (f),
    editAndPreview (e)
{
    jassert (fileTree != nullptr);
    jassert (editAndPreview != nullptr);

    // 2 search textEditors..
    addAndMakeVisible (searchInProject = new TextEditor ());
    searchInProject->addListener (this);

    searchInProject->setColour(TextEditor::textColourId, Colour (0xff303030));
    searchInProject->setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue); 
    searchInProject->setColour(TextEditor::backgroundColourId, Colour(0xffededed));
    searchInProject->setScrollBarThickness (10);
    searchInProject->setFont (SwingUtilities::getFontSize () - 2.f);
    searchInProject->setTextToShowWhenEmpty (TRANS ("Seach in this project..."), Colour(0xffa0a0a0));
    //searchEditor->setTabKeyUsedAsCharacter(true);

    addAndMakeVisible(searchInDoc = new TextEditor());
    searchInDoc->addListener(this);

    searchInDoc->setColour(TextEditor::textColourId, Colour(0xff303030));
    searchInDoc->setColour(TextEditor::focusedOutlineColourId, Colours::lightskyblue);
    searchInDoc->setColour(TextEditor::backgroundColourId, Colour(0xffededed));
    searchInDoc->setScrollBarThickness(10);
    searchInDoc->setFont(SwingUtilities::getFontSize() - 2.f);
    searchInDoc->setTextToShowWhenEmpty(TRANS("Seach in current document..."), Colour(0xffa0a0a0));
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

    bts[add]->setTooltip (TRANS ("Create a new document inside the current folder"));
    bts[add]->setImages (false, true, true,
                         ImageCache::getFromMemory (BinaryData::add_png,
                                                    BinaryData::add_pngSize),
                         imageTrans, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00));

    bts[del]->setTooltip (TRANS ("Remove the selected document"));
    bts[del]->setImages (false, true, true,
                         ImageCache::getFromMemory (BinaryData::del_png,
                                                    BinaryData::del_pngSize),
                         imageTrans, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00));

    bts[edit]->setTooltip (TRANS ("Switch work-area to edit mode"));
    bts[edit]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::edit_png,
                                                     BinaryData::edit_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.000f, Colour (0x00),
                          Image::null, 1.000f, Colour (0x00));

    bts[img]->setTooltip (TRANS ("Insert an image to current document"));
    bts[img]->setImages (false, true, true,
                         ImageCache::getFromMemory (BinaryData::img_png,
                                                    BinaryData::icon_pngSize),
                         imageTrans, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00),
                         Image::null, 1.000f, Colour (0x00));

    bts[table]->setTooltip (TRANS ("Insert a table to current document"));
    bts[table]->setImages (false, true, true,
                           ImageCache::getFromMemory (BinaryData::table_png,
                                                      BinaryData::table_pngSize),
                           imageTrans, Colour (0x00),
                           Image::null, 1.000f, Colour (0x00),
                           Image::null, 1.000f, Colour (0x00));

    bts[view]->setTooltip (TRANS ("Switch work-area to preview mode"));
    bts[view]->setImages (false, true, true,
                          ImageCache::getFromMemory (BinaryData::view_png,
                                                     BinaryData::view_pngSize),
                          imageTrans, Colour (0x00),
                          Image::null, 1.000f, Colour (0x00),
                          Image::null, 1.000f, Colour (0x00));

    bts[generate]->setTooltip (TRANS ("Generate all associated web-pages locally"));
    bts[generate]->setImages (false, true, true,
                              ImageCache::getFromMemory (BinaryData::generate_png,
                                                         BinaryData::generate_pngSize),
                              imageTrans, Colour (0x00),
                              Image::null, 1.000f, Colour (0x00),
                              Image::null, 1.000f, Colour (0x00));

    bts[upload]->setTooltip (TRANS ("Upload all modified web-pages to host"));
    bts[upload]->setImages (false, true, true,
                              ImageCache::getFromMemory (BinaryData::upload_png,
                                                         BinaryData::upload_pngSize),
                              imageTrans, Colour (0x00),
                              Image::null, 1.000f, Colour (0x00),
                              Image::null, 1.000f, Colour (0x00));

    bts[system]->setTooltip (TRANS ("Popup system menu"));
    bts[system]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::system_png,
                                                       BinaryData::system_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colour (0x00),
                            Image::null, 1.000f, Colour (0x00));

    bts[mdHelp]->setTooltip (TRANS ("Review Markdown concise usage"));
    bts[mdHelp]->setImages (false, true, true,
                            ImageCache::getFromMemory (BinaryData::mdhelp_png,
                                                       BinaryData::mdhelp_pngSize),
                            imageTrans, Colour (0x00),
                            Image::null, 1.000f, Colour (0x00),
                            Image::null, 1.000f, Colour (0x00));
}

//=======================================================================
TopToolBar::~TopToolBar()
{
}

//=======================================================================
void TopToolBar::paint (Graphics& g)
{
    g.setColour (Colour(0x00).withAlpha(0.2f));
    g.drawLine (1.0f, getHeight() - 0.5f, getWidth() - 2.0f, getHeight() - 0.5f, 0.6f);

    // indicate the middle of this component, so that convenient for place imageButtons
    g.drawVerticalLine(getWidth() / 2, 5, 40.f);
}

//=======================================================================
void TopToolBar::resized()
{
    // 2 search textEditors
    searchInProject->setBounds(20, 10, 230, 25);
    searchInDoc->setBounds(getWidth() - 250, 10, 230, 25);

    // image buttons
    int x = getWidth() / 2 - 235;

    for (int i = 0; i < totalBts; ++i)
        bts[i]->setTopLeftPosition (x + i * 50, 12);
}

//=========================================================================
void TopToolBar::textEditorReturnKeyPressed (TextEditor& te)
{
}

void TopToolBar::textEditorEscapeKeyPressed (TextEditor& te)
{
}

//=========================================================================
void TopToolBar::buttonClicked (Button* bt)
{
    if (bt == bts[system])
        popupSystemMenu();
}

//=================================================================================================
void TopToolBar::popupSystemMenu()
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
    m.addSeparator ();

    m.addItem (3, TRANS ("Close Project"), fileTree->hasLoadedProject());
    m.addItem (4, TRANS ("Save As..."), fileTree->hasLoadedProject ());
    m.addItem (5, TRANS ("Clean Up..."), fileTree->hasLoadedProject ());
    m.addSeparator ();

    m.addItem (18, TRANS ("How To..."), true);
    m.addItem (19, TRANS ("Check New Version..."), true);
    m.addItem (20, TRANS ("About..."), true);
    
    // display the menu
    const int index = m.show ();

    if (index >= 100 && index < 200)   // recently opened files..
        fileTree->openProject (recentFiles.getFile (index - 100));
    else
        menuPerform (index);
}

//=================================================================================================
void TopToolBar::menuPerform (const int index)
{
    if (index == 1)  // create a new project
        createNewProject ();
    else if (index == 2)  // open an existed project
        openProject ();
    else if (index == 3)  // close current project
        fileTree->closeProject ();
    else if (index == 18)  // how to
        NEED_TO_DO ("How to...");
    else if (index == 19)  // check new version
        URL ("http://underwaySoft.com").launchInDefaultBrowser ();
    else if (index == 20)  // about
        SwingUtilities::showAbout (TRANS ("Write Down, Then Publish"), "2016");
    
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
    if (projectFile.existsAsFile () && !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, 
        TRANS ("Message"), TRANS ("This project already exists, want to overwrite it?")))
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
    p.setProperty ("desc", TRANS ("Description of this project..."), nullptr);
    p.setProperty ("owner", String(), nullptr);
    p.setProperty ("skin", "Elegence", nullptr);
    p.setProperty ("order", 0, nullptr);
    p.setProperty ("render", "themes/theme-1", nullptr);
    p.setProperty ("place", "site", nullptr);
    p.setProperty ("ftpAddress", String (), nullptr);
    p.setProperty ("ftpPort", "21", nullptr);
    p.setProperty ("ftpUserName", String (), nullptr);
    p.setProperty ("ftpPassword", String (), nullptr);

    // 5-2 create dirs and default template files
    projectFile.getSiblingFile ("docs").createDirectory ();
    projectFile.getSiblingFile ("site").createDirectory ();
    projectFile.getSiblingFile ("themes").createDirectory ();

    // TODO: create template files in 'themes/theme-1'..

    // 5-3: create a index.md and initial its properties
    const File& docFile (projectFile.getSiblingFile ("docs/index.md"));
    docFile.create ();
    docFile.appendText ("Hello World! ");

    // doc valueTree
    ValueTree d ("doc");
    d.setProperty ("name", docFile.getFileNameWithoutExtension (), nullptr);
    d.setProperty ("title", docFile.getFileNameWithoutExtension (), nullptr);
    d.setProperty ("author", p.getProperty ("owner").toString (), nullptr);
    d.setProperty ("publish", true, nullptr);
    d.setProperty ("webName", docFile.getFileNameWithoutExtension (), nullptr);
    d.setProperty ("tplFile", p.getProperty ("render").toString () + "/article.html", nullptr);
    d.setProperty ("js", String (), nullptr);

    // 5-4: add the new document's info to project file
    p.addChild (d, 0, nullptr);

    // 5-5: save the project file
    if (SwingUtilities::writeValueTreeToFile (p, projectFile))
        fileTree->openProject (projectFile); // load the new project file
    else
        SHOW_MESSAGE (TRANS ("Something wrong during create this project file."));
}

//=================================================================================================
void TopToolBar::openProject ()
{
    FileChooser fc (TRANS ("Open Project..."), File::nonexistent, "*.wdtp", false);

    if (fc.browseForFileToOpen ())
        fileTree->openProject (fc.getResult ());
}


/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

//==============================================================================
EditAndPreview::EditAndPreview()
{
    // web browser
    addChildComponent (webView = new WebBrowserComponent ());

    // stretched layout, arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.69);  // editor，
    layoutManager.setItemLayout (1, 3, 3, 3);           // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.31);  // propertiesPanel

    addAndMakeVisible (editor = new EditorForMd (docFile));
    addAndMakeVisible (setupPanel = new SetupPanel ());
    addAndMakeVisible (layoutBar = new StretchableLayoutResizerBar (&layoutManager, 1, true));

    // editor
    editor->setMultiLine (true);
    editor->setReturnKeyStartsNewLine (true);
    editor->setTabKeyUsedAsCharacter (true);
    editor->setColour (TextEditor::textColourId, Colour(0xff303030));
    editor->setColour (TextEditor::focusedOutlineColourId, Colour(0xffb4b4b4));
    editor->setColour (TextEditor::backgroundColourId, Colour(0xffededed));
    editor->setScrollBarThickness (10);
    editor->setIndents (10, 10);
    editor->setFont (SwingUtilities::getFontSize());
    editor->setEnabled (false);
    editor->setBorder (BorderSize<int>(1, 1, 1, 1));
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
    stopTimer();
}

//=========================================================================
void EditAndPreview::resized()
{
    // layout
    Component* comps[] = { (webView->isVisible ()) ? 
        static_cast<Component*>(webView) : static_cast<Component*>(editor), 
        layoutBar, setupPanel };

    layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
}

//=================================================================================================
void EditAndPreview::editNewDoc (const File& file)
{
    if (file.existsAsFile())
    {
        saveCurrentDocIfChanged ();
        webView->setVisible (false);

        editor->removeListener (this);
        editor->clear ();
        editor->setEnabled (true);

        docFile = file;
        currentContent = docFile.loadFileAsString ();

        editor->setText (currentContent, false);
        editor->grabKeyboardFocus ();
        editor->moveCaretToEnd (false);

        resized ();
        editor->addListener (this);
    }
    else
    {
        whenFileOrDirNonexists();
        SHOW_MESSAGE (TRANS ("The file") + ": \"" + file.getFullPathName() 
                      + "\"\n" + TRANS ("is missed or demaged."));
    }
}

//=================================================================================================
void EditAndPreview::previewDoc (const File& file)
{
    if (file.existsAsFile ())
    {
        saveCurrentDocIfChanged ();

        editor->setEnabled (false);
        webView->setVisible (true);
        docFile = file;

        //webView->goToURL (docFile.getFullPathName ());
        webView->goToURL ("e:/temp/test.html");
        resized ();
    }
    else
    {
        whenFileOrDirNonexists();
        SHOW_MESSAGE (TRANS ("The file") + ": \"" + file.getFullPathName ()
                      + "\"\n" + TRANS ("is missed or demaged."));
    }
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    saveCurrentDocIfChanged();
    editorInitial ();

    setupPanel->projectClosed ();
    webView->setVisible (false);
}

//=================================================================================================
void EditAndPreview::setSystemProperties()
{
    if (!setupPanel->systemPropertiesIsShowing())
        setupPanel->showSystemProperties();
}

//=================================================================================================
void EditAndPreview::setProjectProperties (ValueTree& projectTree)
{
    saveCurrentDocIfChanged();
    editorInitial();
    setupPanel->showProjectProperties (projectTree);
}

//=================================================================================================
void EditAndPreview::setDirProperties (ValueTree& dirTree)
{
    saveCurrentDocIfChanged ();
    editorInitial();
    setupPanel->showDirProperties (dirTree);
}

//=================================================================================================
void EditAndPreview::setDocProperties (ValueTree& docTree)
{
    setupPanel->showDocProperties (docTree);
}

//=================================================================================================
void EditAndPreview::whenFileOrDirNonexists()
{
    editorInitial();
    setupPanel->showSystemProperties();
}

//=================================================================================================
void EditAndPreview::editorInitial ()
{
    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (false);

    docFile = File::nonexistent;
    docHasChanged = false;
    currentContent = String ();
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when load a doc, so use this ugly judge...
    if (currentContent.compareNatural(editor->getText()) != 0)
    {
        docHasChanged = true;
        startTimer (5000);
    }    
}

//=================================================================================================
void EditAndPreview::timerCallback ()
{
    saveCurrentDocIfChanged();
}

//=================================================================================================
const bool EditAndPreview::saveCurrentDocIfChanged ()
{
    /*static int i = 0;
    DBGX (++i);*/

    stopTimer ();

    if (docHasChanged && docFile != File::nonexistent)
    {
        const String& s (editor->getText());
        currentContent = s;

        TemporaryFile tempFile (docFile);
        tempFile.getFile ().appendText (s);

        if (tempFile.overwriteTargetFileWithTemporary ())
        {
            docHasChanged = false;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

//=================================================================================================
void EditorForMd::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    if (e->mods.isPopupMenu())
    {
        PopupMenu insertMenu;
        insertMenu.addItem (1, TRANS ("Image"));
        insertMenu.addItem (2, TRANS ("Hyperlink"));
        insertMenu.addSeparator();

        insertMenu.addItem (3, TRANS ("Table") + " (2x2)");
        insertMenu.addItem (4, TRANS ("Table") + " (3x3)");
        insertMenu.addSeparator ();

        insertMenu.addItem (5, TRANS ("Code Block"));
        insertMenu.addItem (6, TRANS ("Reference"));
        insertMenu.addSeparator ();

        // list and title
        insertMenu.addItem (7, TRANS ("Unordered List"));
        insertMenu.addItem (8, TRANS ("Ordered List"));
        insertMenu.addSeparator ();

        insertMenu.addItem (9, TRANS ("Secondary Heading"));
        insertMenu.addItem (10, TRANS ("Tertiary Heading"));
        insertMenu.addSeparator ();

        insertMenu.addItem (11, TRANS ("Separator"));
        insertMenu.addItem (12, TRANS ("Date and Time"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile());

        PopupMenu formatMenu;
        formatMenu.addItem (30, TRANS ("Bold"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (31, TRANS ("Italic"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (32, TRANS ("Bold + Italic"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (33, TRANS ("Code Inline"), getHighlightedText().isNotEmpty());

        menu.addSubMenu (TRANS ("Format"), formatMenu, docFile.existsAsFile());
        menu.addSeparator ();

        TextEditor::addPopupMenuItems (menu, e);
    }
}

//=================================================================================================
void EditorForMd::performPopupMenuAction (int index)
{
    String content;

    if (1 == index) // image
    {
        FileChooser fc (TRANS ("Select Images..."), File::nonexistent, "*.jpg;*.png;*.gif", false);
        Array<File> imageFiles;

        if (!fc.browseForMultipleFilesToOpen())
            return;

        imageFiles = fc.getResults();
        const File imgPath (docFile.getSiblingFile ("media"));

        for (auto f : imageFiles)
        {
            const File targetFile (imgPath.getChildFile (f.getFileName()).getNonexistentSibling (true));
            targetFile.create();

            if (f.copyFileTo (targetFile))
                content << newLine 
                << "![](media/" << targetFile.getFileName() << ")" << newLine
                << newLine;
            else
                SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName());
        }
    }
    else if (2 == index) // hyperlink
    {
        AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                            AlertWindow::InfoIcon);

        dialog.addTextEditor ("name", "http://");
        dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

        if (0 == dialog.runModalLoop ())
        {
            const String inputStr (dialog.getTextEditor ("name")->getText().trim());
            content << " [](" << inputStr << ") ";
        }
        else
        {
            return;
        }
    }    
    else if (3 == index) // table 2*2
    {
        content << newLine
            << "|  |  |" << newLine
            << "| --: | :-- |" << newLine
            << "|  |  |" << newLine
            << "|  |  |" << newLine
            << newLine;
    }
    else if (4 == index) // table 3*3
    {
        content << newLine
            << "|  |  |  |" << newLine
            << "| --: | :--: | :-- |" << newLine
            << "|  |  |  |" << newLine
            << "|  |  |  |" << newLine
            << "|  |  |  |" << newLine
            << newLine;        
    }    
    else if (5 == index) // code
    {
        content << newLine
            << "``` c++" << newLine 
            << newLine
            << "```"  << newLine;
    }   
    else if (6 == index) // reference
    {
        content << newLine << "> ";
    }
    else if (7 == index)  // unordered list
    {
        content << newLine 
            << "- " << newLine
            << "- " << newLine
            << "- " << newLine;
    }
    else if (8 == index)  // ordered list
    {
        content << newLine
            << "1. " << newLine
            << "2. " << newLine
            << "3. " << newLine;
    }
    else if (9 == index)  // second heading
    {
        content << newLine << "## ";
    }
    else if (10 == index) // third heading
    {
        content << newLine << "### ";
    }
    else if (11 == index) // separator
    {
        content << newLine << "----" << newLine << newLine;
    }
    else if (12 == index) // date and time
    {
        content << newLine << newLine <<
            SwingUtilities::getTimeStringWithSeparator(SwingUtilities::getCurrentTimeString())
            .dropLastCharacters(3);
    }
    else if (30 == index) // bold
    {
        content << " **" << getHighlightedText() << "** ";
    }
    else if (31 == index) // italic
    {
        content << " *" << getHighlightedText() << "* ";
    }
    else if (32 == index) // bold + italic
    {
        content << " ***" << getHighlightedText() << "*** ";
    }
    else if (33 == index) // code inline
    {
        content << "`" << getHighlightedText() << "`";
    }
    else
    {
        TextEditor::performPopupMenuAction (index);
        return;
    }
    
    insertTextAtCaret (content);

    // move up the currsor...
    if (5 == index)  
    {
        moveCaretUp (false);
        moveCaretUp (false);
    }
    else if (7 == index || 8 == index)
    {
        moveCaretUp (false);
        moveCaretUp (false);
        moveCaretUp (false);
        moveCaretToEndOfLine (false);
    }
}

//=================================================================================================
bool EditorForMd::keyPressed (const KeyPress& key)
{
    if (key == KeyPress(KeyPress::tabKey))
    {
        insertTextAtCaret ("    ");
        return true;
    }

    return TextEditor::keyPressed (key);
}

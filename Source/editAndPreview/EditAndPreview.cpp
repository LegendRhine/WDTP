/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"
#include <fstream>

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
    addAndMakeVisible (setupPanel = new SetupPanel (this));
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
    webView = nullptr;
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
void EditAndPreview::editNewDoc (const ValueTree& docTree_)
{
    saveCurrentDocIfChanged();
    webView->setVisible (false);

    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (true);

    const File& file (DocTreeViewItem::getFileOrDir (docTree_));

    if (file.existsAsFile())
    {
        docFile = file;
        docTree = docTree_;

        editor->applyFontToAllText (FileTreeContainer::fontSize);
        editor->setText (docFile.loadFileAsString(), false);
        currentContent = editor->getText();
        editor->grabKeyboardFocus ();
        editor->addListener (this);
    }   
    else
    {
        editorAndWebInitial();
    }

    resized ();
}

//=================================================================================================
void EditAndPreview::previewDoc (const ValueTree& docTree_)
{
    saveCurrentDocIfChanged();
    editor->setEnabled (false);
          
    docFile = DocTreeViewItem::getFileOrDir (docTree_);
    docTree = docTree_;
    currentContent = docFile.loadFileAsString();

    // must create a new webBrowserComponent, 
    // otherwise createMatchedHtmlFile() can't delete the html file    
    //addAndMakeVisible (webView = new WebBrowserComponent ());    
    webView->setVisible (true);
    webView->stop ();
    webView->goToURL (createMatchedHtmlFile ().getFullPathName ());
    resized ();
}

//=================================================================================================
const File EditAndPreview::createMatchedHtmlFile ()
{
    jassert (FileTreeContainer::projectTree.isValid());

    const String docPath (docFile.withFileExtension("html").getFullPathName());
    File htmlFile (docPath.replace("docs", FileTreeContainer::projectTree.getProperty("place").toString()));

    if (needCreateHtml || !htmlFile.existsAsFile())
    {   
        if (htmlFile.deleteFile())
        {
            htmlFile.create ();
            htmlFile.appendText (Md2Html::mdStringToHtml (currentContent, docTree));
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create html file."));
        }        
    }

    needCreateHtml = false;
    return htmlFile;
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    saveCurrentDocIfChanged();
    editorAndWebInitial ();

    setupPanel->projectClosed ();
    webView->setVisible (false);
}

//=================================================================================================
void EditAndPreview::setProjectProperties (ValueTree& projectTree)
{
    setupPanel->showProjectProperties (projectTree);
}

//=================================================================================================
void EditAndPreview::setDirProperties (ValueTree& dirTree)
{
    setupPanel->showDirProperties (dirTree);
}

//=================================================================================================
void EditAndPreview::setDocProperties (ValueTree& docTree_)
{
    setupPanel->showDocProperties (docTree_);
}

//=================================================================================================
void EditAndPreview::editorAndWebInitial ()
{
    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (false);

    docFile = File::nonexistent;
    docHasChanged = false;
    currentContent = String();
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when load a doc, so use this ugly judge...
    if (currentContent.compare (editor->getText()) != 0)
    {
        docHasChanged = true;
        needCreateHtml = true;
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
        currentContent = editor->getText ();
        const String tileStr (currentContent.upToFirstOccurrenceOf ("\n", false, true)
                              .replace("#", String()).trim());

        TemporaryFile tempFile (docFile);
        tempFile.getFile ().appendText (currentContent);

        if (tempFile.overwriteTargetFileWithTemporary())
        {
            docTree.setProperty ("title", tileStr, nullptr);
            docHasChanged = false;

            return SwingUtilities::writeValueTreeToFile (FileTreeContainer::projectTree, 
                                                         FileTreeContainer::projectFile);
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

        insertMenu.addItem (5, TRANS ("Align Center"));
        insertMenu.addItem (6, TRANS ("Align Right"));
        insertMenu.addSeparator ();

        insertMenu.addItem (7, TRANS ("Unordered List"));
        insertMenu.addItem (8, TRANS ("Ordered List"));
        insertMenu.addSeparator ();

        insertMenu.addItem (9, TRANS ("Primary Heading"));
        insertMenu.addItem (10, TRANS ("Secondary Heading"));
        insertMenu.addItem (11, TRANS ("Tertiary Heading"));
        insertMenu.addSeparator ();

        insertMenu.addItem (14, TRANS ("Separator"));
        insertMenu.addItem (15, TRANS ("Author and Date"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile());

        PopupMenu formatMenu;
        formatMenu.addItem (30, TRANS ("Bold"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (31, TRANS ("Italic"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (32, TRANS ("Code Block"));
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
                << "![ ](media/" << targetFile.getFileName() << ")" << newLine
                << newLine;
            else
                SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName());
        }
    }
    else if (2 == index) // hyperlink
    {
        AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                            AlertWindow::InfoIcon);

        dialog.addTextEditor ("name", String());
        dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

        if (0 == dialog.runModalLoop ())
        {
            const String inputStr (dialog.getTextEditor ("name")->getText().trim());
            content << "[" << inputStr << "](" << inputStr << ") ";
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
    else if (5 == index) // align center
    {
        content << newLine << "* ";
    }   
    else if (6 == index) // align right
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
    else if (8 == index)  // ordered list. it'll parse as "1. 2. 3." etc.
    {
        content << newLine
            << "+ " << newLine
            << "+ " << newLine
            << "+ " << newLine;
    }
    else if (9 == index)  // second heading
    {
        content << newLine << "# ";
    }
    else if (10 == index)  // second heading
    {
        content << newLine << "## ";
    }
    else if (11 == index) // third heading
    {
        content << newLine << "### ";
    }
    else if (14 == index) // separator
    {
        content << newLine << "---" << newLine << newLine;
    }
    else if (15 == index) // author and date
    {
        content << newLine << newLine 
            << "> "
            << TRANS("Author: ")
            << FileTreeContainer::projectTree.getProperty("owner").toString() 
            << " " << newLine << "> "
            << SwingUtilities::getTimeStringWithSeparator(SwingUtilities::getCurrentTimeString()).dropLastCharacters(9)
            << " ";
        
    }
    else if (30 == index) // bold
    {
        content << "**" << getHighlightedText() << "**";
    }
    else if (31 == index) // italic
    {
        content << "*" << getHighlightedText() << "*";
    }
    else if (32 == index) // bold + italic
    {
        content << newLine
            << "```" << newLine
            << getHighlightedText () << newLine
            << "```" << newLine;
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
    if (7 == index || 8 == index)
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

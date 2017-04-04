/*
  ==============================================================================

    ThemeEditor.cpp
    Created: 3 Apr 2017 10:57:50am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern ApplicationCommandManager* cmdManager;

//==============================================================================
ThemeEditor::ThemeEditor (EditAndPreview* parent) :
    editAndPreview (parent)
{
    setFont (SwingUtilities::getFontSize());    
    setMultiLine (true, true);
    setReturnKeyStartsNewLine (true);
    setTabKeyUsedAsCharacter (true);

    setColour (TextEditor::focusedOutlineColourId, Colour (0x000));
    setColour (TextEditor::textColourId, Colour (0xffd7d079));
    setColour (CaretComponent::caretColourId, Colour (0xffd7d079).withAlpha (0.6f));
    setColour (TextEditor::backgroundColourId, Colour (0xff202020));
    setColour (TextEditor::highlightColourId, Colours::lightskyblue);
}

//=================================================================================================
ThemeEditor::~ThemeEditor()
{
}

//=================================================================================================
void ThemeEditor::setFileToEdit (const File& file)
{
    currentFile = file;
    jassert (currentFile.existsAsFile());
    setText (currentFile.loadFileAsString(), false);
}

//=================================================================================================
void ThemeEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const bool fileExists = currentFile.existsAsFile();

    if (e->mods.isPopupMenu() && fileExists)
    {
        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator();

        menu.addItem (applyIndex, TRANS ("Save and Apply"));
        menu.addItem (closeIndex, TRANS ("Close without Save"));
        menu.addItem (saveAsIndex, TRANS ("Overwrite and Save to") + "...");

        /*menu.addSeparator();
        menu.addItem (autoReturn, TRANS ("Line Wrap"), true, isWordWrap());*/
    }
}

//=================================================================================================
void ThemeEditor::performPopupMenuAction (int index)
{
    if (applyIndex == index)
    {
        currentFile.replaceWithText (getText());
        editAndPreview->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        cmdManager->invokeDirectly (TopToolBar::MenuAndCmdIndex::generateCurrent, false);
        editAndPreview->forcePreview();
    }

    else if (closeIndex == index)
    {
        exitEditMode();
    }
    
    else if (saveAsIndex == index)
    {
        FileChooser fc (TRANS ("Save As") + "...", File(), "*", true);

        if (fc.browseForDirectory())
        {
            const File& file (fc.getResult().getChildFile (currentFile.getFileName()));

            file.deleteFile();
            file.create();
            currentFile.copyFileTo (file);
        }
    }

    /*else if (autoReturn == index)
    {
        setMultiLine (true, !isWordWrap());
    }*/
}

//=================================================================================================
void ThemeEditor::exitEditMode()
{
    setText (String(), false);
    currentFile == File();
    editAndPreview->setLayout (true, true);
}

//=================================================================================================
bool ThemeEditor::keyPressed (const KeyPress& key)
{
    // English punctuation matching...
    if (key == KeyPress ('\'', ModifierKeys::shiftModifier, 0)) // "..."
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\"" + selectedStr + "\"");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('\'')) // '...'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\'" + selectedStr + "\'");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }
    
    else if (key == KeyPress ('[', ModifierKeys::shiftModifier, 0)) // '{}'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("{" + selectedStr + "}");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('9', ModifierKeys::shiftModifier, 0)) // '()'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("(" + selectedStr + ")");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress (',', ModifierKeys::shiftModifier, 0)) // '<>'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("<" + selectedStr + ">");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    return TextEditor::keyPressed (key);
}



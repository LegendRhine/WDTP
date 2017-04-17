/*
  ==============================================================================

    SwingEditor.cpp
    Created: 18 Apr 2017 3:33:32am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "SwingEditor.h"


SwingEditor::SwingEditor()
{
    // mark for dragging position of the selected text
    draggingPosition.setFill (Colours::darkgrey);
    addAndMakeVisible (draggingPosition);
}

//=================================================================================================
void SwingEditor::setCaretColourWhenDragging (const Colour& caretColourWhenDragging)
{
    draggingPosition.setFill (caretColourWhenDragging);
}

//=================================================================================================
void SwingEditor::mouseDown (const MouseEvent& e)
{
    if (getHighlightedText().isNotEmpty()
        && getHighlightedRegion().contains (getTextIndexAt (e.x, e.y))
        && !e.mods.isPopupMenu())
    {
        draggingSelected = true;
        yOfViewportWhenDragging = 0;
    }

    else
    {
        TextEditor::mouseDown (e);
    }
}

//=================================================================================================
void SwingEditor::mouseDrag (const MouseEvent& e)
{
    if (draggingSelected)
    {
        setCaretVisible (false);
        setMouseCursor (e.mods.isCommandDown() ? MouseCursor::CopyingCursor
                        : MouseCursor::NormalCursor);

        float cursorX, cursorY;
        float cursorHeight = getFont().getHeight();

        getCharPosition (getTextIndexAt (e.x, e.y), cursorX, cursorY, cursorHeight);
        Rectangle<float> pos (cursorX + 10.f, cursorY + 12.f - getViewport()->getViewPositionY(),
                              2.5f, cursorHeight);

        draggingPosition.setRectangle (pos);

        if (getViewport()->autoScroll (e.x, e.y, 50, 20))
            yOfViewportWhenDragging = getViewport()->getViewPositionY();
    }

    else
    {
        TextEditor::mouseDrag (e);
    }
}

//=================================================================================================
void SwingEditor::mouseUp (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        TextEditor::mouseUp (e);
        return;
    }

    if (draggingSelected
        && !getHighlightedRegion().contains (getTextIndexAt (e.x, e.y)))
    {
        SystemClipboard::copyTextToClipboard (getHighlightedText());

        if (!e.mods.isCommandDown())  // clear the highlight selected
            insertTextAtCaret (String());

        if (yOfViewportWhenDragging != 0)
            getViewport()->setViewPosition (0, yOfViewportWhenDragging);

        setCaretPosition (getTextIndexAt (e.x, e.y));
        insertTextAtCaret (SystemClipboard::getTextFromClipboard());
    }

    else
    {
        TextEditor::mouseUp (e);
        const int caretPos = getCaretPosition();

        if (caretPos != getHighlightedRegion().getStart()
            && getHighlightedRegion().contains (getTextIndexAt (e.x, e.y)))
            setCaretPosition (getCaretPosition());
    }

    setMouseCursor (MouseCursor::IBeamCursor);
    setCaretVisible (true);

    draggingSelected = false;
    yOfViewportWhenDragging = 0;
    draggingPosition.setRectangle (Rectangle<float> (0, 0, 0, 0));
}

//=================================================================================================
void SwingEditor::mouseMove (const MouseEvent& e)
{
    if (getHighlightedText().isNotEmpty()
        && getHighlightedRegion().contains (getTextIndexAt (e.x - 5, e.y)))
        setMouseCursor (MouseCursor::NormalCursor);

    else
        setMouseCursor (MouseCursor::IBeamCursor);
}

//=================================================================================================
void SwingEditor::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& d)
{
    TextEditor::mouseWheelMove (e, d);

    if (draggingSelected && getHighlightedText().isNotEmpty())
        yOfViewportWhenDragging = getViewport()->getViewPositionY();
}

//=================================================================================================
bool SwingEditor::keyPressed (const KeyPress& key)
{
    // for Chinese punctuation matching
    if (key == KeyPress::deleteKey || key == KeyPress::backspaceKey)
    {
        PopupMenu::dismissAllActiveMenus();
        delPressed = true;
    }

    return TextEditor::keyPressed (key);
}

//=================================================================================================
void SwingEditor::insertTextAtCaret (const String& textToInsert)
{
    const String& selectedStr (getHighlightedText());
    bool sthSelected = selectedStr.isNotEmpty();

    TextEditor::insertTextAtCaret (textToInsert);
    //DBGX (selectedStr + " - " + textToInsert);

    // when IME enabled, this method will be called twice and the second is empty highlighted
    if (selectedStr.isNotEmpty())
        selectedForCnPunc = selectedStr;

    // it'll select a char when del and backspace then highlighted will be effected. 
    // see keyPressed() about the judge of delete and backspace key
    if (delPressed)
        selectedForCnPunc.clear();

    // ascii punctuation matching
    if (textToInsert == "\"" || textToInsert == "\'")
    {
        TextEditor::insertTextAtCaret (selectedStr + textToInsert);
        if (!sthSelected)   moveCaretLeft (false, false);
    }
    else if (textToInsert == "[")
    {
        TextEditor::insertTextAtCaret (selectedStr + "]");
        if (!sthSelected)   moveCaretLeft (false, false);
    }

    else if (textToInsert == "(")
    {
        TextEditor::insertTextAtCaret (selectedStr + ")");
        if (!sthSelected)   moveCaretLeft (false, false);
    }
    else if (textToInsert == "<")
    {
        TextEditor::insertTextAtCaret (selectedStr + ">");
        if (!sthSelected)   moveCaretLeft (false, false);
    }

    // markup: `, ~, *
    else if (sthSelected && textToInsert == "`")
    {
        TextEditor::insertTextAtCaret (selectedStr + textToInsert);
    }
    else if (sthSelected && textToInsert == "~")
    {
        TextEditor::insertTextAtCaret (textToInsert + selectedStr + textToInsert + textToInsert);
    }
    else if (sthSelected && textToInsert == "*")
    {
        TextEditor::insertTextAtCaret (selectedStr + textToInsert);
        setHighlightedRegion (Range<int> (getCaretPosition() - selectedStr.length() - 1, getCaretPosition() - 1));
    }

    // chinese punctuation matching and popup tips
    else if (textToInsert.isNotEmpty())
    {
        startTimer (30);
    }

    delPressed = false;
}

//=================================================================================================
void SwingEditor::timerCallback()
{
    stopTimer();
    const Range<int> lastPosition (getCaretPosition() - 1, getCaretPosition());
    const String& lastChar (getTextInRange (lastPosition));
    bool puncMatched = false;
    //DBGX (lastChar);

    if (lastChar == CharPointer_UTF8 ("\xe2\x80\x9c")) // left "
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + String (CharPointer_UTF8 ("\xe2\x80\x9d")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x9d")) // right "
    {
        puncMatched = true;
        moveCaretLeft (false, true);
        TextEditor::insertTextAtCaret (String (CharPointer_UTF8 ("\xe2\x80\x9c")) + selectedForCnPunc
                                       + String (CharPointer_UTF8 ("\xe2\x80\x9d")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x98")) // left '
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + String (CharPointer_UTF8 ("\xe2\x80\x99")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x99")) // right '
    {
        puncMatched = true;
        moveCaretLeft (false, true);
        TextEditor::insertTextAtCaret (String (CharPointer_UTF8 ("\xe2\x80\x98")) + selectedForCnPunc
                                       + String (CharPointer_UTF8 ("\xe2\x80\x99")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe3\x80\x90")) // [
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + String (CharPointer_UTF8 ("\xe3\x80\x91")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe3\x80\x8a")) // <<
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + String (CharPointer_UTF8 ("\xe3\x80\x8b")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xef\xbc\x88")) // (
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + String (CharPointer_UTF8 ("\xef\xbc\x89")));
    }

    else if (lastChar == "{") // {
    {
        puncMatched = true;
        TextEditor::insertTextAtCaret (selectedForCnPunc + "}");
    }

    if (puncMatched && selectedForCnPunc.isEmpty())
        moveCaretLeft (false, false);

    selectedForCnPunc.clear();
}


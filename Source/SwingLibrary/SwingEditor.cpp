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
    draggingPosition.setFill (Colours::black.withAlpha (0.6f));
    addAndMakeVisible (draggingPosition);

    setScrollBarThickness (10);
    setIndents (8, 8);
    setBorder (BorderSize<int> (1, 1, 1, 1));
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
        Rectangle<float> pos (cursorX + 8, cursorY + 10 - getViewport()->getViewPositionY(),
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

    if (draggingSelected && !getHighlightedRegion().contains (getTextIndexAt (e.x, e.y)))
    {
        const String& draggingContent (getHighlightedText());
        const int dropPosition = getTextIndexAt (e.x, e.y);
        int removeNumbers = 0;        

        // clear the highlight selected and get the offset of drop position
        if (!e.mods.isCommandDown())
        {
            insertTextAtCaret (String());

            if (dropPosition > getHighlightedRegion().getEnd())
                removeNumbers = draggingContent.length();
        }

        if (yOfViewportWhenDragging > 0)
            getViewport()->setViewPosition (0, yOfViewportWhenDragging);

        setCaretPosition (dropPosition - removeNumbers);
        insertTextAtCaret (draggingContent);
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

    // tab
    else if (key == KeyPress (KeyPress::tabKey))
    {
        tabKeyInput();
        return true;
    }

    // shift + tab (anti-indent)
    else if (key == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0))
    {
        shiftTabInput();
        return true;
    }

    return TextEditor::keyPressed (key);
}

//=================================================================================================
void SwingEditor::tabKeyInput()
{
    if (getHighlightedText().isEmpty())
    {
        const int position = getCaretPosition();
        String content ("    ");
        moveCaretUp (false);

        while (getCaretPosition() - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) != "\n")
        {
            moveCaretUp (false);
        }

        if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "- ")
            content += "- ";

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "+ ")
            content += "+ ";

        setCaretPosition (position);
        TextEditor::insertTextAtCaret (content);
    }
    else
    {
        StringArray content;
        content.addLines (getHighlightedText());

        for (int i = content.size(); --i >= 0; )
            content.getReference (i) = "    " + content.getReference (i);

        TextEditor::insertTextAtCaret (content.joinIntoString (newLine));
        moveCaretRight (false, false);
    }
}

//=================================================================================================
void SwingEditor::shiftTabInput()
{
    if (getHighlightedText().isEmpty())
    {
        moveCaretToStartOfLine (false);

        if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 4)) == "    ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 4));

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 3)) == "   ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 3));

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "  ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 2));

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 1)) == " ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 1));

        TextEditor::insertTextAtCaret (String());
    }
    else // let the selected anti-indent
    {
        StringArray content;
        content.addLines (getHighlightedText());

        for (int i = content.size(); --i >= 0; )
        {
            if (content[i].substring (0, 4) == "    ")
                content.getReference (i) = content[i].substring (4);

            else if (content[i].substring (0, 3) == "   ")
                content.getReference (i) = content[i].substring (3);

            else if (content[i].substring (0, 2) == "  ")
                content.getReference (i) = content[i].substring (2);

            else if (content[i].substring (0, 1) == " ")
                content.getReference (i) = content[i].substring (1);
        }

        TextEditor::insertTextAtCaret (content.joinIntoString (newLine));
    }
}

//=================================================================================================
const String SwingEditor::getCurrentParagraph() const
{
    const int caretPos = getCaretPosition();
    const int paraStart = getText().substring (0, caretPos).lastIndexOf ("\n");
    const int paraEnd = getText().indexOf (caretPos, "\n");

    if (paraStart == -1 && paraEnd == -1)
        return getText();

    else if (paraStart != -1 && paraEnd == -1)
        return getText().substring (paraStart);

    else if (paraStart == -1 && paraEnd != -1)
        return getText().substring (0, paraEnd);

    else  
        return getTextInRange (Range<int> (paraStart, paraEnd));
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

    // auto append a whitespace
    else if (textToInsert == ",") TextEditor::insertTextAtCaret (" ");
    else if (textToInsert == ";") TextEditor::insertTextAtCaret (" ");
    else if (textToInsert == ":") TextEditor::insertTextAtCaret (" ");
    else if (textToInsert == "!") TextEditor::insertTextAtCaret (" ");

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

//=================================================================================================
const String SwingEditor::calculateNumbersOfCurrentParagraph (const bool isSum) const
{
    StringArray nums;
    nums.addTokens (getCurrentParagraph(), " ", String());
    nums.removeEmptyStrings (true);

    float sumNum = 0.f;
    int num = 0;

    for (int i = nums.size(); --i >= 0; )
    {
        if (nums[i] == "|")
        {
            nums.remove (i);
        }
        else
        {
            if (nums[i].containsAnyOf ("-.1234567890"))
                ++num;

            sumNum += nums[i].getFloatValue();
        }
    }

    return String (isSum ? sumNum : sumNum / num, 2);
}


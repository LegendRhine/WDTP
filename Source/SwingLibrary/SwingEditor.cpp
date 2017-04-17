/*
  ==============================================================================

    SwingEditor.cpp
    Created: 18 Apr 2017 3:33:32am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "SwingEditor.h"


SwingEditor::SwingEditor (const Colour& caretColourWhenDragging)
{
    // mark for dragging position of the selected text
    draggingPosition.setFill (caretColourWhenDragging);
    // Colour::fromString (systemFile->getValue ("editorFontColour")).withAlpha (0.85f)

    addAndMakeVisible (draggingPosition);
}

//=================================================================================================
void SwingEditor::mouseDown (const MouseEvent& e)
{
    if (getHighlightedText ().isNotEmpty ()
        && getHighlightedRegion ().contains (getTextIndexAt (e.x, e.y))
        && !e.mods.isPopupMenu ())
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
        setMouseCursor (e.mods.isCommandDown () ? MouseCursor::CopyingCursor
                        : MouseCursor::NormalCursor);

        float cursorX, cursorY;
        float cursorHeight = getFont ().getHeight ();

        getCharPosition (getTextIndexAt (e.x, e.y), cursorX, cursorY, cursorHeight);
        Rectangle<float> pos (cursorX + 10.f, cursorY + 12.f - getViewport ()->getViewPositionY (),
                              2.5f, cursorHeight);

        draggingPosition.setRectangle (pos);

        if (getViewport ()->autoScroll (e.x, e.y, 50, 20))
            yOfViewportWhenDragging = getViewport ()->getViewPositionY ();
    }

    else
    {
        TextEditor::mouseDrag (e);
    }
}

//=================================================================================================
void SwingEditor::mouseUp (const MouseEvent& e)
{
    if (e.mods.isPopupMenu ())
    {
        TextEditor::mouseUp (e);
        return;
    }

    if (draggingSelected
        && !getHighlightedRegion ().contains (getTextIndexAt (e.x, e.y)))
    {
        SystemClipboard::copyTextToClipboard (getHighlightedText ());

        if (!e.mods.isCommandDown ())  // clear the highlight selected
            insertTextAtCaret (String ());

        if (yOfViewportWhenDragging != 0)
            getViewport ()->setViewPosition (0, yOfViewportWhenDragging);

        setCaretPosition (getTextIndexAt (e.x, e.y));
        insertTextAtCaret (SystemClipboard::getTextFromClipboard ());
    }

    else
    {
        TextEditor::mouseUp (e);
        const int caretPos = getCaretPosition ();

        if (caretPos != getHighlightedRegion ().getStart ()
            && getHighlightedRegion ().contains (getTextIndexAt (e.x, e.y)))
            setCaretPosition (getCaretPosition ());
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
    if (getHighlightedText ().isNotEmpty ()
        && getHighlightedRegion ().contains (getTextIndexAt (e.x - 5, e.y)))
        setMouseCursor (MouseCursor::NormalCursor);

    else
        setMouseCursor (MouseCursor::IBeamCursor);
}

//=================================================================================================
void SwingEditor::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& d)
{
    TextEditor::mouseWheelMove (e, d);

    if (draggingSelected && getHighlightedText ().isNotEmpty ())
        yOfViewportWhenDragging = getViewport ()->getViewPositionY ();
}


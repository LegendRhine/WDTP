/*
  ==============================================================================

    SwingEditor.h
    Created: 18 Apr 2017 3:33:32am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SWINGEDITOR_H_INCLUDED
#define SWINGEDITOR_H_INCLUDED

/** a extend text editor for drag and drop the highlight selected content,
    auto-wrap (punctuation matching) etc.
*/
class SwingEditor : public TextEditor,
                    public Timer
{
public:
    SwingEditor();
    ~SwingEditor() {}

    void setCaretColourWhenDragging (const Colour& caretColourWhenDragging);

    /** for drag and drop the selected text */
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails&) override;

    bool keyPressed (const KeyPress& key) override;

    /** auto-wrap, punctuation matching...*/
    virtual void insertTextAtCaret (const String& textToInsert) override;

    /** for Chinese punc-matching */
    virtual void timerCallback() override;

private:
    //=================================================================================================
    DrawableRectangle draggingPosition;
    String selectedForCnPunc;
    int yOfViewportWhenDragging = 0;
    bool draggingSelected = false;
    bool delPressed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwingEditor)
};


#endif  // SWINGEDITOR_H_INCLUDED

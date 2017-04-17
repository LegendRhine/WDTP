/*
  ==============================================================================

    SwingEditor.h
    Created: 18 Apr 2017 3:33:32am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SWINGEDITOR_H_INCLUDED
#define SWINGEDITOR_H_INCLUDED

/** 
*/
class SwingEditor : public TextEditor
{
public:
    SwingEditor (const Colour& caretColourWhenDragging);
    ~SwingEditor () {}

    /** for drag and drop the selected text */
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails&) override;

private:
    //=================================================================================================
    DrawableRectangle draggingPosition;
    int yOfViewportWhenDragging = 0;
    bool draggingSelected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwingEditor)
};


#endif  // SWINGEDITOR_H_INCLUDED

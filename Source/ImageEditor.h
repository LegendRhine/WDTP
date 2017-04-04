/*
  ==============================================================================

    ImageEditor.h
    Created: 5 Apr 2017 5:26:34am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef IMAGEEDITOR_H_INCLUDED
#define IMAGEEDITOR_H_INCLUDED


//==============================================================================
/*
*/
class ImageEditor    : public Component
{
public:
    ImageEditor();
    ~ImageEditor();

    void paint (Graphics&) override;
    void resized() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageEditor)
};


#endif  // IMAGEEDITOR_H_INCLUDED

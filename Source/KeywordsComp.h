/*
  ==============================================================================

    KeywordsComp.h
    Created: 28 Feb 2017 9:14:57am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef KEYWORDSCOMP_H_INCLUDED
#define KEYWORDSCOMP_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class KeywordsComp    : public Component
{
public:
    KeywordsComp (const bool displayInEditor);
    ~KeywordsComp();

    void paint (Graphics&) override;
    void resized() override;

private:
    Label titleLb;
    ScopedPointer<Viewport> viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsComp)
};


#endif  // KEYWORDSCOMP_H_INCLUDED

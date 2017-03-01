/*
  ==============================================================================

    KeywordsComp.h
    Created: 28 Feb 2017 9:14:57am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef KEYWORDSCOMP_H_INCLUDED
#define KEYWORDSCOMP_H_INCLUDED

class KeywordsComp    : public Component
{
public:
    KeywordsComp (const ValueTree& tree, 
                  const bool displayInEditor, 
                  const StringArray& keywordsToMatch);

    ~KeywordsComp();

    void paint (Graphics&) override;
    void resized() override;

    /** the actionBrodcaster is inside the viewport. */
    ActionBroadcaster* getKeywordsPicker();

private:
    //=================================================================================================
    Label titleLb;
    ScopedPointer<Viewport> viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsComp)
};


#endif  // KEYWORDSCOMP_H_INCLUDED

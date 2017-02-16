/*
==============================================================================

PropertiesPanel.h
Created: 23 Jan 2017 12:27:36am
Author:  SwingCoder

==============================================================================
*/

#ifndef __JUCE_HEADER_F48CB1E948F7606E__
#define __JUCE_HEADER_F48CB1E948F7606E__

//==============================================================================
/** This component used for replace all the matched-content in an item (valueTree) */
class ReplaceComponent : public Component,
                         public ButtonListener
{
public:
    //==============================================================================
    ReplaceComponent (TextEditor* editor, ValueTree& tree);
    ~ReplaceComponent () { }

    void paint (Graphics& g) override;
    void resized () override;
    void buttonClicked (Button* buttonThatWasClicked) override;

private:
    //=================================================================================================
    /** recursive method */
    void replaceContent (ValueTree tree_,
                         const String& originalText,
                         const String& replaceText);

    ValueTree tree;
    TextEditor* editor;
    bool replaced;

    ScopedPointer<Label> label;
    ScopedPointer<Label> orignalLb;
    ScopedPointer<Label> replaceToLb;
    ScopedPointer<TextEditor> originalTe;
    ScopedPointer<TextEditor> replaceTe;
    ScopedPointer<TextButton> replaceBt;
    ScopedPointer<TextButton> cancelBt;
    ScopedPointer<ToggleButton> caseBt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReplaceComponent)
};

#endif   // __JUCE_HEADER_F48CB1E948F7606E__

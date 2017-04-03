/*
  ==============================================================================

    ThemeEditor.h
    Created: 3 Apr 2017 10:57:50am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef THEMEEDITOR_H_INCLUDED
#define THEMEEDITOR_H_INCLUDED

//==============================================================================
/** for edit css, html-tpl file
*/
class ThemeEditor    : public Component
{
public:
    ThemeEditor();
    ~ThemeEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeEditor)
};


#endif  // THEMEEDITOR_H_INCLUDED

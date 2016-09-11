/*
  ==============================================================================

    PropertiesPanel.h
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SYSTEMSETUPPANEL_H_INCLUDED
#define SYSTEMSETUPPANEL_H_INCLUDED


//==============================================================================
/** */
class SystemSetupPanel    : public Component,
    public Value::Listener
{
public:
    SystemSetupPanel();
    ~SystemSetupPanel();

    void resized() override;

    virtual void valueChanged (Value& value) override;

private:
    ScopedPointer<PropertyPanel> panel;

    Value language, skin, fontSize, clickForEdit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemSetupPanel)
};


#endif  // SYSTEMSETUPPANEL_H_INCLUDED

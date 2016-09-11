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
    //=========================================================================
    ScopedPointer<PropertyPanel> panel;

    // system properties' values
    Value language, systemSkin, fontSize, clickForEdit;

    // project properties' values
    Value projectName, projectDesc, owner, projectSkin, 
          render, place;

    // dir properties' values
    Value dirName, dirDesc, isMenu, dirWebName;

    // doc properties' values
    Value docName, title, author, createTime, modifyTime,
        firstPublishTime, lastPublishTime, words, encrypt,
        publish, docWebName, tplFile, js;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemSetupPanel)
};


#endif  // SYSTEMSETUPPANEL_H_INCLUDED

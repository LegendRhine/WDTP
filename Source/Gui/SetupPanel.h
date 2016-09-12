/*
  ==============================================================================

    PropertiesPanel.h
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SETUPPANEL_H_INCLUDED
#define SETUPPANEL_H_INCLUDED


//==============================================================================
/** */
class SetupPanel    : public Component,
    public Value::Listener
{
public:
    SetupPanel();
    ~SetupPanel();

    void resized() override;

    virtual void valueChanged (Value& value) override;

private:
    //=========================================================================
    ScopedPointer<PropertyPanel> panel;

    // system properties' values
    Value language, systemSkin, fontSize, clickForEdit, exEditor;

    // project properties' values
    Value projectName, projectDesc, owner, projectSkin, render, place, 
        ftpAddress, ftpPort, ftpUserName, ftpPassword;

    // dir properties' values
    Value dirDesc, isMenu, dirWebName;

    // doc properties' values
    Value title, author, createTime, modifyTime, words, 
        publish, docWebName, tplFile, js;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupPanel)
};


#endif  // SETUPPANEL_H_INCLUDED

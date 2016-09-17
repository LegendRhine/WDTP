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
/** On the right of main interface. 
*/
class SetupPanel : public Component,
    private Value::Listener,
    private Timer
{
public:
    SetupPanel();
    ~SetupPanel();

    void showSystemProperties();
    void showProjectProperties (ValueTree& projectTree);
    void showDirProperties (ValueTree& dirTree);
    void showDocProperties (ValueTree& docTree);

    void projectClosed();
    void resized() override;

private:
    //=========================================================================
    void valuesAddListener();
    void valuesRemoveListener();
    virtual void valueChanged (Value & value) override;

    virtual void timerCallback () override;
    void savePropertiesIfNeeded ();

    //=========================================================================
    enum
    { 
        // system properties' values, panel group 0
        language = 0, clickForEdit, fontSize,  

        // project properties' values, panel group 1
        projectName, projectDesc, owner, projectSkin, 
        projectRenderDir, place, domain, 
        ftpAddress, ftpPort, ftpUserName, ftpPassword, 

        // dir properties' values, panel group 2
        dirDesc, isMenu, dirRenderDir, dirWebName, 

        // doc properties' values, panel group 3
        title, author, publish, docWebName, tplFile, js, 

        totalValues
    };

    ScopedPointer<PropertyPanel> panel;
    OwnedArray<Value> values;
    ValueTree projectTree;
    bool projectHasChanged = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupPanel)
        
};


#endif  // SETUPPANEL_H_INCLUDED

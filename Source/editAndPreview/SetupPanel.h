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

    void resized () override;
    void showSystemProperties ();
    
    void showProjectProperties (ValueTree& projectTree);
    void showDirProperties (ValueTree& dirTree);
    void showDocProperties (ValueTree& docTree);
    
    void projectClosed();

    /** only add some relative value which current showing */
    void valuesAddListener (const int startIndex, const int endIndex);
    void valuesRemoveListener ();
    
private:
    //=========================================================================
    
    virtual void valueChanged (Value & value) override;

    virtual void timerCallback () override;
    void savePropertiesIfNeeded ();

    //=========================================================================
    // NOTE: must keep the first and the last name of each group!
    // see: valuesAddListener()
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

    ValueTree projectTree = ValueTree::invalid;
    ValueTree dirTree = ValueTree::invalid;
    ValueTree docTree = ValueTree::invalid;
    bool projectHasChanged = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupPanel)
        
};


#endif  // SETUPPANEL_H_INCLUDED

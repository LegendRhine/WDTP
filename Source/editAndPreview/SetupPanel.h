/*
  ==============================================================================

    PropertiesPanel.h
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SETUPPANEL_H_INCLUDED
#define SETUPPANEL_H_INCLUDED

class EditAndPreview;

//==============================================================================
/** On the right of main interface. 
*/
class SetupPanel : public Component,
    private Value::Listener,
    private Timer
{
public:
    SetupPanel (EditAndPreview* editor);
    ~SetupPanel();

    void resized () override;

    void projectClosed();
    void showNothing ()    { panel->clear (); }

    void showProjectProperties (ValueTree& projectTree);
    void showDirProperties (ValueTree& dirTree);
    void showDocProperties (ValueTree& docTree);   

    
private:
    //=========================================================================    
    void valuesAddListener ();
    void valuesRemoveListener ();

    virtual void valueChanged (Value & value) override;
    virtual void timerCallback () override;
    
    void savePropertiesIfNeeded ();

    //=========================================================================
    enum  
    { 
        // project properties' values
        projectTitle = 0, projectDesc, owner, /*projectSkin,*/ 
        projectRenderDir, fontSize, 

        // dir properties' values
        dirName, dirTitle, dirDesc, isMenu, dirDate, 

        // doc properties' values
        docName, docTitle, keywords, docDesc, isPage, docDate, 

        totalValues
    };

    ScopedPointer<PropertyPanel> panel;
    OwnedArray<Value> values;
    EditAndPreview* editor;

    ValueTree currentTree = ValueTree::invalid;
    bool projectHasChanged = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupPanel)
        
};


#endif  // SETUPPANEL_H_INCLUDED

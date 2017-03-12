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
/** This component uses for setup the selected item's properties. */
class SetupPanel : public Component,
                   private Value::Listener,
                   private Timer
{
public:
    SetupPanel (EditAndPreview* editor);
    ~SetupPanel();

    void resized() override;
    void projectClosed();

    void showNothing()                         { panel->clear(); }
    void updateWordCount (const int wordsNum)  { values[wordCount]->setValue (wordsNum); }

    void showProjectProperties (const ValueTree& projectTree);
    void showDirProperties (const ValueTree& dirTree);
    void showDocProperties (const bool currentValuesUpdateTree, const ValueTree& docTree);

private:
    //=========================================================================    
    void valuesAddListener();
    void initialValues (const bool currentValuesUpdateTree, const bool addValues);

    virtual void valueChanged (Value& value) override;

    // for save the systemFile and project
    virtual void timerCallback() override;  
    void savePropertiesIfNeeded();

    //=========================================================================
    enum
    {        
        itsName = 0, itsTitle, keywords, desc, projectOwner,
        copyrightInfo, renderDir, tplFile, jsCode, 
        contact, ad, isMenu, createDate, modifyDate,
        showKeys, wordCount, thumb, thumbName, 
        abbrev, reviewDate, featured, 

        totalValues
    };

    ScopedPointer<PropertyPanel> panel;
    OwnedArray<Value> values;
    EditAndPreview* editor;

    ValueTree currentTree;
    bool projectHasChanged;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetupPanel)

};


#endif  // SETUPPANEL_H_INCLUDED

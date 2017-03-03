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
    void updateDocPanel()                      { showDocProperties (currentTree); }

    void showProjectProperties (ValueTree& projectTree);
    void showDirProperties (ValueTree& dirTree);
    void showDocProperties (ValueTree& docTree);

private:
    //=========================================================================    
    void valuesAddListener();
    void valuesRemoveListener();

    virtual void valueChanged (Value & value) override;

    // for save the systemFile and project
    virtual void timerCallback() override;  
    void savePropertiesIfNeeded();

    //=========================================================================
    enum
    {
        // project properties' values
        projectTitle = 0, projectKeywords, projectDesc, owner,
        copyrightInfo, projectRenderDir, indexTpl, projectJs, 
        contact, ad, keywordIndex,

        // dir properties' values
        dirName, dirTitle, dirKeywords, dirDesc, 
        dirIsMenu, dirTpl, dirJs,
        dirCreateDate, /*dirModifyDate,*/

        // doc properties' values
        docName, docTitle, docKeywords, showKeys, docDesc, 
        docIsMenu, docTpl, docCreateDate, /*docModifyDate,*/ 
        docJs, wordCount, thumb, thumbName, docAbbrev,

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

/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.3.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_F118E4D0B881C0E6__
#define __JUCE_HEADER_F118E4D0B881C0E6__

//[Headers]     -- You can add your own extra header files here --
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class StatisComp  : public Component,
                    public TextEditor::Listener,
                    public ButtonListener
{
public:
    //==============================================================================
    StatisComp (FileTreeContainer* fileTree, DocTreeViewItem* item, const bool isDoc_, const String& statisStr);
    ~StatisComp();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    virtual void textEditorReturnKeyPressed (TextEditor&) override;
    virtual void textEditorEscapeKeyPressed (TextEditor&) override;
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

    void analyseDoc();
    void analyseDir();
    void analyseDir (DocTreeViewItem* currentItem, int& files, int& totalNumbers);

    void showAnalyseResult(const int docNum, const int totalNum);

    FileTreeContainer* treeContainer;
    DocTreeViewItem* dirItem;
    const bool isDoc;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> titleLabel;
    ScopedPointer<TextEditor> infoEditor;
    ScopedPointer<Label> keywordLabel;
    ScopedPointer<TextEditor> keywordEditor;
    ScopedPointer<TextEditor> analyseEditor;
    ScopedPointer<TextButton> analyseBt;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatisComp)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_F118E4D0B881C0E6__

/*
  ==============================================================================

    TopToolBar.h
    Created: 4 Sep 2016 12:25:18am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef TOPTOOLBAR_H_INCLUDED
#define TOPTOOLBAR_H_INCLUDED

class FileTreeContainer;

//==============================================================================
/** */
class TopToolBar    : public Component,
    private TextEditor::Listener,
    private Button::Listener,
    public ChangeListener
{
public:
    TopToolBar (FileTreeContainer* container, EditAndPreview* editAndPreview);
    ~TopToolBar();

    void paint (Graphics&) override;
    void resized() override;

    const bool getStateOfViewButton() const              { return bts[view]->getToggleState(); }
    void enableEditPreviewBt (const bool enableIt, const bool toggleState);

    virtual void changeListenerCallback(ChangeBroadcaster* source) override;

private:
    //==========================================================================
    virtual void textEditorReturnKeyPressed (TextEditor&) override;
    virtual void textEditorEscapeKeyPressed (TextEditor&) override;

    void findInProject (const bool next);
    void findInDoc (const bool next);

    virtual void buttonClicked (Button*) override;
    void popupSystemMenu();
    void menuPerform (const int menuIndex);
    
    void createNewProject ();
    void openProject();
    void cleanAndGenerateAll ();

    /** generate the tree and all its children's html file */
    static void generateHtmlFiles (ValueTree tree);

    //==========================================================================
    enum { prevAll, nextAll, prevPjt, nextPjt, 
           view, system, totalBts };

    /** When mouse enter the button, the cursor will change to a finger */
    class MyImageButton : public ImageButton
    {
    public:
        MyImageButton() 
        {
            setMouseCursor (MouseCursor::PointingHandCursor);
        }

        virtual void enablementChanged() override
        {
            setMouseCursor (isEnabled () ? MouseCursor::PointingHandCursor
                            : MouseCursor::NormalCursor);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyImageButton)
    };

    //=========================================================================
    void setUiBackground();
    void setUiTextColour();
    void resetUiColour();

    OwnedArray<MyImageButton> bts;
    ScopedPointer<TextEditor> searchInProject;
    ScopedPointer<TextEditor> searchInDoc;

    FileTreeContainer* fileTreeContainer;
    EditAndPreview* editAndPreview;

    ScopedPointer<ColourSelectorWithPreset> fontColourSelector;
    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopToolBar)        
};


#endif  // TOPTOOLBAR_H_INCLUDED

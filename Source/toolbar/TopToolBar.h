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
    private Button::Listener
{
public:
    TopToolBar (FileTreeContainer* container, EditAndPreview* editAndPreview);
    ~TopToolBar();

    void paint (Graphics&) override;
    void resized() override;

    const bool getStateOfViewButton() const              { return bts[view]->getToggleState(); }
    void enableEditPreviewBt (const bool enableIt, const bool toggleState);

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

    //==========================================================================
    enum { prevAll, nextAll, prevPjt, nextPjt, 
           view, system, totalBts };

    /** When mouse enter the button, the cursor will change to a finger */
    class MyImageButton : public ImageButton
    {
    public:
        MyImageButton() 
        {
            setMouseCursor(MouseCursor::PointingHandCursor);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyImageButton)
    };

    //=========================================================================
    OwnedArray<MyImageButton> bts;
    ScopedPointer<TextEditor> searchInProject;
    ScopedPointer<TextEditor> searchInDoc;

    FileTreeContainer* fileTreeContainer;
    EditAndPreview* editAndPreview;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopToolBar)

};


#endif  // TOPTOOLBAR_H_INCLUDED

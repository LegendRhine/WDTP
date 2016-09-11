/*
  ==============================================================================

    EditAndPreview.h
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef EDITANDPREVIEW_H_INCLUDED
#define EDITANDPREVIEW_H_INCLUDED

class SystemSetupPanel;

//==============================================================================
/** Blank in initial, it'll show edit-mode or preview-mode base on user's click.

    In edit-mode, a textEditor on the left, a propertiesPanel on the right,
    between the two is the stretched-layoutBar.

    The whole area will be a WebComponent when in preview-mode, which covered the 
    textEditor and the propertiesPanel.
*/
class EditAndPreview    : public Component
{
public:
    EditAndPreview();
    ~EditAndPreview();
    
    void paint(Graphics&) override {}
    void resized() override;

    void editDoc (const File& docFile);
    void previewDoc (const File& docFile);

    void setSystemProperties ();
    void setDocProperties (const ValueTree& docTree);
    void setDirProperties (const ValueTree& dirTree);
    void setProjectProperties ();

    void whenFileOrDirNonexists ();

private:
    ScopedPointer<TextEditor> editor;
    ScopedPointer<WebBrowserComponent> htmlPreview;
    ScopedPointer<SystemSetupPanel> setupPanel;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StretchableLayoutResizerBar> layoutBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditAndPreview)
};


#endif  // EDITANDPREVIEW_H_INCLUDED

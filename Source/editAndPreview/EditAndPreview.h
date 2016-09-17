﻿/*
  ==============================================================================

    EditAndPreview.h
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef EDITANDPREVIEW_H_INCLUDED
#define EDITANDPREVIEW_H_INCLUDED

class SetupPanel;

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
    void projectClosed ();

    void setSystemProperties ();
    void setProjectProperties (ValueTree& projectTree);
    void setDirProperties (ValueTree& dirTree);
    void setDocProperties (ValueTree& docTree);

    void whenFileOrDirNonexists ();

private:
    //=========================================================================
    ScopedPointer<TextEditor> editor;
    ScopedPointer<WebBrowserComponent> webView;
    ScopedPointer<SetupPanel> setupPanel;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StretchableLayoutResizerBar> layoutBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditAndPreview)
};

//=========================================================================
class EditorForMd : public TextEditor
{
public:
    EditorForMd ();
    ~EditorForMd ();

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorForMd)
};

#endif  // EDITANDPREVIEW_H_INCLUDED

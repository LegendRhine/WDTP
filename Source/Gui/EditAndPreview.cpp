/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

//==============================================================================
EditAndPreview::EditAndPreview()
{
    addChildComponent (htmlPreview = new WebBrowserComponent ());

    // stretched layout, arg: index, min-width, max-width£¬default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.72);  // editor£¬
    layoutManager.setItemLayout (1, 3, 3, 3);           // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.28);  // propertiesPanel

    addAndMakeVisible (editor = new TextEditor ());
    addAndMakeVisible (setupPanel = new Component ());
    addAndMakeVisible (layoutBar = new StretchableLayoutResizerBar (&layoutManager, 1, true));

    // editor
    editor->setMultiLine (true);
    editor->setReturnKeyStartsNewLine (true);
    editor->setColour (TextEditor::textColourId, Colour(0xff303030));
    editor->setColour (TextEditor::focusedOutlineColourId, Colour(0xffb4b4b4));
    editor->setColour (TextEditor::backgroundColourId, Colour(0xffededed));
    editor->setScrollBarThickness (10);
    editor->setIndents(6, 6);
    editor->setFont (SwingUtilities::getFontSize());
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
}

//=========================================================================
void EditAndPreview::resized()
{
    // layout
    Component* comps[] = { editor, layoutBar, setupPanel };
    layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
}

//=================================================================================================
void EditAndPreview::editDoc (const File& docFile)
{
    editor->setText (docFile.loadFileAsString());
}

//=================================================================================================
void EditAndPreview::previewDoc (const File& docFile)
{

}

//=================================================================================================
void EditAndPreview::setSystemProperties ()
{
    setupPanel = new SystemSetupPanel ();
    addAndMakeVisible (setupPanel);
    resized ();
}

//=================================================================================================
void EditAndPreview::setDocProperties (const ValueTree& docTree)
{

}

//=================================================================================================
void EditAndPreview::setDirProperties (const ValueTree& dirTree)
{

}

//=================================================================================================
void EditAndPreview::setProjectProperties ()
{

}

//=================================================================================================
void EditAndPreview::whenFileOrDirNonexists ()
{
    NEED_TO_DO ("fileOrDirNonexists");
}


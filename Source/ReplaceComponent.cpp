/*
==============================================================================

PropertiesPanel.h
Created: 23 Jan 2017 12:27:36am
Author:  SwingCoder

==============================================================================
*/

#include "WdtpHeader.h"

//==============================================================================
ReplaceComponent::ReplaceComponent (TextEditor* editor_, 
                                    ValueTree& tree_)
    : editor (editor_),
    tree (tree_)
{
    jassert(editor != nullptr);
    jassert(tree.isValid());

    const String& nameText(tree.getType().toString() == "wdtpProject" ? TRANS("project!") :
                           (tree.getType().toString() == "dir" ? TRANS("dir!") : TRANS("doc!")));

    addAndMakeVisible(label = new Label(String(), TRANS("Warning! This will replace all matching content in this ")
                                        + nameText));

    label->setFont (Font (16.00f, Font::plain));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);
    label->setColour (Label::textColourId, Colours::red);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (orignalLb = new Label (String(),
                                              TRANS("Original Content:")));
    orignalLb->setFont (Font (15.00f, Font::plain));
    orignalLb->setJustificationType (Justification::centredLeft);
    orignalLb->setEditable (false, false, false);
    orignalLb->setColour (TextEditor::textColourId, Colours::black);
    orignalLb->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (replaceToLb = new Label (String(),
                                                TRANS("Replace To: ")));
    replaceToLb->setFont (Font (15.00f, Font::plain));
    replaceToLb->setJustificationType (Justification::centredLeft);
    replaceToLb->setEditable (false, false, false);
    replaceToLb->setColour (TextEditor::textColourId, Colours::black);
    replaceToLb->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (originalTe = new TextEditor (String()));
    originalTe->setMultiLine (false);
    originalTe->setReturnKeyStartsNewLine (false);
    originalTe->setReadOnly (false);
    originalTe->setScrollbarsShown (true);
    originalTe->setCaretVisible (true);
    originalTe->setPopupMenuEnabled (true);
    originalTe->setText (String());

    addAndMakeVisible (replaceTe = new TextEditor (String()));
    replaceTe->setMultiLine (false);
    replaceTe->setReturnKeyStartsNewLine (false);
    replaceTe->setReadOnly (false);
    replaceTe->setScrollbarsShown (true);
    replaceTe->setCaretVisible (true);
    replaceTe->setPopupMenuEnabled (true);
    replaceTe->setText (String());

    addAndMakeVisible (replaceBt = new TextButton (String()));
    replaceBt->setButtonText (TRANS("Replace"));
    replaceBt->addListener (this);

    addAndMakeVisible (cancelBt = new TextButton (String()));
    cancelBt->setButtonText (TRANS("Re-input"));
    cancelBt->addListener (this);

    addAndMakeVisible (caseBt = new ToggleButton (String()));
    caseBt->setButtonText (TRANS("Case sensitive"));
    caseBt->addListener (this);
    
    setSize (385, 175);
}

//=================================================================================================
ReplaceComponent::~ReplaceComponent()
{
    label = nullptr;
    orignalLb = nullptr;
    replaceToLb = nullptr;
    originalTe = nullptr;
    replaceTe = nullptr;
    replaceBt = nullptr;
    cancelBt = nullptr;
    caseBt = nullptr;
}

//==============================================================================
void ReplaceComponent::paint (Graphics& g)
{
    g.fillAll (Colour(0xffdcdbdb));
}

//=================================================================================================
void ReplaceComponent::resized()
{
    label->setBounds (0, 8, getWidth() - 0, 24);
    orignalLb->setBounds (16, 45, 112, 24);
    replaceToLb->setBounds (16, 77, 112, 24);
    originalTe->setBounds (136, 45, 230, 24);
    replaceTe->setBounds (136, 77, 230, 24);
    replaceBt->setBounds (165, 143, 96, 24);
    cancelBt->setBounds (274, 143, 96, 24);
    caseBt->setBounds (13, 109, 150, 24);
}

//=================================================================================================
void ReplaceComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == replaceBt)
    {
        if (originalTe->getText().isEmpty() 
            || replaceTe->getText().isEmpty()
            || originalTe->getText() == replaceTe->getText())
            return;

        const String& originalText(originalTe->getText());
        const String& replaceText(replaceTe->getText());

        replaceContent(tree, originalText, replaceText);

        if (replaced)
            DocTreeViewItem::needCreateAndUpload(tree);

        if (replaced && tree.getType().toString() == "doc")
            editor->setText(DocTreeViewItem::getMdFileOrDir(tree).loadFileAsString());
    }
    else if (buttonThatWasClicked == cancelBt)
    {
        originalTe->setText(String());
        replaceTe->setText(String());
        originalTe->grabKeyboardFocus();
        replaced = false;
    }
    else if (buttonThatWasClicked == caseBt)
    {
        caseSens = caseBt->getToggleState();
        replaced = false;
    }
}

//=================================================================================================
void ReplaceComponent::replaceContent(ValueTree tree_, 
                                      const String& originalText, 
                                      const String& replaceText)
{
    if (tree_.getType().toString() == "doc")
    {
        const File& docFile(DocTreeViewItem::getMdFileOrDir(tree_));
        const String& content(docFile.loadFileAsString());
        
        if (content.contains(originalText))
        {
            docFile.replaceWithText(content.replace(originalText, replaceText, caseSens));
            replaced = true;
        }
        else
        {
            replaced = false;
        }
    } 
    else
    {
        for (int i = tree_.getNumChildren(); --i >= 0; )
            replaceContent(tree_.getChild(i), originalText, replaceText);
    }
}


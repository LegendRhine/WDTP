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
    : tree (tree_),
    editor (editor_)
{
    jassert (editor != nullptr);
    jassert (tree.isValid());

    const String& nameText (tree.getType().toString() == "wdtpProject" ? TRANS ("project!") :
        (tree.getType().toString() == "dir" ? TRANS ("dir!") : TRANS ("doc!")));

    addAndMakeVisible (label = new Label (String(),
                                          TRANS ("Warning! This will replace all matching content in this ")
                                          + nameText));

    label->setFont (Font (17.00f, Font::plain));
    label->setJustificationType (Justification::centred);
    label->setEditable (false, false, false);

    addAndMakeVisible (orignalLb = new Label (String(),
                                              TRANS ("Original Content:")));
    orignalLb->setFont (Font (16.00f, Font::plain));
    orignalLb->setJustificationType (Justification::centredLeft);
    orignalLb->setEditable (false, false, false);

    addAndMakeVisible (replaceToLb = new Label (String(),
                                                TRANS ("Replace To: ")));
    replaceToLb->setFont (Font (16.00f, Font::plain));
    replaceToLb->setJustificationType (Justification::centredLeft);
    replaceToLb->setEditable (false, false, false);
    
    addAndMakeVisible (originalTe = new TextEditor (String()));
    originalTe->setMultiLine (false);
    originalTe->setReturnKeyStartsNewLine (false);
    originalTe->setReadOnly (false);
    originalTe->setCaretVisible (true);
    originalTe->setPopupMenuEnabled (true);
    originalTe->setSelectAllWhenFocused (true);
    originalTe->setFont (16.f);
    originalTe->setText (SystemClipboard::getTextFromClipboard().removeCharacters ("\n")
                         .removeCharacters ("\r"), dontSendNotification);

    addAndMakeVisible (replaceTe = new TextEditor (String()));
    replaceTe->setMultiLine (false);
    replaceTe->setReturnKeyStartsNewLine (false);
    replaceTe->setReadOnly (false);
    replaceTe->setCaretVisible (true);
    replaceTe->setPopupMenuEnabled (true);
    replaceTe->setText (String());
    replaceTe->setSelectAllWhenFocused (true);
    replaceTe->setFont (16.f);
    replaceTe->addListener (this);

    addAndMakeVisible (replaceBt = new TextButton (String()));
    replaceBt->setButtonText (TRANS ("Replace"));
    replaceBt->addListener (this);

    addAndMakeVisible (cancelBt = new TextButton (String()));
    cancelBt->setButtonText (TRANS ("Re-input"));
    cancelBt->addListener (this);

    addAndMakeVisible (caseBt = new ToggleButton (String()));
    caseBt->setButtonText (TRANS ("Case sensitive"));
    caseBt->addListener (this);

    setSize (385, 175);

    replaceTe->setExplicitFocusOrder (1);
    caseBt->setExplicitFocusOrder (2);
    replaceBt->setExplicitFocusOrder (3);
    cancelBt->setExplicitFocusOrder (4);
    originalTe->setExplicitFocusOrder (5);
}

//==============================================================================
void ReplaceComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xffdcdbdb));
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
        if (originalTe->getText().isEmpty() || originalTe->getText() == replaceTe->getText())
            return;

        const String& originalText (originalTe->getText());
        const String& replaceText (replaceTe->getText());

        int numberFilesOfReplaced = 0;
        int numberOfReplaced = 0;
        replaceContent (tree, originalText, replaceText, 
                        numberFilesOfReplaced, numberOfReplaced);
        
        if (numberOfReplaced > 0)
        {
            DocTreeViewItem::needCreate (tree);
            SHOW_MESSAGE (TRANS ("Total replaced: ") 
                          + String (numberOfReplaced) + TRANS(" matched in ")
                          + String (numberFilesOfReplaced) + TRANS(" file(s)."));
        }
        else
        {
            LookAndFeel::getDefaultLookAndFeel().playAlertSound();
            SHOW_MESSAGE (TRANS ("Nothing could be found."));
        }

        if ((numberOfReplaced > 0) && tree.getType().toString() == "doc")
            editor->setText (DocTreeViewItem::getMdFileOrDir (tree).loadFileAsString());
    }
    else if (buttonThatWasClicked == cancelBt)
    {
        originalTe->setText (String());
        replaceTe->setText (String());
        
        originalTe->grabKeyboardFocus();
    }
}

//=================================================================================================
void ReplaceComponent::textEditorReturnKeyPressed (TextEditor& te)
{
    if (&te == replaceTe)
        replaceBt->triggerClick();
}

//=================================================================================================
void ReplaceComponent::replaceContent (ValueTree treeNeedReplaced,
                                       const String& originalText,
                                       const String& replaceText,
                                       int& numberFilesOfReplaced,
                                       int& numberOfReplaced)
{
    if (treeNeedReplaced.getType().toString() == "doc")
    {
        const File& docFile (DocTreeViewItem::getMdFileOrDir (treeNeedReplaced));
        String content (docFile.loadFileAsString());

        const bool contained = caseBt->getToggleState() ? content.contains (originalText)
            : content.containsIgnoreCase (originalText);

        if (contained)
        {
            ++numberFilesOfReplaced;

            int startIndex = caseBt->getToggleState() 
                ? content.indexOf (0, originalText)
                : content.indexOfIgnoreCase (0, originalText);

            while (startIndex != -1)
            {
                content = content.replaceSection (startIndex, originalText.length(), replaceText);
                ++numberOfReplaced;

                startIndex = caseBt->getToggleState()
                    ? content.indexOf (startIndex + replaceText.length(), originalText)
                    : content.indexOfIgnoreCase (startIndex + replaceText.length(), originalText);
            }

            docFile.replaceWithText (content);
        }        
    }
    else
    {
        for (int i = treeNeedReplaced.getNumChildren(); --i >= 0; )
            replaceContent (treeNeedReplaced.getChild (i), originalText, replaceText, 
                            numberFilesOfReplaced, numberOfReplaced);
    }
}


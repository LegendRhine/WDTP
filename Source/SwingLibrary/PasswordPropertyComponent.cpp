/*
  ==============================================================================

    PasswordPropertyComponent.cpp
    Created: 18 Sep 2016 9:35:48am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "PasswordPropertyComponent.h"

//==============================================================================
PasswordPropertyComponent::PasswordPropertyComponent (const String& name)
    : PropertyComponent (name)
{
    createEditor ();
}

PasswordPropertyComponent::PasswordPropertyComponent (const Value& valueToControl,
                                              const String& name)
    : PropertyComponent (name)
{
    createEditor ();
    textEditor->getTextValue().referTo (valueToControl);
}

//=========================================================================
PasswordPropertyComponent::~PasswordPropertyComponent ()
{
}

//=========================================================================
void PasswordPropertyComponent::setText (const String& newText)
{
    textEditor->setText (newText, true);
}

//=========================================================================
String PasswordPropertyComponent::getText () const
{
    return textEditor->getText ();
}

//=========================================================================
Value& PasswordPropertyComponent::getValue () const
{
    return textEditor->getTextValue ();
}

//=========================================================================
void PasswordPropertyComponent::createEditor ()
{
    addAndMakeVisible (textEditor = new TextEditor (String(), '*'));
    textEditor->setColour (TextEditor::outlineColourId, findColour (TextPropertyComponent::outlineColourId));
    textEditor->setInputRestrictions (60);
    textEditor->setSelectAllWhenFocused (true);
}

//=========================================================================
void PasswordPropertyComponent::refresh ()
{
    textEditor->setText (getText(), dontSendNotification);
}

//=========================================================================
void PasswordPropertyComponent::textWasEdited ()
{
    const String newText (textEditor->getText ());

    if (getText () != newText)
        setText (newText);
}

//=================================================================================================
void PasswordPropertyComponent::focusOfChildComponentChanged (FocusChangeType /*cause*/)
{
    if (!hasKeyboardFocus (true))
        textEditor->setHighlightedRegion (Range<int> (0, 0));
}

//=========================================================================
void PasswordPropertyComponent::colourChanged ()
{
    PropertyComponent::colourChanged ();
}

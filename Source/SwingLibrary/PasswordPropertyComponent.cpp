/*
  ==============================================================================

    PasswordPropertyComponent.cpp
    Created: 12 Jan 2017 9:35:48am
    Author:  LoopFine

  ==============================================================================
*/

#include "JuceHeader.h"
#include "PasswordPropertyComponent.h"

//==============================================================================
class PasswordPropertyComponent::LabelComp : public Label
{
public:
    LabelComp(PasswordPropertyComponent& tpc)
        : Label(String(), String()),
        owner(tpc)
    {
        setEditable(true, true, false);
        updateColours();
    }
    
    TextEditor* createEditorComponent() override
    {
        TextEditor* const ed = Label::createEditorComponent();
        //ed->setPasswordCharacter('*');
        ed->setInputRestrictions(120);

        return ed;
    }

    void paint (Graphics& g) override
    {
        g.fillAll(findColour(Label::backgroundColourId));

        if (!isBeingEdited())
        {
            const float alpha = isEnabled() ? 1.0f : 0.5f;
            const Font f(LookAndFeel::getDefaultLookAndFeel().getLabelFont(*this));

            g.setColour(findColour(Label::textColourId).withMultipliedAlpha(alpha));
            g.setFont(f);

            Rectangle<int> textArea(getBorderSize().subtractedFrom(getLocalBounds()));

            const String passwordChars (String::repeatedString("*", getText().length()));
            g.drawFittedText(passwordChars, textArea, getJustificationType(),
                             jmax(1, (int) (textArea.getHeight() / f.getHeight())),
                             getMinimumHorizontalScale());

            g.setColour(findColour(Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (isEnabled())
        {
            g.setColour(findColour(Label::outlineColourId));
        }

        g.drawRect(getLocalBounds());
    }

    void textWasEdited() override
    {
        owner.textWasEdited();
    }

    void updateColours()
    {
        setColour(backgroundColourId, owner.findColour(TextPropertyComponent::backgroundColourId));
        setColour(outlineColourId, owner.findColour(TextPropertyComponent::outlineColourId));
        setColour(textColourId, owner.findColour(TextPropertyComponent::textColourId));
        repaint();
    }

private:
    PasswordPropertyComponent& owner;
};

//===============================================================================
PasswordPropertyComponent::PasswordPropertyComponent(const Value& valueToControl,
                                                     const String& name)
    : PropertyComponent(name)
{
    addAndMakeVisible(textEditor = new LabelComp(*this));
    textEditor->getTextValue().referTo(valueToControl);
}

PasswordPropertyComponent::~PasswordPropertyComponent()
{ }

void PasswordPropertyComponent::setText(const String& newText)
{
    textEditor->setText(newText, sendNotificationSync);
}

String PasswordPropertyComponent::getText() const
{
    return textEditor->getText();
}

Value& PasswordPropertyComponent::getValue() const
{
    return textEditor->getTextValue();
}

void PasswordPropertyComponent::refresh()
{
    textEditor->setText(getText(), dontSendNotification);
}

void PasswordPropertyComponent::textWasEdited()
{
    const String newText(textEditor->getText());

    if (getText() != newText)
        setText(newText);

    callListeners();
}

void PasswordPropertyComponent::addListener(PasswordPropertyComponent::Listener* const listener)
{
    listenerList.add(listener);
}

void PasswordPropertyComponent::removeListener(PasswordPropertyComponent::Listener* const listener)
{
    listenerList.remove(listener);
}

void PasswordPropertyComponent::callListeners()
{
    Component::BailOutChecker checker(this);
    listenerList.callChecked(checker, &PasswordPropertyComponent::Listener::textPropertyComponentChanged, this);
}

void PasswordPropertyComponent::colourChanged()
{
    PropertyComponent::colourChanged();
    textEditor->updateColours();
}


/*
  ==============================================================================

    KeywordsComp.cpp
    Created: 28 Feb 2017 9:14:57am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

//=================================================================================================
struct TextButtonLF : public LookAndFeel_V3
{
    virtual Font getTextButtonFont (TextButton&, int /*buttonHeight*/) override
    {
        return Font (16.f);
    }
};

//=================================================================================================
class KeywordsButtons : public Component,
                        public Button::Listener,
                        public ActionBroadcaster
{
public:
    KeywordsButtons (const ValueTree& tree,
                     const bool showInEditor, 
                     const StringArray& kwToMatch) :
        displayInEditor (showInEditor)
    {        
        StringArray keywords;
        keywords.addTokens (HtmlProcessor::extractKeywordsOfDocs (tree), ",", String());

        // add buttons
        for (int i = keywords.size(); --i >= 0; )
        {
            TextButton* bt = new TextButton (keywords[i].replace ("--", " (")
                                             + (keywords[i].contains ("--") ? ")" : String()));

            if (displayInEditor)
                bt->setTooltip (keywords[i].upToFirstOccurrenceOf ("--", false, true));

            bt->setSize (100, 25);
            bt->setColour (TextButton::buttonColourId, Colours::lightgrey.withAlpha (0.15f));
            bt->setColour (TextButton::buttonOnColourId, Colours::lightskyblue);
            bt->setLookAndFeel (&tlf);

            setToggleIfMatched (bt, kwToMatch);

            bt->addListener (this);
            addAndMakeVisible (bt);
            bts.insert (0, bt);
        }

        const int btNum = keywords.size();
        setSize (550, (btNum / 5 * 30) + ((btNum % 5 != 0) ? 40 : 15));
    }

    //=================================================================================================
    ~KeywordsButtons()     {    }

    //=================================================================================================
    const int getKeywordsNumbers() const { return bts.size(); }

    //=================================================================================================
    void resized()
    {
        int x, y;
        x = y = 0;

        for (int i = 0; i < bts.size(); ++i)
        {
            if (i % 5 == 0 && i != 0)
            {
                x = 0;
                ++y;
            }

            bts[i]->setTopLeftPosition (x * 105 + 10, y * 30 + 5);
            ++x;
        }
    }

    //=================================================================================================
    virtual void buttonClicked (Button* bt) override
    {
        const bool toggled = bt->getToggleState();
        const String& buttonText (bt->getButtonText());
        const String& textWithoutTimes (getTextWithoutTimes (bt));

        if (displayInEditor)
        {
            bt->setToggleState (!toggled, dontSendNotification);
            sendActionMessage ((toggled ? "--" : "++") + textWithoutTimes);

            // times - 1
            if (toggled && buttonText.getLastCharacters (1) == ")")
            {
                int num = bt->getButtonText().dropLastCharacters (1)
                    .fromLastOccurrenceOf ("(", false, true).getIntValue();

                if (2 == num) // no need to display the times when it is the first keyword in table
                    bt->setButtonText (textWithoutTimes);
                else
                    bt->setButtonText (textWithoutTimes + " (" + String (--num) + ")");
            }

            // times + 1
            else if (!toggled && buttonText.getLastCharacters (1) == ")")
            {
                int num = bt->getButtonText().dropLastCharacters (1)
                    .fromLastOccurrenceOf ("(", false, true).getIntValue();

                bt->setButtonText (textWithoutTimes + " (" + String (++num) + ")");
            }

            // times turned to be '2'
            else if (!toggled && buttonText.getLastCharacters (1) != ")")
            {
                bt->setButtonText (textWithoutTimes + " (2)");
            }
        }
        else
        {
            sendActionMessage (textWithoutTimes);
        }
    }

private:
    //=================================================================================================

    void setToggleIfMatched (TextButton* bt, const StringArray& kwToMatch)
    {
        const String& text (getTextWithoutTimes (bt));

        for (int i = kwToMatch.size(); --i >= 0; )
        {
            if (kwToMatch[i] == text)
            {
                bt->setToggleState (true, dontSendNotification);
                return;
            }
        }
    }

    //=================================================================================================
    const String getTextWithoutTimes (Button* bt)
    {
        return bt->getButtonText().upToFirstOccurrenceOf (" (", false, true).trim();
    }

    //=================================================================================================
    const bool displayInEditor;
    TextButtonLF tlf;
    OwnedArray<TextButton> bts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsButtons)
};

//==============================================================================
KeywordsComp::KeywordsComp (const ValueTree& tree, 
                            const bool displayInEditor,
                            const StringArray& keywordsToMatch)
{
    viewport = new Viewport();
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (10);

    KeywordsButtons* bts = new KeywordsButtons (tree, displayInEditor, keywordsToMatch);
    viewport->setViewedComponent (bts);
    addAndMakeVisible (viewport);

    titleLb.setFont (18.f);
    titleLb.setJustificationType (Justification::centred);
    titleLb.setText ((displayInEditor ? TRANS ("Project ") : tree.getProperty ("title").toString() + TRANS ("\'s "))
                     + TRANS ("Keywords Table") + " (" + String (bts->getKeywordsNumbers()) + ")", 
                     dontSendNotification);

    addAndMakeVisible (titleLb);
    setSize (550, 340);
}

//=================================================================================================
KeywordsComp::~KeywordsComp()
{
}
//=================================================================================================
void KeywordsComp::paint (Graphics& g)
{
    g.fillAll (Colour (0xffdcdbdb));
}

//=================================================================================================
void KeywordsComp::resized()
{
    titleLb.setBounds (10, 3, getWidth() - 20, 30);
    viewport->setBounds (0, 35, getWidth(), getHeight() - 35);
}

//=================================================================================================
ActionBroadcaster* KeywordsComp::getKeywordsPicker()
{
    return dynamic_cast<ActionBroadcaster*>(viewport->getViewedComponent());
}

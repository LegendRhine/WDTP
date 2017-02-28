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
    KeywordsButtons (const bool showInEditor, 
                     const StringArray& kwToMatch) :
        displayInEditor (showInEditor)
    {
        TopToolBar::rebuildAllKeywords (false);        

        // get all keywords of this project
        const ValueTree& ptree (FileTreeContainer::projectTree);
        const String& allKws (ptree.getProperty ("allKeywords").toString());

        StringArray keywords;
        keywords.addTokens (allKws, ",", String());

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

            setButtonToggle (bt, kwToMatch);

            bt->addListener (this);
            addAndMakeVisible (bt);
            bts.insert (0, bt);
        }

        setSize (550, keywords.size() / 5 * 30 + 10/* 
                 + (keywords.size() % 5 == 0) ? 35 : 0*/);
    }

    //=================================================================================================
    ~KeywordsButtons()     {    }

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

    }

    //=================================================================================================
private:
    void setButtonToggle (TextButton* bt, const StringArray& kwToMatch)
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
    const String getTextWithoutTimes (TextButton* bt)
    {
        return bt->getButtonText().upToFirstOccurrenceOf (" (", false, true);
    }

    //=================================================================================================
    const bool displayInEditor;
    TextButtonLF tlf;
    OwnedArray<TextButton> bts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsButtons)
};

//==============================================================================
KeywordsComp::KeywordsComp (const bool displayInEditor,
                            const StringArray& keywordsToMatch)
{
    titleLb.setFont (17.f);
    titleLb.setJustificationType (Justification::centred);
    titleLb.setText (TRANS ("Reuse from Keywords Table") + " - "
                     + TRANS ("Click to Pick/Nonuse it"), dontSendNotification);
    addAndMakeVisible (titleLb);

    viewport = new Viewport();
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (10);
    viewport->setViewedComponent (new KeywordsButtons (displayInEditor, keywordsToMatch));

    addAndMakeVisible (viewport);
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
    titleLb.setBounds (10, 5, getWidth() - 20, 30);
    viewport->setBounds (0, 35, getWidth(), getHeight() - 35);
}

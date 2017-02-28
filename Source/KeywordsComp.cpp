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
                        public Button::Listener
{
public:
    KeywordsButtons()
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
                                             + (keywords[i].contains ("--") ? ")" : String()),
                                             TRANS ("Click to Pick/Nonuse it"));
            bt->addListener (this);
            bt->setSize (90, 25);
            bt->setColour (TextButton::buttonColourId, Colours::lightgrey.withAlpha (0.15f));
            bt->setColour (TextButton::buttonOnColourId, Colours::grey);
            bt->setLookAndFeel (&tlf);
            addAndMakeVisible (bt);

            bts.insert (0, bt);
        }

        setSize (500, keywords.size () / 5 * 40 + 10/* 
                 + (keywords.size () % 5 == 0) ? 35 : 0*/);
    }

    //=================================================================================================
    ~KeywordsButtons () 
    {
    }

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

            bts[i]->setTopLeftPosition (x * 95 + 10, y * 35 + 5);
            ++x;
        }
    }

    //=================================================================================================
    virtual void buttonClicked (Button* bt) override
    {
    }

    //=================================================================================================
    virtual void buttonStateChanged (Button* bt) override
    {
    }

    //=================================================================================================
private:
    OwnedArray<TextButton> bts;
    TextButtonLF tlf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsButtons)
};

//==============================================================================
KeywordsComp::KeywordsComp ()
{
    titleLb.setFont (17.f);
    titleLb.setJustificationType (Justification::centred);
    titleLb.setText (TRANS ("Reuse from Keywords Table"), dontSendNotification);
    addAndMakeVisible (titleLb);

    viewport = new Viewport ();
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (10);
    viewport->setViewedComponent (new KeywordsButtons());

    addAndMakeVisible (viewport);
    setSize (500, 320);
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
    titleLb.setBounds (10, 5, getWidth () - 20, 30);
    viewport->setBounds (0, 35, getWidth (), getHeight () - 35);
}

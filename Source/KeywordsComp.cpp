/*
  ==============================================================================

    KeywordsComp.cpp
    Created: 28 Feb 2017 9:14:57am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

//=================================================================================================
class KeywordsButtons : public Component,
                        public Button::Listener
{
public:
    KeywordsButtons ()
    {

    }

    //=================================================================================================
    ~KeywordsButtons () 
    {
    }

    //=================================================================================================
    void resized() override
    {

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeywordsButtons)
};

//==============================================================================
KeywordsComp::KeywordsComp()
{
    viewport = new Viewport ();
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (10);
    viewport->setViewedComponent (new KeywordsButtons());

    setSize (500, 300);
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
    viewport->setBounds (0, 0, getWidth (), getHeight ());
}

/*
  ==============================================================================

    PropertiesPanel.cpp
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

//==============================================================================
SystemSetupPanel::SystemSetupPanel()
{
    
}

SystemSetupPanel::~SystemSetupPanel()
{
}

void SystemSetupPanel::paint (Graphics& g)
{
    g.setColour (Colours::black);
    g.setFont (18.0f);
    g.drawText ("SystemSetupPanel", getLocalBounds(), Justification::centred, true);
}

void SystemSetupPanel::resized()
{
}

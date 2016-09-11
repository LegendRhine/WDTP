/*
  ==============================================================================

    PropertiesPanel.cpp
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

extern PropertiesFile* systemFile;

//==============================================================================
SystemSetupPanel::SystemSetupPanel() :
    language (systemFile->getValue ("language")), 
    skin (systemFile->getValue ("skin")), 
    clickForEdit (systemFile->getValue ("clickForEdit")),
    fontSize (systemFile->getDoubleValue ("fontSize"))
{
    // section 0: system setup
    Array<PropertyComponent*> systemProperties;

    // language
    StringArray lanSa;
    lanSa.add (TRANS ("English"));
    lanSa.add (TRANS ("Simplified Chinese"));

    Array<var> lanVar;
    lanVar.add ("English");
    lanVar.add ("Simplified Chinese");

    systemProperties.add (new ChoicePropertyComponent (language, TRANS ("Language: "), lanSa, lanVar));

    // skin
    StringArray skinSa;
    skinSa.add (TRANS ("Elegance"));
    skinSa.add (TRANS ("Meditation"));

    Array<var> skinVar;
    skinVar.add ("Elegance");
    skinVar.add ("Meditation");

    systemProperties.add (new ChoicePropertyComponent (skin, TRANS ("Skin scheme: "), skinSa, skinVar));

    // click a doc inside the file-tree
    StringArray clickSa;
    clickSa.add (TRANS ("Edit"));
    clickSa.add (TRANS ("Preview"));

    Array<var> clickVar;
    clickVar.add ("Edit");
    clickVar.add ("Preview");

    systemProperties.add (new ChoicePropertyComponent (clickForEdit, TRANS ("Doc click: "), clickSa, clickVar));

    // font size
    systemProperties.add (new SliderPropertyComponent (fontSize, TRANS ("Editor font: "), 12.0, 32.0, 0.1));

    // the properties panel
    addAndMakeVisible (panel = new PropertyPanel());
    panel->addSection (TRANS ("System Setup"), systemProperties);
}

//=========================================================================
SystemSetupPanel::~SystemSetupPanel()
{
}

//=========================================================================
void SystemSetupPanel::resized()
{
    panel->setBounds (0, 0, getWidth() - 2, getHeight());
}

//=================================================================================================
void SystemSetupPanel::valueChanged (Value& value)
{
    // TODO...
}

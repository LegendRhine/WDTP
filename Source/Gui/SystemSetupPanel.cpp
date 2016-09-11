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
    systemSkin (systemFile->getValue ("skin")), 
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

    systemProperties.add (new ChoicePropertyComponent (systemSkin, TRANS ("System skin: "), skinSa, skinVar));

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

    // section 1: project setup
    Array<PropertyComponent*> projectProperties;
    projectProperties.add (new TextPropertyComponent (projectName, TRANS("Project name: "), 60, false));
    projectProperties.add (new TextPropertyComponent (projectDesc, TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (owner, TRANS ("Owner: "), 30, false));
    projectProperties.add (new ChoicePropertyComponent (projectSkin, TRANS ("Project skin: "), skinSa, skinVar));

    // themes dirs
    StringArray themeDirsSa;
    Array<var> themeDirsVar;

    if (FileTreeContainer::projectFile.existsAsFile())
    {
        const File themeDir (FileTreeContainer::projectFile.getSiblingFile ("themes"));
        Array<File> themeDirs;
        themeDir.findChildFiles (themeDirs, File::findDirectories, false);

        for (int i = 0; i < themeDirs.size (); ++i)
        {
            themeDirsSa.add (themeDirs.getUnchecked (i).getFileName ());
            themeDirsVar.add (themeDirs.getUnchecked (i).getFileName ());
        }
    }

    projectProperties.add (new ChoicePropertyComponent (render, TRANS ("Template: "), themeDirsSa, themeDirsVar));
    projectProperties.add (new TextPropertyComponent (place, TRANS ("Render to: "), 60, false));

    // section 2: dir setup

    // section 3: doc setup

    // properties panel add sections
    addAndMakeVisible (panel = new PropertyPanel());
    panel->addSection (TRANS ("System Setup"), systemProperties);
    panel->addSection (TRANS ("Project Setup"), projectProperties);
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

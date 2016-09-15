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
SetupPanel::SetupPanel() :
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

    // click a doc inside the file-tree
    StringArray clickSa;
    clickSa.add (TRANS ("Edit"));
    clickSa.add (TRANS ("Preview"));

    Array<var> clickVar;
    clickVar.add ("Edit");
    clickVar.add ("Preview");

    systemProperties.add (new ChoicePropertyComponent (clickForEdit, TRANS ("Doc Click: "), clickSa, clickVar));
    systemProperties.add (new SliderPropertyComponent (fontSize, TRANS ("Editor Font: "), 12.0, 32.0, 0.1));

    // section 1: project setup
    Array<PropertyComponent*> projectProperties;
    projectProperties.add (new TextPropertyComponent (projectName, TRANS("Project Name: "), 60, false));
    projectProperties.add (new TextPropertyComponent (projectDesc, TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (owner, TRANS ("Owner: "), 30, false));

    // skin
    StringArray skinSa;
    skinSa.add (TRANS ("Elegance"));
    skinSa.add (TRANS ("Meditation"));

    Array<var> skinVar;
    skinVar.add ("Elegance");
    skinVar.add ("Meditation");

    projectProperties.add (new ChoicePropertyComponent (projectSkin, TRANS ("Project Skin: "), skinSa, skinVar));

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

    projectProperties.add (new ChoicePropertyComponent (projectRenderDir, TRANS ("Template: "), themeDirsSa, themeDirsVar));
    projectProperties.add (new TextPropertyComponent (place, TRANS ("Render To: "), 60, false));
    projectProperties.add (new TextPropertyComponent (ftpAddress, TRANS ("FTP URL: "), 60, false));
    projectProperties.add (new TextPropertyComponent (ftpPort, TRANS ("FTP Port: "), 6, false));
    projectProperties.add (new TextPropertyComponent (ftpUserName, TRANS ("FTP Account: "), 60, false));
    projectProperties.add (new TextPropertyComponent (ftpPassword, TRANS ("FTP Password: "), 60, false));

    // section 2: dir setup
    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (dirDesc, TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent(isMenu, TRANS("Web Menu: "), TRANS("Yes")));
    dirProperties.add (new ChoicePropertyComponent (dirRenderDir, TRANS ("Template: "), themeDirsSa, themeDirsVar));
    dirProperties.add (new TextPropertyComponent (dirWebName, TRANS ("Web Name: "), 60, false));

    // section 3: doc setup
    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (title, TRANS ("Title: "), 80, false));
    docProperties.add (new TextPropertyComponent (author, TRANS ("Author: "), 30, false));
    docProperties.add (new TextPropertyComponent (createTime, TRANS ("Create Time: "), 25, false));
    docProperties.add (new TextPropertyComponent (modifyTime, TRANS ("Last Modified: "), 25, false));
    docProperties.add (new TextPropertyComponent (words, TRANS ("Words: "), 10, false));
    docProperties.add (new BooleanPropertyComponent (publish, TRANS ("Publish: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (docWebName, TRANS ("Web Name: "), 80, false));
    docProperties.add (new TextPropertyComponent (tplFile, TRANS ("Template File: "), 60, false));
    docProperties.add (new TextPropertyComponent (js, TRANS ("Java Script: "), 0, true));

    // properties panel add sections
    addAndMakeVisible (panel = new PropertyPanel());
    panel->getViewport ().setScrollBarThickness (10);
    panel->addSection (TRANS ("System Setup"), systemProperties);
    panel->addSection (TRANS ("Project Setup"), projectProperties);
    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    panel->addSection (TRANS ("Document Setup"), docProperties);
}

//=========================================================================
SetupPanel::~SetupPanel()
{
}

//=========================================================================
void SetupPanel::resized()
{
    // if the panel's width less than 90, it'll hit a jassert when in Debug mode, so...
    panel->setVisible (getWidth () > 80);
    panel->setBounds (getLocalBounds ());
}

//=================================================================================================
void SetupPanel::valueChanged (Value& value)
{
    // TODO...
}

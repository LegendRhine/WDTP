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
SetupPanel::SetupPanel()
{
    jassert (systemFile != nullptr);

    for (int i = totalValues; --i >= 0; )
    {
        values.add (new Value());
    }
    
    // assign system properties
    values[language]->setValue (systemFile->getValue ("language"));
    values[clickForEdit]->setValue (systemFile->getValue ("clickForEdit"));
    values[fontSize]->setValue (systemFile->getDoubleValue ("fontSize"));

    // 4 sections
    Array<PropertyComponent*> systemProperties;
    Array<PropertyComponent*> projectProperties;
    Array<PropertyComponent*> dirProperties;
    Array<PropertyComponent*> docProperties;

    // section 0: system setup
    // language
    StringArray lanSa;
    lanSa.add (TRANS ("English"));
    lanSa.add (TRANS ("Simplified Chinese"));

    Array<var> lanVar;
    lanVar.add ("English");
    lanVar.add ("Simplified Chinese");

    systemProperties.add (new ChoicePropertyComponent (*values[language], TRANS ("Language: "), 
                                                       lanSa, lanVar));

    // click a doc inside the file-tree
    StringArray clickSa;
    clickSa.add (TRANS ("Edit"));
    clickSa.add (TRANS ("Preview"));

    Array<var> clickVar;
    clickVar.add ("Edit");
    clickVar.add ("Preview");

    systemProperties.add (new ChoicePropertyComponent (*values[clickForEdit], TRANS ("Doc Click: "), 
                                                       clickSa, clickVar));

    struct SliderProperty : public SliderPropertyComponent
    {
        SliderProperty (const Value& v) : 
            SliderPropertyComponent (v, TRANS ("Editor Font: "), 12.0, 32.0, 0.1)
        {     
            slider.setChangeNotificationOnlyOnRelease (true);
        }

        double getValue () const
        {
            if (!slider.isMouseButtonDown())
                return slider.getValue ();

            return 20.0;
        }

    };

    systemProperties.add (new SliderProperty (*values[fontSize]));

    // section 1: project setup
    projectProperties.add (new TextPropertyComponent (*values[projectName], TRANS("Project Name: "), 60, false));
    projectProperties.add (new TextPropertyComponent (*values[projectDesc], TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[owner], TRANS ("Owner: "), 30, false));

    // skin
    StringArray skinSa;
    skinSa.add (TRANS ("Elegance"));
    skinSa.add (TRANS ("Meditation"));

    Array<var> skinVar;
    skinVar.add ("Elegance");
    skinVar.add ("Meditation");

    projectProperties.add (new ChoicePropertyComponent (*values[projectSkin], TRANS ("Project Skin: "), skinSa, skinVar));

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

    projectProperties.add (new ChoicePropertyComponent (*values[projectRenderDir], TRANS ("Template: "), 
                                                        themeDirsSa, themeDirsVar));
    projectProperties.add (new TextPropertyComponent (*values[place], TRANS ("Render To: "), 60, false));
    projectProperties.add (new TextPropertyComponent (*values[domain], TRANS ("Domain: "), 100, false));
    projectProperties.add (new TextPropertyComponent (*values[ftpAddress], TRANS ("FTP URL: "), 60, false));
    projectProperties.add (new TextPropertyComponent (*values[ftpPort], TRANS ("FTP Port: "), 6, false));
    projectProperties.add (new TextPropertyComponent (*values[ftpUserName], TRANS ("FTP Account: "), 60, false));
    projectProperties.add (new TextPropertyComponent (*values[ftpPassword], TRANS ("FTP Password: "), 60, false));

    // section 2: dir setup
    dirProperties.add (new TextPropertyComponent (*values[dirDesc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent(*values[isMenu], TRANS("Web Menu: "), TRANS("Yes")));
    dirProperties.add (new ChoicePropertyComponent (*values[dirRenderDir], TRANS ("Template: "), 
                                                    themeDirsSa, themeDirsVar));
    dirProperties.add (new TextPropertyComponent (*values[dirWebName], TRANS ("Web Name: "), 60, false));

    // section 3: doc setup    
    docProperties.add (new TextPropertyComponent (*values[title], TRANS ("Title: "), 80, false));
    docProperties.add (new TextPropertyComponent (*values[author], TRANS ("Author: "), 30, false));
    docProperties.add (new BooleanPropertyComponent (*values[publish], TRANS ("Publish: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[docWebName], TRANS ("Web Name: "), 80, false));
    docProperties.add (new TextPropertyComponent (*values[tplFile], TRANS ("Template File: "), 60, false));
    docProperties.add (new TextPropertyComponent (*values[js], TRANS ("Java Script: "), 0, true));

    const int h = 28; // set property-component's height
    for (auto p : systemProperties)   p->setPreferredHeight (h);
    for (auto p : projectProperties)  p->setPreferredHeight (h);
    for (auto p : dirProperties)      p->setPreferredHeight (h);
    for (auto p : docProperties)      p->setPreferredHeight (h);

    // properties panel add sections
    addAndMakeVisible (panel = new PropertyPanel());
    panel->getViewport().setScrollBarThickness (10);   

    panel->addSection (TRANS ("System Setup"), systemProperties);
    panel->addSection (TRANS ("Project Setup"), projectProperties);
    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    panel->addSection (TRANS ("Document Setup"), docProperties);

    showSystemProperties();
}

//=========================================================================
SetupPanel::~SetupPanel()
{
    valuesRemoveListener();
    stopTimer();
}

//=================================================================================================
void SetupPanel::showSystemProperties()
{
    valuesRemoveListener ();

    panel->setSectionEnabled (0, true);
    panel->setSectionEnabled (1, false);
    panel->setSectionEnabled (2, false);
    panel->setSectionEnabled (3, false);

    panel->setSectionOpen (0, true);
    panel->setSectionOpen (1, false);
    panel->setSectionOpen (2, false);
    panel->setSectionOpen (3, false);

    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showProjectProperties (ValueTree& pTree)
{
    valuesRemoveListener ();

    panel->setSectionEnabled (0, true);
    panel->setSectionEnabled (1, true);
    panel->setSectionEnabled (2, false);
    panel->setSectionEnabled (3, false);

    panel->setSectionOpen (0, false);
    panel->setSectionOpen (1, true);
    panel->setSectionOpen (2, false);
    panel->setSectionOpen (3, false);

    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDirProperties (ValueTree& dirTree)
{
    valuesRemoveListener ();

    panel->setSectionEnabled (0, true);
    panel->setSectionEnabled (1, true);
    panel->setSectionEnabled (2, true);
    panel->setSectionEnabled (3, false);

    panel->setSectionOpen (0, false);
    panel->setSectionOpen (1, false);
    panel->setSectionOpen (2, true);
    panel->setSectionOpen (3, false);

    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDocProperties (ValueTree& docTree)
{
    valuesRemoveListener ();

    panel->setSectionEnabled (0, true);
    panel->setSectionEnabled (1, true);
    panel->setSectionEnabled (2, true);
    panel->setSectionEnabled (3, true);

    panel->setSectionOpen (0, false);
    panel->setSectionOpen (1, false);
    panel->setSectionOpen (2, false);
    panel->setSectionOpen (3, true);

    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::projectClosed()
{
    stopTimer();
    savePropertiesIfNeeded();
    valuesRemoveListener ();

    projectTree = ValueTree::invalid;
    projectHasChanged = false;

    for (int i = projectName; i < values.size(); ++i)
        values[i]->setValue (var (0));

    panel->setSectionEnabled (0, true);
    panel->setSectionEnabled (1, false);
    panel->setSectionEnabled (2, false);
    panel->setSectionEnabled (3, false);

    panel->setSectionOpen (0, true);
    panel->setSectionOpen (1, false);
    panel->setSectionOpen (2, false);
    panel->setSectionOpen (3, false);

    valuesAddListener();
}

//=================================================================================================
void SetupPanel::valuesAddListener ()
{
    for (auto v : values)
        v->addListener (this);
}

//=================================================================================================
void SetupPanel::valuesRemoveListener ()
{
    for (auto v : values)
        v->removeListener (this);
}

//=========================================================================
void SetupPanel::resized()
{
    // if the panel's width less than 90, it'll hit a jassert when in Debug mode, so...
    panel->setVisible (getWidth() > 80);
    panel->setBounds (getLocalBounds().reduced (2));
}

//=========================================================================
void SetupPanel::valueChanged (Value & value)
{
    if (value == *values[language])
    {
        systemFile->setValue ("language", value);
    }
    else if (value == *values[clickForEdit])
    {
        systemFile->setValue ("clickForEdit", value);
    }    
    else if (value == *values[fontSize])
    {
        systemFile->setValue ("fontSize", value);
    }    
    else
    {
        projectHasChanged = true;
    }

    startTimer (2000);
}

//=================================================================================================
void SetupPanel::timerCallback ()
{
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::savePropertiesIfNeeded ()
{
//     static int i = 0;
//     DBG (++i);

    systemFile->saveIfNeeded ();

    if (projectHasChanged &&
        FileTreeContainer::projectFile.existsAsFile () &&
        !SwingUtilities::writeValueTreeToFile (projectTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));

    projectHasChanged = false;
    stopTimer ();
}



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
    for (int i = totalValues; --i >= 0; )   values.add (new Value());
     
    addAndMakeVisible (panel = new PropertyPanel());
    panel->getViewport().setScrollBarThickness (10);   
    showSystemProperties();
}

//=========================================================================
SetupPanel::~SetupPanel()
{
    valuesRemoveListener();
    stopTimer();
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::showSystemProperties()
{
    valuesRemoveListener ();
    panel->clear ();

    // assign system properties
    values[language]->setValue (var (systemFile->getValue ("language")));
    values[clickForEdit]->setValue (var (systemFile->getValue ("clickForEdit")));
    values[fontSize]->setValue (var (systemFile->getDoubleValue ("fontSize")));

    Array<PropertyComponent*> systemProperties;

    // language
    StringArray lanSa;
    lanSa.add (TRANS ("English"));
    lanSa.add (TRANS ("Simplified Chinese"));

    Array<var> lanVar;
    lanVar.add ("English");
    lanVar.add ("Simplified Chinese");
    systemProperties.add (new ChoicePropertyComponent (*values[language], TRANS ("Language: "), lanSa, lanVar));

    // click a doc inside the file-tree
    StringArray clickSa;
    clickSa.add (TRANS ("Edit"));
    clickSa.add (TRANS ("Preview"));

    Array<var> clickVar;
    clickVar.add ("Edit");
    clickVar.add ("Preview");

    systemProperties.add (new ChoicePropertyComponent (*values[clickForEdit], TRANS ("Doc Click: "), clickSa, clickVar));
    systemProperties.add (new SliderPropertyComponent (*values[fontSize], TRANS ("Editor Font: "), 12.0, 32.0, 0.1));
        
    for (auto p : systemProperties)   p->setPreferredHeight (28);

    panel->addSection (TRANS ("System Setup"), systemProperties);
    systemSetupShowing = true;
    valuesAddListener (language, fontSize);
}

//=================================================================================================
void SetupPanel::showProjectProperties (ValueTree& pTree)
{
    valuesRemoveListener ();
    projectTree = pTree;
    panel->clear ();
    systemSetupShowing = false;
    jassert (projectTree.isValid () && projectTree.getType ().toString () == "wdtpProject");

    values[projectName]->setValue (pTree.getProperty ("name"));
    values[projectDesc]->setValue (pTree.getProperty ("title"));
    values[owner]->setValue (pTree.getProperty ("owner"));
    values[projectSkin]->setValue (pTree.getProperty ("skin"));
    values[projectRenderDir]->setValue (pTree.getProperty ("render"));
    values[place]->setValue (pTree.getProperty ("place"));
    values[domain]->setValue (pTree.getProperty ("domain"));
    values[ftpAddress]->setValue (pTree.getProperty ("ftpAddress"));
    values[ftpPort]->setValue (pTree.getProperty ("ftpPort"));
    values[ftpUserName]->setValue (pTree.getProperty ("ftpUserName"));
    values[ftpPassword]->setValue (pTree.getProperty ("ftpPassword"));

    Array<PropertyComponent*> projectProperties;

    projectProperties.add (new TextPropertyComponent (*values[projectName], TRANS ("Project Name: "), 60, false));
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

    if (FileTreeContainer::projectFile.existsAsFile ())
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

    for (auto p : projectProperties)  p->setPreferredHeight (28);
    projectProperties[1]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Project Setup"), projectProperties);
    valuesAddListener (projectName, ftpPassword);
}

//=================================================================================================
void SetupPanel::showDirProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    dirTree = dTree;
    systemSetupShowing = false;
    jassert (dirTree.isValid () && dirTree.getType ().toString () == "dir");

    values[dirDesc]->setValue (dirTree.getProperty ("title"));
    values[isMenu]->setValue (dirTree.getProperty ("isMenu"));
    values[dirRenderDir]->setValue (dirTree.getProperty ("render"));
    values[dirWebName]->setValue (dirTree.getProperty ("webName"));

    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (*values[dirDesc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Web Menu: "), TRANS ("Yes")));

    // themes dirs
    StringArray themeDirsSa;
    Array<var> themeDirsVar;

    if (FileTreeContainer::projectFile.existsAsFile ())
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

    dirProperties.add (new ChoicePropertyComponent (*values[dirRenderDir], TRANS ("Template: "),
                                                    themeDirsSa, themeDirsVar));
    dirProperties.add (new TextPropertyComponent (*values[dirWebName], TRANS ("Web Name: "), 60, false));

    for (auto p : dirProperties)      p->setPreferredHeight (28);
    dirProperties[0]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    valuesAddListener (dirDesc, dirWebName);
}

//=================================================================================================
void SetupPanel::showDocProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    docTree = dTree;
    systemSetupShowing = false;
    jassert (docTree.isValid () && docTree.getType ().toString () == "doc");

    values[title]->setValue (docTree.getProperty ("title"));
    values[author]->setValue (docTree.getProperty ("author"));
    values[publish]->setValue (docTree.getProperty ("publish"));
    values[docWebName]->setValue (docTree.getProperty ("webName"));
    values[tplFile]->setValue (docTree.getProperty ("tplFile"));
    values[js]->setValue (docTree.getProperty ("js"));

    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (*values[title], TRANS ("Title: "), 80, false));
    docProperties.add (new TextPropertyComponent (*values[author], TRANS ("Author: "), 30, false));
    docProperties.add (new BooleanPropertyComponent (*values[publish], TRANS ("Publish: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[docWebName], TRANS ("Web Name: "), 80, false));
    docProperties.add (new TextPropertyComponent (*values[tplFile], TRANS ("Template File: "), 60, false));
    docProperties.add (new TextPropertyComponent (*values[js], TRANS ("Java Script: "), 0, true));

    for (auto p : docProperties)      p->setPreferredHeight (28);
    docProperties[5]->setPreferredHeight (28 * 4);

    panel->addSection (TRANS ("Document Setup"), docProperties);
    valuesAddListener (title, js);
}

//=================================================================================================
void SetupPanel::projectClosed()
{
    stopTimer();
    savePropertiesIfNeeded();
    valuesRemoveListener ();

    projectTree = ValueTree::invalid;
    dirTree = ValueTree::invalid;
    docTree = ValueTree::invalid;

    projectHasChanged = false;
    panel->clear();
    showSystemProperties();
}

//=================================================================================================
void SetupPanel::valuesAddListener (const int startIndex, const int endIndex)
{
    for (int i = startIndex; i <= endIndex; ++i)
        values[i]->addListener (this);
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
void SetupPanel::valueChanged (Value& value)
{
    // system properties
    if (value.refersToSameSourceAs (*values[language]))
        systemFile->setValue ("language", value.toString());
    else if (value.refersToSameSourceAs (*values[clickForEdit]))
        systemFile->setValue ("clickForEdit", value.toString());
    else if (value.refersToSameSourceAs (*values[fontSize]))
        systemFile->setValue ("fontSize", value.getValue());
    else
    {
        projectHasChanged = true;

        // project properties
        if (value.refersToSameSourceAs (*values[projectName]))
            projectTree.setProperty ("name", values[projectName]->getValue(), nullptr);
        else if (value.refersToSameSourceAs (*values[projectDesc]))
            projectTree.setProperty ("title", values[projectDesc]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[owner]))
            projectTree.setProperty ("owner", values[owner]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[projectSkin]))
            projectTree.setProperty ("skin", values[projectSkin]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[projectRenderDir]))
            projectTree.setProperty ("render", values[projectRenderDir]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[place]))
            projectTree.setProperty ("place", values[place]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[domain]))
            projectTree.setProperty ("domain", values[domain]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[ftpAddress]))
            projectTree.setProperty ("ftpAddress", values[ftpAddress]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[ftpPort]))
            projectTree.setProperty ("ftpPort", values[ftpPort]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[ftpUserName]))
            projectTree.setProperty ("ftpUserName", values[ftpUserName]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[ftpPassword]))
            projectTree.setProperty ("ftpPassword", values[ftpPassword]->getValue (), nullptr);

        // dir properties
        else if (value.refersToSameSourceAs (*values[dirDesc]))
            dirTree.setProperty ("title", values[dirDesc]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[isMenu]))
            dirTree.setProperty ("isMenu", values[isMenu]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[dirRenderDir]))
            dirTree.setProperty ("render", values[dirRenderDir]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[dirWebName]))
            dirTree.setProperty ("webName", values[dirWebName]->getValue (), nullptr);

        // doc properties
        else if (value.refersToSameSourceAs (*values[title]))
            docTree.setProperty ("title", values[title]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[author]))
            docTree.setProperty ("author", values[author]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[publish]))
            docTree.setProperty ("publish", values[publish]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[docWebName]))
            docTree.setProperty ("webName", values[docWebName]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[tplFile]))
            docTree.setProperty ("tplFile", values[tplFile]->getValue (), nullptr);
        else if (value.refersToSameSourceAs (*values[js]))
            docTree.setProperty ("js", values[js]->getValue (), nullptr);
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

    if (systemFile != nullptr)
        systemFile->saveIfNeeded ();

    if (projectHasChanged && projectTree.isValid() && 
        FileTreeContainer::projectFile.existsAsFile () &&
        !SwingUtilities::writeValueTreeToFile (projectTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving project."));

    projectHasChanged = false;
    stopTimer ();
}



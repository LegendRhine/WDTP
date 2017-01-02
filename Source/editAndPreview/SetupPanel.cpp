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
SetupPanel::SetupPanel (EditAndPreview* ed) :
    editor (ed)
{
    jassert (systemFile != nullptr);
    for (int i = totalValues; --i >= 0; )   values.add (new Value());
     
    addAndMakeVisible (panel = new PropertyPanel());
    panel->getViewport().setScrollBarThickness (10);   
}

//=========================================================================
SetupPanel::~SetupPanel()
{
    valuesRemoveListener();
    stopTimer();
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::showProjectProperties (ValueTree& pTree)
{
    valuesRemoveListener ();
    projectTree = pTree;
    panel->clear ();
    jassert (projectTree.isValid () && projectTree.getType ().toString () == "wdtpProject");

    values[projectName]->setValue (pTree.getProperty ("name"));
    values[projectDesc]->setValue (pTree.getProperty ("title"));
    values[owner]->setValue (pTree.getProperty ("owner"));
    //values[projectSkin]->setValue (pTree.getProperty ("skin"));
    values[projectRenderDir]->setValue (pTree.getProperty ("render"));
    values[fontSize]->setValue (pTree.getProperty ("fontSize"));

    Array<PropertyComponent*> projectProperties;

    projectProperties.add (new TextPropertyComponent (*values[projectName], TRANS ("Project Name: "), 60, false));
    projectProperties.add (new TextPropertyComponent (*values[projectDesc], TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[owner], TRANS ("Owner: "), 30, false));

    // skin
    /*StringArray skinSa;
    skinSa.add (TRANS ("Elegance"));
    skinSa.add (TRANS ("Meditation"));

    Array<var> skinVar;
    skinVar.add ("Elegance");
    skinVar.add ("Meditation");

    projectProperties.add (new ChoicePropertyComponent (*values[projectSkin], TRANS ("Project Skin: "), skinSa, skinVar));*/

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
    projectProperties.add (new SliderPropertyComponent (*values[fontSize], TRANS ("Editor Font: "), 12.0, 60.0, 0.1));

    for (auto p : projectProperties)  p->setPreferredHeight (28);
    projectProperties[1]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Project Setup"), projectProperties);
    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDirProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    dirTree = dTree;
    jassert (dirTree.isValid () && dirTree.getType ().toString () == "dir");

    values[dirDesc]->setValue (dirTree.getProperty ("title"));
    values[isMenu]->setValue (dirTree.getProperty ("isMenu"));

    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (*values[dirDesc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Web Menu: "), TRANS ("Yes")));
    
    for (auto p : dirProperties)     p->setPreferredHeight (28);
    dirProperties[0]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDocProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    docTree = dTree;
    jassert (docTree.isValid () && docTree.getType ().toString () == "doc");

    values[keywords]->setValue (docTree.getProperty ("keywords"));
    values[isPage]->setValue (docTree.getProperty ("isPage"));
    values[js]->setValue (docTree.getProperty ("js"));

    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (*values[keywords], TRANS ("Keywords: "), 120, false));
    docProperties.add (new BooleanPropertyComponent (*values[isPage], TRANS ("Single Page: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[js], TRANS ("Java Script: "), 0, true));

    for (auto p : docProperties)   p->setPreferredHeight (28);
    docProperties[docProperties.size() - 1]->setPreferredHeight (28 * 4);

    panel->addSection (TRANS ("Document Setup"), docProperties);
    valuesAddListener ();
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
void SetupPanel::valueChanged (Value& value)
{    
    // project properties
    if (value.refersToSameSourceAs (*values[projectName]))
        projectTree.setProperty ("name", values[projectName]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[projectDesc]))
        projectTree.setProperty ("title", values[projectDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[owner]))
        projectTree.setProperty ("owner", values[owner]->getValue (), nullptr);
    /*else if (value.refersToSameSourceAs (*values[projectSkin]))
        projectTree.setProperty ("skin", values[projectSkin]->getValue (), nullptr);*/
    else if (value.refersToSameSourceAs (*values[projectRenderDir]))
        projectTree.setProperty ("render", values[projectRenderDir]->getValue (), nullptr);

    else if (value.refersToSameSourceAs (*values[fontSize]))
    {
        const float fs = values[fontSize]->getValue ();

        editor->getEditor ()->applyFontToAllText (fs);
        FileTreeContainer::fontSize = fs;
        projectTree.setProperty ("fontSize", fs, nullptr);
    }

    // dir properties
    else if (value.refersToSameSourceAs (*values[dirDesc]))
        dirTree.setProperty ("title", values[dirDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isMenu]))
        dirTree.setProperty ("isMenu", values[isMenu]->getValue (), nullptr);

    // doc properties
    else if (value.refersToSameSourceAs (*values[keywords]))
        docTree.setProperty ("keywords", values[keywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isPage]))
        docTree.setProperty ("isPage", values[isPage]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[js]))
        docTree.setProperty ("js", values[js]->getValue (), nullptr);

    projectHasChanged = true;
    editor->needRecreateHtml (true);
    startTimer (500);
}

//=================================================================================================
void SetupPanel::timerCallback ()
{
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::savePropertiesIfNeeded ()
{
    if (systemFile != nullptr)
        systemFile->saveIfNeeded ();

    if (projectHasChanged && projectTree.isValid() && 
        FileTreeContainer::projectFile.existsAsFile() &&
        !SwingUtilities::writeValueTreeToFile (projectTree, FileTreeContainer::projectFile))
        SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));

    projectHasChanged = false;
    stopTimer ();
}


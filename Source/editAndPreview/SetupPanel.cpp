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
    currentTree = pTree;
    panel->clear ();
    jassert (currentTree.isValid () && currentTree.getType ().toString () == "wdtpProject");

    values[projectTitle]->setValue (pTree.getProperty ("title"));
    values[projectKeywords]->setValue (pTree.getProperty ("keywords"));
    values[projectDesc]->setValue (pTree.getProperty ("description"));
    values[owner]->setValue (pTree.getProperty ("owner"));
    values[copyrightInfo]->setValue (pTree.getProperty ("copyright"));
    //values[projectSkin]->setValue (pTree.getProperty ("skin"));
    values[projectRenderDir]->setValue (pTree.getProperty ("render"));
    values[indexTpl]->setValue (pTree.getProperty ("tplFile"));
    values[fontSize]->setValue (pTree.getProperty ("fontSize"));

    Array<PropertyComponent*> projectProperties;

    projectProperties.add (new TextPropertyComponent (*values[projectTitle], TRANS ("Title: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[projectKeywords], TRANS ("Keywords: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[projectDesc], TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[owner], TRANS ("Owner: "), 30, false));
    projectProperties.add (new TextPropertyComponent (*values[copyrightInfo], TRANS ("Copyright: "), 0, true));

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

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile ())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile(pTree.getProperty("render").toString()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size (); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName ());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName ());
        }
    }

    projectProperties.add (new ChoicePropertyComponent (*values[indexTpl], TRANS ("Render TPL: "),
                                                        tplFileSa, tplFileVar));

    projectProperties.add (new SliderPropertyComponent (*values[fontSize], TRANS ("Editor Font: "), 12.0, 60.0, 0.1));

    for (auto p : projectProperties)  p->setPreferredHeight (28);
    projectProperties[2]->setPreferredHeight (28 * 3);
    projectProperties[4]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Project Setup"), projectProperties);
    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDirProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    currentTree = dTree;
    jassert (currentTree.isValid () && currentTree.getType ().toString () == "dir");

    values[dirName]->setValue (currentTree.getProperty ("name"));
    values[dirTitle]->setValue (currentTree.getProperty ("title"));
    values[dirKeywords]->setValue (currentTree.getProperty ("keywords"));
    values[dirDesc]->setValue (currentTree.getProperty ("description"));
    values[isMenu]->setValue (currentTree.getProperty ("isMenu"));
    values[dirTpl]->setValue (currentTree.getProperty ("tplFile"));
    values[dirDate]->setValue (currentTree.getProperty ("date"));

    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (*values[dirName], TRANS ("Name: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirTitle], TRANS ("Title: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirKeywords], TRANS ("Keywords: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirDesc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Web Menu: "), TRANS ("Yes")));
    dirProperties.add (new TextPropertyComponent (*values[dirDate], TRANS ("Date: "), 10, false));

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile ())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile (FileTreeContainer::projectTree.getProperty ("render").toString ()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size (); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName ());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName ());
        }
    }

    dirProperties.add (new ChoicePropertyComponent (*values[dirTpl], TRANS ("Render TPL: "),
                                                        tplFileSa, tplFileVar));

    for (auto p : dirProperties)
        p->setPreferredHeight (28);

    dirProperties[0]->setEnabled (false);
    dirProperties[3]->setPreferredHeight (28 * 3);
    
    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::showDocProperties (ValueTree& dTree)
{
    valuesRemoveListener ();
    panel->clear ();
    currentTree = dTree;
    jassert (currentTree.isValid () && currentTree.getType ().toString () == "doc");

    values[docName]->setValue (currentTree.getProperty ("name"));
    values[docTitle]->setValue (currentTree.getProperty ("title"));
    values[docKeywords]->setValue (currentTree.getProperty ("keywords"));
    values[docDesc]->setValue (currentTree.getProperty ("description"));
    values[isPage]->setValue (currentTree.getProperty ("isPage"));
    values[docTpl]->setValue (currentTree.getProperty ("tplFile"));
    values[docDate]->setValue (currentTree.getProperty ("date"));

    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (*values[docName], TRANS ("Name: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docTitle], TRANS ("Title: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docKeywords], TRANS ("Keywords: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docDesc], TRANS ("Description: "), 0, true));
    docProperties.add (new BooleanPropertyComponent (*values[isPage], TRANS ("Single Page: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[docDate], TRANS ("Date: "), 10, false));

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile ())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile (FileTreeContainer::projectTree.getProperty ("render").toString ()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size (); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName ());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName ());
        }
    }

    docProperties.add (new ChoicePropertyComponent (*values[docTpl], TRANS ("Render TPL: "),
                                                    tplFileSa, tplFileVar));

    for (auto p : docProperties)           
        p->setPreferredHeight (28);

    docProperties[0]->setEnabled (false);
    docProperties[1]->setEnabled (false);
    docProperties[3]->setPreferredHeight (28 * 3);

    panel->addSection (TRANS ("Document Setup"), docProperties);
    valuesAddListener ();
}

//=================================================================================================
void SetupPanel::projectClosed()
{
    stopTimer();
    savePropertiesIfNeeded();
    valuesRemoveListener ();

    currentTree = ValueTree::invalid;
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
    if (value.refersToSameSourceAs (*values[projectTitle]))
        currentTree.setProperty ("title", values[projectTitle]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[projectKeywords]))
        currentTree.setProperty ("keywords", values[projectKeywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[projectDesc]))
        currentTree.setProperty ("description", values[projectDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[owner]))
        currentTree.setProperty ("owner", values[owner]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[copyrightInfo]))
        currentTree.setProperty ("copyright", values[copyrightInfo]->getValue (), nullptr);
    /*else if (value.refersToSameSourceAs (*values[projectSkin]))
        currentTree.setProperty ("skin", values[projectSkin]->getValue (), nullptr);*/
    else if (value.refersToSameSourceAs (*values[projectRenderDir]))
        currentTree.setProperty ("render", values[projectRenderDir]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[indexTpl]))
        currentTree.setProperty ("tplFile", values[indexTpl]->getValue (), nullptr);

    else if (value.refersToSameSourceAs (*values[fontSize]))
    {
        const float fs = values[fontSize]->getValue ();

        editor->getEditor ()->applyFontToAllText (fs);
        FileTreeContainer::fontSize = fs;
        currentTree.setProperty ("fontSize", fs, nullptr);
    }

    // dir properties
    else if (value.refersToSameSourceAs (*values[dirTitle]))
        currentTree.setProperty ("title", values[dirTitle]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirKeywords]))
        currentTree.setProperty ("keywords", values[dirKeywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirDesc]))
        currentTree.setProperty ("description", values[dirDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isMenu]))
        currentTree.setProperty ("isMenu", values[isMenu]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirDate]))
        currentTree.setProperty ("date", values[dirDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirTpl]))
        currentTree.setProperty ("tplFile", values[dirTpl]->getValue (), nullptr);

    // doc properties
    else if (value.refersToSameSourceAs (*values[docKeywords]))
        currentTree.setProperty ("keywords", values[docKeywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docDesc]))
        currentTree.setProperty ("description", values[docDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isPage]))
        currentTree.setProperty ("isPage", values[isPage]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docDate]))
        currentTree.setProperty ("date", values[docDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docTpl]))
        currentTree.setProperty ("tplFile", values[docTpl]->getValue (), nullptr);

    DocTreeViewItem::needCreateHtml (currentTree);
    projectHasChanged = true;
    startTimer (200);
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

    if (projectHasChanged && currentTree.isValid() &&
        FileTreeContainer::projectFile.existsAsFile())
    {
        FileTreeContainer::saveProject ();
    }

    projectHasChanged = false;
    stopTimer ();
}


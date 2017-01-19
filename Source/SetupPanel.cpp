/*
  ==============================================================================

    PropertiesPanel.cpp
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

//==============================================================================
SetupPanel::SetupPanel (EditAndPreview* ed) :
    editor (ed)
{
    jassert (systemFile != nullptr);
    
    for (int i = totalValues; --i >= 0; )   
        values.add (new Value());
     
    addAndMakeVisible (panel = new PropertyPanel());
    panel->setMessageWhenEmpty(String());
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
    values[projectRenderDir]->setValue (pTree.getProperty ("render"));
    values[indexTpl]->setValue (pTree.getProperty ("tplFile"));
    values[projectJs]->setValue(pTree.getProperty("js"));
    
    Array<PropertyComponent*> projectProperties;

    projectProperties.add (new TextPropertyComponent (*values[projectTitle], TRANS ("Title: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[projectKeywords], TRANS ("Keywords: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[projectDesc], TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[owner], TRANS ("Owner: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[copyrightInfo], TRANS ("Copyright: "), 0, true));

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

    projectProperties.add (new TextPropertyComponent (*values[projectJs], TRANS ("JavaScript: "), 0, true));
    projectProperties.add(new TextPropertyComponent(Value(pTree.getProperty("modifyDate")), TRANS("Last Modified: "), 0, false));
	
    for (auto p : projectProperties)  p->setPreferredHeight (28);
    projectProperties[2]->setPreferredHeight (28 * 3);
    projectProperties[4]->setPreferredHeight (28 * 3);
    projectProperties[7]->setPreferredHeight (28 * 6);
    projectProperties[8]->setEnabled(false);
    
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
    values[dirJs]->setValue (currentTree.getProperty ("js"));
    values[dirCreateDate]->setValue (currentTree.getProperty ("createDate"));
	values[dirModifyDate]->setValue (currentTree.getProperty ("modifyDate"));
    
    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (*values[dirName], TRANS ("Name: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirTitle], TRANS ("Title: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirKeywords], TRANS ("Keywords: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirDesc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Web Menu: "), TRANS ("Yes")));
    dirProperties.add (new TextPropertyComponent (*values[dirCreateDate], TRANS ("Create Date: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[dirJs], TRANS ("JavaScript: "), 0, true));

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

    dirProperties.add (new ChoicePropertyComponent (*values[dirTpl], TRANS ("Render TPL: "), tplFileSa, tplFileVar));
    dirProperties.add (new TextPropertyComponent (*values[dirModifyDate], TRANS ("Last Modified: "), 0, false));

    for (auto p : dirProperties)
        p->setPreferredHeight (28);

    dirProperties[0]->setEnabled (false);
    dirProperties[3]->setPreferredHeight (28 * 3);
    dirProperties[6]->setPreferredHeight (28 * 6);
    
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
    values[docJs]->setValue (currentTree.getProperty ("js"));
    values[docCreateDate]->setValue (currentTree.getProperty ("createDate"));
	values[docModifyDate]->setValue (currentTree.getProperty ("modifyDate"));

    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (*values[docName], TRANS ("Name: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docTitle], TRANS ("Title: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docKeywords], TRANS ("Keywords: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docDesc], TRANS ("Description: "), 0, true));
    docProperties.add (new BooleanPropertyComponent (*values[isPage], TRANS ("Single Page: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[docCreateDate], TRANS ("Create Date: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[docJs], TRANS ("JavaScript: "), 0, true));

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
    docProperties.add (new TextPropertyComponent (*values[docModifyDate], TRANS ("Last Modified: "), 0, false));
	docProperties.add (new TextPropertyComponent (*values[wordCount], TRANS ("Word Count: "), 0, false));

    for (auto p : docProperties)           
        p->setPreferredHeight (28);

    docProperties[0]->setEnabled (false);
    docProperties[9]->setEnabled (false);

    docProperties[3]->setPreferredHeight (28 * 3);
    docProperties[6]->setPreferredHeight (28 * 6);

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

    values[wordCount]->removeListener(this);
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
    else if (value.refersToSameSourceAs (*values[projectRenderDir]))
        currentTree.setProperty ("render", values[projectRenderDir]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[indexTpl]))
        currentTree.setProperty ("tplFile", values[indexTpl]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[projectJs]))
        currentTree.setProperty ("js", values[projectJs]->getValue (), nullptr);
    
    // dir properties
    else if (value.refersToSameSourceAs (*values[dirTitle]))
        currentTree.setProperty ("title", values[dirTitle]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirKeywords]))
        currentTree.setProperty ("keywords", values[dirKeywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirDesc]))
        currentTree.setProperty ("description", values[dirDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isMenu]))
        currentTree.setProperty ("isMenu", values[isMenu]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirCreateDate]))
        currentTree.setProperty ("createDate", values[dirCreateDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirModifyDate]))
        currentTree.setProperty ("modifyDate", values[dirModifyDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirTpl]))
        currentTree.setProperty ("tplFile", values[dirTpl]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[dirJs]))
        currentTree.setProperty ("js", values[dirJs]->getValue (), nullptr);
	
    // doc properties
    else if (value.refersToSameSourceAs (*values[docTitle]))
        currentTree.setProperty ("title", values[docTitle]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docKeywords]))
        currentTree.setProperty ("keywords", values[docKeywords]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docDesc]))
        currentTree.setProperty ("description", values[docDesc]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[isPage]))
        currentTree.setProperty ("isPage", values[isPage]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docCreateDate]))
        currentTree.setProperty ("createDate", values[docCreateDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docModifyDate]))
        currentTree.setProperty ("modifyDate", values[docModifyDate]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docTpl]))
        currentTree.setProperty ("tplFile", values[docTpl]->getValue (), nullptr);
    else if (value.refersToSameSourceAs (*values[docJs]))
        currentTree.setProperty ("js", values[docJs]->getValue (), nullptr);
	
    DocTreeViewItem::needCreateAndUpload (currentTree);
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


/*
  ==============================================================================

    PropertiesPanel.cpp
    Created: 4 Sep 2016 12:27:36am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern ApplicationCommandManager* cmdManager;

//==============================================================================
SetupPanel::SetupPanel (EditAndPreview* ed) 
    : editorAndPreView (ed),
	projectHasChanged (false)
{
    addAndMakeVisible (panel = new PropertyPanel());
    panel->setMessageWhenEmpty (String());
    panel->getViewport().setScrollBarThickness (10);
}

//=========================================================================
SetupPanel::~SetupPanel()
{
    initialValues (true, false);
    stopTimer();
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::showProjectProperties (const ValueTree& pTree)
{
    initialValues (true, true);

    currentTree = pTree;
    panel->clear();
    jassert (currentTree.isValid() && currentTree.getType().toString() == "wdtpProject");

    values[itsTitle]->setValue (pTree.getProperty ("title"));
    values[keywords]->setValue (pTree.getProperty ("keywords"));
    values[desc]->setValue (pTree.getProperty ("description"));
    values[projectOwner]->setValue (pTree.getProperty ("owner"));
    values[copyrightInfo]->setValue (pTree.getProperty ("copyright"));
    values[renderDir]->setValue (pTree.getProperty ("render"));
    values[tplFile]->setValue (pTree.getProperty ("tplFile"));
    values[jsCode]->setValue (pTree.getProperty ("js"));
    values[ad]->setValue (pTree.getProperty ("ad"));
    values[contact]->setValue (pTree.getProperty ("contact"));
    values[modifyDate]->setValue (pTree.getProperty ("modifyDate"));
    values[resources]->setValue (pTree.getProperty ("resources"));

    Array<PropertyComponent*> projectProperties;
    projectProperties.add (new TextPropertyComponent (*values[itsTitle], TRANS ("Title: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[keywords], TRANS ("Keywords: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[desc], TRANS ("Description: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[projectOwner], TRANS ("Owner: "), 0, false));

    // themes dirs
    StringArray themeDirsSa;
    Array<var> themeDirsVar;

    if (FileTreeContainer::projectFile.existsAsFile())
    {
        const File themeDir (FileTreeContainer::projectFile.getSiblingFile ("themes"));
        Array<File> themeDirs;
        themeDir.findChildFiles (themeDirs, File::findDirectories, false);

        for (int i = 0; i < themeDirs.size(); ++i)
        {
            themeDirsSa.add (themeDirs.getUnchecked (i).getFileName());
            themeDirsVar.add (themeDirs.getUnchecked (i).getFileName());
        }
    }

    projectProperties.add (new ChoicePropertyComponent (*values[renderDir], TRANS ("Theme: "),
                                                        themeDirsSa, themeDirsVar));

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile (pTree.getProperty ("render").toString()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size(); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName());
        }
    }

    projectProperties.add (new ChoicePropertyComponent (*values[tplFile], TRANS ("Render TPL: "),
                                                        tplFileSa, tplFileVar));

    projectProperties.add (new TextPropertyComponent (*values[jsCode], TRANS ("Code: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[ad], TRANS ("Advertisement: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[contact], TRANS ("Contact Info: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[copyrightInfo], TRANS ("Copyright: "), 0, true));
    projectProperties.add (new TextPropertyComponent (*values[modifyDate], TRANS ("Last Modified: "), 0, false));
    projectProperties.add (new TextPropertyComponent (*values[resources], TRANS ("Ex-resources: "), 0, true));

    for (auto p : projectProperties)  
        p->setPreferredHeight (28);

    projectProperties[2]->setPreferredHeight (28 * 3);
    projectProperties[6]->setPreferredHeight (28 * 3);
    projectProperties[7]->setPreferredHeight (28 * 3);
    projectProperties[8]->setPreferredHeight (28 * 3);
    projectProperties[9]->setPreferredHeight (28 * 3);
    projectProperties[11]->setPreferredHeight (28 * 4);
    projectProperties[10]->setEnabled (false);

    panel->addSection (TRANS ("Project Setup"), projectProperties);
    valuesAddListener();
}

//=================================================================================================
void SetupPanel::showDirProperties (const ValueTree& dTree)
{
    initialValues (true, true);

    panel->clear();
    currentTree = dTree;
    jassert (currentTree.isValid() && currentTree.getType().toString() == "dir");

    values[itsName]->setValue (currentTree.getProperty ("name"));
    values[itsTitle]->setValue (currentTree.getProperty ("title"));
    values[keywords]->setValue (currentTree.getProperty ("keywords"));
    values[desc]->setValue (currentTree.getProperty ("description"));
    values[isMenu]->setValue (currentTree.getProperty ("isMenu"));
    values[tplFile]->setValue (currentTree.getProperty ("tplFile"));
    values[jsCode]->setValue (currentTree.getProperty ("js"));
    values[createDate]->setValue (currentTree.getProperty ("createDate"));
    values[modifyDate]->setValue (currentTree.getProperty ("modifyDate"));

    Array<PropertyComponent*> dirProperties;
    dirProperties.add (new TextPropertyComponent (*values[itsName], TRANS ("Name: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[itsTitle], TRANS ("Title: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[keywords], TRANS ("Keywords: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[desc], TRANS ("Description: "), 0, true));
    dirProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Site Menu: "), TRANS ("Yes")));
    dirProperties.add (new TextPropertyComponent (*values[jsCode], TRANS ("Code: "), 0, true));

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile (FileTreeContainer::projectTree.getProperty ("render").toString()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size(); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName());
        }
    }

    dirProperties.add (new ChoicePropertyComponent (*values[tplFile], TRANS ("Render TPL: "), tplFileSa, tplFileVar));
    dirProperties.add (new TextPropertyComponent (*values[createDate], TRANS ("Create Date: "), 0, false));
    dirProperties.add (new TextPropertyComponent (*values[modifyDate], TRANS ("Last Modified: "), 0, false));

    for (auto p : dirProperties)
        p->setPreferredHeight (28);

    dirProperties[0]->setEnabled (false);

    // 2 level menu, otherwise, it cannot be a site menu
    dirProperties[4]->setEnabled (currentTree.getParent().getType().toString() == "wdtpProject"
                                  || currentTree.getParent().getParent().getType().toString() == "wdtpProject");

    dirProperties[3]->setPreferredHeight (28 * 3);
    dirProperties[5]->setPreferredHeight (28 * 5);
    dirProperties[8]->setEnabled (false);

    panel->addSection (TRANS ("Folder Setup"), dirProperties);
    valuesAddListener();
}

//=================================================================================================
void SetupPanel::showDocProperties (const bool currentValuesUpdateTree,
                                    const ValueTree& dTree)
{
    initialValues (currentValuesUpdateTree, true);

    panel->clear();
    currentTree = dTree;
    jassert (currentTree.isValid() && currentTree.getType().toString() == "doc");

    values[itsName]->setValue (currentTree.getProperty ("name"));
    values[itsTitle]->setValue (currentTree.getProperty ("title"));
    values[keywords]->setValue (currentTree.getProperty ("keywords"));
    values[showKeys]->setValue (currentTree.getProperty ("showKeywords"));
    values[desc]->setValue (currentTree.getProperty ("description"));
    values[isMenu]->setValue (currentTree.getProperty ("isMenu"));
    values[tplFile]->setValue (currentTree.getProperty ("tplFile"));
    values[jsCode]->setValue (currentTree.getProperty ("js"));
    values[createDate]->setValue (currentTree.getProperty ("createDate"));
    values[modifyDate]->setValue (currentTree.getProperty ("modifyDate"));
    values[thumb]->setValue (currentTree.getProperty ("thumb"));
    values[thumbName]->setValue (currentTree.getProperty ("thumbName"));
    values[abbrev]->setValue (currentTree.getProperty ("abbrev"));
    values[reviewDate]->setValue (currentTree.getProperty ("reviewDate"));
    values[featured]->setValue (currentTree.getProperty ("featured"));
    values[hideMode]->setValue (currentTree.getProperty ("hide"));
    values[archiveMode]->setValue (currentTree.getProperty ("archive"));

    Array<PropertyComponent*> docProperties;
    docProperties.add (new TextPropertyComponent (*values[itsName], TRANS ("Name: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[itsTitle], TRANS ("Title: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[keywords], TRANS ("Keywords: "), 0, false));
    docProperties.add (new BooleanPropertyComponent (*values[showKeys], TRANS ("Show Keywords: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[desc], TRANS ("Description: "), 0, true));
    docProperties.add (new BooleanPropertyComponent (*values[isMenu], TRANS ("Site Menu: "), TRANS ("Yes")));
    docProperties.add (new TextPropertyComponent (*values[jsCode], TRANS ("Code: "), 0, true));

    // render tpl-file
    StringArray tplFileSa;
    Array<var> tplFileVar;

    if (FileTreeContainer::projectFile.existsAsFile())
    {
        const File renderDir (FileTreeContainer::projectFile
                              .getSiblingFile ("themes")
                              .getChildFile (FileTreeContainer::projectTree.getProperty ("render").toString()));
        Array<File> tplFiles;
        renderDir.findChildFiles (tplFiles, File::findFiles, false, "*.html");

        for (int i = 0; i < tplFiles.size(); ++i)
        {
            tplFileSa.add (tplFiles.getUnchecked (i).getFileName());
            tplFileVar.add (tplFiles.getUnchecked (i).getFileName());
        }
    }

    docProperties.add (new ChoicePropertyComponent (*values[tplFile], TRANS ("Render TPL: "),
                                                    tplFileSa, tplFileVar));
    docProperties.add (new TextPropertyComponent (*values[createDate], TRANS ("Create Date: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[modifyDate], TRANS ("Last Modified: "), 0, false));
    docProperties.add (new TextPropertyComponent (*values[wordCount], TRANS ("Word Count: "), 0, false));
    docProperties.add (new BooleanPropertyComponent (*values[thumb], TRANS ("Title Image: "), TRANS ("Using")));

    // images in this doc
    StringArray imgFileSa;
    Array<var> imgFileVar;
    const String mdContent (DocTreeViewItem::getMdFileOrDir (currentTree).loadFileAsString());
    int indexStart = mdContent.indexOfIgnoreCase (0, "![");

    while (indexStart != -1)
    {
        // jump alt content and get img path
        const int altEnd = mdContent.indexOfIgnoreCase (indexStart + 2, "](");
        if (altEnd == -1)            break;

        const int imgEnd = mdContent.indexOfIgnoreCase (altEnd + 2, ")");
        if (imgEnd == -1)            break;

        const String imgPath (mdContent.substring (altEnd + 2, imgEnd));
        imgFileSa.add (imgPath);
        imgFileVar.add (imgPath);

        indexStart = mdContent.indexOfIgnoreCase (imgEnd + 1, "![");
    }

    docProperties.add (new ChoicePropertyComponent (*values[thumbName], TRANS ("Image File: "),
                                                    imgFileSa, imgFileVar));
    docProperties.add (new TextPropertyComponent (*values[abbrev], TRANS ("Abbrev: "), 0, true));
    docProperties.add (new TextPropertyComponent (*values[reviewDate], TRANS ("Remind Date: "), 0, false));
    docProperties.add (new BooleanPropertyComponent (*values[featured], TRANS ("Featured: "), TRANS ("Yes")));
    docProperties.add (new BooleanPropertyComponent (*values[hideMode], TRANS ("Hide: "), TRANS ("Yes")));
    docProperties.add (new BooleanPropertyComponent (*values[archiveMode], TRANS ("Archive: "), TRANS ("Yes")));

    // set height
    for (auto p : docProperties)
        p->setPreferredHeight (28);

    docProperties[0]->setEnabled (false);
    docProperties[9]->setEnabled (false);
    docProperties[10]->setEnabled (false);

    // 2 level menu, otherwise, it cannot be a site menu
    docProperties[5]->setEnabled (currentTree.getParent().getType().toString() == "wdtpProject" ||
                                  currentTree.getParent().getParent().getType().toString() == "wdtpProject");

    docProperties[4]->setPreferredHeight (28 * 4);
    docProperties[6]->setPreferredHeight (28 * 4);
    docProperties[13]->setPreferredHeight (28 * 4);

    panel->addSection (TRANS ("Document Setup"), docProperties);
    valuesAddListener();
}

//=================================================================================================
void SetupPanel::projectClosed()
{
    stopTimer();
    savePropertiesIfNeeded();
    initialValues (true, false);

    currentTree = ValueTree::invalid;
    projectHasChanged = false;
    panel->clear();
}

//=================================================================================================
void SetupPanel::valuesAddListener()
{
    for (auto v : values)
        v->addListener (this);

    values[wordCount]->removeListener (this);
    values[itsName]->removeListener (this);
    values[modifyDate]->removeListener (this);
}

//=================================================================================================
void SetupPanel::initialValues (const bool currentValuesUpdateTree,
                                const bool addValues)
{
    /* prevent doesn't save the modified text when switch item in fileTree
    
       note: here set value of currentTree doesn't save the project actually,
       somehow, after switched the new tree will send message about values[xxx] was changed,
       then it'll save the projectTree immediately.
       it's a bit of unclear in meaning, but it works well...
    */
    if (currentTree.isValid() && currentValuesUpdateTree && values.size() > 0)
    {
        if (values[itsTitle]->getValue() != currentTree.getProperty ("title"))
        {
            currentTree.setProperty ("title", values[itsTitle]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (values[keywords]->getValue() != currentTree.getProperty ("keywords"))
        {
            currentTree.setProperty ("keywords", values[keywords]->getValue().toString()
                                     .trim().replace (")", String()).replace ("(", "_")  // Chinese ','
                                     .replace (CharPointer_UTF8 ("\xef\xbc\x8c"), ", "), nullptr);

            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (values[desc]->getValue() != currentTree.getProperty ("description"))
        {
            currentTree.setProperty ("description", values[desc]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (values[jsCode]->getValue() != currentTree.getProperty ("js"))
        {
            currentTree.setProperty ("js", values[jsCode]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "wdtpProject"
                 && values[projectOwner]->getValue() != currentTree.getProperty ("owner"))
        {
            currentTree.setProperty ("owner", values[projectOwner]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "wdtpProject"
                 && values[copyrightInfo]->getValue() != currentTree.getProperty ("copyright"))
        {
            currentTree.setProperty ("copyright", values[copyrightInfo]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "wdtpProject"
                 && values[contact]->getValue() != currentTree.getProperty ("contact"))
        {
            currentTree.setProperty ("contact", values[contact]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "wdtpProject"
                 && values[ad]->getValue() != currentTree.getProperty ("ad"))
        {
            currentTree.setProperty ("ad", values[ad]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() != "wdtpProject"
                 && values[createDate]->getValue() != currentTree.getProperty ("createDate"))
        {
            currentTree.setProperty ("createDate", values[createDate]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "doc"
                 && values[abbrev]->getValue() != currentTree.getProperty ("abbrev"))
        {
            currentTree.setProperty ("abbrev", values[abbrev]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }

        else if (currentTree.getType().toString() == "doc"
                 && values[reviewDate]->getValue() != currentTree.getProperty ("reviewDate"))
        {
            currentTree.setProperty ("reviewDate", values[reviewDate]->getValue().toString().trim(), nullptr);
            DocTreeViewItem::needCreate (currentTree);
            FileTreeContainer::saveProject();
        }
    }
    
    values.clear();
    projectHasChanged = false;

    if (addValues)
    {
        for (int i = totalValues; --i >= 0; )
            values.add (new Value());
    }
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
    if (value.refersToSameSourceAs (*values[itsTitle]))
        currentTree.setProperty ("title", values[itsTitle]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[keywords]))
        currentTree.setProperty ("keywords", values[keywords]->getValue().toString().trim()
                                 .replace (")", String()).replace ("(", "_")
                                 .replace (CharPointer_UTF8 ("\xef\xbc\x8c"), ", "), nullptr);  // Chinese ','

    else if (value.refersToSameSourceAs (*values[desc]))
        currentTree.setProperty ("description", values[desc]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[projectOwner]))
        currentTree.setProperty ("owner", values[projectOwner]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[copyrightInfo]))
        currentTree.setProperty ("copyright", values[copyrightInfo]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[renderDir]))
        currentTree.setProperty ("render", values[renderDir]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[tplFile]))
        currentTree.setProperty ("tplFile", values[tplFile]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[jsCode]))
        currentTree.setProperty ("js", values[jsCode]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[ad]))
        currentTree.setProperty ("ad", values[ad]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[contact]))
        currentTree.setProperty ("contact", values[contact]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[resources]))
        currentTree.setProperty ("resources", values[resources]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[isMenu]))
        currentTree.setProperty ("isMenu", values[isMenu]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[createDate]))
        currentTree.setProperty ("createDate", values[createDate]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[showKeys]))
        currentTree.setProperty ("showKeywords", values[showKeys]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[thumb]))
        currentTree.setProperty ("thumb", values[thumb]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[thumbName]))
        currentTree.setProperty ("thumbName", values[thumbName]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[abbrev]))
        currentTree.setProperty ("abbrev", values[abbrev]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[reviewDate]))
        currentTree.setProperty ("reviewDate", values[reviewDate]->getValue().toString().trim(), nullptr);

    else if (value.refersToSameSourceAs (*values[featured]))
        currentTree.setProperty ("featured", values[featured]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[hideMode]))
        currentTree.setProperty ("hide", values[hideMode]->getValue(), nullptr);

    else if (value.refersToSameSourceAs (*values[archiveMode]))
        currentTree.setProperty ("archive", values[archiveMode]->getValue(), nullptr);

    values[modifyDate]->setValue (SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), true));

    if (!value.refersToSameSourceAs (*values[resources])
        && !value.refersToSameSourceAs (*values[reviewDate])
        && !value.refersToSameSourceAs (*values[archiveMode]))
    {
        DocTreeViewItem::needCreate (currentTree);
    }

    projectHasChanged = true;

    // set mdEditor read only or not base on 'archive'
    editorAndPreView->setMdEditorReadOnly (values[archiveMode]->getValue());

    // regenarate the current page
    if (editorAndPreView->getCureentState())
        cmdManager->invokeDirectly (TopToolBar::generateCurrent, true);

    // update the panel
    if (currentTree.getType().toString() == "wdtpProject")
        showProjectProperties (currentTree);
    else if (currentTree.getType().toString() == "dir")
        showDirProperties (currentTree);
    else if (currentTree.getType().toString() == "doc")
        showDocProperties (false, currentTree);

    startTimer (200);
}

//=================================================================================================
void SetupPanel::timerCallback()
{
    savePropertiesIfNeeded();
}

//=================================================================================================
void SetupPanel::savePropertiesIfNeeded()
{
    if (projectHasChanged && currentTree.isValid() &&
        FileTreeContainer::projectFile.existsAsFile())
    {
        FileTreeContainer::saveProject();
    }

    projectHasChanged = false;
    stopTimer();
}


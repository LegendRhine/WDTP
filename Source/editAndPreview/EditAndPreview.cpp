﻿/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"
#include <fstream>

//==============================================================================
EditAndPreview::EditAndPreview () 
{
    addAndMakeVisible (webView);

    // stretched layout, arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.69);  // editor，
    layoutManager.setItemLayout (1, 3, 3, 3);           // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.31);  // propertiesPanel

    addAndMakeVisible (editor = new EditorForMd (this));
    addAndMakeVisible (setupPanel = new SetupPanel (this));
    addAndMakeVisible (layoutBar = new StretchableLayoutResizerBar (&layoutManager, 1, true));

    // editor
    editor->setMultiLine (true);
    editor->setReturnKeyStartsNewLine (true);
    editor->setTabKeyUsedAsCharacter (true);
    editor->setColour (TextEditor::textColourId, Colour(0xff303030));
    editor->setColour (TextEditor::focusedOutlineColourId, Colour(0xffb4b4b4));
    editor->setColour (TextEditor::backgroundColourId, Colour(0xffdedede));
    editor->setScrollBarThickness (10);
    editor->setIndents (10, 10);
    editor->setFont (SwingUtilities::getFontSize());
    editor->setEnabled (false);
    editor->setBorder (BorderSize<int>(1, 1, 1, 1));
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
    stopTimer();
}

//=========================================================================
void EditAndPreview::resized()
{
    // layout
    Component* comps[] = { (webView.isVisible ()) ? 
        static_cast<Component*>(&webView) : static_cast<Component*>(editor), 
        layoutBar, setupPanel };

    layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
}

//=================================================================================================
void EditAndPreview::startWork (ValueTree& newDocTree)
{
    saveCurrentDocIfChanged ();

    if (newDocTree != docOrDirTree || docOrDirFile != DocTreeViewItem::getMdFileOrDir (newDocTree))
    {
        editor->removeListener (this);
        docOrDirTree = newDocTree;
        docOrDirFile = DocTreeViewItem::getMdFileOrDir (newDocTree);

        if (docOrDirFile.existsAsFile ())
        {
            editor->applyFontToAllText (FileTreeContainer::fontSize);
            editor->setText (docOrDirFile.loadFileAsString (), false);
            currentContent = editor->getText ();
            editor->addListener (this);
        }
    }

    TopToolBar* toolBar = findParentComponentOfClass<MainContentComponent>()->getToolbar();
    jassert (toolBar != nullptr);

    if (docOrDirFile.isDirectory())
    {
        previewCurrentDoc ();
        toolBar->enableEditPreviewBt (false, true);
    }
    else  // doc
    {
        const bool justCreatedThisDoc (Time::getCurrentTime() - docOrDirFile.getCreationTime() < RelativeTime (2.0));

        if (toolBar->getStateOfViewButton() && !justCreatedThisDoc)
        {
            previewCurrentDoc();
            toolBar->enableEditPreviewBt (true, true);
        }
        else
        {
            editCurrentDoc();
            toolBar->enableEditPreviewBt (true, false);

            if (justCreatedThisDoc)
                editor->moveCaretToEnd (false);
        }
    }
}

//=================================================================================================
void EditAndPreview::editCurrentDoc ()
{
    webView.setVisible (false);
    editor->setEnabled (true);
    editor->grabKeyboardFocus ();    

    setupPanel->setEnabled (true);
    resized ();
}

//=================================================================================================
void EditAndPreview::previewCurrentDoc ()
{
    editor->setEnabled (false);
    webView.setVisible (true);
    webView.stop ();
    
    if (docOrDirFile.existsAsFile ())
        webView.goToURL (createArticleHtml().getFullPathName());
    else
        webView.goToURL (createIndexHtml().getFullPathName());    

    setupPanel->setEnabled (docOrDirFile.isDirectory());
    resized ();
}

//=================================================================================================
const File EditAndPreview::createArticleHtml ()
{
    jassert (FileTreeContainer::projectTree.isValid());
    jassert (docOrDirFile.existsAsFile ());  // selected a dir currently??    

    const String docPath (docOrDirFile.getFullPathName ());
    const File htmlFile (File (docPath.replace ("docs", "site")).withFileExtension ("html"));

    if ((bool)docOrDirTree.getProperty ("needCreateHtml") || !htmlFile.existsAsFile())
    {   
        if (htmlFile.deleteFile())
        {
            const String tplPath (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                  .getFullPathName () + File::separator
                                  + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                  + File::separator);

            const File tplFile = tplPath + ((bool (docOrDirTree.getProperty ("isPage"))) 
                                 ? "page.html" : "article.html");

            // get the description (the second line which not empty-line)
            // the first line should be title
            StringArray contentArray;
            contentArray.addLines (currentContent);
            contentArray.removeEmptyStrings ();
            String description;

            for (int i = 0; i < contentArray.size(); ++i)
            {
                if (i > 0 && contentArray.getReference(i).trim().isNotEmpty())
                {
                    description = contentArray.getReference (i).trim ()
                        .replace ("+", String ())
                        .replace ("-", String ())
                        .replace ("*", String ())
                        .replace ("#", String ())
                        .replace ("`", String ())
                        .replace (">", String ())
                        .replace ("|", String ())
                        .replace ("<", String ())
                        .replace ("!", String ())
                        .replace ("[", String ())
                        .replace ("]", String ());

                    break;
                }
            }

            // get the path which relative the site root-dir, for css path            
            const String htmlFilePath (htmlFile.getFullPathName ());
            const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory().getFullPathName() 
                                         + File::separator + "site");
            const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
            String cssRelativePath;

            for (int i = tempStr.length(); --i >= 0;)
            {
                if (tempStr[i] == File::separator)
                    cssRelativePath << String ("../");
            }
            
            // generate the doc's html
            htmlFile.create ();
            const String htmlStr (Md2Html::mdStringToHtml (currentContent));

            htmlFile.appendText (Md2Html::renderHtmlContent (htmlStr,
                                                             tplFile,
                                                             docOrDirTree.getProperty ("keywords").toString (),
                                                             description.trim (),
                                                             docOrDirTree.getProperty ("title").toString (),
                                                             cssRelativePath));

            // here, we copy this doc's media file to the site's
            const String docMediaDirStr (docOrDirFile.getSiblingFile ("media").getFullPathName ());
            const String htmlMediaDirStr (htmlFile.getSiblingFile ("media").getFullPathName ());
            Array<File> docMedias;
            Array<File> htmlMedias;

            int indexStart = htmlStr.indexOfIgnoreCase (0, "src=\"");
            int indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");

            while (indexStart != -1 && indexEnd != -1)
            {
                docMedias.add (File (docMediaDirStr + File::separator + htmlStr.substring (indexStart + 11, indexEnd)));
                htmlMedias.add (File (htmlMediaDirStr + File::separator + htmlStr.substring (indexStart + 11, indexEnd)));

                indexStart = htmlStr.indexOfIgnoreCase (indexEnd + 2, "src=\"");

                if (indexStart != -1)
                    indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");
            }

            jassert (docMedias.size () == htmlMedias.size ());
            String errorStr;

            for (int i = docMedias.size(); --i >= 0; )
            {
                htmlMedias[i].create ();

                if (! docMedias[i].copyFileTo (htmlMedias[i]))
                    errorStr << docMedias[i].getFullPathName () << newLine;
            }

            if (errorStr.isNotEmpty ())
            {
                SHOW_MESSAGE (TRANS ("Can't generate these media-files:")
                              + newLine + newLine + errorStr + newLine);
            } 

            docOrDirTree.setProperty ("needCreateHtml", false, nullptr);
            FileTreeContainer::saveProject ();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this document's html file."));
        }        
    }

    return htmlFile;
}

//=================================================================================================
const File EditAndPreview::createIndexHtml ()
{
    jassert (FileTreeContainer::projectTree.isValid ());
    jassert (docOrDirFile.isDirectory ());  // selected a article currently?? 

    const String dirPath (docOrDirFile.getFullPathName ());
    const File siteDir (dirPath.replace ("docs", "site"));
    File indexHtml (siteDir.getChildFile ("index.html"));

    if ((bool) docOrDirTree.getProperty ("needCreateHtml") 
        || !indexHtml.existsAsFile())
    {
        if (indexHtml.deleteFile ())
        {
            const bool isWebIndex = (siteDir.getFileName () == "site");

            const File tplFile (FileTreeContainer::projectFile.getSiblingFile ("themes")
                                .getFullPathName () + File::separator
                                + FileTreeContainer::projectTree.getProperty ("render").toString ()
                                + File::separator 
                                + (isWebIndex ? "index.html" : "category.html"));
            
            // get the path which relative the site root-dir, for css path            
            const String htmlFilePath (indexHtml.getFullPathName ());
            const String webRootDirPath (FileTreeContainer::projectFile.getParentDirectory ().getFullPathName ()
                                         + File::separator + "site");
            const String tempStr (htmlFilePath.trimCharactersAtStart (webRootDirPath));
            String cssRelativePath;

            for (int i = tempStr.length (); --i >= 0;)
            {
                if (tempStr[i] == File::separator)
                    cssRelativePath << String ("../");
            }

            const String tplStr (tplFile.loadFileAsString());
            const String indexTileStr (docOrDirTree.getProperty ("title").toString ());
            String indexContent (tplStr.replace("{{siteRelativeRootPath}}", cssRelativePath)
                                 .replace ("{{title}}", indexTileStr)
                                 .replace ("{{keywords}}", indexTileStr)
                                 .replace ("{{description}}", indexTileStr + "-" + TRANS ("file list")));
            
            // title of this index.html
            if (tplStr.contains ("{{titleOfDir}}"))
                indexContent = indexContent.replace ("{{titleOfDir}}", "<div align=center><h1>" 
                                                 + indexTileStr + "</h1></div>" + newLine);

            // process TAGs, it's nothing about the content but the dir or something else
            if (tplStr.contains("{{fileAndDirList}}"))
            {
                indexContent = indexContent.replace ("{{fileAndDirList}}", "<div>"
                                                     + TplTagProcessor::fileAndDirList (docOrDirTree)
                                                     + "</div>");
            }

            indexHtml.create ();
            indexHtml.appendText (indexContent);

            docOrDirTree.setProperty ("needCreateHtml", false, nullptr);
            FileTreeContainer::saveProject ();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create this folder's index.html."));
        }        
    }

    return indexHtml;
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    saveCurrentDocIfChanged();

    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (false);    

    setupPanel->projectClosed ();
    webView.setVisible (false);

    docOrDirFile = File::nonexistent;
    docOrDirTree = ValueTree::invalid;
    docHasChanged = false;    
    currentContent.clear();
}

//=================================================================================================
void EditAndPreview::setProjectProperties (ValueTree& projectTree)
{
    setupPanel->showProjectProperties (projectTree);
}

//=================================================================================================
void EditAndPreview::setDirProperties (ValueTree& dirTree)
{
    setupPanel->showDirProperties (dirTree);
}

//=================================================================================================
void EditAndPreview::setDocProperties (ValueTree& docTree_)
{
    setupPanel->showDocProperties (docTree_);
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when load a doc, so use this ugly judge...
    if (currentContent.compare (editor->getText()) != 0)
    {
        docHasChanged = true;
        jassert (docOrDirFile.existsAsFile ());

        ValueTree rootTree = docOrDirTree;
        rootTree.setProperty ("needCreateHtml", true, nullptr);

        while (rootTree.getParent().isValid ())
        {
            rootTree = rootTree.getParent ();
            rootTree.setProperty ("needCreateHtml", true, nullptr);
        }

        startTimer (5000);
    }    
}

//=================================================================================================
void EditAndPreview::timerCallback ()
{
    saveCurrentDocIfChanged();
}

//=================================================================================================
const bool EditAndPreview::saveCurrentDocIfChanged ()
{
    /*static int i = 0;
    DBGX (++i);*/

    stopTimer ();

    if (docHasChanged && docOrDirFile != File::nonexistent)
    {
        currentContent = editor->getText ();
        const String tileStr (currentContent.trim().upToFirstOccurrenceOf ("\n", false, true)
                              .replace("#", String()).trim());

        TemporaryFile tempFile (docOrDirFile);
        tempFile.getFile ().appendText (currentContent);

        if (tempFile.overwriteTargetFileWithTemporary())
        {
            docOrDirTree.setProperty ("title", tileStr, nullptr);
            docHasChanged = false;

            return FileTreeContainer::saveProject();
        }
        else
        {
            return false;
        }
    }

    return true;
}

//=================================================================================================
void EditorForMd::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const File& docFile (parent->getCurrentDocFile ());

    if (e->mods.isPopupMenu())
    {
        menu.addItem (20, TRANS ("Pickup Keyword"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();

        menu.addItem (21, TRANS ("Search Selected"), getHighlightedText ().isNotEmpty ());
        menu.addItem (22, TRANS ("Replace Selected"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();

        PopupMenu insertMenu;
        insertMenu.addItem (1, TRANS ("Iamge(s)..."));
        insertMenu.addItem (2, TRANS ("Hyperlink..."));
        insertMenu.addItem (3, TRANS ("Table (4 x 3)"));
        insertMenu.addItem (4, TRANS ("Quotation"));
        insertMenu.addSeparator ();

        insertMenu.addItem (5, TRANS ("Align Center"));
        insertMenu.addItem (6, TRANS ("Align Right"));
        insertMenu.addSeparator ();

        insertMenu.addItem (7, TRANS ("Unordered List"));
        insertMenu.addItem (8, TRANS ("Ordered List"));
        insertMenu.addSeparator ();

        insertMenu.addItem (9, TRANS ("Primary Heading"));
        insertMenu.addItem (10, TRANS ("Secondary Heading"));
        insertMenu.addItem (11, TRANS ("Tertiary Heading"));
        insertMenu.addSeparator ();

        insertMenu.addItem (14, TRANS ("Separator"));
        insertMenu.addItem (15, TRANS ("Author and Date"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile());

        PopupMenu formatMenu;
        formatMenu.addItem (30, TRANS ("Bold"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (31, TRANS ("Italic"), getHighlightedText().isNotEmpty());
        formatMenu.addItem (32, TRANS ("Code Block"));
        formatMenu.addItem (33, TRANS ("Code Inline"), getHighlightedText().isNotEmpty());

        menu.addSubMenu (TRANS ("Format"), formatMenu, docFile.existsAsFile());
        menu.addSeparator ();

        TextEditor::addPopupMenuItems (menu, e);
    }
}

//=================================================================================================
void EditorForMd::performPopupMenuAction (int index)
{
    String content;
    ValueTree& docTree = parent->getCurrentTree ();

    if (20 == index)  // add the selected to this doc's keywords
    {
        content = getHighlightedText ();
        const String currentKeyWords (docTree.getProperty ("keywords").toString().trim());

        String keyWords (currentKeyWords);
        bool needRecreateHtm = true;

        // update the doc-tree
        if (currentKeyWords.isNotEmpty ())
        {
            if (!currentKeyWords.containsIgnoreCase (content))
                keyWords = currentKeyWords + ", " + content;
            else
                needRecreateHtm = false;
        }
        else
        {
            keyWords = content;
        }

        docTree.setProperty ("keywords", keyWords, nullptr);
    }
    else if (21 == index)  // search by selected
    {
        NEED_TO_DO ("search by selected");
        return;  // don't insert anything in current content
    }
    else if (22 == index)  // replace selected to something else
    {
        NEED_TO_DO ("replace selected");
        return;  // don't insert anything in current content
    }
    else if (1 == index) // image
    {
        FileChooser fc (TRANS ("Select Images..."), File::nonexistent,
                        "*.jpg;*.png;*.gif", true);
        Array<File> imageFiles;

        if (!fc.browseForMultipleFilesToOpen())
            return;

        imageFiles = fc.getResults();
        const File imgPath (DocTreeViewItem::getMdFileOrDir (docTree).getSiblingFile ("media"));

        for (auto f : imageFiles)
        {
            const File targetFile (imgPath.getChildFile (f.getFileName()).getNonexistentSibling (false));
            targetFile.create();

            if (f.copyFileTo (targetFile))
                content << newLine << "![ ](media/" << targetFile.getFileName() << ")" << newLine;
            else
                SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName());
        }
    }
    else if (2 == index) // hyperlink
    {
        AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                            AlertWindow::InfoIcon);

        dialog.addTextEditor ("name", String());
        dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

        if (0 == dialog.runModalLoop ())
        {
            const String inputStr (dialog.getTextEditor ("name")->getText().trim());
            content << "[" << inputStr << "](" << inputStr << ") ";
        }
        else
        {
            return;  // for: no need to perform other codes below...
        }
    }    
    else if (3 == index) // table (4 x 3)
    {
        content << newLine 
            << "<table>" << newLine
            << "  <tr><th> H1 </th><th> H2 </th><th> H3 </th></tr>" << newLine
            << "  <tr><td> 11 </td><td> 12 </td><td> 13 </td></tr>" << newLine
            << "  <tr><td> 21 </td><td> 22 </td><td> 23 </td></tr>" << newLine
            << "  <tr><td> 31 </td><td> 32 </td><td> 33 </td></tr>" << newLine
            << "  <tr><td> 41 </td><td> 42 </td><td> 43 </td></tr>" << newLine
            << "</table>" << newLine;
    }
    else if (4 == index) // Quotation
    {
        content << newLine << "> ";
    }
    else if (5 == index) // align center
    {
        content << newLine << ">|< ";
    }   
    else if (6 == index) // align right
    {
        content << newLine << ">>> ";
    }
    else if (7 == index)  // unordered list
    {
        content << newLine 
            << "- " << newLine
            << "- " << newLine
            << "- " << newLine;
    }
    else if (8 == index)  // ordered list. it'll parse as "1. 2. 3." etc.
    {
        content << newLine
            << "+ " << newLine
            << "+ " << newLine
            << "+ " << newLine;
    }
    else if (9 == index)  // second heading
    {
        content << newLine << "# ";
    }
    else if (10 == index)  // second heading
    {
        content << newLine << "## ";
    }
    else if (11 == index) // third heading
    {
        content << newLine << "### ";
    }
    else if (14 == index) // separator
    {
        content << newLine << "---" << newLine << newLine;
    }
    else if (15 == index) // author and date
    {
        content << newLine << newLine 
            << ">>> "
            << TRANS ("Author: ")
            << FileTreeContainer::projectTree.getProperty ("owner").toString ()
            << " " << newLine << ">>> "
            << SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString ()).dropLastCharacters (9)
            << " ";
        
    }
    else if (30 == index) // bold
    {
        content << "**" << getHighlightedText() << "**";
    }
    else if (31 == index) // italic
    {
        content << "*" << getHighlightedText() << "*";
    }
    else if (32 == index) // bold + italic
    {
        content << newLine
            << "```" << newLine
            << getHighlightedText () << newLine
            << "```" << newLine;
    }
    else if (33 == index) // code inline
    {
        content << "`" << getHighlightedText() << "`";
    }
    else
    {
        TextEditor::performPopupMenuAction (index);
        return;
    }
    
    insertTextAtCaret (content);

    // move up the currsor...
    if (7 == index || 8 == index)
    {
        moveCaretUp (false);
        moveCaretUp (false);
        moveCaretUp (false);
        moveCaretToEndOfLine (false);
    }

    docTree.setProperty ("needCreateHtml", true, nullptr);

    // save the project then update the setup panel
    FileTreeContainer::saveProject ();
    parent->getSetupPanel()->showDocProperties (docTree);
}

//=================================================================================================
bool EditorForMd::keyPressed (const KeyPress& key)
{
    if (key == KeyPress(KeyPress::tabKey))
    {
        insertTextAtCaret ("    ");
        return true;
    }

    return TextEditor::keyPressed (key);
}

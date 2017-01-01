/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"
#include <fstream>

//==============================================================================
EditAndPreview::EditAndPreview()
{
    addChildComponent (webView);

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
    editor->setColour (TextEditor::backgroundColourId, Colour(0xffededed));
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
void EditAndPreview::startWork (ValueTree& newDocTree, const bool enterEditState)
{
    saveCurrentDocIfChanged ();

    if (newDocTree != docTree)
    {
        const File& file (DocTreeViewItem::getFileOrDir (newDocTree));

        if (file.existsAsFile ())
        {
            docFile = file;
            docTree = newDocTree;

            editor->removeListener (this);
            editor->applyFontToAllText (FileTreeContainer::fontSize);
            editor->setText (docFile.loadFileAsString (), false);

            currentContent = editor->getText ();
            editor->grabKeyboardFocus ();
            editor->addListener (this);
        }
        else
        {
            editorAndWebInitial ();
        }
    }

    enterEditState ? previewCurrentDoc () : editCurrentDoc ();
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

    if (docFile.existsAsFile ())
    {
        webView.goToURL (createMatchedHtmlFile ().getFullPathName ());
    }
    else if (docFile.isDirectory ())
    {
        editorAndWebInitial ();
        const String dirPath (docFile.getFullPathName ());
        const File siteDir (dirPath.replace ("docs", "site"));

        webView.goToURL (siteDir.getChildFile ("index.html").getFullPathName ());
    }

    setupPanel->setEnabled (false);
    resized ();
}

//=================================================================================================
const File EditAndPreview::createMatchedHtmlFile ()
{
    jassert (FileTreeContainer::projectTree.isValid());

    const String docPath (docFile.withFileExtension("html").getFullPathName());
    File htmlFile (docPath.replace("docs", "site"));

    if (needCreateHtml || !htmlFile.existsAsFile())
    {   
        if (htmlFile.deleteFile())
        {
            const File tplFile (FileTreeContainer::projectFile
                                .getSiblingFile (docTree.getProperty ("tplFile").toString()));

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

            // generate the doc's html
            htmlFile.create ();
            htmlFile.appendText (Md2Html::mdStringToHtml (currentContent, 
                                                          tplFile,
                                                          docTree.getProperty("keywords").toString(),
                                                          description.trim(), 
                                                          docTree.getProperty ("title").toString ()));

            // here, we copy the doc's media dir to the site's
            // maybe this is a ugly implement, need to be improved
            const File docsMediaDir (docFile.getSiblingFile ("media"));

            if (docsMediaDir.isDirectory())
            {
                const File htmlsMediaDir (htmlFile.getSiblingFile ("media"));
                docsMediaDir.copyDirectoryTo (htmlsMediaDir);
            }
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Something wrong during create html file."));
        }        
    }

    needCreateHtml = false;
    return htmlFile;
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    saveCurrentDocIfChanged();
    editorAndWebInitial ();

    setupPanel->projectClosed ();
    webView.setVisible (false);
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
void EditAndPreview::editorAndWebInitial ()
{
    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (false);

    docFile = File::nonexistent;
    docTree = ValueTree::invalid;
    docHasChanged = false;
    currentContent = String();
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when load a doc, so use this ugly judge...
    if (currentContent.compare (editor->getText()) != 0)
    {
        docHasChanged = true;
        needCreateHtml = true;
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

    if (docHasChanged && docFile != File::nonexistent)
    {
        currentContent = editor->getText ();
        const String tileStr (currentContent.trim().upToFirstOccurrenceOf ("\n", false, true)
                              .replace("#", String()).trim());

        TemporaryFile tempFile (docFile);
        tempFile.getFile ().appendText (currentContent);

        if (tempFile.overwriteTargetFileWithTemporary())
        {
            docTree.setProperty ("title", tileStr, nullptr);
            docHasChanged = false;

            return SwingUtilities::writeValueTreeToFile (FileTreeContainer::projectTree, 
                                                         FileTreeContainer::projectFile);
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
        const String selectedStr (getHighlightedText ());
        const String currentKeyWords (docTree.getProperty ("keywords").toString().trim());

        String keyWords (currentKeyWords);
        bool needRecreateHtm = true;

        // update the doc-tree
        if (currentKeyWords.isNotEmpty ())
        {
            if (!currentKeyWords.containsIgnoreCase (selectedStr))
                keyWords = currentKeyWords + ", " + selectedStr;
            else
                needRecreateHtm = false;
        }
        else
        {
            keyWords = selectedStr;
        }

        docTree.setProperty ("keywords", keyWords, nullptr);
        parent->needRecreateHtml (needRecreateHtm);

        // save the project
        ValueTree rootTree = docTree.getParent();

        while (rootTree.getParent ().isValid ())
            rootTree = rootTree.getParent ();

        if (!SwingUtilities::writeValueTreeToFile (rootTree, FileTreeContainer::projectFile))
            SHOW_MESSAGE (TRANS ("Something wrong during saving this project."));

        // then update the setup panel
        parent->getSetupPanel()->showDocProperties (docTree);

        return;  // don't insert anything in current content

    }
    else if (21 == index)  // search by selected
    {
        return;  // don't insert anything in current content
    }
    else if (22 == index)  // replace selected to something else
    {
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
        const File imgPath (DocTreeViewItem::getFileOrDir (docTree).getSiblingFile ("media"));

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

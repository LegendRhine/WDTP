/*
  ==============================================================================

    MarkdownEditor.cpp
    Created: 8 Feb 2017 2:46:52am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

//=================================================================================================
MarkdownEditor::MarkdownEditor (EditAndPreview* parent_)
    : parent (parent_),
    fontSizeSlider (Slider::LinearHorizontal, Slider::TextBoxBelow)
{
    fontSizeSlider.setRange (15.0, 35.0, 1.0);
    fontSizeSlider.setDoubleClickReturnValue (true, 20.0);
    fontSizeSlider.setSize (300, 60);
    fontSizeSlider.addListener (this);
}

//=================================================================================================
void MarkdownEditor::paint (Graphics& g)
{
    TextEditor::paint (g);
    g.setColour (Colours::grey);
    g.drawVerticalLine (getWidth() - 1, 0, getBottom() - 0.f);
}

//=================================================================================================
void MarkdownEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const File& docFile (parent->getCurrentDocFile());

    if (e->mods.isPopupMenu())
    {
        menu.addItem (pickTitle, TRANS ("Pickup as Title"), getHighlightedText().isNotEmpty());
        menu.addSeparator ();

        menu.addItem (addKeywords, TRANS ("Add to Keywords"), getHighlightedText ().isNotEmpty ());
        menu.addItem (pickFromAllKeywords, TRANS ("Reuse from Keywords Table") + "...");
        menu.addSeparator ();
        
        menu.addItem (pickDesc, TRANS ("Pickup as Description"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator();

        PopupMenu insertMenu;
        String ctrlStr ("  (Ctrl + ");

#if JUCE_MAC
        ctrlStr = "  (Cmd + ";
#endif

        insertMenu.addItem (insertImage, TRANS ("Iamge(s)...") + ctrlStr + "M)");
        insertMenu.addItem (insertHyperlink, TRANS ("Hyperlink...") + ctrlStr + "H)");
        insertMenu.addItem (insertTable, TRANS ("Table (4 x 3)") + ctrlStr + "T)");
        insertMenu.addItem (insertQuota, TRANS ("Quotation"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertAlignCenter, TRANS ("Align Center") + ctrlStr + "N)");
        insertMenu.addItem (insertAlignRight, TRANS ("Align Right") + ctrlStr + "R)");
        insertMenu.addSeparator();

        insertMenu.addItem (insertUnoerderList, TRANS ("Unordered List"));
        insertMenu.addItem (insertOrderList, TRANS ("Ordered List"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertFirstTitle, TRANS ("Primary Heading"));
        insertMenu.addItem (insertSecondTitle, TRANS ("Secondary Heading"));
        insertMenu.addItem (insertThirdTitle, TRANS ("Tertiary Heading"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertToc, TRANS ("Table of Contents"));
        insertMenu.addItem (insertEndnote, TRANS ("Endnote"));
        insertMenu.addItem (insertIdentifier, TRANS ("Identifier"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertCaption, TRANS ("Image/Table Caption") + ctrlStr + "P)");
        insertMenu.addItem (insertSeparator, TRANS ("Separator"));
        insertMenu.addItem (insertAuthor, TRANS ("Author") + ctrlStr + "O)");
        insertMenu.addSeparator();

        const String internalLinkStr (SystemClipboard::getTextFromClipboard());
        insertMenu.addItem (insertInterLink, TRANS ("Internal Link"), internalLinkStr.contains ("*_wdtpGetPath_*"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile());

        PopupMenu formatMenu;
        formatMenu.addItem (formatBold, TRANS ("Bold") + ctrlStr + "B)");
        formatMenu.addItem (formatItalic, TRANS ("Italic") + ctrlStr + "I)");
        formatMenu.addItem (formatBoldAndItalic, TRANS ("Bold + Italic"));
        formatMenu.addSeparator();
        formatMenu.addItem (formatHighlight, TRANS ("Highlight") + ctrlStr + "U)");
        formatMenu.addSeparator();
        formatMenu.addItem (inlineCode, TRANS ("Code Inline") + ctrlStr + "L)");
        formatMenu.addItem (codeBlock, TRANS ("Code Block") + ctrlStr + "K)");

        menu.addSubMenu (TRANS ("Format"), formatMenu, docFile.existsAsFile());
        menu.addSeparator();

        menu.addItem (searchNext, TRANS ("Search Next Selection") + "  F3", getHighlightedText().isNotEmpty());
        menu.addItem (searchPrev, TRANS ("Search Prev Selection") + "  Shift + F3", getHighlightedText().isNotEmpty());
        menu.addSeparator();

        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator();

        PopupMenu editorSetup;
        editorSetup.addItem (fontSize, TRANS ("Font Size..."));
        editorSetup.addItem (fontColor, TRANS ("Font Color..."));
        editorSetup.addItem (setBackground, TRANS ("Backgroud..."));
        editorSetup.addSeparator();
        editorSetup.addItem (resetDefault, TRANS ("Reset to Default"));

        menu.addSubMenu (TRANS ("Editor Setup"), editorSetup, docFile.existsAsFile());
    }
}

//=================================================================================================
void MarkdownEditor::performPopupMenuAction (int index)
{
    if (addKeywords == index)
    {
        addSelectedToKeywords();
        parent->getSetupPanel()->updateDocPanel();
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (pickTitle == index)
    {
        parent->getCurrentTree().setProperty ("title", getHighlightedText(), nullptr);
        parent->getSetupPanel()->updateDocPanel();
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (pickDesc == index)
    {
        parent->getCurrentTree().setProperty ("description", getHighlightedText(), nullptr);
        parent->getSetupPanel()->updateDocPanel();
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (insertSeparator == index)      insertTextAtCaret (newLine + "---" + newLine);
    else if (pickFromAllKeywords == index)  showAllKeywords ();
    else if (searchPrev == index)           searchBySelectPrev();
    else if (searchNext == index)           searchBySelectNext();
    else if (insertImage == index)          insertImages();
    else if (insertHyperlink == index)      hyperlinkInsert();
    else if (insertTable == index)          tableInsert();
    else if (insertQuota == index)          quotaInsert();
    else if (insertAlignCenter == index)    alignCenterInsert();
    else if (insertAlignRight == index)     alignRightInsert();
    else if (insertUnoerderList == index)   unorderListInsert();
    else if (insertOrderList == index)      orderListInsert();
    else if (insertFirstTitle == index)     insertTitle (1);
    else if (insertSecondTitle == index)    insertTitle (2);
    else if (insertThirdTitle == index)     insertTitle (3);
    else if (insertToc == index)            tocInsert();
    else if (insertEndnote == index)        endnoteInsert();
    else if (insertIdentifier == index)     identifierInsert();
    else if (insertCaption == index)        captionInsert();
    else if (insertAuthor == index)         authorInsert();
    else if (insertInterLink == index)      interLinkInsert();
    else if (formatBold == index)           inlineFormat (bold);
    else if (formatItalic == index)         inlineFormat (italic);
    else if (formatBoldAndItalic == index)  inlineFormat (boldAndItalic);
    else if (formatHighlight == index)      inlineFormat (highlight);
    else if (inlineCode == index)           inlineFormat (codeOfinline);
    else if (codeBlock == index)            codeBlockFormat();
    else if (fontSize == index)             setFontSize();
    else if (fontColor == index)            setFontColour();
    else if (setBackground == index)        setBackgroundColour();
    else if (resetDefault == index)         resetToDefault();

    else
        TextEditor::performPopupMenuAction (index);
}

//=================================================================================================
void MarkdownEditor::resetToDefault()
{
    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                      TRANS ("Are you sure you want to reset the font size,\n"
                                             "text color and background to the default?")))
    {

        systemFile->setValue ("fontSize", SwingUtilities::getFontSize());
        systemFile->setValue ("editorFontColour", Colour (0xff303030).toString());
        systemFile->setValue ("editorBackground", Colour (0xffdedede).toString());

        parent->getEditor()->setColour (TextEditor::textColourId, Colour (0xff303030));
        parent->getEditor()->setColour (CaretComponent::caretColourId, Colour (0xff303030).withAlpha (0.6f));
        parent->getEditor()->setColour (TextEditor::backgroundColourId, Colour (0xffdedede));
        parent->getEditor()->setFont (SwingUtilities::getFontSize());

        parent->getEditor()->applyFontToAllText (SwingUtilities::getFontSize());
        systemFile->saveIfNeeded();
    }
}

//=================================================================================================
void MarkdownEditor::setBackgroundColour()
{
    bgColourSelector = new ColourSelectorWithPreset();

    bgColourSelector->setSize (450, 480);
    bgColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("editorBackground")));
    bgColourSelector->addChangeListener (this);

    CallOutBox callOut (*bgColourSelector, getLocalBounds(), this);
    callOut.runModalLoop();

    systemFile->setValue ("editorBackground", bgColourSelector->getCurrentColour().toString());
    systemFile->saveIfNeeded();
}

//=================================================================================================
void MarkdownEditor::setFontColour()
{
    fontColourSelector = new ColourSelectorWithPreset();

    fontColourSelector->setSize (450, 480);
    fontColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("editorFontColour")));
    fontColourSelector->addChangeListener (this);

    CallOutBox callOut (*fontColourSelector, getLocalBounds(), this);
    callOut.runModalLoop();

    systemFile->setValue ("editorFontColour", fontColourSelector->getCurrentColour().toString());
    systemFile->saveIfNeeded();
}

//=================================================================================================
void MarkdownEditor::setFontSize()
{
    fontSizeSlider.setValue (systemFile->getValue ("fontSize").getDoubleValue(),
                             dontSendNotification);
    CallOutBox callOut (fontSizeSlider, getLocalBounds(), this);
    callOut.runModalLoop();

    systemFile->setValue ("fontSize", fontSizeSlider.getValue());
    systemFile->saveIfNeeded();
}

//=================================================================================================
void MarkdownEditor::codeBlockFormat()
{
    String content;
    content << newLine
        << "```" << newLine
        << getHighlightedText() << newLine
        << "```" << newLine;

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::inlineFormat (const inlineFormatIndex& format)
{
    String content (getHighlightedText());
    const bool selectNothing = content.isEmpty();

    if (format == bold)
        content = "**" + content + "**";

    else if (format == italic)
        content = "*" + content + "*";

    else if (format == boldAndItalic)
        content = "***" + content + "***";

    else if (format == highlight)
        content = "~~" + content + "~~";

    else if (format == codeOfinline)
        content = "`" + content + "`";

    insertTextAtCaret (content);

    if (selectNothing)
    {
        moveCaretLeft (false, false);

        if (format == bold || format == highlight || format == boldAndItalic)
            moveCaretLeft (false, false);

        // here must another if instead of 'else if' because 
        // the caret should move left thrice for '***' (bold + italic)
        if (format == boldAndItalic)
            moveCaretLeft (false, false);
    }
}

//=================================================================================================
void MarkdownEditor::interLinkInsert()
{
    String linkPath (SystemClipboard::getTextFromClipboard());
    linkPath = linkPath.substring (String ("*_wdtpGetPath_*").length());

    const String titleStr (linkPath.upToFirstOccurrenceOf ("@_=#_itemPath_#=_@", false, false));
    linkPath = linkPath.fromFirstOccurrenceOf ("@_=#_itemPath_#=_@", false, false);

    const String siteRoot (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName()
                           + File::separatorString);
    linkPath = linkPath.fromFirstOccurrenceOf (siteRoot, false, false);

    const String currentHtmlRelativeToRoot (HtmlProcessor::getRelativePathToRoot (
        DocTreeViewItem::getHtmlFileOrDir (parent->getCurrentTree())));

    String content;
    content << "[" << titleStr << "](" << currentHtmlRelativeToRoot << linkPath.replace ("\\", "/") << ")";

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::authorInsert()
{
    String content;
    content << newLine 
        << ">>> " << FileTreeContainer::projectTree.getProperty ("owner").toString() 
        << newLine;

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::captionInsert()
{
    insertTextAtCaret (newLine + "^^ ");
}

//=================================================================================================
void MarkdownEditor::insertTitle (const int level)
{
    String content;

    if (1 == level)
        content << newLine << "# ";

    else if (2 == level)
        content << newLine << "## ";

    else if (3 == level)
        content << newLine << "### ";

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::endnoteInsert()
{
    insertTextAtCaret ("[^]");
    moveCaretLeft (false, false);
}

//=================================================================================================
void MarkdownEditor::tocInsert()
{
    insertTextAtCaret (newLine + "[TOC]" + newLine + newLine);
}

//=================================================================================================
void MarkdownEditor::identifierInsert()
{
    insertTextAtCaret (newLine + "*********************************" + newLine + newLine);
}

//=================================================================================================
void MarkdownEditor::orderListInsert()
{
    String content;
    content << newLine
            << "+ " << newLine
            << "+ " << newLine
            << "+ " << newLine;

    insertTextAtCaret (content);

    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretToEndOfLine (false);
}

//=================================================================================================
void MarkdownEditor::unorderListInsert()
{
    String content;
    content << newLine
            << "- " << newLine
            << "- " << newLine
            << "- " << newLine;

    insertTextAtCaret (content);

    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretToEndOfLine (false);
}

//=================================================================================================
void MarkdownEditor::alignRightInsert()
{
    insertTextAtCaret (newLine + ">>> ");
}

//=================================================================================================
void MarkdownEditor::alignCenterInsert()
{
    insertTextAtCaret (newLine + ">|< ");
}

//=================================================================================================
void MarkdownEditor::quotaInsert()
{
    insertTextAtCaret (newLine + "> ");
}

//=================================================================================================
void MarkdownEditor::tableInsert()
{
    String content;
    content << newLine
            << " H1 | H2 | H3 " << newLine
            << "--------------" << newLine
            << " 11 | 12 | 13 " << newLine
            << " 21 | 22 | 23 " << newLine
            << " 31 | 32 | 33 " << newLine
            << " 41 | 42 | 43 " << newLine << newLine
            << "^^ " << TRANS ("Table: ");

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::showAllKeywords ()
{   
    ScopedPointer<KeywordsComp> keywordsComp = new KeywordsComp (true);
    CallOutBox callOut (*keywordsComp, getLocalBounds (), this);
    callOut.runModalLoop ();

//     parent->getSetupPanel ()->updateDocPanel ();
//     DocTreeViewItem::needCreate (parent->getCurrentTree ());
}

//=================================================================================================
void MarkdownEditor::hyperlinkInsert()
{
    AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", String());
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    String content;

    if (0 == dialog.runModalLoop())
    {
        content << "[](" << dialog.getTextEditor ("name")->getText().trim() << ") ";

        insertTextAtCaret (content);
    }
}

//=================================================================================================
void MarkdownEditor::insertImages()
{
    FileChooser fc (TRANS ("Select Images..."), File::nonexistent,
                    "*.jpg;*.jpeg;*.png;*.gif", true);

    if (!fc.browseForMultipleFilesToOpen())
        return;

    Array<File> imageFiles (fc.getResults());
    insertImages (imageFiles);
}

//=================================================================================================
void MarkdownEditor::insertImages (const Array<File>& imageFiles)
{
    // remove non-image file(s)
    Array<File> files (imageFiles);

    for (int i = files.size(); --i >= 0; )
    {
        if (!files[i].hasFileExtension (".jpg;jpeg;png;gif"))
            files.remove (i);
    }

    // doesn't import project-internal images
    const File& projectDir (FileTreeContainer::projectFile.getParentDirectory());

    if (files[0].getFullPathName().contains (projectDir.getFullPathName()))
    {
        SHOW_MESSAGE (TRANS ("Can't import image(s) inside the current project!"));
        return;
    }

    // copy and insert image-syntax
    ValueTree& docTree (parent->getCurrentTree());
    const File imgPath (DocTreeViewItem::getMdFileOrDir (docTree).getSiblingFile ("media"));
    String content;

    for (auto f : files)
    {
        const File targetFile (imgPath.getChildFile (f.getFileName()).getNonexistentSibling (false));
        targetFile.create();

        if (f.copyFileTo (targetFile))
            content << newLine << "![ ](media/" << targetFile.getFileName() << ")" << newLine
            << "^^ " << TRANS ("Image: ") << newLine;
        else
            SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName());
    }

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::autoWrapSelected (const KeyPress& key)
{
    const String& content (getHighlightedText());
    String keyStr (key.getTextDescription().replace ("shift + 8", "*").replace ("shift + `", "~~"));
    keyStr = keyStr.replace ("ctrl + 8", "*").replace ("ctrl + `", "~~"); // for Chinese IME
    keyStr = keyStr.replace ("command + 8", "*").replace ("command + `", "~~"); // for Chinese IME
    //DBGX (keyStr);

    insertTextAtCaret (keyStr + content + keyStr);

    if (keyStr != getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 1))
        && keyStr != "~~"
        && keyStr != "`")
    {
        setHighlightedRegion (Range<int> (getCaretPosition() - content.length() - 1, getCaretPosition() - 1));
    }

    // move the caret after input 2 '*'
    if (String ("*") == getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 1))
        && String ("*") == getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())))
    {
        setCaretPosition (getCaretPosition() + 1);
    }
}

//=================================================================================================
void MarkdownEditor::tabKeyInput()
{
    if (getHighlightedText().isEmpty())
    {
        const int position = getCaretPosition();
        String content ("    ");
        moveCaretUp (false);

        while (getCaretPosition() - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) != "\n")
        {
            moveCaretUp (false);
        }

        if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "- ")
            content += "- ";

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "+ ")
            content += "+ ";

        setCaretPosition (position);
        insertTextAtCaret (content);
    } 
    else
    {
        StringArray content;
        content.addLines (getHighlightedText());

        for (int i = content.size(); --i >= 0; )
            content.getReference (i) = "    " + content.getReference (i);

        insertTextAtCaret (content.joinIntoString (newLine));
        moveCaretRight (false, false);
    }
}

//=================================================================================================
void MarkdownEditor::shiftTabInput()
{
    if (getHighlightedText().isEmpty())
    {
        moveCaretToStartOfLine (false);

        if (getTextInRange (Range<int>(getCaretPosition(), getCaretPosition() + 4)) == "    ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 4));
        
        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 3)) == "   ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 3));

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "  ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 2));

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 1)) == " ")
            setHighlightedRegion (Range<int> (getCaretPosition(), getCaretPosition() + 1));

        insertTextAtCaret (String());
    }
    else // let the selected anti-indent
    {
        StringArray content;
        content.addLines (getHighlightedText());

        for (int i = content.size(); --i >= 0; )
        {
            if (content[i].substring (0, 4) == "    ")
                content.getReference (i) = content[i].substring(4);

            else if (content[i].substring (0, 3) == "   ")
                content.getReference (i) = content[i].substring (3);

            else if (content[i].substring (0, 2) == "  ")
                content.getReference (i) = content[i].substring (2);

            else if (content[i].substring (0, 1) == " ")
                content.getReference (i) = content[i].substring (1);
        }

        insertTextAtCaret (content.joinIntoString (newLine));
    }
}

//=================================================================================================
void MarkdownEditor::returnKeyInput()
{
    if (getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) == "\n")
    {
        TextEditor::keyPressed (KeyPress (KeyPress::returnKey));
        return;
    }

    const int position = getCaretPosition();
    String content;

    // cancel list mark when user doesn't want input anything on the current line
    if (getTextInRange (Range<int> (position - 2, position)) == "- "
        && getTextInRange (Range<int> (position - 3, position - 2)) == "\n")
    {
        moveCaretToStartOfLine (true);
        insertTextAtCaret ("    - ");
    }
    else if (getTextInRange (Range<int> (position - 2, position)) == "+ "
             && getTextInRange (Range<int> (position - 3, position - 2)) == "\n")
    {
        moveCaretToStartOfLine (true);
        insertTextAtCaret ("    + ");
    }
    else if ((getTextInRange (Range<int> (position - 6, position)) == "    - "
             || getTextInRange (Range<int> (position - 6, position)) == "    + ")
             && getTextInRange (Range<int> (position - 7, position - 6)) == "\n")
    {
        moveCaretToStartOfLine (true);
        insertTextAtCaret (newLine);
    }
    
    else  // inherit the list mark when the previous paragraph has one
    {
        moveCaretToStartOfLine (false);

        while (getCaretPosition() - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) != "\n")
        {
            moveCaretUp (false);
        }

        if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "- ")
            content += "- ";

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 2)) == "+ ")
            content += "+ ";

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 6)) == "    - ")
            content += "    - ";

        else if (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 6)) == "    + ")
            content += "    + ";

        setCaretPosition (position);
        TextEditor::keyPressed (KeyPress (KeyPress::returnKey));
        insertTextAtCaret (content);
    }    
}

//=================================================================================================
void MarkdownEditor::pasteForCtrlV()
{
    const String& content (SystemClipboard::getTextFromClipboard());
    const int position = getCaretPosition();

    String linkText (TRANS ("Click here"));
    bool needSelectLinkText = false;

    // url or web-image
    if (content.substring (0, 4) == "http")
    {
        // image
        if (content.getLastCharacters (3) == "jpg"
            || content.getLastCharacters (3) == "JPG"
            || content.getLastCharacters (3) == "png"
            || content.getLastCharacters (3) == "PNG"
            || content.getLastCharacters (3) == "gif"
            || content.getLastCharacters (3) == "GIF"
            || content.getLastCharacters (4) == "jpeg"
            || content.getLastCharacters (4) == "JPEG")
        {
            insertTextAtCaret ("![](" + content + ")");
        }
        else     // url
        {
            if (getHighlightedText().isNotEmpty())
                linkText = getHighlightedText();
            else
                needSelectLinkText = true;

            insertTextAtCaret ("[" + linkText + "](" + content + ")");
        }

    }

    // internal link
    else if (content.contains ("*_wdtpGetPath_*"))  
        interLinkInsert();

    // others
    else  
        TextEditor::keyPressed (KeyPress ('v', ModifierKeys::commandModifier, 0));

    // select 'Click here' when paste an url and selected nothing before
    if (needSelectLinkText)  
        setHighlightedRegion (Range<int> (position + 1, position + linkText.length() + 1));
}

//=================================================================================================
bool MarkdownEditor::keyPressed (const KeyPress& key)
{
    // tab
    if (key == KeyPress (KeyPress::tabKey))
    {
        tabKeyInput();
        return true;
    }

    // shift + tab
    else if (key == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0))
    {
        shiftTabInput();
        return true;
    }

    // F3 for search the next of current selection
    else if (key == KeyPress (KeyPress::F3Key))
    {
        searchBySelectNext();
        return true;
    }

    // Shift + F3 for search the previous of current selection
    else if (key == KeyPress (KeyPress::F3Key, ModifierKeys::shiftModifier, 0))
    {
        searchBySelectPrev();
        return true;
    }

    // return-key 
    else if (key == KeyPress (KeyPress::returnKey))
    {
        returnKeyInput();
        return true;
    }

    // ctrl + return: insert new paragraph above the current paragraph
    else if (key == KeyPress (KeyPress::returnKey, ModifierKeys::commandModifier, 0))
    {
        moveCaretToStartOfLine (false);

        while (getCaretPosition() - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) != "\n")
        {
            moveCaretUp (false);
        }

        TextEditor::keyPressed (KeyPress (KeyPress::returnKey));
        return moveCaretUp (false);
    }

    // ctrl + v: paste
    else if (key == KeyPress ('v', ModifierKeys::commandModifier, 0))
    {
        pasteForCtrlV();
        return true;
    }

    // cut/copy the current paragraph when selected nothing
    else if (key == KeyPress ('x', ModifierKeys::commandModifier, 0)
             || key == KeyPress ('c', ModifierKeys::commandModifier, 0))
    {
        if (getHighlightedText().isEmpty())
        {
            //DBGX (getTextInRange (Range<int> (getCaretPosition(), getCaretPosition() + 1)));
            moveCaretToStartOfLine (false);

            while (getCaretPosition() - 1 >= 0
                   && getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) != "\n")
            {
                moveCaretUp (false);
            }

            int startAt = getCaretPosition();
            int endAt = getText().indexOfAnyOf (newLine, startAt, true);

            // last line and no '\n' at the end
            if (-1 == endAt)
                endAt = getTotalNumChars();

            // select from the previous line, by this way, there're no extra empty line after cut
            if (startAt - 1 >= 0)
                --startAt;

            // select to the next line, by this way, there're no extra empty line after cut
            if (endAt + 1 < getTotalNumChars())
                ++endAt;

            setHighlightedRegion (Range<int> (startAt, endAt));
        }

        return TextEditor::keyPressed (key);
    }

    // Markdown shortcut below...

    // format bold
    else if (key == KeyPress ('b', ModifierKeys::commandModifier, 0))
        inlineFormat (bold);

    // format italic
    else if (key == KeyPress ('i', ModifierKeys::commandModifier, 0))
        inlineFormat (italic);

    // format highlight
    else if (key == KeyPress ('u', ModifierKeys::commandModifier, 0))
        inlineFormat (highlight);

    // format inline-code
    else if (key == KeyPress ('l', ModifierKeys::commandModifier, 0))
        inlineFormat (codeOfinline);

    // code block
    else if (key == KeyPress ('k', ModifierKeys::commandModifier, 0))
        codeBlockFormat();

    // insert hyperlink
    else if (key == KeyPress ('h', ModifierKeys::commandModifier, 0))
        hyperlinkInsert();

    // insert images
    else if (key == KeyPress ('m', ModifierKeys::commandModifier, 0))
        insertImages();

    // insert table
    else if (key == KeyPress ('t', ModifierKeys::commandModifier, 0))
        tableInsert();

    // insert align center
    else if (key == KeyPress ('n', ModifierKeys::commandModifier, 0))
        alignCenterInsert();

    // insert align right
    else if (key == KeyPress ('r', ModifierKeys::commandModifier, 0))
        alignRightInsert();

    // insert caption
    else if (key == KeyPress ('p', ModifierKeys::commandModifier, 0))
        captionInsert();

    // insert author and date
    else if (key == KeyPress ('o', ModifierKeys::commandModifier, 0))
        authorInsert();

    // auto-wrap the selected (when input '`, *, **, ~~' whilst some text was selected)
    else if (getHighlightedText().isNotEmpty() && (key == KeyPress('`')
                                                   || key == KeyPress ('*')
                                                   || key == KeyPress ('8', ModifierKeys::shiftModifier, 0)
                                                   || key == KeyPress ('8', ModifierKeys::commandModifier, 0) // for Chinese IME
                                                   || key == KeyPress ('`', ModifierKeys::commandModifier, 0) // for Chinese IME
                                                   || key == KeyPress ('`', ModifierKeys::shiftModifier, 0)))
    {
        autoWrapSelected (key);
        return true;
    }

    // English punctuation matching...
    else if (key == KeyPress ('\'', ModifierKeys::shiftModifier, 0))
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\"" + selectedStr + "\"");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('\''))
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\'" + selectedStr + "\'");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('['))
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("[" + selectedStr + "]");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('[', ModifierKeys::shiftModifier, 0))
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("{" + selectedStr + "}");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('9', ModifierKeys::shiftModifier, 0)) // '('
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("(" + selectedStr + ")");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress (',', ModifierKeys::shiftModifier, 0)) // '<'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("<" + selectedStr + ">");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    // Chinese punctuation matching
    else if (key.getKeyCode() == 0)
    {
        puncMatchingForChinese (key);
        return true;
    }

    //DBGX (key.getKeyCode());
    return TextEditor::keyPressed (key);
}

//=================================================================================================
const bool MarkdownEditor::puncMatchingForChinese (const KeyPress& key)
{
    selectedForPunc = getHighlightedText();
    bool returnValue = TextEditor::keyPressed (key);
    startTimer (5);    

    return returnValue;
}

//=================================================================================================
void MarkdownEditor::timerCallback()
{
    const Range<int> lastPosition (getCaretPosition() - 1, getCaretPosition());
    const String& lastChar (getTextInRange (lastPosition));
    bool puncMatched = false;
    //DBGX (lastChar);

    if (lastChar == CharPointer_UTF8 ("\xe2\x80\x9c")) // 1. Chinese "
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + String (CharPointer_UTF8 ("\xe2\x80\x9d")));
    }
    
    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x9d")) // Chinese "
    {
        puncMatched = true;
        moveCaretLeft (false, true);
        insertTextAtCaret (String (CharPointer_UTF8 ("\xe2\x80\x9c")) + selectedForPunc
                           + String (CharPointer_UTF8 ("\xe2\x80\x9d")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x98")) // 2. Chinese '
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + String (CharPointer_UTF8 ("\xe2\x80\x99")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe2\x80\x99")) // Chinese '
    {
        puncMatched = true;
        moveCaretLeft (false, true);
        insertTextAtCaret (String (CharPointer_UTF8 ("\xe2\x80\x98")) + selectedForPunc
                           + String (CharPointer_UTF8 ("\xe2\x80\x99")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe3\x80\x90")) // 3. Chinese [
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + String (CharPointer_UTF8 ("\xe3\x80\x91")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xe3\x80\x8a")) // 4. Chinese <<
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + String (CharPointer_UTF8 ("\xe3\x80\x8b")));
    }

    else if (lastChar == CharPointer_UTF8 ("\xef\xbc\x88")) // 5. Chinese (
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + String (CharPointer_UTF8 ("\xef\xbc\x89")));
    }

    else if (lastChar == "{") // 6. Chinese {
    {
        puncMatched = true;
        insertTextAtCaret (selectedForPunc + "}");
    }

    if (puncMatched && selectedForPunc.isEmpty())
        moveCaretLeft (false, false);

    selectedForPunc.clear();
    stopTimer();
}

//=================================================================================================
void MarkdownEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &fontSizeSlider)
    {
        parent->getEditor()->setFont ((float)slider->getValue());
        parent->getEditor()->applyFontToAllText ((float)slider->getValue());
    }
}

//=================================================================================================
void MarkdownEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == fontColourSelector)
    {
        Colour textClr (fontColourSelector->getCurrentColour());
        parent->getEditor()->setColour (TextEditor::textColourId, textClr);
        parent->getEditor()->setColour (CaretComponent::caretColourId, textClr.withAlpha (0.6f));
        parent->getEditor()->applyFontToAllText (systemFile->getValue ("fontSize").getFloatValue());
    }
    else if (source == bgColourSelector)
    {
        parent->getEditor()->setColour (TextEditor::backgroundColourId, bgColourSelector->getCurrentColour());
    }
}

//=================================================================================================
bool MarkdownEditor::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

//=================================================================================================
void MarkdownEditor::filesDropped (const StringArray& pathes, int, int)
{
    Array<File> files;

    for (auto path : pathes)
        files.add (File (path));

    insertImages (files);
}

//=================================================================================================
void MarkdownEditor::addSelectedToKeywords()
{
    ValueTree& docTree (parent->getCurrentTree());
    const String& selectedStr = getHighlightedText();
    const String& currentKeyWords (docTree.getProperty ("keywords").toString().trim());

    String keyWords (currentKeyWords);

    // update the doc-tree
    if (currentKeyWords.isNotEmpty())
    {
        if (!currentKeyWords.containsIgnoreCase (selectedStr))
            keyWords = currentKeyWords + ", " + selectedStr;
    }
    else
    {
        keyWords = selectedStr;
    }

    docTree.setProperty ("keywords", keyWords, nullptr);
}

//=================================================================================================
void MarkdownEditor::searchBySelectPrev()
{
    const String& selected (getHighlightedText());

    if (selected.isNotEmpty())
    {
        const int startIndex = getText().substring (0, getCaretPosition() - 1).lastIndexOfIgnoreCase (selected);

        if (startIndex != -1)
        {
            //Array<Range<int>> rangeArray;
            //rangeArray.add (Range<int> (startIndex, startIndex + selected.length()));
            //setTemporaryUnderlining (rangeArray);
            setHighlightedRegion (Range<int> (startIndex, startIndex + selected.length()));
            return;
        }
    }
 
    LookAndFeel::getDefaultLookAndFeel().playAlertSound();
}

//=================================================================================================
void MarkdownEditor::searchBySelectNext()
{
    const String& selected (getHighlightedText());

    if (selected.isNotEmpty())
    {
        const int startIndex = getText().indexOfIgnoreCase (getCaretPosition() + selected.length(), selected);

        if (startIndex != -1)
        {
            //Array<Range<int>> rangeArray;
            //rangeArray.add (Range<int> (startIndex, startIndex + selected.length()));
            //setTemporaryUnderlining (rangeArray);
            setHighlightedRegion (Range<int> (startIndex, startIndex + selected.length()));
            return;
        }
    }
     
    LookAndFeel::getDefaultLookAndFeel().playAlertSound();
}


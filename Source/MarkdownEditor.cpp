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
    fontSizeSlider.setRange (12.0, 60.0, 0.1);
    fontSizeSlider.setDoubleClickReturnValue (true, 20.0);
    fontSizeSlider.setSize (300, 60);
    fontSizeSlider.addListener (this);
}

//=================================================================================================
void MarkdownEditor::paint (Graphics& g)
{
    TextEditor::paint (g);
    g.setColour (Colours::grey);
    g.drawVerticalLine (getWidth () - 1, 0, getBottom () - 0.f);
}

//=================================================================================================
void MarkdownEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const File& docFile (parent->getCurrentDocFile ());

    if (e->mods.isPopupMenu ())
    {
        menu.addItem (pickTitle, TRANS ("Pickup as Title"), getHighlightedText ().isNotEmpty ());
        menu.addItem (addKeywords, TRANS ("Add to Keywords"), getHighlightedText ().isNotEmpty ());
        menu.addItem (pickDesc, TRANS ("Pickup as Description"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();

        PopupMenu insertMenu;
        String ctrlStr ("  (Ctrl + ");

#if JUCE_MAC
        ctrlStr = "  (Cmd + ";
#endif

        insertMenu.addItem (insertImage, TRANS ("Iamge(s)...") + ctrlStr + "M)");
        insertMenu.addItem (insertHyperlink, TRANS ("Hyperlink...") + ctrlStr + "H)");
        insertMenu.addItem (insertTable, TRANS ("Table (4 x 3)") + ctrlStr + "T)");
        insertMenu.addItem (insertQuota, TRANS ("Quotation"));
        insertMenu.addSeparator ();

        insertMenu.addItem (insertAlignCenter, TRANS ("Align Center") + ctrlStr + "N)");
        insertMenu.addItem (insertAlignRight, TRANS ("Align Right") + ctrlStr + "R)");
        insertMenu.addSeparator ();

        insertMenu.addItem (insertUnoerderList, TRANS ("Unordered List"));
        insertMenu.addItem (insertOrderList, TRANS ("Ordered List"));
        insertMenu.addSeparator ();

        insertMenu.addItem (insertFirstTitle, TRANS ("Primary Heading"));
        insertMenu.addItem (insertSecondTitle, TRANS ("Secondary Heading"));
        insertMenu.addItem (insertThirdTitle, TRANS ("Tertiary Heading"));
        insertMenu.addSeparator ();

        insertMenu.addItem (insertCaption, TRANS ("Image/Table Caption") + ctrlStr + "P)");
        insertMenu.addItem (insertSeparator, TRANS ("Separator"));
        insertMenu.addItem (insertAuthor, TRANS ("Author and Date") + ctrlStr + "O)");
        insertMenu.addSeparator ();

        const String internalLinkStr (SystemClipboard::getTextFromClipboard ());
        insertMenu.addItem (insertInterLink, TRANS ("Internal Link"), internalLinkStr.contains ("@_=#_itemPath_#=_@"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile ());

        PopupMenu formatMenu;
        formatMenu.addItem (formatBold, TRANS ("Bold") + ctrlStr + "B)");
        formatMenu.addItem (formatItalic, TRANS ("Italic") + ctrlStr + "I)");
        formatMenu.addItem (formatHighlight, TRANS ("Highlight") + ctrlStr + "U)");
        formatMenu.addSeparator ();
        formatMenu.addItem (inlineCode, TRANS ("Code Inline") + ctrlStr + "L)");
        formatMenu.addItem (codeBlock, TRANS ("Code Block") + ctrlStr + "K)");

        menu.addSubMenu (TRANS ("Format"), formatMenu, docFile.existsAsFile ());
        menu.addSeparator ();

        menu.addItem (searchNext, TRANS ("Search Next Selection...") + "  F3", getHighlightedText ().isNotEmpty ());
        menu.addItem (searchPrev, TRANS ("Search Prev Selection...") + "  Shift + F3", getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();

        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator ();

        PopupMenu editorSetup;
        editorSetup.addItem (fontSize, TRANS ("Font Size..."));
        editorSetup.addItem (fontColor, TRANS ("Font Color..."));
        editorSetup.addItem (setBackground, TRANS ("Backgroud..."));
        editorSetup.addSeparator ();
        editorSetup.addItem (resetDefault, TRANS ("Reset to Default"));

        menu.addSubMenu (TRANS ("Editor Setup"), editorSetup, docFile.existsAsFile ());
    }
}

//=================================================================================================
void MarkdownEditor::performPopupMenuAction (int index)
{
        if (addKeywords == index)           addSelectedToKeywords ();
    else if (pickTitle == index)            pickSelectedAsTitle ();
    else if (pickDesc == index)             pickAsDescription ();
    else if (searchPrev == index)           searchBySelectPrev ();
    else if (searchNext == index)           searchBySelectNext ();
    else if (insertImage == index)          insertImages ();
    else if (insertHyperlink == index)      hyperlinkInsert ();
    else if (insertTable == index)          tableInsert ();
    else if (insertQuota == index)          quotaInsert ();
    else if (insertAlignCenter == index)    alignCenterInsert ();
    else if (insertAlignRight == index)     alignRightInsert ();
    else if (insertUnoerderList == index)   unorderListInsert ();
    else if (insertOrderList == index)      orderListInsert ();
    else if (insertFirstTitle == index)     insertTitle (1);
    else if (insertSecondTitle == index)    insertTitle (2);
    else if (insertThirdTitle == index)     insertTitle (3);
    else if (insertCaption == index)        captionInsert ();
    else if (insertSeparator == index)      separatorInsert ();
    else if (insertAuthor == index)         authorInsert ();
    else if (insertInterLink == index)      interLinkInsert ();
    else if (formatBold == index)           inlineFormat (bold);
    else if (formatItalic == index)         inlineFormat (italic);
    else if (formatHighlight == index)      inlineFormat (highlight);
    else if (inlineCode == index)           inlineFormat (codeOfinline);
    else if (codeBlock == index)            codeBlockFormat ();
    else if (fontSize == index)             setFontSize ();
    else if (fontColor == index)            setFontColour ();
    else if (setBackground == index)        setBackgroundColour ();
    else if (resetDefault == index)         resetToDefault ();

    else
        TextEditor::performPopupMenuAction (index);
}

//=================================================================================================
void MarkdownEditor::resetToDefault ()
{
    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                      TRANS ("Are you sure you want to reset the font size,\n"
                                             "text color and background to the default?")))
    {

        systemFile->setValue ("fontSize", SwingUtilities::getFontSize ());
        systemFile->setValue ("editorFontColour", Colour (0xff303030).toString ());
        systemFile->setValue ("editorBackground", Colour (0xffdedede).toString ());

        parent->getEditor ()->setColour (TextEditor::textColourId, Colour (0xff303030));
        parent->getEditor ()->setColour (CaretComponent::caretColourId, Colour (0xff303030).withAlpha (0.6f));
        parent->getEditor ()->setColour (TextEditor::backgroundColourId, Colour (0xffdedede));
        parent->getEditor ()->setFont (SwingUtilities::getFontSize ());

        parent->getEditor ()->applyFontToAllText (SwingUtilities::getFontSize ());
        systemFile->saveIfNeeded ();
    }
}

//=================================================================================================
void MarkdownEditor::setBackgroundColour ()
{
    bgColourSelector = new ColourSelectorWithPreset ();

    bgColourSelector->setSize (450, 480);
    bgColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("editorBackground")));
    bgColourSelector->addChangeListener (this);

    CallOutBox callOut (*bgColourSelector, getLocalBounds (), this);
    callOut.runModalLoop ();

    systemFile->setValue ("editorBackground", bgColourSelector->getCurrentColour ().toString ());
    systemFile->saveIfNeeded ();
}

//=================================================================================================
void MarkdownEditor::setFontColour ()
{
    fontColourSelector = new ColourSelectorWithPreset ();

    fontColourSelector->setSize (450, 480);
    fontColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("editorFontColour")));
    fontColourSelector->addChangeListener (this);

    CallOutBox callOut (*fontColourSelector, getLocalBounds (), this);
    callOut.runModalLoop ();

    systemFile->setValue ("editorFontColour", fontColourSelector->getCurrentColour ().toString ());
    systemFile->saveIfNeeded ();
}

//=================================================================================================
void MarkdownEditor::setFontSize ()
{
    fontSizeSlider.setValue (systemFile->getValue ("fontSize").getDoubleValue (),
                             dontSendNotification);
    CallOutBox callOut (fontSizeSlider, getLocalBounds (), this);
    callOut.runModalLoop ();

    systemFile->setValue ("fontSize", fontSizeSlider.getValue ());
    systemFile->saveIfNeeded ();
}

//=================================================================================================
void MarkdownEditor::codeBlockFormat ()
{
    String content;
    content << newLine
        << "```" << newLine
        << getHighlightedText () << newLine
        << "```" << newLine;

    insertTextAtCaret (content);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::inlineFormat (const inlineFormatIndex& format)
{
    String content (getHighlightedText ());
    const bool selectNothing = content.isEmpty ();

    if (format == bold)
        content = "**" + content + "**";

    else if (format == italic)
        content = "*" + content + "*";

    else if (format == highlight)
        content = "~~" + content + "~~";

    else if (format == codeOfinline)
        content = "`" + content + "`";

    insertTextAtCaret (content);

    if (selectNothing)
    {
        moveCaretLeft (false, false);

        if (format == bold || format == highlight)
            moveCaretLeft (false, false);
    }

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::interLinkInsert ()
{
    String linkPath (SystemClipboard::getTextFromClipboard ());
    const String titleStr (linkPath.upToFirstOccurrenceOf ("@_=#_itemPath_#=_@", false, false));
    linkPath = linkPath.fromFirstOccurrenceOf ("@_=#_itemPath_#=_@", false, false);

    const String siteRoot (FileTreeContainer::projectFile.getSiblingFile ("site").getFullPathName ()
                           + File::separatorString);
    linkPath = linkPath.fromFirstOccurrenceOf (siteRoot, false, false);

    const String currentHtmlRelativeToRoot (HtmlProcessor::getRelativePathToRoot (
        DocTreeViewItem::getHtmlFileOrDir (parent->getCurrentTree ())));

    String content;
    content << "[" << titleStr << "](" << currentHtmlRelativeToRoot << linkPath.replace ("\\", "/") << ")";

    insertTextAtCaret (content);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::authorInsert ()
{
    String content;
    content << newLine << ">>> " << TRANS ("Author: ")
        << FileTreeContainer::projectTree.getProperty ("owner").toString ()
        << " " << newLine << ">>> "
        << SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString (), false)
        << " ";

    insertTextAtCaret (content);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::separatorInsert ()
{
    insertTextAtCaret (newLine + "---" + newLine);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::captionInsert ()
{
    insertTextAtCaret (newLine + "^^ ");
    saveAndUpdate ();
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
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::orderListInsert ()
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

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::unorderListInsert ()
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

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::alignRightInsert ()
{
    insertTextAtCaret (newLine + ">>> ");
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::alignCenterInsert ()
{
    insertTextAtCaret (newLine + ">|< ");
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::quotaInsert ()
{
    insertTextAtCaret (newLine + "> ");
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::tableInsert ()
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
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::hyperlinkInsert ()
{
    AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", String ());
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    String content;

    if (0 == dialog.runModalLoop ())
    {
        content << "[](" << dialog.getTextEditor ("name")->getText ().trim () << ") ";

        insertTextAtCaret (content);
        saveAndUpdate ();
    }
}

//=================================================================================================
void MarkdownEditor::insertImages ()
{
    FileChooser fc (TRANS ("Select Images..."), File::nonexistent,
                    "*.jpg;*.png;*.gif", true);
    Array<File> imageFiles;

    if (!fc.browseForMultipleFilesToOpen ())
        return;

    imageFiles = fc.getResults ();
    ValueTree& docTree (parent->getCurrentTree ());
    const File imgPath (DocTreeViewItem::getMdFileOrDir (docTree).getSiblingFile ("media"));
    String content;

    for (auto f : imageFiles)
    {
        const File targetFile (imgPath.getChildFile (f.getFileName ()).getNonexistentSibling (false));
        targetFile.create ();

        if (f.copyFileTo (targetFile))
            content << newLine << "![ ](media/" << targetFile.getFileName () << ")" << newLine
            << "^^ " << TRANS ("Image: ");
        else
            SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName ());
    }

    insertTextAtCaret (content);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::pickAsDescription ()
{
    ValueTree& docTree (parent->getCurrentTree ());
    docTree.setProperty ("description", getHighlightedText (), nullptr);

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::pickSelectedAsTitle ()
{
    ValueTree& docTree (parent->getCurrentTree ());
    docTree.setProperty ("title", getHighlightedText (), nullptr);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::autoWrapSelected (const KeyPress& key)
{
    const String content (getHighlightedText ());
    const String keyStr (key.getTextDescription().replace ("shift + 8", "*").replace ("shift + `", "~~"));
    //DBGX (keyStr);

    insertTextAtCaret (keyStr + content + keyStr);

    if (keyStr != getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 1))
        && keyStr != "~~"
        && keyStr != "`")
    {
        setHighlightedRegion (Range<int> (getCaretPosition () - content.length () - 1, getCaretPosition () - 1));
    }

    // move the caret after input 2 '*'
    if (String("*") == getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 1))
        && String ("*") == getTextInRange (Range<int> (getCaretPosition () - 1, getCaretPosition ())))
        setCaretPosition (getCaretPosition () + 1);

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::tabKeyInput ()
{
    if (getHighlightedText ().isEmpty ())
    {
        String content ("    ");
        moveCaretUp (false);

        if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 2)) == "- ")
            content += "- ";

        else if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 2)) == "+ ")
            content += "+ ";

        moveCaretDown (false);
        insertTextAtCaret (content);
    } 
    else
    {
        StringArray content;
        content.addLines (getHighlightedText ());

        for (int i = content.size (); --i >= 0; )
            content.getReference (i) = "    " + content.getReference (i);

        insertTextAtCaret (content.joinIntoString (newLine));
        moveCaretRight (false, false);
    }

    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::shiftTabInput ()
{
    if (getHighlightedText ().isEmpty ())
    {
        moveCaretToStartOfLine (false);

        if (getTextInRange (Range<int>(getCaretPosition (), getCaretPosition () + 4)) == "    ")
            setHighlightedRegion (Range<int> (getCaretPosition (), getCaretPosition () + 4));
        
        else if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 3)) == "   ")
            setHighlightedRegion (Range<int> (getCaretPosition (), getCaretPosition () + 3));

        else if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 2)) == "  ")
            setHighlightedRegion (Range<int> (getCaretPosition (), getCaretPosition () + 2));

        else if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 1)) == " ")
            setHighlightedRegion (Range<int> (getCaretPosition (), getCaretPosition () + 1));

        insertTextAtCaret (String ());
    }
    else // let the selected anti-indent
    {
        StringArray content;
        content.addLines (getHighlightedText ());

        for (int i = content.size (); --i >= 0; )
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

    saveAndUpdate ();
}

//=================================================================================================
bool MarkdownEditor::keyPressed (const KeyPress& key)
{
    if (key == KeyPress (KeyPress::tabKey))
    {
        tabKeyInput ();
        return true;
    }
    else if (key == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0))
    {
        shiftTabInput ();
        return true;
    }

    // F3 for search the next of current selection
    else if (key == KeyPress (KeyPress::F3Key))
    {
        searchBySelectNext ();
        return true;
    }

    // Shift + F3 for search the previous of current selection
    else if (key == KeyPress (KeyPress::F3Key, ModifierKeys::shiftModifier, 0))
    {
        searchBySelectPrev ();
        return true;
    }

    // return-key 
    else if (key == KeyPress (KeyPress::returnKey))
    {
        const int position = getCaretPosition ();
        String content;
        moveCaretToStartOfLine (false);

        while (getCaretPosition () - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition () - 1, getCaretPosition ())) != "\n")
        {
            moveCaretUp (false);
        }

        if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 2)) == "- ")
            content += "- ";

        else if (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 2)) == "+ ")
            content += "+ ";

        setCaretPosition (position);
        TextEditor::keyPressed (key);
        insertTextAtCaret (content);

        saveAndUpdate ();
        return true;
    }

    // ctrl + return: insert new paragraph above the current paragraph
    else if (key == KeyPress (KeyPress::returnKey, ModifierKeys::commandModifier, 0))
    {
        moveCaretToStartOfLine (false);

        while (getCaretPosition () - 1 >= 0
               && getTextInRange (Range<int> (getCaretPosition () - 1, getCaretPosition ())) != "\n")
        {
            moveCaretUp (false);
        }

        TextEditor::keyPressed (KeyPress (KeyPress::returnKey));
        //TextEditor::keyPressed (KeyPress (KeyPress::returnKey));

        return moveCaretUp (false);
    }

    // cut/copy the current paragraph when selected nothing
    else if (key == KeyPress ('x', ModifierKeys::commandModifier, 0)
             || key == KeyPress ('c', ModifierKeys::commandModifier, 0))
    {
        if (getHighlightedText ().isEmpty ())
        {
            //DBGX (getTextInRange (Range<int> (getCaretPosition (), getCaretPosition () + 1)));
            moveCaretToStartOfLine (false);

            while (getCaretPosition () - 1 >= 0
                   && getTextInRange (Range<int> (getCaretPosition () - 1, getCaretPosition ())) != "\n")
            {
                moveCaretUp (false);
            }

            int startAt = getCaretPosition ();
            int endAt = getText ().indexOfAnyOf (newLine, startAt, true);

            // last line and no '\n' at the end
            if (-1 == endAt)
                endAt = getTotalNumChars ();

            // select from the previous line, by this way, there're no extra empty line after cut
            if (startAt - 1 >= 0)
                --startAt;

            // select to the next line, by this way, there're no extra empty line after cut
            if (endAt + 1 < getTotalNumChars ())
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
        codeBlockFormat ();

    // insert hyperlink
    else if (key == KeyPress ('h', ModifierKeys::commandModifier, 0))
        hyperlinkInsert ();

    // insert images
    else if (key == KeyPress ('m', ModifierKeys::commandModifier, 0))
        insertImages ();

    // insert table
    else if (key == KeyPress ('t', ModifierKeys::commandModifier, 0))
        tableInsert ();

    // insert align center
    else if (key == KeyPress ('n', ModifierKeys::commandModifier, 0))
        alignCenterInsert ();

    // insert align right
    else if (key == KeyPress ('r', ModifierKeys::commandModifier, 0))
        alignRightInsert ();

    // insert caption
    else if (key == KeyPress ('p', ModifierKeys::commandModifier, 0))
        captionInsert ();

    // insert author and date
    else if (key == KeyPress ('o', ModifierKeys::commandModifier, 0))
        authorInsert ();

    // auto-wrap the selected (when input '`, *, **, ~~' whilst some text was selected)
    else if (getHighlightedText().isNotEmpty() && (key == KeyPress('`')
                                                   || key == KeyPress ('*')
                                                   || key == KeyPress ('8', ModifierKeys::shiftModifier, 0)
                                                   || key == KeyPress ('`', ModifierKeys::shiftModifier, 0)))
    {
        autoWrapSelected (key);
        return true;
    }


    return TextEditor::keyPressed (key);
}

//=================================================================================================
void MarkdownEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &fontSizeSlider)
    {
        parent->getEditor ()->setFont ((float)slider->getValue ());
        parent->getEditor ()->applyFontToAllText ((float)slider->getValue ());
    }
}

//=================================================================================================
void MarkdownEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == fontColourSelector)
    {
        Colour textClr (fontColourSelector->getCurrentColour ());
        parent->getEditor ()->setColour (TextEditor::textColourId, textClr);
        parent->getEditor ()->setColour (CaretComponent::caretColourId, textClr.withAlpha (0.6f));
        parent->getEditor ()->applyFontToAllText (systemFile->getValue ("fontSize").getFloatValue ());
    }
    else if (source == bgColourSelector)
    {
        parent->getEditor ()->setColour (TextEditor::backgroundColourId, bgColourSelector->getCurrentColour ());
    }
}

//=================================================================================================
void MarkdownEditor::addSelectedToKeywords ()
{
    ValueTree& docTree (parent->getCurrentTree ());
    const String& selectedStr = getHighlightedText ();
    const String currentKeyWords (docTree.getProperty ("keywords").toString ().trim ());

    String keyWords (currentKeyWords);

    // update the doc-tree
    if (currentKeyWords.isNotEmpty ())
    {
        if (!currentKeyWords.containsIgnoreCase (selectedStr))
            keyWords = currentKeyWords + ", " + selectedStr;
    }
    else
    {
        keyWords = selectedStr;
    }

    docTree.setProperty ("keywords", keyWords, nullptr);
    saveAndUpdate ();
}

//=================================================================================================
void MarkdownEditor::searchBySelectPrev ()
{
    const String& selected (getHighlightedText ());

    if (selected.isNotEmpty ())
    {
        const int startIndex = getText ().substring (0, getCaretPosition () - 1).lastIndexOfIgnoreCase (selected);

        if (startIndex != -1)
            setHighlightedRegion (Range<int> (startIndex, startIndex + selected.length ()));
        else
            LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
    }
    else
    {
        LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
    }
}

//=================================================================================================
void MarkdownEditor::searchBySelectNext ()
{
    const String& selected (getHighlightedText ());

    if (selected.isNotEmpty ())
    {
        const int startIndex = getText ().indexOfIgnoreCase (getCaretPosition () + selected.length (), selected);

        if (startIndex != -1)
            setHighlightedRegion (Range<int> (startIndex, startIndex + selected.length ()));
        else
            LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
    }
    else
    {
        LookAndFeel::getDefaultLookAndFeel ().playAlertSound ();
    }
}

//=================================================================================================
void MarkdownEditor::saveAndUpdate ()
{
    ValueTree& docTree (parent->getCurrentTree ());
    DocTreeViewItem::needCreate (docTree);
    FileTreeContainer::saveProject ();

    parent->getSetupPanel ()->showDocProperties (docTree);
}


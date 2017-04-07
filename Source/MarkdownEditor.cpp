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

    //setLineSpacing (1.35f);
}

//=================================================================================================
void MarkdownEditor::paint (Graphics& g)
{
    TextEditor::paint (g);
    g.setColour (Colours::grey);
    g.drawVerticalLine (getWidth() - 1, 0, getBottom() - 0.f);
}

//=================================================================================================
void MarkdownEditor::popupOutlineMenu (EditAndPreview* editAndPreview, 
                                       const String& editorContent)
{
    StringArray sentences;
    sentences.addTokens (editorContent, newLine, String());
    sentences.removeEmptyStrings (true);

    // only remain the sencond and third title
    for (int i = sentences.size(); --i >= 0; )
    {
        if (sentences[i].trimStart().substring (0, 3) == "## "
            || sentences[i].trimStart().substring (0, 4) == "### ")
            continue;

        else
            sentences.remove (i);
    }

    if (sentences.size() < 1)
        return;

    sentences.insert (0, "---- " + TRANS ("Beginning") + " ----");
    sentences.add ("---- " + TRANS ("End") + " ----");

    // add menu-item from the stringArray
    PopupMenu outlineMenu;

    for (int i = 0; i < sentences.size(); ++i)
    {
        if (sentences[i].trimStart().substring (0, 3) == "## ")
            outlineMenu.addItem (i + 1, 
                                 Md2Html::extractLinkText (sentences[i].trimStart().substring (3)), 
                                 true, false);
        else if (sentences[i].trimStart().substring (0, 4) == "### ")
            outlineMenu.addItem (i + 1, ".   " 
                                 + Md2Html::extractLinkText (sentences[i].trimStart().substring (4)), 
                                 true, false);
        else
            outlineMenu.addItem (i + 1, sentences[i], true, false);
    }

    sentences.insert (0, "tempForMatchMenuSelectIndex");
    const int menuItemIndex = outlineMenu.show();  // show it here

    editAndPreview->outlineGoto (sentences, menuItemIndex);
}

//=================================================================================================
void MarkdownEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const bool docExists = parent->getCurrentDocFile().existsAsFile();
    const bool selectSomething = getHighlightedText().isNotEmpty();
    const bool notArchived = !(bool)parent->getCurrentTree().getProperty ("archive");

    if (e->mods.isPopupMenu())
    {
        menu.addItem (pickTitle, TRANS ("Pickup as Title"), selectSomething);
        menu.addItem (pickDesc, TRANS ("Pickup as Description"), selectSomething);
        menu.addSeparator();

        menu.addItem (addKeywords, TRANS ("Add to Keywords"), selectSomething);
        menu.addItem (pickFromAllKeywords, TRANS ("Keywords Table") + "...", docExists);
        menu.addSeparator();

        PopupMenu insertMenu;
        String ctrlStr ("  (Ctrl + ");

#if JUCE_MAC
        ctrlStr = "  (Cmd + ";
#endif

        insertMenu.addItem (insertMedia, TRANS ("Image/Audio/Video...") + ctrlStr + "M)");
        insertMenu.addItem (insertHyperlink, TRANS ("Hyperlink...") + ctrlStr + "H)");
        insertMenu.addSeparator();
        insertMenu.addItem (insertQuota, TRANS ("Quotation"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertNormalTable, TRANS ("Normal Table") + ctrlStr + "T)");
        insertMenu.addItem (insertInterlaced, TRANS ("Interlaced Table"));
        insertMenu.addItem (insertNoborderTable, TRANS ("Frameless Table"));
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
        insertMenu.addItem (insertBackToTop, TRANS ("Back to Top"));
        insertMenu.addSeparator();

        insertMenu.addItem (insertCaption, TRANS ("Image/Table Caption") + ctrlStr + "P)");
        insertMenu.addItem (insertSeparator, TRANS ("Separator"));
        insertMenu.addItem (insertAuthor, TRANS ("Author") + ctrlStr + "O)");
        insertMenu.addSeparator();

        const String internalLinkStr (SystemClipboard::getTextFromClipboard());
        insertMenu.addItem (insertInterLink, TRANS ("Internal Link"), internalLinkStr.contains ("*_wdtpGetPath_*"));
        menu.addSubMenu (TRANS ("Insert"), insertMenu, docExists && notArchived);

        PopupMenu formatMenu;
        formatMenu.addItem (formatBold, TRANS ("Bold") + ctrlStr + "B)");
        formatMenu.addItem (formatItalic, TRANS ("Italic") + ctrlStr + "I)");
        formatMenu.addItem (formatBoldAndItalic, TRANS ("Bold + Italic"));
        formatMenu.addSeparator();
        formatMenu.addItem (formatHighlight, TRANS ("Highlight") + ctrlStr + "U)");
        formatMenu.addItem (formatPostil, TRANS ("Postil"));
        formatMenu.addSeparator();
        formatMenu.addItem (inlineCode, TRANS ("Code Inline") + ctrlStr + "L)");
        formatMenu.addItem (codeBlock, TRANS ("Code Block") + ctrlStr + "K)");
        formatMenu.addItem (hybridLayout, TRANS ("Hybrid Layout"));
        formatMenu.addSeparator();
        formatMenu.addItem (commentBlock, TRANS ("Comment Block"));
        formatMenu.addSeparator();
        formatMenu.addItem (antiIndent, TRANS ("Anti-Indent"));
        formatMenu.addItem (forceIndent, TRANS ("Force Indent"));

        menu.addSubMenu (TRANS ("Format"), formatMenu, docExists && notArchived);

        PopupMenu expandMark;
        expandMark.addItem (latestPublish, TRANS ("Latest Publish"));
        expandMark.addItem (latestModify, TRANS ("Latest Modified"));
        expandMark.addItem (featuredArticle, TRANS ("Featured Articles"));
        expandMark.addSeparator();
        expandMark.addItem (allModify, TRANS ("Modify List"));
        expandMark.addItem (allPublish, TRANS ("Publish List"));
        expandMark.addItem (randomArticle, TRANS ("Random Articles"));
        expandMark.addItem (allKeywords, TRANS ("All Keywords"));

        menu.addSubMenu (TRANS ("Expand Mark"), expandMark, docExists && notArchived);
        menu.addSeparator();

        menu.addItem (audioRecord, TRANS ("Audio Record") + "..." + ctrlStr + "W)", docExists && notArchived);
        menu.addSeparator();

        // search
        menu.addItem (searchNext, TRANS ("Search Next Selection") + "  (F3)", selectSomething);
        menu.addItem (searchPrev, TRANS ("Search Prev Selection") + "  (Shift + F3)", selectSomething);

        PopupMenu exSearch;
        exSearch.addItem (searchByGoogle, "Google...", selectSomething);
        exSearch.addItem (searchByBing, TRANS ("Bing..."), selectSomething);
        exSearch.addItem (searchByWiki, TRANS ("Wikipedia..."), selectSomething);

        menu.addSubMenu (TRANS ("External Search Selection"), exSearch, docExists);

        PopupMenu exEdit;
        const bool selectedMediaFile = (getHighlightedText().containsIgnoreCase (".jpg")
                                        || getHighlightedText().containsIgnoreCase (".png")
                                        || getHighlightedText().containsIgnoreCase (".gif")
                                        || getHighlightedText().containsIgnoreCase (".mp3")
                                        || getHighlightedText().containsIgnoreCase (".jpeg"));

        bool canEdit = false;

        if (getHighlightedText().containsIgnoreCase (".mp3") && systemFile->getValue ("audioEditor").isNotEmpty())
            canEdit = true;
        else if ((getHighlightedText().containsIgnoreCase (".jpg")
                  || getHighlightedText().containsIgnoreCase (".png")
                  || getHighlightedText().containsIgnoreCase (".gif")
                  || getHighlightedText().containsIgnoreCase (".jpeg"))
                 && systemFile->getValue ("imageEditor").isNotEmpty())
            canEdit = true;

        exEdit.addItem (editMediaByExEditor, TRANS ("Edit by External Editor") + "...", canEdit);
        exEdit.addItem (setExEditorForMedia, TRANS ("Specify External Editor") + "...", selectedMediaFile);

        menu.addSubMenu (TRANS ("External Edit Media File"), exEdit, docExists && notArchived);
        menu.addSeparator();

        menu.addItem (showTips, TRANS ("Tips/Replace") + "..." + ctrlStr + "G)", selectSomething);
        menu.addItem (joinTips, TRANS ("Add to Tips Bank") + "...", selectSomething);
        menu.addSeparator();

        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator();

        PopupMenu editorSetup;
        editorSetup.addItem (fontSize, TRANS ("Font Size..."));
        editorSetup.addItem (fontColor, TRANS ("Font Color..."));
        editorSetup.addItem (setBackground, TRANS ("Backgroud..."));
        editorSetup.addSeparator();
        editorSetup.addItem (resetDefault, TRANS ("Reset to Default"));

        menu.addSubMenu (TRANS ("Editor Setup"), editorSetup, docExists && notArchived);

        menu.addSeparator();
        menu.addItem (outlineMenu, TRANS ("Document Outline...") + ctrlStr + "J)", docExists);
    }
}

//=================================================================================================
void MarkdownEditor::performPopupMenuAction (int index)
{
    if (addKeywords == index)
    {
        addSelectedToKeywords (getHighlightedText());
        parent->getSetupPanel()->showDocProperties (false, parent->getCurrentTree());
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (pickTitle == index)
    {
        parent->getCurrentTree().setProperty ("title", getHighlightedText(), nullptr);
        parent->getSetupPanel()->showDocProperties (false, parent->getCurrentTree());
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (pickDesc == index)
    {
        parent->getCurrentTree().setProperty ("description", getHighlightedText(), nullptr);
        parent->getSetupPanel()->showDocProperties (false, parent->getCurrentTree());
        DocTreeViewItem::needCreate (parent->getCurrentTree());
    }

    else if (outlineMenu == index)
    {
        parent->saveCurrentDocIfChanged();
        HtmlProcessor::createArticleHtml (parent->getCurrentTree(), true);

        popupOutlineMenu (parent, getText().replace (CharPointer_UTF8 ("\xef\xbc\x83"), "#"));
    }

    else if (editMediaByExEditor == index)
    {
        const File& mediaFile (parent->getCurrentDocFile().getSiblingFile ("media")
                               .getChildFile (getHighlightedText()));

        if (mediaFile.existsAsFile())
        {
            if (getHighlightedText().containsIgnoreCase (".mp3"))
            {
                Process::openDocument (systemFile->getValue ("audioEditor"), 
                                       mediaFile.getFullPathName());
            }
            else if (getHighlightedText().containsIgnoreCase (".jpg")
                      || getHighlightedText().containsIgnoreCase (".png")
                      || getHighlightedText().containsIgnoreCase (".gif")
                      || getHighlightedText().containsIgnoreCase (".jpeg"))
            {
                Process::openDocument (systemFile->getValue ("imageEditor"), 
                                       mediaFile.getFullPathName());
            }

            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("The selected media file doesn't inside this doc's media dir."));
        }
    }

    else if (setExEditorForMedia == index)
    {
        String tipInfo;
        String valueName;

        if (getHighlightedText().containsIgnoreCase (".mp3"))
        {
            tipInfo = TRANS ("Please Specify an External Editor for Audio");
            valueName = "audioEditor";
        }
        else if (getHighlightedText().containsIgnoreCase (".jpg")
                  || getHighlightedText().containsIgnoreCase (".png")
                  || getHighlightedText().containsIgnoreCase (".gif")
                  || getHighlightedText().containsIgnoreCase (".jpeg"))
        {
            tipInfo = TRANS ("Please Specify an External Editor for Image");
            valueName = "imageEditor";
        }

        FileChooser fc (tipInfo, File::getSpecialLocation (File::globalApplicationsDirectory), "*.exe");

        if (fc.browseForFileToOpen())
        {
            systemFile->setValue (valueName, fc.getResult().getFullPathName());
            systemFile->saveIfNeeded();

            SHOW_MESSAGE (TRANS ("External editor specified successful."));
        }
    }

    else if (insertSeparator == index)      insertTextAtCaret (newLine + "---" + newLine);
    else if (pickFromAllKeywords == index)  showAllKeywords();
    else if (searchPrev == index)           searchPrevious();
    else if (searchNext == index)           searchForNext();
    else if (searchByGoogle == index)       externalSearch (searchByGoogle);
    else if (searchByBing == index)         externalSearch (searchByBing);
    else if (searchByWiki == index)         externalSearch (searchByWiki);
    else if (showTips == index)             startTimer (showTipsBank, 50);
    else if (joinTips == index)             selectedAddToTipsBank();

    else if (insertMedia == index)          insertMedias();
    else if (insertHyperlink == index)      hyperlinkInsert();
    else if (insertNormalTable == index)    tableInsert (insertNormalTable);
    else if (insertInterlaced == index)     tableInsert (insertInterlaced);
    else if (insertNoborderTable == index)  tableInsert (insertNoborderTable);
    else if (insertQuota == index)          quotaInsert();
    else if (antiIndent == index)           insertIndent (false);
    else if (forceIndent == index)          insertIndent (true);
    else if (insertBackToTop == index)      insertTextAtCaret (newLine + "[TOP]" + newLine);
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

    else if (formatBold == index)           inlineFormat (formatBold);
    else if (formatItalic == index)         inlineFormat (formatItalic);
    else if (formatBoldAndItalic == index)  inlineFormat (formatBoldAndItalic);
    else if (formatHighlight == index)      inlineFormat (formatHighlight);
    else if (formatPostil == index)         inlineFormat (formatPostil);
    else if (inlineCode == index)           inlineFormat (inlineCode);
    else if (codeBlock == index)            codeBlockFormat();
    else if (hybridLayout == index)         hybridFormat();
    else if (commentBlock == index)         commentBlockFormat();

    else if (latestPublish == index)        insertExpandMark (latestPublish);
    else if (latestModify == index)         insertExpandMark (latestModify);
    else if (featuredArticle == index)      insertExpandMark (featuredArticle);
    else if (allKeywords == index)          insertExpandMark (allKeywords);
    else if (randomArticle == index)        insertExpandMark (randomArticle);
    else if (allPublish == index)           insertExpandMark (allPublish);
    else if (allModify == index)            insertExpandMark (allModify);

    else if (audioRecord == index)          recordAudio();
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

        systemFile->setValue ("fontSize", SwingUtilities::getFontSize() + 1.f);
        systemFile->setValue ("editorFontColour", Colour (0xff181818).toString());
        systemFile->setValue ("editorBackground", Colour (0xffafcc90).toString());

        parent->getEditor()->setColour (TextEditor::textColourId, Colour (0xff181818));
        parent->getEditor()->setColour (CaretComponent::caretColourId, Colour (0xff181818).withAlpha (0.6f));
        parent->getEditor()->setColour (TextEditor::backgroundColourId, Colour (0xffafcc90));
        parent->getEditor()->setFont (SwingUtilities::getFontSize() + 1.f);
        parent->getEditor()->applyFontToAllText (SwingUtilities::getFontSize() + 1.f);

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
    fontSizeSlider.setValue (systemFile->getValue ("fontSize").getDoubleValue(), dontSendNotification);
    fontSizeSlider.addListener (this);
    CallOutBox callOut (fontSizeSlider, getLocalBounds(), this);
    callOut.runModalLoop();
    
    fontSizeSlider.removeListener (this);

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
    moveCaretUp (false);
    moveCaretUp (false);
}

//=================================================================================================
void MarkdownEditor::hybridFormat()
{
    String content;
    content << newLine
        << "~~~5" << newLine
        << getHighlightedText() << newLine
        << "~~~" << newLine;

    insertTextAtCaret (content);
    moveCaretUp (false);
    moveCaretUp (false);
}

//=================================================================================================
void MarkdownEditor::commentBlockFormat()
{
    String content;
    content << newLine
        << "////////////" << newLine
        << getHighlightedText() << newLine
        << "////////////" << newLine;

    insertTextAtCaret (content);
    moveCaretUp (false);
    moveCaretUp (false);
}

//=================================================================================================
void MarkdownEditor::inlineFormat (const int format)
{ 
    String content (getHighlightedText());
    const bool selectNothing = content.isEmpty();

    if (format == formatBold)
        content = "**" + content + "**";
    else if (format == formatItalic)
        content = "*" + content + "*";
    else if (format == formatBoldAndItalic)
        content = "***" + content + "***";
    else if (format == formatHighlight)
        content = "~~" + content + "~~";
    else if (format == formatPostil)
        content = "(" + content + ")[]";
    else if (format == inlineCode)
        content = "`" + content + "`";

    insertTextAtCaret (content);

    if (selectNothing)
    {
        moveCaretLeft (false, false);

        if (format == formatBold || format == formatHighlight 
            || format == formatBoldAndItalic || format == formatPostil)
            moveCaretLeft (false, false);

        // here must another 'if' instead of 'else if' because 
        // the caret should move left thrice for '***' and '()[]' (postil)
        if (format == formatBoldAndItalic || format == formatPostil)
            moveCaretLeft (false, false);
    }
    else
    {
        if (format == formatPostil)
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
        DocTreeViewItem::getHtmlFile (parent->getCurrentTree())));

    String content;
    content << "[" << titleStr << "](" << currentHtmlRelativeToRoot << linkPath.replace ("\\", "/") << ")";

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::authorInsert()
{
    String content;
    content << newLine 
        << "(>) " << FileTreeContainer::projectTree.getProperty ("owner").toString() 
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
    insertTextAtCaret (newLine + "(>) ");
}

//=================================================================================================
void MarkdownEditor::alignCenterInsert()
{
    insertTextAtCaret (newLine + "(^) ");
}

//=================================================================================================
void MarkdownEditor::quotaInsert()
{
    insertTextAtCaret (newLine + "> ");
}

//=================================================================================================
void MarkdownEditor::insertIndent (const bool isIndent)
{
    moveCaretToStartOfLine (false);
    insertTextAtCaret (isIndent ? "(+) " : "(-) ");
}

//=================================================================================================
void MarkdownEditor::tableInsert (const int tableStyle)
{    
    String styleStr ("////////");

    if (tableStyle == insertInterlaced)
        styleStr = "========";
    else if (tableStyle == insertNormalTable)
        styleStr = "--------";

    String content;
    content << newLine
            << "(>)H1 | (^)H2 | (^)H3 | H4 " << newLine
            << styleStr << newLine
            << "11 | 12 | 13 | 14 " << newLine
            << "21 | 22 | 23 | 24 " << newLine
            << "31 | 32 | 33 | 34 " << newLine            
            << newLine
            << "^^ " << TRANS ("Table: ");

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::showAllKeywords()
{   
    StringArray kws;
    kws.addTokens (parent->getCurrentTree().getProperty ("keywords").toString(), ",", String());
    kws.trim();
    kws.removeEmptyStrings (true);

    ScopedPointer<KeywordsComp> keywordsComp = new KeywordsComp (FileTreeContainer::projectTree, 
                                                                 true, kws);
    
    if (keywordsComp->getKeywordsPicker() != nullptr)
        keywordsComp->getKeywordsPicker()->addActionListener (this);

    CallOutBox callOut (*keywordsComp, getLocalBounds(), this);
    callOut.runModalLoop();
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
void MarkdownEditor::insertMedias()
{
    FileChooser fc (TRANS ("Select Images/Audios/Videos..."), File::nonexistent,
                    "*.jpg;*.jpeg;*.png;*.gif;*.mp3;*.mp4", true);

    if (!fc.browseForMultipleFilesToOpen())
        return;

    Array<File> imageFiles (fc.getResults());
    insertMedias (imageFiles);
}

//=================================================================================================
void MarkdownEditor::insertMedias (const Array<File>& mediaFiles)
{
    // remove non-media file(s)
    Array<File> files (mediaFiles);

    for (int i = files.size(); --i >= 0; )
    {
        if (!files[i].hasFileExtension (".jpg;jpeg;png;gif;mp3;mp4"))
            files.remove (i);
    }

    // doesn't import project-internal medias
    const File& projectDir (FileTreeContainer::projectFile.getParentDirectory());

    if (files[0].getFullPathName().contains (projectDir.getFullPathName()))
    {
        SHOW_MESSAGE (TRANS ("Can't import media file(s) inside the current project!"));
        return;
    }

    // copy and insert image/audio-syntax
    ValueTree& docTree (parent->getCurrentTree());
    const File mediaPath (DocTreeViewItem::getMdFileOrDir (docTree).getSiblingFile ("media"));
    String content;

    for (auto f : files)
    {
        const File targetFile (mediaPath.getChildFile (f.getFileName()).getNonexistentSibling (false));
        targetFile.create();

        if (f.copyFileTo (targetFile))
        {
            if (targetFile.getFileExtension() == ".mp3")
                content << newLine << "~[](media/" << targetFile.getFileName() << ")" << newLine
                << "^^ " << TRANS ("Audio: ") << newLine;
                
            else if (targetFile.getFileExtension() == ".mp4")
                content << newLine << "@[](media/" << targetFile.getFileName() << " = 680)" << newLine
                << "^^ " << TRANS ("Video: ") << newLine;

            else
                content << newLine << "![](media/" << targetFile.getFileName() << ")" << newLine
                << "^^ " << TRANS ("Image: ") << newLine;
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't insert this media: ") + newLine + f.getFullPathName());
        }
    }

    insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::insertExpandMark (const int expandIndex)
{
    String markStr ("[latestPublish]");

    if (expandIndex == latestModify)
        markStr = "[latestModify]";
    else if (expandIndex == featuredArticle)
        markStr = "[featuredArticle]";
    else if (expandIndex == allKeywords)
        markStr = "[keywords]";
    else if (expandIndex == randomArticle)
        markStr = "[randomArticle]";
    else if (expandIndex == allPublish)
        markStr = "[allPublish]";
    else if (expandIndex == allModify)
        markStr = "[allModify]";
    
    insertTextAtCaret ("\n" + markStr + newLine);
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
        if (content.getLastCharacters (3).toLowerCase() == "jpg"
            || content.getLastCharacters (3).toLowerCase() == "png"
            || content.getLastCharacters (3).toLowerCase() == "gif"
            || content.getLastCharacters (4).toLowerCase() == "jpeg")
        {
            insertTextAtCaret ("![](" + content + ")");
        }
        // audio
        else if (content.getLastCharacters (3).toLowerCase() == "mp3")
        {
            insertTextAtCaret ("~[](" + content + ")");
        }
        // video
        else if (content.getLastCharacters (3).toLowerCase() == "mp4")
        {
            insertTextAtCaret ("@[](" + content + " = 680)");
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

    // include '\t'
    else if (content.containsIgnoreCase ("\t"))
        insertTextAtCaret (content.replace ("\t", "    "));

    // others
    else  
        TextEditor::keyPressed (KeyPress ('v', ModifierKeys::commandModifier, 0));

    // select 'Click here' when paste an url and selected nothing before
    if (needSelectLinkText)  
        setHighlightedRegion (Range<int> (position + 1, position + linkText.length() + 1));
}

//=================================================================================================
void MarkdownEditor::recordAudio()
{
    RecordComp recordComp (parent->getCurrentDocFile());
    recordComp.addActionListener (this);
    OptionalScopedPointer<Component> recorder (&recordComp, false);

    DialogWindow::LaunchOptions dialog;
    dialog.dialogTitle = TRANS ("Audio Record");
    dialog.escapeKeyTriggersCloseButton = false;
    dialog.resizable = false;
    dialog.content = recorder;

    dialog.runModal();
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

    // shift + tab (anti-indent)
    else if (key == KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier, 0))
    {
        shiftTabInput();
        return true;
    }

    // F3 for search the next of current selection
    else if (key == KeyPress (KeyPress::F3Key))
    {
        searchForNext();
        return true;
    }

    // Shift + F3 for search the previous of current selection
    else if (key == KeyPress (KeyPress::F3Key, ModifierKeys::shiftModifier, 0))
    {
        searchPrevious();
        return true;
    }

    // ctrl + G for show tips
    else if (key == KeyPress (KeyPress::returnKey, ModifierKeys::commandModifier, 0))
    {
        if (getHighlightedText().isNotEmpty())
            startTimer (showTipsBank, 50);
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
        inlineFormat (formatBold);

    // format italic
    else if (key == KeyPress ('i', ModifierKeys::commandModifier, 0))
        inlineFormat (formatItalic);

    // format highlight
    else if (key == KeyPress ('u', ModifierKeys::commandModifier, 0))
        inlineFormat (formatHighlight);

    // format inline-code
    else if (key == KeyPress ('l', ModifierKeys::commandModifier, 0))
        inlineFormat (inlineCode);

    // code block
    else if (key == KeyPress ('k', ModifierKeys::commandModifier, 0))
        codeBlockFormat();

    // insert hyperlink
    else if (key == KeyPress ('h', ModifierKeys::commandModifier, 0))
        hyperlinkInsert();

    // insert images
    else if (key == KeyPress ('m', ModifierKeys::commandModifier, 0))
        insertMedias();

    // insert table
    else if (key == KeyPress ('t', ModifierKeys::commandModifier, 0))
        tableInsert (insertNormalTable);

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

    // audio record
    else if (key == KeyPress ('w', ModifierKeys::commandModifier, 0))
        recordAudio();

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
        startTimer (showTipsBank, 100);
        return true;
    }

    // popup menu
    else if (key == KeyPress ('j', ModifierKeys::commandModifier, 0))
    {
        parent->saveCurrentDocIfChanged();
        HtmlProcessor::createArticleHtml (parent->getCurrentTree(), true);

        popupOutlineMenu (parent, getText().replace (CharPointer_UTF8 ("\xef\xbc\x83"), "#"));
        return true;
    }

    //DBGX (key.getKeyCode());
    const bool returnValue = TextEditor::keyPressed (key);

    // show tips
    if (key != KeyPress::deleteKey && key != KeyPress::backspaceKey
        && key != KeyPress::upKey && key != KeyPress::downKey
        && key != KeyPress::leftKey && key != KeyPress::rightKey
        && key != KeyPress::pageUpKey && key != KeyPress::pageDownKey
        && key != KeyPress::spaceKey
        && key != KeyPress (KeyPress::leftKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress (KeyPress::rightKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress (KeyPress::upKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress (KeyPress::downKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress (KeyPress::pageUpKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress (KeyPress::pageDownKey, ModifierKeys::shiftModifier, 0)
        && key != KeyPress ('z', ModifierKeys::commandModifier, 0)
        && key != KeyPress ('y', ModifierKeys::commandModifier, 0)
        && key != KeyPress ('s', ModifierKeys::commandModifier, 0)
        && key != KeyPress ('j', ModifierKeys::commandModifier, 0))
    {
        startTimer (showTipsBank, 100);
    }

    return returnValue;
}

//=================================================================================================
const bool MarkdownEditor::puncMatchingForChinese (const KeyPress& key)
{
    selectedForPunc = getHighlightedText();
    bool returnValue = TextEditor::keyPressed (key);
    startTimer (chinesePunc, 5);    

    return returnValue;
}

//=================================================================================================
void MarkdownEditor::selectedAddToTipsBank()
{
    if (parent->getCurrentDocFile().getFileName() == "tips.md"
        && parent->getCurrentDocFile().getParentDirectory().getFileName() == "docs")
        return;

    const File& tipsFile (FileTreeContainer::projectFile.getSiblingFile ("docs").getChildFile ("tips.md"));

    if (tipsFile.existsAsFile())
    {
        AlertWindow dialog (TRANS ("Addd a new tip"), TRANS ("Please input the new tip's name."),
                            AlertWindow::InfoIcon);

        dialog.addTextEditor ("name", String());
        dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

        if (0 == dialog.runModalLoop())
        {
            const String& tipName (dialog.getTextEditor ("name")->getText().trim());
            
            if (tipsFile.appendText (newLine 
                                     + "- " + tipName + newLine 
                                     + "    - " + getHighlightedText().trim()))
            {
                if (TipsBank::getInstance()->addNewTip (tipName, getHighlightedText().trim()))
                    SHOW_MESSAGE (TRANS ("A new tip has been added successful!"));
                else
                    SHOW_MESSAGE (TRANS ("This tip has been there already."));
            }
            else
            {
                SHOW_MESSAGE (TRANS ("Shomehow this tip added failed."));
            }
        }
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Cannot find 'root/tips'!\nPlease create the tips doc first."));
    }
}

//=================================================================================================
void MarkdownEditor::externalSearch (const int searchType)
{
    const String& content (getHighlightedText().trim());
    URL url;

    if (searchType == searchByGoogle)
        url = "https://www.google.com/#q=" + content + "&*";
        
    else if (searchType == searchByBing)
        url = "https://www.bing.com/search?q=" + content;

    else if (searchType == searchByWiki)
    {
        if (TRANS ("Wikipedia...") == "Wikipedia...")
            url = "https://en.wikipedia.org/wiki/" + content;
        else
            url = "https://zh.wikipedia.org/wiki/" + content;
    }

    url.launchInDefaultBrowser();
}

//=================================================================================================
void MarkdownEditor::timerCallback (int timerID)
{
    if (chinesePunc == timerID)
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
        stopTimer (chinesePunc);
    }

    else if (showTipsBank == timerID)
    {
        stopTimer (showTipsBank);
        const HashMap<String, String>& tips (TipsBank::getInstance()->getTipsBank());
        
        if (tips.size() > 0)
        {
            // get the last 2 characters if nothing has been selected
            String chars (getTextInRange (Range<int> (getCaretPosition() - 2, getCaretPosition())));

            if (getHighlightedText().isNotEmpty())
                chars = getHighlightedText().trim();

            PopupMenu tipsMenu;
            StringArray menuItems;
            menuItems.add (String());

            int i = 1;

            for (HashMap<String, String>::Iterator itr (tips); itr.next(); )
            {
                if (itr.getKey().containsIgnoreCase (chars))
                {
                    tipsMenu.addItem (i, itr.getValue());
                    menuItems.add (itr.getValue());
                    ++i;
                }
            }

            if (tipsMenu.getNumItems() > 0)
            {
                const int index = tipsMenu.showAt (getCaretRectangle()
                                                   .translated (getScreenBounds().getX() + 12,
                                                                getScreenBounds().getY() + 12));

                if (index != 0)
                {
                    const int currentPos = getCaretPosition();
                    const bool isSelectSomething = getHighlightedText().isNotEmpty();

                    if (!(bool)parent->getCurrentTree().getProperty ("archive"))
                        insertTextAtCaret (menuItems[index]);

                    if (!isSelectSomething)
                        setCaretPosition (currentPos);
                }
            }
        }
    }
}

//=================================================================================================
void MarkdownEditor::actionListenerCallback (const String& message)
{
    const String& prefix (message.substring (0, 2));
    const String& postfix (message.fromLastOccurrenceOf (".", false, true));

    if (prefix == "++")
    {
        addSelectedToKeywords (message.substring (2));
        parent->getSetupPanel()->showDocProperties (false, parent->getCurrentTree());
    }
    else if (prefix == "--")
    {
        subtractFromKeywords (message.substring (2));
        parent->getSetupPanel()->showDocProperties (false, parent->getCurrentTree());
    }
    else if (postfix == "mp3")
    {
        insertTextAtCaret (newLine + "~[](media/" + message + ")" + newLine + "^^ ");
    }

    DocTreeViewItem::needCreate (parent->getCurrentTree());
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

    insertMedias (files);
}

//=================================================================================================
void MarkdownEditor::addSelectedToKeywords (const String& selectedStr)
{
    ValueTree& docTree (parent->getCurrentTree());
    String currentKeyWords (docTree.getProperty ("keywords").toString().trim());

    // update the doc-tree
    if (currentKeyWords.isNotEmpty())
    {
        if (!currentKeyWords.containsIgnoreCase (selectedStr))
            currentKeyWords = currentKeyWords + ", " + selectedStr;
    }
    else
    {
        currentKeyWords = selectedStr;
    }

    docTree.setProperty ("keywords", currentKeyWords, nullptr);
}

//=================================================================================================
void MarkdownEditor::subtractFromKeywords (const String& keyword)
{
    ValueTree& docTree (parent->getCurrentTree());
    String currentKeyWords (docTree.getProperty ("keywords").toString().trim());

    // how could to remove a keyword that hasn't been there already?!?
    jassert (currentKeyWords.contains (keyword));

    StringArray kws;
    kws.addTokens (currentKeyWords, ",", String());
    kws.trim();
    kws.removeEmptyStrings (true);
    kws.removeString (keyword);

    docTree.setProperty ("keywords", kws.joinIntoString (", "), nullptr);
}

//=================================================================================================
void MarkdownEditor::searchPrevious()
{
    // make sure continue to search when modified the previous search result
    if (getHighlightedText().isNotEmpty())
    {
        SystemClipboard::copyTextToClipboard (getHighlightedText());
        parent->setSearchKeyword (getHighlightedText());
    }
    
    const String& searchFor (SystemClipboard::getTextFromClipboard());

    if (searchFor.isNotEmpty())
    {
        const int startIndex = getText().substring (0, getCaretPosition() - 1).lastIndexOfIgnoreCase (searchFor);

        if (startIndex != -1)
        {
            setHighlightedRegion (Range<int> (startIndex, startIndex + searchFor.length()));
            return;
        }

        SHOW_MESSAGE (TRANS ("Nothing could be found."));
    }

}

//=================================================================================================
void MarkdownEditor::searchForNext()
{
    // make sure continue to search when modified the previous search result
    if (getHighlightedText().isNotEmpty())
    {
        SystemClipboard::copyTextToClipboard (getHighlightedText());
        parent->setSearchKeyword (getHighlightedText());
    }

    const String& searchFor (SystemClipboard::getTextFromClipboard());

    if (searchFor.isNotEmpty())
    {
        const int startIndex = getText().indexOfIgnoreCase (getCaretPosition() + searchFor.length(), searchFor);

        if (startIndex != -1)
        {
            setHighlightedRegion (Range<int> (startIndex, startIndex + searchFor.length()));
            return;
        }
     
        SHOW_MESSAGE (TRANS ("Nothing could be found."));
    }     
}


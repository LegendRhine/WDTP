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
                                       const String& editorContent,
                                       bool showMessageWhenNoOutline)
{
    StringArray sentences;
    sentences.addLines (editorContent);
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

    if (sentences.size() > 0)
    {
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
    else if (showMessageWhenNoOutline)
    {
        SHOW_MESSAGE (TRANS ("This doc has no any secondary and tertiary heading"));
    }
}

//=================================================================================================
void MarkdownEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    // here need do this since the tips-menu might be showing at the present
    exitModalState (0);
    PopupMenu::dismissAllActiveMenus();

    const bool docExists = parent->getCurrentDocFile().existsAsFile();
    const bool selectSomething = getHighlightedText().isNotEmpty();
    const bool notArchived = !(bool)parent->getCurrentTree().getProperty ("archive");

    if (e->mods.isPopupMenu())
    {
        String ctrlStr ("  (Ctrl + ");

#if JUCE_MAC
        ctrlStr = "  (Cmd + ";
#endif

        menu.addItem (pickTitle, TRANS ("Pickup as Title"), selectSomething);
        menu.addItem (pickDesc, TRANS ("Pickup as Description"), selectSomething);
        menu.addItem (addKeywords, TRANS ("Add to Keywords"), selectSomething);
        menu.addSeparator();
        menu.addItem (pickFromAllKeywords, TRANS ("Project Keywords Table") + "...", docExists);
        menu.addItem (outlineMenu, TRANS ("Document Outline...") + ctrlStr + "J)", docExists);
        menu.addSeparator();

        PopupMenu insertMenu;
        insertMenu.addItem (insertMedia, TRANS ("Image/Audio/Video...") + ctrlStr + "M)");
        insertMenu.addItem (insertHyperlink, TRANS ("Hyperlink...") + ctrlStr + "E)");
        insertMenu.addSeparator();
        insertMenu.addItem (insertQuota, TRANS ("Quotation"));
        insertMenu.addItem (timeLine, TRANS ("Time Line") + "...");
        insertMenu.addSeparator();

        insertMenu.addItem (insertNormalTable, TRANS ("Normal Table") + ctrlStr + "T)");
        insertMenu.addItem (insertInterlaced, TRANS ("Interlaced Table"));
        insertMenu.addItem (insertNoborderTable, TRANS ("Frameless Table"));

        PopupMenu autoSumAndAv;
        autoSumAndAv.addItem (autoSumPara, TRANS ("Sum Numbers of This Paragraph"));
        autoSumAndAv.addItem (autoAvPara, TRANS ("Average Numbers of This Paragraph"));
        insertMenu.addSubMenu (TRANS ("Auto Sum and Average"), autoSumAndAv, docExists && notArchived);
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
        insertMenu.addItem (insertTime, TRANS ("Date and Time"));
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

        menu.addItem (syntax, TRANS ("Text Mark Syntax and Demo...") + "  (F1)");
        menu.addSeparator();

        menu.addItem (audioRecord, TRANS ("Audio Record") + "..." + ctrlStr + "W)", docExists && notArchived);

        PopupMenu exEdit;
        const bool selectedMediaFile = (getSelectedMediaType() != -1);
        const bool selectedImgFile = (getSelectedMediaType() == 0);
        bool canEdit = false;

        if ((getSelectedMediaType() == 1 && systemFile->getValue ("audioEditor").isNotEmpty())
            || (getSelectedMediaType() == 0 && systemFile->getValue ("imageEditor").isNotEmpty()))
            canEdit = true;

        exEdit.addItem (convertToJpg, TRANS ("Convert to JPG Format"),
            (getSelectedFileName().getLastCharacters (4) == ".png"
             || getSelectedFileName().getLastCharacters (4) == ".PNG"));

        exEdit.addItem (transparentImg, TRANS ("Transparentize Background"), selectedImgFile);
        exEdit.addSeparator();

        exEdit.addItem (rotateImgLeft, TRANS ("Rotate Left 90 Degress"), selectedImgFile);
        exEdit.addItem (rotateImgRight, TRANS ("Rotate Right 90 Degress"), selectedImgFile);
        exEdit.addSeparator();

        exEdit.addItem (threeQuarterWidth, TRANS ("Width Decrease a Quarter"), selectedImgFile);
        exEdit.addItem (halfWidth, TRANS ("Half Width"), selectedImgFile);
        exEdit.addSeparator();

        exEdit.addItem (editMediaByExEditor, TRANS ("Edit by External Editor") + "...", canEdit);
        exEdit.addItem (setExEditorForMedia, TRANS ("Specify External Editor") + "...", selectedMediaFile);

        menu.addSubMenu (TRANS ("Edit Media File"), exEdit, docExists && notArchived);
        menu.addSeparator();

        // search
        menu.addItem (searchNext, TRANS ("Search Next Selection") + "  (F3)", selectSomething);
        menu.addItem (searchPrev, TRANS ("Search Prev Selection") + "  (Shift + F3)", selectSomething);

        PopupMenu exSearch;
        exSearch.addItem (searchByGoogle, "Google...", selectSomething);
        exSearch.addItem (searchByBing, TRANS ("Bing..."), selectSomething);
        exSearch.addItem (searchByWiki, TRANS ("Wikipedia..."), selectSomething);
        exSearch.addItem (baiduBaike, TRANS ("Baidu Baike..."), selectSomething);
        menu.addSubMenu (TRANS ("External Search Selection"), exSearch, docExists); 
        
        // trans
        PopupMenu transMenu;
        transMenu.addItem (transByGoogle, TRANS ("Google Translate") + "...", selectSomething);
        transMenu.addItem (transByBaidu, TRANS ("Baidu Translate") + "...", selectSomething);
        menu.addSubMenu (TRANS ("External Translate Selection"), transMenu, docExists);
        menu.addSeparator();

        menu.addItem (showTips, TRANS ("Tips/Replace") + "..." + ctrlStr + "G)", selectSomething);
        menu.addItem (joinTips, TRANS ("Add to Tips Bank") + "..." + ctrlStr + "F1)", selectSomething);
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

        popupOutlineMenu (parent, getText().replace (CharPointer_UTF8 ("\xef\xbc\x83"), "#"), true);
    }

    else if (editMediaByExEditor == index)
    {
        const File& mediaFile (parent->getCurrentDocFile().getSiblingFile ("media")
                               .getChildFile (getSelectedFileName()));

        if (mediaFile.existsAsFile())
        {
            if (getSelectedMediaType() == 1)
            {
                Process::openDocument (systemFile->getValue ("audioEditor"), 
                                       mediaFile.getFullPathName());
            }
            else if (getSelectedMediaType() == 0)
            {
                Process::openDocument (systemFile->getValue ("imageEditor"), 
                                       mediaFile.getFullPathName());
            }

            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't find this file in media dir."));
        }
    }

    else if (setExEditorForMedia == index)
    {
        String tipInfo;
        String valueName;

        if (getSelectedMediaType() == 1)
        {
            tipInfo = TRANS ("Please Specify an External Editor for Audio");
            valueName = "audioEditor";
        }
        else if (getSelectedMediaType() == 0)
        {
            tipInfo = TRANS ("Please Specify an External Editor for Image");
            valueName = "imageEditor";
        }

        FileChooser fc (tipInfo, File::getSpecialLocation (File::globalApplicationsDirectory), "*.exe;*.app");

        if (fc.browseForFileToOpen())
        {
            systemFile->setValue (valueName, fc.getResult().getFullPathName());
            systemFile->saveIfNeeded();

            SHOW_MESSAGE (TRANS ("External editor specified successful."));
        }
    }

    else if (convertToJpg == index)
    {
        const File& pngFile (parent->getCurrentDocFile().getSiblingFile ("media")
                               .getChildFile (getSelectedFileName()));
        const File& jpgFile (pngFile.withFileExtension ("jpg").getNonexistentSibling (false));

        if (!pngFile.existsAsFile())
        {
            SHOW_MESSAGE (TRANS ("Can't find this file in media dir."));
            return;
        }
            
        if (SwingUtilities::convertPngToJpg (pngFile, jpgFile, 0.7f, false))
        {
            setHighlightedRegion (Range<int> (getHighlightedRegion().getStart(),
                                              getHighlightedRegion().getEnd() + 4));

            insertTextAtCaret (jpgFile.getFileName());
            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        }
        else
        {            
            SHOW_MESSAGE (TRANS ("Somehow the convert failed."));
        }
    }

    else if (transparentImg == index)
    {
        const File& origFile (parent->getCurrentDocFile().getSiblingFile ("media")
                             .getChildFile (getSelectedFileName()));
        const File& pngFile (origFile.withFileExtension ("png").getNonexistentSibling (false));

        if (!origFile.existsAsFile())
        {
            SHOW_MESSAGE (TRANS ("Can't find this file in media dir."));
            return;
        }

        if (SwingUtilities::transparentImage (origFile, pngFile, false))
        {
            setHighlightedRegion (Range<int> (getHighlightedRegion().getStart(),
                                              getHighlightedRegion().getEnd() + 4));

            insertTextAtCaret (pngFile.getFileName());
            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Somehow the convert failed."));
        }
    }

    else if (halfWidth == index || threeQuarterWidth == index)
    {
        const File& imgFile (parent->getCurrentDocFile().getSiblingFile ("media")
                             .getChildFile (getSelectedFileName()));

        if (SwingUtilities::processImageWidth (imgFile, (halfWidth == index ? 0.5f : 0.75f)))
            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        else
            SHOW_MESSAGE (TRANS ("Somehow this operation failed."));
    }

    else if (rotateImgLeft == index || rotateImgRight == index)
    {
        const File& originalmgFile (parent->getCurrentDocFile().getSiblingFile ("media")
                             .getChildFile (getSelectedFileName()));
        const File& targetFile (originalmgFile.getNonexistentSibling (false));

        if (SwingUtilities::rotateImage (originalmgFile, targetFile, (rotateImgLeft == index), false))
        {
            setHighlightedRegion (Range<int> (getHighlightedRegion().getStart(),
                                              getHighlightedRegion().getEnd() + 4));

            insertTextAtCaret (targetFile.getFileName());
            parent->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Somehow this operation failed."));
        }
    }

    else if (transByGoogle == index)
    {
        const String& selectedStr (getHighlightedText().trim());

        if (selectedStr.substring (0, 1).containsAnyOf ("abcdefghijklmnopqistuvwxyz'"
                                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ\""))
        {
            URL ("http://translate.google.cn/#en/zh-CN/"
                 + URL::addEscapeChars (selectedStr, false)).launchInDefaultBrowser();
        }
        else
        {
            URL ("http://translate.google.cn/#zh-CN/en/"
                 + URL::addEscapeChars (selectedStr, false)).launchInDefaultBrowser();
        }
        
    }

    else if (transByBaidu == index)
    {
        URL ("http://fanyi.baidu.com/#en/zh/" 
             + URL::addEscapeChars (getHighlightedText().trim(), false))
            .launchInDefaultBrowser();
    }

    else if (autoSumPara == index)          insertTextAtCaret (calculateNumbersOfCurrentParagraph (true));
    else if (autoAvPara == index)           insertTextAtCaret (calculateNumbersOfCurrentParagraph (false));
    else if (insertSeparator == index)      TextEditor::insertTextAtCaret (newLine + "---" + newLine);
    else if (pickFromAllKeywords == index)  showAllKeywords();
    else if (searchPrev == index)           searchPrevious();
    else if (searchNext == index)           searchForNext();
    else if (searchByGoogle == index)       externalSearch (searchByGoogle);
    else if (searchByBing == index)         externalSearch (searchByBing);
    else if (searchByWiki == index)         externalSearch (searchByWiki);
    else if (baiduBaike == index)           externalSearch (baiduBaike);
    else if (showTips == index)             startTimer (30);
    else if (joinTips == index)             selectedAddToTipsBank();

    else if (insertMedia == index)          insertExternalFiles();
    else if (insertHyperlink == index)      hyperlinkInsert();
    else if (insertNormalTable == index)    tableInsert (insertNormalTable);
    else if (insertInterlaced == index)     tableInsert (insertInterlaced);
    else if (insertNoborderTable == index)  tableInsert (insertNoborderTable);
    else if (timeLine == index)             insertTimeLine();
    else if (insertQuota == index)          quotaInsert();
    else if (antiIndent == index)           insertIndent (false);
    else if (forceIndent == index)          insertIndent (true);
    else if (insertBackToTop == index)      TextEditor::insertTextAtCaret (newLine + "[TOP]" + newLine);
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
    else if (insertTime == index)           dateAndTimeInsert();
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
    else if (syntax == index) URL ("http://underwaysoft.com/works/wdtp/syntaxMark.html").launchInDefaultBrowser();

    else
        TextEditor::performPopupMenuAction (index);
}

//=================================================================================================
const String MarkdownEditor::getSelectedFileName() const
{
    if (getHighlightedText().isEmpty())
        return String();

    return getTextInRange (Range<int>(getHighlightedRegion().getStart(),
                                      getHighlightedRegion().getEnd() + 4));
}

//=================================================================================================
const int MarkdownEditor::getSelectedMediaType() const
{
    const String& fileName (getSelectedFileName());
    //DBGX (fileName);

    if (fileName.isNotEmpty() && fileName.length() > 4)
    {
        if (fileName.getLastCharacters (4) == ".jpg"
            || fileName.getLastCharacters (4) == ".JPG"
            || fileName.getLastCharacters (4) == ".png"
            || fileName.getLastCharacters (4) == ".PNG"
            || fileName.getLastCharacters (4) == ".gif"
            || fileName.getLastCharacters (4) == ".GIF")
        {
            return 0;
        }

        else if (fileName.getLastCharacters (4) == ".mp3"
                 || fileName.getLastCharacters (4) == ".MP3")
        {
            return 1;
        }
    }

    return -1;
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

        parent->getMdEditor()->setColour (TextEditor::textColourId, Colour (0xff181818));
        parent->getMdEditor()->setColour (CaretComponent::caretColourId, Colour (0xff181818).withAlpha (0.6f));
        parent->getMdEditor()->setColour (TextEditor::backgroundColourId, Colour (0xffafcc90));
        parent->getMdEditor()->setFont (SwingUtilities::getFontSize() + 1.f);
        parent->getMdEditor()->applyFontToAllText (SwingUtilities::getFontSize() + 1.f);

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

    TextEditor::insertTextAtCaret (content);
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

    TextEditor::insertTextAtCaret (content);
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

    TextEditor::insertTextAtCaret (content);
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

    TextEditor::insertTextAtCaret (content);

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

    TextEditor::insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::authorInsert()
{
    String content;
    content << newLine 
        << "(>) " << FileTreeContainer::projectTree.getProperty ("owner").toString() 
        << newLine;

    TextEditor::insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::dateAndTimeInsert()
{
    TextEditor::insertTextAtCaret (SwingUtilities::getTimeStringWithSeparator (
        SwingUtilities::getCurrentTimeString(), true));
}

//=================================================================================================
void MarkdownEditor::captionInsert()
{
    TextEditor::insertTextAtCaret (newLine + "^^ ");
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

    TextEditor::insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::endnoteInsert()
{
    TextEditor::insertTextAtCaret ("[^]");
    moveCaretLeft (false, false);
}

//=================================================================================================
void MarkdownEditor::tocInsert()
{
    TextEditor::insertTextAtCaret (newLine + "[TOC]" + newLine + newLine);
}

//=================================================================================================
void MarkdownEditor::identifierInsert()
{
    TextEditor::insertTextAtCaret (newLine + "*********************************" + newLine + newLine);
}

//=================================================================================================
void MarkdownEditor::orderListInsert()
{
    String content;
    content << newLine
            << "+ " << newLine
            << "+ " << newLine
            << "+ " << newLine;

    TextEditor::insertTextAtCaret (content);

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

    TextEditor::insertTextAtCaret (content);

    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretUp (false);
    moveCaretToEndOfLine (false);
}

//=================================================================================================
void MarkdownEditor::alignRightInsert()
{
    TextEditor::insertTextAtCaret (newLine + "(>) ");
}

//=================================================================================================
void MarkdownEditor::alignCenterInsert()
{
    TextEditor::insertTextAtCaret (newLine + "(^) ");
}

//=================================================================================================
void MarkdownEditor::quotaInsert()
{
    TextEditor::insertTextAtCaret (newLine + "> ");
}

//=================================================================================================
void MarkdownEditor::insertIndent (const bool isIndent)
{
    moveCaretToStartOfLine (false);
    TextEditor::insertTextAtCaret (isIndent ? "(+) " : "(-) ");
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

    TextEditor::insertTextAtCaret (content);
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

        TextEditor::insertTextAtCaret (content);
    }
}

//=================================================================================================
void MarkdownEditor::insertTimeLine()
{
    AlertWindow window (TRANS ("Set Time Line"),
                        TRANS ("You may type a new unit in 'Time Unit' combo box."),
                        AlertWindow::NoIcon);

    StringArray timeUnits;
    timeUnits.add (TRANS (" Second"));
    timeUnits.add (TRANS (" Minute "));
    timeUnits.add (TRANS (" Hour "));
    timeUnits.add (TRANS (" Day "));
    timeUnits.add (TRANS (" Week "));
    timeUnits.add (TRANS (" Month "));
    timeUnits.add (TRANS (" Year "));

    window.addComboBox ("timeUnit", timeUnits, TRANS ("Time Unit") + ": ");
    window.getComboBoxComponent ("timeUnit")->setEditableText (true);

    window.addTextEditor ("initialValue", "0", TRANS ("Initial Value") + ": ");
    window.getTextEditor ("initialValue")->setInputRestrictions (3, "0123456789");

    window.addTextEditor ("intervalTime", "6", TRANS ("Interval Time") + ": ");
    window.getTextEditor ("intervalTime")->setInputRestrictions (3, "0123456789");

    window.addTextEditor ("duration", "180", TRANS ("Duration") + ": ");
    window.getTextEditor ("duration")->setInputRestrictions (4, "0123456789");

    window.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    window.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));
    
    if (0 == window.runModalLoop())
    {
        const String& unit (window.getComboBoxComponent ("timeUnit")->getText());
        const int iniValue = window.getTextEditor ("initialValue")->getText().getIntValue();
        const int interval = jmax (1, window.getTextEditor ("intervalTime")->getText().getIntValue());
        const int dur = jmax (interval, window.getTextEditor ("duration")->getText().getIntValue());

        StringArray result;

        // get time points
        while (result.size() * interval < dur)
        {
            const int noX = result.size() * interval + iniValue;
            const int nextNoX = noX + interval;

            if (unit == TRANS (" Second"))
                result.add ("- **" + TRANS ("No.") 
                            + String (noX / 60) + TRANS (" Minute ") + String (noX % 60) + unit + "~"
                            + String (nextNoX / 60) + TRANS (" Minute ") + String (nextNoX % 60) + unit
                            + "**" + newLine + "    - " + newLine);
            
            else
                result.add ("- **" + TRANS ("No.") 
                            + String (noX) + "~"
                            + String (nextNoX) + unit
                            + "**" + newLine + "    - " + newLine);
        }

        insertTextAtCaret (result.joinIntoString (newLine));
    }
}

//=================================================================================================
void MarkdownEditor::insertExternalFiles()
{
    FileChooser fc (TRANS ("Select files you want to insert"), File::nonexistent,
                    "*", true);

    if (!fc.browseForMultipleFilesToOpen())
        return;

    Array<File> imageFiles (fc.getResults());
    insertExternalFiles (imageFiles);
}

//=================================================================================================
void MarkdownEditor::insertExternalFiles (const Array<File>& mediaFiles)
{
    // remove non-media file(s)
    Array<File> files (mediaFiles);

    for (int i = files.size(); --i >= 0; )
    {
        if (!files[i].hasFileExtension (".jpg;png;gif;mp3;mp4"))
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

    TextEditor::insertTextAtCaret (content);
}

//=================================================================================================
void MarkdownEditor::insertExpandMark (const int expandIndex)
{
    String markStr ("[latestPublish]");

    if (expandIndex == latestModify)            markStr = "[latestModify]";
    else if (expandIndex == featuredArticle)    markStr = "[featuredArticle]";
    else if (expandIndex == allKeywords)        markStr = "[keywords]";
    else if (expandIndex == randomArticle)      markStr = "[randomArticle]";
    else if (expandIndex == allPublish)         markStr = "[allPublish]";
    else if (expandIndex == allModify)          markStr = "[allModify]";
    
    TextEditor::insertTextAtCaret ("\n" + markStr + newLine);
}

//=================================================================================================
void MarkdownEditor::returnKeyInput()
{
    if (getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())) == "\n"
        || getHighlightedText().isNotEmpty())
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
        TextEditor::insertTextAtCaret ("    - ");
    }
    else if (getTextInRange (Range<int> (position - 2, position)) == "+ "
             && getTextInRange (Range<int> (position - 3, position - 2)) == "\n")
    {
        moveCaretToStartOfLine (true);
        TextEditor::insertTextAtCaret ("    + ");
    }
    else if ((getTextInRange (Range<int> (position - 6, position)) == "    - "
             || getTextInRange (Range<int> (position - 6, position)) == "    + ")
             && getTextInRange (Range<int> (position - 7, position - 6)) == "\n")
    {
        moveCaretToStartOfLine (true);
        TextEditor::insertTextAtCaret (newLine);
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
        TextEditor::insertTextAtCaret (content);
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
            || content.getLastCharacters (3).toLowerCase() == "gif")
        {
            TextEditor::insertTextAtCaret ("![](" + content + ")");
        }
        // audio
        else if (content.getLastCharacters (3).toLowerCase() == "mp3")
        {
            TextEditor::insertTextAtCaret ("~[](" + content + ")");
        }
        // video
        else if (content.getLastCharacters (3).toLowerCase() == "mp4")
        {
            TextEditor::insertTextAtCaret ("@[](" + content + " = 680)");
        }
        else     // url
        {
            if (getHighlightedText().isNotEmpty())
                linkText = getHighlightedText();
            else
                needSelectLinkText = true;

            TextEditor::insertTextAtCaret ("[" + linkText + "](" + content + ")");
        }
    }

    // internal link
    else if (content.contains ("*_wdtpGetPath_*"))
        interLinkInsert();

    // include '\t'
    else if (content.containsIgnoreCase ("\t"))
        TextEditor::insertTextAtCaret (content.replace ("\t", "    "));
    
    else  // others
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
    // F1 for view the markup syntax web page
    if (key == KeyPress (KeyPress::F1Key))
    {
        URL ("http://underwaysoft.com/works/wdtp/syntaxMark.html").launchInDefaultBrowser();
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
    else if (key == KeyPress ('g', ModifierKeys::commandModifier, 0))
    {
        if (getHighlightedText().isNotEmpty())
            startTimer (30);
    }

    // ctrl + F1 for add the selected to tips-bank
    else if (key == KeyPress (KeyPress::F1Key, ModifierKeys::commandModifier, 0))
    {
        selectedAddToTipsBank();
    }

    // return-key 
    else if (key == KeyPress (KeyPress::returnKey))
    {
        // for tips-menu is showing
        exitModalState (0);

        if (Component::getCurrentlyModalComponent() != nullptr)
            Component::getCurrentlyModalComponent()->keyPressed (key);
        else
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

    // popup outline menu
    else if (key == KeyPress ('j', ModifierKeys::commandModifier, 0))
    {
        parent->saveCurrentDocIfChanged();
        HtmlProcessor::createArticleHtml (parent->getCurrentTree(), true);

        popupOutlineMenu (parent, getText().replace (CharPointer_UTF8 ("\xef\xbc\x83"), "#"), true);
        return true;
    }
    
    // Markup shortcut below...
    else if (key == KeyPress ('b', ModifierKeys::commandModifier, 0))    inlineFormat (formatBold);
    else if (key == KeyPress ('i', ModifierKeys::commandModifier, 0))    inlineFormat (formatItalic);
    else if (key == KeyPress ('u', ModifierKeys::commandModifier, 0))    inlineFormat (formatHighlight);
    else if (key == KeyPress ('l', ModifierKeys::commandModifier, 0))    inlineFormat (inlineCode);
    else if (key == KeyPress ('k', ModifierKeys::commandModifier, 0))    codeBlockFormat();
    else if (key == KeyPress ('e', ModifierKeys::commandModifier, 0))    hyperlinkInsert();
    else if (key == KeyPress ('m', ModifierKeys::commandModifier, 0))    insertExternalFiles();
    else if (key == KeyPress ('t', ModifierKeys::commandModifier, 0))    tableInsert (insertNormalTable);
    else if (key == KeyPress ('n', ModifierKeys::commandModifier, 0))    alignCenterInsert();
    else if (key == KeyPress ('r', ModifierKeys::commandModifier, 0))    alignRightInsert();
    else if (key == KeyPress ('p', ModifierKeys::commandModifier, 0))    captionInsert();
    else if (key == KeyPress ('o', ModifierKeys::commandModifier, 0))    authorInsert();
    else if (key == KeyPress ('w', ModifierKeys::commandModifier, 0))    recordAudio();

    // transfer the key event to tips-menu if it's showing
    else if (key == KeyPress::downKey || key == KeyPress::upKey
             || key == KeyPress::leftKey || key == KeyPress::rightKey 
             || key == KeyPress::escapeKey)
    {
        exitModalState (0);

        if (Component::getCurrentlyModalComponent() != nullptr)
        {
            Component::getCurrentlyModalComponent()->keyPressed (key);
            return true;
        }
    }

    return SwingEditor::keyPressed (key);
}

//=================================================================================================
void MarkdownEditor::insertTextAtCaret (const String& textToInsert)
{
    if (isCurrentlyModal())
        exitModalState (0);

    posBeforeInputNewText = getCaretPosition();
    SwingEditor::insertTextAtCaret (textToInsert);
    
    // popup tips
    if (textToInsert.isNotEmpty())
        startTimer (30);
}

//=================================================================================================
void MarkdownEditor::selectedAddToTipsBank()
{
    if (getHighlightedText().isEmpty())
        return;

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

            if (tipName.length() < 2)
            {
                SHOW_MESSAGE (TRANS ("The tip's name must more than 1 character"));
                return;
            }

            const String tipContent (getHighlightedText().replace ("\r", String()).replace ("\n", "<br>"));

            if (TipsBank::getInstance()->addNewTip (tipName, tipContent))
            {
                if (tipsFile.appendText ("\n- " + tipName + "\n    - " + tipContent + "\n"))
                    SHOW_MESSAGE (TRANS ("A new tip has been added successful!"));
                else
                    SHOW_MESSAGE (TRANS ("Shomehow this tip added failed."));
            }
            else
            {
                SHOW_MESSAGE (TRANS ("This tip has been there already."));
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

    else if (searchType == baiduBaike)
        url = "http://baike.baidu.com/item/" + content;

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
static void menuItemChosenCallback (int index, MarkdownEditor* mdEditor)
{
    if (mdEditor->isCurrentlyModal())
        mdEditor->exitModalState (0);

    if (index != 0 && mdEditor != nullptr)
        mdEditor->autoComplete (index);
}

//=================================================================================================
void MarkdownEditor::timerCallback()
{    
    stopTimer();
    PopupMenu::dismissAllActiveMenus();
    SwingEditor::timerCallback();
    const HashMap<String, String>& tips (TipsBank::getInstance()->getTipsBank());

    if (tips.size() < 1)
        return;

    // get the last 2 characters if nothing has been selected
    String chars (getTextInRange (Range<int> (getCaretPosition() - 2, getCaretPosition())));

    if (getHighlightedText().isNotEmpty())
        chars = getHighlightedText().trim();

    PopupMenu tipsMenu;
    menuItems.clear();
    menuItems.add (String());

    for (HashMap<String, String>::Iterator itr (tips); itr.next(); )
    {
        if (itr.getKey().containsIgnoreCase (chars))
        {
            String menuStr (itr.getValue().replace ("<br>", " "));

            if (menuStr.length() > 35)
                menuStr = menuStr.substring (0, 35) + "...";

            tipsMenu.addItem (menuItems.size(), menuStr);
            menuItems.add (itr.getValue());
        }
    }

    if (tipsMenu.getNumItems() > 0)
    {
        const Rectangle<int> posOfMenu (getCaretRectangle()
                                        .translated (getScreenBounds().getX() + 12,
                                                     getScreenBounds().getY() + 12 - getViewport()->getViewPositionY()));

        tipsMenu.showMenuAsync (PopupMenu::Options().withTargetScreenArea (posOfMenu),
                                ModalCallbackFunction::forComponent (menuItemChosenCallback, this));

        Desktop::getInstance().getMainMouseSource().setScreenPosition (posOfMenu.getPosition()
                                                                         .translated (5, 35).toFloat());

        // for ascii char input continuously, no need do this when IME enabled
        const String& lastChar (getTextInRange (Range<int> (getCaretPosition() - 1, getCaretPosition())));

        if (lastChar.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "0123456789`~!@#$%^&*()-=_+\\|[{]};:'\",<.>/?"))
            enterModalState();
    }
}

//=================================================================================================
void MarkdownEditor::autoComplete (const int index)
{
    if (getHighlightedText().isEmpty())
    {
#if JUCE_WINDOWS
        setHighlightedRegion (Range<int> (posBeforeInputNewText - 1, getCaretPosition()));
#else
        moveCaretLeft (false, true);
        moveCaretLeft (false, true);
#endif
    }

    if (!(bool)parent->getCurrentTree().getProperty ("archive"))
        TextEditor::insertTextAtCaret (menuItems[index].replace ("<br>", newLine));
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
        TextEditor::insertTextAtCaret (newLine + "~[](media/" + message + ")" + newLine + "^^ ");
    }

    DocTreeViewItem::needCreate (parent->getCurrentTree());
}

//=================================================================================================
void MarkdownEditor::sliderValueChanged (Slider* slider)
{
    if (slider == &fontSizeSlider)
    {
        parent->getMdEditor()->setFont ((float)slider->getValue());
        parent->getMdEditor()->applyFontToAllText ((float)slider->getValue());
    }
}

//=================================================================================================
void MarkdownEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == fontColourSelector)
    {
        Colour textClr (fontColourSelector->getCurrentColour());
        parent->getMdEditor()->setColour (TextEditor::textColourId, textClr);
        parent->getMdEditor()->setColour (CaretComponent::caretColourId, textClr.withAlpha (0.6f));
        parent->getMdEditor()->applyFontToAllText (systemFile->getValue ("fontSize").getFloatValue());
    }
    else if (source == bgColourSelector)
    {
        parent->getMdEditor()->setColour (TextEditor::backgroundColourId, bgColourSelector->getCurrentColour());
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

    insertExternalFiles (files);
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


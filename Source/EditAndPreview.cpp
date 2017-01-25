/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

//==============================================================================
EditAndPreview::EditAndPreview (MainContentComponent* mainComp_) :
    mainComp(mainComp_)
{
    addAndMakeVisible (webView = new WebBrowserComp(this));
    webView->setWantsKeyboardFocus(false);

    // stretched layout, arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.72);  // editor，
    layoutManager.setItemLayout (1, 2, 2, 2);           // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.28);  // propertiesPanel

    addAndMakeVisible (editor = new EditorForMd (this));
    addAndMakeVisible (setupPanel = new SetupPanel (this));
    addAndMakeVisible (layoutBar = new StrechableBar (&layoutManager, 1, true));

    // editor
    editor->setMultiLine (true);
    editor->setReturnKeyStartsNewLine (true);
    editor->setTabKeyUsedAsCharacter (true);

    Colour textClr = Colour::fromString(systemFile->getValue("editorFontColour"));

    editor->setColour (TextEditor::focusedOutlineColourId, Colour (0x000));
    editor->setColour(TextEditor::textColourId, textClr);
    editor->setColour(CaretComponent::caretColourId, textClr.withAlpha(0.6f));
    editor->setColour (TextEditor::backgroundColourId, Colour::fromString (systemFile->getValue ("editorBackground")));
    editor->setFont (systemFile->getValue ("fontSize").getFloatValue ());

    editor->setScrollBarThickness (10);
    editor->setIndents (10, 10);
    editor->setEnabled (false);
    editor->setBorder (BorderSize<int> (1, 1, 1, 1));
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
    stopTimer();
}

//=========================================================================
void EditAndPreview::resized()
{
    Component* wordArea = (webView->isVisible () ? dynamic_cast<Component*>(webView.get())
                                                : dynamic_cast<Component*>(editor.get()));

    jassert (wordArea != nullptr);

    if (getParentComponent()->getWidth() > 1020)  // stretched layout
    {
        setupPanel->setVisible(true);
        layoutBar->setVisible(true);
        
        Component* comps[] = { wordArea, layoutBar, setupPanel };
        
        layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
    }
    else  // simple-mode (only makes the editor visable)
    {
        setupPanel->setVisible(false);
        layoutBar->setVisible(false);
        wordArea->setBounds (0, 0, getWidth(), getHeight());
    }
}

//=================================================================================================
void EditAndPreview::startWork (ValueTree& newDocTree)
{
    jassert (newDocTree.isValid ());
    saveCurrentDocIfChanged ();

    if (newDocTree != docOrDirTree || docOrDirFile != DocTreeViewItem::getMdFileOrDir (newDocTree))
    {
        editor->removeListener (this);
        docOrDirTree = newDocTree;
        docOrDirFile = DocTreeViewItem::getMdFileOrDir (newDocTree);

        if (docOrDirFile.existsAsFile ())
        {
            editor->setText (docOrDirFile.loadFileAsString (), false);
            currentContent = editor->getText ();
            editor->addListener (this);
        }
    }    

    // prevent auto-enter preview mode when created a new document
    if (currentContent.length() < 3)
    {
        switchMode(false);
        editor->moveCaretToEnd(false);
    }
    else
    {
        switchMode(true);
    }
    
    // word count doesn't include ' ' and newLine. 
    setupPanel->updateWordCount (currentContent.removeCharacters (" ")
                                 .removeCharacters (newLine).length ());
}

//=================================================================================================
void EditAndPreview::switchMode(const bool switchToPreview)
{
    TopToolBar* toolBar = findParentComponentOfClass<MainContentComponent>()->getToolbar();
    jassert(toolBar != nullptr);

    if ((docOrDirTree.getType().toString() != "doc") || (switchToPreview && toolBar->getStateOfViewButton()))
    {
        previewCurrentDoc();
        toolBar->enableEditPreviewBt(!docOrDirFile.isDirectory(), true);
    } 
    else
    {
        editCurrentDoc();
        toolBar->enableEditPreviewBt(true, false);
    }
}

//=================================================================================================
void EditAndPreview::editCurrentDoc ()
{
    webView->setVisible (false);
    editor->setEnabled (true);
    editor->grabKeyboardFocus ();    

    resized ();
}

//=================================================================================================
void EditAndPreview::previewCurrentDoc ()
{
    editor->setEnabled (false);
    webView->setVisible (true);
    webView->stop ();
    
    if (docOrDirFile.existsAsFile ())
    {
        String fileUrl (HtmlProcessor::createArticleHtml (docOrDirTree, true).getFullPathName());
        
        // escape Chinese characters on OSX..
    #if JUCE_OSX
        fileUrl = CppTokeniserFunctions::addEscapeChars(fileUrl).replace("\\x", "%");
    #endif

        //DBGX (URL::addEscapeChars (fileUrl, true));
        //DBGX (fileUrl);        
        webView->goToURL(fileUrl);
    }
    else
    {
        webView->goToURL (HtmlProcessor::createIndexHtml (docOrDirTree, true).getFullPathName());
    }
    
    resized ();
}

//=================================================================================================
const bool EditAndPreview::getCureentState() const
{
    return webView->isVisible();
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    saveCurrentDocIfChanged();

    editor->removeListener (this);
    editor->setText (String (), false);
    editor->setEnabled (false);    

    setupPanel->projectClosed ();
    webView->setVisible (false);

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
const bool EditAndPreview::selectItemFromHtmlFile(const File& html)
{
    return mainComp->selectItemFromHtmlFile(html);
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when load a doc, so use this ugly judge...
    if (currentContent.compare (editor->getText()) != 0)
    {
        currentContent = editor->getText ();
        docHasChanged = true;
        DocTreeViewItem::needCreate (docOrDirTree);

        startTimer (3000);
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
    stopTimer ();
    bool returnValue = true;

    if (docHasChanged && docOrDirFile != File::nonexistent)
    {
        TemporaryFile tempFile (docOrDirFile);
        tempFile.getFile ().appendText (currentContent);

        if (tempFile.overwriteTargetFileWithTemporary())
        {
            // title
            if (docOrDirTree.getProperty ("title").toString ().isEmpty ())
            {
                const String tileStr (currentContent.trim ().upToFirstOccurrenceOf ("\n", false, true)
                                      .replace ("#", String ()).trim ());

                docOrDirTree.setProperty ("title", tileStr, nullptr);
            }

            // description
            if (docOrDirTree.getProperty("description").toString().isEmpty())
            {
                // get the description (the second line which not empty-line)
                // the first line should be title
                StringArray contentArray;
                contentArray.addLines (currentContent);
                contentArray.removeEmptyStrings ();
                String description;

                for (int i = 1; i < contentArray.size (); ++i)
                {
                    if (contentArray.getReference (i).trim ().isNotEmpty ())
                    {
                        description = contentArray.getReference (i).trim ()
                            .replace ("+", String ()).replace ("-", String ())
                            .replace ("*", String ()).replace ("#", String ())
                            .replace ("`", String ()).replace (">", String ())
                            .replace ("|", String ()).replace ("<", String ())
                            .replace ("!", String ()).replace ("[", " ")
                            .replace ("]", " ");

                        break;
                    }
                }

                docOrDirTree.setProperty ("description", description, nullptr);
            }

            docHasChanged = false;
            setupPanel->showDocProperties (docOrDirTree);
            returnValue = FileTreeContainer::saveProject();
        }
        else
        {
            returnValue = false;
        }

        // word count doesn't include ' ' and newLine. 
        setupPanel->updateWordCount (currentContent.removeCharacters (" ")
                                     .removeCharacters (newLine).length());
    }

    return returnValue;
}

//=================================================================================================
EditorForMd::EditorForMd (EditAndPreview* parent_) 
    : parent (parent_),
    fontSizeSlider (Slider::LinearHorizontal, Slider::TextBoxBelow)
{
    fontSizeSlider.setRange (12.0, 60.0, 0.1);
    fontSizeSlider.setDoubleClickReturnValue (true, 20.0);
    fontSizeSlider.setSize (300, 60);
    fontSizeSlider.addListener (this);
}

//=================================================================================================
void EditorForMd::paint (Graphics& g)
{
    TextEditor::paint (g);
    g.setColour (Colours::grey);
    g.drawVerticalLine (getWidth() - 1, 0, getBottom () - 0.f);
}

//=================================================================================================
void EditorForMd::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const File& docFile (parent->getCurrentDocFile ());

    if (e->mods.isPopupMenu())
    {
        menu.addItem (20, TRANS ("Add to Keywords"), getHighlightedText ().isNotEmpty ());
        menu.addItem (23, TRANS ("Pickup Description"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();

        /*menu.addItem (21, TRANS ("Search Selected"), getHighlightedText ().isNotEmpty ());
        menu.addItem (22, TRANS ("Replace Selected"), getHighlightedText ().isNotEmpty ());
        menu.addSeparator ();*/

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

        menu.addItem(34, TRANS("Search Prev Selection..."), getHighlightedText().isNotEmpty());
        menu.addItem(35, TRANS("Search Next Selection..."), getHighlightedText().isNotEmpty());
        menu.addSeparator();

        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator ();

        PopupMenu editorSetup;
        editorSetup.addItem (40, TRANS ("Font Size..."));
        editorSetup.addItem (41, TRANS ("Font Color..."));
        editorSetup.addItem (42, TRANS ("Backgroud..."));
        editorSetup.addSeparator ();
        editorSetup.addItem (43, TRANS ("Reset to Default"));

        menu.addSubMenu (TRANS ("Editor Setup"), editorSetup);
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

        // update the doc-tree
        if (currentKeyWords.isNotEmpty ())
        {
            if (!currentKeyWords.containsIgnoreCase (content))
                keyWords = currentKeyWords + ", " + content;
        }
        else
        {
            keyWords = content;
        }

        docTree.setProperty ("keywords", keyWords, nullptr);
    }
    else if (23 == index)  // add the selected to this doc's description
    {
        content = getHighlightedText ();
        docTree.setProperty ("description", content, nullptr);
    }
    else if (34 == index)  // search by selected prev
    {
        searchBySelectPrev();
        return;  // don't insert anything in current content
    }
    else if (35 == index)  // search by selected next
    {
        searchBySelectNext();
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
            << " H1 | H2 | H3 " << newLine
            << "--------------" << newLine
            << " 11 | 12 | 13 " << newLine
            << " 21 | 22 | 23 " << newLine
            << " 31 | 32 | 33 " << newLine
            << " 41 | 42 | 43 " << newLine
            << newLine;
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
        content << newLine << "---" << newLine;
    }
    else if (15 == index) // author and date
    {
        content << newLine << newLine 
            << ">>> "
            << TRANS ("Author: ")
            << FileTreeContainer::projectTree.getProperty ("owner").toString ()
            << " " << newLine << ">>> "
            << SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), false)
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
    else if (40 == index)  // font size
    {
        fontSizeSlider.setValue (systemFile->getValue ("fontSize").getDoubleValue(), 
                                 dontSendNotification);
        CallOutBox callOut (fontSizeSlider, getLocalBounds (), this);
        callOut.runModalLoop ();

        systemFile->setValue ("fontSize", fontSizeSlider.getValue());
        systemFile->saveIfNeeded();
    }
    else if (41 == index)  // font color
    {
        fontColourSelector = new ColourSelectorWithPreset ();

        //fontColourSelector->setColour (ColourSelector::backgroundColourId, Colour (0xffededed));
        fontColourSelector->setSize (450, 480);
        fontColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue("editorFontColour")));
        fontColourSelector->addChangeListener (this);

        CallOutBox callOut (*fontColourSelector, getLocalBounds(), this);
        callOut.runModalLoop ();

        systemFile->setValue ("editorFontColour", fontColourSelector->getCurrentColour().toString());
        systemFile->saveIfNeeded ();
    }
    else if (42 == index)  // background color
    {
        bgColourSelector = new ColourSelectorWithPreset ();

        //bgColourSelector->setColour (ColourSelector::backgroundColourId, Colour (0xffededed));
        bgColourSelector->setSize (450, 480);
        bgColourSelector->setCurrentColour (Colour::fromString (systemFile->getValue ("editorBackground")));
        bgColourSelector->addChangeListener (this);

        CallOutBox callOut (*bgColourSelector, getLocalBounds(), this);
        callOut.runModalLoop ();

        systemFile->setValue ("editorBackground", bgColourSelector->getCurrentColour().toString());
        systemFile->saveIfNeeded ();
    }
    else if (43 == index)  // reset color and font-size
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                      TRANS ("Are you sure you want to reset the font size,\n"
                                             "text color and background to the default?")))
        {

            systemFile->setValue ("fontSize", SwingUtilities::getFontSize());
            systemFile->setValue ("editorFontColour", Colour (0xff303030).toString());
            systemFile->setValue ("editorBackground", Colour (0xffdedede).toString());

            parent->getEditor()->setColour(TextEditor::textColourId, Colour(0xff303030));
            parent->getEditor()->setColour(CaretComponent::caretColourId, Colour(0xff303030).withAlpha(0.6f));
            parent->getEditor()->setColour (TextEditor::backgroundColourId, Colour (0xffdedede));
            parent->getEditor()->setFont (SwingUtilities::getFontSize());

            parent->getEditor ()->applyFontToAllText (SwingUtilities::getFontSize());
            systemFile->saveIfNeeded ();
        }
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

    DocTreeViewItem::needCreate (docTree);

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

//=================================================================================================
void EditorForMd::sliderValueChanged (Slider* slider)
{
    if (slider == &fontSizeSlider)
    {
        parent->getEditor ()->setFont ((float) slider->getValue ());
        parent->getEditor ()->applyFontToAllText ((float) slider->getValue ());
    }
}

//=================================================================================================
void EditorForMd::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == fontColourSelector)
    {
        Colour textClr(fontColourSelector->getCurrentColour());
        parent->getEditor()->setColour(TextEditor::textColourId, textClr);
        parent->getEditor()->setColour(CaretComponent::caretColourId, textClr.withAlpha(0.6f));
        parent->getEditor ()->applyFontToAllText (systemFile->getValue ("fontSize").getFloatValue ());
    }
    else if (source == bgColourSelector)
    {
        parent->getEditor ()->setColour (TextEditor::backgroundColourId, bgColourSelector->getCurrentColour());
    }
}

//=================================================================================================
void EditorForMd::searchBySelectPrev()
{
    const String& selected(getHighlightedText());

    if (selected.isNotEmpty())
    {
        const int startIndex = getText().substring(0, getCaretPosition() - 1).lastIndexOfIgnoreCase(selected);

        if (startIndex != -1)
            setHighlightedRegion(Range<int>(startIndex, startIndex + selected.length()));
        else
            LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    }
}

//=================================================================================================
void EditorForMd::searchBySelectNext()
{
    const String& selected(getHighlightedText());

    if (selected.isNotEmpty())
    {
        const int startIndex = getText().indexOfIgnoreCase(getCaretPosition() + selected.length(), selected);

        if (startIndex != -1)
            setHighlightedRegion(Range<int>(startIndex, startIndex + selected.length()));
        else
            LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    }
}

//=================================================================================================
WebBrowserComp::WebBrowserComp(EditAndPreview* parent_) 
    : WebBrowserComponent (false),
    parent(parent_)
{

}

//=================================================================================================
void WebBrowserComp::newWindowAttemptingToLoad(const String& newURL)
{
    WebBrowserComp* web = new WebBrowserComp(parent);
    web->setSize(1000, 600);
    web->goToURL(newURL);

    OptionalScopedPointer<Component> comp(web, true);
    DialogWindow::LaunchOptions option;

    option.dialogTitle = newURL;
    option.dialogBackgroundColour = Colours::black;
    option.content = comp;
    option.escapeKeyTriggersCloseButton = true;
    option.useNativeTitleBar = true;
    option.resizable = true;
    option.useBottomRightCornerResizer = false;

    option.launchAsync();
}

//=================================================================================================
bool WebBrowserComp::pageAboutToLoad(const String& newURL)
{
    if (newURL.substring(0, 3) == "res" ||
        newURL.substring(0, 4) == "http" ||
        newURL.substring(0, 3) == "ftp" ||
        newURL.substring(0, 5) == "email" ||
        newURL == "about:blank" ||
        newURL == DocTreeViewItem::getHtmlFileOrDir(parent->getCurrentTree()).getFullPathName())
    {
        return true;
    }
    else
    {
        const File& htmlFile(newURL);
        return !(parent->selectItemFromHtmlFile(htmlFile));
    }
}



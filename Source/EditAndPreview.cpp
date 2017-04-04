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
EditAndPreview::EditAndPreview (MainContentComponent* mainComp_) 
    : docHasChanged (false),
    mainComp (mainComp_),
    showSetupArea (true)
{
    // stretched layout, arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.74);  // editor，
    layoutManager.setItemLayout (1, 2, 2, 2);            // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.26);     // propertiesPanel

    addAndMakeVisible (mdEditor = new MarkdownEditor (this));
    addChildComponent (webView = new WebBrowserComp (this));

    addAndMakeVisible (setupPanel = new SetupPanel (this));
    addChildComponent (themeEditor = new ThemeEditor (this));
    addAndMakeVisible (layoutBar = new StrechableBar (&layoutManager, 1, true));

    themeEditor->setVisible (true);
    setupPanel->setVisible (false);

    // markdown editor
    mdEditor->setMultiLine (true);
    mdEditor->setReturnKeyStartsNewLine (true);
    mdEditor->setTabKeyUsedAsCharacter (true);

    Colour textClr = Colour::fromString (systemFile->getValue ("editorFontColour"));

    mdEditor->setColour (TextEditor::focusedOutlineColourId, Colour (0x000));
    mdEditor->setColour (TextEditor::textColourId, textClr);
    mdEditor->setColour (CaretComponent::caretColourId, textClr.withAlpha (0.6f));
    mdEditor->setColour (TextEditor::backgroundColourId, Colour::fromString (systemFile->getValue ("editorBackground")));
    mdEditor->setFont (systemFile->getValue ("fontSize").getFloatValue());

    mdEditor->setScrollBarThickness (10);
    mdEditor->setIndents (10, 10);
    mdEditor->setVisible (false);
    mdEditor->setBorder (BorderSize<int> (1, 1, 1, 1));
    mdEditor->setPopupMenuEnabled (false);
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
    stopTimer();
}

//=========================================================================
void EditAndPreview::resized()
{
    Component* workArea = (webView->isVisible() ? (Component*)(webView.get())
                           : (Component*)(mdEditor.get()));

    Component* setupArea = (setupPanel->isVisible() ? (Component*)(setupPanel.get())
                           : (Component*)(themeEditor.get()));

    jassert (workArea != nullptr && setupArea != nullptr);

    if (getParentComponent()->getWidth() > 1020 && showSetupArea)  // full-mode
    {
        setupArea->setVisible (true);
        layoutBar->setVisible (true);

        Component* comps[] = { workArea, layoutBar, setupArea };
        layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
    }
    else  // silent-mode (only makes the editor visable)
    {
        setupArea->setVisible (false);
        layoutBar->setVisible (false);
        workArea->setBounds (0, 0, getWidth(), getHeight());
    }
}

//=================================================================================================
void EditAndPreview::setLayout (const bool showSetupArea_, const bool showSetupPanel)
{
    showSetupArea = showSetupArea_;

    setupPanel->setVisible (showSetupPanel);
    themeEditor->setVisible (!showSetupPanel);

    resized();
}

//=================================================================================================
const bool EditAndPreview::setupAreaIsShowing() const
{
    return (setupPanel->isVisible() || themeEditor->isVisible());
}

//=================================================================================================
const File& EditAndPreview::getEditingThemeFile() const
{
    return themeEditor->getCurrentFile();
}

//=================================================================================================
void EditAndPreview::workAreaStartWork (ValueTree& newDocTree)
{
    jassert (newDocTree.isValid());
    saveCurrentDocIfChanged();

    if (newDocTree != docOrDirTree || docOrDirFile != DocTreeViewItem::getMdFileOrDir (newDocTree))
    {
        mdEditor->removeListener (this);
        docOrDirTree = newDocTree;
        docOrDirFile = DocTreeViewItem::getMdFileOrDir (docOrDirTree);

        if (docOrDirFile.existsAsFile())
        {
            mdEditor->setText (docOrDirFile.loadFileAsString(), false);
            currentContent = mdEditor->getText();
            mdEditor->addListener (this);
        }
    }

    // prevent auto-enter preview mode when created a new document
    switchMode (!(docOrDirFile.exists() && currentContent.length() < 3));

    if (currentContent.length() < 3)
        mdEditor->moveCaretToEnd (false);
    else
        mdEditor->moveCaretToTop (false);

    // word count doesn't include the ' ' and newLine of current content 
    setupPanel->updateWordCount (currentContent.removeCharacters (" ")
                                 .removeCharacters (newLine).length());
}

//=================================================================================================
void EditAndPreview::updateEditorContent()
{
    mdEditor->removeListener (this);
    docOrDirFile = DocTreeViewItem::getMdFileOrDir (docOrDirTree);

    if (docOrDirFile.existsAsFile())
    {
        mdEditor->setText (docOrDirFile.loadFileAsString(), false);
        currentContent = mdEditor->getText();
        mdEditor->addListener (this);

        // update web page
        switchMode (false);
        docOrDirTree.setProperty ("needCreateHtml", true, nullptr);
    }
}

//=================================================================================================
void EditAndPreview::editThemeFile (const File& themeFile)
{
    themeEditor->setFileToEdit (themeFile);
    setLayout (true, false);
}

//=================================================================================================
void EditAndPreview::switchMode (const bool switchToPreview)
{
    TopToolBar* toolBar = findParentComponentOfClass<MainContentComponent>()->getToolbar();
    jassert (toolBar != nullptr);

    if (!docOrDirFile.existsAsFile())
    {
        previewCurrentDoc();
        toolBar->enableEditPreviewBt (false, true);
    }
    else if ((docOrDirTree.getType().toString() != "doc") 
             || (switchToPreview && toolBar->getStateOfViewButton()))
    {
        previewCurrentDoc();
        toolBar->enableEditPreviewBt (!docOrDirFile.isDirectory(), true);
    }
    else
    {
        editCurrentDoc();
        toolBar->enableEditPreviewBt (true, false);
    }
}

//=================================================================================================
void EditAndPreview::editCurrentDoc()
{
    webView->setVisible (false);
    mdEditor->setVisible (true);
    mdEditor->grabKeyboardFocus();
    mdEditor->setPopupMenuEnabled (true);

    // here must goto the html url of the doc on osx, although the broswer doesn't visible.
    // otherwise, it'll load the previous page when switch to preview another doc,
    // especially after created a doc, edited then preview it.
    const String urlStr ((docOrDirFile.existsAsFile()) ?
                         HtmlProcessor::createArticleHtml (docOrDirTree, true).getFullPathName() :
                         HtmlProcessor::createIndexHtml (docOrDirTree, true).getFullPathName());
    
    if (urlStr != currentUrl)
    {
        webView->goToURL (urlStr);
        currentUrl = urlStr;
    }
    
    resized();
}

//=================================================================================================
void EditAndPreview::previewCurrentDoc()
{
    mdEditor->setVisible (false);
    webView->setVisible (true);
    webView->stop();

    if (docOrDirFile.exists())
    {
        const bool itNeedsCreate = (bool)docOrDirTree.getProperty ("needCreateHtml");

        const String urlStr ((docOrDirFile.existsAsFile()) ?
                             HtmlProcessor::createArticleHtml (docOrDirTree, true).getFullPathName() :
                             HtmlProcessor::createIndexHtml (docOrDirTree, true).getFullPathName());

        // prevent load it every time when preview a non-changed and the same web-page.
        // the browser's scrollbar will always rolled on top (of course its default behavior) 
        // after load a page every time. it's very annoying..
        if (urlStr != currentUrl)
        {
            webView->goToURL (urlStr);
            currentUrl = urlStr;
        }
        else
        {
            if (itNeedsCreate)
                webView->refresh();            
        }
    }
    else  // file doesn't exist
    {
        const String htmlStr ("<!doctype html>\n"
                              "<html lang=\"en\">\n"
                              "  <head>\n"
                              "    <meta charset=\"UTF-8\">\n"
                              "  </head>\n"
                              "  <body bgcolor=\"#cccccc\">\n"
                              "<p>\n &emsp;");

        File urlFile (File::getSpecialLocation (File::tempDirectory).getSiblingFile ("404.html"));

        if (docOrDirTree.getType().toString() == "doc")
            urlFile.replaceWithText (htmlStr + TRANS ("The file doesn't exist!") + "\n  </body>\n</html>");

        else
            urlFile.replaceWithText (htmlStr + TRANS ("The folder doesn't exist!") + "\n  </body>\n</html>");

        webView->goToURL (urlFile.getFullPathName());
    }

    resized();
}

//=================================================================================================
void EditAndPreview::outlineGoto (const StringArray& titleStrs, const int itemIndex)
{
    if (itemIndex == 1)
    {
        mdEditor->moveCaretToTop (false);
        webView->goToURL (currentUrl + "#top");
    }
    else if (itemIndex == titleStrs.size() - 1)
    {
        mdEditor->moveCaretToEnd (false);
        webView->goToURL (currentUrl + "#wdtpPageBottom");
    }
    else if (itemIndex > 0 && itemIndex < titleStrs.size() - 1)
    {
        // replace Chinese '#' temporaily instead of change it in reality
        const String content (currentContent.replace (CharPointer_UTF8 ("\xef\xbc\x83"), "#"));
        
        // here need position the caret twice and pageDown() after the first one.
        // this'll make sure the scroll position is on top of the editor-view
        // instead of at the bottom when place downward
        int positionIndex = content.indexOf (titleStrs[itemIndex]);
        mdEditor->setCaretPosition (positionIndex);
        mdEditor->pageDown (false);
        mdEditor->setCaretPosition (positionIndex);
        mdEditor->moveCaretToEndOfLine (true);

        const String& jumpTo (Md2Html::extractLinkText (titleStrs[itemIndex]
                                                        .fromFirstOccurrenceOf ("## ", false, false)));
        webView->goToURL (currentUrl + "#" + jumpTo);
    }

    mdEditor->grabKeyboardFocus();
}

//=================================================================================================
void EditAndPreview::projectClosed()
{
    saveCurrentDocIfChanged();
    webView->setVisible (false);
    setupPanel->projectClosed();

    mdEditor->removeListener (this);
    mdEditor->setText (String(), false);
    mdEditor->setVisible (false);
    mdEditor->setPopupMenuEnabled (false);

    docOrDirFile = File::nonexistent;
    docOrDirTree = ValueTree::invalid;
    docHasChanged = false;
    currentContent.clear();

    resized();
}

//=================================================================================================
void EditAndPreview::showProperties (const bool saveCurrentValues, const ValueTree& tree)
{
    if (tree.getType().toString() == "doc")
        setupPanel->showDocProperties (saveCurrentValues, tree);

    else if (tree.getType().toString() == "dir")
        setupPanel->showDirProperties (tree);

    else // root
        setupPanel->showProjectProperties (tree);
}

//=================================================================================================
const bool EditAndPreview::selectItemFromHtmlFile (const File& html)
{
    return mainComp->selectItemFromHtmlFile (html);
}

//=================================================================================================
void EditAndPreview::setSearchKeyword (const String& keyword)
{
    if (TopToolBar* toolBar = findParentComponentOfClass<MainContentComponent>()->getToolbar())
        toolBar->setSearchKeyword (keyword);
}

//=================================================================================================
void EditAndPreview::textEditorTextChanged (TextEditor&)
{
    // somehow, this method always be called when about to load a doc, 
    // so this ugly judge has to be here...
    if (currentContent.compare (mdEditor->getText()) != 0)
    {
        currentContent = mdEditor->getText();
        docHasChanged = true;
        DocTreeViewItem::needCreate (docOrDirTree);

        startTimer (3000);
    }
}

//=================================================================================================
void EditAndPreview::timerCallback()
{
    saveCurrentDocIfChanged();
}

//=================================================================================================
const bool EditAndPreview::saveCurrentDocIfChanged()
{
    stopTimer();
    bool returnValue = true;

    if (docHasChanged && docOrDirFile != File::nonexistent)
    {
        TemporaryFile tempFile (docOrDirFile);
        tempFile.getFile().appendText (currentContent);

        if (tempFile.overwriteTargetFileWithTemporary())
        {
            docHasChanged = false;
            setupPanel->showDocProperties (false, docOrDirTree);
            returnValue = FileTreeContainer::saveProject();

            // rebuild tips bank
            if (docOrDirFile.getFileName() == "tips.md" 
                && docOrDirFile.getParentDirectory().getFileName() == "docs")
            {
                TipsBank::getInstance()->rebuildTipsBank();
            }
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
WebBrowserComp::WebBrowserComp (EditAndPreview* parent_)
    : WebBrowserComponent (false),
    parent (parent_)
{

}

//=================================================================================================
void WebBrowserComp::newWindowAttemptingToLoad (const String& newURL)
{
    openUrlInNewWindow (newURL);
}

//=================================================================================================
void WebBrowserComp::openUrlInNewWindow (const String& newURL)
{
    WebBrowserComponent* web = new WebBrowserComponent();
    web->setSize (1000, 600);
    web->goToURL (newURL);

    OptionalScopedPointer<Component> comp (web, true);
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
bool WebBrowserComp::pageAboutToLoad (const String& newURL)
{
    String urlStr (newURL);

#if JUCE_WINDOWS
    if (urlStr.substring (0, 8) == "file:///")
        urlStr = urlStr.substring (8);
#endif
   
#if JUCE_MAC
    if (urlStr.substring (0, 7) == "file://")
        urlStr = urlStr.substring (7);
    
    urlStr = URL::removeEscapeChars (urlStr);
#endif
    
    String currentTreeUrl (DocTreeViewItem::getHtmlFile (parent->getCurrentTree()).getFullPathName());

    //DBGX (urlStr);
    //DBGX (currentTreeUrl);
    //DBGX (urlStr.upToFirstOccurrenceOf ("#", false, true));

    if (urlStr.substring (0, 3) == "res" ||
        urlStr.substring (0, 4) == "http" ||
        urlStr.substring (0, 3) == "ftp" ||
        urlStr.getLastCharacters (4) == "#top" ||
        urlStr.getLastCharacters (8) == "404.html" ||
        urlStr.substring (0, 5) == "email" ||
        urlStr == "about:blank" ||
        urlStr == currentTreeUrl ||
        urlStr.upToFirstOccurrenceOf ("#", false, true) == currentTreeUrl.replace ("\\", "/") ||
        (!urlStr.contains ("http") && urlStr.contains ("index-"))
        )
    {
        return true;
    }    

    else
    {
        // doesn't load it instead of select the matched item
        return !(parent->selectItemFromHtmlFile (File (urlStr)));
    }
}



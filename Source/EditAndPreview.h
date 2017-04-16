/*
  ==============================================================================

    EditAndPreview.h
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef EDITANDPREVIEW_H_INCLUDED
#define EDITANDPREVIEW_H_INCLUDED

class SetupPanel;
class MarkdownEditor;
class WebBrowserComp;
class ThemeEditor;

//==============================================================================
/** For edit a doc or preview the selected item's html and setup its properties.
*/
class EditAndPreview : public Component,
                       private TextEditor::Listener,
                       private Timer
{
public:
    EditAndPreview (MainContentComponent* mainComp);
    ~EditAndPreview();

    void workAreaStartWork (ValueTree& newDocTree);
    void updateEditorContent();
    void editThemeFile (const File& themeFile, bool needRegenerate);

    /** false for switch to edit mode. but note: 
        true isn't make sure switch to preview mode 
        (it depends on the state of toolbar's 'big eye' button) */
    void switchMode (const bool switchToPreview);
    void forcePreview();

    void refreshCurrentPage()                   { webView->refresh(); }
    void restartWebBrowser();
    void setMdEditorReadOnly (const bool onlyForRead);

    /** true for stretch theme editor, false for normal */
    void stretchThemeEditor (const bool stretchIt);

    //=================================================================================
    void paint (Graphics&) override {}
    void resized() override;

    /** if arg 1 is true and arg 2 'showSetupPanel' is false, it'll show theme editor */
    void displaySetupArea (const bool showSetupArea, 
                           const bool showSetupPanel);

    const bool setupAreaIsShowing() const;
    const bool themeEditorIsShowing() const;
    const File& getEditingThemeFile() const;

    TextEditor* getMdEditor() const             { return mdEditor; }
    WebBrowserComponent* getWebBrowser() const  { return webView; }
    const String& getCurrentUrl() const         { return currentUrl; }

    const File& getCurrentDocFile() const       { return docOrDirFile; }
    const String& getCurrentContent() const     { return currentContent; }

    ValueTree& getCurrentTree()                 { return docOrDirTree; }
    SetupPanel* getSetupPanel() const           { return setupPanel; }

    /** return true if preview state at the present, flase for edit state. */
    const bool getCureentState() const          { return webView->isVisible(); }

    /** see DocTreeViewItem::itemClicked() left-click */
    void outlineGoto (const StringArray& titleStrs, const int itemIndex);

    void projectClosed();
    const bool saveCurrentDocIfChanged();
    void showProperties (const bool saveCurrentValues, const ValueTree& tree);
    
    /** a transfer method */
    const bool selectItemFromHtmlFile (const File& htmlFile);

    void setSearchKeyword (const String& keyword);

private:
    //=========================================================================
    void editCurrentDoc();
    void previewCurrentDoc();

    virtual void textEditorTextChanged (TextEditor&) override;
    virtual void timerCallback() override;

    //=========================================================================
    File docOrDirFile;
    ValueTree docOrDirTree;
    bool docHasChanged;
    String currentContent, currentUrl;

    MainContentComponent* mainComp;

    ScopedPointer<TextEditor> mdEditor;
    ScopedPointer<WebBrowserComponent> webView;
    ScopedPointer<SetupPanel> setupPanel;
    ScopedPointer<ThemeEditor> themeEditor;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StrechableBar> layoutBar;
    bool showSetupArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditAndPreview)
};

//=================================================================================================
/** A WebBrowserComponent, it could load an url which with the tag "target=_blank"
    in a new modal window. By default, juce's webBrowser can't do this.

    and it'll intercept the inter-link, get it's matched item and select it in the file-tree panel.

    Usage: same as JUCE's WebBrowserComponent */
class WebBrowserComp : public WebBrowserComponent
{
public:
    WebBrowserComp (EditAndPreview* parent_);
    ~WebBrowserComp() { }

    /** new dialog window to display the URL */
    virtual void newWindowAttemptingToLoad (const String& newURL) override;

    void openUrlInNewWindow (const String& newURL);

    /** intercept the inter-link and select the matched item in file tree */
    virtual bool pageAboutToLoad (const String& newURL) override;

private:
    EditAndPreview* parent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComp)
};

#endif  // EDITANDPREVIEW_H_INCLUDED

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

//==============================================================================
/** For edit a doc or preview the selected item's html and setup its properties.
*/
class EditAndPreview : public Component,
                       private TextEditor::Listener,
                       private Timer
{
public:
    EditAndPreview (MainContentComponent* mainComp);
    ~EditAndPreview ();

    void startWork (ValueTree& newDocTree);

    /** false for switch to edit mode. true is preview mode */
    void switchMode (const bool switchToPreview);

    void paint (Graphics&) override {}
    void resized () override;

    TextEditor* getEditor () const          { return editor; }
    const File& getCurrentDocFile () const  { return docOrDirFile; }
    ValueTree& getCurrentTree ()            { return docOrDirTree; }
    SetupPanel* getSetupPanel () const      { return setupPanel; }

    /** return true if preview state at the present, flase for edit state. */
    const bool getCureentState () const;

    void projectClosed ();
    const bool saveCurrentDocIfChanged ();

    void setProjectProperties (ValueTree& projectTree);
    void setDirProperties (ValueTree& dirTree);
    void setDocProperties (ValueTree& docTree);

    /** a transfer method */
    const bool selectItemFromHtmlFile (const File& htmlFile);

private:
    //=========================================================================
    void editCurrentDoc ();
    void previewCurrentDoc ();

    virtual void textEditorTextChanged (TextEditor&) override;
    virtual void timerCallback () override;

    //=========================================================================
    File docOrDirFile = File::nonexistent;
    ValueTree docOrDirTree;
    bool docHasChanged = false;
    String currentContent, currentUrl;

    MainContentComponent* mainComp;

    ScopedPointer<TextEditor> editor;
    ScopedPointer<WebBrowserComp> webView;
    ScopedPointer<SetupPanel> setupPanel;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StrechableBar> layoutBar;

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
    ~WebBrowserComp () { }

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

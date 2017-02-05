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
class EditorForMd;
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
    String currentContent;

    MainContentComponent* mainComp;

    ScopedPointer<TextEditor> editor;
    ScopedPointer<WebBrowserComp> webView;
    ScopedPointer<SetupPanel> setupPanel;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StrechableBar> layoutBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditAndPreview)
};

//=========================================================================
class EditorForMd : public TextEditor,
                    public Slider::Listener,
                    public ChangeListener
{
public:
    EditorForMd (EditAndPreview* parent_);
    ~EditorForMd () { }
    void paint (Graphics& g) override;

    virtual void addPopupMenuItems (PopupMenu& menuToAddTo, 
                                    const MouseEvent* mouseClickEvent) override;
    virtual void performPopupMenuAction (int menuItemID) override;

    /** for TAB keypress input 4 spaces.
        somehow the original TextEditor can't do it whilst the caret doesn't move at all. */
    bool keyPressed (const KeyPress& key) override;

    /** for set the font-size and color of font and backgroud */
    virtual void sliderValueChanged (Slider* slider) override;
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

private:
    void searchBySelectPrev ();
    void searchBySelectNext ();

    EditAndPreview* parent;
    Slider fontSizeSlider;
    ScopedPointer<ColourSelectorWithPreset> fontColourSelector;
    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorForMd)
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

    /** intercept the inter-link and select the matched item in file tree */
    virtual bool pageAboutToLoad (const String& newURL) override;

private:
    EditAndPreview* parent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComp)
};

#endif  // EDITANDPREVIEW_H_INCLUDED

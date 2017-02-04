/*
  ==============================================================================

    TopToolBar.h
    Created: 4 Sep 2016 12:25:18am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef TOPTOOLBAR_H_INCLUDED
#define TOPTOOLBAR_H_INCLUDED

class FileTreeContainer;

//==============================================================================
/** The app's toolbar which places in the top of main interface. */
class TopToolBar :  public Component,
                    private TextEditor::Listener,
                    private Button::Listener,
                    public ChangeListener,
                    public ApplicationCommandTarget
{
public:
    TopToolBar (FileTreeContainer* container, 
                EditAndPreview* editAndPreview);

    ~TopToolBar () { }

    void paint (Graphics&) override;
    void resized () override;

    const bool getStateOfViewButton () const { return bts[view]->getToggleState (); }
    void enableEditPreviewBt (const bool enableIt, const bool toggleState);

    /** for change the background-color and the UI text color in realtime */
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

    /** these four methods use for application command-target */
    virtual ApplicationCommandTarget* getNextCommandTarget () override;
    virtual void getAllCommands (Array<CommandID>& commands) override;
    virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    virtual bool perform (const InvocationInfo& info) override;

private:
    //==========================================================================
    virtual void textEditorReturnKeyPressed (TextEditor&) override;
    virtual void textEditorEscapeKeyPressed (TextEditor&) override;

    void findInProject (const bool next);
    void findInDoc (const bool next);

    virtual void buttonClicked (Button*) override;
    void popupSystemMenu ();
    void menuPerform (const int menuIndex);

    void createNewProject ();
    void openProject ();
    void closeProject ();
    void cleanAndGenerateAll ();
    void cleanLocalMedias ();

    /** generate the tree and all its children's html file */
    static void generateHtmlFiles (ValueTree tree);
    void generateHtmlsIfNeeded ();
    static void generateHtmlFilesIfNeeded (ValueTree tree);
    void generateCurrentPage ();

    void setUiColour ();
    void resetUiColour ();

    /** the pack file's extension must be ".wpck" */
    static void packProject ();  
    static void exportCurrentTpls ();
    void importExternalTpls ();
    static void releaseSystemTpls (const File& projectFile, const bool askAndShowMessage);

    enum LanguageID { English = 0, Chinese = 1 };
    void setUiLanguage (const LanguageID& id);
    void setEmptyTextOfSearchBox ();

    //==========================================================================
    enum BtIndex 
    {
        prevAll = 0, nextAll, prevPjt, nextPjt,
        view, system, width, totalBts
    };

    enum MenuAndCmdIndex 
    {
        newPjt = 1, openPjt, closePjt, packPjt, unpackPjt,
        generateCurrent, generateNeeded, generateWhole, cleanUpLocal,
        exportTpl, importTpl, releaseSystemTpl, uiEnglish, uiChinese,
        setUiColor, resetUiColor, gettingStarted, checkNewVersion, showAbout,
        switchEdit, switchWidth
    };

    //=====================================================================================
    /** When mouse enter the button, the cursor will change to a finger */
    class MyImageButton : public ImageButton
    {
    public:
        MyImageButton ()
        {
            setMouseCursor (MouseCursor::PointingHandCursor);
        }

        virtual void enablementChanged () override
        {
            setMouseCursor (isEnabled () ? MouseCursor::PointingHandCursor 
                                         : MouseCursor::NormalCursor);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyImageButton)
    };

    //=========================================================================
    OwnedArray<MyImageButton> bts;
    ScopedPointer<TextEditor> searchInProject;
    ScopedPointer<TextEditor> searchInDoc;

    FileTreeContainer* fileTreeContainer;
    EditAndPreview* editAndPreview;

    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;
    String languageStr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopToolBar)
};


#endif  // TOPTOOLBAR_H_INCLUDED

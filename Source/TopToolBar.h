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
                    public ApplicationCommandTarget,
                    private Thread
{
public:
    TopToolBar (FileTreeContainer* container, 
                EditAndPreview* editAndPreview);

    ~TopToolBar();

    void paint (Graphics&) override;
    void resized() override;

    const bool getStateOfViewButton() const { return bts[viewBt]->getToggleState(); }
    void enableEditPreviewBt (const bool enableIt, const bool toggleState);

    /** for change the background-color and the UI text color in realtime */
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

    /** these four methods use for application command-target */
    virtual ApplicationCommandTarget* getNextCommandTarget() override;
    virtual void getAllCommands (Array<CommandID>& commands) override;
    virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    virtual bool perform (const InvocationInfo& info) override;

    static void generateHtmlFilesIfNeeded (ValueTree tree);

    void setSearchKeyword (const String& kw);
    void hasNewVersion();

private:
    //==========================================================================
    /** for progressBar when generate all */
    virtual void run() override; 

    virtual void textEditorReturnKeyPressed (TextEditor&) override;
    virtual void textEditorEscapeKeyPressed (TextEditor&) override;

    void keywordSearch (const bool next);
    virtual void buttonClicked (Button*) override;
    void popupSystemMenu();
    void systemMenuPerform (const int menuIndex);

    void popupLayoutMenu();
    void switchSilentMode (const bool enterSilent);

    void createNewProject();
    void openProject();
    void closeProject();
    void cleanAndGenerateAll();
    void cleanNeedlessMedias (const bool showMessageWhenNoAnyNeedless);

    /** generate the tree and all its children's html file */
    static void generateHtmlFiles (ValueTree tree);
    static double progressValue;
    static int totalItems;
    static int accumulator;

    static void generateHtmlsIfNeeded();
    void generateCurrentPage();

    void setUiColour();
    void setTooltips();
    void resetUiColour();
    void setupAudioDevice();

    /** the pack file's extension must be ".wpck" */
    static void packProject();  
    static void exportCurrentTpls();    
    static void releaseSystemTpls (const bool askAndShowMessage);
    void importExternalTpls();

    /** imgType: 0 for site ico, 1 for project logo */
    void setSiteImgs (const int imgType);

    enum LanguageID { English = 0, Chinese = 1 };
    void setUiLanguage (const LanguageID& id);

    void createThemeFilesMenu (PopupMenu& menu, const int baseId);

    //==========================================================================
    enum BtIndex 
    {
        searchPrev = 0, searchNext, 
        viewBt, systemBt, layoutBt, 
        totalBts
    };

public:
    enum MenuAndCmdIndex 
    {
        newPjt = 1, openPjt, closePjt,  
        packPjt, unpackPjt,
        generateCurrent, generateNeeded, generateWhole, cleanUpLocal,
        importIco, importLogo, exportTpl, importTpl, releaseSystemTpl, 
        uiEnglish, uiChinese, setupAudio,
        setUiColor, resetUiColor, checkNewVersion, 
        wdtpUpdateList, gettingStarted, syntax, faq, feedback, 
        showAboutDialog,
        switchEdit, switchWidth, activeSearch
    };

private:

    //=====================================================================================
    /** When mouse enter the button, the cursor will change to a finger */
    class MyImageButton : public ImageButton
    {
    public:
        MyImageButton()
        {
            setMouseCursor (MouseCursor::PointingHandCursor);
        }

        virtual void enablementChanged() override
        {
            setMouseCursor (isEnabled() ? MouseCursor::PointingHandCursor 
                                         : MouseCursor::NormalCursor);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyImageButton)
    };

    //=========================================================================
    OwnedArray<MyImageButton> bts;
    ScopedPointer<TextEditor> searchInput;

    FileTreeContainer* fileTreeContainer;
    EditAndPreview* editAndPreview;

    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;
    String languageStr;
    ProgressBar progressBar;
    bool newVersionIsReady;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopToolBar)
};


#endif  // TOPTOOLBAR_H_INCLUDED

/*
  ==============================================================================

    MarkdownEditor.h
    Created: 8 Feb 2017 2:46:52am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef MARKDOWNEDITOR_H_INCLUDED
#define MARKDOWNEDITOR_H_INCLUDED

/** A text editor especially for Markdown input, display and edit. 
    The powerful functions totally in its right-click popup-menu and 
    some shortcuts - see its keyPressed(). 
*/
class MarkdownEditor :  public TextEditor,
                        public Slider::Listener,
                        public ChangeListener,
                        public FileDragAndDropTarget,
                        public Timer,
                        private ActionListener
{
public:
    MarkdownEditor (EditAndPreview* parent_);
    ~MarkdownEditor() { }

    void paint (Graphics& g) override;

    //=================================================================================================
    static void popupOutlineMenu (EditAndPreview* editAndPreview, const String& editorContent);

    virtual void addPopupMenuItems (PopupMenu& menuToAddTo,
                                    const MouseEvent* mouseClickEvent) override;

    virtual void performPopupMenuAction (int menuItemID) override;        
    bool keyPressed (const KeyPress& key) override;

    void insertImages (const Array<File>& imageFiles);

    /** for set the font-size and color of font and backgroud */
    virtual void sliderValueChanged (Slider* slider) override;
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

    /** drag native images to copy and insert their mark */
    virtual bool isInterestedInFileDrag (const StringArray& files) override;
    virtual void filesDropped (const StringArray& files, int x, int y) override;
    
    /** for Chinese punc-matching */
    virtual void timerCallback() override;

private:
    //=============================================================================================
    enum MenuIndex
    {
        pickTitle = 1, addKeywords, pickFromAllKeywords, pickDesc,
        insertImage, insertAudio, insertHyperlink, insertQuota,
        insertNormalTable, insertInterlaced, insertNoborderTable,
        insertAlignCenter, insertAlignRight, 
        insertUnoerderList, insertOrderList,
        insertFirstTitle, insertSecondTitle, insertThirdTitle, 
        insertToc, insertEndnote,
        insertIdentifier,
        insertCaption, insertSeparator, insertAuthor, insertInterLink,
        formatBold, formatItalic, formatBoldAndItalic, formatHighlight,        
        codeBlock, hybridLayout, commentBlock, inlineCode, antiIndent, forceIndent,
        audioRecord,
        latestPublish, latestModify, featuredArticle, 
        allKeywords, randomArticle, allPublish, allModify,
        searchNext, searchPrev,
        fontSize, fontColor, setBackground, resetDefault,
        outlineMenu
    };

    /** for click to select/unselect a keyword from keywords-table component.
        see showAllKeywords().    */
    virtual void actionListenerCallback (const String& message) override;
    void showAllKeywords();

    /** tableStyle: should be the menu index see above */
    void tableInsert (const int tableStyle);
    void hyperlinkInsert();
    void insertImages();
    void insertAudioFiles();

    /** expandIndex: should be the menu index see above */
    void insertExpandMark (const int expandIndex);
    void quotaInsert();
    void insertIndent (const bool isIndent);  // false for anti-indent
    void insertTitle (const int level);
    void endnoteInsert();
    void tocInsert();
    void identifierInsert();
    void addSelectedToKeywords (const String& selectedStr);
    void subtractFromKeywords (const String& keyword);
    void alignCenterInsert();
    void alignRightInsert();
    void orderListInsert();
    void unorderListInsert();
    void captionInsert();
    void interLinkInsert();
    void authorInsert();
    void codeBlockFormat();
    void hybridFormat();
    void commentBlockFormat();

    void autoWrapSelected (const KeyPress& key);
    void tabKeyInput();
    void shiftTabInput();
    void returnKeyInput();
    void pasteForCtrlV();
    void recordAudio();

    void searchPrevious();
    void searchForNext();

    void setFontSize();
    void setFontColour();
    void setBackgroundColour();
    void resetToDefault();

    /** formatIndex: should be the menu index see above */
    void inlineFormat (const int formatIndex);

    const bool puncMatchingForChinese (const KeyPress& key);

    //=============================================================================================
    EditAndPreview* parent;
    Slider fontSizeSlider;
    ScopedPointer<ColourSelectorWithPreset> fontColourSelector;
    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;
    String selectedForPunc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkdownEditor)
};


#endif  // MARKDOWNEDITOR_H_INCLUDED

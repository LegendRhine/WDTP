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
                        public MultiTimer,
                        private ActionListener
{
public:
    MarkdownEditor (EditAndPreview* parent_);
    ~MarkdownEditor()  { }

    void paint (Graphics& g) override;

    //=================================================================================================
    static void popupOutlineMenu (EditAndPreview* editAndPreview, 
                                  const String& editorContent,
                                  bool showMessageWhenNoOutline);

    virtual void addPopupMenuItems (PopupMenu& menuToAddTo,
                                    const MouseEvent* mouseClickEvent) override;

    virtual void performPopupMenuAction (int menuItemID) override;
    bool keyPressed (const KeyPress& key) override;
    void insertExternalFiles (const Array<File>& mediaFiles);

    /** for drag and drop the selected text */
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;

    /** for set the font-size and color of font and backgroud */
    virtual void sliderValueChanged (Slider* slider) override;
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

    /** drag native images to copy and insert their mark */
    virtual bool isInterestedInFileDrag (const StringArray& files) override;
    virtual void filesDropped (const StringArray& files, int x, int y) override;

    /** auto-wrap, punctuation matching, auto-complete, show tips...*/
    virtual void insertTextAtCaret (const String& textToInsert) override;    
    void autoComplete (const int index);

    /** for Chinese punc-matching and show real-time tips */
    virtual void timerCallback (int timerID) override;
    enum TimerId { chinesePunc = 1, showTipsBank };

private:
    //=============================================================================================
    enum MenuIndex
    {
        pickTitle = 1, addKeywords, pickFromAllKeywords, pickDesc,
        insertMedia, insertHyperlink, insertQuota, timeLine,
        insertNormalTable, insertInterlaced, insertNoborderTable,
        insertAlignCenter, insertAlignRight, 
        insertUnoerderList, insertOrderList,
        insertFirstTitle, insertSecondTitle, insertThirdTitle, 
        insertToc, insertEndnote, insertBackToTop, formatPostil,
        insertIdentifier, insertTime,
        insertCaption, insertSeparator, insertAuthor, insertInterLink,
        formatBold, formatItalic, formatBoldAndItalic, formatHighlight,        
        codeBlock, hybridLayout, commentBlock, inlineCode, antiIndent, 
        forceIndent, audioRecord,
        latestPublish, latestModify, featuredArticle, 
        allKeywords, randomArticle, allPublish, allModify,
        searchNext, searchPrev, 
        searchByGoogle, searchByBing, searchByWiki, baiduBaike,
        transByGoogle, transByBaidu, showTips, joinTips,
        fontSize, fontColor, setBackground, resetDefault,
        outlineMenu, setExEditorForMedia, editMediaByExEditor,
        convertToJpg, halfWidth, threeQuarterWidth, transparentImg, 
        rotateImgLeft, rotateImgRight, syntax
    };

    /** for click to select/unselect a keyword from keywords-table component.
        see showAllKeywords().    */
    virtual void actionListenerCallback (const String& message) override;
    void showAllKeywords();

    /** if the selected text is a file name without its extension, 
        it will return its 'full' name (doesn't include its path, 
        only the file's name with its extension) */
    const String getSelectedFileName() const;

    /** return: 0 for image file, 1 for mp3 file, -1 for unknown type */
    const int getSelectedMediaType() const;

    /** tableStyle: should be the menu index see above */
    void tableInsert (const int tableStyle);
    void hyperlinkInsert();
    void insertExternalFiles();
    void insertTimeLine();

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
    void dateAndTimeInsert();
    void codeBlockFormat();
    void hybridFormat();
    void commentBlockFormat();

    void tabKeyInput();
    void shiftTabInput();
    void returnKeyInput();
    void pasteForCtrlV();
    void recordAudio();

    void searchPrevious();
    void searchForNext();
    void externalSearch (const int searchType);

    void setFontSize();
    void setFontColour();
    void setBackgroundColour();
    void resetToDefault();

    /** formatIndex: should be the menu index see above */
    void inlineFormat (const int formatIndex);
    void selectedAddToTipsBank();

    //=============================================================================================
    EditAndPreview* parent;
    Slider fontSizeSlider;
    ScopedPointer<ColourSelectorWithPreset> fontColourSelector;
    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;

    StringArray menuItems; // for popup tips
    String selectedForCnPunc;
    DrawableRectangle draggingPosition;

    int posBeforeInputNewText = 0;
    bool delPressed = false, draggingSelected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkdownEditor)
};


#endif  // MARKDOWNEDITOR_H_INCLUDED

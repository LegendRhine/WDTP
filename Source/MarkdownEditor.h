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
                        public ChangeListener
{
public:
    MarkdownEditor (EditAndPreview* parent_);
    ~MarkdownEditor () { }

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
    //=============================================================================================
    enum MenuIndex
    {
        pickTitle = 1, addKeywords, pickDesc,
        insertImage, insertHyperlink, insertTable, insertQuota,
        insertAlignCenter, insertAlignRight, insertUnoerderList, insertOrderList,
        insertFirstTitle, insertSecondTitle, insertThirdTitle,
        insertCaption, insertSeparator, insertAuthor, insertInterLink,
        formatBold, formatItalic, codeBlock, inlineCode,
        searchNext, searchPrev,
        fontSize, fontColor, setBackground, resetDefault
    };

    void hyperlinkInsert ();
    void insertImages ();
    void tableInsert ();
    void quotaInsert ();
    void insertTitle (const int level);
    void pickAsDescription ();
    void pickSelectedAsTitle ();
    void addSelectedToKeywords ();
    void alignCenterInsert ();
    void alignRightInsert ();
    void orderListInsert ();
    void unorderListInsert ();
    void captionInsert ();
    void separatorInsert ();
    void interLinkInsert ();
    void authorInsert ();
    void codeBlockFormat ();

    void searchBySelectPrev ();
    void searchBySelectNext ();

    void setFontSize ();
    void setFontColour ();
    void setBackgroundColour ();
    void resetToDefault ();

    /** 0 for bold, 1 for italic, 2 for inline code */
    enum inlineFormatIndex { bold = 0, italic, codeOfinline };
    void inlineFormat (const inlineFormatIndex& format);

    void saveAndUpdate ();

    //=============================================================================================
    EditAndPreview* parent;
    Slider fontSizeSlider;
    ScopedPointer<ColourSelectorWithPreset> fontColourSelector;
    ScopedPointer<ColourSelectorWithPreset> bgColourSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MarkdownEditor)
};


#endif  // MARKDOWNEDITOR_H_INCLUDED

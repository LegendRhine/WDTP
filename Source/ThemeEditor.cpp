/*
  ==============================================================================

    ThemeEditor.cpp
    Created: 3 Apr 2017 10:57:50am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern ApplicationCommandManager* cmdManager;

//==============================================================================
/** search comp for theme editor */
class SearchComp : public Component,
                   private TextEditor::Listener,
                   private Button::Listener
{
public:
    SearchComp (ThemeEditor* te, const String& searchFor) : editor (te) 
    {
        addAndMakeVisible (lb);
        lb.setFont (Font (17.00f, Font::plain));
        lb.setJustificationType (Justification::centred);
        lb.setEditable (false, false, false);
        lb.setText (TRANS ("Search Content"), dontSendNotification);

        addAndMakeVisible (searchInput);
        searchInput.addListener (this);
        searchInput.setText ((searchFor.isNotEmpty() ? searchFor 
                             : SystemClipboard::getTextFromClipboard())
                             .removeCharacters ("\n")
                             .removeCharacters ("\r"), false);

        searchInput.setColour (TextEditor::textColourId, Colour (0xff303030));
        searchInput.setColour (TextEditor::focusedOutlineColourId, Colours::lightskyblue);
        searchInput.setColour (TextEditor::backgroundColourId, Colour (0xffededed).withAlpha (0.6f));
        searchInput.setFont (SwingUtilities::getFontSize() - 3.f);
        searchInput.setSelectAllWhenFocused (true);
  
        addAndMakeVisible (nextBt);
        nextBt.addListener (this);
        nextBt.setButtonText (TRANS ("Find Next"));

        addAndMakeVisible (prevBt);
        prevBt.addListener (this);
        prevBt.setButtonText (TRANS ("Find Previous"));

        setSize (320, 130);
    }

    ~SearchComp() { }

    //=================================================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xffacabab));
    }

    //=================================================================================================
    void resized() override
    {
        lb.setBounds (10, 5, getWidth() - 20, 30);
        searchInput.setBounds (15, 40, 290, 25);
        prevBt.setBounds (60, 90, 80, 25);
        nextBt.setBounds (180, 90, 80, 25);
    }

private:
    //=================================================================================================
    virtual void textEditorReturnKeyPressed (TextEditor& te) override
    {
        if (&te == &searchInput)
            keywordSearch (true);
    }

    //=================================================================================================
    virtual void textEditorEscapeKeyPressed (TextEditor& te) override
    {
        if (&te == &searchInput)
            searchInput.setText (String(), false);
    }

    //=================================================================================================
    virtual void buttonClicked (Button* bt) override
    {
        if (bt == &nextBt)
            keywordSearch (true);

        else if (bt == &prevBt)
            keywordSearch (false);
    }

    //=================================================================================================
    void keywordSearch (const bool next)
    {
        const String& keyword (searchInput.getText());

        if (keyword.isNotEmpty() && editor->getText().isNotEmpty())
        {
            const int currentPos = editor->getCaretPosition();

            const int startIndex = next 
                ? editor->getText().indexOfIgnoreCase (currentPos + 1, keyword)
                : editor->getText().substring (0, currentPos - 1).lastIndexOfIgnoreCase (keyword);

            if (startIndex != -1)
                editor->setHighlightedRegion (Range<int> (startIndex, startIndex + keyword.length()));

            else
                SHOW_MESSAGE (TRANS ("Nothing could be found."));
        }
    }

    //=================================================================================================
    ThemeEditor* editor;
    TextEditor searchInput;

    Label lb;
    TextButton nextBt;
    TextButton prevBt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SearchComp)
};

//==============================================================================
ThemeEditor::ThemeEditor (EditAndPreview* parent) :
    editAndPreview (parent)
{
    setFont (SwingUtilities::getFontSize() - 1.f);    
    setMultiLine (true, true);
    setReturnKeyStartsNewLine (true);
    setTabKeyUsedAsCharacter (true);

    setColour (TextEditor::focusedOutlineColourId, Colour (0x000));
    setColour (TextEditor::textColourId, Colour (0xffd7d079));
    setColour (CaretComponent::caretColourId, Colour (0xffd7d079).withAlpha (0.6f));
    setColour (TextEditor::backgroundColourId, Colour (0xff202020));
    setColour (TextEditor::highlightColourId, Colours::lightskyblue);

    initializeTags();
}

//=================================================================================================
ThemeEditor::~ThemeEditor()
{
}

//=================================================================================================
void ThemeEditor::setFileToEdit (const File& file)
{
    currentFile = file;
    jassert (currentFile.existsAsFile());
    setText (currentFile.loadFileAsString(), false);
}

//=================================================================================================
void ThemeEditor::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    const bool fileExists = currentFile.existsAsFile();

    const int selectStart = getHighlightedRegion().getStart();
    const int selectEnd = getHighlightedRegion().getEnd();
    const String& beforeStart (getTextInRange (Range<int> (selectStart - 1, selectStart)));
    const String& afterEnd (getTextInRange (Range<int> (selectEnd, selectEnd + 1)));

    if (e->mods.isPopupMenu() && fileExists)
    {
        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator();

        String ctrlStr ("  (Ctrl + ");
#if JUCE_MAC
        strlStr = "  (cmd + ";
#endif

        menu.addItem (searchSth, TRANS ("Search Content") + ctrlStr + "F)");
        menu.addItem (selectClr, TRANS ("Set Color") + "...",
                      (beforeStart == "#" && afterEnd == ";" &&
                       (getHighlightedText().length() == 3 || getHighlightedText().length() == 6)));

        menu.addSubMenu (TRANS ("Insert Template Tags"), tagsMenu, currentFile.getFileExtension() == ".html");
        menu.addSeparator();

        menu.addItem (applyIndex, TRANS ("Save and Apply") + ctrlStr + "S)");
        menu.addItem (closeIndex, TRANS ("Close without Save"));
        menu.addItem (saveAsIndex, TRANS ("Overwrite and Save to") + "...");

        /*menu.addSeparator();
        menu.addItem (autoReturn, TRANS ("Line Wrap"), true, isWordWrap());*/
    }
}

//=================================================================================================
void ThemeEditor::performPopupMenuAction (int index)
{
    if (applyIndex == index)
    {
        currentFile.replaceWithText (getText());
        editAndPreview->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        cmdManager->invokeDirectly (TopToolBar::MenuAndCmdIndex::generateCurrent, false);
        editAndPreview->forcePreview();
    }

    else if (closeIndex == index)
    {
        exitEditMode();
    }
    
    else if (saveAsIndex == index)
    {
        FileChooser fc (TRANS ("Save As") + "...", File(), "*", true);

        if (fc.browseForDirectory())
        {
            const File& file (fc.getResult().getChildFile (currentFile.getFileName()));

            file.deleteFile();
            file.create();
            currentFile.copyFileTo (file);
        }
    }

    else if (searchSth == index)
    {
        ScopedPointer<SearchComp> searchComp = new SearchComp (this, getHighlightedText());

        CallOutBox callOut (*searchComp, getScreenBounds(), nullptr);
        callOut.runModalLoop();        
    }

    else if (selectClr == index)
    {        
        String selectedStr (getHighlightedText());
        SystemClipboard::copyTextToClipboard (selectedStr);

        if (selectedStr.length() == 3)
            selectedStr += selectedStr;

        Colour currentClr ((uint8)selectedStr.substring (0, 2).getHexValue32(),
                           (uint8)selectedStr.substring (2, 4).getHexValue32(),
                           (uint8)selectedStr.substring (4, 6).getHexValue32());

        clrSelector = new ColourSelectorWithPreset (ColourSelector::showColourAtTop
                                                    | ColourSelector::showSliders
                                                    | ColourSelector::showColourspace);
        clrSelector->setSize (450, 480);
        clrSelector->setCurrentColour (currentClr);
        clrSelector->addChangeListener (this);

        CallOutBox callOut (*clrSelector, getScreenBounds(), nullptr);
        callOut.runModalLoop();
    }

    else if (index >= 100 && index < 300)  // insert template tag
    {
        const String& tagToInsert (tags.getAllKeys()[index - 100]);

        if (getText().indexOf (0, tagToInsert) != -1)
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                          TRANS ("This tag is already here, insert it in any case?")))
                insertTextAtCaret (tagToInsert + newLine);
        }
        else
        {
            insertTextAtCaret (tagToInsert + newLine);
        }
    }

    else
    {
        TextEditor::performPopupMenuAction (index);
    }
}

//=================================================================================================
void ThemeEditor::exitEditMode()
{
    setText (String(), false);
    currentFile = File();

    MainContentComponent* main = (MainContentComponent*)(editAndPreview->getParentComponent());
    jassert (main != nullptr);
    main->displayFileTree (true);

    editAndPreview->displaySetupArea (true, true);
    editAndPreview->stretchThemeEditor (false);
}

//=================================================================================================
bool ThemeEditor::keyPressed (const KeyPress& key)
{
    // tab
    if (key == KeyPress (KeyPress::tabKey))
    {
        insertTextAtCaret ("    ");
        return true;
    }

    // ctrl + f: search
    else if (key == KeyPress ('f', ModifierKeys::commandModifier, 0))
    {
        performPopupMenuAction (searchSth);
        return true;
    }

    // ctrl + s: save and update the page
    else if (key == KeyPress ('s', ModifierKeys::commandModifier, 0))
    {
        performPopupMenuAction (applyIndex);
        return true;
    }

    // English punctuation matching...
    else if (key == KeyPress ('\'', ModifierKeys::shiftModifier, 0)) // "..."
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\"" + selectedStr + "\"");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('\'')) // '...'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("\'" + selectedStr + "\'");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }
    
    else if (key == KeyPress ('[', ModifierKeys::shiftModifier, 0)) // '{}'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("{" + selectedStr + "}");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress ('9', ModifierKeys::shiftModifier, 0)) // '()'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("(" + selectedStr + ")");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    else if (key == KeyPress (',', ModifierKeys::shiftModifier, 0)) // '<>'
    {
        const String& selectedStr (getHighlightedText());
        insertTextAtCaret ("<" + selectedStr + ">");

        if (selectedStr.isEmpty())
            moveCaretLeft (false, false);

        return true;
    }

    return TextEditor::keyPressed (key);
}

//=================================================================================================
void ThemeEditor::changeListenerCallback (ChangeBroadcaster* source)
{
    if (clrSelector == source)
    {
        insertTextAtCaret (clrSelector->getCurrentColour().toString().substring (2));
        setHighlightedRegion (Range<int> (getCaretPosition() - 6, getCaretPosition()));
    }
}

//=================================================================================================
void ThemeEditor::initializeTags()
{
    // template tags in <body>
    tags.set ("{{siteRelativeRootPath}}", TRANS ("Root Path"));
    tags.set ("{{siteLogo}}", TRANS ("Project Logo"));
    tags.set ("{{siteMenu}}", TRANS ("Site Menu"));
    tags.set ("{{siteNavi}}", TRANS ("Site Navigation"));
    tags.set ("{{siteLink}}", TRANS ("Home Text Link"));

    tags.set ("{{contentTitle}}", TRANS ("Article Title"));
    tags.set ("{{contentDesc}}", TRANS ("Article Description"));
    tags.set ("{{content}}", TRANS ("Article Content"));
    tags.set ("{{createAndModifyTime}}", TRANS ("Create and Modify Time"));
    tags.set ("{{previousAndNext}}", TRANS ("Previous and Next Article"));
    tags.set ("{{random}}", TRANS ("Five Random Articles"));
    tags.set ("{{ad}}", TRANS ("Advertisement Images"));
    tags.set ("{{contact}}", TRANS ("Contact Info"));

    tags.set ("{{toTop}}", TRANS ("Goto Top"));
    tags.set ("{{backPrevious}}", TRANS ("Back to Upper Level"));
    tags.set ("{{bottomCopyright}}", TRANS ("Copyright Info"));

    tags.set ("{{titleOfDir}}", TRANS ("Folder Title"));
    tags.set ("{{blogList}}", TRANS ("Blog List"));
    tags.set ("{{bookList}}", TRANS ("Catalogs List"));

    for (int i = 0; i < tags.size(); ++i)
    {
        tagsMenu.addItem (100 + i, tags.getAllValues()[i]);

        if (4 == i || 12 == i || 15 == i)
            tagsMenu.addSeparator();
    }
}


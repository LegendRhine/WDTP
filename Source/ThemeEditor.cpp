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
    void paint (Graphics& g)
    {
        g.fillAll (Colour (0xffacabab));
    }

    //=================================================================================================
    void resized()
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
    setFont (SwingUtilities::getFontSize());    
    setMultiLine (true, true);
    setReturnKeyStartsNewLine (true);
    setTabKeyUsedAsCharacter (true);

    setColour (TextEditor::focusedOutlineColourId, Colour (0x000));
    setColour (TextEditor::textColourId, Colour (0xffd7d079));
    setColour (CaretComponent::caretColourId, Colour (0xffd7d079).withAlpha (0.6f));
    setColour (TextEditor::backgroundColourId, Colour (0xff202020));
    setColour (TextEditor::highlightColourId, Colours::lightskyblue);
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

    if (e->mods.isPopupMenu() && fileExists)
    {
        TextEditor::addPopupMenuItems (menu, e);
        menu.addSeparator();

        menu.addItem (searchSth, TRANS ("Search Content"));
        menu.addSeparator();

        menu.addItem (applyIndex, TRANS ("Save and Apply"));
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

        CallOutBox callOut (*searchComp, this->getScreenBounds(), nullptr);
        callOut.runModalLoop();        
    }

    /*else if (autoReturn == index)
    {
        setMultiLine (true, !isWordWrap());
    }*/
}

//=================================================================================================
void ThemeEditor::exitEditMode()
{
    setText (String(), false);
    currentFile = File();
    editAndPreview->setLayout (true, true);
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



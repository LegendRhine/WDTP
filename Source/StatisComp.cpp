/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.3.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "WdtpHeader.h"
//[/Headers]

#include "StatisComp.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
static String statisKeyword = "";
//[/MiscUserDefs]

//==============================================================================
StatisComp::StatisComp (FileTreeContainer* fileTree, DocTreeViewItem* item, const bool isDoc_, const String& statisStr)
    : treeContainer (fileTree), dirItem (item), isDoc (isDoc_)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (titleLabel = new Label (String(),
                                               TRANS("Statistics Info")));
    titleLabel->setFont (Font (18.00f, Font::bold));
    titleLabel->setJustificationType (Justification::centred);
    titleLabel->setEditable (false, false, false);
    titleLabel->setColour (TextEditor::textColourId, Colours::black);
    titleLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (infoEditor = new TextEditor (String()));
    infoEditor->setMultiLine (true);
    infoEditor->setReturnKeyStartsNewLine (true);
    infoEditor->setReadOnly (true);
    infoEditor->setScrollbarsShown (false);
    infoEditor->setCaretVisible (false);
    infoEditor->setPopupMenuEnabled (true);
    infoEditor->setColour (TextEditor::backgroundColourId, Colour (0x00ffffff));
    infoEditor->setText (String());

    addAndMakeVisible (keywordLabel = new Label ("new label",
                                                 TRANS("Feature Word: ")));
    keywordLabel->setFont (Font (17.00f, Font::plain));
    keywordLabel->setJustificationType (Justification::centred);
    keywordLabel->setEditable (false, false, false);
    keywordLabel->setColour (TextEditor::textColourId, Colours::black);
    keywordLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (keywordEditor = new TextEditor ("new text editor"));
    keywordEditor->setExplicitFocusOrder (1);
    keywordEditor->setMultiLine (false);
    keywordEditor->setReturnKeyStartsNewLine (false);
    keywordEditor->setReadOnly (false);
    keywordEditor->setScrollbarsShown (false);
    keywordEditor->setCaretVisible (true);
    keywordEditor->setPopupMenuEnabled (true);
    keywordEditor->setText (String());

    addAndMakeVisible (analyseEditor = new TextEditor ("new text editor"));
    analyseEditor->setMultiLine (false);
    analyseEditor->setReturnKeyStartsNewLine (false);
    analyseEditor->setReadOnly (true);
    analyseEditor->setScrollbarsShown (false);
    analyseEditor->setCaretVisible (false);
    analyseEditor->setPopupMenuEnabled (true);
    analyseEditor->setColour (TextEditor::backgroundColourId, Colour (0x00ffffff));
    analyseEditor->setText (String());

    addAndMakeVisible (analyseBt = new TextButton (String()));
    analyseBt->setExplicitFocusOrder (2);
    analyseBt->setButtonText (TRANS("Analyse"));
    analyseBt->addListener (this);


    //[UserPreSize]

    infoEditor->setFont (17.f);
    keywordEditor->setFont (17.f);
    analyseEditor->setFont (17.f);

    infoEditor->setText (statisStr);
    keywordEditor->setText (statisKeyword);
    keywordEditor->setSelectAllWhenFocused (true);
    keywordEditor->addListener (this);

    if (statisKeyword.isEmpty())
        keywordEditor->setText (SystemClipboard::getTextFromClipboard().removeCharacters ("\n")
                                .removeCharacters ("\r"), false);

    //[/UserPreSize]

    setSize (385, 210);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

StatisComp::~StatisComp()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    titleLabel = nullptr;
    infoEditor = nullptr;
    keywordLabel = nullptr;
    keywordEditor = nullptr;
    analyseEditor = nullptr;
    analyseBt = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void StatisComp::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffdcdbdb));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void StatisComp::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    titleLabel->setBounds (120, 5, 150, 24);
    infoEditor->setBounds (30, 32, getWidth() - 60, 70);
    keywordLabel->setBounds (5, 110, 110, 25);
    keywordEditor->setBounds (120, 110, getWidth() - 150, 26);
    analyseEditor->setBounds (15, 144, getWidth() - 30, 26);
    analyseBt->setBounds (140, 178, 100, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void StatisComp::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == analyseBt)
    {
        //[UserButtonCode_analyseBt] -- add your button handler code here..

        statisKeyword = keywordEditor->getText();

        if (statisKeyword.isEmpty())
            return;

        isDoc ? analyseDoc() : analyseDir();

        //[/UserButtonCode_analyseBt]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void StatisComp::analyseDoc()
{
    treeContainer->getEditAndPreview()->switchMode (false);
    MarkdownEditor* editor = (MarkdownEditor*)treeContainer->getEditAndPreview()->getEditor();
    const String& content = editor->getText();

    int startIndex = 0;
    editor->setCaretPosition (0);
    int caretIndex = 0;

    // find the start index of the keyword
    startIndex = content.indexOfIgnoreCase (caretIndex, statisKeyword);
    Array<Range<int>> rangeArray;
    int numbers = 0;

    while (startIndex != -1)
    {
        ++numbers;
        rangeArray.add (Range<int> (startIndex, startIndex + statisKeyword.length()));

        editor->setCaretPosition (startIndex + statisKeyword.length());
        startIndex = content.indexOfIgnoreCase (editor->getCaretPosition(), statisKeyword);
    }

    editor->setTemporaryUnderlining (rangeArray);

    if (numbers == 0)
        analyseEditor->setText (TRANS ("Nothing could be found. "));
    else
        analyseEditor->setText (TRANS ("This keyword appears in this doc sum total ")
                                + String (numbers) + TRANS (" time(s)."), false);
}

//=================================================================================================
void StatisComp::analyseDir()
{
    dirItem->setOpen (true);
    int files = 0;
    int totalNumbers = 0;

    analyseDir (dirItem, files, totalNumbers);

    if (files == 0)
    {
        analyseEditor->setText (TRANS ("Nothing could be found. "));
    }
    else
    {
        showAnalyseResult (files, totalNumbers);
        dirItem->setSelected (false, false);
    }
}

//=================================================================================================
void StatisComp::analyseDir (DocTreeViewItem* currentItem, int& files, int& totalNumbers)
{
    for (int i = currentItem->getNumSubItems(); --i >= 0; )
    {
        DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (currentItem->getSubItem (i));

        if (item == nullptr)
            continue;

        const File& docFile (DocTreeViewItem::getMdFileOrDir (item->getTree()));

        if (docFile.existsAsFile())
        {
            const String& docContent (docFile.loadFileAsString());

            if (docContent.containsIgnoreCase (statisKeyword))
            {
                ++files;
                item->setSelected (true, false);
                treeContainer->getTreeView().scrollToKeepItemVisible (item);

                int startIndex = docContent.indexOfIgnoreCase (0, statisKeyword);

                while (startIndex != -1)
                {
                    ++totalNumbers;
                    startIndex = docContent.indexOfIgnoreCase (startIndex + statisKeyword.length(), statisKeyword);
                }
            }
        }
        else if (docFile.isDirectory())
        {
            analyseDir (item, files, totalNumbers);
        }
    }
}

//=================================================================================================
void StatisComp::showAnalyseResult(const int docNum, const int totalNum)
{
    analyseEditor->setText (TRANS ("This keyword appears in ") + String (docNum) + TRANS (" doc(s). ")
                            + TRANS ("Sum total ") + String (totalNum) + TRANS (" time(s)."), false);
}

//=================================================================================================
void StatisComp::textEditorReturnKeyPressed (TextEditor& te)
{
    if (keywordEditor == &te && keywordEditor->getText().isNotEmpty())
        analyseBt->triggerClick();
}
//=================================================================================================
void StatisComp::textEditorEscapeKeyPressed (TextEditor& te)
{
    if (keywordEditor == &te)
        keywordEditor->setText (String());
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="StatisComp" componentName=""
                 parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="FileTreeContainer* fileTree, DocTreeViewItem* item, const bool isDoc_, const String&amp; statisStr"
                 variableInitialisers="treeContainer (fileTree), dirItem (item), isDoc (isDoc_)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="385" initialHeight="210">
  <BACKGROUND backgroundColour="ffdcdbdb"/>
  <LABEL name="" id="3e0696e99e0e768d" memberName="titleLabel" virtualName=""
         explicitFocusOrder="0" pos="120 5 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Statistics Info" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="18" bold="1" italic="0" justification="36"/>
  <TEXTEDITOR name="" id="2be33f20c3c2a579" memberName="infoEditor" virtualName=""
              explicitFocusOrder="0" pos="30 32 60M 70" bkgcol="ffffff" initialText=""
              multiline="1" retKeyStartsLine="1" readonly="1" scrollbars="0"
              caret="0" popupmenu="1"/>
  <LABEL name="new label" id="518978ad17a9f68b" memberName="keywordLabel"
         virtualName="" explicitFocusOrder="0" pos="5 110 110 25" edTextCol="ff000000"
         edBkgCol="0" labelText="Feature Word: " editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="17" bold="0" italic="0" justification="36"/>
  <TEXTEDITOR name="new text editor" id="b495c65e0a391d5e" memberName="keywordEditor"
              virtualName="" explicitFocusOrder="1" pos="120 110 150M 26" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="new text editor" id="90b4a9a5ac61a342" memberName="analyseEditor"
              virtualName="" explicitFocusOrder="0" pos="15 144 30M 26" bkgcol="ffffff"
              initialText="" multiline="0" retKeyStartsLine="0" readonly="1"
              scrollbars="0" caret="0" popupmenu="1"/>
  <TEXTBUTTON name="" id="5b948568fee478a0" memberName="analyseBt" virtualName=""
              explicitFocusOrder="2" pos="140 178 100 24" buttonText="Analyse"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

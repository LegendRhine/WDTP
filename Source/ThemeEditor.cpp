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
ThemeEditor::ThemeEditor (EditAndPreview* parent) :
    editAndPreview (parent)
{
    // buttons
    for (int i = totalBts; --i >= 0; )
    {
        TextButton* bt = new TextButton();
        addAndMakeVisible (bt);
        bt->addListener (this);
        bt->setSize (60, 25);
        bts.add (bt);
    }

    bts[applyBt]->setButtonText (TRANS ("Apply"));
    bts[applyBt]->setTooltip (TRANS ("Save and Apply the Change"));

    bts[closeBt]->setButtonText (TRANS ("Close"));
    bts[closeBt]->setTooltip (TRANS ("Close Theme Editor Without Saving"));
    
    bts[saveAsBt]->setButtonText (TRANS ("Save As"));
    bts[saveAsBt]->setTooltip (TRANS ("Save to Another File"));

    // editor
    colorToken = new XmlTokeniser();

    addAndMakeVisible (editor = new CodeEditorComponent (codeDoc, colorToken));
    editor->setFont (SwingUtilities::getFontSize() - 3.f);
    editor->setLineNumbersShown (true);
    editor->setScrollbarThickness (10);
}

//=================================================================================================
ThemeEditor::~ThemeEditor()
{
}

//=================================================================================================
void ThemeEditor::paint (Graphics& g)
{
    g.setColour (Colours::grey);
    g.drawLine (1.0f, getHeight() - 35.f, getWidth() - 2.0f, 35.f, 0.6f);
}

//=================================================================================================
void ThemeEditor::resized()
{
    editor->setBounds (2, 2, getWidth() - 4, getHeight() - 37);

    for (int i = totalBts; --i >= 0; )
        bts[i]->setTopLeftPosition (i * 75 + (getWidth() - totalBts * 85), getHeight() - 30);
}

//=================================================================================================
void ThemeEditor::setFileToEdit (const File& file)
{
    currentFile = file;
    jassert (currentFile.existsAsFile());

    editor->loadContent (currentFile.loadFileAsString());
}

//=================================================================================================
void ThemeEditor::buttonClicked (Button* bt)
{
    if (bt == bts[applyBt])
    {
        currentFile.replaceWithText (codeDoc.getAllContent());
        editAndPreview->getCurrentTree().setProperty ("needCreateHtml", true, nullptr);
        cmdManager->invokeDirectly (TopToolBar::MenuAndCmdIndex::generateCurrent, false);
    }
    else if (bt == bts[closeBt])
    {
        editor->loadContent (String());
        currentFile == File();
        editAndPreview->setLayout (true, true);
    }
    else if (bt == bts[saveAsBt])
    {
        FileChooser fc (TRANS ("Save As") + "...", File::nonexistent, currentFile.getFileName(), true);

        if (fc.browseForFileToSave (true))
        {
            File file (fc.getResult());

            file.deleteFile();
            file.create();
            file.appendText (editor->getDocument().getAllContent());
        }
    }
}


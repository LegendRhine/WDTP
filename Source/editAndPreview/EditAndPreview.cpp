/*
  ==============================================================================

    EditAndPreview.cpp
    Created: 4 Sep 2016 12:28:00am
    Author:  SwingCoder

  ==============================================================================
*/

#include "../WdtpHeader.h"

//==============================================================================
EditAndPreview::EditAndPreview()
{
    // web browser
    addChildComponent (webView = new WebBrowserComponent ());

    // stretched layout, arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, -0.5, -1.0, -0.69);  // editor，
    layoutManager.setItemLayout (1, 3, 3, 3);           // layoutBar
    layoutManager.setItemLayout (2, 2, -0.5, -0.31);  // propertiesPanel

    addAndMakeVisible (editor = new EditorForMd (docFile));
    addAndMakeVisible (setupPanel = new SetupPanel ());
    addAndMakeVisible (layoutBar = new StretchableLayoutResizerBar (&layoutManager, 1, true));

    // editor
    editor->setMultiLine (true);
    editor->setReturnKeyStartsNewLine (true);
    editor->setColour (TextEditor::textColourId, Colour(0xff303030));
    editor->setColour (TextEditor::focusedOutlineColourId, Colour(0xffb4b4b4));
    editor->setColour (TextEditor::backgroundColourId, Colour(0xffededed));
    editor->setScrollBarThickness (10);
    editor->setIndents(6, 6);
    editor->setFont (SwingUtilities::getFontSize());
}

//=========================================================================
EditAndPreview::~EditAndPreview()
{
}

//=========================================================================
void EditAndPreview::resized()
{
    // layout
    Component* comps[] = { (webView->isVisible ()) ? 
        static_cast<Component*>(webView) : static_cast<Component*>(editor), 
        layoutBar, setupPanel };

    layoutManager.layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
}

//=================================================================================================
void EditAndPreview::editDoc (const File& file)
{
    webView->setVisible (false);
    docFile = file;
    editor->setText (docFile.loadFileAsString ());
    resized ();
}

//=================================================================================================
void EditAndPreview::previewDoc (const File& file)
{
    webView->setVisible (true);
    docFile = file;
    //webView->goToURL (docFile.getFullPathName ());
    webView->goToURL ("e:/temp/test.html");
    resized ();
}

//=================================================================================================
void EditAndPreview::projectClosed ()
{
    editor->setText (String());
    setupPanel->projectClosed ();
    webView->setVisible (false);

    // TODO: ...
}

//=================================================================================================
void EditAndPreview::setSystemProperties()
{
    if (!setupPanel->systemPropertiesIsShowing())
        setupPanel->showSystemProperties();
}

//=================================================================================================
void EditAndPreview::setProjectProperties (ValueTree& projectTree)
{
    setupPanel->showProjectProperties (projectTree);
}

//=================================================================================================
void EditAndPreview::setDirProperties (ValueTree& dirTree)
{
    setupPanel->showDirProperties (dirTree);
}

//=================================================================================================
void EditAndPreview::setDocProperties (ValueTree& docTree)
{
    setupPanel->showDocProperties (docTree);
}

//=================================================================================================
void EditAndPreview::whenFileOrDirNonexists ()
{
    setupPanel->showSystemProperties();
}

//=================================================================================================
void EditorForMd::addPopupMenuItems (PopupMenu& menu, const MouseEvent* e)
{
    if (e->mods.isPopupMenu())
    {
        PopupMenu insertMenu;
        insertMenu.addItem (1, TRANS ("Image"));
        insertMenu.addItem (2, TRANS ("Table") + " (2x2)");
        insertMenu.addItem (3, TRANS ("Table") + " (3x3)");
        insertMenu.addItem (4, TRANS ("Hyperlink"));

        menu.addSubMenu (TRANS ("Insert"), insertMenu, docFile.existsAsFile());


        menu.addSeparator();

        TextEditor::addPopupMenuItems (menu, e);
    }
}

//=================================================================================================
void EditorForMd::performPopupMenuAction (int index)
{
    TextEditor::performPopupMenuAction (index);
    String content;

    if (1 == index) // image
    {
        FileChooser fc (TRANS ("Select Images..."), File::nonexistent, "*.jpg;*.png;*.gif", false);
        Array<File> imageFiles;

        if (fc.browseForMultipleFilesToOpen())
            imageFiles = fc.getResults ();

        const File imgPath (docFile.getSiblingFile ("media"));

        for (auto f : imageFiles)
        {
            const File targetFile (imgPath.getChildFile (f.getFileName()).getNonexistentSibling (true));
            targetFile.create();

            if (f.copyFileTo (targetFile))
                content << newLine << "![](media/" << targetFile.getFileName () << ")" << newLine;
            else
                SHOW_MESSAGE (TRANS ("Can't insert this image: ") + newLine + f.getFullPathName());
        }
    }
    else if (2 == index) // table 2*2
    {
        content << newLine
            << "|  |  |" << newLine
            << "| --: | :-- |" << newLine
            << "|  |  |" << newLine
            << "|  |  |" << newLine;
    }
    else if (3 == index) // table 3*3
    {
        content << newLine
            << "|  |  |  |" << newLine
            << "| --: | :--: | :-- |" << newLine
            << "|  |  |  |" << newLine
            << "|  |  |  |" << newLine
            << "|  |  |  |" << newLine;        
    }
    else if (4 ==index) // hyperlink
    {
        AlertWindow dialog (TRANS ("Insert Hyperlink"), TRANS ("Please input the url."),
                            AlertWindow::InfoIcon);

        dialog.addTextEditor ("name", "http://");
        dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
        dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

        if (0 == dialog.runModalLoop ())
        {
            const String inputStr (dialog.getTextEditor ("name")->getText().trim());
            content << " [](" << inputStr << ") ";
        }
    }

    insertTextAtCaret (content);
}

/*
  ==============================================================================

    MainComponent.cpp
    Created: 4 Sep 2016 11:08:17am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

//==============================================================================
MainContentComponent::MainContentComponent() 
{
    // must be these order...
    addAndMakeVisible (editAndPreview = new EditAndPreview ());
    addAndMakeVisible (fileTree = new FileTreeContainer (editAndPreview));
    addAndMakeVisible (toolBar = new TopToolBar (fileTree, editAndPreview));
    addAndMakeVisible (layoutBar = new StrechableBar (&layoutManager, 1, true));

    // here must disenable the preview button of toolbar
    // to prevent the jassert when the app doesn't load any project
    // the arg 2 will deside the editor's mode (preview or edit) after the app running with a project
    toolBar->enableEditPreviewBt (false, true);

    // stretched layout. arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, 2, -0.3, -0.21);  // fileTree
    layoutManager.setItemLayout (1, 2, 2, 2);            // layoutBar
    layoutManager.setItemLayout (2, -0.7, -1.0, -0.79);   // editAndPreview

    setSize (1200, 780);
}

//=======================================================================
MainContentComponent::~MainContentComponent()
{

}
//=========================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll(Colour::fromString(systemFile->getValue("uiBackground")));
}

//=========================================================================
void MainContentComponent::resized()
{
    toolBar->setBounds (0, 0, getWidth (), 45);
    
    if (getWidth() > 760)  // stretched layout
    {
        fileTree->setVisible(true);
        layoutBar->setVisible(true);
        
        Component* comps[] = { fileTree, layoutBar, editAndPreview };
        layoutManager.layOutComponents (comps, 3, 0, 45, getWidth (), getHeight () - 45, false, true);
    }
    else  // simple-mode (only makes the editor visable)
    {
        fileTree->setVisible(false);
        layoutBar->setVisible(false);
        editAndPreview->setBounds (0, 45, getWidth (), getHeight () - 45);
    }    
}

//=================================================================================================
MainWindow::MainWindow (String name) : 
    DocumentWindow (name, Colours::lightgrey, DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentOwned (mainComp = new MainContentComponent (), true);
    setResizable (true, false);
    setResizeLimits (640, 480, 3200, 2400);

    centreWithSize (getWidth (), getHeight ());
    setVisible (true);
}

//=================================================================================================
void MainWindow::closeButtonPressed ()
{
    if (mainComp->getFileTree()->saveDocAndProject())
    {
        JUCEApplication::getInstance()->systemRequestedQuit();        
    }
    else
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                          TRANS ("Something wrong during saving this project.") +
                                          newLine + newLine +
                                          TRANS ("Do you really want to quit?")))
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    }
}

//=================================================================================================
void MainWindow::openProject (const File& projectFile)
{
    mainComp->getFileTree()->openProject (projectFile);
}

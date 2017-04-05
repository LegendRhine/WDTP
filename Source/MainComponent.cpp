/*
  ==============================================================================

    MainComponent.cpp
    Created: 4 Sep 2016 11:08:17am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;
extern ApplicationCommandManager* cmdManager;
File lameEncoder;

//==============================================================================
MainContentComponent::MainContentComponent() :
    Thread ("CheckNewVersion"),
    showFileTreePanel (true)
{
    // must be these order...
    addAndMakeVisible (editAndPreview = new EditAndPreview (this));
    addAndMakeVisible (fileTree = new FileTreeContainer (editAndPreview));
    addAndMakeVisible (toolBar = new TopToolBar (fileTree, editAndPreview));
    addAndMakeVisible (layoutBar = new StrechableBar (&layoutManager, 1, true));

    /* here must disable the preview button of the toolbar
       to prevent the jassert when the app doesn't load any project
       the arg 2 will decide the editor's mode (preview or edit) after
       the app running with a project has loaded */
    toolBar->enableEditPreviewBt (false, true);

    // stretched layout. arg: index, min-width, max-width，default x%
    layoutManager.setItemLayout (0, 2, -0.3, -0.20);        // fileTree
    layoutManager.setItemLayout (1, 2, 2, 2);               // layoutBar
    layoutManager.setItemLayout (2, -0.7, -1.0, -0.80);     // editAndPreview

    setSize (1260, 780);

    // check new version and download mp3-encoder if it's not there
    startTimer (1000);
}

//=======================================================================
MainContentComponent::~MainContentComponent()
{
    stopTimer();

    if (isThreadRunning())
        stopThread (3000);
}
//=========================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour::fromString (systemFile->getValue ("uiBackground")));
}

//=========================================================================
void MainContentComponent::resized()
{
    toolBar->setBounds (0, 0, getWidth(), 45);

    if (getWidth() > 760 && showFileTreePanel)  // stretched layout
    {
        fileTree->setVisible (true);
        layoutBar->setVisible (true);

        Component* comps[] = { fileTree, layoutBar, editAndPreview };
        layoutManager.layOutComponents (comps, 3, 0, 45, getWidth(), getHeight() - 45, false, true);
    }
    else  // silent-mode (only makes the editor visable)
    {
        fileTree->setVisible (false);
        layoutBar->setVisible (false);
        editAndPreview->setBounds (0, 45, getWidth(), getHeight() - 45);
    }
}

//=================================================================================================
void MainContentComponent::setLayout (const bool showFileTree)
{
    showFileTreePanel = showFileTree;
    resized();
}

//=================================================================================================
const bool MainContentComponent::aDocSelectedCurrently() const
{
    return fileTree->aDocSelectedCurrently();
}

//=================================================================================================
void MainContentComponent::reloadCurrentDoc()
{
    fileTree->reloadCurrentDoc();
}

//=================================================================================================
const bool MainContentComponent::selectItemFromHtmlFile (const File& html)
{
    return fileTree->selectItemFromHtmlFile (html);
}

//=================================================================================================
void MainContentComponent::timerCallback()
{
    stopTimer();

    showStartTip();
    startThread();  // must using background thread on Android when connect internet
}

//=================================================================================================
void MainContentComponent::run()
{
    // check new version
    URL url ("https://raw.githubusercontent.com/LegendRhine/WDTP/master/Source/version");
    const String& urlContent = url.readEntireTextStream().trim();

    if (urlContent.isEmpty()
        || urlContent.length() > 10
        || urlContent.length() < 8  // 1.0.0101 ~ 99.99.1231
        || urlContent.containsAnyOf ("abcdefghijklmnopqrstuvwxyz"
                                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "<>/\\+-*!@#$%^&*()_=-`~[]{}"))
    {
        return;
    }

    const String& currentVersion ("1" + String (ProjectInfo::versionString).replace (".", String()));
    const String& urlVersion ("1" + urlContent.replace (".", String()));
    //DBGX (currentVersion << " - " << urlVersion);

    if (currentVersion.getIntValue() < urlVersion.getIntValue())
    {
        const MessageManagerLock mmLock;
        toolBar->hasNewVersion();
    }

    // download mp3 encoder if it's not there
#if JUCE_WINDOWS
    lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame.exe");
    url = URL::createWithoutParsing ("https://github.com/LegendRhine/gitBackup/raw/master/applications/lame-win.zip");
#elif JUCE_MAC
    lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame.app");
    url = URL::createWithoutParsing ("https://github.com/LegendRhine/gitBackup/raw/master/applications/lame-osx.zip");
#endif

    if (!lameEncoder.existsAsFile())
    {
        MemoryBlock mb;
        
        // put it in userDocumentsDirectory
        if (url.readEntireBinaryStream (mb))
        {
#if JUCE_WINDOWS
            MemoryInputStream inputSteam (mb, false);
            ZipFile zip (inputSteam);
            zip.uncompressEntry (0, File::getSpecialLocation (File::userDocumentsDirectory));
#elif JUCE_MAC
            //lameEncoder.create();

            const File lameZip (lameEncoder.getSiblingFile("lame-osx.zip"));
            lameZip.deleteFile();
            lameZip.create();
            
            if (!lameZip.replaceWithData (mb.getData(), mb.getSize()))
            {
                lameZip.deleteFile();
            }
            else
            {
                AlertWindow::showMessageBox (AlertWindow::InfoIcon, TRANS ("Congratulations"),
                                             TRANS ("The MP3 encoder 'lame-osx.zip' has been downloaded.")
                                             + newLine
                                             + TRANS ("Please double click the zip file to release it."));
                lameZip.revealToUser();
            }
#endif
        }
    }
}

//=================================================================================================
void MainContentComponent::showStartTip()
{
    if (!fileTree->hasLoadedProject())
    {
        Label info (String(), TRANS ("Click here and 'New Project...'\n\nStart your journey of WDTP!"));
        info.setColour (Label::textColourId, Colours::greenyellow.withAlpha (0.8f));
        info.setFont (SwingUtilities::getFontSize() - 2.f);
        info.setJustificationType (Justification::centred);
        info.setSize (240, 70);

        CallOutBox callOut (info, toolBar->getScreenBounds(), nullptr);
        callOut.runModalLoop();
    }
}

//=================================================================================================
MainWindow::MainWindow (const String& name) :
    DocumentWindow (name, Colours::lightgrey, DocumentWindow::allButtons)
{
    setContentOwned (mainComp = new MainContentComponent(), true);
    
    // command manager
    cmdManager->registerAllCommandsForTarget (mainComp->getToolbar());
    addKeyListener (cmdManager->getKeyMappings());
    cmdManager->setFirstCommandTarget (mainComp->getToolbar());

    setResizable (true, false);
    setResizeLimits (640, 480, 3200, 2400);
    setUsingNativeTitleBar (true);

    setFullScreen (true);
    setVisible (true);

    mainComp->grabKeyboardFocus();
}

//=================================================================================================
MainWindow::~MainWindow()
{

}

//=================================================================================================
void MainWindow::closeButtonPressed()
{
    // save and exit
    if (mainComp->getFileTree()->saveOpenSateAndSelect (true) 
        && mainComp->getEditAndPreview()->saveCurrentDocIfChanged())
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

//=================================================================================================
void MainWindow::activeWindowStatusChanged()
{
    if (isActiveWindow())
        mainComp->reloadCurrentDoc();
}


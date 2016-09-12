/*
  ==============================================================================

    Main.cpp
    Created: 4 Sep 2016 11:08:17am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

// global object
/** managed in mainApplication class */
PropertiesFile* systemFile = nullptr;

//==============================================================================
class WDTPApplication  : public JUCEApplication
{
public:
    //==============================================================================
    WDTPApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }
    
    //==============================================================================
    void initialise (const String& /*commandLine*/) override
    {
        // initial system properties file
        const File& f (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("wdtp.sys"));
        PropertiesFile::Options options;
        options.storageFormat = PropertiesFile::storeAsXML; 
        systemFile = new PropertiesFile (f, options);

        // first run this application
        if (!f.existsAsFile())
        {
            systemFile->setValue ("language", "English");
            systemFile->setValue ("clickForEdit", "Edit");
            systemFile->setValue ("fontSize", SwingUtilities::getFontSize());
            systemFile->setValue ("exEditor", String ());

            systemFile->save ();
        }

        // initial application's GUI
        LookAndFeel::setDefaultLookAndFeel (lnf = new SwingLookAndFeel());
        mainWindow = new MainWindow (getApplicationName());

        // open the previous project
        RecentlyOpenedFilesList  recentFiles;
        recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
        const File& projectFile (recentFiles.getFile (0));

        if (projectFile.existsAsFile())
            mainWindow->openProject (projectFile);
    }
    //=========================================================================
    void shutdown() override
    {
        deleteAndZero (systemFile);
        mainWindow = nullptr;
    }

    //=========================================================================
    void systemRequestedQuit () override
    {
        quit ();
    }

    //=========================================================================
    void anotherInstanceStarted (const String& commandLine) override
    {
        mainWindow->openProject (File (commandLine));
    }
        
private:
    ScopedPointer<SwingLookAndFeel> lnf;
    ScopedPointer<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (WDTPApplication)

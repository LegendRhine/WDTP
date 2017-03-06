/*
  ==============================================================================

    Main.cpp
    Created: 4 Sep 2016 11:08:17am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

// global objects which be managed in mainApplication class 
PropertiesFile* systemFile              = nullptr;
ApplicationCommandManager* cmdManager   = nullptr;

AudioFormatManager* formatManager = nullptr;
AudioDeviceManager* deviceManager = nullptr;
File lameEncoder;

//==============================================================================
class WDTPApplication : public JUCEApplication
{
public:
    //==============================================================================
    WDTPApplication() {}

    const String getApplicationName() override      { return ProjectInfo::projectName; }
    const String getApplicationVersion() override   { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override      { return true; }
    void systemRequestedQuit() override             { quit(); }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // for WebBroswerComponent's web-core on Windows (IE7-IE11)
        // otherwise, the embedded broswer cannot load any js (e.g. code-hightlight..)
        SwingUtilities::fixWindowsRegistry();

        // initial system properties file
        const File& f (File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("wdtp.sys"));
        PropertiesFile::Options options;
        options.storageFormat = PropertiesFile::storeAsXML;
        systemFile = new PropertiesFile (f, options);

        // when first run this application...
        if (!f.existsAsFile())
        {
            systemFile->setValue ("language", 0);  // 0 for English
            systemFile->setValue ("uiBackground", Colour (0xffdcdbdb).toString());
            systemFile->setValue ("uiTextColour", Colour (0xff303030).toString());
            systemFile->setValue ("editorFontColour", Colour (0xff303030).toString());
            systemFile->setValue ("editorBackground", Colour (0xffdedede).toString());
            systemFile->setValue ("fontSize", SwingUtilities::getFontSize());

            systemFile->save();
        }

        // command manager and audio format
        cmdManager = new ApplicationCommandManager();        
        formatManager = new AudioFormatManager();
        formatManager->registerBasicFormats();

        // initial audio device
        deviceManager = new AudioDeviceManager();
        ScopedPointer<XmlElement> audioState (systemFile->getXmlValue ("audioState"));
        deviceManager->initialise (2, 2, audioState, true);

        // initial lame encoder file
        int lameZipFileIndex = 0;

#if JUCE_WINDOWS
        lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame.exe");
#elif JUCE_MAC
        lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame");
        lameZipFileIndex = 1;
#endif

        if (!lameEncoder.existsAsFile())
        {
            // release lame encoder in userDocumentsDirectory
            MemoryInputStream inputSteam (BinaryData::lame_zip, BinaryData::lame_zipSize, false);
            ZipFile zip (inputSteam);
            zip.uncompressEntry (lameZipFileIndex, File::getSpecialLocation (File::userDocumentsDirectory));
        }

        // initial application's GUI
        LookAndFeel::setDefaultLookAndFeel (lnf = new SwingLookAndFeel());
        mainWindow = new MainWindow (getApplicationName());

        // double click to open a project/packed-project in OS-finder
        if (commandLine.isNotEmpty() &&
            (commandLine.contains (".wdtp") || commandLine.contains (".wpck")))
        {
            mainWindow->openProject (File (commandLine.unquoted()));
        }
        else // open the project when eixts the app the last time
        {
            RecentlyOpenedFilesList  recentFiles;
            recentFiles.restoreFromString (systemFile->getValue ("recentFiles"));
            const File& projectFile (recentFiles.getFile (0));

            if (projectFile.existsAsFile())
                mainWindow->openProject (projectFile);
        }
    }

    //=========================================================================
    void shutdown() override
    {
        // must destroy all guis first since they're using the systemFile object
        mainWindow = nullptr;
        systemFile->saveIfNeeded();

        deleteAndZero (systemFile);
        deleteAndZero (cmdManager);
        deleteAndZero (formatManager);
        deleteAndZero (deviceManager);

        // delete all desktop when one of them is showing while exit the application
        for (int i = Component::getNumCurrentlyModalComponents(); --i >= 0; )
            delete (Component::getCurrentlyModalComponent (i));
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

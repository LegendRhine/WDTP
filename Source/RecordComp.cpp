/*
  ==============================================================================

    RecordAndEditComp.cpp
    Created: 8 Jun 2015 1:03:18am
    Author:  mit2000

  ==============================================================================
*/

#include "WdtpHeader.h"

extern AudioDeviceManager* deviceManager;
extern AudioFormatManager* formatManager;
extern File lameEncoder;

const float transValue = 0.65f;

//==============================================================================
RecordComp::RecordComp (const File& docFile) :
    currentSeconds (0),
    totalSeconds (0),
    baseTime (0),
    needSaveToMediaDir (false),
    mediaDir (docFile.getSiblingFile ("media")),
    player (new AudioDataPlayer()), 
    audioReader (nullptr)
{
    // buttons
    for (int i = 0; i < totalButtons; ++i)
    {
        buttons.add (new ImageButton());
        addAndMakeVisible (buttons[i]);
        buttons[i]->addListener (this);
    }

    buttons[recBt]->setTooltip (TRANS("Record/Stop"));
    buttons[recBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::mic_png, BinaryData::mic_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[playBt]->setTooltip (TRANS("Play/Pause"));
    buttons[playBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::play_png, BinaryData::play_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[delBt]->setTooltip (TRANS("Delete"));
    buttons[delBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::close_png, BinaryData::close_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[doneBt]->setTooltip (TRANS("Done"));
    buttons[doneBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::ok_png, BinaryData::ok_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    // audio record things
    addAndMakeVisible (recordThumbnail = new RecordingThumbnail());
    recordThumbnail->setThumbColour (Colours::cyan);
    recorder = new AudioRecorder (*deviceManager, recordThumbnail->getAudioThumbnail());
    
    // labels
    addAndMakeVisible (currentTimeLabel = new Label (String::empty, "0:00.0"));
    currentTimeLabel->setColour (Label::textColourId, Colours::yellowgreen);
    currentTimeLabel->setJustificationType (Justification::centred);
    
    addAndMakeVisible (totalTimeLabel = new Label (String::empty, "0:00.0"));
    totalTimeLabel->setColour (Label::textColourId, Colour (0xffd37bf8));
    totalTimeLabel->setJustificationType (Justification::centred);

    // play line
    currentPositionMarker.setFill (Colours::white.withAlpha (0.85f));
    addAndMakeVisible (currentPositionMarker);
    recordThumbnail->setInterceptsMouseClicks (false, false);

    setSize (580, 350);

    player->addListener (this);
    startRecord();
}
//=========================================================================
RecordComp::~RecordComp()
{
    stopTimer();

    player->removeListener (this);
    player->stop();
    player->setAudioSource (nullptr);

    deleteAndZero (player);
    deleteAndZero (recorder);
    deleteAndZero (recordThumbnail);
}
//=========================================================================
void RecordComp::paint (Graphics& g)
{
    g.fillAll();

    const int labelHeight = (getHeight() - 40 )/ 6;
    g.setColour (Colours::grey.withAlpha (0.80f));
    g.drawHorizontalLine (labelHeight, 1.0f, getWidth() - 2.0f);
    g.drawHorizontalLine (labelHeight * 3, 1.0f, getWidth() - 2.0f);
    g.drawHorizontalLine (getHeight() - 40 - labelHeight, 1.0f, getWidth() - 2.0f);

    g.setColour (Colours::darkgrey.darker());
    g.fillRect (0, getHeight() - 45, getWidth(), 45);
}
//=========================================================================
void RecordComp::resized()
{
    const int labelHeight = (getHeight() - 40) / 6;
    const int thumbHeight = labelHeight * 4;

    currentTimeLabel->setFont (labelHeight / 1.5f);
    totalTimeLabel->setFont (labelHeight / 1.5f);

    totalTimeLabel->setBounds (0, 5, getWidth(), labelHeight - 10);
    recordThumbnail->setBounds (1, (getHeight() - 40 - thumbHeight) / 2, getWidth() - 2, thumbHeight);
    currentTimeLabel->setBounds (0, getHeight() - 37 - labelHeight, getWidth(), labelHeight - 10);

    const int centerX = (getWidth() - 90) / 2;
    buttons[playBt]->setBounds (centerX, getHeight() - 40, 32, 32);
    buttons[recBt]->setBounds (buttons[playBt]->getX() - 57, buttons[playBt]->getY(), 32, 32);
    buttons[delBt]->setBounds (buttons[playBt]->getRight() + 25, buttons[playBt]->getY(), 32, 32);
    buttons[doneBt]->setBounds (buttons[delBt]->getRight() + 25, buttons[playBt]->getY(), 32, 32);

    grabKeyboardFocus();
}
//=================================================================================================
void RecordComp::buttonClicked (Button* button)
{
    // record
    if (buttons[recBt] == button)
    {
        recorder->isRecording() ? stopRecord() : beginRecord();
    }

    // play / pause
    else if (buttons[playBt] == button)
    {        
        if (player->isPlaying())
        {
            player->stop();
            stopTimer();
        }
        else
        {
            player->start();
            startTimer (33);
        }
    }
    
    // delete current audio file which was in the attach dir
    else if (buttons[delBt] == button)
    {
        setAudioReader (nullptr);        

        currentPositionMarker.setRectangle (Rectangle<float> (0.0f, 
            (getHeight() - 40) / 4.0f + 5.0f, 1.5f, (getHeight() - 40) / 2.0f - 8.0f));

        audioReader = nullptr;
        needSaveToMediaDir = false;
        repaint();
    }

    // save and close
    else if (buttons[doneBt] == button)
    {
        player->removeListener (this);
        player->stop();

        if (needSaveToMediaDir && audioReader != nullptr)
        {
            const String& audioName (SwingUtilities::getCurrentTimeString() + ".mp3");
            writeMp3AudioToMediaDir (audioName);
            needSaveToMediaDir = false;  
        }

        DialogWindow* dialog = findParentComponentOfClass<DialogWindow>();

        if (dialog != nullptr)
            dialog->exitModalState (0);
    }
}
//=================================================================================================
void RecordComp::setAudioReader (AudioFormatReader* reader)
{
    audioReader = reader;
    player->setAudioSource (audioReader);

    if (audioReader != nullptr)
    {
        recordThumbnail->setDisplayFullThumbnail (true);

        buttons[recBt]->setEnabled (false);
        buttons[playBt]->setEnabled (true);
        buttons[delBt]->setEnabled (true);
        buttons[doneBt]->setEnabled (true);
    }
    else
    {
        recordThumbnail->setReader (nullptr);
        recordThumbnail->setDisplayFullThumbnail (false);

        buttons[recBt]->setEnabled (true);
        buttons[playBt]->setEnabled (false);
        buttons[delBt]->setEnabled (false);
        buttons[doneBt]->setEnabled (true);
    }

    currentTimeLabel->setText ("0:00.0", dontSendNotification);
    totalTimeLabel->setText (SwingUtilities::doubleToString (player->getLengthInSeconds()), dontSendNotification);

    const float labelHeight = (getHeight() - 40) / 4.0f;
    currentPositionMarker.setRectangle (Rectangle<float> (
        0.0f, 
        labelHeight + 5.0f,
        1.5f, labelHeight * 2.0f - 8.0f));
}
//=================================================================================================
void RecordComp::playFinished (AudioDataPlayer* player_)
{
    buttons[playBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::play_png, BinaryData::play_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[delBt]->setEnabled (true);
    buttons[doneBt]->setEnabled (true);

    player_->setPosition (0.0);
    stopTimer();
}
//=================================================================================================
void RecordComp::playerStarted (AudioDataPlayer* /*player*/)
{
    buttons[playBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::stop_png, BinaryData::stop_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[delBt]->setEnabled (false);
    buttons[doneBt]->setEnabled (false);
}
//=================================================================================================
void RecordComp::playerStopped (AudioDataPlayer* /*player*/)
{
    buttons[playBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::play_png, BinaryData::play_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[delBt]->setEnabled (true);
    buttons[doneBt]->setEnabled (true);
}

//=================================================================================================
void RecordComp::writeMp3AudioToMediaDir (const String& fileName)
{
/*

#if JUCE_WINDOWS
    const File lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame.exe");
#elif JUCE_MAC
    const File lameEncoder = File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("lame.app");
#endif

    if (!lameEncoder.existsAsFile())
    {
        SHOW_MESSAGE (TRANS ("Cannot find Lame encoder.") + newLine + newLine
        + TRANS ("Please download the encoder and put it into OS 'Documents' directory."));

        return;
    }*/

    if (!lameEncoder.existsAsFile())
    {
        SHOW_MESSAGE (TRANS ("Cannot find Lame-Encoder.") + newLine + newLine
                      + TRANS ("Please download the encoder and put it into OS 'Documents' directory."));
        return;
    }

    File audioFile (mediaDir.getChildFile (fileName).getNonexistentSibling (false));
    audioFile.create();    

    LAMEEncoderAudioFormat mp3Format (lameEncoder);
    FileOutputStream* outputStream (audioFile.createOutputStream());
    ScopedPointer<AudioFormatWriter> writer = mp3Format.createWriterFor (outputStream,
                                audioReader->sampleRate, 1, 16, StringPairArray(), 6);

    jassert (writer != nullptr);

    if (writer->writeFromAudioReader (*audioReader, 0, -1))
        sendActionMessage (fileName);
    else
        SHOW_MESSAGE (TRANS ("Can't save this audio."));
}

//=================================================================================================
void RecordComp::timerCallback()
{
    if (recorder->isRecording())
    {
        const double currentRecordingSeconds = (Time::getMillisecondCounter() - baseTime) / 1000.0;
        currentTimeLabel->setText (SwingUtilities::doubleToString (currentRecordingSeconds), dontSendNotification);
        totalTimeLabel->setText (SwingUtilities::doubleToString (currentRecordingSeconds), dontSendNotification);
    }
    else
    {
        const float timeToX = float (getWidth() * (player->getCurrentPosition() / player->getLengthInSeconds()));
        const double currentSeconds_ = player->getCurrentPosition();
        currentTimeLabel->setText (SwingUtilities::doubleToString (currentSeconds_), dontSendNotification);

        const float labelHeight = (getHeight() - 40) / 6.0f;

        currentPositionMarker.setRectangle (Rectangle<float> (
            timeToX - 0.75f, 
            labelHeight + 2.0f,
            1.5f, labelHeight * 4.0f - 2.0f));
    }    
}

//=================================================================================================
void RecordComp::mouseDown (const MouseEvent& e)
{
    if ((e.y < getHeight() - 40) && player->getLengthInSeconds() > 0)
    {
        mouseDrag (e);

        if (player->isPlaying())
            stopTimer();
    }
}
//=================================================================================================
void RecordComp::mouseDrag (const MouseEvent& e)
{
    if ((e.y < getHeight() - 40) && player->getLengthInSeconds() > 0)
    {
        const int labelHeight = (getHeight() - 40) / 4;
        currentPositionMarker.setRectangle (Rectangle<float> (
            float (e.x), 
            labelHeight + 5.0f,
            1.5f, labelHeight * 2.0f - 8.0f));

        double xToTime = double (e.x) / getWidth() * player->getLengthInSeconds();
        xToTime = jlimit (0.0, player->getLengthInSeconds(), xToTime);
        currentTimeLabel->setText (SwingUtilities::doubleToString (xToTime), dontSendNotification);

        currentPositionMarker.setRectangle (Rectangle<float> (
            e.x - 0.75f, 
            labelHeight + 5.0f,
            1.5f, labelHeight * 2.0f - 8.0f));
    }
}
//=================================================================================================
void RecordComp::mouseUp (const MouseEvent& e)
{
    if (player->getLengthInSeconds() > 0)
    {
        double xToTime = double (e.x) / getWidth() * player->getLengthInSeconds();
        xToTime = jlimit (0.0, player->getLengthInSeconds(), xToTime);
        player->setPosition (xToTime);

        if (player->isPlaying())
            startTimer (33);
    }
}
//=================================================================================================
void RecordComp::startRecord()
{
    if (audioReader == nullptr)
        beginRecord();
}
//=================================================================================================
void RecordComp::beginRecord()
{
    stopTimer();
    player->setAudioSource (nullptr);
    recordThumbnail->setDisplayFullThumbnail (false);
    recorder->startRecording();
    baseTime = Time::getMillisecondCounter();
    startTimer (33);
    needSaveToMediaDir = false;

    buttons[recBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::stop_png, BinaryData::stop_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[recBt]->setEnabled (true);
    buttons[playBt]->setEnabled (false);
    buttons[delBt]->setEnabled (false);
    buttons[doneBt]->setEnabled (false);
}
//=================================================================================================
void RecordComp::stopRecord()
{
    stopTimer();
    recorder->stop();
    audioReader = formatManager->createReaderFor (recorder->getTempFile());
    player->setAudioSource (audioReader);

    recordThumbnail->setDisplayFullThumbnail (true);
    totalTimeLabel->setText (SwingUtilities::doubleToString (player->getLengthInSeconds()), dontSendNotification);
    needSaveToMediaDir = true;

    buttons[recBt]->setImages (false, true, true,
        ImageCache::getFromMemory (BinaryData::mic_png, BinaryData::mic_pngSize), 
        transValue, Colour (0x00),
        Image::null, 1.000f, Colour (0x00),
        Image::null, 1.000f, Colour (0x00));

    buttons[recBt]->setEnabled (false);
    buttons[playBt]->setEnabled (true);
    buttons[delBt]->setEnabled (true);
    buttons[doneBt]->setEnabled (true);
}


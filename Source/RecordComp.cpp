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
    startSample (0),
    samplesNum(0),
    volGain (1.f)
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

    buttons[cutBt]->setTooltip (TRANS ("Cut by Pointer"));
    buttons[cutBt]->setImages (false, true, true,
                                ImageCache::getFromMemory (BinaryData::cut_png, BinaryData::cut_pngSize),
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

    // volume slider
    addAndMakeVisible (volSlider = new Slider (Slider::LinearHorizontal, Slider::NoTextBox));
    volSlider->setRange (0.2, 10.0, 0.1);
    volSlider->setDoubleClickReturnValue (true, 1.0);

    volSlider->setValue (1.0, dontSendNotification);
    volSlider->addListener (this);

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

    const int centerX = (getWidth() - 150) / 2;
    buttons[playBt]->setBounds (centerX, getHeight() - 40, 32, 32);
    buttons[recBt]->setBounds (buttons[playBt]->getX() - 57, buttons[playBt]->getY(), 32, 32);
    buttons[cutBt]->setBounds (buttons[playBt]->getRight() + 25, buttons[playBt]->getY(), 32, 30);
    buttons[delBt]->setBounds (buttons[cutBt]->getRight() + 25, buttons[playBt]->getY(), 32, 32);
    buttons[doneBt]->setBounds (buttons[delBt]->getRight() + 25, buttons[playBt]->getY(), 32, 32);

    volSlider->setBounds (getWidth() - 160, 12, 140, 30);
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

    // cut by pointer
    else if (buttons[cutBt] == button && player->getReaderOfCuurentHold() != nullptr)
    {
        int64 position = int64 (player->getCurrentPosition() * player->getReaderOfCuurentHold()->sampleRate);
        int64 currentSamples = player->getReaderOfCuurentHold()->lengthInSamples;

        if (position < 480 || position > currentSamples - 480)
            return;

        const bool cutFrontPart = (position < currentSamples / 2);
        AudioFormatReader* formatReader = formatManager->createReaderFor (recorder->getTempFile());

        if (cutFrontPart)
        {
            startSample += position;
            samplesNum = currentSamples - position;
        }
        else
        {
            samplesNum = position;
        }

        AudioSubsectionReader* subReader = new AudioSubsectionReader (formatReader, startSample, samplesNum, true);
        recordThumbnail->setReader (new AudioSubsectionReader (formatReader, startSample, samplesNum, false));        
        setAudioReader (subReader);
    }
    
    // delete current audio file which was in the attach dir
    else if (buttons[delBt] == button)
    {
        setAudioReader (nullptr);        
        needSaveToMediaDir = false;

        currentPositionMarker.setRectangle (Rectangle<float> (0.0f, 
            (getHeight() - 40) / 4.0f + 5.0f, 1.5f, (getHeight() - 40) / 2.0f - 8.0f));

        repaint();
    }

    // save and close
    else if (buttons[doneBt] == button)
    {
        player->removeListener (this);
        player->stop();

        if (needSaveToMediaDir && player->getReaderOfCuurentHold() != nullptr)
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
    player->setAudioSource (reader);

    if (reader != nullptr)
    {
        recordThumbnail->setDisplayFullThumbnail (true);
        recordThumbnail->setZoomFactor (volGain);

        buttons[recBt]->setEnabled (false);
        buttons[playBt]->setEnabled (true);
        buttons[delBt]->setEnabled (true);
        buttons[cutBt]->setEnabled (true);
        buttons[doneBt]->setEnabled (true);

        volSlider->setEnabled (true);
        player->setGain (volGain);
    }
    else
    {
        recordThumbnail->setReader (nullptr);
        recordThumbnail->setDisplayFullThumbnail (false);

        buttons[recBt]->setEnabled (true);
        buttons[playBt]->setEnabled (false);
        buttons[cutBt]->setEnabled (false);
        buttons[delBt]->setEnabled (false);
        buttons[doneBt]->setEnabled (true);

        volSlider->setValue (1.0);
        volSlider->setEnabled (false);
    }

    currentTimeLabel->setText ("0:00.0", dontSendNotification);
    totalTimeLabel->setText (SwingUtilities::doubleToString (player->getLengthInSeconds()), dontSendNotification);

    const float labelHeight = (getHeight() - 40) / 6.0f;
    currentPositionMarker.setRectangle (Rectangle<float> (
        0.0f, 
        labelHeight + 5.0f,
        1.5f, labelHeight * 4.0f - 2.0f));
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
    buttons[cutBt]->setEnabled (true);

    player_->setPosition (0.0);
    stopTimer();

    const float labelHeight = (getHeight() - 40) / 6.0f;
    currentPositionMarker.setRectangle (Rectangle<float> (
        0.0f,
        labelHeight + 5.0f,
        1.5f, labelHeight * 4.0f - 2.0f));
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
    buttons[cutBt]->setEnabled (false);
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
    buttons[cutBt]->setEnabled (true);
}

//=================================================================================================
void RecordComp::writeMp3AudioToMediaDir (const String& fileName)
{
    if (!lameEncoder.existsAsFile())
    {
        SHOW_MESSAGE (TRANS ("Cannot find MP3 encoder.") + newLine
                      + TRANS ("Please connect internet and restart the app."));
        return;
    }

    File audioFile (mediaDir.getChildFile (fileName).getNonexistentSibling (false));
    audioFile.create();    

    LAMEEncoderAudioFormat mp3Format (lameEncoder);
    FileOutputStream* outputStream (audioFile.createOutputStream());
    ScopedPointer<AudioFormatWriter> writer = mp3Format.createWriterFor (outputStream,
                                              player->getReaderOfCuurentHold()->sampleRate, 1, 16, StringPairArray(), 6);

    jassert (writer != nullptr);

    // gain ramp process
    AudioFormatReader* reader = player->getReaderOfCuurentHold();
    AudioSampleBuffer buffer (reader->numChannels, (int)reader->lengthInSamples);
    reader->read (&buffer, 0, (int)reader->lengthInSamples, 0, true, true);

    buffer.applyGain (0, (int)reader->lengthInSamples, volGain);
    buffer.applyGainRamp (0, 4410, 0.f, 1.f);
    buffer.applyGainRamp ((int)reader->lengthInSamples - 4410, 4410, 1.f, 0.f);

    // write to mp3
    if (writer->writeFromAudioSampleBuffer (buffer, 0, (int)reader->lengthInSamples))
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
        const int labelHeight = (getHeight() - 40) / 6;
        double xToTime = double (e.x) / getWidth() * player->getLengthInSeconds();

        xToTime = jlimit (0.0, player->getLengthInSeconds(), xToTime);
        currentTimeLabel->setText (SwingUtilities::doubleToString (xToTime), dontSendNotification);

        currentPositionMarker.setRectangle (Rectangle<float> (
            e.x - 0.75f, 
            labelHeight + 5.0f,
            1.5f, labelHeight * 4.0f - 2.0f));
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
void RecordComp::sliderValueChanged (Slider* slider)
{
    if (slider == volSlider)
    {
        volGain = (float)volSlider->getValue();
        player->setGain (volGain);
        recordThumbnail->setZoomFactor (volGain);
    }
}

//=================================================================================================
void RecordComp::startRecord()
{
    if (player->getReaderOfCuurentHold() == nullptr)
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
    buttons[cutBt]->setEnabled (false);
    buttons[delBt]->setEnabled (false);
    buttons[doneBt]->setEnabled (false);
    volSlider->setEnabled (false);
}
//=================================================================================================
void RecordComp::stopRecord()
{
    stopTimer();
    recorder->stop();
    
    AudioFormatReader* formatReader = formatManager->createReaderFor (recorder->getTempFile());
    AudioSubsectionReader* subReader = new AudioSubsectionReader (formatReader, 0, formatReader->lengthInSamples, true);

    startSample = 0;
    samplesNum = formatReader->lengthInSamples;
    player->setAudioSource (subReader);

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
    buttons[cutBt]->setEnabled (true);
    buttons[delBt]->setEnabled (true);
    buttons[doneBt]->setEnabled (true);
    volSlider->setEnabled (true);
}


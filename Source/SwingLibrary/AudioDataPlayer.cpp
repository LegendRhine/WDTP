/*
  ==============================================================================

    AudioDataPlayer.cpp
    Created: 20 Nov 2013 12:55:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "AudioDataPlayer.h"

extern AudioDeviceManager* deviceManager;
extern AudioFormatManager* formatManager;

//=================================================================================================
AudioDataPlayer::AudioDataPlayer()
    : timeSliceThread ("AudioDataPlayer's TimeSliceThread"),
    reader (nullptr)
{
    timeSliceThread.startThread (3);

    audioTransportSource.addChangeListener (this);
    sourcePlayer.setSource (&audioTransportSource);
    deviceManager->addAudioCallback (&sourcePlayer);
}
//=================================================================================================
AudioDataPlayer::~AudioDataPlayer()
{
    deviceManager->removeAudioCallback (&sourcePlayer);

    audioTransportSource.stop();
    audioTransportSource.setSource (nullptr);
    audioTransportSource.removeChangeListener (this);
    sourcePlayer.setSource (nullptr);
}
//=================================================================================================
void AudioDataPlayer::setAudioSource (AudioFormatReader* audioReader)
{
    audioTransportSource.stop();
    audioTransportSource.setSource (nullptr);
    reader = audioReader;
    
    if (reader == nullptr)
    {
        audioSource = nullptr;
    }
    else
    {
        audioSource = new AudioFormatReaderSource (reader, true);
        audioTransportSource.setSource (audioSource, 
                                        32768, 
                                        &timeSliceThread, 
                                        reader->sampleRate);
    }
}
//=================================================================================================
void AudioDataPlayer::setAudioSource (const File& audioFile)
{
    AudioFormatReader* formatReader = formatManager->createReaderFor (audioFile);
    setAudioSource (formatReader);
}
//=================================================================================================
void AudioDataPlayer::start()
{
    audioTransportSource.removeChangeListener (this);
    audioTransportSource.start();
    listeners.call (&Listener::playerStarted, this);
    audioTransportSource.addChangeListener (this);
}
//=================================================================================================
void AudioDataPlayer::stop()
{
    audioTransportSource.removeChangeListener (this);
    audioTransportSource.stop();
    listeners.call (&Listener::playerStopped, this);
    audioTransportSource.addChangeListener (this);
}
//=================================================================================================
void AudioDataPlayer::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &audioTransportSource)
    {        
        if (! audioTransportSource.isPlaying())
            listeners.call (&Listener::playFinished, this);
    }
}


/*
  ==============================================================================

    AudioRecorder.cpp
    Created: 19 Oct 2013 11:54:28pm
    Author:  mit2000

  ==============================================================================
*/

#include "JuceHeader.h"
#include "AudioRecorder.h"

extern AudioDeviceManager* deviceManager;
extern AudioFormatManager* formatManager;

//=================================================================================================
AudioRecorder::AudioRecorder (AudioDeviceManager& dm,
                              AudioThumbnail& thumbnailToUpdate) : 
    deviceManager(dm),
    backgroundThread ("Audio Recorder Thread"),
    thumbnail (thumbnailToUpdate),
    sampleRate (0.00),
    nextSampleNum (0),
    activeWriter (nullptr)
{
#if (JUCE_ANDROID || JUCE_IOS)
    tempFile = File::getSpecialLocation (File::userDocumentsDirectory)
                                        .getNonexistentChildFile ("MyBioTempRecord", ".ogg");
#else
    tempFile = File::createTempFile ("ogg");
#endif
    
    backgroundThread.startThread();
    deviceManager.addAudioCallback (this);
}

//=================================================================================================
AudioRecorder::~AudioRecorder ()
{
    deviceManager.removeAudioCallback (this);
    stop();
    tempFile.deleteFile();
}

//=================================================================================================
void AudioRecorder::startRecording ()
{
    stop();

    if (! tempFile.deleteFile())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, TRANS("Can't start recording"),
                                     TRANS("Temp file maybe using now or missing."));
        return;
    }

    ScopedPointer<FileOutputStream> outputStream (tempFile.createOutputStream());

    if (outputStream == nullptr || sampleRate <= 0)
        return;

    OggVorbisAudioFormat oggFormat;

    // createWriterFor()最后一个参数目前设置为6（192 kbps）。可采用其他设置：
    // 2: 96 kbps。 3: 112 kbps。4: 128 kbps。5: 160 kbps。
    // 6: 192 kbps。7: 224 kbps。8: 256 kbps。9: 320 kbps。
    AudioFormatWriter* writer = oggFormat.createWriterFor (outputStream, sampleRate, 1, 16, StringPairArray(), 6);

    if (writer == nullptr)
        return;

    outputStream.release(); 
    threadedWriter = new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768);

    // AudioThumbnail对象重置所要绘制的音频数据
    thumbnail.reset (writer->getNumChannels(), writer->getSampleRate());
    nextSampleNum = 0;

    const ScopedLock sl (writerLock);
    activeWriter = threadedWriter;
}

//=================================================================================================
void AudioRecorder::stop ()
{
    {
        const ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }

    threadedWriter = nullptr;
}

//=================================================================================================
void AudioRecorder::audioDeviceIOCallback (const float** inputChannelData, 
                                           int /*numInputChannels*/, 
                                           float** outputChannelData, 
                                           int numOutputChannels, 
                                           int numSamples)
{
    const ScopedLock sl (writerLock);

    // 录音数据写入（追加到）磁盘文件
    if (activeWriter != nullptr)
    {
        activeWriter->write (inputChannelData, numSamples);

        // 创建音频缓冲，引用声卡输入端口进来的音频数据
        const AudioSampleBuffer buffer (const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);

        thumbnail.addBlock (nextSampleNum, buffer, 0, numSamples);
        nextSampleNum += numSamples;
    }

    for (int i = 0; i < numOutputChannels; ++i)
    {
        if (outputChannelData[i] != nullptr)
            zeromem (outputChannelData[i], sizeof (float) * (size_t) numSamples);
    }
}
//=================================================================================================
const bool AudioRecorder::saveToFile (File& file) const
{
    file.create();
    return tempFile.moveFileTo (file);
}

//=================================================================================================
RecordingThumbnail::RecordingThumbnail () : 
                            thumbnailCache (3),
                            thumbnail (512, *formatManager, thumbnailCache),
                            thumbColour (Colours::green),
                            displayFullThumb (false)
{
    thumbnail.addChangeListener (this);
}
//=================================================================================================
void RecordingThumbnail::setDisplayFullThumbnail (bool displayFull)
{
    displayFullThumb = displayFull;
    repaint();
}
//=================================================================================================
void RecordingThumbnail::paint (Graphics& g)
{
    g.setColour (thumbColour);

    if (thumbnail.getTotalLength() > 0.0)
    {
        const double endTime = displayFullThumb ? thumbnail.getTotalLength()
                                                : jmax (30.0, thumbnail.getTotalLength());

        thumbnail.drawChannels (g, getLocalBounds().reduced (2), 0.0, endTime, 1.0f);
    }
}
//=================================================================================================
void RecordingThumbnail::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}


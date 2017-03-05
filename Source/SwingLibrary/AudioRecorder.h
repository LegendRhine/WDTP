/*
  ==============================================================================

    AudioRecorder.h
    Created: 19 Oct 2013 11:54:28pm
    Author:  mit2000

  ==============================================================================
*/

#ifndef AUDIORECORDER_H_INCLUDED
#define AUDIORECORDER_H_INCLUDED

/** 录制Ogg格式的音频数据（采样率为当前用户声卡的采样率、声道数1、压缩品质6, 位率16 bit）。
    可将录音结果保存为所指定的磁盘文件。录音时，本类可实时更新 AudioThumbnail 对象所要绘制的音频数据。

    每次录音时，内部将录音数据保存到一个临时文件中，该文件位于系统临时目录下(C:/Users/mit2000/AppData/Local/Temp，文件名以temp开头，扩展名为：“*.ogg”)，销毁本类时一并删除该临时文件。可返回该临时文件供播放器所用，以便试听本次录音结果。

    用法：
    - 创建对象，构造参数应为全局性 AudioDeviceManager。
    - 调用 startRecording() 开始录音。
    - 调用 stop() 停止录音。
    - 调用 getTempFile() 可返回本次录音的临时文件。
    - 调用 saveToFile() 可将本次录音保存为指定的磁盘文件。
    - 判断当前是否处于录音状态，可调用 isRecording() 函数。
*/
class AudioRecorder : public AudioIODeviceCallback
{
public:
    /** 构造1参：本类所需的全局性音频设备管理器。2参：录音时，本类实时更新该对象所要绘制的音频数据。 */
    AudioRecorder (AudioDeviceManager& dm, 
                   AudioThumbnail& thumbnailToUpdate);

    /** 析构函数。 */
    ~AudioRecorder();

    //=================================================================================================
    /** 开始录音，启动计时器，数据写入临时文件中（覆盖上次的录音数据）。
    
        如要保留上次录音，则调用此函数之前，先将临时文件另存。
        当然，创建本类对象后，第一次调用此函数之前，无需另存临时文件。
    */
    void startRecording ();

    /** 停止录音。 */
    void stop();

    /** 如果当前正在录音，则返回true。 */
    bool isRecording() const                                { return activeWriter != nullptr; }

    //=================================================================================================
    /** 返回尚未另存的临时录音文件。此函数可供播放器使用，以便试听本次录音。 */
    const File& getTempFile() const                         { return tempFile; }

    /** 将本次录音保存为给定的文件。该文件中已有的数据将全部被覆盖。保存后，原来的临时录音文件依然存在，但已无任何数据。
    
        @return    保存成功则返回true。否则返回false。*/
    const bool saveToFile (File& file) const;

    //=================================================================================================
    /** 实现基类的纯虚函数。 */
    void audioDeviceAboutToStart (AudioIODevice* device) override   { sampleRate = device->getCurrentSampleRate(); }

    /** 实现基类的纯虚函数。 */
    void audioDeviceStopped() override                               { sampleRate = 0; }

    /** 实现基类的纯虚函数。 */
    void audioDeviceIOCallback (const float** inputChannelData, 
                                int numInputChannels,
                                float** outputChannelData, 
                                int numOutputChannels,
                                int numSamples) override;

private:
    //=================================================================================================
    AudioDeviceManager& deviceManager;      /**< 外部传来的设备管理器，用于回调本类。 */
    TimeSliceThread backgroundThread;       /**< 用于将音频数据写入磁盘文件的时间片线程。 */
    AudioThumbnail& thumbnail;              /**< 本类录音时，实时更新该对象所要绘制的音频波形数据 */

    File tempFile;                          /**< 临时保存录音数据的磁盘临时文件。 */
    double sampleRate;                      /**< 采样率 */
    int64 nextSampleNum;                    /**< 音频设备每次回调时，AudioThumbnail从该位置读取采样 */

    CriticalSection writerLock;             /**< 进入临界区的锁。*/

    /** 基于FIFO来缓冲输入数据的音频格式写入器。*/
    ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter;    

    /** 执行写入磁盘文件的写入器。*/
    AudioFormatWriter::ThreadedWriter* volatile activeWriter; 
};

//=================================================================================================
/** 可实时显示录音波形缩略图和普通音频文件波形缩略图的组件.

    用法：按普通组件创添显设删本类对象，创建后， setThumbColour()设置本类所绘制的缩略图的颜色。

    如需显示音频文件的缩略图，则首先调用 setAudioSource()函数，而后调用 setDisplayFullThumbnail()。

    如需实时显示录音波形，则创建本类对象后，创建 AudioRecorder 对象，该类的构造2参即本类所持有的AudioThumbnail对象。
    而后，本类对象调用 setDisplayFullThumbnail (false)， AudioRecorder对象开始录音即可。
    录音结束后，可 setDisplayFullThumbnail (true)，此时即可显示本次录音的完整波形。
*/
class RecordingThumbnail  : public Component,
                            private ChangeListener
{
public:
    RecordingThumbnail();
    ~RecordingThumbnail()                               { thumbnail.removeChangeListener (this); }

    //=================================================================================================
    /** 设置本类是否绘制完整的缩略图。 */
    void setDisplayFullThumbnail (bool displayFull);

    /** 设置本类所需绘制的音频数据来源。如需实时显示录音波形，则无需调用此函数。 */
    void setAudioSource (InputSource* source)         { thumbnail.setSource (source); }

    void setReader (AudioFormatReader* reader)    { thumbnail.setReader (reader, Time::getCurrentTime().toMilliseconds()); }

    /** 返回本类创建并持有的 AudioThumbnail 对象。 */
    AudioThumbnail& getAudioThumbnail()                 { return thumbnail; }

    //=================================================================================================
    /** 本类所绘制的波形缩略图的颜色。 */
    void setThumbColour (const Colour& colour)          { thumbColour = colour; }

    /** 在此绘制音频波形缩略图。 */
    void paint (Graphics& g) override;

private:
    //=================================================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    ScopedPointer<FileInputStream> thumbFile;

    Colour thumbColour;     /**< 本类所绘制的缩略图颜色 */
    bool displayFullThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordingThumbnail)

};


#endif  // AUDIORECORDER_H_INCLUDED

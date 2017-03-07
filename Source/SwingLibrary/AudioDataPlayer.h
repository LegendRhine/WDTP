/*
  ==============================================================================

    AudioDataPlayer.h
    Created: 20 Nov 2013 12:55:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef AUDIODATAPLAYER_H_INCLUDED
#define AUDIODATAPLAYER_H_INCLUDED

/** 普通的音频播放器，可播放给出的 AudioFormatReader 和音频文件。 AudioFormatReader 堆对象由本类负责销毁。

    基于嵌套的抽象基类 Listener，本类在开始播放，停止播放，当前加载的音频数据播放完成时对外发出回调。
    使用本类对象的类可继承本类的嵌套类 Listener，以捕获并处理这三个事件的回调。

    本类创建并持有的 AudioTransportSource 对象在开始播放、停止播放、当前数据播放完成（单次循环结束）时均发出可变消息。
    本类继承了ChangeListener可变捕获，在changeListenerCallback()回调函数中做出相应的判断处理并对外发出回调。
    
    用法：创建对象，调用 setAudioSource()加载要播放的音频来源，而后进行各类播放控制。如要卸载当前加载的音频数据，
    则 setAudioSource (nullptr)。只有当前加载的音频文件被卸载，才可以移动、删除之。
*/
class AudioDataPlayer : private ChangeListener
{
public:
    //=================================================================================================
	AudioDataPlayer();
	~AudioDataPlayer();
	
    //=================================================================================================
	/** 加载准备播放的 AudioFormatReader 。1参对象由本类负责销毁。如果为nullptr，则视为卸载当前已加载的音频数据。*/
	void setAudioSource (AudioFormatReader* reader);

    /** 加载准备播放的音频文件。 */
    void setAudioSource (const File& audioFile);

    //=================================================================================================
    /** 播放当前所加载的音频数据。*/
    void start();

    /** 暂停播放。暂停后，当前播放位置停留在该处，不做任何设置和假设。 */
    void stop();

    /** 返回当前语句是否处于播放状态。*/
    const bool isPlaying() const                    { return audioTransportSource.isPlaying(); }

    AudioFormatReader* getReaderOfCuurentHold()     { return reader; }

    //=================================================================================================
    /** 设置播放位置（秒数）。*/
    void setPosition (double newPosition)           { audioTransportSource.setPosition (newPosition); }

    /** 返回当前的播放位置（秒数）。*/
    const double getCurrentPosition() const         { return audioTransportSource.getCurrentPosition(); }

    /** 返回所加载的音频数据的总长度（秒数）。*/
    const double getLengthInSeconds() const         { return audioTransportSource.getLengthInSeconds(); }

    //=================================================================================================
    /** 本类的派生类可捕获并处理 AudioDataPlayer 所发出回调。 */
    class Listener
    {
    public:
        virtual ~Listener() {}
		
        virtual void playFinished (AudioDataPlayer* player)  = 0;
        virtual void playerStarted (AudioDataPlayer* player) = 0;
        virtual void playerStopped (AudioDataPlayer* player) = 0;
    };
	
    //=================================================================================================
    void addListener (Listener* listener)           { listeners.add (listener); }
    void removeListener (Listener* listener)        { listeners.remove (listener); }
    void removeAllListeners()                       { listeners.clear(); }
    
private:	
    //=================================================================================================
    /** 在此对外发出当前所加载的音频数据已播放完成的回调消息。 */
    void changeListenerCallback (ChangeBroadcaster* source);
    
    //=================================================================================================
    AudioTransportSource audioTransportSource;
    AudioSourcePlayer sourcePlayer;
    TimeSliceThread timeSliceThread;
    AudioFormatReader* reader;

    ScopedPointer<AudioFormatReaderSource> audioSource;
    ListenerList<Listener> listeners;
        
    //=================================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDataPlayer);
    
};


#endif  // AUDIODATAPLAYER_H_INCLUDED

/*
  ==============================================================================

    RecordAndEditComp.h
    Created: 8 Jun 2015 1:03:18am
    Author:  mit2000

  ==============================================================================
*/

#ifndef RECORDANDCOMP_H_INCLUDED
#define RECORDANDCOMP_H_INCLUDED

class RecordingThumbnail;

//==============================================================================
/** Usage: whenever use this class, create object for each time! then setVisible()!
*/
class RecordComp : public Component,
                   public ActionBroadcaster,
                   public ButtonListener,
                   private AudioDataPlayer::Listener,
                   private Timer
{
public:
    RecordComp (const File& docFile);
    ~RecordComp();

    //void setRecToPlay (AudioFormatReader* reader);

    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked (Button* button) override;

    // click to position the play line
    void mouseDown (const MouseEvent& event) override;
    void mouseDrag (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;

    AudioFormatReader* getAudioReader()        { return audioReader; }

private:
    //=========================================================================
    enum Buttons  { recBt = 0, playBt, delBt, doneBt, totalButtons };

    // refresh labels
    virtual void timerCallback() override;

    void setAudioReader (AudioFormatReader* reader);
        
    void startRecord();
    void beginRecord();
    void stopRecord();

    virtual void playFinished (AudioDataPlayer* player) override;
    virtual void playerStarted (AudioDataPlayer* player) override;
    virtual void playerStopped (AudioDataPlayer* player) override;

    void writeMp3AudioToMediaDir (const String& fileName);

    //=========================================================================
    double currentSeconds, totalSeconds;
    uint32 baseTime;
    bool needSaveToMediaDir;
    const File mediaDir;

    AudioRecorder* recorder;
    AudioDataPlayer* player;
    AudioFormatReader* audioReader;
    RecordingThumbnail* recordThumbnail;
    DrawableRectangle currentPositionMarker;

    OwnedArray<ImageButton> buttons;
    ScopedPointer<Label> currentTimeLabel;
    ScopedPointer<Label> totalTimeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordComp)        

};


#endif  // RECORDANDCOMP_H_INCLUDED

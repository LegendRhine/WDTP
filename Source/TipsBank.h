/*
  ==============================================================================

    TipsBank.h
    Created: 30 Mar 2017 8:08:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef TIPSBANK_H_INCLUDED
#define TIPSBANK_H_INCLUDED

/** Extrct the content of 'project/docs/tips.md' and build/process the tips bank.

    Tips format:

    - key string 1 (must more than one character)
        - value string 1
    - key string 2
        - value string 2
    ...
*/
class TipsBank : private Thread
{
public:
    ~TipsBank();
    
    juce_DeclareSingleton (TipsBank, true);

    void rebuildTipsBank();
    const bool isRebuilding()                               { return isThreadRunning(); }

    const HashMap<String, String>& getTipsBank() const      { return tipsBank; }
    
private:
    TipsBank();

    /** build tips bank */
    virtual void run() override;

    HashMap<String, String> tipsBank;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TipsBank)
};



#endif  // TIPSBANK_H_INCLUDED

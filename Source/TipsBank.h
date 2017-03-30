/*
  ==============================================================================

    TipsBank.h
    Created: 30 Mar 2017 8:08:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef TIPSBANK_H_INCLUDED
#define TIPSBANK_H_INCLUDED

/** Usage: create object, then call rebuid() */
class TipsBank : private Thread
{
public:
    TipsBank();
    ~TipsBank();

    void rebuild();
    
private:
    /** build tips bank */
    virtual void run() override;

    HashMap<String, String> tipsBank;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TipsBank)
};



#endif  // TIPSBANK_H_INCLUDED

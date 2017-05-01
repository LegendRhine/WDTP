/*
  ==============================================================================

    TipsBank.h
    Created: 30 Mar 2017 8:08:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef TIPSBANK_H_INCLUDED
#define TIPSBANK_H_INCLUDED

/** Extract the content of 'project/docs/tips.md' and build/process the tips bank.

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

    /** using background thread to rebuild the tips bank */
    void rebuildTipsBank();
    void cleanupTipsBank()     { tipsBank.clear(); }

    const bool isRebuilding() const                         { return isThreadRunning(); }
    const HashMap<String, String>& getTipsBank() const      { return tipsBank; }

    /** nothing would be done and return false if the name (key) has been there already. */
    const bool addNewTip (const String& name, const String& content);

    /** return true if any key of the tips bank contains the para (no need fully ematched) */
    const bool hasThisKey (const String& keyStr) const;
    
private:
    TipsBank();
    virtual void run() override;

    void tipsFromProjectFiles (ValueTree tree);
    HashMap<String, String> tipsBank;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TipsBank)
};



#endif  // TIPSBANK_H_INCLUDED

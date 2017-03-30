/*
  ==============================================================================

    TipsBank.cpp
    Created: 30 Mar 2017 8:08:24pm
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

TipsBank::TipsBank() 
    : Thread ("TipsBankThread")
{

}

//=================================================================================================
TipsBank::~TipsBank()
{
    stopThread (3000);
    clearSingletonInstance();
}

juce_ImplementSingleton (TipsBank);

//=================================================================================================
void TipsBank::rebuildTipsBank()
{
    if (FileTreeContainer::projectTree.isValid() 
        && FileTreeContainer::projectFile.existsAsFile()
        && !isThreadRunning())
        startThread();
}

//=================================================================================================
void TipsBank::run()
{
    tipsBank.clear();
    const File& tipsFile (FileTreeContainer::projectFile.getSiblingFile ("docs").getChildFile ("tips.md"));

    if (tipsFile.existsAsFile())
    {
        StringArray strs;
        strs.addTokens (tipsFile.loadFileAsString(), newLine, String());
        strs.removeEmptyStrings (true);
        strs.trim();

        // note: is more than one of tips has the same key
        // it'll remain the first only
        for (int i = strs.size(); --i >= 0; )
        {
            if (!strs[i].contains ("=")
                || strs[i].substring (0, 1) == "#"
                || strs[i].substring (0, 3) == "***")
            {
                strs.remove (i);
            }
            else
            {
                const String& key (strs[i].upToFirstOccurrenceOf ("=", false, false).trimEnd());
                const String& value (strs[i].fromFirstOccurrenceOf ("=", false, false).trimStart());

                tipsBank.set (key, value);
            }
        }

        /*for (HashMap<String, String>::Iterator i (tipsBank); i.next();)
            DBG (i.getKey() << " -> " << i.getValue());*/
    }
}

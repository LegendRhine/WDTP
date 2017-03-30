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
    const File& tipsFile (FileTreeContainer::projectFile.getChildFile ("docs").getChildFile ("tips.md"));

    if (tipsFile.existsAsFile())
    {
        StringArray strs;
    }
}

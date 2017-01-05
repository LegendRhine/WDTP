/*
  ==============================================================================

    TplTagProcessor.h
    Created: 4 Jan 2017 3:34:15pm
    Author:  miti2000

  ==============================================================================
*/

#ifndef TPLTAGPROCESSOR_H_INCLUDED
#define TPLTAGPROCESSOR_H_INCLUDED

struct TplTagProcessor
{
    /** process {{fileAndDirList}}, its order base on cureent-treeView's order */
    static const String fileAndDirList (const ValueTree& dirTree);


};



#endif  // TPLTAGPROCESSOR_H_INCLUDED

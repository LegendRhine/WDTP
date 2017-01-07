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
    /** process {{fileAndDirList_xx_xx}} 
    
        @pagram dirTree         process this dir's html and dir
        @pagram reverse         true for reverse-date display the items.
        @pagram includeDir      true will display its sub-dir's title
        @pagram extrctIntro     true will display the item's description under its title (like blog)
        @pagram itemsPrePage    0 for unlimit    
    */
    static const String fileAndDirList (const ValueTree& dirTree,
                                        const bool reverse,
                                        const bool includeDir,
                                        const bool extrctIntro,
                                        const int itemsPrePage = 0);

    //=========================================================================
    const int compareElements (const ValueTree& ft, const ValueTree& st);


};



#endif  // TPLTAGPROCESSOR_H_INCLUDED

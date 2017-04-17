/*
  ==============================================================================

    ThemeEditor.h
    Created: 3 Apr 2017 10:57:50am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef THEMEEDITOR_H_INCLUDED
#define THEMEEDITOR_H_INCLUDED

#include "SwingLibrary/SwingEditor.h"

//==============================================================================
/** for edit css, html-tpl file
*/
class ThemeEditor : public SwingEditor,
                    public ChangeListener
{
public:
    ThemeEditor (EditAndPreview* parent);
    ~ThemeEditor();

    void setFileToEdit (const File& file, bool needRegenerate);
    const File& getCurrentFile() const                  { return currentFile; }
    void exitEditMode();

    virtual void addPopupMenuItems (PopupMenu& menuToAddTo, 
                                    const MouseEvent* mouseClickEvent) override;

    virtual void performPopupMenuAction (int menuItemID) override;
    virtual bool keyPressed (const KeyPress& key) override;

    /** set color in real-time when selected a color value. */
    virtual void changeListenerCallback (ChangeBroadcaster* source) override;

private:
    //=============================================================================
    void initializeTags();

    enum 
    { 
        applyIndex = 1, closeIndex, saveAsIndex, 
        searchSth, selectClr 
    };

    EditAndPreview* editAndPreview;
    File currentFile;

    ScopedPointer<ColourSelectorWithPreset> clrSelector;
    StringPairArray tags;
    PopupMenu tagsMenu;
    bool needRegenerate = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeEditor)
};


#endif  // THEMEEDITOR_H_INCLUDED

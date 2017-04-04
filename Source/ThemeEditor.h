/*
  ==============================================================================

    ThemeEditor.h
    Created: 3 Apr 2017 10:57:50am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef THEMEEDITOR_H_INCLUDED
#define THEMEEDITOR_H_INCLUDED

//==============================================================================
/** for edit css, html-tpl file
*/
class ThemeEditor : public TextEditor
{
public:
    ThemeEditor (EditAndPreview* parent);
    ~ThemeEditor();

    void setFileToEdit (const File& file);
    const File& getCurrentFile() const                  { return currentFile; }

    virtual void addPopupMenuItems (PopupMenu& menuToAddTo, 
                                    const MouseEvent* mouseClickEvent) override;

    virtual void performPopupMenuAction (int menuItemID) override;
    virtual bool keyPressed (const KeyPress& key) override;

private:
    enum { applyIndex = 1, closeIndex, saveAsIndex/*, autoReturn*/ };

    EditAndPreview* editAndPreview;
    File currentFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeEditor)
};


#endif  // THEMEEDITOR_H_INCLUDED

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
class ThemeEditor : public Component,
                    public Button::Listener
{
public:
    ThemeEditor (EditAndPreview* parent);
    ~ThemeEditor();

    void paint (Graphics&) override;
    void resized() override;

    void setFileToEdit (const File& file);
    const File& getCurrentFile() const                  { return currentFile; }

    virtual void buttonClicked (Button*) override;

private:
    EditAndPreview* editAndPreview;
    File currentFile;

    CodeDocument codeDoc;
    ScopedPointer<XmlTokeniser> colorToken;
    ScopedPointer<CodeEditorComponent> editor;

    enum { applyBt = 0, closeBt, saveAsBt, totalBts };
    OwnedArray<TextButton> bts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThemeEditor)
};


#endif  // THEMEEDITOR_H_INCLUDED

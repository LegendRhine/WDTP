/*
  ==============================================================================

    MainComponent.cpp
    Created: 4 Sep 2016 11:08:17am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

class TopToolBar;
class FileTreeContainer;
class EditAndPreview;

//==============================================================================
class MainContentComponent   : public Component
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

    FileTreeContainer* getFileTree () const      { return fileTree; }
    TopToolBar* getToolbar() const               { return toolBar; }
    EditAndPreview* getEditAndPreview() const    { return editAndPreview; }

private:
    //=========================================================================
    ScopedPointer<TopToolBar> toolBar;
    ScopedPointer<FileTreeContainer> fileTree;
    ScopedPointer<EditAndPreview> editAndPreview;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StrechableBar> layoutBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

//==============================================================================
/** Main window on desktop */
class MainWindow : public DocumentWindow
{
public:
    MainWindow (String name);
    ~MainWindow() { }

    void closeButtonPressed () override;
    void openProject (const File& projectFile);

private:
    TooltipWindow  toolTips;
    ScopedPointer<MainContentComponent> mainComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};


#endif  // MAINCOMPONENT_H_INCLUDED

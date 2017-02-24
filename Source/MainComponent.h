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
class MainContentComponent : public Component
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

    FileTreeContainer* getFileTree() const         { return fileTree; }
    TopToolBar* getToolbar() const                 { return toolBar; }
    EditAndPreview* getEditAndPreview() const      { return editAndPreview; }

    /** transfer method */
    const bool aDocSelectedCurrently() const;

    /** transfer method */
    void reloadCurrentDoc();

    /** transfer method */
    const bool selectItemFromHtmlFile (const File& htmlFile);

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
    MainWindow (const String& name);
    ~MainWindow();

    void closeButtonPressed() override;
    void openProject (const File& projectFile);

    /** reload the current doc which has been edited from external editor */
    virtual void activeWindowStatusChanged() override;

private:
    ScopedPointer<MainContentComponent> mainComp;
    TooltipWindow toolTips;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};


#endif  // MAINCOMPONENT_H_INCLUDED

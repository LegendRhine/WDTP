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
class MainContentComponent : public Component,
                             public Thread,
                             private Timer
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

    /** transfer method */
    void setSearchInputVisible (const bool shouldVisible);

    void displayFileTree (const bool showFileTree);
    FileTreeContainer* getFileTree() const          { return fileTree; }
    TopToolBar* getToolbar() const                  { return toolBar; }
    EditAndPreview* getEditAndPreview() const       { return editAndPreview; }
    

    /** 3 transfer methods */
    const bool aDocSelectedCurrently() const;
    void reloadCurrentDoc();
    const bool selectItemFromHtmlFile (const File& htmlFile);

    /** these 2 for check the new version and download mp3-encoder if it's not there. */
    virtual void timerCallback() override;
    virtual void run() override;

    /** show some info after launch the app and no project has been loaded  */
    void showStartTip();

private:
    //=========================================================================
    ScopedPointer<TopToolBar> toolBar;
    ScopedPointer<FileTreeContainer> fileTree;
    ScopedPointer<EditAndPreview> editAndPreview;

    StretchableLayoutManager layoutManager;
    ScopedPointer<StrechableBar> layoutBar;
    bool showFileTreePanel;

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

    /** transfer method */
    void setSearchInputVisible (const bool shouldVisible) { mainComp->setSearchInputVisible (shouldVisible); }

    /** reload the current doc which has been edited from external editor */
    virtual void activeWindowStatusChanged() override;

private:
    ScopedPointer<MainContentComponent> mainComp;
    TooltipWindow toolTips;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};


#endif  // MAINCOMPONENT_H_INCLUDED

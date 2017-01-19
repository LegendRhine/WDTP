/*
  ==============================================================================

    UploadComponent.h
    Created: 16 Jan 2017 11:00:42am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef UPLOADCOMPONENT_H_INCLUDED
#define UPLOADCOMPONENT_H_INCLUDED

#include "SwingLibrary/FtpProcessor.h"

//==============================================================================
/**
*/
class UploadComponent    : public Component,
	public TableListBoxModel,
	public Button::Listener,
	public FtpProcessor::Listener
{
public:
    UploadComponent ();
    ~UploadComponent();

    void paint (Graphics&) override;
    void resized() override;

	virtual int getNumRows () override;
	virtual void paintRowBackground (Graphics&, int , int , int , bool ) override;
	virtual void paintCell (Graphics&, int , int , int , int , bool ) override;

	virtual void buttonClicked (Button*) override;
	virtual void transferSuccess (FtpProcessor*) override;
	virtual void transferFailed (FtpProcessor*) override;

	//=================================================================================================
private:
	void loadData (const ValueTree& tree);

	enum { upload = 0, totalBts };

	Array<File> files;
	OwnedArray<TextButton> bts;
	OwnedArray<FtpProcessor> ftps;
	TableListBox table;

	double progressValue = 0.0;
	ProgressBar progressBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UploadComponent)
};


#endif  // UPLOADCOMPONENT_H_INCLUDED

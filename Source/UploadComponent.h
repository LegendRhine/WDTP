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
	void selectRow (const int row, bool selected);

	virtual int getNumRows () override;
	virtual void paintRowBackground (Graphics&, int , int , int , bool ) override;
	virtual void paintCell (Graphics&, int , int , int , int , bool ) override;
	virtual Component* refreshComponentForCell (int , int , bool , Component* ) override;
	virtual String getCellTooltip (int , int ) override;

	virtual void buttonClicked (Button*) override;
	virtual void transferSuccess (FtpProcessor*) override;

	//=================================================================================================
private:
	void loadData (const ValueTree& tree);

	enum { upload = 0, test, totalBts };

	ValueTree filesTree;
	OwnedArray<TextButton> bts;
	TableListBox table;
	Label lb;

	double progressValue = 0.0;
	ProgressBar progressBar;
	ScopedPointer<FtpProcessor> ftp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UploadComponent)
};


#endif  // UPLOADCOMPONENT_H_INCLUDED

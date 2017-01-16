/*
  ==============================================================================

    UploadComponent.h
    Created: 16 Jan 2017 11:00:42am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef UPLOADCOMPONENT_H_INCLUDED
#define UPLOADCOMPONENT_H_INCLUDED


//==============================================================================
/**
*/
class UploadComponent    : public Component,
	public TableListBoxModel,
	public Button::Listener
{
public:
    UploadComponent ();
    ~UploadComponent();

    void paint (Graphics&) override;
    void resized() override;
	
	virtual int getNumRows () override;
	virtual void paintRowBackground (Graphics&, int , int , int , bool ) override;
	virtual void paintCell (Graphics&, int , int , int , int , bool ) override;
	virtual Component* refreshComponentForCell (int , int , bool , Component* ) override;
	virtual String getCellTooltip (int , int ) override;

	virtual void buttonClicked (Button*) override;

	//=================================================================================================
private:
	void loadData (const ValueTree& tree);

	enum { upload = 0, test, totalBts };

	Array<File> files;
	OwnedArray<TextButton> bts;
	TableListBox table;
	Label lb;

	double progressValue = 0.0;
	ProgressBar progressBar;	

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UploadComponent)
};


#endif  // UPLOADCOMPONENT_H_INCLUDED

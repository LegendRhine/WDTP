/*
  ==============================================================================

    UploadComponent.cpp
    Created: 16 Jan 2017 11:00:42am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

//==============================================================================
UploadComponent::UploadComponent () :
	table (String(), this),
	progressBar (progressValue)
{
	loadData (FileTreeContainer::projectTree);
	addAndMakeVisible (table);
	addAndMakeVisible (progressBar);
	progressBar.setColour (ProgressBar::backgroundColourId, Colour (0x00));
	progressBar.setPercentageDisplay (false);

	table.getHeader ().setVisible (false);
	//table.getHeader ().setPopupMenuActive (false);
	addAndMakeVisible (lb);
	lb.setJustificationType (Justification::centred);
	lb.setText (TRANS ("If you don't want to publish a certian file, click the checkbox on the right to unselect it."), 
		dontSendNotification);

	// bts
	for (int i = totalBts; --i >= 0; )
	{
		TextButton* bt = new TextButton ();
		bt->setSize (60, 25);
		bt->addListener (this);
		bts.add (bt);
		addAndMakeVisible (bt);
	}

	bts[upload]->setButtonText (TRANS ("Publish"));
	bts[test]->setButtonText (TRANS ("Test Connection"));
	bts[test]->setSize (150, 25);

	// table
	table.getHeader ().addColumn (TRANS ("File"), 1, 460);
	table.getHeader ().addColumn (String(), 2, 30);

	table.setHeaderHeight (30);
	table.getHeader ().setSortColumnId (1, true);
	table.setMultipleSelectionEnabled (false);
	table.setColour (ListBox::backgroundColourId, Colour (0x00));
	table.updateContent ();

	setSize (510, 360);
}

//=================================================================================================
UploadComponent::~UploadComponent()
{
}

//=================================================================================================
void UploadComponent::loadData (const ValueTree& tree)
{
	if ((bool)tree.getProperty ("needUpload"))
	{
		const File& f (DocTreeViewItem::getHtmlFileOrDir (tree));
		files.add (f);

		DocTreeViewItem::getHtmlMediaFiles (f, files);
	}

	for (int i = 0; i < tree.getNumChildren (); ++i)
		loadData (tree.getChild (i));
}

//=================================================================================================
void UploadComponent::paint (Graphics& g)
{
	g.setColour (Colours::grey.withAlpha (0.6f));
	g.drawHorizontalLine (getHeight () - 35, 0, getWidth () - 0.f);
}

//=================================================================================================
void UploadComponent::resized()
{
	table.setBounds (2, 2, getWidth () - 4, getHeight () - 52);
	progressBar.setBounds (2, getHeight () - 47, getWidth () - 4, 12);
	lb.setBounds (0, 0, getWidth (), 30);

	bts[upload]->setTopRightPosition (getWidth () - 10, getHeight () - 30);
	bts[test]->setTopRightPosition (bts[upload]->getX () - 10, bts[upload]->getY ());
}

//=================================================================================================
int UploadComponent::getNumRows ()
{
	return files.size ();
}

//=================================================================================================
void UploadComponent::paintRowBackground (Graphics& g, 
										int rowNumber, 
										int width, 
										int height, 
										bool rowIsSelected)
{
	if (rowIsSelected)
		g.fillAll (Colours::lightskyblue.withAlpha (0.6f));
	else
		g.fillAll (Colours::grey.withAlpha ((0 == rowNumber % 2) ? 0.15f : 0.0f));
}

//=================================================================================================
void UploadComponent::paintCell (Graphics& g, 
								int rowNumber, 
								int columnId, 
								int width, 
								int height, 
								bool rowIsSelected)
{
	if (1 == columnId)
	{
		g.setColour (Colour (0xff303030));
		g.setFont (SwingUtilities::getFontSize () - 4.0f);

		String text (files[rowNumber].getFullPathName ()
			.replace (FileTreeContainer::projectFile
				.getSiblingFile("site").getFullPathName () + File::separator, String()));

		if (text.getLastCharacters (4) != "html")
			text = "    |-- " + text.fromFirstOccurrenceOf ("media", false, true).substring (1);

		g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);

		g.setColour (Colours::black.withAlpha (0.2f));
		g.fillRect (width - 1, 0, 1, height);
	}
}

//=================================================================================================
Component* UploadComponent::refreshComponentForCell (int rowNumber, 
													int columnId, 
													bool isRowSelected, 
													Component* existingComponentToUpdate)
{
	return nullptr;
}

//=================================================================================================
String UploadComponent::getCellTooltip (int rowNumber, int columnId)
{
	return String ();
}

//=================================================================================================
void UploadComponent::buttonClicked (Button* bt)
{

}


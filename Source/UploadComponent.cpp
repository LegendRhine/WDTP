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
	filesTree ("filesToUpload"),
	table (String(), this),
	progressBar (progressValue)
{
	ValueTree pTree (FileTreeContainer::projectTree);
	loadData (pTree);

	addAndMakeVisible (table);
	addAndMakeVisible (progressBar);
	progressBar.setColour (ProgressBar::backgroundColourId, Colour (0x00));
	progressBar.setPercentageDisplay (false);

	table.getHeader ().setVisible (false);
	addAndMakeVisible (lb);
	lb.setJustificationType (Justification::centred);
	lb.setFont (SwingUtilities::getFontSize () - 3.0f);
	lb.setText (TRANS ("Click the checkbox to unselect the file if you don't want to publish it."), 
		dontSendNotification);

	// bts
	for (int i = totalBts; --i >= 0; )
	{
		TextButton* bt = new TextButton ();
		bt->setSize (70, 25);
		bt->addListener (this);
		bts.add (bt);
		addAndMakeVisible (bt);
	}

	bts[upload]->setButtonText (TRANS ("Publish"));
	bts[test]->setButtonText (TRANS ("Test Connection"));
	bts[test]->setSize (130, 25);

	// table
	table.getHeader ().addColumn (TRANS ("File"), 1, 460);
	table.getHeader ().addColumn (String(), 2, 30);

	table.setHeaderHeight (30);
	table.getHeader ().setSortColumnId (1, true);
	table.setMultipleSelectionEnabled (false);
	table.setColour (ListBox::backgroundColourId, Colour (0x00));
	table.updateContent ();

	// ftp upload
	ftp = new FtpProcessor ();
	ftp->addListener (this);
	ftp->setRemoteRootDir (pTree.getProperty ("ftpAddress").toString());
	ftp->setUserNameAndPassword (pTree.getProperty ("ftpUserName").toString(), 
		pTree.getProperty ("ftpPassword").toString ());

	setSize (512, 360);
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

		ValueTree vf ("file");
		vf.setProperty ("filePath", f.getFullPathName (), nullptr);
		vf.setProperty ("upload", true, nullptr);

		filesTree.addChild (vf, -1, nullptr);

		Array<File> files;
		DocTreeViewItem::getHtmlMediaFiles (f, files);

		for (int i = 0; i < files.size (); ++i)
		{
			ValueTree vm ("file");
			vm.setProperty ("filePath", files[i].getFullPathName (), nullptr);
			vm.setProperty ("upload", true, nullptr);

			filesTree.addChild (vm, -1, nullptr);
		}
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
	return filesTree.getNumChildren();
}

//=================================================================================================
void UploadComponent::paintRowBackground (Graphics& g, 
										int rowNumber, 
										int /*width*/, 
										int /*height*/, 
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
								bool /*rowIsSelected*/)
{
	if (1 == columnId)
	{
		g.setColour (Colour (0xff303030));
		g.setFont (SwingUtilities::getFontSize () - 4.0f);

		String text (filesTree.getChild (rowNumber).getProperty ("filePath").toString ()
			.replace (FileTreeContainer::projectFile
				.getSiblingFile("site").getFullPathName () + File::separator, String()));

		if (text.getLastCharacters (4) != "html")
			text = "      |-- " + text.fromFirstOccurrenceOf ("media", false, true).substring (1);
		else
			text = "- " + text;

		g.drawText (text, 5, 0, width - 10, height, Justification::centredLeft, true);

		g.setColour (Colours::black.withAlpha (0.2f));
		g.fillRect (width - 1, 0, 1, height);
	}
}

//=================================================================================================
void UploadComponent::selectRow (const int row, bool selected)
{
	filesTree.getChild (row).setProperty ("upload", selected, nullptr);
}

//=================================================================================================

class ToggleComp : public Component,
				   public Button::Listener
{
public:
	ToggleComp (UploadComponent& uc) :
		uploadComp (uc)
	{
		addAndMakeVisible (tb);
		tb.setToggleState (true, dontSendNotification);
		tb.addListener (this);
	}

	~ToggleComp ()
	{

	}

	void resized () override
	{
		tb.setBoundsInset (BorderSize<int> (2));
	}

	void setRow (const int newRow)
	{
		row = newRow;
		tb.setToggleState (true, dontSendNotification);
	}
	
	virtual void buttonClicked (Button*) override
	{
		uploadComp.selectRow (row, tb.getToggleState());
	}

private:
	UploadComponent& uploadComp;
	ToggleButton tb;
	int row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleComp)
};

//=================================================================================================
Component* UploadComponent::refreshComponentForCell (int rowNumber, 
													int columnId, 
													bool /*isRowSelected*/, 
													Component* existingComponentToUpdate)
{
	if (2 == columnId)
	{
		ToggleComp* tb = static_cast<ToggleComp*>(existingComponentToUpdate);
		if (tb == nullptr)	tb = new ToggleComp (*this);
		tb->setRow (rowNumber);

		return tb;
	}
	else
	{
		jassert (existingComponentToUpdate == nullptr);
		return nullptr;
	}
}

//=================================================================================================
String UploadComponent::getCellTooltip (int /*rowNumber*/, int /*columnId*/)
{
	return String ();
}

//=================================================================================================
void UploadComponent::buttonClicked (Button* bt)
{
	if (bt == bts[upload])
	{
		for (int i = filesTree.getNumChildren(); --i >= 0; )
		{
			if (!(bool)filesTree.getChild (i).getProperty ("upload"))
			{
				const String& localPath (filesTree.getChild (i).getProperty ("filePath").toString());
				const String& ftpPath (localPath.replace (FileTreeContainer::projectFile
						.getSiblingFile ("site").getFullPathName () + File::separator, String ()));

				ftp->uploadToRemote (File (localPath), ftpPath);

			}
		}
	} 
	else if (bt == bts[test])
	{
		String result;

		if (ftp->connectOk (result))
			SHOW_MESSAGE (TRANS ("Connect successful! "));
		else
			AlertWindow::showMessageBox (AlertWindow::WarningIcon, TRANS ("Connect failed"), result);
	}
}

//=================================================================================================
void UploadComponent::transferSuccess (FtpProcessor* )
{
	table.updateContent ();
}


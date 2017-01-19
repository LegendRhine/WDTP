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
	
	// table
	table.getHeader ().setVisible (false);
	table.getHeader ().addColumn ("File", 1, 435);
	table.setHeaderHeight (1);
	table.setMultipleSelectionEnabled (false);
	table.setColour (ListBox::backgroundColourId, Colour (0x00));
	table.updateContent ();	

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

	setSize (458, 300);
}

//=================================================================================================
UploadComponent::~UploadComponent()
{
	ftps.clear (true);
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

	bts[upload]->setTopRightPosition (getWidth () - 10, getHeight () - 30);
}

//=================================================================================================
int UploadComponent::getNumRows ()
{
	return files.size();
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

		String text (files[rowNumber].getFullPathName ()
			.replace (FileTreeContainer::projectFile
				.getSiblingFile("site").getFullPathName () + File::separator, String()));

		if (text.getLastCharacters (4) != "html")
			text = "      |-- " + text.fromFirstOccurrenceOf ("media", false, true).substring (1);
		else
			text = "- " + text;

		g.drawText (text, 5, 0, width - 10, height, Justification::centredLeft, true);
		/*g.setColour (Colours::black.withAlpha (0.2f));
		g.fillRect (width - 1, 0, 1, height);*/
	}
}

//=================================================================================================
void UploadComponent::buttonClicked (Button* bt)
{
	if (bt == bts[upload])
	{
		ftps.clear (true);

		for (int i = files.size(); --i >= 0; )
		{
			const String& ftpPath (files[i].getFullPathName ()
				.replace (FileTreeContainer::projectFile
				.getSiblingFile ("site").getFullPathName () + File::separator, String ()));

			// ftp upload
			FtpProcessor* ftp = new FtpProcessor ();
			ftp->addListener (this);
			ftp->setRemoteRootDir (FileTreeContainer::projectTree.getProperty ("ftpAddress").toString ());
			ftp->setUserNameAndPassword (FileTreeContainer::projectTree.getProperty ("ftpUserName").toString (),
				FileTreeContainer::projectTree.getProperty ("ftpPassword").toString ());
			
			String result;

			if (ftp->connectOk (result))
			{
				ftp->uploadToRemote (files[i], ftpPath);
				progressValue = ftp->getProgress ();
				ftps.add (ftp);
			}
			else
			{
				AlertWindow::showMessageBox (AlertWindow::WarningIcon, TRANS ("Connect failed"), result);
			}			
		}
	} 	
}

//=================================================================================================
void UploadComponent::transferSuccess (FtpProcessor* f)
{
	for (int i = ftps.size(); --i >= 0; )
	{
		if (ftps[i] == f)
		{
			const File& file (f->getLocalFile ());

			for (int j = files.size (); --j >= 0; )
			{
				if (files[j] == file)
					files.remove (j);
			}

			ftps.remove (i, false);

			MessageManagerLock ml;
			table.updateContent ();

			ValueTree v (DocTreeViewItem::getTreeFromHtmlFile (FileTreeContainer::projectTree, file));

			if (v.isValid ())
				v.setProperty ("needUpload", false, nullptr);
		}
	}	
}

//=================================================================================================
void UploadComponent::transferFailed (FtpProcessor*)
{
}


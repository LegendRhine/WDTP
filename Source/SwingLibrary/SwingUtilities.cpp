/*
  ==============================================================================

    SwingUtilities.cpp
    Created: 6 Sep 2016 10:53:31am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "SwingUtilities.h"

const float SwingUtilities::getFontSize ()
  {
#if JUCE_MAC
    return 23.f;
#else
    return 20.f;
#endif
  }

//=================================================================================================
const String SwingUtilities::getFontName ()
{
#if JUCE_WINDOWS
    return "Microsoft YaHei";
#elif JUCE_MAC
    return "Hiragino Sans GB";
#elif JUCE_LINUX
    return "Droid Sans Fallback";
#else
    return "Heiti SC";  // only for iOS

// Android need to be confirm

#endif
}

//=================================================================================================
void SwingUtilities::showAbout (const String& shortDescription, const String& copyrightYear)
{
    SwingDialog* window = new SwingDialog (shortDescription);

    window->addTextBlock (TRANS("Version: ") + String (ProjectInfo::versionString) + 
                          newLine + newLine +  
                          TRANS ("Author: ") + "SwingCoder" + newLine + 
                          "underwaySoft@126.com" + newLine +
                          "QQ: 843775" + newLine + newLine +
                          "(C) " + copyrightYear + " UnderwaySoft. " + 
                          TRANS ("All Right Reserved.") + newLine);

    Component* linkButton = new HyperlinkButton ("UnderwaySoft", URL ("http://underwaySoft.com"));
    linkButton->setSize (150, 25);
    window->addCustomComponent (linkButton);

    window->setSize (400, 150);
    window->showDialog (TRANS ("About ") + ProjectInfo::projectName);
}

//=================================================================================================
const bool SwingUtilities::writeValueTreeToFile (const ValueTree& treeToWrite, 
                                                 const File& fileToWriteTo,
                                                 const bool usingGZipCompresData/* = false*/)
{
    if (fileToWriteTo.hasWriteAccess ())
    {
        TemporaryFile tempFile (fileToWriteTo);
        FileOutputStream* outputStream (tempFile.getFile ().createOutputStream ());
        jassert (outputStream != nullptr);

        ScopedPointer<OutputStream> out = nullptr;

        if (usingGZipCompresData)
            out = new GZIPCompressorOutputStream (outputStream, 9, true);
        else
            out = outputStream;
        
        if (out != nullptr)
        {
            treeToWrite.writeToStream (*out);
            out->flush ();
            out = nullptr;

            return tempFile.overwriteTargetFileWithTemporary ();
        }
    }

    return false;
}

//=================================================================================================
const ValueTree SwingUtilities::readValueTreeFromFile (const File& fileToReadFrom,
                                                       const bool usingGZipCompresData/* = false*/)
{
    FileInputStream* fileInputStream (fileToReadFrom.createInputStream ());

    if (fileInputStream != nullptr)
    {
        ScopedPointer<InputStream> input = nullptr;
        
        if (usingGZipCompresData)
            input = new GZIPDecompressorInputStream (fileInputStream, true);
        else
            input = fileInputStream;

        if (fileInputStream->openedOk () && input != nullptr)
        {
            return ValueTree::readFromStream (*input);
        }
    }

    return ValueTree::invalid;
}

//=================================================================================================
const String SwingUtilities::getTimeString (const Time& time)
{
    String s;

    s << String (time.getYear ())
        << String (time.getMonth () + 1).paddedLeft ('0', 2)
        << String (time.getDayOfMonth ()).paddedLeft ('0', 2)
        << String (time.getHours ()).paddedLeft ('0', 2)
        << String (time.getMinutes ()).paddedLeft ('0', 2)
        << String (time.getSeconds ()).paddedLeft ('0', 2);

    return s;
}

//=================================================================================================
const String SwingUtilities::getCurrentTimeString (const int plusDays /*= 0*/)
{
    return getTimeString (Time::getCurrentTime () + RelativeTime::days (plusDays));
}

//=================================================================================================
const String SwingUtilities::getTimeStringWithSeparator (const String& dateAndTimeString)
{
    if (!isTimeStringValid (dateAndTimeString))
        return String::empty;

    return dateAndTimeString.substring (0, 4) + "-" +
        dateAndTimeString.substring (4, 6) + "-" +
        dateAndTimeString.substring (6, 8) + " " +
        dateAndTimeString.substring (8, 10) + ":" +
        dateAndTimeString.substring (10, 12) + ":" +
        dateAndTimeString.substring (12, 14);
}

//=================================================================================================
const String SwingUtilities::getDateWeekdayAndTimeString (const String& dateAndTimeString)
{
    if (dateAndTimeString.isEmpty ())
        return String::empty;

    const String& weekday (getTimeFromString (dateAndTimeString).getWeekdayName (true));

    return (dateAndTimeString.substring (0, 4) + "-" +        // year
            dateAndTimeString.substring (4, 6) + "-" +        // month
            dateAndTimeString.substring (6, 8) + " " +        // day
            weekday + " " +                                   // weekday
            dateAndTimeString.substring (8, 10) + ":" +       // hour
            dateAndTimeString.substring (10, 12) + ":" +      // minute
            dateAndTimeString.substring (12));                // second
}

//=================================================================================================
const String SwingUtilities::getWeekdayFromTimeString (const String& dateAndTimeString)
{
    if (dateAndTimeString.isEmpty ())
        return String::empty;

    return TRANS (getTimeFromString (dateAndTimeString).getWeekdayName (true));
}

//=================================================================================================
const bool SwingUtilities::earlyThanCurrentTime (const String& dateAndTimeString)
{
    if (isTimeStringValid (dateAndTimeString))
        return dateAndTimeString <= getCurrentTimeString ();
    else
        return false;
}

//=================================================================================================
const Time SwingUtilities::getTimeFromString (const String& dateAndTimeString)
{
    if (dateAndTimeString.isEmpty ())
        return Time::getCurrentTime ();

    const int year  = dateAndTimeString.substring (0, 4).getIntValue ();
    const int month = dateAndTimeString.substring (4, 6).getIntValue ();
    const int day   = dateAndTimeString.substring (6, 8).getIntValue ();
    const int hour  = dateAndTimeString.substring (8, 10).getIntValue ();
    const int mins  = dateAndTimeString.substring (10, 12).getIntValue ();

    return Time (year, month - 1, day, hour, mins);
}

//=================================================================================================
const bool SwingUtilities::isTimeStringValid (const String& dateAndTimeString)
{
    return (dateAndTimeString.length () == 14 &&
            dateAndTimeString.substring (0, 4).getIntValue () >= 2000 &&
            dateAndTimeString.substring (0, 4).getIntValue () <= 2100 &&
            dateAndTimeString.substring (4, 6).getIntValue () >= 1 &&
            dateAndTimeString.substring (4, 6).getIntValue () <= 12 &&
            dateAndTimeString.substring (6, 8).getIntValue () >= 1 &&
            dateAndTimeString.substring (6, 8).getIntValue () <= 31 &&
            dateAndTimeString.substring (8, 10).getIntValue () >= 0 &&
            dateAndTimeString.substring (8, 10).getIntValue () <= 24 &&
            dateAndTimeString.substring (10, 12).getIntValue () >= 0 &&
            dateAndTimeString.substring (10, 12).getIntValue () <= 59 &&
            dateAndTimeString.substring (12, 14).getIntValue () >= 0 &&
            dateAndTimeString.substring (14, 16).getIntValue () <= 59);
}

//=================================================================================================
const String SwingUtilities::getValidFileName (const String& originalStr)
{
    return originalStr.trim()
        .replaceCharacter ('.', '-').replaceCharacter ('?', '-')
        .replaceCharacter ('*', '-').replaceCharacter ('/', '-')
        .replaceCharacter ('~', '-').replaceCharacter (':', '-')
        .replaceCharacter ('|', '-').replaceCharacter ('<', '-')
        .replaceCharacter ('>', '-').replaceCharacter ('\"', '-')
        .replaceCharacter ('\\', '-').replaceCharacter ('\'', '-');
}

//==============================================================================
SwingDialog::SwingDialog (const String& description) :
    logo (ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize))
{    
    addAndMakeVisible (titleLb = new Label (String::empty, description));
    titleLb->setColour (Label::textColourId, Colour (0xffb85c1a));
    titleLb->setJustificationType (Justification::centred);
    titleLb->setFont (Font (SwingUtilities::getFontSize()));
}
//=========================================================================
SwingDialog::~SwingDialog ()
{
}
//=========================================================================
void SwingDialog::paint (Graphics& g)
{
    g.fillAll (Colour (0xffdcdbdb));
    g.drawImageAt (logo, getWidth () - 165, 2);
}
//=========================================================================
void SwingDialog::resized ()
{
    int h = 5;

    if (titleLb->getText ().isNotEmpty ())
    {
        titleLb->setBounds (10, h, getWidth () - 130, 28);
        h += 35;
    }

    // text blocks
    for (int i = 0; i < blocks.size (); ++i)
    {
        blocks[i]->setBounds (20, h, getWidth () - 40, jmin (200, blocks[i]->getTextHeight () + 10));
        //DBG (blocks[i]->getTextHeight());
        h += blocks[i]->getHeight ();
    }

    // text labels (tips) and editors (inputs)
    for (int i = 0; i < inputs.size (); ++i)
    {
        if (inputLbs[i]->getText ().isNotEmpty ())
        {
            inputLbs[i]->setBounds (15, h, getWidth () - 30, 23);
            inputs[i]->setBounds (15, h + 23, getWidth () - 30, 25);
            h += 53;
        } else
        {
            inputs[i]->setBounds (15, h, getWidth () - 30, 25);
            h += 30;
        }
    }

    // custom component
    if (custComp != nullptr)
    {
        custComp->setBounds (15, h + 5, getWidth () - 30, custComp->getHeight ());
        h += custComp->getHeight () + 10;
    }

    // buttons
    if (bts.size () > 0)
    {
        const int firstBtsX = (getWidth () - bts.size () * 70) / 2 + 5;

        for (int i = 0; i < bts.size (); ++i)
        {
            bts[i]->setSize (60, 25);
            bts[i]->setTopLeftPosition (firstBtsX + i * 70, h + 10);
        }

        h += 35;
    }

    setSize (getWidth (), h + 10);
}
//=================================================================================================
void SwingDialog::addButton (const String& btName, const KeyPress& shortKey)
{
    for (int i = bts.size (); --i >= 0; )
        jassert (btName != bts[i]->getName ());  // mustn't use the same name!

    TextButton* bt = new TextButton (btName);
    bts.add (bt);
    addAndMakeVisible (bt);
    bt->addListener (this);
    bt->addShortcut (shortKey);
}
//=================================================================================================
TextButton* SwingDialog::getButton (const String& btName)
{
    for (int i = bts.size (); --i >= 0; )
    {
        if (btName == bts[i]->getName ())
            return bts[i];
    }

    return nullptr;
}

//=================================================================================================
void SwingDialog::addTextEditor (const String& teName,
                                       const String& content,
                                       const String& tip,
                                       const bool isPw,
                                       const bool readOnly)
{
    for (int i = inputs.size (); --i >= 0; )
        jassert (teName != inputs[i]->getName ());  // mustn't use the same name!

                                                    // label tip
    Label* lb = new Label (String::empty, tip);
    inputLbs.add (lb);
    addAndMakeVisible (lb);
    lb->setFont (Font (SwingUtilities::getFontSize () - 3.0f));
    //lb->setColour (Label::textColourId, Colours::lightgrey);

    // text editor
    InputTextEditor* te = new InputTextEditor (teName, (isPw ? '*' : 0U));
    inputs.add (te);
    addAndMakeVisible (te);
    te->setFont (Font (SwingUtilities::getFontSize () - 1.0f));
    te->setSelectAllWhenFocused (true);
    te->setText (TRANS (content), false);

    if (readOnly)
    {
        te->setReadOnly (true);
        te->setColour (TextEditor::backgroundColourId, Colour (0x00));
        //te->setColour (TextEditor::textColourId, Colours::lightgrey);
        te->applyFontToAllText (te->getFont ());
    }
}
//=================================================================================================
const String SwingDialog::getTextEditorContents (const String& editorName)
{
    for (int i = inputs.size (); --i >= 0; )
    {
        if (editorName == inputs[i]->getName ())
            return inputs[i]->getText ();
    }

    return String::empty;
}
//=================================================================================================
void SwingDialog::buttonClicked (Button*)
{
    DialogWindow* dw = findParentComponentOfClass<DialogWindow> ();

    if (dw != nullptr)
        dw->setVisible (false);
}
//=================================================================================================
void SwingDialog::setFocus ()
{
    if (inputs.size () > 0)
        inputs[0]->grabKeyboardFocus ();
}
//=================================================================================================
TextEditor* SwingDialog::getTextEditor (const String& teName)
{
    for (int i = inputs.size (); --i >= 0; )
    {
        if (teName == inputs[i]->getName ())
            return inputs[i];
    }

    return nullptr;
}
//=================================================================================================
void SwingDialog::addCustomComponent (Component* comp)
{
    custComp = comp;
    addAndMakeVisible (custComp);
}
//=================================================================================================
void SwingDialog::addTextBlock (const String& content)
{
    TextEditor* te = new TextEditor ();
    te->setFont (Font (SwingUtilities::getFontSize () - 3.0f));
    te->setReadOnly (true);
    te->setColour (TextEditor::textColourId, Colour(0xff303030));
    te->setColour (TextEditor::backgroundColourId, Colours::white.withAlpha(0.15f));
    te->setColour (TextEditor::highlightColourId, Colours::lightskyblue);
    //te->setColour (TextEditor::highlightedTextColourId, Colours::black);
    te->setScrollbarsShown (true);
    te->setScrollBarThickness (10);
    te->setMultiLine (true, true);
    te->setCaretVisible (false);
    te->setText (content, false);

    blocks.add (te);
    addAndMakeVisible (te);
}

//=================================================================================================
bool SwingDialog::keyPressed (const KeyPress& key)
{
    for (int i = bts.size (); --i >= 0; )
    {
        if (bts[i]->isRegisteredForShortcut (key))
        {
            bts[i]->triggerClick ();
            return true;
        }
    }

    return Component::keyPressed (key);
}
//=================================================================================================
void SwingDialog::showDialog (const String& windowTitle)
{
    resized ();

    OptionalScopedPointer<Component> comp (this, true);
    DialogWindow::LaunchOptions option;

    option.dialogTitle = windowTitle;
    option.dialogBackgroundColour = Colours::black;
    option.content = comp;
    option.escapeKeyTriggersCloseButton = true;
    option.useNativeTitleBar = true;
    option.resizable = true;
    option.useBottomRightCornerResizer = false;

    option.launchAsync ();
    setFocus ();
}

//=================================================================================================
void WebBrowserComp::newWindowAttemptingToLoad (const String& newURL)
{
    WebBrowserComp* web = new WebBrowserComp ();
    web->setSize (1000, 600);
    web->goToURL (newURL);

    OptionalScopedPointer<Component> comp (web, true);
    DialogWindow::LaunchOptions option;

    option.dialogTitle = newURL;
    option.dialogBackgroundColour = Colours::black;
    option.content = comp;
    option.escapeKeyTriggersCloseButton = true;
    option.useNativeTitleBar = true;
    option.resizable = true;
    option.useBottomRightCornerResizer = false;

    option.launchAsync ();
}


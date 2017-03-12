/*
  ==============================================================================

    SwingUtilities.cpp
    Created: 6 Sep 2016 10:53:31am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "SwingUtilities.h"

//=================================================================================================
const float SwingUtilities::getFontSize()
{
#if JUCE_WINDOWS
    return 20.f;
#elif JUCE_MAC
    return 21.f;
#else
    jassertfalse;
#endif
}

//=================================================================================================
const String SwingUtilities::getFontName()
{
#if JUCE_WINDOWS
    return "Microsoft YaHei";
#elif JUCE_MAC
    //return "Microsoft YaHei";
    return "Lantinghei SC";
    //return "Heiti SC";
    //return "PingFang SC";
    //return "STHeiti";
    //return "Yuppy SC"; // small
    //return "Hiragino Sans GB";
#else
    jassertfalse;
#endif
}

//=================================================================================================
void SwingUtilities::showAbout (const String& shortDescription,
                                const String& copyrightYear)
{
    SwingDialog* window = new SwingDialog (shortDescription);

    window->addTextBlock (TRANS ("Author: ") + "SwingCoder  QQ: 843775" + newLine +
                          "underwaySoft@126.com" + newLine + newLine +
                          String (CharPointer_UTF8 ("\xc2\xa9")) + copyrightYear + " UnderwaySoft " +
                          TRANS ("All Right Reserved.") + newLine  +
                          TRANS (" WDTP is GPL(v3) licensed.") + newLine +
                          "=================================\n" +
                          TRANS ("Acknowledgements:") + newLine +
                          "  - Framework: JUCE (https://juce.com)\n" +
                          //"  - Library: cURL (https://curl.haxx.se)\n" +
                          //"  - Mr. Chen Ping: (http://cpww601.blog.163.com)\n" +
                          "  - Highlight.js (https://highlightjs.org)");

    Component* linkButton = new HyperlinkButton ("UnderwaySoft", URL ("http://underwaySoft.com"));
    linkButton->setSize (150, 25);
    window->addCustomComponent (linkButton);

    window->setSize (420, 160);
    window->showDialog (TRANS ("About ") + ProjectInfo::projectName + " "
                        + String (ProjectInfo::versionString));
}

//=================================================================================================
void SwingUtilities::fixWindowsRegistry()
{
#ifdef JUCE_WINDOWS

    // add a key which indicates our app's WebBroswerComponent will using newer IE web-core
    const String keypath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\Main\\"
                     "FeatureControl\\FEATURE_BROWSER_EMULATION\\";

    // build the full path to the key
    const String key = keypath + JUCEApplication::getInstance()->getApplicationName() + ".exe";

    // this is the value we want
	// see: https://msdn.microsoft.com/en-us/library/ee330730(v=vs.85).aspx
    const uint32 correctValue = 10001;  // IE 10
    bool ok = false;

    // lets look for it anyway
    if (WindowsRegistry::valueExists (key))
    {
        MemoryBlock data;
        unsigned int sz = WindowsRegistry::getBinaryValue (key, data);

        if (sz == 4)             // DWORD
        {
            uint32 val = *(unsigned int*)data.getData();

            if (val == correctValue)
                ok = true;
        }
    }

    if (!ok)
    {
        WindowsRegistry::setValue (key, correctValue);
    }

#endif
}

//=================================================================================================
const bool SwingUtilities::writeValueTreeToFile (const ValueTree& treeToWrite,
                                                 const File& fileToWriteTo,
                                                 const bool usingGZipCompresData/* = false*/)
{
    if (fileToWriteTo.hasWriteAccess())
    {
        TemporaryFile tempFile (fileToWriteTo);
        FileOutputStream* outputStream (tempFile.getFile().createOutputStream());
        jassert (outputStream != nullptr);

        ScopedPointer<OutputStream> out = nullptr;

        if (usingGZipCompresData)
            out = new GZIPCompressorOutputStream (outputStream, 9, true);
        else
            out = outputStream;

        if (out != nullptr)
        {
            treeToWrite.writeToStream (*out);
            out->flush();
            out = nullptr;

            return tempFile.overwriteTargetFileWithTemporary();
        }
    }

    return false;
}

//=================================================================================================
const ValueTree SwingUtilities::readValueTreeFromFile (const File& fileToReadFrom,
                                                       const bool usingGZipCompresData/* = false*/)
{
    FileInputStream* fileInputStream (fileToReadFrom.createInputStream());

    if (fileInputStream != nullptr)
    {
        ScopedPointer<InputStream> input = nullptr;

        if (usingGZipCompresData)
            input = new GZIPDecompressorInputStream (fileInputStream, true);
        else
            input = fileInputStream;

        if (fileInputStream->openedOk() && input != nullptr)
        {
            return ValueTree::readFromStream (*input);
        }
    }

    return ValueTree();
}

//=================================================================================================
const String SwingUtilities::getTimeString (const Time& time)
{
    String s;

    s << String (time.getYear())
        << String (time.getMonth() + 1).paddedLeft ('0', 2)
        << String (time.getDayOfMonth()).paddedLeft ('0', 2)
        << String (time.getHours()).paddedLeft ('0', 2)
        << String (time.getMinutes()).paddedLeft ('0', 2)
        << String (time.getSeconds()).paddedLeft ('0', 2);

    return s;
}

//=================================================================================================
const String SwingUtilities::getCurrentTimeString (const int plusDays /*= 0*/)
{
    return getTimeString (Time::getCurrentTime() + RelativeTime::days (plusDays));
}

//=================================================================================================
const String SwingUtilities::getTimeStringWithSeparator (const String& dateAndTimeString,
                                                         bool includeTime)
{
    if (!isTimeStringValid (dateAndTimeString))
        return String::empty;

    return dateAndTimeString.substring (0, 4) + "." +
        dateAndTimeString.substring (4, 6) + "." +
        dateAndTimeString.substring (6, 8) +
        (includeTime ? " " +
         dateAndTimeString.substring (8, 10) + ":" +
         dateAndTimeString.substring (10, 12) + ":" +
         dateAndTimeString.substring (12, 14) : String());
}

//=================================================================================================
const String SwingUtilities::getDateWeekdayAndTimeString (const String& dateAndTimeString)
{
    if (dateAndTimeString.isEmpty())
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
    if (dateAndTimeString.isEmpty())
        return String::empty;

    return TRANS (getTimeFromString (dateAndTimeString).getWeekdayName (true));
}

//=================================================================================================
const bool SwingUtilities::earlyThanCurrentTime (const String& dateAndTimeString)
{
    if (isTimeStringValid (dateAndTimeString))
        return dateAndTimeString <= getCurrentTimeString();
    else
        return false;
}

//=================================================================================================
const Time SwingUtilities::getTimeFromString (const String& dateAndTimeString)
{
    if (dateAndTimeString.isEmpty())
        return Time::getCurrentTime();

    const int year = dateAndTimeString.substring (0, 4).getIntValue();
    const int month = dateAndTimeString.substring (4, 6).getIntValue();
    const int day = dateAndTimeString.substring (6, 8).getIntValue();
    const int hour = dateAndTimeString.substring (8, 10).getIntValue();
    const int mins = dateAndTimeString.substring (10, 12).getIntValue();

    return Time (year, month - 1, day, hour, mins);
}

//=================================================================================================
const bool SwingUtilities::isTimeStringValid (const String& dateAndTimeString)
{
    return (dateAndTimeString.length() == 14 &&  // 2017 0308 0755 23
            dateAndTimeString.substring (0, 4).getIntValue() >= 2000 &&
            dateAndTimeString.substring (0, 4).getIntValue() <= 2100 &&
            dateAndTimeString.substring (4, 6).getIntValue() >= 1 &&
            dateAndTimeString.substring (4, 6).getIntValue() <= 12 &&
            dateAndTimeString.substring (6, 8).getIntValue() >= 1 &&
            dateAndTimeString.substring (6, 8).getIntValue() <= 31 &&
            dateAndTimeString.substring (8, 10).getIntValue() >= 0 &&
            dateAndTimeString.substring (8, 10).getIntValue() <= 24 &&
            dateAndTimeString.substring (10, 12).getIntValue() >= 0 &&
            dateAndTimeString.substring (10, 12).getIntValue() <= 59 &&
            dateAndTimeString.substring (12, 14).getIntValue() >= 0 &&
            dateAndTimeString.substring (14, 16).getIntValue() <= 59);
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

//=================================================================================================
const String SwingUtilities::doubleToString (const double& seconds)
{
    const int cs = roundDoubleToInt (seconds * 10.0);

    return String (cs / 600).paddedLeft ('0', 2) + ":" +
        String (cs / 10 % 60).paddedLeft ('0', 2) + "." +
        String (cs % 10);
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
SwingDialog::~SwingDialog()
{
}
//=========================================================================
void SwingDialog::paint (Graphics& g)
{
    g.fillAll (Colour (0xffdcdbdb));
    g.drawImageAt (logo, getWidth() - 165, 2);
}
//=========================================================================
void SwingDialog::resized()
{
    int h = 5;

    if (titleLb->getText().isNotEmpty())
    {
        titleLb->setBounds (10, h, getWidth() - 130, 28);
        h += 35;
    }

    // text blocks
    for (int i = 0; i < blocks.size(); ++i)
    {
        blocks[i]->setBounds (20, h, getWidth() - 40, jmin (210, blocks[i]->getTextHeight() + 15));
        //DBG (blocks[i]->getTextHeight());
        h += blocks[i]->getHeight();
    }

    // text labels (tips) and editors (inputs)
    for (int i = 0; i < inputs.size(); ++i)
    {
        if (inputLbs[i]->getText().isNotEmpty())
        {
            inputLbs[i]->setBounds (15, h, getWidth() - 30, 23);
            inputs[i]->setBounds (15, h + 23, getWidth() - 30, 25);
            h += 53;
        }
        else
        {
            inputs[i]->setBounds (15, h, getWidth() - 30, 25);
            h += 30;
        }
    }

    // custom component
    if (custComp != nullptr)
    {
        custComp->setBounds (15, h + 5, getWidth() - 30, custComp->getHeight());
        h += custComp->getHeight() + 10;
    }

    // buttons
    if (bts.size() > 0)
    {
        const int firstBtsX = (getWidth() - bts.size() * 70) / 2 + 5;

        for (int i = 0; i < bts.size(); ++i)
        {
            bts[i]->setSize (60, 25);
            bts[i]->setTopLeftPosition (firstBtsX + i * 70, h + 10);
        }

        h += 35;
    }

    setSize (getWidth(), h + 15);
}
//=================================================================================================
void SwingDialog::addButton (const String& btName, const KeyPress& shortKey)
{
    for (int i = bts.size(); --i >= 0; )
        jassert (btName != bts[i]->getName());  // mustn't use the same name!

    TextButton* bt = new TextButton (btName);
    bts.add (bt);
    addAndMakeVisible (bt);
    bt->addListener (this);
    bt->addShortcut (shortKey);
}
//=================================================================================================
TextButton* SwingDialog::getButton (const String& btName)
{
    for (int i = bts.size(); --i >= 0; )
    {
        if (btName == bts[i]->getName())
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
    for (int i = inputs.size(); --i >= 0; )
        jassert (teName != inputs[i]->getName());  // mustn't use the same name!

    // label 
    Label* lb = new Label (String::empty, tip);
    inputLbs.add (lb);
    addAndMakeVisible (lb);
    lb->setFont (Font (SwingUtilities::getFontSize() - 3.0f));
    //lb->setColour (Label::textColourId, Colours::lightgrey);

    // text editor
    InputTextEditor* te = new InputTextEditor (teName, (isPw ? '*' : 0U));
    inputs.add (te);
    addAndMakeVisible (te);
    te->setFont (Font (SwingUtilities::getFontSize() - 1.0f));
    te->setSelectAllWhenFocused (true);
    te->setText (TRANS (content), false);

    if (readOnly)
    {
        te->setReadOnly (true);
        te->setColour (TextEditor::backgroundColourId, Colour (0x00));
        //te->setColour (TextEditor::textColourId, Colours::lightgrey);
        te->applyFontToAllText (te->getFont());
    }
}
//=================================================================================================
const String SwingDialog::getTextEditorContents (const String& editorName)
{
    for (int i = inputs.size(); --i >= 0; )
    {
        if (editorName == inputs[i]->getName())
            return inputs[i]->getText();
    }

    return String::empty;
}
//=================================================================================================
void SwingDialog::buttonClicked (Button*)
{
    DialogWindow* dw = findParentComponentOfClass<DialogWindow>();

    if (dw != nullptr)
        dw->setVisible (false);
}
//=================================================================================================
void SwingDialog::setFocus()
{
    if (inputs.size() > 0)
        inputs[0]->grabKeyboardFocus();
}
//=================================================================================================
TextEditor* SwingDialog::getTextEditor (const String& teName)
{
    for (int i = inputs.size(); --i >= 0; )
    {
        if (teName == inputs[i]->getName())
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
    TextEditor* te = new TextEditor();
    te->setFont (Font (SwingUtilities::getFontSize() - 3.0f));
    te->setReadOnly (true);
    te->setColour (TextEditor::textColourId, Colour (0xff303030));
    te->setColour (TextEditor::backgroundColourId, Colour (0x00));
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
    for (int i = bts.size(); --i >= 0; )
    {
        if (bts[i]->isRegisteredForShortcut (key))
        {
            bts[i]->triggerClick();
            return true;
        }
    }

    return Component::keyPressed (key);
}
//=================================================================================================
void SwingDialog::showDialog (const String& windowTitle)
{
    resized();

    OptionalScopedPointer<Component> comp (this, true);
    DialogWindow::LaunchOptions option;

    option.dialogTitle = windowTitle;
    option.dialogBackgroundColour = Colours::black;
    option.content = comp;
    option.escapeKeyTriggersCloseButton = true;
    option.useNativeTitleBar = true;
    option.resizable = false;
    option.useBottomRightCornerResizer = false;

    option.launchAsync();
    setFocus();
}

//=================================================================================================
/** static arry */
Array<Colour> ColourSelectorWithPreset::colours;

//=================================================================================================
ColourSelectorWithPreset::ColourSelectorWithPreset()
{
    if (colours.size() <= 0)
        setPresetDefaultColour();
}
//=================================================================================================
void ColourSelectorWithPreset::setSwatchColour (int index,
                                                const Colour& newColour) const
{
    colours.set (index, newColour);
}
//=================================================================================================
void ColourSelectorWithPreset::setPresetDefaultColour()
{
    colours.add (Colour (0xFF000000));
    colours.add (Colour (0xFFFFFFFF));
    colours.add (Colour (0xFFFF0000));
    colours.add (Colour (0xFF00FF00));
    colours.add (Colour (0xFF0000FF));
    colours.add (Colour (0xFFFFFF00));
    colours.add (Colour (0xFFFF00FF));
    colours.add (Colour (0xFF00FFFF));

    colours.add (Colours::burlywood);
    colours.add (Colours::cyan);
    colours.add (Colours::darkcyan);
    colours.add (Colours::darkgreen);
    colours.add (Colours::darkgrey);
    colours.add (Colours::darkorange);
    colours.add (Colours::fuchsia);
    colours.add (Colours::gold);

    colours.add (Colours::grey);
    colours.add (Colours::lightseagreen);
    colours.add (Colours::lightsteelblue);
    colours.add (Colours::mediumblue);
    colours.add (Colours::mediumseagreen);
    colours.add (Colours::olive);
    colours.add (Colours::purple);
    colours.add (Colours::slategrey);
}


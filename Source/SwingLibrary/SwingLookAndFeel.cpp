/*
  ==============================================================================

    SwingLookAndFeel.cpp
    Created: 6 Sep 2016 10:52:26am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "SwingUtilities.h"
#include "SwingLookAndFeel.h"

//=================================================================================================
SwingLookAndFeel::SwingLookAndFeel ()
{
    setDefaultSansSerifTypefaceName (SwingUtilities::getFontName());
}

//=================================================================================================
TextLayout SwingLookAndFeel::layoutTooltipText (const String& text, Colour colour) noexcept
{
    const float tooltipFontSize = SwingUtilities::getFontSize () - 3.0f;
    const int maxToolTipWidth = 500;

    AttributedString s;
    s.setJustification (Justification::centred);
    s.append (text, Font (tooltipFontSize), colour);

    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
    return tl;
}

//=================================================================================================
Rectangle<int> SwingLookAndFeel::getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea)
{
    const TextLayout tl (layoutTooltipText (tipText, Colours::black));

    const int w = (int) (tl.getWidth () + 20.0f);
    const int h = (int) (tl.getHeight () + 10.0f);

    return Rectangle<int> (screenPos.x > parentArea.getCentreX () ?
                           screenPos.x - (w + 12) : screenPos.x + 24,
                           screenPos.y > parentArea.getCentreY () ?
                           screenPos.y - (h + 6) : screenPos.y + 6,
                           w, h).constrainedWithin (parentArea);
}

//=================================================================================================
void SwingLookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    g.fillAll (findColour (TooltipWindow::backgroundColourId));

#if ! JUCE_MAC // The mac windows already have a non-optional 1 pix outline, so don't double it here..
    g.setColour (findColour (TooltipWindow::outlineColourId));
    g.drawRect (0, 0, width, height, 1);
#endif

    layoutTooltipText (text, findColour (TooltipWindow::textColourId))
        .draw (g, Rectangle<float> ((float) width, (float) height));
}

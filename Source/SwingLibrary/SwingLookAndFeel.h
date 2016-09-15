/*
  ==============================================================================

    SwingLookAndFeel.h
    Created: 6 Sep 2016 10:52:26am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef SWINGLOOKANDFEEL_H_INCLUDED
#define SWINGLOOKANDFEEL_H_INCLUDED

class SwingLookAndFeel : public LookAndFeel_V3
{
public:
    SwingLookAndFeel ();
    ~SwingLookAndFeel ()    { }    

    /** Tooltip... */
    static TextLayout layoutTooltipText (const String& text, Colour colour) noexcept;
    Rectangle<int> getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea);
    void drawTooltip (Graphics& g, const String& text, int width, int height);

};



#endif  // SWINGLOOKANDFEEL_H_INCLUDED
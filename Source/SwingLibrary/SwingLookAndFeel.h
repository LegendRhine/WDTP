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
    SwingLookAndFeel();
    ~SwingLookAndFeel()    { }    

    /** Tooltip... */
    static TextLayout layoutTooltipText (const String& text,
                                         Colour colour) noexcept;
    
    Rectangle<int> getTooltipBounds (const String& tipText,
                                     Point<int> screenPos,
                                     Rectangle<int> parentArea) override;
    
    void drawTooltip (Graphics& g, const String& text,
                      int width, int height) override;
    
    /** PropertyPanel's header text */
     void drawPropertyPanelSectionHeader (Graphics& g, const String& name,
                                          bool isOpen, int width, int height) override;
    
    /** popup menu */
    virtual Font getPopupMenuFont() override
    {
        return SwingUtilities::getFontSize() - 2.f;
    }
    
};

#endif  // SWINGLOOKANDFEEL_H_INCLUDED

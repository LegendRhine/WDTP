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
    virtual Font getPopupMenuFont() override;
    /*virtual void drawPopupMenuItem (Graphics& g, const Rectangle<int>& area,
                                    bool isSeparator, bool isActive,
                                    bool isHighlighted, bool isTicked,
                                    bool hasSubMenu, const String& text,
                                    const String& shortcutKeyText,
                                    const Drawable *icon, const Colour *textColour) override;*/
    
    /** alertWindow and textButton */
    virtual Font getAlertWindowTitleFont() override;
    virtual Font getAlertWindowMessageFont() override;
    virtual Font getTextButtonFont (TextButton& bt, int buttonHeight) override;
    virtual Font getAlertWindowFont() override;    
    
};

#endif  // SWINGLOOKANDFEEL_H_INCLUDED

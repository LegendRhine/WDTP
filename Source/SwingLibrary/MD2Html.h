/*
  ==============================================================================

    MD2Html.h
    Created: 4 Sep 2016 12:29:29am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef MD2HTML_H_INCLUDED
#define MD2HTML_H_INCLUDED

class Md2Html
{
public:
    /** Base on the argu Markdown string and template file, return html string. */
    static const String mdStringToHtml (const String& mdString, const File& tplFile);

private:
    // call these methods must be ordered just like below order
    static const String boldParse (const String& mdString);
    static const String italicParse (const String& mdString);
    static const String headingThereParse (const String& mdString);
    static const String headingTwoParse (const String& mdString);
    static const String headingOneParse (const String& mdString);

    /** http://xxx.com  open in new tab/window, must start with: " http" and end with " "   */
    static const String spaceLinkParse (const String& mdString);
    static const String imageParse (const String& mdString);  /**< ![](media/xxx.jpg) */
    static const String mdLinkParse (const String& mdString); /**< [](http://xxx.com) */

    static const String unorderedListParse (const String& mdString); /**< -  */
    static const String orderedListParse (const String& mdString); /**< +  */

    /** ---  Must at least 3 "-" and place in a single line */
    static const String hrParse (const String& mdString);
    static const String inlineCodeParse (const String& mdString);
    static const String codeBlockParse (const String& mdString);
    static const String alignCenterParse (const String& mdString);
    static const String alignRightParse (const String& mdString);
    static const String tableParse (const String& mdString);
    static const String newLineParse (const String& mdString);

};



#endif  // MD2HTML_H_INCLUDED

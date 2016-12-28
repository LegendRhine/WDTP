/*
  ==============================================================================

    MD2Html.h
    Created: 4 Sep 2016 12:29:29am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef MD2HTML_H_INCLUDED
#define MD2HTML_H_INCLUDED

struct Md2Html
{
public:
    static const String mdStringToHtml (const String& mdString, const ValueTree& docTree);
    static const bool generateHtmlFile (const String& htmlString, const File& tplFile);

private:
    // call these methods must be ordered just like below order
    static const String boldParse (const String& mdString);
    static const String italicParse (const String& mdString);
    static const String headingThereParse (const String& mdString);
    static const String headingTwoParse (const String& mdString);
    static const String headingOneParse (const String& mdString);

    /**  http://xxx.com  open in new tab/window, 
        must start with: " http" and end with " "   */
    static const String spaceLinkParse (const String& mdString); 
    static const String imageParse (const String& mdString); /**< ![](media/xxx.jpg) */
    static const String mdLinkParse (const String& mdString); /**< [](http://xxx.com) */
    static const String unorderedListParse (const String& mdString); /**< -  */
    static const String orderedListParse (const String& mdString); /**< +  */
    static const String hrParse (const String& mdString); /**< ---  */
    static const String inlineCodeParse (const String& mdString);
    static const String codeBlockParse (const String& mdString);
    static const String alignCenterParse (const String& mdString);
    static const String alignRightParse (const String& mdString);
    static const String tableParse (const String& mdString);
    static const String newLineParse (const String& mdString);

};



#endif  // MD2HTML_H_INCLUDED

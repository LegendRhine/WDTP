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
    static const String mdStringToHtml (const String& mdString, const ValueTree& docTree);
    static const bool generateHtmlFile (const String& htmlString, const File& htmlFile);

};



#endif  // MD2HTML_H_INCLUDED

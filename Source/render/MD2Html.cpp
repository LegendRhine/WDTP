/*
  ==============================================================================

    MD2Html.cpp
    Created: 4 Sep 2016 12:29:29am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "MD2Html.h"

//=================================================================================================

//=================================================================================================
const String Md2Html::mdStringToHtml (const String& mdString,
                                      const ValueTree& docTree)
{
    const String& content = mdString.replace (newLine + newLine, "<p>")
        .replace(newLine, "<br>");

    String s;
    s << "<!doctype html>" << newLine <<
        "<html lang=\"en\">" << newLine <<
        " <head>" << newLine <<
        "  <meta charset=\"UTF-8\">" << newLine <<
        "  <meta name=\"Generator\" content=\"WDTP\">" << newLine <<
        "  <meta name=\"Author\" content=\"\">" << newLine <<
        "  <meta name=\"Keywords\" content=\"\">" << newLine <<
        "  <meta name=\"Description\" content=\"\">" << newLine <<
        "  <title>Document</title>" << newLine <<
        " </head>" << newLine <<
        " <body>" << newLine <<
        content << newLine <<
        "</body>" << newLine <<
        "</html>";

    return s;
}

//=================================================================================================
const bool Md2Html::generateHtmlFile (const String& htmlString, const File& htmlFile)
{

    return true;
}

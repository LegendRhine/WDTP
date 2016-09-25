/*
  ==============================================================================

    MD2Html.cpp
    Created: 4 Sep 2016 12:29:29am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "MD2Html.h"
#include <sstream>
#include "markdown.h"

//=================================================================================================
const String Md2Html::mdStringToHtml (const String& mdString,
                                      const ValueTree& docTree)
{
    /*const String& content = mdString.replace (newLine + newLine, "<p>")
        .replace(newLine, "<br>");*/
    markdown::Document mdRender;
    mdRender.read (mdString.toStdString ());

    std::stringstream stream;
    stream.clear ();
    std::string stdContent;

    mdRender.write (stream);
    stream.flush ();
    
    String content (stream.str());
    //DBG (content);

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

    stream.clear ();

    return s;
}

//=================================================================================================
const bool Md2Html::generateHtmlFile (const String& htmlString, const File& htmlFile)
{

    return true;
}

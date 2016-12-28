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
const String Md2Html::mdStringToHtml (const String& mdString, 
                                      const File& tplFile)
{
    String content (mdString);

    content = boldParse (content);
    content = italicParse (content);
    content = headingThereParse (content);
    content = headingTwoParse (content);
    content = headingOneParse (content);
    content = spaceLinkParse (content);
    content = imageParse (content);
    content = mdLinkParse (content);
    content = hrParse (content);

    content = newLineParse (content);

    //DBG (content);

    String s;
    s << "<!doctype html>" << newLine <<
        "<html lang=\"cn\">" << newLine <<
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
const String Md2Html::boldParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "**");

    for (int i = 1; index != -1; ++i)
    {
        if (resultStr.substring (index - 1, index) != "\\"
            && resultStr.substring (index - 1, index) != "/"
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 2, index + 3) != "*"
            && resultStr.substring (index + 2, index + 3) != "/"
            && resultStr.substring (index + 1, index + 2) != "<")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 2, "<strong>");
            else
                resultStr = resultStr.replaceSection (index, 2, "</strong>");
        }
        else
        {
            --i;
        }

        index = resultStr.indexOfIgnoreCase (index + 2, "**");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::italicParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "*");

    for (int i = 1; index != -1; ++i)
    {
        if (resultStr.substring (index - 1, index) != "\\"
            && resultStr.substring (index - 1, index) != "/"
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 1, index + 2) != "*"
            && resultStr.substring (index + 1, index + 2) != "/"
            && resultStr.substring (index + 1, index + 2) != "<")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 1, "<em>");
            else
                resultStr = resultStr.replaceSection (index, 1, "</em>");
        }
        else
        {
            --i;
        }

        index = resultStr.indexOfIgnoreCase (index + 1, "*");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::headingThereParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "### ");

    while (index != -1)
    {
        if (resultStr.substring (index - 1, index) != "\\")
        {
            resultStr = resultStr.replaceSection (index, 4, "<h3>");

            const int lineEndIndex = resultStr.indexOfIgnoreCase (index + 4, newLine);

            if (lineEndIndex != -1)
                resultStr = resultStr.replaceSection (lineEndIndex, 2, "</h3>\n");
        }

        index = resultStr.indexOfIgnoreCase (index + 5, "### ");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::headingTwoParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "## ");

    while (index != -1)
    {
        if (resultStr.substring (index - 1, index) != "\\")
        {
            resultStr = resultStr.replaceSection (index, 3, "<h2>");

            const int lineEndIndex = resultStr.indexOfIgnoreCase (index + 4, newLine);

            if (lineEndIndex != -1)
                resultStr = resultStr.replaceSection (lineEndIndex, 2, "</h2>\n");
        }

        index = resultStr.indexOfIgnoreCase (index + 5, "## ");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::headingOneParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "# ");

    while (index != -1)
    {
        if (resultStr.substring (index - 1, index) != "\\")
        {
            resultStr = resultStr.replaceSection (index, 2, "<div align=center><h1>");

            const int lineEndIndex = resultStr.indexOfIgnoreCase (index + 22, newLine);

            if (lineEndIndex != -1)
                resultStr = resultStr.replaceSection (lineEndIndex, 2, "</h1></div>\n");
        }

        index = resultStr.indexOfIgnoreCase (index + 12, "# ");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::spaceLinkParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, " http");

    while (indexStart != -1)
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 10, " ");

        if (indexEnd == -1)
            break;

        const String linkAddress (resultStr.substring (indexStart + 1, indexEnd));
        const String linkStr (" <a href=\"" + linkAddress + "\" target=\"_blank\">" + linkAddress + "</a>");

        //DBG (linkAddress);
        if (!linkAddress.contains (newLine) && linkAddress.containsNonWhitespaceChars())
            resultStr = resultStr.replaceSection (indexStart, linkAddress.length () + 1, linkStr);
        
        indexStart = resultStr.indexOfIgnoreCase (indexStart + linkAddress.length(), " http");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::imageParse (const String& mdString)
{
    /**< ![](media/xxx.jpg) */
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "![");

    while (indexStart != -1)
    {
        // get alt content
        const int altEnd = resultStr.indexOfIgnoreCase (indexStart + 2, "](");
        if (altEnd == -1)            break;        
        const String altContent (resultStr.substring (indexStart + 2, altEnd));

        // get img path
        const int imgEnd = resultStr.indexOfIgnoreCase (altEnd + 2, ")");
        if (imgEnd == -1)            break;
        const String imgPath (resultStr.substring (altEnd + 2, imgEnd));

        const String imgStr ("<div align=center><img src=\"" + imgPath + "\" alt=\"" 
                             + altContent + "\" />  <br>" + TRANS ("Illustration: ") 
                             + altContent + "</div>");

        resultStr = resultStr.replaceSection (indexStart, imgEnd + 1 - indexStart, imgStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + imgStr.length (), "![");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::mdLinkParse (const String& mdString)
{
    // [](http://xxx.com)
    String resultStr (mdString);
    int midStart = resultStr.indexOfIgnoreCase (0, "](");

    while (midStart != -1)
    {
        // get alt content
        const String splitContent (resultStr.substring (0, midStart));
        const int altStart = splitContent.lastIndexOfIgnoreCase ("[");
        if (altStart == -1)            break;
        const String altContent (resultStr.substring (altStart + 1, midStart));

        // get link path
        const int pathEnd = resultStr.indexOfIgnoreCase (midStart + 2, ")");
        if (pathEnd == -1)            break;

        const String linkPath (resultStr.substring (midStart + 2, pathEnd));
        const String linkStr ("<a href=\"" + linkPath + "\">" + altContent + "</a>");

        resultStr = resultStr.replaceSection (altStart, pathEnd + 1 - altStart, linkStr);
        midStart = resultStr.indexOfIgnoreCase (midStart + linkStr.length (), "](");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::hrParse (const String& mdString)
{
    StringArray contentByLine;
    contentByLine.addLines (mdString);

    for (int i = contentByLine.size(); --i >= 0; )
    {
        const String currentLine (contentByLine.getReference (i));

        if ('-' == currentLine[0] && '-' == currentLine.getLastCharacter () 
            && currentLine.contains ("---"))
            contentByLine.getReference (i) = "<hr>\n";
    }

    return contentByLine.joinIntoString (newLine);
}

//=================================================================================================
const String Md2Html::unorderedListParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::orderedListParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::inlineCodeParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::codeBlockParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::alignCenterParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::alignRightParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::tableParse (const String& mdString)
{
    String resultStr (mdString);

    return resultStr;
}

//=================================================================================================
const String Md2Html::newLineParse (const String& mdString)
{
    String resultStr (mdString.replace (newLine + newLine, "<p>\n")
                      .replace (newLine, "<br>\n"));

    int index = resultStr.indexOfIgnoreCase (0, "<br>");

    while (index != -1)
    {
        if (resultStr.substring (index - 2, index).contains (">"))
            resultStr = resultStr.replaceSection (index, 5, String());

        index = resultStr.indexOfIgnoreCase (index + 4, "<br>");
    }

    //DBG (resultStr);
    return resultStr;
}

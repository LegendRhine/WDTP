/*
  ==============================================================================

    MD2Html.cpp
    Created: 4 Sep 2016 12:29:29am
    Author:  SwingCoder

  ==============================================================================
*/

#include "JuceHeader.h"
#include "../HtmlProcessor.h"
#include "MD2Html.h"

//=================================================================================================
const String Md2Html::mdStringToHtml (const String& mdString)
{
    if (mdString.isEmpty ())
        return String ();

    // parse markdown, must followed by these order
    String htmlContent (mdString);
    htmlContent = tableParse (htmlContent);
    htmlContent = codeBlockParse (htmlContent);
    htmlContent = inlineCodeParse (htmlContent);
    htmlContent = boldAndItalicParse (htmlContent);
    htmlContent = boldParse (htmlContent);
    htmlContent = italicParse (htmlContent);
    htmlContent = highlightParse (htmlContent);
    htmlContent = tocParse (htmlContent);
    htmlContent = processByLine (htmlContent);
    htmlContent = spaceLinkParse (htmlContent);
    htmlContent = imageParse (htmlContent);
    htmlContent = mdLinkParse (htmlContent);
    htmlContent = orderedListParse (htmlContent, true);
    htmlContent = orderedListParse (htmlContent, false);
    htmlContent = cleanUp (htmlContent);

    //DBG (htmlContent);
    return htmlContent;
}

//=================================================================================================
const String Md2Html::tableParse (const String& mdString)
{
    if (mdString.isEmpty ())
        return mdString;

    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__table@MDtag@Parse__%%");
    contentByLine.add ("%%__table@MDtag@Parse__%%");

    for (int i = 1; i < contentByLine.size () - 1; ++i)
    {
        String& prevLine = contentByLine.getReference (i - 1);
        String& currentLine (contentByLine.getReference (i));
        String& nextLine = contentByLine.getReference (i + 1);

        if (currentLine.substring (0, 6) == "------"
            && prevLine.contains (" | ")
            && nextLine.contains (" | "))
        {
            currentLine = prevLine;
            prevLine = "<table>";

            currentLine = "<tr><th>" + currentLine.replace (" | ", "</th><th>") + "</th></tr>";
            nextLine = "<tr><td>" + nextLine.replace (" | ", "</td><td>") + "</td></tr>";

            int rowNums = i + 1;

            while (++rowNums < contentByLine.size ())
            {
                String& thisLine = contentByLine.getReference (rowNums);

                if (thisLine.contains (" | "))
                {
                    thisLine = "<tr><td>" + thisLine.replace (" | ", "</td><td>") + "</td></tr>";
                }
                else
                {
                    contentByLine.insert (rowNums, "</table>");
                    break;
                }
            }

            i = rowNums;
        }
    }

    contentByLine.removeString ("%%__table@MDtag@Parse__%%");
    return contentByLine.joinIntoString (newLine);
}

//=================================================================================================
const String Md2Html::codeBlockParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "```");

    while (indexStart != -1)
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 4, "```");

        if (indexEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexStart, indexEnd + 3));
        const String htmlStr ("<pre><code>"
                              + mdCode.replace ("```", String ())
                              .replace ("*", "_%5x|z%!##!_") // see cleanup(), prevent bold and italic parse it
                              + "</code></pre>");

        //DBG (htmlStr);
        resultStr = resultStr.replaceSection (indexStart, mdCode.length (), htmlStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + htmlStr.length (), "```");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::inlineCodeParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "`");

    for (int i = 1; index != -1; ++i)
    {
        if (resultStr.substring (index - 1, index) != "\\"
            && resultStr.substring (index - 1, index) != "/"
            && resultStr.substring (index - 1, index) != "`"
            && resultStr.substring (index + 1, index + 2) != "`"
            && resultStr.substring (index + 1, index + 2) != "/"
            && resultStr.substring (index + 1, index + 2) != "<")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 1, "<code>");
            else
                resultStr = resultStr.replaceSection (index, 1, "</code>");
        }
        else
        {
            --i;
        }

        index = resultStr.indexOfIgnoreCase (index + 1, "`");
    }

    // for bold and italic parse (replace '*' to prevent it'll be parsed)
    int indexStart = resultStr.indexOfIgnoreCase (0, "<code>");

    while (indexStart != -1)
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 6, "</code>");

        if (indexEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexStart, indexEnd).replace ("*", "_%5x|z%!##!_"));
        resultStr = resultStr.replaceSection (indexStart, indexEnd - indexStart, mdCode);

        indexStart = resultStr.indexOfIgnoreCase (indexStart + mdCode.length (), "<code>");
    }

    //DBG (resultStr);
    return resultStr;    
}

//=================================================================================================
const String Md2Html::boldAndItalicParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "***");

    for (int i = 1; index != -1; ++i)
    {
        if (resultStr.substring (index - 1, index) != "\\"
            && resultStr.substring (index - 1, index) != "/"
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 3, index + 4) != "*"
            && resultStr.substring (index + 3, index + 4) != "/"
            && resultStr.substring (index + 4, index + 4) != "<")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 2, "<em><strong>");
            else
                resultStr = resultStr.replaceSection (index, 2, "</strong></em>");
        }
        else
        {
            --i;
        }

        index = resultStr.indexOfIgnoreCase (index + 3, "***");
    }

    //DBG (resultStr);
    return resultStr;
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
            && resultStr.substring (index + 2, index + 3) != "<")
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
const String Md2Html::highlightParse (const String& mdString)
{
    String resultStr (mdString);
    int index = resultStr.indexOfIgnoreCase (0, "~~");

    for (int i = 1; index != -1; ++i)
    {
        if (resultStr.substring (index - 1, index) != "\\"
            && resultStr.substring (index - 1, index) != "/"
            && resultStr.substring (index - 1, index) != "~"
            && resultStr.substring (index + 2, index + 3) != "~"
            && resultStr.substring (index + 2, index + 3) != "/"
            && resultStr.substring (index + 1, index + 2) != "<")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 2, "<span style=\"background: #bbdddd\">");
            else
                resultStr = resultStr.replaceSection (index, 2, "</span>");
        }
        else
        {
            --i;
        }

        index = resultStr.indexOfIgnoreCase (index + 2, "~~");
    }

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const String Md2Html::tocParse (const String& mdString)
{
    String resultStr (mdString);

    if (!resultStr.contains ("[TOC]"))
        return resultStr;

    if (!(resultStr.contains ("# ") || resultStr.contains ("## ") || resultStr.contains ("### ")))
        return resultStr;

    // escape '\[TOC]'
    resultStr = resultStr.replace ("\\[TOC]", "_@_tocParseReplaceStr_@_");
    
    // get lines which include '#'s
    StringArray lines;
    lines.addLines (resultStr);
    
    for (int i = lines.size(); --i >= 0; )
    {
        if (lines[i].trimStart ().substring (0, 1) != "#")
            lines.remove (i);
    }

    // doesn't extrct the title (h1) -- for article toc
    // but when export a big-single html, it'll extrct all
    lines.remove (0);

    // process h1, h2 and h3
    for (int i = lines.size(); --i >= 0; )
    {
        if (lines[i].substring (0, 2) == "# ")
            lines.getReference (i) = "<a href=\"#" + lines[i].substring (2) + "\">"
            + lines[i].substring (2) + "</a><br>";

        else if (lines[i].substring (0, 3) == "## ")
            lines.getReference (i) = " &emsp;&emsp;"
            + String (CharPointer_UTF8 ("\xc2\xb7"))
            + " <a href=\"#" + lines[i].substring (3) + "\">"
            + lines[i].substring (3) + "</a><br>";

        else if (lines[i].substring (0, 4) == "### ")
            lines.getReference (i) = " &emsp;&emsp;&emsp;&emsp;"
            + String (CharPointer_UTF8 ("\xc2\xb7"))
            + " <a href=\"#"
            + lines[i].substring (4) + "\">"
            + lines[i].substring (4) + "</a><br>";

        else
            lines.remove (i);
    }

    const String tocContent ("<div class=toc>" + lines.joinIntoString (newLine) + "</div>");
    resultStr = resultStr.replace ("[TOC]", tocContent);
    resultStr = resultStr.replace ("_@_tocParseReplaceStr_@_", "[TOC]");
    
    return resultStr;
}

//=================================================================================================
const String Md2Html::processByLine (const String& mdString)
{
    StringArray contentByLine;
    contentByLine.addLines (mdString);

    for (int i = contentByLine.size (); --i >= 0; )
    {
        String& currentLine (contentByLine.getReference (i));

        // <hr>
        if (currentLine.trimStart ().substring (0, 3) == "---")
            currentLine = "<hr>";

        // <blockquote>
        else if (currentLine.trimStart ().substring (0, 2) == "> ")
            currentLine = "<blockquote>" + currentLine.trimStart ().substring (2) + "</blockquote>";

        // <h6> ~ <h1>
        else if (currentLine.trimStart ().substring (0, 7) == "###### ")
            currentLine = "<h6>" + currentLine.trimStart ().substring (7) + "</h6>";

        else if (currentLine.trimStart ().substring (0, 6) == "##### ")
            currentLine = "<h5>" + currentLine.trimStart ().substring (6) + "</h5>";

        else if (currentLine.trimStart ().substring (0, 5) == "#### ")
            currentLine = "<h4>" + currentLine.trimStart ().substring (5) + "</h4>";

        // <h3> anchor
        else if (currentLine.trimStart ().substring (0, 4) == "### ")
            currentLine = "<h3 id=\"" + currentLine.trimStart ().substring (4) + "\">"
            + currentLine.trimStart ().substring (4) + "</h3>";

        // <h2> anchor
        else if (currentLine.trimStart ().substring (0, 3) == "## ")
            currentLine = "<h2 id=\"" + currentLine.trimStart ().substring (3) + "\">" 
            + currentLine.trimStart ().substring (3) + "</h2>";

        // <h1> anchor
        else if (currentLine.trimStart ().substring (0, 2) == "# ")
            currentLine = "<h1 id=\"" + currentLine.trimStart ().substring (2) + "\">" 
            + currentLine.trimStart ().substring (2) + "</h1>";

        // align
        else if (currentLine.trimStart ().substring (0, 4) == ">|< ")
            currentLine = "<div align=center>" + currentLine.substring (4) + "</div>";

        else if (currentLine.trimStart ().substring (0, 4) == ">>> ")
            currentLine = "<div align=right>" + currentLine.substring (4) + "</div>";

        // diagram description
        else if (currentLine.trimStart ().substring (0, 3) == "^^ ")
            currentLine = "<h5 align=center>" + currentLine.substring (3) + "</h5></div>";
    }

    return contentByLine.joinIntoString (newLine);
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
        if (!linkAddress.contains (newLine) && linkAddress.containsNonWhitespaceChars ())
            resultStr = resultStr.replaceSection (indexStart, linkAddress.length () + 1, linkStr);

        indexStart = resultStr.indexOfIgnoreCase (indexStart + linkAddress.length (), " http");
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
                             + altContent + "\" />" + "</div>");

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
const String Md2Html::orderedListParse (const String& mdString, const bool isOrdered)
{
    if (mdString.isEmpty ())
        return mdString;

    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__ordered@List@Parse__%%");
    contentByLine.add ("%%__ordered@List@Parse__%%");

    for (int i = 1; i < contentByLine.size () - 1; ++i)
    {
        const String& prevLine = contentByLine.getReference (i - 1);
        const String& nextLine = contentByLine.getReference (i + 1);
        String& currentLine (contentByLine.getReference (i));
        String prefix, postfix;

        const String nestTag (isOrdered ? "    + " : "    - ");
        const String nestedStart (isOrdered ? "    <ol>" : "    <ul>");
        const String nestedEnd (isOrdered ? "    </ol>" : "    </ul>");
        const String listTag (isOrdered ? "+ " : "- ");
        const String listStart (isOrdered ? "<ol>" : "<ul>");
        const String listEnd (isOrdered ? "</ol>" : "</ul>");

        if (currentLine.substring (0, 6) == nestTag)
        {
            if (prevLine.substring (0, 6) != nestTag
                && prevLine.substring (0, 8) != nestedStart
                && prevLine.substring (0, 8) != "    <li>")
                prefix = nestedStart;
            if (nextLine.substring (0, 6) != nestTag)
                postfix = nestedEnd;

            currentLine = prefix + "    <li>" + currentLine.fromFirstOccurrenceOf (nestTag, false, true) + "</li>" + postfix;
        }
        else if (currentLine.substring (0, 2) == listTag)
        {
            if (prevLine.substring (0, 2) != listTag
                && prevLine.trimStart ().substring (0, 4) != "<li>"
                && prevLine.trimStart ().substring (0, 4) != listStart)
                prefix = listStart;
            if (nextLine.substring (0, 2) != listTag
                && nextLine.substring (0, 6) != nestTag)
                postfix = listEnd;

            currentLine = prefix + "<li>" + currentLine.fromFirstOccurrenceOf (listTag, false, true) + "</li>" + postfix;
        }

        if ((currentLine.substring (0, 8) == "    <li>"
             || currentLine.substring (0, 8) == nestedStart)
            && currentLine.getLastCharacters (5) == listEnd
            && nextLine.substring (0, 2) != listTag)
        {
            currentLine += listEnd;
        }
    }

    contentByLine.removeString ("%%__ordered@List@Parse__%%");
    return contentByLine.joinIntoString (newLine);
}

//=================================================================================================
const String Md2Html::cleanUp (const String& mdString)
{
    // transform newLine to <p> and <br>
    String resultStr (mdString.replace ("_%5x|z%!##!_", "*") // for code parse
                      .replace (newLine + newLine, "<p>\n")
                      .replace (newLine, "<br>\n")
    );

    // clean extra <br> when it's after any html-tag
    int indexBr = resultStr.indexOfIgnoreCase (0, "<br>");

    while (indexBr != -1)
    {
        if (resultStr.substring (indexBr - 1, indexBr).contains (">"))
            resultStr = resultStr.replaceSection (indexBr, 4, newLine);

        indexBr = resultStr.indexOfIgnoreCase (indexBr + 4, "<br>");
    }

    // clean extra <p> and <br> which is in code-block(s)
    int indexCodeStart = resultStr.indexOfIgnoreCase (0, "<pre><code>");

    while (indexCodeStart != -1 && indexCodeStart + 16 <= resultStr.length ())
    {
        const int indexCodeEnd = resultStr.indexOfIgnoreCase (indexCodeStart + 16, "</code></pre>");

        if (indexCodeEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexCodeStart, indexCodeEnd));
        const String codeHtml (mdCode.replace ("<p>", newLine).replace ("<br>", String ()));

        resultStr = resultStr.replaceSection (indexCodeStart, mdCode.length (), codeHtml);
        indexCodeStart = resultStr.indexOfIgnoreCase (indexCodeStart + 16, "<pre><code>");
    }

    // somehow, there's this ugly thing. dont know why.
    resultStr = resultStr.replace ("<pre><code>				", "<pre><code>");

    // clean extra <br> and <p> which before <pre><code>
    resultStr = resultStr.replace ("<br>\n<pre>", newLine + "<pre>")
        .replace ("<p>\n<pre>", newLine + "<pre>");

    // make the <pre><code> at the same line with the code
    // otherwise the vertical-gap will too wide
    resultStr = resultStr.replace (String ("<pre><code>") + newLine, "<pre><code>");

    // give it a <p> after '<hr>' and '</code></pre>'
    //resultStr = resultStr.replace (String ("</table>") + newLine, "</table><p>");
    resultStr = resultStr.replace (String ("<hr>") + newLine, "<hr>\n<p>");
    resultStr = resultStr.replace (String ("</code></pre>") + newLine, "</code></pre>\n<p>");

    // for escape
    resultStr = resultStr.replace (String ("\\*"), "*");
    resultStr = resultStr.replace (String ("\\~"), "~");
    resultStr = resultStr.replace (String ("\\`"), "`");
    resultStr = resultStr.replace (String ("\\```"), "```");
    resultStr = resultStr.replace (String ("\\#"), "#");
    resultStr = resultStr.replace (String ("\\!["), "![");

    //DBG (resultStr);
    return resultStr;
}


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
    if (mdString.isEmpty())
        return String();

    // parse markdown, must followed by these order
    String htmlContent (mdString);

    htmlContent = identifierParse (htmlContent);
    htmlContent = commentParse (htmlContent);
    htmlContent = tableParse (htmlContent);
    htmlContent = hybridParse (htmlContent);
    htmlContent = codeBlockParse (htmlContent);
    htmlContent = endnoteParse (htmlContent);
    htmlContent = inlineCodeParse (htmlContent);
    htmlContent = boldAndItalicParse (htmlContent);
    htmlContent = boldParse (htmlContent);
    htmlContent = italicParse (htmlContent);
    htmlContent = highlightParse (htmlContent);
    htmlContent = tocParse (htmlContent);
    htmlContent = processByLine (htmlContent);
    htmlContent = spaceLinkParse (htmlContent);
    htmlContent = imageParse (htmlContent);
    htmlContent = audioParse (htmlContent);
    htmlContent = videoParse (htmlContent);
    htmlContent = mdLinkParse (htmlContent);
    htmlContent = listParse (htmlContent, true);
    htmlContent = listParse (htmlContent, false);
    htmlContent = cnBracketParse (htmlContent);
    htmlContent = cleanUp (htmlContent);

    //DBG (htmlContent);
    return htmlContent;
}

//=================================================================================================
const String Md2Html::tableParse (const String& mdString)
{
    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__table@MDtag@Parse__%%");
    contentByLine.add ("%%__table@MDtag@Parse__%%");
    
    for (int i = 1; i < contentByLine.size() - 1; ++i)
    {
        String& prevLine = contentByLine.getReference (i - 1);
        String& currentLine (contentByLine.getReference (i));
        String& nextLine = contentByLine.getReference (i + 1);

        if ((currentLine.substring (0, 6) == "------"
             || currentLine.substring (0, 6) == "======"
             || currentLine.substring (0, 6) == "//////")
            && prevLine.contains (" | ") 
            && nextLine.contains (" | "))
        {
            // style
            String styleClass;

            if (currentLine.substring (0, 6) == "------")
                styleClass = "<table class=normalTable>";

            else if (currentLine.substring (0, 6) == "======")
                styleClass = "<table class=interlacedTable>";

            else if (currentLine.substring (0, 6) == "//////")
                styleClass = "<table class=noBorderTable>";

            // the first column's align
            String firstColumnAlign (">");  // default for left

            if (prevLine.trimStart().substring (0, 3) == "(>)")
                firstColumnAlign = " align=right>";
            else if (prevLine.trimStart().substring (0, 3) == "(^)")
                firstColumnAlign = " align=center>";

            // get align marks for other columns
            StringArray alignArray;
            int alignIndex = prevLine.indexOf (" | ");

            while (alignIndex != -1)
            {
                if (prevLine.substring (alignIndex + 3, alignIndex + 6) == "(>)")
                    alignArray.add (" align=right>");
                else if (prevLine.substring (alignIndex + 3, alignIndex + 6) == "(^)")
                    alignArray.add (" align=center>");
                else
                    alignArray.add (">");

                alignIndex = prevLine.indexOf (alignIndex + 3, " | "); // 3 for ' | '
            }
            
            // process the table-head line
            currentLine = prevLine.replace ("(>)", String()).replace ("(^)", String());
            prevLine = styleClass;
            currentLine = "<tr><th>" + currentLine.replace (" | ", "</th><th>") + "</th></tr>";

            // process next line (the first line of this table)
            nextLine = "<tr><td" + firstColumnAlign + nextLine.replace (" | ", "</td><td>") + "</td></tr>";
            alignIndex = nextLine.indexOf (8, "<td>"); // 8 for at least is '<tr><td>'
            int indexOfMarkArray = 0;

            while (alignIndex != -1)
            {
                // here to prevent table-head columns less than this line's column
                if (indexOfMarkArray < alignArray.size())
                    nextLine = nextLine.replaceSection (alignIndex + 3, 1, alignArray[indexOfMarkArray]);
                else
                    break;

                ++indexOfMarkArray;
                alignIndex = nextLine.indexOf (alignIndex + 9, "<td>"); // 9 for '<td></td>'
            }

            // process other lines
            int rowNums = i + 1;
            int numForEvenLine = 0;

            while (++rowNums < contentByLine.size())
            {
                String& thisLine = contentByLine.getReference (rowNums);

                if (thisLine.contains (" | "))
                {
                    thisLine = "<tr><td" + firstColumnAlign + thisLine.replace (" | ", "</td><td>") + "</td></tr>";

                    // process align
                    alignIndex = thisLine.indexOf (8, "<td>"); // 8 for at least is '<tr><td>'
                    indexOfMarkArray = 0;

                    while (alignIndex != -1)
                    {
                        // here to prevent table-head columns less than this line's column
                        if (indexOfMarkArray < alignArray.size())
                            thisLine = thisLine.replaceSection (alignIndex + 3, 1, alignArray[indexOfMarkArray]);
                        else
                            break;

                        ++indexOfMarkArray;
                        alignIndex = thisLine.indexOf (alignIndex + 9, "<td>"); // 9 for '<td></td>'
                    }

                    // for interlace style
                    if (styleClass == "<table class=interlacedTable>" && (numForEvenLine % 2 == 0))
                        thisLine = thisLine.replaceSection (0, 4, "<tr class=interlacedEven>");

                    ++numForEvenLine;
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
    return contentByLine.joinIntoString ("\n");
}

//=================================================================================================
const String Md2Html::hybridParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "~~~");

    while (indexStart != -1 && resultStr[indexStart - 1] != '\\')
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 3, "~~~");

        if (indexEnd == -1)
            break;

        if (resultStr[indexEnd - 1] == '\\')
        {
            indexStart = resultStr.indexOfIgnoreCase (indexStart + 3, "~~~");
            continue;
        }

        // get margin value
        int marginNum = 0;
        const String& marginChar (resultStr.substring (indexStart + 3, indexStart + 4));

        if (marginChar.isNotEmpty())
            marginNum = jlimit (0, 9, marginChar.getIntValue());

        const String contentStr (resultStr.substring (indexStart, indexEnd));

        // process by line
        StringArray lines;
        lines.addTokens (contentStr, newLine, String());

        // remove 2 '~~~'
        lines.remove (0);
        lines.remove (lines.size() - 1);

        // make sure there isn't any empty line at the begin and end
        while (lines.size() > 0 && lines[0].isEmpty())
            lines.remove (0);   

        while (lines.size() > 0 && lines[lines.size() - 1].isEmpty())
            lines.remove (lines.size() - 1);

        // process the 1st column of the first line
        if (lines.size() < 1)
        {
            return resultStr.replaceSection (indexStart, indexEnd - indexStart + 3, String());
        }
        else if (lines.size() == 1)
        {
            lines.getReference (0) = "<tr><td>" + lines[0] + "</td></tr>";
        }
        else
        {
            lines.getReference (0) = "<tr><td>" + lines[0] + "</td>";

            // process others..
            for (int i = lines.size() - 1; --i > 0; )
            {
                if (lines[i - 1].trim().isEmpty())
                    lines.getReference (i) = "<tr><td>" + lines[i] + "</td>";
                else if (lines[i + 1].trim().isEmpty())
                    lines.getReference (i) = "<td>" + lines[i] + "</td></tr>";
                else if (lines[i].trim().isNotEmpty())
                    lines.getReference (i) = "<td>" + lines[i] + "</td>";
            }

            lines.removeEmptyStrings (true);

            // process the last column of the last line
            lines.getReference (lines.size() - 1) = "<td>" + lines[lines.size() - 1] + "</td></tr>";
        }
        
        // finally...
        lines.insert (0, "<table class=hybridTable id=hybrid-" + String (marginNum) + ">");
        lines.add ("</table>");

        const String& htmlStr (lines.joinIntoString (newLine));

        //DBG (htmlStr);
        resultStr = resultStr.replaceSection (indexStart, contentStr.length() + 3, htmlStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + htmlStr.length(), "~~~");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::identifierParse (const String& mdString)
{
    StringArray lines;
    lines.addLines (mdString);
    
    for (int i = lines.size(); --i >= 0; )
    {
        if (lines[i].substring (0, 6) == "******")
            lines.getReference (i) = "<p>";
    }

    return lines.joinIntoString (newLine);
}

//=================================================================================================
const String Md2Html::commentParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "//////");

    while (indexStart != -1)
    {
        if (resultStr.substring (indexStart - 1, indexStart) == "\\")
        {
            indexStart = resultStr.indexOfIgnoreCase (indexStart + 6, "//////");
            continue;
        }

        // get to the row end. because more than 6 '/' at the begin might be
        int tempIndex = indexStart + 6;

        while (resultStr.indexOfIgnoreCase (tempIndex + 1, "/") != -1
               && resultStr.substring (tempIndex, tempIndex + 1) != "\n")
            ++tempIndex;

        int indexEnd = resultStr.indexOfIgnoreCase (tempIndex, "//////");

        if (indexEnd == -1)
            break;

        if (resultStr.substring (indexEnd - 1, indexEnd) == "\\")
        {
            indexStart = resultStr.indexOfIgnoreCase (indexEnd + 6, "//////");
            continue;
        }
        
        // get to the end. because more than 6 '/' at the end might be
        indexEnd += 6;

        while (resultStr.indexOfIgnoreCase (indexEnd + 1, "/") != -1
               && resultStr.substring (indexEnd, indexEnd + 1) != "\n")
            ++indexEnd;

        resultStr = resultStr.replaceSection (indexStart, indexEnd - indexStart + 1, "<p>");
        indexStart = resultStr.indexOfIgnoreCase (indexStart, "//////");
    }

    //DBG (resultStr);
    return resultStr;
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

        const String& mdCode (resultStr.substring (indexStart, indexEnd + 3));

        const int codeStyleIndexEnd = mdCode.indexOf (0, "\n");
        const String& highlightStyle (mdCode.substring (3, codeStyleIndexEnd).trim());
        const String& mdCodeNew = mdCode.substring (codeStyleIndexEnd);

        const String& htmlStr ("<pre><code class=\"" + highlightStyle + "\">"
                               + mdCodeNew.replace ("```", String())
                               .replace ("*", "_%5x|z%!##!_") // see cleanup(), prevent bold and italic parse it
                               .replace ("<", "&lt;").replace (">", "&gt;")  // escape html code
                               + "</code></pre>");

        //DBG (htmlStr);
        resultStr = resultStr.replaceSection (indexStart, mdCode.length(), htmlStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + htmlStr.length(), "```");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::endnoteParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "[^");
    int noteNumber = 0;
    StringArray notes;

    while (indexStart != -1)
    {
        // get note content's end index
        int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 2, "]");

        if (indexEnd == -1)
            break;

        // need to process the link mark which may inside endnote
        if (resultStr.substring (indexEnd + 1, indexEnd + 2) == "(")
            indexEnd = resultStr.indexOfIgnoreCase (indexEnd + 2, "]");

        if (resultStr.substring (indexStart - 1, indexStart) != "\\"
            && resultStr.substring (indexEnd - 1, indexEnd) !="\\")
        {
            ++noteNumber;

            // get note content
            const String noteStr (resultStr.substring (indexStart + 2, indexEnd));

            if (noteStr.trim().isNotEmpty())
            {
                notes.add ("<li><span id=\"endnote-" + String (noteNumber) + "\">"
                           + noteStr + "</span></li>\n");

                resultStr = resultStr.replaceSection (indexStart + 2, noteStr.length(), String());
                resultStr = resultStr.replaceSection (indexStart, 3, "<sup><a href=\"#endnote-"
                                                      + String (noteNumber) + "\">"
                                                      + "[" + String (noteNumber) + "]</a></sup>");
            }
        }

        indexStart = resultStr.indexOfIgnoreCase (indexStart + 2, "[^");
    }

    if (notes.size() > 0)
    {
        notes.insert (0, "**" + TRANS ("Endnote(s): ") + "**");
        notes.insert (1, "<ol><div class=endnote>");
        notes.add ("</ol></div>");

        resultStr = resultStr.trimEnd() + newLine + "----" + newLine;
        resultStr = resultStr + notes.joinIntoString (newLine);
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
            && resultStr.substring (index - 1, index) != "`"
            && resultStr.substring (index + 1, index + 2) != "`")
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

    // for bold, italic and html code parse
    int indexStart = resultStr.indexOfIgnoreCase (0, "<code>");

    while (indexStart != -1)
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 6, "</code>");

        if (indexEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexStart + 1, indexEnd)
                             .replace ("*", "_%5x|z%!##!_")
                             .replace ("<", "&lt;"));

        resultStr = resultStr.replaceSection (indexStart, indexEnd - indexStart, "<" + mdCode);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + mdCode.length(), "<code>");
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
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 3, index + 4) != "*")
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
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 2, index + 3) != "*")
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
            && resultStr.substring (index - 1, index) != "*"
            && resultStr.substring (index + 1, index + 2) != "*")
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
            && resultStr.substring (index - 1, index) != "~"
            && resultStr.substring (index + 2, index + 3) != "~")
        {
            if (i % 2 == 1)
                resultStr = resultStr.replaceSection (index, 2, "<span style=\"background: #CCFF66\">");
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
        if (lines[i].trimStart().substring (0, 1) != "#")
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

    for (int i = contentByLine.size(); --i >= 0; )
    {
        String& currentLine (contentByLine.getReference (i));

        // <hr>
        if (currentLine.trimStart().substring (0, 3) == "---")
            currentLine = "<hr>";

        // <blockquote>
        else if (currentLine.trimStart().substring (0, 2) == "> ")
            currentLine = "<blockquote>" + currentLine.trimStart().substring (2) + "</blockquote>";

        // <h6> ~ <h1>, also parse Chinese '#'
        else if (currentLine.trimStart().substring (0, 7) == "###### "
                 || currentLine.trimStart().substring (0, 7) ==
                 CharPointer_UTF8 ("\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83 "))
            currentLine = "<h6>" + currentLine.trimStart().substring (7) + "</h6>";

        else if (currentLine.trimStart().substring (0, 6) == "##### "
                 || currentLine.trimStart().substring (0, 6) ==
                 CharPointer_UTF8 ("\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83 "))
            currentLine = "<h5>" + currentLine.trimStart().substring (6) + "</h5>";

        else if (currentLine.trimStart().substring (0, 5) == "#### "
                 || currentLine.trimStart().substring (0, 5) ==
                 CharPointer_UTF8 ("\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83 "))
            currentLine = "<h4>" + currentLine.trimStart().substring (5) + "</h4>";

        // <h3> anchor
        else if (currentLine.trimStart().substring (0, 4) == "### "
                 || currentLine.trimStart().substring (0, 4) ==
                 CharPointer_UTF8 ("\xef\xbc\x83\xef\xbc\x83\xef\xbc\x83 "))
            currentLine = "<h3 id=\"" + currentLine.trimStart().substring (4) + "\">"
            + currentLine.trimStart().substring (4) + "</h3>";

        // <h2> anchor
        else if (currentLine.trimStart().substring (0, 3) == "## "
                 || currentLine.trimStart().substring (0, 3) ==
                 CharPointer_UTF8 ("\xef\xbc\x83\xef\xbc\x83 "))
            currentLine = "<h2 id=\"" + currentLine.trimStart().substring (3) + "\">"
            + currentLine.trimStart().substring (3) + "</h2>";

        // <h1> anchor
        else if (currentLine.trimStart().substring (0, 2) == "# "
                 || currentLine.trimStart().substring (0, 2) ==
                 CharPointer_UTF8 ("\xef\xbc\x83 "))
            currentLine = "<h1 id=\"" + currentLine.trimStart().substring (2) + "\">"
            + currentLine.trimStart().substring (2) + "</h1>";

        // align
        else if (currentLine.trimStart().substring (0, 4) == "(^) ")
            currentLine = "<div align=center>" + currentLine.trimStart().substring (4) + "</div>";

        else if (currentLine.trimStart().substring (0, 4) == "(>) ")
            currentLine = "<div align=right>" + currentLine.trimStart().substring (4) + "</div>";

        // diagram description
        else if (currentLine.trimStart().substring (0, 3) == "^^ ")
            currentLine = "<h5 align=center>" + currentLine.trimStart().substring (3) + "</h5></div>";

        // indent (it might be inside a table)
        else if (currentLine.trimStart().substring (0, 4) == "(+) ")
            currentLine = "<div style=\"text-indent: 2em; padding: 0;\">"
            + currentLine.trimStart().substring (4) + "</div>";

        else if (currentLine.trimStart().substring (4, 8) == "(+) ")
            currentLine = currentLine.trimStart().substring (0, 4)
            + "<div style=\"text-indent: 2em; padding: 0;\">"
            + currentLine.trimStart().substring (8) + "</div>";

        else if (currentLine.trimStart().substring (8, 12) == "(+) ")
            currentLine = currentLine.trimStart().substring (0, 8)
            + "<div style=\"text-indent: 2em; padding: 0;\">"
            + currentLine.trimStart().substring (12) + "</div>";

        // anti-indent
        else if (currentLine.trimStart().substring (0, 4) == "(-) ")
            currentLine = "<div style=\"text-indent: 0; padding: 0;\">" 
            + currentLine.trimStart().substring (4) + "</div>";
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
        if (!linkAddress.contains (newLine) && linkAddress.containsNonWhitespaceChars())
            resultStr = resultStr.replaceSection (indexStart, linkAddress.length() + 1, linkStr);

        indexStart = resultStr.indexOfIgnoreCase (indexStart + linkAddress.length(), " http");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::imageParse (const String& mdString)
{
    /**< ![](media/xxx.jpg =500) */
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "![");

    while (indexStart != -1)
    {
        if (resultStr.substring (indexStart - 1, indexStart) == "\\")
        {
            indexStart = resultStr.indexOfIgnoreCase (indexStart + 2, "![");
            continue;
        }

        // get alt content
        const int altEnd = resultStr.indexOfIgnoreCase (indexStart + 2, "](");

        if (altEnd == -1)
            break;

        const String& altContent (resultStr.substring (indexStart + 2, altEnd));

        // get img str include width
        const int imgEnd = resultStr.indexOfIgnoreCase (altEnd + 2, ")");

        if (imgEnd == -1)
            break;

        const String& imgStrIncludeWidth (resultStr.substring (altEnd + 2, imgEnd));
        const String& widthValueStr (imgStrIncludeWidth.fromLastOccurrenceOf ("=", false, false).trim());

        String imgPath (imgStrIncludeWidth);
        String widthStr;

        // img path and width
        if (widthValueStr != imgStrIncludeWidth)
        {
            imgPath = imgPath.upToLastOccurrenceOf ("=", false, false).trim();

            if (widthValueStr.getIntValue() > 0)
                widthStr = " width=\"" + String (widthValueStr.getIntValue()) + "\"";
        }

        const String& imgStr ("<div align=center><img src=\"" + imgPath + "\" title=\""
                             + altContent + "\"" + widthStr + " />" + "</div>");

        resultStr = resultStr.replaceSection (indexStart, imgEnd + 1 - indexStart, imgStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + imgStr.length(), "![");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::audioParse (const String& mdString)
{
    /**< ~[](media/xxx.mp3) */
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "~[](");

    while (indexStart != -1)
    {
        if (resultStr.substring (indexStart - 1, indexStart) == "\\")
        {
            indexStart = resultStr.indexOfIgnoreCase (indexStart + 4, "~[](");
            continue;
        }
                
        // get audio file path
        const int audioEnd = resultStr.indexOfIgnoreCase (indexStart + 4, ")");

        if (audioEnd == -1)
            break;

        const String& audioPath (resultStr.substring (indexStart + 4, audioEnd).trim());

        const String& audioStr ("<div align=center><audio src=\"" + audioPath + "\""
                                + " preload=\"auto\" controls />" + "</div>");

        resultStr = resultStr.replaceSection (indexStart, audioEnd + 1 - indexStart, audioStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + audioStr.length(), "~[](");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::videoParse (const String& mdString)
{
    /**< @[](media/xxx.mp4 = 680) */
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, "@[](");

    while (indexStart != -1)
    {
        if (resultStr.substring (indexStart - 1, indexStart) == "\\")
        {
            indexStart = resultStr.indexOfIgnoreCase (indexStart + 4, "@[](");
            continue;
        }
                
        // get video str include width
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 4, ")");

        if (indexEnd == -1)
            break;

        const String& videoStrIncludeWidth (resultStr.substring (indexStart + 4, indexEnd));
        const String& widthValueStr (videoStrIncludeWidth.fromLastOccurrenceOf ("=", false, false).trim());

        String videoPath (videoStrIncludeWidth);
        String widthStr;

        // video path and width
        if (widthValueStr != videoStrIncludeWidth)
        {
            videoPath = videoPath.upToLastOccurrenceOf ("=", false, false).trim();

            if (widthValueStr.getIntValue() > 0)
                widthStr = " width=\"" + String (widthValueStr.getIntValue()) + "\"";
        }

        const String& videoStr ("<div align=center><video src=\"" + videoPath + "\""
                              + widthStr + " preload=\"auto\" controls />" + "</div>");

        resultStr = resultStr.replaceSection (indexStart, indexEnd + 1 - indexStart, videoStr);
        indexStart = resultStr.indexOfIgnoreCase (indexStart + videoStr.length(), "@[](");
    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::mdLinkParse (const String& mdString)
{
    // [](http://xxx.com)
    String resultStr (mdString);
    int linkPathStart = resultStr.indexOfIgnoreCase (0, "](");

    while (linkPathStart != -1)
    {
        // get alt content
        const String splitContent (resultStr.substring (0, linkPathStart));
        const int altStart = splitContent.lastIndexOfIgnoreCase ("[");

        if (altStart == -1)
            break;

        if (resultStr.substring (altStart - 1, altStart) != "\\"
            && resultStr.substring (altStart - 1, altStart) != "!"
            && resultStr.substring (altStart - 1, altStart) != "~"
            && resultStr.substring (altStart - 1, altStart) != "@")
        {
            const String altContent (resultStr.substring (altStart + 1, linkPathStart));

            // get link path
            const int pathEnd = resultStr.indexOfIgnoreCase (linkPathStart + 2, ")");

            if (pathEnd == -1)
                break;

            String linkPath (resultStr.substring (linkPathStart + 2, pathEnd).trimEnd());

            // [](http://xxx.com/xxx.html -), the end ' -' will open the link in new window
            if (linkPath.getLastCharacters (2) == " -")
                linkPath = "\"" + linkPath.dropLastCharacters (2) + "\" target=\"_blank\"";
            else
                linkPath = "\"" + linkPath + "\"";

            const String linkStr ("<a href=" + linkPath + ">" + altContent + "</a>");

            resultStr = resultStr.replaceSection (altStart, pathEnd + 1 - altStart, linkStr);
            linkPathStart = resultStr.indexOfIgnoreCase (altStart + linkStr.length(), "](");
        }
        else
        {
            linkPathStart = resultStr.indexOfIgnoreCase (linkPathStart + 2, "](");
        }

    }

    return resultStr;
}

//=================================================================================================
const String Md2Html::listParse (const String& mdString, const bool isOrdered)
{
    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__ordered@List@Parse__%%");
    contentByLine.add ("%%__ordered@List@Parse__%%");

    for (int i = 1; i < contentByLine.size() - 1; ++i)
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
                && prevLine.trimStart().substring (0, 4) != "<li>"
                && prevLine.trimStart().substring (0, 4) != listStart)
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
const String Md2Html::cnBracketParse (const String& mdString)
{
    String resultStr (mdString);
    int indexStart = resultStr.indexOfIgnoreCase (0, CharPointer_UTF8 ("\xef\xbc\x88"));

    while (indexStart != -1)
    {
        // get note content's end index
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 1, CharPointer_UTF8 ("\xef\xbc\x89"));

        if (indexEnd == -1)
            break;

        // get content inside 2 brackets
        const String content (resultStr.substring (indexStart + 1, indexEnd));

        if (content.isNotEmpty())
        {
            const String withSpan ("<span class=cnBracket>" + content + "</span>");
            resultStr = resultStr.replaceSection (indexStart + 1, content.length(), withSpan);
        }

        indexStart = resultStr.indexOfIgnoreCase (indexEnd, CharPointer_UTF8 ("\xef\xbc\x88"));
    }
    
    return resultStr;
}

//=================================================================================================
const String Md2Html::cleanUp (const String& mdString)
{
    // transform newLine to <p> and <br>
    String resultStr (mdString.replace ("_%5x|z%!##!_", "*") // for code parse
                      .replace (newLine + newLine, "<p>\n")
                      .replace (newLine, "<br>\n"));

    // clean extra <br> when it's after any html-tag
    int indexBr = resultStr.indexOfIgnoreCase (0, "<br>");

    while (indexBr != -1)
    {
        if (resultStr.substring (indexBr - 1, indexBr) == ">"
            // prevent extra empty row of code-block (the first row of it)
            && resultStr.substring (indexBr - 2, indexBr) != "\">")
            resultStr = resultStr.replaceSection (indexBr, 4, newLine);

        indexBr = resultStr.indexOfIgnoreCase (indexBr + 4, "<br>");
    }

    // clean extra <p> and <br> of code-block(s)
    int indexCodeStart = resultStr.indexOfIgnoreCase (0, "<pre><code class=");

    while (indexCodeStart != -1 && indexCodeStart + 20 <= resultStr.length())
    {
        const int indexCodeEnd = resultStr.indexOfIgnoreCase (indexCodeStart + 20, "</code></pre>");

        if (indexCodeEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexCodeStart, indexCodeEnd));
        const String codeHtml (mdCode.replace ("<p>", newLine).replace ("<br>", String()));

        resultStr = resultStr.replaceSection (indexCodeStart, mdCode.length(), codeHtml);
        indexCodeStart = resultStr.indexOfIgnoreCase (indexCodeStart + 33, "<pre><code class=");
    }

    // clean extra <p> and <br> of page's js-code (inside <body/>)
    indexCodeStart = resultStr.indexOfIgnoreCase (0, "<script");

    while (indexCodeStart != -1 && indexCodeStart + 8 <= resultStr.length())
    {
        const int indexCodeEnd = resultStr.indexOfIgnoreCase (indexCodeStart + 8, "</script>");

        if (indexCodeEnd == -1)
            break;

        const String jsCode (resultStr.substring (indexCodeStart, indexCodeEnd));
        const String codeHtml (jsCode.replace ("<p>", newLine).replace ("<br>", String()));

        resultStr = resultStr.replaceSection (indexCodeStart, jsCode.length(), codeHtml);
        indexCodeStart = resultStr.indexOfIgnoreCase (indexCodeStart + 17, "<script");
    }

    // clean up empty line in table
    resultStr = resultStr.replace ("\n\n<td>", "\n<td>")
        .replace ("\n<tr>", "<tr>")
        .replace ("</tr>\n", "<tr>")
        .replace ("\n</table>", "</table>");

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
    resultStr = resultStr.replace ("\\*", "*");
    resultStr = resultStr.replace ("\\~", "~");
    resultStr = resultStr.replace ("\\`", "`");
    resultStr = resultStr.replace ("\\```", "```");
    resultStr = resultStr.replace ("\\#", "#");
    resultStr = resultStr.replace ("\\[TOP]", "@@##ToTOPOfThePage##@@");  // escape for [TOP]
    resultStr = resultStr.replace ("\\[", "[");
    resultStr = resultStr.replace ("\\![", "![");
    resultStr = resultStr.replace ("\\~[", "~[");
    resultStr = resultStr.replace ("\\@[", "@[");
    resultStr = resultStr.replace ("\\[^", "[^");
    resultStr = resultStr.replace ("\\]", "]");
    resultStr = resultStr.replace ("\\/", "/");
    resultStr = resultStr.replace ("<p><br>", "<p>"); 
    resultStr = resultStr.replace ("<!--<br>", "<!--");
    resultStr = resultStr.replace ("<!--<p>", "<!--");

    // parse [TOP]: a html-button on page for 'back to top'
    resultStr = resultStr.replace ("[TOP]", "<div class=page_navi id=right><a href=\"#top\">" 
                                   + TRANS ("Back to Top") + "</a></div>");
    resultStr = resultStr.replace ("@@##ToTOPOfThePage##@@", "[TOP]");

    // for scroll to bottom
    resultStr += newLine + "<span id=\"wdtpPageBottom\"></span>";

    //DBG (resultStr);
    return resultStr;
}

//=================================================================================================
const ValueTree FrontMatterParser::processIfHasFrontMatter (String& content)
{
    if (content.trimStart().substring (0, 3) == "+++" || content.trimStart().substring (0, 3) == "---")
    {
        content = content.trimStart();
        const String frontMark ((content.substring (0, 3) == "---") ? "---" : "+++");

        int indexEnd = content.indexOf (3, frontMark);
        indexEnd = content.indexOf (indexEnd + 3, "\n");

        if (indexEnd != -1)
        {
            StringArray frontStrs;
            frontStrs.addTokens (content.substring (0, indexEnd).replace (" ", String()), newLine, String());

            frontStrs.removeEmptyStrings (true);
            frontStrs.remove (0);
            frontStrs.remove (frontStrs.size() - 1);

            ValueTree tree ("doc");

            for (auto str : frontStrs)
            {
                if (str.substring (0, 4) == "date")
                {
                    // ="2013-06-21T11:27:27
                    const String& dateStr (str.substring (6, 25).replace ("T", " ").replace ("-", "."));
                    tree.setProperty ("createDate", dateStr, nullptr);
                }

                else if (str.substring (0, 5) == "title")
                {
                    const String& titleStr (str.substring (7).replace ("\"", String()));
                    tree.setProperty ("title", titleStr.trimEnd(), nullptr);
                }

                else if (str.substring (0, 11) == "description")
                {
                    const String& descStr (str.substring (13).replace ("\"", String()));
                    tree.setProperty ("description", descStr.trimEnd(), nullptr);
                }

                else if (str.substring (0, 4) == "tags")
                {
                    const String& tagStr (str.substring (7).replace ("\"", String()).replace ("]", String()));
                    tree.setProperty ("keywords", tagStr.trimEnd(), nullptr);
                }
            }

            content = content.substring (indexEnd).trimStart();
            return tree;
        }
    }

    return ValueTree();
}

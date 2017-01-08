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
const String Md2Html::mdStringToHtml (const String& mdString)
{
    if (mdString.isEmpty ())
        return String();

    // parse markdown
    String htmlContent (mdString);
    htmlContent = codeBlockParse (htmlContent);
    htmlContent = inlineCodeParse (htmlContent);
    htmlContent = boldParse (htmlContent);
    htmlContent = italicParse (htmlContent);
    htmlContent = processByLine (htmlContent);
    htmlContent = spaceLinkParse (htmlContent);
    htmlContent = imageParse (htmlContent);
    htmlContent = mdLinkParse (htmlContent);
    htmlContent = orderedListParse (htmlContent);
    htmlContent = unorderedListParse (htmlContent);
    htmlContent = cleanUp (htmlContent);
    
    htmlContent += copyrightInfo;
    //DBG (htmlContent);

    return htmlContent;
}

//=================================================================================================
const String Md2Html::renderHtmlContent (const String& htmlContentStr, 
                                         const File& tplFile,
                                         const String& htmlKeywords,
                                         const String& htmlAuthor,
                                         const String& htmlDescription,
                                         const String& htmlTitle,
                                         const String& cssPath,
                                         const bool codeHighlight)
{
    String htmlStr (tplFile.loadFileAsString ());

    if (htmlContentStr.isEmpty ())
        return htmlStr;

    if (codeHighlight)
    {
        htmlStr = htmlStr.replace (newLine + "  <title>", 
                                   "\n  <script src = \""
                                   + cssPath + "add-in/hl.js\"></script>\n"
                                   "  <script>hljs.initHighlightingOnLoad(); </script>\n"
                                   "  <title>");
    }

    return htmlStr.replace ("{{keywords}}", htmlKeywords)
        .replace ("{{author}}", htmlAuthor)
        .replace ("{{description}}", htmlDescription)
        .replace ("{{title}}", htmlTitle)
        .replace ("{{siteRelativeRootPath}}", cssPath)
        .replace ("{{content}}", htmlContentStr);
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
                              + mdCode.replace ("```", String())
                              .replace ("*", "_%5x|z%!##!_") // for bold and italic parse
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
    int indexStart = resultStr.indexOfIgnoreCase (0, "`");

    while (indexStart != -1)
    {
        const int indexEnd = resultStr.indexOfIgnoreCase (indexStart + 2, "`");

        if (indexEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexStart, indexEnd + 1));
        const String htmlStr ("<code>" + mdCode.replace ("`", String ())
                              .replace ("*", "_%5x|z%!##!_") // for bold and italic parse
                              /*.replace ("\"", "&quot;")
                              .replace ("&", "&amp;")
                              .replace ("<", "&lt;")
                              .replace (">", "&gt;")
                              .replace (" ", "&nbsp;")*/
                              + "</code>");

        //DBG (htmlStr);
        if (!htmlStr.contains (newLine)) // must place in the same line            
            resultStr = resultStr.replaceSection (indexStart, mdCode.length (), htmlStr);        

        indexStart = resultStr.indexOfIgnoreCase (indexStart + htmlStr.length (), "`");
    }

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
const String Md2Html::processByLine (const String& mdString)
{
    StringArray contentByLine;
    contentByLine.addLines (mdString);

    for (int i = contentByLine.size (); --i >= 0; )
    {
        String& currentLine (contentByLine.getReference (i));

        // <hr>
        if (currentLine.substring (0, 3) == "---")
            currentLine = "<hr>";

        // <blockquote>
        else if (currentLine.substring (0, 2) == "> ")
            currentLine = "<blockquote>" + currentLine.substring (2) + "</blockquote>";

        // <h6> ~ <h1>
        else if (currentLine.substring (0, 7) == "###### ")
            currentLine = "<h6>" + currentLine.substring (7) + "</h6>";

        else if (currentLine.substring (0, 6) == "##### ")
            currentLine = "<h5>" + currentLine.substring (6) + "</h5>";

        else if (currentLine.substring (0, 5) == "#### ")
            currentLine = "<h4>" + currentLine.substring (5) + "</h4>";

        else if (currentLine.substring (0, 4) == "### ")
            currentLine = "<h3>" + currentLine.substring (4) + "</h3>";

        else if (currentLine.substring (0, 3) == "## ")
            currentLine = "<h2>" + currentLine.substring (3) + "</h2>";

        else if (currentLine.substring (0, 2) == "# ")
            currentLine = "<div align=center><h1>" + currentLine.substring (2) + "</h1></div>";

        // align
        else if (currentLine.substring (0, 4) == ">|< ")
            currentLine = "<div align=center>" + currentLine.substring (4) + "</div>";

        else if (currentLine.substring (0, 4) == ">>> ")
            currentLine = "<div align=right>" + currentLine.substring (4) + "</div>";
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
const String Md2Html::unorderedListParse (const String& mdString)
{
    if (mdString.isEmpty ())
        return mdString;

    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__unordered@List@Parse__%%");
    contentByLine.add ("%%__unordered@List@Parse__%%");

    for (int i = 1; i < contentByLine.size () - 1; ++i)
    {
        const String& prevLine (contentByLine.getReference (i - 1));
        String& currentLine (contentByLine.getReference (i));
        const String& nextLine (contentByLine.getReference (i + 1));
        String prefix, postfix;

        if (currentLine.substring (0, 6) == "    - ")
        {
            if (prevLine.substring (0, 6) != "    - "
                && prevLine.substring (0, 8) != "    <ul>"
                && prevLine.substring (0, 8) != "    <li>")
                prefix = "    <ul>";
            if (nextLine.substring (0, 6) != "    - ")
                postfix = "    </ul>";

            currentLine = prefix + "    <li>" + currentLine.fromFirstOccurrenceOf ("    - ", false, true) + "</li>" + postfix;
        }
        else if (currentLine.substring (0, 2) == "- ")
        {
            if (prevLine.substring (0, 2) != "- "
                && prevLine.substring (0, 4) != "<li>"
                && prevLine.substring (0, 4) != "<ul>")
                prefix = "<ul>";
            if (nextLine.substring (0, 2) != "- "
                && nextLine.substring (0, 6) != "    - ")
                postfix = "</ul>";

            currentLine = prefix + "<li>" + currentLine.fromFirstOccurrenceOf ("- ", false, true) + "</li>" + postfix;
        }
    }

    contentByLine.removeString ("%%__unordered@List@Parse__%%");
    return contentByLine.joinIntoString (newLine).replace ("    </ul>", "    </ul></ul>");
}

//=================================================================================================
const String Md2Html::orderedListParse (const String& mdString)
{
    if (mdString.isEmpty ())
        return mdString;

    StringArray contentByLine;
    contentByLine.addLines (mdString);

    contentByLine.insert (0, "%%__ordered@List@Parse__%%");
    contentByLine.add ("%%__ordered@List@Parse__%%");

    for (int i = 1; i < contentByLine.size() - 1; ++i)
    {
        const String& prevLine (contentByLine.getReference (i - 1));
        String& currentLine (contentByLine.getReference (i));
        const String& nextLine (contentByLine.getReference (i + 1));
        String prefix, postfix;

        if (currentLine.substring (0, 6) == "    + ")
        {
            if (prevLine.substring (0, 6) != "    + "
                && prevLine.substring (0, 8) != "    <ol>"
                && prevLine.substring (0, 8) != "    <li>")
                prefix = "    <ol>";
            if (nextLine.substring (0, 6) != "    + ")
                postfix = "    </ol>";

            currentLine = prefix + "    <li>" + currentLine.fromFirstOccurrenceOf("    + ", false, true) + "</li>" + postfix;
        }
        else if (currentLine.substring (0, 2) == "+ ")
        {
            if (prevLine.substring (0, 2) != "+ " 
                && prevLine.substring (0, 4) != "<li>"
                && prevLine.substring (0, 4) != "<ol>")
                prefix = "<ol>";
            if (nextLine.substring (0, 2) != "+ " 
                && nextLine.substring (0, 6) != "    + ")
                postfix = "</ol>";

            currentLine = prefix + "<li>" + currentLine.fromFirstOccurrenceOf ("+ ", false, true) + "</li>" + postfix;
        }
    }

    contentByLine.removeString ("%%__ordered@List@Parse__%%");
    return contentByLine.joinIntoString (newLine).replace("    </ol>", "    </ol></ol>");
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
        if (resultStr.substring (indexBr - 2, indexBr).contains (">"))
            resultStr = resultStr.replaceSection (indexBr, 5, newLine);

        indexBr = resultStr.indexOfIgnoreCase (indexBr + 4, "<br>");
    }

    // clean extra <p> when it's after any html-tag
    /*int indexP = resultStr.indexOfIgnoreCase (0, "<p>");

    while (indexP != -1)
    {
        if (resultStr.substring (indexP - 2, indexP).contains (">")
            && !resultStr.substring (indexP - 8, indexP).contains ("</div>"))
            resultStr = resultStr.replaceSection (indexP, 4, newLine);

        indexP = resultStr.indexOfIgnoreCase (indexP + 3, "<p>");
    }*/

    // clean extra <p> and <br> which is in code-block(s)
    int indexCodeStart = resultStr.indexOfIgnoreCase (0, "<pre><code>");

    while (indexCodeStart != -1 && indexCodeStart + 16 <= resultStr.length())
    {
        const int indexCodeEnd = resultStr.indexOfIgnoreCase (indexCodeStart + 16, "</code></pre>");

        if (indexCodeEnd == -1)
            break;

        const String mdCode (resultStr.substring (indexCodeStart, indexCodeEnd));        
        const String codeHtml (mdCode.replace ("<p>", "\n").replace ("<br>", ""));

        resultStr = resultStr.replaceSection (indexCodeStart, mdCode.length (), codeHtml);
        indexCodeStart = resultStr.indexOfIgnoreCase (indexCodeStart + 16, "<pre><code>");
    }

    // clean extra <br> and <p> which before <pre><code>
    resultStr = resultStr.replace ("<br>\n<pre>", newLine + "<pre>")
        .replace ("<p>\n<pre>", newLine + "<pre>");

    // make the <pre><code> at the same line with the code
    // otherwise the vertical-gap will too wide
    resultStr = resultStr.replace (String ("<pre><code>") + newLine, "<pre><code>");

    // give it a <p> after table, '<hr>' and '</code></pre>'
    resultStr = resultStr.replace (String ("</table>") + newLine, "</table><p>");
    resultStr = resultStr.replace (String ("<hr>") + newLine, "<hr>\n<p>");
    resultStr = resultStr.replace (String ("</code></pre>") + newLine, "</code></pre>\n<p>");

    //DBG (resultStr);
    return resultStr;
}

//=========================================================================
const String Md2Html::copyrightInfo ("<p><hr>\n"
                                     "<div align=\"right\">Powered by "
                                     "<a href=\"http://www.underwaySoft.com/wdtp\""
                                     " target=\"_blank\">WDTP</a> " + 
                                     String (ProjectInfo::versionString) + 
                                     "</div>");


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
    /** Base on the argu Markdown string, parse and return its html string. */
    static const String mdStringToHtml (const String& mdString);

    /* the 6th arg 'cssPath' must end in "/". */
    static const String renderHtmlContent (const String& htmlContentStr,
                                           const File& tplFile,
                                           const String& htmlKeywords,
                                           const String& htmlAuthor,
                                           const String& htmlDescription,
                                           const String& htmlTitle,
                                           const String& cssPath,
                                           const bool codeHighlight);
    
private:
    // call these methods must according to the below order

    /** process table. doesn't support align MD-tag. 
        It must at least 2 rows and 2 columns. 
        
        Markdown form:    
                                     
    headOne | headTwo | headThree    -> at least one ' | ', how many ' | ' then the columns + 1
    ------------------------------   -> begin with and at least 6 '-' in the second line
    row-1-1 | row-1-2 |  row-1-3     -> divide cells by ' | '. if '|'s more than the head line's, the extra will be cut off
    row-2-1 |         |  row-2-3     -> empty between two ' | ' means empty cell
    row-3-1                          -> means this row only has one content-cell which at the very left cell
                                     -> end up with an empty line
    */
    static const String tableParse (const String& mdString);

    /** ``` [code] ```: <pre><code>...</pre></code> */
    static const String codeBlockParse (const String& mdString);

    /** If one "`" in a single line (paragaph), then "`" needn't to escape :) */
    static const String inlineCodeParse (const String& mdString);

    static const String boldParse (const String& mdString);
    static const String italicParse (const String& mdString);

    /** This method will process: <hr>, <blockquote>, <h1> ~ <h6>, <align>.
        All of these Markdown-character(s) must at the first of a line.

        <hr>: "---", must at least 3 "-" and place in a single line.
        otherwise, all of the characters after it will be replaced to "<hr>".

        <blockquote>: "> " 
        <h1>: "# " will auto place in center. 
        My custom define: center: ">|< ", right: ">>> " */
    static const String processByLine (const String& mdString);

    /** http://xxx.com  open in new tab/window, must start with: " http" and end with " "   */
    static const String spaceLinkParse (const String& mdString);
    static const String imageParse (const String& mdString);  /**< ![](media/xxx.jpg) */
    static const String mdLinkParse (const String& mdString); /**< [](http://xxx.com) */    

    // only support two-level nested lists. + for ordered (true), - for unordered (false)
    static const String orderedListParse (const String& mdString, const bool isOrdered); 
    
    static const String cleanUp (const String& mdString);

};



#endif  // MD2HTML_H_INCLUDED

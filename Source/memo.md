# Walden Tips (山湖录)
 
### System Setup
- PropertiesFile (systemFile, the file of system properties) is place in the local 'user-documents/wdtp.sys'.
	- recentFiles: 10 items of the recent-opened.
	- language: The language of UI text (String, language name).
	- uiBackground
	- uiTextColour: it has only 2 colors (dark/light) based on the background of UI.
	- fontName: (haven't been using yet.)
	- fontSize: The font-size of TextEditor.
	- editorFontColour
	- editorBackground

### Project File
- '.wdtp' for the normal project file, the packed project is '.wpck', '.wtpl' is the theme when it has been exported.
- It uses ValueTree (data-model), TreeView (UI) and TreeViewItems (controller) to manage/display/operate all the items which recorded in the project file.
- The structure of project is same as the structure of local-disk file system, however it doesn't include any 'media' or other folder/files.

### Properties of Project ValueTree

#### Structure:

- wdtpProject
	- dir
		- doc

- wdtpProject: root valueTree, also represents the Project Properties.
    - name: it should be 'site' always.
    - tile
	- keywords: keywords of its own (means: keywords of this dir, not all the keywords of this project)
    - description
	- owner
	- contact: matches {{contact}} in a tpl
	- copyright: it could be place at the bottom of a page
	- order: It'll decide the items' display order.
	    - 0: file name
		- 1: title
		- 2: page file name
		- 3: file size
		- 4: create time
		- 5: last modified time
	- dirFirst: 0 for dir first, 1 for doc first
	- ascending: up (0), down (1)
	- showWhat: 
	    - file name (0)
		- title (1)
		- page name (2)
	- tooltip:
	    - file name (0)
		- title (1)
		- page name (2)
	- render: the dir of tpls which is placed in the 'project-dir/themes'.
	- tplFile: for render the index.html of the site, it should be a '.html' tpl-file inside the render dir.
	- js: JavaScript code, it'll site within < head > area of the index.html.
	- ad: The form should be: 'Image-file-name (whitespace) Link-address', one for pre line. It matched the {{ad}} tag from within a tpl.
    - modifyDate
	- needCreateHtml
	- stateAndSelect: the file tree's openness and selected state

- Properities of Dir 
    - name: the name of this dir, it doesn't include any parent's name (path).
	- title: it also be the menu-text if it was set to a site menu
	- keywords
    - description
	- createDate
    - modifyDate
	- isMenu: if true, it will be a site-menu-item
	- tplFile: for render this dir's index.html
	- js: see above
	- needCreateHtml

- Properities of Doc (if a doc's name is 'tips' and belong to the root-item/project-item, it'll be the think-bank source and auto 'hide')
    - name: file name, it doesn't include file-extension and any parent path.
	- title
	- keywords
	- showKeywords
    - description
	- createDate
    - modifyDate
	- isMenu: the doc must not deep than 3 levels (relative to the root ('site/'))
	- tplFile: for render this doc
	- js
	- thumb: extract an image which in this doc as the doc's title-graph or not.
	- thumbName: name of the title-graph (image), it should begin with 'media/'
	- needCreateHtml
	- abbrev: form: (abbrev-name)(space)(original content), 1 per line
	- reviewDate
	- featured
	- hide: doesn't appear in all kinds of list, also don't put its keywords in to statis

### Tags (for developer. These tags could be placed in < head >)
    - {{siteRelativeRootPath}} relative root-path of a page. eg. the page is 'site/dir/subDir/00.html', 
	  its root-path should be '../../../'.
	  Note: the last charactor must be '/'. 
	  This tag could be used in < head >, for link the external css-file

	- {{keywords}}: meta in < head >
	- {{description}}: meta in < head >
	- {{title}}: meta in < head >

### Tempalte Tags (for web/tpl designer. These tags should be placed in < body > area)
	- {{contentTitle}}: Title of the doc/dir.
	- {{contentDesc}}: Description of the doc/dir.
	- {{content}}: Content of the doc.

	- {{siteLogo}}: the logo image should place in 'site/add-in/' and its name should be 'logo.png'.
	- {{siteMenu}}
	- {{siteNavi}}
	- {{siteLink}}: the result is "<a href=../../index.html>ProjectTitle</a>"

	- {{createAndModifyTime}}
	- {{previousAndNext}}
	- {{ad}}: see 'ad' of Project property. Note: ad images should place in 'site/add-in/'.
	- {{random}}: 5 random articles.
	- {{contact}}

	- {{toTop}}: click to the top of the page.
	- {{backPrevious}}: for 'book' tpls, click to back to the parent's index.html.
	- {{bottomCopyright}}

	- {{titleOfDir}}
	- {{blogList}}: for 'blog' index.html. Articles list. 10/page.
	- {{bookList}}: for 'book' index.html, include dir. all in one page.

### 3rd Library Hack

- Solve the confusing of text line break-up of the default Editor when it blends Chinese charactors and English words:
    - L227 in TextEditor, the 'if' statement need comment `text.isWhitespace() && `
- Increase line-spacing:
    - Same class above, L481 change the original to `lineHeight = section->font.getHeight() + 4;`
	- L540 change the original to `atomX, (float) roundToInt (lineY + lineHeight - maxDescent - 2));`
	- L573 change the original to `atomX, (float) roundToInt (lineY + lineHeight - maxDescent - 2));`
	- L560-561 change to (underline change to backgound highlight):
```
	//g.reduceClipRegion (Rectangle<int> (startX, baselineY, endX - startX, 1));
    //g.fillCheckerBoard (Rectangle<int> (endX, baselineY + 1), 3, 1, colour, Colours::transparentBlack);
    g.setColour (Colours::cyan.withAlpha (0.95f));
    g.fillRoundedRectangle (startX - 1.f, baselineY - lineHeight + 10.5f, endX - startX + 2.f, lineHeight - 2.5f, 5.f);
    drawSelectedText (g, underline, colour);

	// for lineSpacing backup
	/* g.fillRoundedRectangle (startX - 1.f, baselineY - lineHeight + 4.f,
                            endX - startX + 2.f, lineHeight + 1.f, 5.f);*/
```

### Shortcut Assign

- F3: select next selection
- Shift + F3: select previous selection
- F5: regenerate the current page and refresh it
- F6: regenarate all the changed docs and dirs (not the whole site)

- Ctrl + A: select all content (text mark syntax)
- Ctrl + B: bold (text mark syntax)
- Ctrl + C: copy 
- Ctrl + D: switch silent/full mode
- Ctrl + E: 
- Ctrl + F: active search
- Ctrl + G: popup tips for the selected text (if it has)
- Ctrl + H: insert text hyper link (text mark syntax)
- Ctrl + I: italic (text mark syntax)
- Ctrl + J: popup outline menu of editor
- Ctrl + K: insert code block (text mark syntax)
- Ctrl + L: insert inline code (text mark syntax)
- Ctrl + M: insert images (text mark syntax)
- Ctrl + N: align center (text mark syntax)
- Ctrl + O: insert author (text mark syntax)
- Ctrl + P: insert caption (text mark syntax)
- Ctrl + Q: 
- Ctrl + R: align right (text mark syntax)
- Ctrl + S: switch edit/preview 
- Ctrl + T: insert table (text mark syntax)
- Ctrl + U: highlight text (text mark syntax)
- Ctrl + V: intelligent paste
- Ctrl + W: audio record (text mark syntax)
- Ctrl + X: cut
- Ctrl + Y: redo
- Ctrl + Z: undo

- Punctuation Matching...

### About Compile

On Windows, using VS2015 to open and compile the VS2012-project, which exported by Projucer.
In its settings (Projucer-VS2012), the 'Platform Toolset' should be set to 'V110_XP' (for downward compatibility).

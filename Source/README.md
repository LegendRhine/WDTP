﻿# Walden Trip (山·湖·路)

![WDTP](http://underwaysoft.com/works/wdtp/media/wdtp-main.jpg)
 
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
	- imageEditor: default application for edit image file
	- audioEditor: default application for edit audiao file

### Project File
- '.wdtp' for the normal project file, the packed project is '.wpck', '.wtpl' is the theme when it has been exported.
- It uses ValueTree (data-model), TreeView (UI) and TreeViewItems (controller) to manage/display/operate all the items which recorded in the project file.
- The structure of project is same as the structure of local-disk file system, however it doesn't include any 'media' or other folder/files.

### Properties of Project ValueTree

#### Structure:

- wdtpProject
	- resource
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
	- ad: The form should be: 'Image-file-name (whitespace) Link-address', one for pre row. It matched the {{ad}} tag from within a tpl.
    - modifyDate
	- needCreateHtml
	- stateAndSelect: the file tree's openness and selected state
	- resources: the form should be: 'name = path', one for pre row, allow empty line between rows.
	  'name': specified by user and will be showed in system menu
	  'path': full path of the resource

- Dir: child tree(s) of wdtoProject
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

- Doc: child tree(s) of Dir (if a doc's name is 'tips' and belong to the root-item/project-item, it'll be the think-bank source and auto 'hide')
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
	- archive: can't edit anymore, can't replace anything by default

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

### Shortcut Assign (totally 49+)

- F1: help for markup syntax
- Ctrl + F1: add the selected content to tips-bank
- F2: sum all numbers of current row
- Ctrl + F2: average all numbers of current row
- F3: select next selection
- Shift + F3: select previous selection
- F4: sum all numbers of current column (table special)
- Ctrl + F4: average all numbers of current column (table special)
- F5: regenerate the current page and refresh it
- F6: regenarate all the changed docs and dirs (not the whole site)
- F7: insert timeline
- F8: insert table of contents (TOC)
- F9: insert endnote
- F10: insert date and time of current

- Ctrl + 1: popup right click menu (both for md editor and theme editor)
- Ctrl + 2: show keywords table of current project
- Ctrl + 3: insert hybrid markup
- Ctrl + 4: postil the selected text
- Ctrl + 5: insert back to top markup
- Ctrl + 6: pick the selected text as the doc's title
- Ctrl + 7: pick the selected text as the doc's description
- Ctrl + 8: add the selected text to the doc's keywords list
- Ctrl + 9: reset the editor's font size, font and background color to system default

- Ctrl + A: select all content (text markup syntax)
- Ctrl + B: bold (text markup syntax)
- Ctrl + C: copy 
- Ctrl + D: switch silent/full mode
- Ctrl + E: popup a dialog to insert a text hyperlink
- Ctrl + F: active search
- Ctrl + G: popup tips for the selected text (if it has)
- Ctrl + H: hide (minimize) the app
- Ctrl + I: italic (text markup syntax)
- Ctrl + J: popup outline menu of editor
- Ctrl + K: insert code block (text markup syntax)
- Ctrl + L: insert inline code (text markup syntax)
- Ctrl + M: insert images (text markup syntax)
- Ctrl + N: align center (text markup syntax)
- Ctrl + O: insert author (text markup syntax)
- Ctrl + P: insert caption (text markup syntax)
- Ctrl + Q: exit the app
- Ctrl + R: align right (text markup syntax)
- Ctrl + S: switch edit/preview 
- Ctrl + T: insert table (text markup syntax)
- Ctrl + U: highlight text (text markup syntax)
- Ctrl + V: intelligent paste
- Ctrl + W: audio record (text markup syntax)
- Ctrl + X: cut
- Ctrl + Y: redo
- Ctrl + Z: undo

- Punctuation matching and auto popup tips menu...

### 3rd Library and 3rd Lib-hacked
- JUCE: https://juce.com
- ASIO: https://steinberg.net
- JUCE lib-hacked source files: Source/libHackBackup

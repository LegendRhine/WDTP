/*
  ==============================================================================

    DocTreeViewItem.cpp
    Created: 16 Sep 2016 5:20:49am
    Author:  SwingCoder

  ==============================================================================
*/

#include "WdtpHeader.h"

extern PropertiesFile* systemFile;

//=================================================================================================
DocTreeViewItem::DocTreeViewItem (const ValueTree& tree_,
                                  FileTreeContainer* container,
                                  ItemSorter* itemSorter) :
    tree (tree_),
    treeContainer (container),
    sorter (itemSorter)
{
    jassert (treeContainer != nullptr);

    // highlight for the whole line
    //setDrawsInLeftMargin (true); 
    setLinesDrawnForSubItems (true);
    tree.addListener (this);
}

//=================================================================================================
DocTreeViewItem::~DocTreeViewItem()
{
    tree.removeListener (this);
    treeContainer = nullptr;
}

//=================================================================================================
bool DocTreeViewItem::mightContainSubItems()
{
    return tree.getNumChildren() > 0;
}

//=================================================================================================
String DocTreeViewItem::getUniqueName() const
{
    return tree.getProperty ("name").toString();
}

//=================================================================================================
void DocTreeViewItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen && getNumSubItems() == 0)
        refreshDisplay();
    else
        clearSubItems();
}

//=================================================================================================
void DocTreeViewItem::paintItem (Graphics& g, int width, int height)
{
    g.setFont (SwingUtilities::getFontSize() - 2.f);
    int leftGap = 4;
    Colour c (Colour::fromString (systemFile->getValue ("uiTextColour")));

    if (!getMdFileOrDir (tree).exists())
        c = Colours::red;

    g.setColour (c);

    jassert (sorter != nullptr);
    String itemName;

    if (sorter->getShowWhat() == 0 && tree.getType().toString() != "wdtpProject")  // file name
        itemName = tree.getProperty ("name").toString();

    else if (sorter->getShowWhat() == 1 || tree.getType().toString() == "wdtpProject") // title or intro
        itemName = tree.getProperty ("title").toString();

    // mark of doc and dir item
    String markStr;
    const bool needGenerate = (bool)tree.getProperty ("needCreateHtml");

    if (!getMdFileOrDir (tree).exists())
        markStr = CharPointer_UTF8 (tree.getType().toString() == "doc"
                                    ? "\xe2\x97\x8f " : "\xe2\x96\xa0 ");
    else
        markStr = CharPointer_UTF8 (needGenerate ? "* " :
        (tree.getType().toString() == "doc") ? "\xe2\x97\x8f " : "\xe2\x96\xa0 ");

    g.drawText (markStr + itemName, leftGap, 0, width - 4, height, Justification::centredLeft, true);
}

//=================================================================================================
const File DocTreeViewItem::getMdFileOrDir (const ValueTree& tree)
{
    if (!tree.isValid())
        return File::nonexistent;

    const File& root (FileTreeContainer::projectFile.getSiblingFile ("docs"));

    if (tree.getType().toString() == "wdtpProject")
    {
        return root;
    }
    else
    {
        String path = tree.getProperty ("name").toString();
        ValueTree t (tree);

        while (t.getParent().isValid() && t.getParent().getType().toString() != "wdtpProject")
        {
            t = t.getParent();
            path = t.getProperty ("name").toString() + File::separator + path;
        }

        if (tree.getType().toString() == "doc")
            path += ".md";

        return root.getChildFile (path);
    }
}

//=================================================================================================
const File DocTreeViewItem::getHtmlFileOrDir (const File& mdFileOrDir)
{
    if (mdFileOrDir.isDirectory())
        return File (mdFileOrDir.getFullPathName().replace ("docs", "site")).getChildFile ("index.html");
    else
        return File (mdFileOrDir.getFullPathName().replace ("docs", "site")).withFileExtension ("html");
}

//=================================================================================================
const File DocTreeViewItem::getHtmlFileOrDir (const ValueTree& tree)
{
    return getHtmlFileOrDir (getMdFileOrDir (tree));
}

//=================================================================================================
const int DocTreeViewItem::getHtmlMediaFiles (const File& htmlFile, Array<File>& files)
{
    if (!htmlFile.existsAsFile())
        return 0;

    const String htmlStr (htmlFile.loadFileAsString());

    if (htmlStr.trim() == String())
        return 0;

    int indexStart = htmlStr.indexOfIgnoreCase (0, "src=\"");
    int indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");

    while (indexStart != -1 && indexEnd != -1)
    {
        const File& f (htmlFile.getSiblingFile ("media")
                       .getChildFile (htmlStr.substring (indexStart + 11, indexEnd) // 11: src="media/
                                      .trimCharactersAtStart ("/")));

        if (f.existsAsFile() && f.getSize() > 0)
            files.add (f);

        indexStart = htmlStr.indexOfIgnoreCase (indexEnd + 2, "src=\"");

        if (indexStart != -1)
            indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");
    }

    return files.size();
}

//=================================================================================================
const int DocTreeViewItem::getMdMediaFiles (const File& doc, Array<File>& files)
{
    if (doc.isDirectory() || !doc.getSiblingFile ("media").exists())
        return 0;

    String htmlStr = Md2Html::imageParse (doc.loadFileAsString());
    htmlStr = Md2Html::audioParse (htmlStr);

    if (htmlStr.trim().isEmpty())
        return 0;

    int indexStart = htmlStr.indexOfIgnoreCase (0, "src=\"");
    int indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");

    while (indexStart != -1 && indexEnd != -1)
    {
        const File& f (doc.getSiblingFile ("media")
                       .getChildFile (htmlStr.substring (indexStart + 11, indexEnd) // 11: src="media/
                                      .trimCharactersAtStart ("/")));

        if (f.existsAsFile() && f.getSize() > 0)
            files.add (f);

        indexStart = htmlStr.indexOfIgnoreCase (indexEnd + 2, "src=\"");

        if (indexStart != -1)
            indexEnd = htmlStr.indexOfIgnoreCase (indexStart + 5, "\"");
    }

    return files.size();
}

//=================================================================================================
void DocTreeViewItem::needCreate (ValueTree tree)
{
    if (!tree.isValid())
        return;

    ValueTree parentTree = tree;
    const String modifyDate (SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), true));

    parentTree.setProperty ("needCreateHtml", true, nullptr);
    parentTree.setProperty ("modifyDate", modifyDate, nullptr);

    while (parentTree.getParent().isValid())
    {
        parentTree = parentTree.getParent();
        parentTree.setProperty ("needCreateHtml", true, nullptr);
        parentTree.setProperty ("modifyDate", modifyDate, nullptr);
    }

    allChildrenNeedCreate (tree);
}

//=================================================================================================
void DocTreeViewItem::allChildrenNeedCreate (ValueTree tree)
{
    tree.setProperty ("needCreateHtml", true, nullptr);

    for (int i = tree.getNumChildren(); --i >= 0; )
    {
        tree.getChild (i).setProperty ("needCreateHtml", true, nullptr);
        allChildrenNeedCreate (tree.getChild (i));
    }
}

//=================================================================================================
void DocTreeViewItem::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        EditAndPreview* editArea = treeContainer->getEditAndPreview();

        // set properties on the right side
        if (tree.getType().toString() == "doc")
            editArea->setDocProperties (tree);

        else if (tree.getType().toString() == "dir")
            editArea->setDirProperties (tree);

        else // root
            editArea->setProjectProperties (tree);

        // this must after setXxxProperties() since
        // startWork() will update the word count of this doc to setup-panel
        editArea->startWork (tree);
        
        // cancel existed temp-underline of the editor 
        // the lines will still remain after ran 'statistics' and then switched to another doc
        Array<Range<int>> nonUnderline;
        nonUnderline.add (Range<int> (0, 0));
        editArea->getEditor()->setTemporaryUnderlining ((nonUnderline));
        
        treeContainer->setIdentityOfLastSelectedItem (getItemIdentifierString());
    }
}

//=================================================================================================
// for right-click menu
void DocTreeViewItem::itemClicked (const MouseEvent& e)
{
    const bool exist = getMdFileOrDir (tree).exists();
    const bool isDoc = (tree.getType().toString() == "doc");
    const bool isRoot = (tree.getType().toString() == "wdtpProject");
    const bool onlyOneSelected = getOwnerView()->getNumSelectedItems() == 1;
    const bool isCrossPaste = File::getSpecialLocation (File::tempDirectory).getChildFile ("wdtpCrossCopy").existsAsFile();

    jassert (sorter != nullptr);

    // right click
    if (e.mods.isPopupMenu())
    {
        PopupMenu m;

        // at most 3 level dir, but it can be done through double click for any deep of level
        m.addItem (newDir, TRANS ("New Folder..."), exist && !isDoc && onlyOneSelected
                   && !tree.getParent().getParent().getParent().isValid());  
        m.addItem (newDoc, TRANS ("New Document..."), exist && !isDoc && onlyOneSelected);

        // import various external data...
        PopupMenu importMenu;
        importMenu.addItem (importTextDocs, TRANS ("Text Doc(s)..."), exist && !isDoc && onlyOneSelected);

        m.addSubMenu (TRANS ("Import External Data"), importMenu);
        m.addSeparator();

        PopupMenu packMenu;
        packMenu.addItem (packHtmls, TRANS ("Pack Without Medias"), exist && onlyOneSelected && !isDoc);
        packMenu.addItem (packMedias, TRANS ("Pack Without Htmls"), exist && onlyOneSelected && !isDoc);
        packMenu.addSeparator();
        packMenu.addItem (packWholeSite, TRANS ("Pack All Data"), exist && onlyOneSelected && !isDoc);

        m.addSubMenu (TRANS ("Pack Site Data"), packMenu);
        m.addSeparator();

        m.addItem (exportTextDoc, TRANS ("Export Text Doc..."), exist && onlyOneSelected && isDoc);
        m.addItem (exportDocs, TRANS ("Export Single Big-Html..."), exist && onlyOneSelected && !isDoc);
        m.addSeparator();

        m.addItem (dataStatis, TRANS ("Statistics..."), exist && onlyOneSelected);
        m.addItem (keywordsTable, TRANS ("Keywords Table") + "...", exist && onlyOneSelected && !isDoc);
        m.addSeparator();

        m.addItem (getItemPath, TRANS ("Get Path"), exist && onlyOneSelected);
        m.addItem (copyForAnotherProject, TRANS ("Copy For Another Project"), isDoc && onlyOneSelected);
        m.addItem (pasteFromAnotherProject, TRANS ("Paste From Another Project"), !isDoc && onlyOneSelected && isCrossPaste);
        m.addSeparator();

        PopupMenu sortMenu;
        sortMenu.addItem (100, TRANS ("File Name"), true, sorter->getOrder() == 0);
        sortMenu.addItem (101, TRANS ("Title"), true, sorter->getOrder() == 1);
        sortMenu.addItem (103, TRANS ("File Size"), true, sorter->getOrder() == 3);
        sortMenu.addItem (104, TRANS ("Create Time"), true, sorter->getOrder() == 4);
        sortMenu.addItem (105, TRANS ("Last Modified"), true, sorter->getOrder() == 5);
        sortMenu.addSeparator();
        sortMenu.addItem (106, TRANS ("Ascending Order"), true, sorter->getAscending() == 0);
        sortMenu.addItem (107, TRANS ("Folder First"), true, sorter->getWhichFirst() == 0);

        m.addSubMenu (TRANS ("Sort by"), sortMenu);

        PopupMenu showedAsMenu;
        showedAsMenu.addItem (200, TRANS ("File Name"), true, sorter->getShowWhat() == 0);
        showedAsMenu.addItem (201, TRANS ("Title"), true, sorter->getShowWhat() == 1);

        m.addSubMenu (TRANS ("Showed as"), showedAsMenu);

        PopupMenu tooltipAsMenu;
        tooltipAsMenu.addItem (300, TRANS ("File Path"), true, sorter->getTooltipToShow() == 0);
        tooltipAsMenu.addItem (301, TRANS ("Title"), true, sorter->getTooltipToShow() == 1);
        tooltipAsMenu.addItem (302, TRANS ("Keywords"), true, sorter->getTooltipToShow() == 2);
        tooltipAsMenu.addItem (303, TRANS ("Description"), true, sorter->getTooltipToShow() == 3);

        m.addSubMenu (TRANS ("Tooltip for"), tooltipAsMenu);
        m.addSeparator();

        m.addItem (replaceIn, TRANS ("Replace Content..."), exist && onlyOneSelected);
        m.addSeparator();

        m.addItem (rename, TRANS ("Rename..."), !isRoot && onlyOneSelected);
        m.addItem (deleteThis, TRANS ("Delete..."), !isRoot);
        m.addSeparator();

        m.addItem (viewInFinder, TRANS ("View in Explorer/Finder..."), exist && onlyOneSelected);
        m.addItem (openInExEditor, TRANS ("Open in External Editor..."), exist && onlyOneSelected);
        m.addItem (browseInEx, TRANS ("Browse in External Browser..."), exist && onlyOneSelected);

        menuPerform (m.show());
    }
}

//=================================================================================================
void DocTreeViewItem::itemDoubleClicked (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        if (tree.getType().toString() == "doc")
            renameSelectedItem();
        else if (tree.getType().toString() == "dir")
            createNewDocument();
        else
            createNewFolder();
    }
}

//=================================================================================================
void DocTreeViewItem::menuPerform (const int index)
{
    jassert (sorter != nullptr);

         if (index == newDir)                   createNewFolder();
    else if (index == newDoc)                   createNewDocument();
    else if (index == importTextDocs)           importExternalDocs();
    else if (index == packHtmls)                packSiteData (true, false);
    else if (index == packMedias)               packSiteData (false, true);
    else if (index == packWholeSite)            packSiteData (true, true);
    else if (index == exportTextDoc)            exportAsTextDoc();
    else if (index == exportDocs)               exportAsHtml();
    else if (index == dataStatis)               statistics();
    else if (index == keywordsTable)            showKeywordsTable();
    else if (index == getItemPath)              getPath();
    else if (index == copyForAnotherProject)    crossProjectCopy();
    else if (index == pasteFromAnotherProject)  crossProjectPaste();
    else if (index == replaceIn)                replaceContent();
    else if (index == rename)                   renameSelectedItem();
    else if (index == deleteThis)               deleteSelected();
    else if (index == viewInFinder)             getMdFileOrDir (tree).revealToUser();
    else if (index == openInExEditor)           getMdFileOrDir (tree).startAsProcess();
    else if (index == browseInEx)               getHtmlFileOrDir (tree).startAsProcess();

    // sort and show what...
    else if (index >= 100 && index <= 105)  sorter->setOrder (index - 100);
    else if (index == 106)                  sorter->setAscending ((sorter->getAscending() == 0) ? 1 : 0);
    else if (index == 107)                  sorter->setWhichFirst ((sorter->getWhichFirst() == 0) ? 1 : 0);
    else if (index >= 200 && index <= 202)  sorter->setShowWhat (index - 200);
    else if (index >= 300 && index <= 303)  sorter->setTooltipToShow (index - 300);
}

//=================================================================================================
void DocTreeViewItem::packSiteData (const bool includeHtmls, const bool includeMedias)
{
    jassert (includeHtmls || includeMedias);  // pack nothing?
    TopToolBar::generateHtmlFilesIfNeeded (tree);

    const File thisDir (getHtmlFileOrDir (tree).getParentDirectory());
    ZipFile::Builder builder;
    Array<File> htmlFiles;
    thisDir.findChildFiles (htmlFiles, File::findFiles, true, "*");

    for (int i = htmlFiles.size(); --i >= 0; )
    {
        if (htmlFiles[i].getFileName() != "desktop.ini"
            && htmlFiles[i].getFileName() != ".DS_Store"
            && htmlFiles[i].getFileExtension() != ".zip")
        {
            String pathStr (htmlFiles[i].getFullPathName()
                            .fromFirstOccurrenceOf (thisDir.getFullPathName()
                                                    + File::separatorString, false, false));

            pathStr = pathStr.replace (File::separatorString, "/");

            if (includeHtmls && !includeMedias && htmlFiles[i].getFileExtension() != ".html")
                continue;

            else if (!includeHtmls && includeMedias && htmlFiles[i].getFileExtension() == ".html")
                continue;

            // Note: it'll include 'add-in' when pack the root-item
            // and perhaps include other data. for example:
            // user created or copied some data
            builder.addFile (htmlFiles[i], 9, pathStr);
        }
    }

    // write to zip file
    String postfix;

    if (includeHtmls && !includeMedias)
        postfix = "-htmls";
    else if (!includeHtmls && includeMedias)
        postfix = "-medias";

    const File packZipFile (thisDir.getChildFile (tree.getProperty ("name").toString() + postfix + ".zip"));
    packZipFile.deleteFile();
    packZipFile.create();

    ScopedPointer<FileOutputStream> out = packZipFile.createOutputStream();

    if (builder.writeToStream (*out, nullptr))
    {
        out->flush();
        out = nullptr;
        SHOW_MESSAGE (TRANS ("Pack successful!"));

        packZipFile.revealToUser();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Somehow pack failed."));
    }
}

//=================================================================================================
void DocTreeViewItem::exportAsTextDoc()
{
    FileChooser fc (TRANS ("Export document"),
                    File::getSpecialLocation (File::userDocumentsDirectory)
                    .getChildFile (tree.getProperty ("name").toString() + ".txt"),
                    "*.txt", true);

    if (!fc.browseForFileToSave (false))
        return;

    File textFile (fc.getResult());
    textFile = textFile.getSiblingFile (SwingUtilities::getValidFileName (textFile.getFileNameWithoutExtension()));

    if (!textFile.hasFileExtension ("txt"))
        textFile = textFile.withFileExtension ("txt");

    // overwrite or not whether it has been there
    if (textFile.existsAsFile() &&
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                       TRANS ("This file already exists, want to overwrite it?")))
    {
        return;
    }

    if (!textFile.deleteFile())
    {
        SHOW_MESSAGE (TRANS ("Can't write to this file! "));
        return;
    }

    textFile.create();

    if (getMdFileOrDir (tree).copyFileTo (textFile))
        textFile.startAsProcess();
    else
        SHOW_MESSAGE (TRANS ("Export Failed."));
}

//=================================================================================================
void DocTreeViewItem::renameSelectedItem()
{
    const File& docFileOrDir (getMdFileOrDir (tree));

    AlertWindow dialog (TRANS ("Rename the selected item"), TRANS ("Please input the new name."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", docFileOrDir.getFileNameWithoutExtension());
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop())
    {
        const String inputStr (dialog.getTextEditor ("name")->getText());
        String newName (SwingUtilities::getValidFileName (inputStr));

        if (newName == tree.getProperty ("name").toString())
            return;

        if (newName.isEmpty())
            newName = TRANS ("Untitled");
        else if (newName == "index" || newName == "media"
                 || newName == "add-in" || newName == "docs" || newName == "site")
            newName += "-2";

        File newDocFile (docFileOrDir.getSiblingFile (newName + (docFileOrDir.isDirectory() ? String() : ".md")));
        newDocFile = newDocFile.getNonexistentSibling (true);

        if (docFileOrDir.moveFileTo (newDocFile))
        {
            // save the project file
            tree.setProperty ("name", newDocFile.getFileNameWithoutExtension(), nullptr);
            needCreate (tree.getType().toString() == "doc" ? tree : FileTreeContainer::projectTree);

            // rename the site dir or html-file
            File siteOldFile;
            File siteNewFile;

            if (newDocFile.isDirectory())
            {
                siteOldFile = File (docFileOrDir.getFullPathName().replace ("docs", "site"));
                siteNewFile = File (siteOldFile.getSiblingFile (newName)).getNonexistentSibling (true);
            }
            else
            {
                siteOldFile = File (docFileOrDir.getFullPathName().replace ("docs", "site")).withFileExtension ("html");
                siteNewFile = File (siteOldFile.getSiblingFile (newName + ".html")).getNonexistentSibling (true);
            }

            if (siteOldFile.exists())
                siteOldFile.moveFileTo (siteNewFile);

            // here must re-select this item
            setSelected (false, false, dontSendNotification);
            setSelected (true, true);

            FileTreeContainer::saveProject();
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't rename this item. "));
        }
    }
}

//=================================================================================================
void DocTreeViewItem::exportAsHtml()
{
    FileChooser fc (TRANS ("Export document(s) as a single html file..."), 
                    File::getSpecialLocation (File::userDocumentsDirectory)
                    .getChildFile (tree.getProperty ("name").toString() + ".html"),
        "*.html", true);

    if (!fc.browseForFileToSave (false))
        return;

    File htmlFile (fc.getResult());
    htmlFile = htmlFile.getSiblingFile (SwingUtilities::getValidFileName (htmlFile.getFileNameWithoutExtension()));

    if (!htmlFile.hasFileExtension ("html"))
        htmlFile = htmlFile.withFileExtension ("html");

    // overwrite or not whether it has been there
    if (htmlFile.existsAsFile() &&
        !AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Message"),
                                       TRANS ("This file already exists, want to overwrite it?")))
    {
        return;
    }

    if (!htmlFile.deleteFile())
    {
        SHOW_MESSAGE (TRANS ("Can't write to this file! "));
        return;
    }

    htmlFile.create();
    
    // create a md-text file temporarily, then gathering all medias of this dir
    File mdFile (File::getSpecialLocation (File::tempDirectory).getSiblingFile ("wdtpExport.md"));
    mdFile.deleteFile();
    mdFile.create();
    Array<File> allMedias;
            
    if (getDirDocsAndAllMedias (this, mdFile, allMedias))
    {
        // add title, description and [TOC], then generate the html file
        const String titleStr (tree.getProperty ("title").toString());
        const String descStr (tree.getProperty ("description").toString());
        const String contentStr = "# " + titleStr + newLine + newLine
            + descStr + newLine + newLine
            + "[TOC]" + newLine + newLine + mdFile.loadFileAsString();

        htmlFile.appendText ("<!doctype html>\n"
                             "<html lang = \"en\">\n"
                             "<head>\n"
                             "<meta charset = \"UTF-8\">\n"
                             "  <link rel = \"stylesheet\" type = \"text/css\" href = \"media/style.css\"/>\n"
                             "  <script src = \"media/hl.js\"></script>\n"
                             "  <script>hljs.initHighlightingOnLoad(); </script>\n"
                             "<title>" + titleStr + "</title>\n"
                             "</head>\n"
                             "<body>\n\n" + Md2Html::mdStringToHtml (contentStr) + "\n\n"
                             "</body>\n"
                             "</html>");

        // create its media folder and copy all medias into it
        const File mediaDir (htmlFile.getSiblingFile ("media"));
        mediaDir.createDirectory();

        for (int i = allMedias.size(); --i >= 0; )
        {
            const File targetFile (mediaDir.getChildFile (allMedias[i].getFileName()));
            targetFile.create();
            allMedias[i].copyFileTo (targetFile);
        }

        // copy stylesheet into the media folder
        const File& styleCss (FileTreeContainer::projectFile.getSiblingFile ("site")
                               .getChildFile ("add-in").getChildFile ("style.css"));

        const File& hlJs (FileTreeContainer::projectFile.getSiblingFile ("site")
                          .getChildFile ("add-in").getChildFile ("hl.js"));

        const File targetCss (mediaDir.getChildFile (styleCss.getFileName()));
        const File targetJs (mediaDir.getChildFile (hlJs.getFileName()));
        targetCss.create();
        targetJs.create();

        styleCss.copyFileTo (targetCss);
        hlJs.copyFileTo (targetJs);

        // when all above has done, browse it in an external browser..
        htmlFile.startAsProcess();
    }
    else
    {
        SHOW_MESSAGE (TRANS ("Export Failed."));
    }

    mdFile.deleteFile();
}

//=================================================================================================
void DocTreeViewItem::createNewDocument()
{
    // can't create any doc under a doc!
    jassert (tree.getType().toString() != "doc");

    AlertWindow dialog (TRANS ("Create a new document"), TRANS ("Please input the new doc's name."),
                        AlertWindow::InfoIcon);
    const String fileName (SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), true));

    dialog.addTextEditor ("name", fileName.dropLastCharacters (9).replace (".", "-"));
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop())
    {
        const String& docName (SwingUtilities::getValidFileName (dialog.getTextEditor ("name")->getText()));
        createDoc (docName, "# ", true);
        FileTreeContainer::saveProject();
    }
}

//=================================================================================================
const File DocTreeViewItem::createDoc (const String& docName, 
                                       const String& content, 
                                       const bool selectAfterCreated)
{
    String docFileName (docName);

    if (docFileName.isEmpty())
        docFileName = TRANS ("Untitled");
    else if (docFileName == "index")
        docFileName = "index-2";
    else if (docFileName == "site")
        docFileName = "site-doc";
    else if (docFileName == "docs")
        docFileName = "docs-1";
    else if (docFileName == "media")
        docFileName = "media-doc";

    // create this doc on disk
    const File& thisDoc (getMdFileOrDir (tree).getChildFile (docFileName + ".md")
                         .getNonexistentSibling (false));
    thisDoc.create();
    thisDoc.appendText (content);

    // valueTree of this doc
    ValueTree docTree ("doc");
    docTree.setProperty ("name", thisDoc.getFileNameWithoutExtension(), nullptr);
    docTree.setProperty ("title", thisDoc.getFileNameWithoutExtension(), nullptr);
    docTree.setProperty ("description", TRANS ("Description of this doc..."), nullptr);
    docTree.setProperty ("keywords", String(), nullptr);
    docTree.setProperty ("isMenu", false, nullptr);
    docTree.setProperty ("thumb", true, nullptr);
    docTree.setProperty ("tplFile", "article.html", nullptr);
    docTree.setProperty ("createDate",
                         SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), true),
                         nullptr);
    docTree.setProperty ("reviewDate",
                         SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString (2), true),
                         nullptr);

    // must update this tree before show this new item
    tree.removeListener (this);
    tree.addChild (docTree, 0, nullptr);
    needCreate (docTree);
    tree.addListener (this);

    // add and select the new item 
    setOpen (true);
    DocTreeViewItem* docItem = new DocTreeViewItem (docTree, treeContainer, sorter);
    addSubItemSorted (*sorter, docItem);
    docItem->setSelected (selectAfterCreated, selectAfterCreated);

    return thisDoc;
}

//=================================================================================================
void DocTreeViewItem::createNewFolder()
{
    // can't create any dir under a doc!
    jassert (tree.getType().toString() != "doc");

    AlertWindow dialog (TRANS ("Create a new folder"), TRANS ("Please input the new folder's name."),
                        AlertWindow::InfoIcon);

    dialog.addTextEditor ("name", String());
    dialog.addButton (TRANS ("OK"), 0, KeyPress (KeyPress::returnKey));
    dialog.addButton (TRANS ("Cancel"), 1, KeyPress (KeyPress::escapeKey));

    if (0 == dialog.runModalLoop())
    {
        String dirName (SwingUtilities::getValidFileName (dialog.getTextEditor ("name")->getText()));

        if (dirName.isEmpty())
            dirName = TRANS ("New folder");
        else if (dirName == "media" || dirName == "add-in" || dirName == "docs" || dirName == "site")
            dirName += "-2";

        // create this dir on disk
        File thisDir (getMdFileOrDir (tree).getChildFile (dirName).getNonexistentSibling (false));
        thisDir.createDirectory();

        // dir tree
        ValueTree dirTree ("dir");
        dirTree.setProperty ("name", thisDir.getFileNameWithoutExtension(), nullptr);
        dirTree.setProperty ("title", thisDir.getFileNameWithoutExtension(), nullptr);
        dirTree.setProperty ("isMenu", true, nullptr);
        dirTree.setProperty ("tplFile", "category.html", nullptr);
        dirTree.setProperty ("createDate",
                             SwingUtilities::getTimeStringWithSeparator (SwingUtilities::getCurrentTimeString(), true),
                             nullptr);

        // must update this tree before show this new item
        tree.removeListener (this);
        tree.addChild (dirTree, 0, nullptr);
        needCreate (FileTreeContainer::projectTree);
        tree.addListener (this);

        // this item add the new dir, then select the index item 
        setOpen (true);
        DocTreeViewItem* dirItem = new DocTreeViewItem (dirTree, treeContainer, sorter);
        addSubItemSorted (*sorter, dirItem);
        dirItem->setSelected (true, true);

        FileTreeContainer::saveProject();
    }
}

//=================================================================================================
void DocTreeViewItem::deleteSelected()
{
    // get all selected items
    OwnedArray<ValueTree> selectedTrees;
    TreeView* treeView = getOwnerView();

    for (int i = 0; i < treeView->getNumSelectedItems(); ++i)
    {
        const DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView->getSelectedItem (i));
        jassert (item != nullptr);

        selectedTrees.add (new ValueTree (item->tree));
    }

    if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, TRANS ("Confirm"),
                                      TRANS ("Do you really want to delete ") +
                                      String (selectedTrees.size()) +
                                      TRANS (" selected item(s)? ") + newLine +
                                      TRANS ("This operation CANNOT be undone! ") + newLine + newLine +
                                      TRANS ("Tips: The deleted items could be found in OS's Recycle Bin. ")))
    {
        treeContainer->getTreeView().getRootItem()->setSelected (true, false);

        // delete one by one
        for (int i = selectedTrees.size(); --i >= 0; )
        {
            ValueTree& v = *selectedTrees.getUnchecked (i);
            needCreate (v.getType().toString() == "doc" ? v : FileTreeContainer::projectTree);

            if (v.getParent().isValid())
            {
                const File mdFile (getMdFileOrDir (v));
                const File siteFile (getHtmlFileOrDir (mdFile));

                // here should delete its media-file(s) first
                Array<File> htmlMedias;

                for (int j = getHtmlMediaFiles (siteFile, htmlMedias); --j >= 0; )
                {
                    const String mediaFileName (htmlMedias[j].getFullPathName());
                    htmlMedias[j].moveToTrash();
                    File (mediaFileName.replace ("site", "docs")).moveToTrash();
                }

                // delete the two-files
                mdFile.moveToTrash();
                siteFile.deleteRecursively();
                v.getParent().removeChild (v, nullptr);
            }
        }

        FileTreeContainer::saveProject();
    }
}

//=================================================================================================
void DocTreeViewItem::statistics()
{
    String staStr;
    const bool isDoc = (tree.getType().toString() == "doc");

    if (isDoc)
    {
        int words = 0;
        int imgNums = 0;
        getWordsAndImgNumsInDoc (tree, words, imgNums);

        staStr = TRANS ("File: ") + tree.getProperty ("name").toString() + newLine
            + TRANS ("Title: ") + tree.getProperty ("title").toString() + newLine
            + TRANS ("Words: ") + String (words) + "  "
            + TRANS ("Images: ") + String (imgNums);
    }
    else
    {
        int docNums = 0;
        HtmlProcessor::getDocNumbersOfTheDir (tree, docNums);

        int dirNums = -1;  // non-include itself
        int totalWords = 0;
        int totalImgs = 0;
        const bool isDir = (tree.getType().toString() == "dir");

        statis (tree, dirNums, totalWords, totalImgs);
        staStr = (isDir ? (TRANS ("Dir: ") + tree.getProperty ("name").toString() + "  ") : String())
            + (isDir ? TRANS ("Title: ") : TRANS ("Project: "))
            + tree.getProperty ("title").toString() + newLine
            + TRANS ("Sub-dirs: ") + String (dirNums) + "  "
            + TRANS ("Docs: ") + String (docNums) + newLine
            + TRANS ("Total Words: ") + String (totalWords) + "  "
            + TRANS ("Total Images: ") + String (totalImgs);
    }

    ScopedPointer<StatisComp> statisComp = new StatisComp (treeContainer, this, isDoc, staStr);
    CallOutBox callOut (*statisComp, treeContainer->getScreenBounds(), nullptr);

    callOut.runModalLoop();
}

//=================================================================================================
void DocTreeViewItem::getPath()
{      
    SystemClipboard::copyTextToClipboard ("*_wdtpGetPath_*" + tree.getProperty ("title").toString()
                                          + "@_=#_itemPath_#=_@" 
                                          + getHtmlFileOrDir (tree).getFullPathName());
}

//=================================================================================================
void DocTreeViewItem::replaceContent()
{
    if (tree.getType().toString() == "doc")
        treeContainer->getEditAndPreview()->switchMode (false);

    ScopedPointer<ReplaceComponent> replaceComp = new ReplaceComponent (
        treeContainer->getEditAndPreview()->getEditor(), tree);

    CallOutBox callOut (*replaceComp, treeContainer->getScreenBounds(), nullptr);
    callOut.runModalLoop();
}

//=================================================================================================
void DocTreeViewItem::refreshDisplay()
{
    clearSubItems();

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        DocTreeViewItem* itm = new DocTreeViewItem (tree.getChild (i), treeContainer, sorter);
        addSubItemSorted (*sorter, itm);
    }
}

//=================================================================================================
void DocTreeViewItem::valueTreePropertyChanged (ValueTree&, const Identifier&)
{
    repaintItem();
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildAdded (ValueTree& parentTree, ValueTree& /*childTree*/)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::valueTreeChildOrderChanged (ValueTree& parentTree, int, int)
{
    treeChildrenChanged (parentTree);
}

//=================================================================================================
void DocTreeViewItem::treeChildrenChanged (const ValueTree& parentTree)
{
    if (parentTree == tree)
    {
        refreshDisplay();
        treeHasChanged();
        setOpen (true);
    }
}

//=================================================================================================
void DocTreeViewItem::crossProjectCopy()
{
    // copy its properties
    const DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (getOwnerView()->getSelectedItem (0));
    ValueTree docTree (item->tree.createCopy());
    docTree.setProperty ("fileFullPath", getMdFileOrDir (item->tree).getFullPathName(), nullptr);

    // copy its media files' path
    Array<File> medias;

    for (int i = getMdMediaFiles (getMdFileOrDir (item->tree), medias); --i >= 0; )
    {
        ValueTree mediaTree ("media");
        mediaTree.setProperty ("mediaFullPath", medias[i].getFullPathName(), nullptr);
        docTree.addChild (mediaTree, 0, nullptr);
    }

    // write docTree to temp file
    const File crossCopyFile (File::getSpecialLocation (File::tempDirectory).getChildFile ("wdtpCrossCopy"));
    crossCopyFile.deleteFile();
    crossCopyFile.create();
    SwingUtilities::writeValueTreeToFile (docTree, crossCopyFile);
}

//=================================================================================================
void DocTreeViewItem::crossProjectPaste()
{    
    const File crossCopyFile (File::getSpecialLocation (File::tempDirectory).getChildFile ("wdtpCrossCopy"));
    ValueTree docTree (SwingUtilities::readValueTreeFromFile (crossCopyFile));
    const File docFile (docTree.getProperty ("fileFullPath").toString());

    Array<File> files;
    files.add (docFile);

    if (importExternalDocs (files, true))
    {
        DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (getOwnerView()->getSelectedItem (0));

        // copy its medias
        files.clear();

        for (int i = docTree.getNumChildren(); --i >= 0; )
            files.add (File (docTree.getChild (i).getProperty ("mediaFullPath").toString()));

        for (int i = files.size(); --i >= 0; )
        {
            const File& targetFile (getMdFileOrDir (tree).getChildFile ("media")
                                    .getChildFile (files[i].getFileName())
                                    .getNonexistentSibling (false));
            targetFile.create();
            files[i].copyFileTo (targetFile);

            // here must replace the doc's media name if there's alreay one
            if (targetFile.getFileName() != files[i].getFileName())
            {
                const File& pastedFile (getMdFileOrDir (item->tree));
                pastedFile.replaceWithText (pastedFile.loadFileAsString()
                                            .replace (files[i].getFileName(), targetFile.getFileName()));
            }
        }

        // change its properties after paste (import)
        docTree.removeProperty ("fileFullPath", nullptr);
        docTree.removeAllChildren (nullptr);

        // here must reset the 'name' since it may rename when the same name file exists
        docTree.setProperty ("name", item->tree.getProperty ("name").toString(), nullptr);
        item->tree.copyPropertiesFrom (docTree, nullptr);

        treeContainer->getEditAndPreview()->setDocProperties (item->tree);
        treeContainer->getEditAndPreview()->updateEditorContent();

        crossCopyFile.deleteFile();
    }
}

//=================================================================================================
void DocTreeViewItem::showKeywordsTable()
{
    ScopedPointer<KeywordsComp> keywordsComp = new KeywordsComp (tree, false, StringArray());

    if (keywordsComp->getKeywordsPicker() != nullptr)
        keywordsComp->getKeywordsPicker()->addActionListener (this);

    CallOutBox callOut (*keywordsComp, treeContainer->getScreenBounds(), nullptr);
    callOut.runModalLoop();
}

//=================================================================================================
void DocTreeViewItem::actionListenerCallback (const String& message)
{
    treeContainer->getTreeView().clearSelectedItems();
    selectChildren (this, message);
}

//=================================================================================================
void DocTreeViewItem::selectChildren (DocTreeViewItem* currentItem, const String& keyword)
{
    StringArray kws;
    kws.addTokens (currentItem->getTree().getProperty ("keywords").toString()
                   .replace (CharPointer_UTF8 ("\xef\xbc\x8c"), ", "), ",", String());
    kws.trim();
    kws.removeEmptyStrings (true);

    for (int i = kws.size(); --i >= 0; )
    {
    	if (kws[i] == keyword)
        {
            currentItem->setSelected (true, false);
            treeContainer->getTreeView().scrollToKeepItemVisible (currentItem);
            break;
        }
    }

    for (int i = currentItem->getNumSubItems(); --i >= 0; )
    {
        DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (currentItem->getSubItem (i));
        selectChildren (item, keyword);
    }
}

//=================================================================================================
void DocTreeViewItem::importExternalDocs()
{
    FileChooser fc (TRANS ("Import External Data"), File::nonexistent,
                    "*.txt;*.md;*.html;*.htm;*.markdown;*.mkd;*.ini", true);

    if (fc.browseForMultipleFilesToOpen())
        importExternalDocs (fc.getResults(), false);
}

//=================================================================================================
const bool DocTreeViewItem::importExternalDocs (const Array<File>& docs,
                                          const bool selectOneByOneAfterImport)
{
    // can't import docs under a doc!
    jassert (tree.getType().toString() != "doc");

    // doesn't import project-internal docs
    const File& projectDir (FileTreeContainer::projectFile.getParentDirectory());

    if (docs[0].getFullPathName().contains (projectDir.getFullPathName()))
    {
        SHOW_MESSAGE (TRANS ("Can't import doc(s) inside the current project!"));
        return false;
    }

    // import...
    bool needSaveProject = false;

    for (int i = docs.size(); --i >= 0; )
    {
        // doesn't import any big file
        if (docs[i].getSize() / 1024 <= 256)  
        {
            const String& content (docs[i].loadFileAsString());

            if (content.length() > 4)
            {
                createDoc (docs[i].getFileNameWithoutExtension(), content, selectOneByOneAfterImport);
                needSaveProject = true;
            }
            else
            {
                SHOW_MESSAGE (TRANS ("No neeed to import a file which can't/no need to read :)")
                              + newLine + newLine + docs[i].getFullPathName());
            }
        }
        else
        {
            SHOW_MESSAGE (TRANS ("Can't import a file which more than 256K :)") 
                          + newLine + newLine + docs[i].getFullPathName());
        }
    }

    if (needSaveProject)
        return FileTreeContainer::saveProject();

    return needSaveProject;
}

//=================================================================================================
void DocTreeViewItem::getWordsAndImgNumsInDoc (const ValueTree& tree, int& words, int& imgNums)
{
    const String& content (getMdFileOrDir (tree).loadFileAsString());
    words = content.removeCharacters (" ").removeCharacters (newLine).length() + words;

    int index = content.indexOf (0, "![");

    while (index != -1)
    {
        ++imgNums;
        index = content.indexOf (index + 1, "![");
    }

    index = content.indexOf (0, "<img src=");

    while (index != -1)
    {
        ++imgNums;
        index = content.indexOf (index + 8, "<img src=");
    }
}

//=================================================================================================
const bool DocTreeViewItem::getDirDocsAndAllMedias (DocTreeViewItem* item,
                                                    const File& mdFile,
                                                    Array<File>& medias)
{
    item->setOpen (true);

    if (item->getNumSubItems() > 0)
    {
        for (int i = 0; i < item->getNumSubItems(); ++i)
        {
            DocTreeViewItem* currentItem = dynamic_cast<DocTreeViewItem*> (item->getSubItem (i));
            jassert (currentItem != nullptr);

            // recursive traversal
            if (!getDirDocsAndAllMedias (currentItem, mdFile, medias))
                return false;
        }
    }
    else
    {
        const File& currentFile (getMdFileOrDir (item->getTree()));

        if (currentFile.existsAsFile() && currentFile.getSize() > 0)
        {
            getMdMediaFiles (currentFile, medias);
            String mdStr (HtmlProcessor::processAbbrev (item->getTree(),
                                                               currentFile.loadFileAsString()));

            // cancel '[TOC]' withou remove its escape
            mdStr = mdStr.replace ("\\[TOC]", "@#@_wdtpToc_@#@");

            mdStr = mdStr.replace (newLine + "[TOC]" + newLine, String())
                .replace (newLine + "[TOC]", String())
                .replace (String ("[TOC]") + newLine, String())
                .replace ("[TOC]", String())
                .replace ("@#@_wdtpToc_@#@", "\\[TOC]");

            return mdFile.appendText (mdStr.trimEnd() + newLine + newLine);
        }
    }

    return true;
}

//=================================================================================================
DocTreeViewItem* DocTreeViewItem::getRootItem (DocTreeViewItem* subItem)
{
    DocTreeViewItem* item = subItem;

    while (item->getParentItem() != nullptr)
        item = dynamic_cast<DocTreeViewItem*>(item->getParentItem());

    return item;
}

//=================================================================================================
void DocTreeViewItem::statis (const ValueTree& tree, int& dirNums, int& totalWords, int& totalImgs)
{
    if (tree.getType().toString() == "doc")
    {
        getWordsAndImgNumsInDoc (tree, totalWords, totalImgs);
    }
    else
    {
        ++dirNums;

        for (int i = tree.getNumChildren(); --i >= 0; )
            statis (tree.getChild (i), dirNums, totalWords, totalImgs);
    }
}

//=================================================================================================
var DocTreeViewItem::getDragSourceDescription()
{
    if (tree.getType().toString() != "wdtpProject" && getMdFileOrDir (tree).exists())
        return "dragFileOrDir";

    return String();
}

//=================================================================================================
bool DocTreeViewItem::isInterestedInDragSource (const DragAndDropTarget::SourceDetails& details)
{
    if (tree.getType().toString() != "doc")
        return details.description == "dragFileOrDir";

    return false;
}

//=================================================================================================
void DocTreeViewItem::itemDropped (const DragAndDropTarget::SourceDetails& details,
                                   int /*insertIndex*/)
{
    if (details.description != "dragFileOrDir")
        return;

    jassert (getMdFileOrDir (tree).isDirectory());

    OwnedArray<ValueTree> selectedTrees;
    TreeView* treeView = getOwnerView();

    for (int i = 0; i < treeView->getNumSelectedItems(); ++i)
    {
        const DocTreeViewItem* item = dynamic_cast<DocTreeViewItem*> (treeView->getSelectedItem (i));
        jassert (item != nullptr);

        selectedTrees.add (new ValueTree (item->tree));
    }

    moveItems (selectedTrees, tree);
}

//=================================================================================================
bool DocTreeViewItem::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

//=================================================================================================
void DocTreeViewItem::filesDropped (const StringArray& pathes, int /*insertIndex*/)
{
    // only get text files
    Array<File> files;

    for (auto path : pathes)
    {
        const File file (path);

        if (file.hasFileExtension (".txt;md;html;htm;markdown;mkd;ini"))
            files.add (file);
    }
    
    // import...
    if (tree.getType().toString() == "doc")
    {
        DocTreeViewItem* dirItem = static_cast<DocTreeViewItem*> (getParentItem());

        if (dirItem != nullptr)
            dirItem->importExternalDocs (files, false);
    }
    else
    {
        importExternalDocs (files, false);
    }
}

//=================================================================================================
void DocTreeViewItem::moveItems (const OwnedArray<ValueTree>& items,
                                 ValueTree thisTree)
{
    if (items.size() < 1)
        return;

#define CHOICE_BOX AlertWindow::showYesNoCancelBox

    for (int i = items.size(); --i >= 0;)
    {
        ValueTree& v = *items.getUnchecked (i);

        if (v.getParent().isValid() && thisTree != v && !thisTree.isAChildOf (v))
        {
            const File& thisFile (getMdFileOrDir (v));
            File targetFile (getMdFileOrDir (thisTree).getChildFile (thisFile.getFileName()));

            // prevent same-location move. but this will also prevent sort by mannul-mode
            if (thisFile.exists() && targetFile.exists() && thisFile == targetFile)
                continue;

            if (targetFile.exists())
            {
                const int choice = CHOICE_BOX (AlertWindow::QuestionIcon, TRANS ("Message"),
                                               "\"" + targetFile.getFullPathName() + "\"\n" +
                                               TRANS ("already exists. So what do you want? "),
                                               TRANS ("Keep Both"), TRANS ("Overwrite"));

                if (choice == 0)
                    continue;
                else if (choice == 1)
                    targetFile = targetFile.getNonexistentSibling (true);
            }

            if (thisFile.copyFileTo (targetFile)) // here must copy!
            {
                // move its media files first
                Array<File> medias;

                for (int j = getMdMediaFiles (thisFile, medias); --j >= 0; )
                {
                    const File& targetMediaFile (targetFile.getSiblingFile ("media")
                                                 .getChildFile (medias[j].getFileName())
                                                 .getNonexistentSibling (true));
                    targetMediaFile.create();
                    medias[j].moveFileTo (targetMediaFile);

                    // prevent same file name, so here need rename the media name of the doc
                    const String& content (thisFile.loadFileAsString()
                                           .replaceCharacters (medias[j].getFileName(), targetMediaFile.getFileName()));
                    thisFile.replaceWithText (content);
                }

                thisFile.deleteRecursively();

                v.getParent().removeChild (v, nullptr);
                needCreate (v.getParent());

                thisTree.addChild (v, 0, nullptr);
                needCreate (thisTree);
            }
            else
            {
                SHOW_MESSAGE ("\"" + thisFile.getFullPathName() + "\"\n" +
                              TRANS ("move failed. "));
            }
        }
    }

#undef CHOICE_BOX

    FileTreeContainer::saveProject();
}

//=================================================================================================
void DocTreeViewItem::paintHorizontalConnectingLine (Graphics& g, const Line<float>& line)
{
    g.setColour (Colour::fromString (systemFile->getValue ("uiTextColour")).withAlpha (0.6f));
    g.drawLine (line);
}

//=================================================================================================
void DocTreeViewItem::paintVerticalConnectingLine (Graphics& g, const Line<float>& line)
{
    g.setColour (Colour::fromString (systemFile->getValue ("uiTextColour")).withAlpha (0.6f));
    g.drawLine (line);
}

//=================================================================================================
void DocTreeViewItem::paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour, bool)
{
    Path p;
    p.addTriangle (0.0f, 0.0f, 1.0f, isOpen() ? 0.0f : 0.5f, isOpen() ? 0.5f : 0.0f, 1.0f);

    g.setColour (Colour::fromString (systemFile->getValue ("uiTextColour")).withAlpha (0.6f));
    g.fillPath (p, p.getTransformToScaleToFit (area.reduced (2, area.getHeight() / 4), true));
}

//=================================================================================================
String DocTreeViewItem::getTooltip()
{
    // full path of file
    if (sorter->getTooltipToShow() == 0)
        return getMdFileOrDir (tree).getFullPathName();

    // title
    else if (sorter->getTooltipToShow() == 1)
        return tree.getProperty ("title").toString();

    // keywords
    else if (sorter->getTooltipToShow() == 2)
        return tree.getProperty ("keywords").toString();

    // description
    else if (sorter->getTooltipToShow() == 3)
        return tree.getProperty ("description").toString();

    return String();
}


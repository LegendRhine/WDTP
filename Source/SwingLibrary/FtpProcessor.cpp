/*
  ==============================================================================

  FtpProcessor.cpp

  This class base on dRowAudio's cURL module, changed and expanded some useful functions.
  This file under the MIT agreement.

  UnderwaySoft.com

  ==============================================================================
*/

#include "JuceHeader.h"
#include "curl/curl.h"
#include "FtpProcessor.h"

//==============================================================================
class FtpHelper : public TimeSliceThread,
				  public DeletedAtShutdown
{
public:
	//==============================================================================
	juce_DeclareSingleton (FtpHelper, true);
	
    FtpHelper() : TimeSliceThread ("cURL Thread")
    {
        CURLcode result = curl_global_init (CURL_GLOBAL_ALL);

        (void) result;
        jassert (result == CURLE_OK);
    }

	~FtpHelper()
    {
        curl_global_cleanup();
    }
    
	//==============================================================================
	CURL* createEasyCurlHandle()
    {
        return curl_easy_init();
    }
	
	void cleanUpEasyCurlHandle (CURL* handle)
    {
        curl_easy_cleanup (handle);
        handle = nullptr;
    }
	
	/**	Returns a list of the supported protocols. */
	StringArray getSupportedProtocols()
    {
        if (curl_version_info_data* info = curl_version_info (CURLVERSION_NOW))
            return StringArray (info->protocols);

        return StringArray();
    }

    /** get the current version of cURL */
    const String getVersionOfCurrentCurl()
    {
        if (curl_version_info_data* info = curl_version_info (CURLVERSION_NOW))
            return String (info->version);

        return String::empty;
    }
    
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FtpHelper);
};

juce_ImplementSingleton (FtpHelper);

//==============================================================================
FtpProcessor::FtpProcessor()
    : handle (FtpHelper::getInstance()->createEasyCurlHandle()),
      transferType (nonType),
      shouldStopTransfer (false),
      progress (0.0f)
{
	enableFullDebugging (true);
    DBG ("cURL version: " + FtpHelper::getInstance()->getVersionOfCurrentCurl());

	curl_easy_setopt (handle, CURLOPT_NOPROGRESS, false);
}

//=========================================================================
FtpProcessor::~FtpProcessor()
{
    removeAllListener ();

	FtpHelper::getInstance()->removeTimeSliceClient (this);

	if (FtpHelper::getInstance()->getNumClients() == 0)
		FtpHelper::getInstance()->stopThread (3000);

	FtpHelper::getInstance()->cleanUpEasyCurlHandle (handle);
}
//=================================================================================================
void FtpProcessor::setRemoteRootDir (const String& rootDir)
{
    remoteRootDir = rootDir;

    if (remoteRootDir.getLastCharacters (1) != "/")
        remoteRootDir += "/";

    curl_easy_setopt (handle, CURLOPT_URL, remoteRootDir.toUTF8().getAddress());    
}

//=========================================================================
void FtpProcessor::setRemotePathForFtp (const String& destRemotePath,
                                        const String& tempPathForUse/* = String::empty*/)
{
	remoteFileOrDirPath = destRemotePath;

    if (remoteFileOrDirPath.substring (0, 1) == String (File::separator) ||
        remoteFileOrDirPath.substring (0, 1) == "/")
        remoteFileOrDirPath = remoteFileOrDirPath.substring (1);

    secondRemotePath = tempPathForUse;

    if (secondRemotePath.substring (0, 1) == String (File::separator) ||
        secondRemotePath.substring (0, 1) == "/")
        secondRemotePath = secondRemotePath.substring (1);
}
//=========================================================================
void FtpProcessor::setUserNameAndPassword (String username, String password)
{
	userNameAndPassword = username << ":" << password;
	curl_easy_setopt (handle, CURLOPT_USERPWD, userNameAndPassword.toUTF8().getAddress());
}
//=========================================================================
StringArray FtpProcessor::getDirectoryListing (const String& pathRelativeToRootDir)
{
    String subDir = pathRelativeToRootDir;

    if (subDir.isNotEmpty() && subDir.getLastCharacters (1) != "/")
        subDir += "/";

    String remoteUrl (remoteRootDir + subDir);
	curl_easy_setopt (handle, CURLOPT_URL, remoteUrl.toUTF8().getAddress());

	directoryContentsList.setSize (0);

    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 6);
    curl_easy_setopt (handle, CURLOPT_PROGRESSFUNCTION, 0L);
	curl_easy_setopt (handle, CURLOPT_UPLOAD, 0L);
	curl_easy_setopt (handle, CURLOPT_DIRLISTONLY, 1L);
	curl_easy_setopt (handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, directoryListingCallback);

	// perform  the tranfer
	progress = 0.0f;
	CURLcode result = curl_easy_perform (handle);
	reset();

	if (result == CURLE_OK)
	{
		StringArray list;
		list.addLines (directoryContentsList.toString().trim());

		return list;
	}
	else
    {
		return StringArray (curl_easy_strerror (result));
	}
}

//==============================================================================
void FtpProcessor::enableFullDebugging (bool shouldEnableFullDebugging)
{
	curl_easy_setopt (handle, CURLOPT_VERBOSE, shouldEnableFullDebugging ? 1L : 0L);
}
//=========================================================================
void FtpProcessor::beginTransfer (const TransferType& type)
{
	transferType = type;
    shouldStopTransfer = false;

    FtpHelper::getInstance()->addTimeSliceClient (this);
    FtpHelper::getInstance()->startThread();
}
//=========================================================================
void FtpProcessor::stopTransfer()
{
    shouldStopTransfer = true;
}
//=========================================================================
void FtpProcessor::reset()
{
	curl_easy_reset (handle);
	curl_easy_setopt (handle, CURLOPT_URL, remoteRootDir.toUTF8().getAddress());
	curl_easy_setopt (handle, CURLOPT_USERPWD, userNameAndPassword.toUTF8().getAddress());
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 6);
	curl_easy_setopt (handle, CURLOPT_NOPROGRESS, false);
}
//==============================================================================
void FtpProcessor::addListener (FtpProcessor::Listener* const listener)
{
    listeners.add (listener);
}

void FtpProcessor::removeListener (FtpProcessor::Listener* const listener)
{
    listeners.remove (listener);
}

void FtpProcessor::removeAllListener ()
{
    for (int i = listeners.size(); --i >= 0; )
        listeners.clear();
}
//==============================================================================
int FtpProcessor::useTimeSlice()
{
    performTransfer ();
	return -1;
}
//==============================================================================
size_t FtpProcessor::writeCallback (void* sourcePointer,
                                    size_t blockSize,
                                    size_t numBlocks,
                                    FtpProcessor* session)
{
    jassert (session->outputStream != nullptr);  // haven't set the local file to write the remote's data?

	if (session != nullptr)
	{
		if (session->outputStream->failedToOpen())
		{
            /* failure, can't open file to write */
			return ! (blockSize * numBlocks); // return a value not equal to (blockSize * numBlocks)
		}

		session->outputStream->write (sourcePointer, blockSize * numBlocks);
		return blockSize * numBlocks;
	}

	return ! (blockSize * numBlocks); // return a value not equal to (blockSize * numBlocks)
}
//=========================================================================
size_t FtpProcessor::readCallback (void* destinationPointer,
                                   size_t blockSize,
                                   size_t numBlocks,
                                   FtpProcessor* session)
{
	if (session != nullptr)
	{
		if (session->inputStream.get() == nullptr)
		{
			return CURL_READFUNC_ABORT; /* failure, can't open file to read */
		}

		return session->inputStream->read (destinationPointer, int (blockSize * numBlocks));
	}

	return CURL_READFUNC_ABORT;
}
//=========================================================================
size_t FtpProcessor::directoryListingCallback (void* sourcePointer,
                                               size_t blockSize,
                                               size_t numBlocks,
                                               FtpProcessor* session)
{
	if (session != nullptr)
	{
		session->directoryContentsList.append (sourcePointer, (int) (blockSize * numBlocks));

		return blockSize * numBlocks;
	}

	return ! (blockSize * numBlocks); // return a positive value not equal to (blockSize * numBlocks)
}
//=========================================================================
int FtpProcessor::internalProgressCallback (FtpProcessor* session,
                                            double dltotal,
                                            double dlnow, double /*ultotal*/,
                                            double ulnow)
{
	session->progress = (float) ((session->transferType == typeForUpload)
        ? (ulnow / session->inputStream->getTotalLength())
        : (dlnow / dltotal));

    session->listeners.call (&FtpProcessor::Listener::transferProgressUpdate, session);

	return (int) session->shouldStopTransfer;
}
//==============================================================================
void FtpProcessor::performTransfer ()
{
    jassert (transferType != nonType);  // must set the transfer type!
    reset();

         if (typeForUpload                  == transferType)    curlSetToUploadFile ();
	else if (typeForDownload                == transferType)    curlSetToDownloadFile ();
    else if (typeForDelRemoteFile           == transferType)    curlSetToDelRemoteFile ();
    else if (typeForDelRemoteDir            == transferType)    curlSetToDelRemoteDir ();
    else if (typeForMoveOrRenameRemotePath  == transferType)    curlSetToMoveRemoteFile ();
    else if (typeForCreateRemoteDir         == transferType)    curlSetToCreateRemoteDir ();

	//perform the transfer
	progress = 0.0f;
    listeners.call (&FtpProcessor::Listener::transferAboutToStart, this);
	CURLcode result = curl_easy_perform (handle);

	// delete the streams to flush the buffers
	releaseSource();
    listeners.call (&FtpProcessor::Listener::transferEnded, this);

    DBG ("cURL return: " + String (result));

    if (result == 0 || result ==  19)
        listeners.call (&FtpProcessor::Listener::transferSuccess, this);
    else
        listeners.call (&FtpProcessor::Listener::transferFailed, this);
}
//=================================================================================================
void FtpProcessor::curlSetUpOrDown ()
{
    curl_easy_setopt (handle, CURLOPT_URL, (remoteRootDir + remoteFileOrDirPath).toUTF8().getAddress());
    curl_easy_setopt (handle, CURLOPT_UPLOAD, (long) (transferType == typeForUpload));
    curl_easy_setopt (handle, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt (handle, CURLOPT_PROGRESSFUNCTION, internalProgressCallback);
}
//=================================================================================================
void FtpProcessor::curlSetToUploadFile ()
{
    curlSetUpOrDown ();

    // sets the pointer to be passed to the read callback
    curl_easy_setopt (handle, CURLOPT_READDATA, this);
    curl_easy_setopt (handle, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt (handle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);  // create nonexist dir

    inputStream->setPosition (0);
}
//=================================================================================================
void FtpProcessor::curlSetToDownloadFile ()
{
    curlSetUpOrDown ();

    // sets the pointer to be passed to the write callback
    curl_easy_setopt (handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, writeCallback);

    inputStream = nullptr;

    if (localFile.deleteFile())
    {
        localFile.create ();
        outputStream = localFile.createOutputStream();
    }
    else
    {
        outputStream = localFile.getNonexistentSibling().createOutputStream();
    }
}
//=================================================================================================
void FtpProcessor::curlSetToDelRemoteFile ()
{
    // must set the remote file first! it must be relative path to remoteRootDir and must with full file name.
    // eg. the relative path may be: "wwwroot/temp/000.txt"
    jassert (remoteFileOrDirPath.isNotEmpty() && remoteFileOrDirPath.getLastCharacters (1) != "/");

    const String delCommand ("DELE " + remoteFileOrDirPath);
    curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, delCommand.toUTF8().getAddress());
}
//=================================================================================================
void FtpProcessor::curlSetToDelRemoteDir ()
{
    // must set to a remote relative path (dir) before perform this operation!
    jassert (remoteFileOrDirPath.isNotEmpty());
    
    curl_easy_setopt (handle, CURLOPT_URL, (remoteRootDir + remoteFileOrDirPath).toUTF8().getAddress());
    const StringArray filesName (getDirectoryListing (remoteFileOrDirPath));
    reset ();

    // first, need to delete all the files which in the dir
    for (int i = filesName.size(); --i >= 0; )
    {
        const String delFileCommand ("DELE " + remoteFileOrDirPath + "/" + filesName[i]);
        curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, delFileCommand.toUTF8().getAddress());
        curl_easy_perform (handle);
    }

    // then delete the dir...
    const String delDirCommand ("RMD " + remoteFileOrDirPath.fromLastOccurrenceOf ("/", false, false));
    curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, delDirCommand.toUTF8().getAddress());
}
//=================================================================================================
void FtpProcessor::curlSetToMoveRemoteFile ()
{
    // must set remote pathes before perform this operation!
    jassert (remoteFileOrDirPath.isNotEmpty() && remoteFileOrDirPath.getLastCharacters (1) != "/");
    jassert (secondRemotePath.isNotEmpty() && secondRemotePath.getLastCharacters (1) != "/");

    const String moveFromCommand ("RNFR " + remoteFileOrDirPath);
    const String moveToCommand ("RNTO " + secondRemotePath);

    curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, moveFromCommand.toUTF8().getAddress());
    curl_easy_perform (handle);   // must perform after the first command 'RNFR'
    curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, moveToCommand.toUTF8().getAddress());
}
//=================================================================================================
void FtpProcessor::curlSetToCreateRemoteDir ()
{
    jassert (remoteFileOrDirPath.isNotEmpty() && !remoteFileOrDirPath.contains ("/"));

    const String createDirCommand ("MKD " + remoteFileOrDirPath);
    curl_easy_setopt (handle, CURLOPT_CUSTOMREQUEST, createDirCommand.toUTF8().getAddress());
}
//=================================================================================================
const bool FtpProcessor::connectOk (String& testResult)
{
    testResult = getDirectoryListing (String::empty).joinIntoString (newLine);
    DBG ("FtpProcessor::connectOk() - Root-dir listing or connect test: " << newLine << testResult);

    return  testResult != "Login denied" &&
            testResult != "Couldn't resolve host name" &&
            testResult != "Timeout was reached" &&
            testResult != "Unsupported protocol" &&
            testResult != "Failed initialization" &&
            testResult != "URL using bad/illegal format or missing URL" &&
            !testResult.contains ("<html>");
}


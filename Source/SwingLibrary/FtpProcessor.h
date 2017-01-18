/*
  ==============================================================================

      FtpProcessor.h

  This class base on dRowAudio's cURL module, changed and expanded some useful functions.
  This file under the MIT agreement.

  UnderwaySoft.com

  ==============================================================================
*/

#ifndef __FTPPROCESSOR_H__
#define __FTPPROCESSOR_H__

#if JUCE_WINDOWS
#pragma warning (disable: 4204)
#endif

typedef void CURL;

//==============================================================================
/**	This class uses background thread, can be used for FTP download, upload, delete, 
    move and rename remote file/dir, also can list remote dirs and files. 

    Usage:

    1. Create object, setRemoteRootDir() for your ftp server, all of the succedent operations
       will base on this root-dir. Then setUserNameAndPassword(). 
       Then call connectOk() to check out if the connection successful or not.

    2. If everything is OK, call the functions do what you want.

    3. For get the files and dirs list, you may need to call getDirectoryListing().
       This method will return a stringArray (file or dir lists) base on the argument path
       which relative to the root-dir.

    4. Also, you may use FtpProcessor::Listener class handle some needs.

    5. Want a progress value when upload and download file? Just call getProgress() after
       perform a operation.

    Note: to use this class, make sure you have the permission of your server and 
    the normal FTP commands must be performed correctly on this server, 
    and the remote file and dir has the proper attribute that can be deleted, created, renamed or moved.
 */
class FtpProcessor : public TimeSliceClient
{
public:
	//==============================================================================
    /** Creates an uninitialised CURLEasySession.
        You will need to set up the transfer using setLocalFile() and setRemotePathForFtp()
        and then call beginTransfer() to actucally perform the transfer.
     */
	FtpProcessor();
    
    /** Destructor. */
	~FtpProcessor();

     /**	Sets the user name and password of the connection.
     This is only used if required by the connection to the server.
	 */
	void setUserNameAndPassword (String username, String password);
    
    /** Before any ftp operation, you should call this first.
        All ftp operation will base on this path. eg. "ftp://www.xxx.com/wwwroot/myDir"

        There is no need "/" at the last. If forgot "/", this method will auto append it.
    */
    void setRemoteRootDir (const String& rootDir);

    /** Test wheather the remote server has been connected successfully or not. 
        
        the test result will write to the agrument testResult. */
    const bool connectOk (String& testResult);

    //=========================================================================
    /** The remoteFileName may contain relative pathes to remoteRootDir path, 
        all the pathes which relative to the root dir will be auto created.

        There is no need full remote path, also no need begin with "/". 
        If it's just a file name, it will upload to the remote root dir 
        which have been set through setRemoteRootDir().
     */
    void uploadToRemote (const File& localFile_, const String& remoteFileName)
    {
        setLocalFile (localFile_);
        setRemotePathForFtp (remoteFileName);
        beginTransfer (typeForUpload);
    }

    /** The remoteFileName may contain relative pathes to remoteRootDir path, 
        all the pathes which relative to the root dir will be auto created.

        There is no need full remote path, also no need begin with "/". 
        If it's just a file name, it will upload to the remote root dir 
        which have been set through setRemoteRootDir().
     */
    void uploadToRemote (InputStream* stream, const String& remoteFileName)
    {
        localFile = File::nonexistent;
        inputStream = stream;

        setRemotePathForFtp (remoteFileName);
        beginTransfer (typeForUpload);
    }

    /** It'll overlay the local file which has the same name.

        If the local file is using at the present, and cannot be delete first, 
        it will auto create a sibling file. 

        The remoteFileName may contain relative pathes to remoteRootDir path.
        Note: if the remoteFileName actually is a dir, it will download a 0 byte file to local.
        So, you may need to check it after call this method.
    */
    void downloadFromRemote (const String& remoteFileName, const File& localFile_)
    {
        setRemotePathForFtp (remoteFileName);
        setLocalFile (localFile_);
        beginTransfer (typeForDownload);
    }

    /** Delete an remote file. If the remote nonexists, it'll do nothing.
    
        The remoteFileName may be a path string that relative to the remote root-dir.
        For example: deleteRemoteFile ("aDir/subDir/000.mp3")
    */
    void deleteRemoteFile (const String& remoteFileName)
    {
        setRemotePathForFtp (remoteFileName);
        beginTransfer (typeForDelRemoteFile);
    }

    /** Delete an remote dir, also delete all its child files. 
        There is no need the "/" at the last of remoteDirName. 
        
        If the dir or file nonexists, it'll do nothing. 
        The remotePathName may be a path string that relative to the remote root-dir.

        Note: this method isn't using recursion for delete the specified remote dir,
        if it contains sub-dirs or muilt-level dirs, it will not delete them.
        Only the files which directly in the dir will be deleted,
        the dir itself and all its sub-dirs will still remain there.

        In this case, the perform result will successful if you used the FtpProcessor::Listener.
        It is a bad bug need to be fixed though...
    */
    void deleteRemoteDir (const String& remoteDirName)
    {
        jassert (remoteDirName.getLastCharacters (1) != "/");

        setRemotePathForFtp (remoteDirName);
        beginTransfer (typeForDelRemoteDir);
    }

    /** It can only create one level dir which relative to the root-dir. 
        For example:
        
        createRemoteDir ("00");         // will successful
        createRemoteDir ("00/01/02");   // nothing can be done

        If the dir already exists, it'll do nothing.
        Note: the remoteDirName no need contain "/". 
    */
    void createRemoteDir (const String& remoteDirName)
    {
        jassert (!remoteDirName.contains ("/"));

        setRemotePathForFtp (remoteDirName);
        beginTransfer (typeForCreateRemoteDir);
    }

    /** Move or rename an remote file or dir.

        When move an remote file or dir, make sure the dest-dir already exists. 
        If in the same dir, it'll rename instead of move it.
        Also, you can move a file or dir with the diffirent name in the new place:

        For example:
        curl.moveRemoteFileOrDir ("26/22/000.ogg", "26/24/001.ogg");
        
        Both the remotePath are relative to the root-dir.

        Note: if rename a file or dir, and the new name is the same with a exists one,
        the exists one will be overwrited! But, if the one is a dir and include sub-dir,
        it will not be overwrited, that is, this method will do nothing in this case.
    */
    void moveRemoteFileOrDir (const String& remoteExistsPath, const String& remoteNewPath)
    {
        setRemotePathForFtp (remoteExistsPath, remoteNewPath);
        beginTransfer (typeForMoveOrRenameRemotePath);
    }

    //=========================================================================	
    /** Note: The return String will contain the "/" at the last. */
    const String& getRemoteRootDir ()                   { return remoteRootDir; }

    /**	Returns the remote dirs and files listing. 

        The pathRelativeToRootDir must be a dir name of remote, 
        it may contains pathes relative to the root-dir. 

        If you have no permission to login the server, it will return some extra info,
        such as "Login denied". So, you could use the return value to check out 
        wheather login successful or not.        
    */
	StringArray getDirectoryListing (const String& pathRelativeToRootDir);
	    
    /**	Turns on full debugging information.
		This is probably best turned off in release builds to avoid littering the console.
	 */
	void enableFullDebugging (bool shouldEnableFullDebugging);    

    /** Stops the current transfer.  */
    void stopTransfer();

	/** Resets the state of the session to the parameters that have been specified. 

        Note: this will reset the romete dir to remoteRootDir and
        will not reset or change the current transformType. */
	void reset();
	    
    /** Returns the progress of the current transfer.  */
    float getProgress()            { return progress.get();  }

    //==============================================================================
    /** A class for receiving callbacks from a CURLEasySession.
        
        Note that these callbacks will be called from the transfer thread so make sure
        any code within them is thread safe! */
    class  Listener
    {
    public:
        //==============================================================================
        /** Destructor. */
        virtual ~Listener() {}
		
        //==============================================================================
        /** Called when a transfer is about to start. */
        virtual void transferAboutToStart (FtpProcessor* /*session*/)  { };
		
        /** Called when a transfer is running. */
        virtual void transferProgressUpdate (FtpProcessor* /*session*/)  { };

        /** Called when a transfer is about to end. */
        virtual void transferEnded (FtpProcessor* /*session*/)  { };

        /** Called when something goes wrong. */
        virtual void transferFailed (FtpProcessor* /*session*/) = 0;

        /** Called when everything is ok. */
        virtual void transferSuccess (FtpProcessor* /*session*/) = 0;
    };
	
    /** Adds a listener to be called when the transfer about to start, end and running. */
    void addListener (Listener* listener);
	
    /** Removes a previously-registered listener. */
    void removeListener (Listener* listener);

    void removeAllListener();
	
    //==============================================================================
	/** @internal */
	int useTimeSlice();
	
private:
    //=========================================================================
    /**  */
    void releaseSource()
    {
        localFile = File::nonexistent;
        inputStream = nullptr;
        outputStream = nullptr;
    }

	void setRemotePathForFtp (const String& destRemotePath, 
                              const String& remotePathForMove = String::empty);
    
    void setLocalFile (const File& newLocalFile)        
    {
        localFile = newLocalFile; 
        inputStream = localFile.createInputStream();
    }

    enum TransferType  { typeForUpload, typeForDownload, typeForDelRemoteFile, 
                         typeForDelRemoteDir, typeForMoveOrRenameRemotePath, typeForCreateRemoteDir, 
                         nonType};

	void beginTransfer (const TransferType& type);
    //==============================================================================

	CURL* handle;
    String userNameAndPassword, remoteRootDir, remoteFileOrDirPath, secondRemotePath; 

	TransferType transferType;
    bool shouldStopTransfer;
	Atomic<float> progress;
	
	File localFile;
	ScopedPointer<FileOutputStream> outputStream;
	ScopedPointer<InputStream> inputStream;
	MemoryBlock directoryContentsList;
    
    CriticalSection transferLock;
    ListenerList<Listener> listeners;

    //==============================================================================
    void performTransfer ();

    void curlSetToCreateRemoteDir ();
    void curlSetToMoveRemoteFile ();
    void curlSetToDelRemoteDir ();
    void curlSetToDelRemoteFile ();
    void curlSetToDownloadFile ();
    void curlSetToUploadFile ();
    void curlSetUpOrDown ();

    static size_t writeCallback (void* sourcePointer, size_t blockSize, size_t numBlocks, FtpProcessor* session);
	static size_t readCallback (void* destinationPointer, size_t blockSize, size_t numBlocks, FtpProcessor* session);
	static size_t directoryListingCallback (void* sourcePointer, size_t blockSize, size_t numBlocks, FtpProcessor* session);
	static int internalProgressCallback (FtpProcessor* session, double dltotal, double dlnow, double ultotal, double ulnow);
	    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FtpProcessor);
};


#endif  // __FTPPROCESSOR_H__

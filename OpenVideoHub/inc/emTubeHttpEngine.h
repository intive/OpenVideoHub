/*
 * Copyright (c) 2007-2009 BLStream Oy.
 *
 * This file is part of OpenVideoHub.
 *
 * OpenVideoHub is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * OpenVideoHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpenVideoHub; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef EMTUBE_HTTP_ENGINE_H
#define EMTUBE_HTTP_ENGINE_H

#include <e32def.h>
#include <e32base.h>
#include <s32file.h>
#include <ES_SOCK.H>
#include <commdb.h>
#include <CommDbConnPref.h>
#include <mhttptransactioncallback.h>
#include <mhttpsessioneventcallback.h>
#include <rhttpsession.h>
#include <chttpformencoder.h>

#include "emTubePluginManager.h"
#include "emTubeConnectionProgressObserver.h"

#include "emTubeTimeOutTimer.h"
#include "emTubeProgress.h"
#include "emTubeVideoEntry.h"

class CEmTubeAppUi;
class CQueueEntry;

#include <rconnmon.h>

#define HTTP_BUFFER_SIZE ( 65536 / 4 )
const TInt KMaxHeaderNameLen = 256;
const TInt KMaxHeaderValueLen = 256;
const TInt KOfflineProfile = 5;

class MEmTubeDataSupplier
{
public:
	virtual TInt Length() = 0;
	virtual void DataBlock( TPtrC8& aData ) = 0;
	virtual TInt NextDataBlock() = 0;
	virtual TBool EndOfData() = 0;
	virtual ~MEmTubeDataSupplier() {};
};

//data supplier -> buffer
class CEmTubeDataSupplierBuffer : public CBase, public MEmTubeDataSupplier
{
public:
	static CEmTubeDataSupplierBuffer* NewL(const TDesC8& aData);
	static CEmTubeDataSupplierBuffer* NewLC(const TDesC8& aData);
	virtual ~CEmTubeDataSupplierBuffer();

public: //from MEmTubeDataSupplier
	TInt Length();
	void DataBlock( TPtrC8& aData );
	TInt NextDataBlock();
	TBool EndOfData() { return ETrue; }

private:
	CEmTubeDataSupplierBuffer();
	void ConstructL(const TDesC8& aData);

private:
	HBufC8* iData;
};

#define CHUNK_SIZE 2048
//data supplier file
class CEmTubeDataSupplierFile : public CBase, public MEmTubeDataSupplier
{
public:
	static CEmTubeDataSupplierFile* NewL(const TDesC& aFilename);
	static CEmTubeDataSupplierFile* NewLC(const TDesC& aFilename);
	virtual ~CEmTubeDataSupplierFile();

public: //from MEmTubeDataSupplier
	TInt Length();
	void DataBlock( TPtrC8& aData );
	TInt NextDataBlock();
	TBool EndOfData() { return iLastBlock; }

private:
	CEmTubeDataSupplierFile();
	void ConstructL(const TDesC& aFilename);

private:
	RFs iFs;
	RFile iFile;
	TInt iFileSize;
	TBool iLastBlock;
	TBuf8<CHUNK_SIZE> iFileBuffer;

};


class CEmTubeHTTPDataSupplier : public CBase, public MHTTPDataSupplier
{
public:
	static CEmTubeHTTPDataSupplier* NewL();
	static CEmTubeHTTPDataSupplier* NewLC();
	virtual ~CEmTubeHTTPDataSupplier();

public:
	void PrepareL( );
	void PrepareL( const TDesC8& aData, const TDesC8& aContentType );
	void PrepareL( const TDesC8& aData1, const TDesC8& aData2, const TDesC8& aContentType );
	void PrepareL( const TDesC& aFilename, const TDesC8& aData, const TDesC8& aContentType, MProgressObserver& aProgressObserver );
	void PrepareL( const TDesC& aFilename, const TDesC8& aData1, const TDesC8& aData2, const TDesC8& aContentType, MProgressObserver& aProgressObserver );

	void SetTransaction( RHTTPTransaction& aTransaction ) { iTransaction = &aTransaction; }
	TDesC8& ContentType() { return *iContentType; }

public: //from MHTTPDataSupplier
	TBool GetNextDataPart(TPtrC8& aDataPart);
	void ReleaseData();
	TInt Reset();
	TInt OverallDataSize();

private:
	CEmTubeHTTPDataSupplier();
	void ConstructL();

private:
	RHTTPTransaction* iTransaction;
	MProgressObserver* iProgressObserver;

	HBufC8* iContentType;
	TInt iCurrentChunk;
	TInt iCurrentSize;

	RPointerArray<MEmTubeDataSupplier> iSuppliers;
};

class MHttpEngineObserver
{
public:
	virtual void RequestFinishedL( TInt aRequest, TDesC8& aResponseBuffer ) = 0;
	virtual void RequestCanceledL( TInt aRequest ) = 0;
	virtual TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize ) = 0;
	virtual void ShowErrorNoteL( TInt aResourceId ) = 0;
	virtual void ShowErrorNoteL( const TDesC& aText ) = 0;
};

_LIT( KAppUpdateUrl, "http://www.emtube.yoyo.pl/version" );

class CEmTubeHttpEngine : public CActive, public MHTTPTransactionCallback, public MTimeOutObserver, public MConnectionProgressObserver
{
public:
	enum TRequest
		{
		ERequestNone = 0,
		ERequestImageDownload,
		ERequestMovieDownload,
		ERequestMovieDownload2,
		ERequestSearch,
		ERequestCheckForUpdate,
		ERequestPreLogin,
		ERequestLogin,
		ERequestMovieUploadStep0,
		ERequestMovieUploadStep1,
		ERequestMovieUploadStep2
		};

	enum TRequestType
		{
		ERequestPost = 0,
		ERequestGet
		};

	enum TRequestErrors
		{
		ERequestFailed,
		ERequestCanceled,
		EWrongAccessPoint,
		ECouldNotConnect,
		EConnectionLost,
		EConnectionTimedOut,
		ENoLocationHeader,
		EContentLock,
		EVideoRemoved,
		EVideoNotAvailable,
		ENoPlugin
		};

public:
	static CEmTubeHttpEngine* NewL( CEmTubePluginManager& aManager, TBool aRemoveRedirectFilter );
	static CEmTubeHttpEngine* NewLC( CEmTubePluginManager& aManager, TBool aRemoveRedirectFilter );
	~CEmTubeHttpEngine();
	CEmTubeHttpEngine( CEmTubePluginManager& aManager );

	void ConstructL( TBool aRemoveRedirectFilter );

public: // from CActive
	void RunL();
	void DoCancel();

public: //from MConnectionProgressObserver
	void HandleConnectionEventL( TInt aState );

public:
	void SearchL( const TDesC& aString, MHttpEngineObserver &aObserver );
	void DownloadImageL( CVideoEntry *aEntry, MHttpEngineObserver &aObserver );
	void DownloadMovieL( TDesC& aOutput, CVideoEntry& aEntry, TInt aStartOffset, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );
	void DownloadMovieL( TDesC& aOutput, TDesC& aUrl, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );
	void DownloadMovieL( TDesC& aUrl, MHttpEngineObserver& aObserver );
	void CheckForUpdateL( MHttpEngineObserver& aObserver );

	void LoginL( TUint32 aPluginUid, MHttpEngineObserver& aObserver, TBool aPost );
	void LoginL( const TDesC& aUrl, const TDesC& aUsername, const TDesC& aPassword, MHttpEngineObserver& aObserver, TBool aPost );

	void UploadMovieStep0L( TUint32 aPluginUid, MHttpEngineObserver& aObserver );
	void UploadMovieStep1L( TUint32 aPluginUid, CQueueEntry& aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );
	void UploadMovieStep2L( TUint32 aPluginUid, CQueueEntry& aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );

	void CancelOperationL();
	void SetClientObserver( MHttpEngineObserver *aObserver ) { iObserver = aObserver; }

	TInt AccessPoint() { return iAccessPoint; }
	void SetAccessPoint( TInt aAccessPoint );
	static TBool AccessPointExistsL( TInt aAp );
	static TBool IsAccessPointWLAN( TUint32 aAp );
	static TInt ActiveProfileL();

private:
	HBufC* EscapeUrlL( const TDesC& aUrl );

	TInt StartGetRequestL( const TDesC& aUrl );
	TInt StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset = 0 );
	TInt StartPostRequestL( const TDesC& aUrl );
	void IssueRequestL();
	void SubmitTransactionL();
	void CancelRequestL();
	void CloseRequestL();

	void SetHttpBufferSize( TInt aSize );

	void GetResponseBodyDataL();

	void AddCookiesL( RHTTPHeaders aHttpHeaders );

	void SetProgressObserver( MProgressObserver* aProgressObserver ) { iProgress = aProgressObserver; }

	TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize );

	void AddHttpHeaderL( TInt aParam, const TDesC8& aValue );
	void HandleErrorL(TInt aError);

	void RequestCompletedL();

	void DumpHeadersL( RHTTPTransaction& aTrans );

public: // from MHTTPTransactionCallback
	void MHFRunL(RHTTPTransaction aTransaction,const THTTPEvent& aEvent);
	TInt MHFRunError(TInt aError ,RHTTPTransaction,const THTTPEvent& aEvent);

public: //from MHTTPTimeOutObserver
	void TimerExpired( TInt aMode );

private:
	void RequestFinishedL( TInt aState, TDesC8& aResponseBuffer );
	void RequestCanceledL( TInt aState );

private:

	TBool iSaveToFile;
	TBool iFileOpened;

	RFs iFsSession;
	RFileWriteStream iFileStream;
	RFile iFile;

	HBufC* iFileName;
	HBufC* iMovieFileName;

	RHTTPTransaction iTransaction;
	RHTTPSession iSession;
	HBufC8* iUrl;
	TUriParser8 iUri;
	HBufC8* iResponse;

	TUint iAccessPoint;
	TInt iBodySize;

	TBool iConnectionOpened;
	TBool iConnectionStarted;
	TBool iTransactionOpened;

	RSocketServ iSockServ;
	RConnection iConn;

	TCommDbConnPref iConnPref;
	CEmTubeTimeOutTimer *iTimer;

	MProgressObserver* iProgress;
	TInt iDataReceived;

	RBuf8 iHttpBuffer;
	TInt iHttpBufferSize;

	TInt iStartOffset;

	TRequest iRequest;

	CEmTubeHTTPDataSupplier *iBody;

	MHttpEngineObserver* iObserver;
	CEmTubeAppUi* iAppUi;

	TRequestType iRequestType;

	CEmTubePluginManager* iPluginManager;

//cache stuff
	HBufC* iSearchString;

	TUint32 iPluginUid;

	TInt iProfileId;
    TBool iForceIAPSelectionDialog;
    TBool iForceCloseConnection;
//progress observer
    CEmTubeConnectionProgressObserver* iProgressObserver;

    
//download speed
    TUint32 iDownloadSpeedTicks;
    TUint32 iDownloadSpeed;
};

#endif // EMTUBE_HTTP_ENGINE_H

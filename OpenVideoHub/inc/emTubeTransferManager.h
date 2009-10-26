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

#ifndef EMTUBE_TRANSFER_MANAGER_H
#define EMTUBE_TRANSFER_MANAGER_H

#include <e32base.h>

#include "emTubeHttpEngine.h"
#include "emTubeProgress.h"

class CEmTubeAppUi;

class CQueueEntry : public CBase, public MVideoData
{
	public:
		enum TEntryType
		{
			EEntryDownload = 0,
			EEntryUpload
		};

	public:
		enum TEntryStatus
		{
			EEntryQueued = 0,
			EEntryFinished,
			EEntryInitializing,
			EEntryUploading,
			EEntryDownloading,
			EEntryFailed
		};

	public:
		static CQueueEntry* NewL();
		static CQueueEntry* NewLC();
		~CQueueEntry();

		TEntryType Type() { return iType; }
		void SetType( TEntryType aType ) { iType = aType; }

		TEntryStatus Status() { return iStatus; }
		void SetStatus( TEntryStatus aStatus ) { iStatus = aStatus; }

		TDesC& Title() { return *iTitle; }
		void SetTitleL( const TDesC& aTitle );

		TDesC& Tags() { return *iTags; }
		void SetTagsL( const TDesC& aTags );

		TDesC& Description() { return *iDescription; }
		void SetDescriptionL( const TDesC& aDescription );

		void SetCategory( TMovieCategory aCategory ) { iCategory = aCategory; }
		TMovieCategory Category() { return iCategory; }

		TBool Public() { return iPublic; }
		void SetPublic( TBool aPublic ) { iPublic = aPublic; }

		TReal64 Latitude() { return iLatitude; }
		void SetLatitude( TReal64 aLatitude ) { iLatitude = aLatitude; }

		TReal64 Longitude() { return iLongitude; }
		void SetLongitude( TReal64 aLongitude ) { iLongitude = aLongitude; }

		TDesC& Url() { return *iUrl; }
		void SetUrlL( const TDesC& aUrl );

		TUint32 Uid() { return iUid; }
		void SetUid( TUint32 aUid );

		TDesC& Filename() { return iFilename; }
		void SetFilename( const TDesC& aFilename );

		TInt Size() { return iSize; }
		void SetSize( TInt aSize ) { iSize = aSize; }

		TInt CurrentSize() { return iCurrentSize; }
		void SetCurrentSize( TInt aCurrentSize ) { iCurrentSize = aCurrentSize; }

        void ExternalizeL( RWriteStream& aStream );
        void InternalizeL( RReadStream& aStream );

	private:
		CQueueEntry();
		void ConstructL();

	private:
		HBufC* iUrl;
		HBufC* iTitle;
		HBufC* iTags;
		HBufC* iDescription;
		TFileName iFilename;
		TEntryType iType;
		TEntryStatus iStatus;

		TUint32 iUid;

		TMovieCategory iCategory;
		TBool iPublic;
		
		TInt iSize;
		TInt iCurrentSize;
//TODO -> externalize/internalize
		TReal64 iLatitude;
		TReal64 iLongitude;
};

class CEmTubeTransferManager : public CBase, public MHttpEngineObserver, public MProgressObserver
{
	public:
		static CEmTubeTransferManager* NewL();
		static CEmTubeTransferManager* NewLC();
		~CEmTubeTransferManager();

	public:
		void AddToQueueL( const TDesC& aName, const TDesC& aUrl, const TDesC& aFilename );
		void AddToQueueL( CQueueEntry& aEntry  );
		void RemoveFromQueueL( TInt aEntryIndex );
		void MoveUpInQueueL( TInt aEntryIndex );
		void MoveDownInQueueL( TInt aEntryIndex );
		RPointerArray<CQueueEntry>& Queue();

		void StartL( CQueueEntry* aEntry );
        // when passed ETrue (optional) it sets iTMStarted to ETrue, transfer manager runnig
        void StartL( TBool aStartManager = EFalse );
        //returns index of an entry that was active during Stop()
        TInt StopL( );

		CQueueEntry* CurrentEntry() { return iCurrentEntry; }

	public: //from MHttpEngineObserver
		void RequestFinishedL( TInt aRequest, TDesC8& aResponseBuffer );
		void RequestCanceledL( TInt aRequest );
		TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize );
		void ShowErrorNoteL( TInt aResourceId );
		void ShowErrorNoteL( const TDesC& aText );

	public: //from MProgressObserver
		void ProgressStart( TInt aCompleteSize );
		void ProgressUpdate( TInt aCurrent, TInt aDownloadSpeed );
		void ProgressComplete();

    public:
        void LoadQueueL();
        void SaveQueueL();    
        
        // gives back ETrue when something is being downloaded or uploaded
        TBool Processing();
        
        void RemoveFinishedEntriesL();
        TInt FinishedEntries();
        TInt FailedEntries();

        TBool SafeToMoveEntry( TInt aEntryIndex, TBool aMoveUp );
        
        TBool EntryAdded( TDesC& aEntryUrl );

        void RegisterObserverL( MProgressObserver& aObserver );
        void UnRegisterObserver( MProgressObserver& aObserver );

        // gives back ETrue when transfers started
        void SetTMStarted( TBool aStarted ) { iTMStarted = aStarted; }

        // gives back ETrue when all files either EEntryQueued or EEntryFailed
        TBool AllProcessed();
        
	private:
		CEmTubeTransferManager();
		void ConstructL();
        void CancelOperationL();

	private:
		RPointerArray<CQueueEntry> iQueue;
		CEmTubeHttpEngine* iHttpEngine;
		CQueueEntry* iCurrentEntry;
		CEmTubeAppUi* iAppUi;
		RPointerArray<MProgressObserver> iObserverArray;
        TBool iTMStarted;

};

#endif //EMTUBE_TRANSFER_MANAGER_H

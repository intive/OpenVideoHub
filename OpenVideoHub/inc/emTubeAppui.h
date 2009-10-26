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

#ifndef EMTUBE_APPUI_H
#define EMTUBE_APPUI_H

#ifdef EMTUBE_UIQ
#include <QikAppUi.h>
#else
#include <aknviewappui.h>
#endif

class CAknWaitDialog;
class CAknNoteDialog;

#include "emTubeHttpEngine.h"
#include "emTubeImageLoader.h"
#include "emTubeThumbnail.h"
#include "emTubeCache.h"

_LIT(KBitmapFileName, "\\resource\\apps\\openvideohub.mbm");
_LIT(KSystemIconFile, "\\resource\\apps\\avkon2.mbm");

class CVideoEntry;
class CStreamer;
class CEmTubeTransferManager;
class CEmTubeInboxFlvFinder;
class CEmTubePluginManager;
class CEmTubePlaylistManager;
class CEmTubePlaylistEntry;
class CEmTubeSettingsData;

#ifndef EMTUBE_UIQ
class CEmTubeAppUi : public CAknViewAppUi, MHttpEngineObserver, MImageLoaderCallback, MVideoEntryHandler
#else
class CEmTubeAppUi : public CQikAppUi, MHttpEngineObserver, MImageLoaderCallback, MVideoEntryHandler
#endif
	{
public:
	enum TVideoScaleMode
	{
		EVideoScaleNone = 0,
		EVideoScaleNormal,
		EVideoScaleExtended
	};

	enum TStartPlaybackMode
	{
		EStartPlaybackManual = 0,
		EStartPlaybackAfterDownload = 1,
		EStartPlaybackAsap = 2
	};

public:
	enum TSearchDisplayMode
	{
		ESearchDisplayFavEntries = 0,
		ESearchDisplaySearchEntries,
		ESearchDisplaySavedEntries,
	};

public:
	enum TMemoryChoice
	{
		EMemoryPhone = 0,
		EMemorySd,
		EMemoryInbox,
		EMemoryNone

	};

public:
	void ConstructL();
	CEmTubeAppUi();
	~CEmTubeAppUi();

public: // from CAknAppUi
	void HandleCommandL( TInt aCommand );
	void HandleForegroundEventL ( TBool aForeground );
	TBool ProcessCommandParametersL( CApaCommandLine& aCommandLine );
	CArrayFix <TCoeHelpContext>* HelpContextL() const;

public: // from MHttpEngineObserver
	void RequestFinishedL( TInt aRequest, TDesC8& aResponseBuffer );
	void RequestCanceledL( TInt arequest );

public: // from MImageLoaderCallback
	void ImageLoadedL( TInt aError );

public: // from MVideoEntryHandler
	MVideoEntry* AddVideoEntryL();

public: //http interface

	TBool MMCAvailable();
	TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize );

	void LoginL( TUint32 aUid, TBool aPost );
	void LoginL( TUint32 aUid, MHttpEngineObserver& aObserver, TBool aPost );

	void DownloadMovieL( CVideoEntry* aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );
	void DownloadMovieL( TDesC& aFile, CVideoEntry &aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver );
	void CancelDownloadMovieL();

	void StartDownloadingImagesL();

	void DownloadImageL( CVideoEntry* aEntry, MHttpEngineObserver& aObserver );
	void DownloadImageL( TInt aIndex  );
	void CancelDownloadImageL();

	TInt & MaxResults() { return iSearchMaxResults;	}
	void SetMaxResults( TInt aMaxResults ) { iSearchMaxResults = aMaxResults; }

	TInt & MaxCacheSize() { return iMaxCacheSize;	}
	void SetMaxCacheSize( TInt aMaxCacheSize ) { iMaxCacheSize = aMaxCacheSize; iCache->SetMaxCacheSize( aMaxCacheSize ); }

	RPointerArray<CVideoEntry>& SearchEntries();
	TSearchDisplayMode SearchDisplayMode() { return iSearchDisplayMode; }

	TBool SearchDialogL();
	void SearchL();
	void SearchL( TInt aFeature, TOrderBy aOrderBy, TPeriod aTimeFrame );
	void StartSearchL( TBool aByUrl );
	TDes& SearchString() { return iSearchString; }

	void ShowErrorNoteL( TInt aResourceId );
	void ShowErrorNoteL( const TDesC& aText );
	TInt ConfirmationQueryL( TInt aResourceId );

	TDesC& SaveLoadDirectory() { return iSaveLoadDirectory; }
	void SetSaveLoadDirectory( TDesC& aDir );

	void SetSearchNaviPaneId( TInt aId ) { iSearchNaviPaneId = aId; }
	TInt SearchNaviPaneId() { return iSearchNaviPaneId; }

	TBool Embedded() { return iEmbedded; }
	void SetEmbedded( TBool aEmbedded ) { iEmbedded = aEmbedded; }

	TBool AutoRotate() { return iAutoRotate; }
	void SetAutoRotate( TBool aAutoRotate ) { iAutoRotate = aAutoRotate; }

	TVideoScaleMode VideoScaleMode() { return iVideoScaleMode; }
	void SetVideoScaleMode( TVideoScaleMode aMode ) { iVideoScaleMode = aMode; }

	TBool TempMemory() { return iTempMemory; }
	void SetTempMemory( TBool aTempMemory );

	TInt & Volume() { return iVolume; }
	void SetVolume( TInt aVolume ) { iVolume = aVolume; }

	TInt & AccessPoint() { return iAccessPoint; }
	void SetAccessPoint( TInt aAccessPoint ) { iAccessPoint = aAccessPoint; iHttpEngine->SetAccessPoint(aAccessPoint); }

	TInt & SelectedAccessPoint() { return iSelectedAccessPoint; }
	void SetSelectedAccessPoint( TInt aAccessPoint ) { iSelectedAccessPoint = aAccessPoint; }

	TStartPlaybackMode & StartPlaybackMode() { return iStartPlaybackMode; }
	void SetStartPlaybackMode( TStartPlaybackMode aStart ) { iStartPlaybackMode = aStart; }

	TOrderBy & SortResultsBy() { return iOrderBy; }
	void SetSortResultsBy( TOrderBy aSort ) { iOrderBy = aSort; }

	void InitTmpVideoFileNameL();
	TDesC& VideoFileName() { return *iTmpVideoFile; }

	HBufC* ConvertFileNameL( TDesC& aFileName );

	void SetLastVideoUrlL( TDesC& aLast );
	TDesC& LastVideoUrl();

	CEmTubeCache& Cache() { return *iCache; }

	CEmTubeTransferManager& TransferManager() { return *iTransferManager; }

	TVersion Version() { return iVersion; }

	void ClearSearchResults();

	TInt CurrentViewId() { return iCurrentViewId; }

public:
	CEmTubePluginManager& PluginManager() { return *iPluginManager; }

	void OpenFileL( RFile& aFile );
	void OpenFileL( const TDesC& aFilename, const TDesC& aTitle );
	void OpenFileL( CVideoEntry *aEntry );
	void OpenFileL( CEmTubePlaylistEntry *aEntry );

	void LoadFavoritesL();
	void SaveFavoritesL();
	void AddToFavoritesL( CVideoEntry* aEntry );
	void DeleteFromFavoritesL( CVideoEntry* aEntry );
	TInt FindInFavoritesL( CVideoEntry* aEntry );

	void LoadSavedClipsL();
	void SaveSavedClipsL();
	void AddToSavedClipsL( CVideoEntry* aEntry );
	void AddToSavedClipsL( CQueueEntry* aEntry );
	void DeleteFromSavedClipsL( CVideoEntry* aEntry );
	TInt FindInSavedClipsL( CVideoEntry* aEntry );
	void RenameSavedClipL( CVideoEntry* aEntry, const TDesC& aName );

	void LoadSettingsL();
	void SaveSettingsL();

	void LoadSearchHistoryL();
	void SaveSearchHistoryL();
	void AddToSearchHistoryL( const TDesC& aString );

    CEmTubeAppUi::TMemoryChoice SelectDriveL( );

    TBool S60Ui() { return iS60Ui; }
    void SetS60Ui( TBool aS60Ui ) { iS60Ui = aS60Ui; }

    void ChangeScreenLayoutL( TBool aForceS60Ui );

    void StartDownloadByTML();

	TInt Feature()  { return iFeature; }

	void SetTitlePaneTextL( const TDesC& aText );
	
	TBool GotFeature();
	TInt SearchStartIndex();
	
private:
	void InternalConstructL();

	void AboutL();
	void ChangeOrderL( TOrderBy aOrderBy );
	static TInt CallbackScanningL( TAny* aThis );
	TInt NextScanningStep();

private:
	TVersion iVersion;

	TBool iForeground;
	CEmTubeHttpEngine *iHttpEngine;
	CEmTubeHttpEngine *iLoginHttpEngine;

	TBool iNewSearch;
	TInt iSearchMaxResults;
	RPointerArray<CVideoEntry> iSearchEntries;
	RPointerArray<CVideoEntry> iTmpSearchEntries;
	RPointerArray<CVideoEntry> iFavsEntries;

	RPointerArray<CVideoEntry> iSavedEntries;

	TBuf<256> iSearchString;
	TBool iSearchByUrl;

	TInt iMaxCacheSize;
	TBool iAutoRotate;
	TVideoScaleMode iVideoScaleMode;
	TInt iTempMemory;

	RFs iSession;
	TFileName iFullPrivatePath;

	//image loading
	TInt iCurrentIndex;
	CImageLoader* iImageLoader;

	TInt iCurrentViewId;
	TInt iPreviousViewId;

	TSearchDisplayMode iSearchDisplayMode;

	TOrderBy iOrderBy;
	TPeriod iTimeFrame;
	TInt iSearchNaviPaneId;

	TBool iEmbedded;
	TStartPlaybackMode iStartPlaybackMode;
	TInt iVolume;
	HBufC* iTmpVideoFile;

	TFileName iSaveLoadDirectory;

	HBufC* iLastVideoUrl;

	CVideoEntry* iEntry;

	CEmTubeCache* iCache;

	CAknWaitDialog* iCheckForUpdatesDialog;
	CAknWaitDialog* iScannerDialog;
	CAknWaitDialog* iLoginDialog;
	CIdle* iIdle;
	TInt iScanningStep;

	CEmTubeTransferManager* iTransferManager;

	TInt iAccessPoint;
	TInt iSelectedAccessPoint;

	RPointerArray<HBufC> iSearchHistory;

    CEmTubeInboxFlvFinder* iInboxFlvFinder;

    CEmTubePluginManager* iPluginManager;
	TUint32 iPluginUid;

    CEmTubePlaylistManager* iPlaylistManager;

	TInt iFeature;
	TBuf<256> iSearchStringBase;

	TBool iStartDownloadByTM;

	TUint32 iLoginUid;

	TBool iS60Ui;

	CVideoEntry* iPlaylistVideoEntry;

	};

#endif // EMTUBE_APPUI_H

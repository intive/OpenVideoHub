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

#include <string.h>
#include <e32base.h>
#include <S32FILE.H>
#include <apgcli.h>
#include <apgtask.h>
#include <apmstd.h>
#include <BAUTILS.H>
#ifdef EKA2	// S60 3rd Ed
#include <rsendas.h>
#include <rsendasmessage.h>
#include <senduiconsts.h>
#endif
#include <HLPLCH.H>
#include <aknpopup.h>
#include <aknlists.h>
#include <akniconarray.h>
#include <akncontext.h>
#include <avkon.mbg>
#include <EscapeUtils.h>

#include <PluginInterface.h>

#ifndef EMTUBE_UIQ
#include <DocumentHandler.h>
#include <akndef.h>
#include <aknsconstants.h>
#include <aknmessagequerydialog.h>
#include <maknfilefilter.h>
#include <caknfileselectiondialog.h>
#include <stringloader.h>
#include <AknWaitDialog.h>
#include <AknCommonDialogs.h> // For single function calls
#include <CAknMemorySelectionDialog.h>
#include <CAknMemorySelectionSettingPage.h>
#include <CAknFileSelectionDialog.h>
#include <CAknFileNamePromptDialog.h>
#include <MAknFileFilter.h>
#include <akndoc.h>
#include <akntitle.h>
#else
#endif

#include "emTubeFileScanner.h"
#include "emTubeAppUi.h"
#include "emTube.hrh"
#include "emTubeApplication.h"
#include "emTubeMainView.h"
#include "emTubeSearchView.h"
#include "emTubeServersView.h"
#include "emTubeSearchViewContainer.h"
#include "emTubeSettingsView.h"
#include "emTubeSplashView.h"
#include "emTubeTransferView.h"
#include "emTubePlayView.h"
#include "emTubePlaylistView.h"
#include "emTubeCache.h"
#include "emTube.pan"
#include "emTubeLog.h"
#include "emTubeTransferManager.h"
#include "emTubeThumbnail.h"

#include "emTubePluginManager.h"
#include "emTubePlaylistManager.h"

#include "OpenVideohub.hlp.hrh"

#include "emTubeResource.h"

#include "avlibaudio.h"
#include "avlibvideo.h"

#include "emTubeHttpEngine.h"
#include "emTubeVideoEntry.h"

#include "emTubeInboxFlvFinder.h"

#define MAX_SEARCH_HISTORY_ENTRIES 20

void CEmTubeAppUi::ConstructL()
	{
#ifndef EMTUBE_UIQ
	BaseConstructL( EAknEnableSkin );
	SetKeyBlockMode( ENoKeyBlock );
#else
	BaseConstructL();
#endif

	InternalConstructL();
	}

void CEmTubeAppUi::InternalConstructL()
	{
	iSession.Connect();

	// Set priority
	RThread myThread;
	myThread.SetPriority(EPriorityMore);

	SetEmbedded( EFalse );

	iPlaylistManager = CEmTubePlaylistManager::NewL();

	CEmTubeMainView* mainView = CEmTubeMainView::NewLC();
	AddViewL( mainView );
	CleanupStack::Pop( mainView );

	CEmTubePlayView* playView = CEmTubePlayView::NewLC( iPlaylistManager );
	AddViewL( playView );
	CleanupStack::Pop( playView );

	CEmTubeSearchView* searchView = CEmTubeSearchView::NewLC( iPlaylistManager );
	AddViewL( searchView );
	CleanupStack::Pop( searchView );

	CEmTubeSettingsView* settingsView = CEmTubeSettingsView::NewLC();
	AddViewL( settingsView );
	CleanupStack::Pop( settingsView );

	CEmTubeTransferView* transferView = CEmTubeTransferView::NewLC();
	AddViewL( transferView );
	CleanupStack::Pop( transferView );

	CEmTubeSplashView* splashView = CEmTubeSplashView::NewLC();
	AddViewL( splashView );
	CleanupStack::Pop( splashView );

	CEmTubeServersView* siteView = CEmTubeServersView::NewLC();
	AddViewL( siteView );
	CleanupStack::Pop( siteView );

	CEmTubePlaylistView* playlistView = CEmTubePlaylistView::NewLC( iPlaylistManager );
	AddViewL( playlistView );
	CleanupStack::Pop( playlistView );

	iCurrentViewId = EMTVSplashViewId;
	SetDefaultViewL( *splashView );

	iPluginManager = CEmTubePluginManager::NewL();

	iHttpEngine = CEmTubeHttpEngine::NewL( *iPluginManager, EFalse );
	iLoginHttpEngine = CEmTubeHttpEngine::NewL( *iPluginManager, ETrue );

//	iStreamer = CStreamer::NewL();

	iImageLoader = CImageLoader::NewL( *this );

#ifndef __WINS__
	TParsePtrC parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
	iFullPrivatePath.Copy( parse.Drive() );
	TPath privateDir;
	User::LeaveIfError( iSession.PrivatePath( privateDir ) );
	iFullPrivatePath.Append( privateDir );
#else
	iFullPrivatePath.Copy( _L("C:\\Data\\") );
#endif

	iTransferManager = CEmTubeTransferManager::NewL();

	iSession.MkDirAll( iFullPrivatePath );

	TRAP_IGNORE( LoadSettingsL() );
	TRAP_IGNORE( LoadFavoritesL() );
	TRAP_IGNORE( LoadSavedClipsL() );
	TRAP_IGNORE( LoadSearchHistoryL() );

	if( !MMCAvailable() )
		iTempMemory = 0;

	SetTempMemory( iTempMemory );

	iCache = CEmTubeCache::NewL( iMaxCacheSize );

	iInboxFlvFinder = CEmTubeInboxFlvFinder::NewL();

	if( PluginManager().SelectPlugin( iPluginUid ) != KErrNotFound )
		{
//		SetTitlePaneTextL( PluginManager().Plugin()->Name() );
		PluginManager().SetDefaultPluginUid( PluginManager().Uid() );
		}

	AV_VideoRegister();

/* change status pane icon - not used anymore.
 	TUid contextPaneUid = TUid::Uid( EEikStatusPaneUidContext );
	CEikStatusPaneBase::TPaneCapabilities subPaneContext = StatusPane()->PaneCapabilities( contextPaneUid );
	if ( subPaneContext.IsPresent() && subPaneContext.IsAppOwned() )
		{
		CAknContextPane* context1 = static_cast< CAknContextPane* > ( StatusPane()->ControlL( contextPaneUid ) );
		context1->SetPictureFromFileL( KBitmapFileName, EMbmOpenvideohubSp_appicon, EMbmOpenvideohubSp_appicon_mask );
		}
*/
	SetOrientationL( CAknAppUiBase::EAppUiOrientationPortrait );
	}

void CEmTubeAppUi::SetTitlePaneTextL( const TDesC& aText )
	{
	TUid titlePaneUid = TUid::Uid( EEikStatusPaneUidTitle );
	CEikStatusPaneBase::TPaneCapabilities subPaneTitle = StatusPane()->PaneCapabilities( titlePaneUid );
	if ( subPaneTitle.IsPresent() && subPaneTitle.IsAppOwned() )
		{
		CAknTitlePane* title = static_cast< CAknTitlePane* >( StatusPane()->ControlL( titlePaneUid ) );
		title->SetTextL( aText );
		}
	}

CEmTubeAppUi::CEmTubeAppUi()
	:iVersion( 1, 0, 38),
	iForeground( ETrue ),
	iSearchMaxResults( 5 ),
	iMaxCacheSize( 512 * 1024 ),
	iAutoRotate( ETrue ),
	iVideoScaleMode( EVideoScaleNone ),
	iTempMemory( 0 ), //C drive
	iOrderBy( ERelevance ),
	iVolume( 5 ),
	iS60Ui( ETrue )
	{
	}

CEmTubeAppUi::~CEmTubeAppUi()
	{
	SetEmbedded( ETrue ); //workaround to avoid crash in player view setpanetitle

	AV_VideoCleanup();
	AV_AudioCleanup();

	delete iPlaylistVideoEntry;
	delete iPlaylistManager;
	delete iTransferManager;
	delete iCache;
	delete iEntry;

	delete iLastVideoUrl;

	if( iTmpVideoFile )
		BaflUtils::DeleteFile( iSession, *iTmpVideoFile );
	delete iTmpVideoFile;

	iSearchHistory.ResetAndDestroy();
	iSearchHistory.Close();

	iTmpSearchEntries.ResetAndDestroy();
	iTmpSearchEntries.Close();

	iSearchEntries.ResetAndDestroy();
	iSearchEntries.Close();

	iFavsEntries.ResetAndDestroy();
	iFavsEntries.Close();

	iSavedEntries.ResetAndDestroy();
	iSavedEntries.Close();

	delete iPluginManager;

	delete iInboxFlvFinder;

	delete iIdle;

	delete iHttpEngine;
	delete iLoginHttpEngine;
	delete iImageLoader;
	iSession.Close();

	}

class CFileFilter : public MAknFileFilter
	{
	TBool Accept(const TDesC &/*aDriveAndPath*/, const TEntry &aEntry) const
		{
		if( aEntry.IsDir() || aEntry.iName.Right(4) == _L(".flv") || aEntry.iName.Right(4) == _L(".FLV"))
			return ETrue;
		return EFalse;
		}
	};


TInt CEmTubeAppUi::CallbackScanningL( TAny* aThis )
	{
	return static_cast<CEmTubeAppUi*>( aThis )->NextScanningStep();
	}

TInt CEmTubeAppUi::NextScanningStep()
	{
	switch( iScanningStep )
		{
		case 0:
			{
			CEmTubeFileScanner *scanner = CEmTubeFileScanner::NewLC();
			scanner->ScanDirectoryL( _L("C:\\Data\\"), iSavedEntries );
			CleanupStack::PopAndDestroy( scanner );
			}
		break;

		case 1:
			{
			CEmTubeFileScanner *scanner = CEmTubeFileScanner::NewLC();
			TRAP_IGNORE( scanner->ScanDirectoryL( _L("E:\\"), iSavedEntries ) );
			CleanupStack::PopAndDestroy( scanner );
			}
		break;

		case 2:
			{
			CThumbnailRetriever* tr = CThumbnailRetriever::NewLC();
			for(TInt i=iSavedEntries.Count()-1;i>=0;i--)
				{
				CVideoEntry* e = iSavedEntries[i];

				if( !BaflUtils::FileExists( iSession, e->SavedFileName() ) )
					{
					DeleteFromSavedClipsL( e );
					}
				else
					{
					if( e->ThumbnailFile().Length() )
						{
						if( !BaflUtils::FileExists( iSession, e->ThumbnailFile() ) )
							{
							e->SetThumbnailFileL( KNullDesC() );
							}
						}

					if( !e->ThumbnailFile().Length() || !e->Duration() )
						{
						TFileName thumb;
						TParsePtrC parse( e->SavedFileName() );
						thumb.Copy( iFullPrivatePath );
						thumb.Append( parse.Name() );
						thumb.Append( KCacheFileExtension() );
						e->SetThumbnailFileL( thumb );

						tr->RetrieveThumbnailL( e );
						}
					}
				}
			CleanupStack::PopAndDestroy( tr );
			}
		break;
		}

	iScanningStep++;
	if( iScanningStep >= 3 )
		{
		SaveSavedClipsL();

		iScannerDialog->ProcessFinishedL();
		delete iScannerDialog;
		iScannerDialog = NULL;

		CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
		sv->DisplaySearchResultsL();

		return 0;
		}
	return 1;
	}

void CEmTubeAppUi::HandleCommandL(TInt aCommand)
	{
	switch(aCommand)
		{
		case EMTVSortByRelevanceCommand:
			ChangeOrderL( ERelevance );
		break;

		case EMTVSortByDateAddedCommand:
			ChangeOrderL( EUpdated );
		break;

		case EMTVSortByViewCountCommand:
			ChangeOrderL( EViewCount );
		break;

		case EMTVSortByRatingCommand:
			ChangeOrderL( ERating );
		break;

		case EMTVScanDirectoriesCommand:
			{
			iScanningStep = 0;
			iScannerDialog = new ( ELeave )CAknWaitDialog( reinterpret_cast<CEikDialog**>( &iScannerDialog ), ETrue );
			iScannerDialog->ExecuteDlgLD( CAknNoteDialog::ENoTone, R_SCANNING_DRIVES_NOTE );

			delete iIdle;
			iIdle = CIdle::NewL( CActive::EPriorityStandard );
			TCallBack callback( CallbackScanningL, this );
			iIdle->Start( callback );
			}
		break;

		case EAknCmdHelp:
		case EMTVHelpCommand:
			{
			CArrayFix <TCoeHelpContext>* buf = CCoeAppUi::AppHelpContextL();
			HlpLauncher::LaunchHelpApplicationL( iEikonEnv->WsSession(), buf );
			}
		break;

		case EMTVCheckForUpdate:
			iCheckForUpdatesDialog = new ( ELeave )CAknWaitDialog( reinterpret_cast<CEikDialog**>( &iCheckForUpdatesDialog ), ETrue );
			iCheckForUpdatesDialog->ExecuteDlgLD( CAknNoteDialog::ENoTone, R_CHECKING_FOR_UPDATES_NOTE );
			iHttpEngine->CheckForUpdateL( *this );
		break;

		case EMTVOpenVideoByIdCommand:
			{
			TBuf<256> id;
			CAknTextQueryDialog* dlg = new(ELeave)CAknTextQueryDialog( id, CAknQueryDialog::ENoTone );
			dlg->SetPredictiveTextInputPermitted( ETrue );
			TInt ret = dlg->ExecuteLD( R_EMTV_ENTER_ID_DIALOG );
			if( ret )
				{
				HBufC* url = PluginManager().Plugin()->CreateUrlFromIdL( id );
				CleanupStack::PushL( url );

				delete iEntry;
				iEntry = CVideoEntry::NewL();
				iEntry->SetVideoUrlL( *url );

				CancelDownloadImageL();
				OpenFileL( iEntry );

				CleanupStack::PopAndDestroy( url );
				}
			}
		break;

		case EMTVPlayLocalFileCommand:
			{
			TInt ret;
			TFileName fileName;

			TBool done = EFalse;
			TBool memDialog = ETrue;
			TBool inboxFile = EFalse;
			TMsvId messageId = 0;

			if( iSaveLoadDirectory.Length() )
				memDialog = EFalse;

			while( !done )
				{
				done = EFalse;
				inboxFile = EFalse;
				ret = 0;

				if( memDialog )
					{
					fileName.Copy( KNullDesC() );

					CEmTubeAppUi::TMemoryChoice  memoryChoice = SelectDriveL();

					switch(memoryChoice)
					{
						case EMemoryPhone:
						{
							fileName.Copy( _L("C:\\") );
							break;
						}

						case EMemorySd:
						{
							fileName.Copy( _L("E:\\") );
							break;
						}

						case EMemoryInbox:
						{
							inboxFile = ETrue;
							break;
						}

						default:
							done = ETrue;
							break;
					}
					}
				else
					{
					TParsePtrC parse( iSaveLoadDirectory );
					fileName.Copy( parse.Drive() );
					fileName.Append( _L("\\") );
					ret = 1;
					}

				memDialog = ETrue;

				if( inboxFile )
					{
//					iInboxFlvFinder->FindFlvL();
					CAknSinglePopupMenuStyleListBox* plist = new(ELeave) CAknSinglePopupMenuStyleListBox;
					CleanupStack::PushL(plist);

					CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_SELECT_CANCEL, AknPopupLayouts::EMenuWindow);
					CleanupStack::PushL(popupList);

					plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
					plist->CreateScrollBarFrameL(ETrue);
					plist->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

					MDesCArray* itemList = plist->Model()->ItemTextArray();
					CDesCArray* items = (CDesCArray*) itemList;

					RArray<TFileName>*  names = iInboxFlvFinder->GetNamesArray( );
					for(TInt i=0 ; i < names->Count() ; i++)
					{
						items->AppendL( (*names)[i] );
					}

					popupList->SetTitleL( _L("Select video:") );

					TInt ret = popupList->ExecuteLD();

					if( ret )
						{
						messageId = (*iInboxFlvFinder->GetIdsArray( ))[plist->CurrentItemIndex()];
						done = ETrue;
						}

					CleanupStack::Pop( ); //popupList
					CleanupStack::PopAndDestroy( );	//list
					}
				else if(!done)
					{
					CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL( ECFDDialogTypeSelect, R_FILE_SELECTION_DIALOG );
					CFileFilter* filter = new(ELeave) CFileFilter;

					if( iSaveLoadDirectory.Length() )
						{
						TParsePtrC parse( iSaveLoadDirectory );
						TPtrC path = parse.Path();
						dlg->SetDefaultFolderL( path.Right( path.Length() - 1 ) );
						}

					dlg->AddFilterL( filter );
					ret = dlg->ExecuteL( fileName );
					delete dlg;

					if( ret )
						{
						done = ETrue;
						}
					else
						{
						 fileName.Copy(_L(""));
						}

					SetSaveLoadDirectory( fileName );
					}
				}

			if( inboxFile )
				{
				RFile file;
				if( iInboxFlvFinder->GetFlvL( messageId, file ) )
					{
					file.FullName( fileName );
					iPlaylistManager->UpdateSavedClipsStatsL( fileName, fileName );
					OpenFileL( file );
					}
				}
			else if( fileName.Length() )
				{
				CVideoEntry *e = NULL;
				TInt idx = CEmTubeFileScanner::Find( fileName, iSavedEntries );
				if(  idx == KErrNotFound )
					{
					e = CVideoEntry::NewLC();
					TParsePtrC parse( fileName );

					e->SetSavedFileName( fileName );
					e->SetVideoFileSize( 0 );
					e->SetDownloadFinished( ETrue );
					e->SetTitleL( parse.Name() );
					e->SetAuthorNameL( KNullDesC() );
					iSavedEntries.Append( e );

					iPlaylistManager->UpdateSavedClipsStatsL( fileName, e->MediaTitle() );

					CleanupStack::Pop( e );

					CThumbnailRetriever* tr = CThumbnailRetriever::NewLC();

					TFileName thumb;
					thumb.Copy( iFullPrivatePath );
					thumb.Append( parse.Name() );
					thumb.Append( KCacheFileExtension() );
					e->SetThumbnailFileL( thumb );
					tr->RetrieveThumbnailL( e );
					CleanupStack::PopAndDestroy( tr );

					}
				else
					{
					e = iSavedEntries[ idx ];
					}

				OpenFileL( fileName, e->MediaTitle() );
				}
			}
		break;

		case EMTVSearchNextCommand:
			{
			CancelDownloadImageL();
			SearchL();
			}
		break;

		case EEikCmdExit:
		case EAknCmdExit:
		case EAknSoftkeyExit:
		case EMTVExitCommand:
			{
			SaveSettingsL();
			SaveFavoritesL();
			SaveSavedClipsL();
			SaveSearchHistoryL();
			iPlaylistManager->ExportPlaylistsL();
		    iTransferManager->SaveQueueL();
		    PluginManager().ExportCredentialsL();
			Exit();
			}
		break;

		case EMTVActivateServersViewCommand:
			{
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVServersViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVServersViewId ) );
			}
		break;

		case EAknSoftkeyBack:
		case EMTVActivatePreviousViewCommand:
			{
			iCurrentViewId = iPreviousViewId;
			ActivateLocalViewL ( TUid::Uid( iCurrentViewId ) );
			}
		break;

		case EMTVActivatePlaylistsViewCommand:
			{
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVPlaylistViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVPlaylistViewId ) );
			}
		break;

		case EMTVActivateTransferManagerViewCommand:
			{
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVTransferViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVTransferViewId ) );
			}
		break;

		case EMTVActivatePlayViewCommand:
			{
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVPlayViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVPlayViewId ) );
			}
		break;

		case EMTVActivateMainViewCommand:
			{
			ClearSearchResults();

			if( PluginManager().DefaultPluginUid() )
				{
				PluginManager().SelectPlugin( PluginManager().DefaultPluginUid() );

//				SetTitlePaneTextL( PluginManager().Plugin()->Name() );

				iPreviousViewId = iCurrentViewId;
				iCurrentViewId = EMTVMainViewId;
				ActivateLocalViewL ( TUid::Uid( EMTVMainViewId ) );
				}
			else
				{
				iPreviousViewId = iCurrentViewId;
				iCurrentViewId = EMTVServersViewId;
				ActivateLocalViewL ( TUid::Uid( EMTVServersViewId ) );
				}
			}
		break;

		case EMTVActivateSettingsViewCommand:
			{
			CancelDownloadImageL();
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVSettingsViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVSettingsViewId ) );
			}
		break;

		case EMTVActivateSearchViewCommand:
			{
			iPreviousViewId = iCurrentViewId;
			iCurrentViewId = EMTVSearchViewId;
			ActivateLocalViewL ( TUid::Uid( EMTVSearchViewId ) );
			}
		break;

		case EMTVProgressDialogCancel:
			{
			iHttpEngine->CancelOperationL();
			}
		break;

		case EMTVSearchCommand:
			{
			SearchDialogL();
			}
		break;

		case EMTVAboutCommand:
			{
			AboutL();
			}
		break;

		case EMTVShowFavoritesCommand:
			{
			if( iFavsEntries.Count() )
				{
				CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
				sv->SetCurrentItemIndex( 0 );
				iSearchDisplayMode = ESearchDisplayFavEntries;
				HandleCommandL( EMTVActivateSearchViewCommand );
				}
			}
		break;

		case EMTVShowSavedClipsCommand:
			{
			CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
			sv->SetCurrentItemIndex( 0 );
			iSearchDisplayMode = ESearchDisplaySavedEntries;
			HandleCommandL( EMTVActivateSearchViewCommand );
			}
		break;

		case EMTVDisplaySearchResultsCommand:
			{
			CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
			sv->DisplaySearchResultsL();
			}
		break;

		default:
		break;
		}
	}

RPointerArray<CVideoEntry>& CEmTubeAppUi::SearchEntries()
	{
	if ( iSearchDisplayMode == ESearchDisplayFavEntries )
		return iFavsEntries;
	else if( iSearchDisplayMode == ESearchDisplaySavedEntries )
		return iSavedEntries;

	return iSearchEntries;
	}

void CEmTubeAppUi::HandleForegroundEventL(TBool aForeground)
	{
	iForeground = aForeground;
	if( !iForeground )
		{
		CEmTubePlayView* pv = static_cast<CEmTubePlayView*>( View( TUid::Uid(EMTVPlayViewId) ) );
		pv->PlayPauseL( ETrue );
		}
	}

CArrayFix <TCoeHelpContext>* CEmTubeAppUi::HelpContextL() const
	{
	CArrayFixFlat <TCoeHelpContext>* array = new ( ELeave )CArrayFixFlat <TCoeHelpContext> ( 1 );
	CleanupStack::PushL( array );
	array->AppendL( TCoeHelpContext( KUidEmTubeApp, KContextApplication ) );
	CleanupStack::Pop( array );
	return array;
	}

TDesC& CEmTubeAppUi::LastVideoUrl()
	{
	if( !iLastVideoUrl )
		{
		iLastVideoUrl = KNullDesC().AllocL();
		}
	return *iLastVideoUrl;
	}

void CEmTubeAppUi::SetLastVideoUrlL( TDesC& aLast )
	{
	delete iLastVideoUrl;
	iLastVideoUrl = aLast.AllocL();
	}

void CEmTubeAppUi::LoginL( TUint32 aUid, MHttpEngineObserver& aObserver, TBool aPost )
	{
	iLoginHttpEngine->LoginL( aUid, aObserver, aPost );
	}

void CEmTubeAppUi::LoginL( TUint32 aUid, TBool aPost )
	{
	if( !aPost )
		{
		iLoginDialog = new ( ELeave )CAknWaitDialog( reinterpret_cast<CEikDialog**>( &iLoginDialog ), ETrue );
		iLoginDialog->ExecuteDlgLD( CAknNoteDialog::ENoTone, R_LOGGING_IN_NOTE );
		}
		
	iLoginUid = aUid;
	iLoginHttpEngine->LoginL( aUid, *this, aPost );
	}

void CEmTubeAppUi::DownloadMovieL( CVideoEntry* aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	BaflUtils::DeleteFile( iSession, *iTmpVideoFile );
	TInt startOffset = 0;

	if( iLastVideoUrl && !aEntry->Url().Compare( *iLastVideoUrl ) )
		{
		RFile file;
		TInt err = file.Open( iSession, VideoFileName(), EFileStream | EFileRead | EFileShareAny );
		if( err == KErrNone )
			{
			file.Size( startOffset );
			file.Close();
			}
		}
	SetLastVideoUrlL( aEntry->Url() );
	iHttpEngine->DownloadMovieL( VideoFileName(), *aEntry, startOffset, aObserver, aProgressObserver );
	}

void CEmTubeAppUi::DownloadMovieL( TDesC& aFile, CVideoEntry &aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	TInt startOffset = 0;
	RFile file;
	TInt err = file.Open( iSession, aFile, EFileStream | EFileRead | EFileShareAny );
	if( err == KErrNone )
		{
		file.Size( startOffset );
		file.Close();
		}
	iHttpEngine->DownloadMovieL( aFile, aEntry, startOffset, aObserver, aProgressObserver );
	SetLastVideoUrlL( aEntry.Url() );
	}

void CEmTubeAppUi::CancelDownloadMovieL()
	{
	if( iHttpEngine )
		iHttpEngine->CancelOperationL();

	BaflUtils::DeleteFile( iSession, *iTmpVideoFile );
	}

void CEmTubeAppUi::CancelDownloadImageL()
	{
	iCurrentIndex = -1;
	iImageLoader->StopLoadingImage();
	if( iHttpEngine )
		iHttpEngine->CancelOperationL();
	}

void CEmTubeAppUi::DownloadImageL( TInt aIndex  )
	{
	DownloadImageL( SearchEntries()[ aIndex ], *this );
	}

void CEmTubeAppUi::DownloadImageL( CVideoEntry* aEntry, MHttpEngineObserver& aObserver )
	{
	if( aEntry->ThumbnailFile().Length() && iCache->IsFileInCache( aEntry->ThumbnailFile() ) )
		{
		HBufC8*tmp = NULL;
		aObserver.RequestFinishedL( CEmTubeHttpEngine::ERequestImageDownload, *tmp );
		}
	else
		{
		if( !aEntry->ThumbnailUrl().Length() )
			{
			HBufC8*tmp = NULL;
			aObserver.RequestFinishedL( CEmTubeHttpEngine::ERequestImageDownload, *tmp );
			}
		else
			{
			iCache->UpdateCacheL();
			iHttpEngine->DownloadImageL( aEntry, aObserver );
			}
		}
	}

void CEmTubeAppUi::RequestCanceledL( TInt aRequest )
	{
	switch( aRequest )
		{
		case CEmTubeHttpEngine::ERequestPreLogin:
		case CEmTubeHttpEngine::ERequestLogin:
			{
			if( iLoginDialog )
				{
				iLoginDialog->ProcessFinishedL();
				delete iLoginDialog;
				iLoginDialog = NULL;
				}
			PluginManager().SelectPlugin( PluginManager().DefaultPluginUid() );
			}
		break;

		case CEmTubeHttpEngine::ERequestCheckForUpdate:
			{
			if ( iCheckForUpdatesDialog )
				{
				iCheckForUpdatesDialog->ProcessFinishedL();
				}
			iCheckForUpdatesDialog = NULL;
			}
		break;

		case CEmTubeHttpEngine::ERequestSearch:
			{
			CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
			sv->CancelProgressL();
			}
		break;

		}
	}

void CEmTubeAppUi::RequestFinishedL( TInt aRequest, TDesC8& aResponseBuffer )
	{
	switch( aRequest )
		{
		case CEmTubeHttpEngine::ERequestPreLogin:
			{
			if( !PluginManager().Plugin( iLoginUid )->ParseLoginResponseL( aResponseBuffer ) )
				{
				HBufC8* b = NULL;
				RequestFinishedL( CEmTubeHttpEngine::ERequestLogin, *b );
				}
			else
				{
				LoginL( iLoginUid, ETrue );
				}
			}
		break;

		case CEmTubeHttpEngine::ERequestLogin:
			{
			if( iLoginDialog )
				{
				iLoginDialog->ProcessFinishedL();
				delete iLoginDialog;
				iLoginDialog = NULL;
				}

			if( !PluginManager().LoggedIn( iLoginUid ) )
				ShowErrorNoteL( R_INCORRECT_LOGIN_TXT );

			PluginManager().SelectPlugin( PluginManager().DefaultPluginUid() );

			CEmTubeServersView* ssv = static_cast<CEmTubeServersView*>( View( TUid::Uid(EMTVServersViewId) ) );
			ssv->UpdateListL();
			}
		break;

		case CEmTubeHttpEngine::ERequestCheckForUpdate:
			{
#ifdef ENABLE_CHECK_FOR_UPDATES_MENU_ITEM
			if ( iCheckForUpdatesDialog )
				{
				iCheckForUpdatesDialog->ProcessFinishedL();
				}
			iCheckForUpdatesDialog = NULL;

			TBuf8<24> version;
			version.Format(_L8("%d.%d.%d"), iVersion.iMajor, iVersion.iMinor, iVersion.iBuild );

			if( version.Compare( aResponseBuffer ) )
				{
				HBufC* txt = StringLoader::LoadLC( R_DIALOG_NEW_VERSION_AVAILABLE_TXT );

				CAknQueryDialog* dlg = CAknQueryDialog::NewL();
				dlg->SetPromptL( *txt );
				TInt ret = dlg->ExecuteLD( R_UPDATE_INFO_DIALOG );
				CleanupStack::PopAndDestroy( txt );

				if( ret == EAknSoftkeyYes )
					{
					TFileName pips;
					pips.Copy( iFullPrivatePath );
					pips.Append( _L("pips") );
					if( BaflUtils::FileExists( iSession, pips ) )
						{
						pips.Copy( _L("emTube_pips.sisx") );
						}
					else
						{
						pips.Copy( _L("emTube.sisx") );
						}

					RApaLsSession apaLsSession;
					const TUid KOSSBrowserUidValue = {0x10008d39};

					_LIT(KUrl, "www.emtube.yoyo.pl/");

					HBufC* param = HBufC::NewLC(KUrl().Length( ) + 2 + pips.Length() );
					param->Des().Copy( _L("4 ") );
					param->Des().Append( KUrl() );
					param->Des().Append( pips );

					TUid id(KOSSBrowserUidValue);
					TApaTaskList taskList(CEikonEnv::Static()->WsSession());
					TApaTask task = taskList.FindApp(id);
					if(task.Exists())
						{
						task.BringToForeground();
						HBufC8* param8 = HBufC8::NewLC(param->Length());
						param8->Des().Append(*param);
						task.SendMessage(TUid::Uid(0), *param8); // UID not used
						CleanupStack::PopAndDestroy(param8);
						}
					else
						{
						if(!apaLsSession.Handle())
							{
							User::LeaveIfError(apaLsSession.Connect());
							}
						TThreadId thread;
						User::LeaveIfError(apaLsSession.StartDocument(*param, KOSSBrowserUidValue, thread));
						apaLsSession.Close();
						}
					CleanupStack::PopAndDestroy(param);
					}
				}
			else
				{
				ShowErrorNoteL( R_DIALOG_NO_NEW_VERSION_AVAILABLE_TXT );
				}
#endif
			}
		break;

		case CEmTubeHttpEngine::ERequestImageDownload:
			{
			if( iCurrentIndex >= 0 )
				{
				CVideoEntry* entry = SearchEntries()[ iCurrentIndex ];
				iImageLoader->LoadFileL( entry );
				}
			}
		break;

		case CEmTubeHttpEngine::ERequestSearch:
			{
			TInt count = PluginManager().Plugin()->ParseSearchResponseL( aResponseBuffer, *this );

			if( count < 0 )
				{
				ShowErrorNoteL( R_SEARCH_NO_VIDEOS_FOUND_TXT );
				PluginManager().SetSearchState( ESearchFinished );
				}
			else if( count == 0 )
				{
				if( iNewSearch )
					{
					ShowErrorNoteL( R_SEARCH_NO_VIDEOS_FOUND_TXT );
					}
				else
					{
					PluginManager().SetSearchState( ESearchFinished );
					}
				}
			else
				{									
				for(TInt i=0;i<iTmpSearchEntries.Count();i++)
					{
					iTmpSearchEntries[i]->GenerateThumbnailFileL( iCache->CacheDirectory() );
					}

				for(TInt i=0;i<iTmpSearchEntries.Count();i++)
					{
					iSearchEntries.Append( iTmpSearchEntries[i] );
					}
				iTmpSearchEntries.Reset();

				TInt maxRes = iSearchMaxResults;
				if( PluginManager().Plugin()->Capabilities() & KCapsConstResultsPerPage )
					maxRes = PluginManager().Plugin()->SearchResultsPerPage();

				if( count < maxRes )
					PluginManager().SetSearchState( ESearchFinished );
				else
					PluginManager().SetSearchState( ESearching );				
				}
			iNewSearch = EFalse;

			HandleCommandL( EMTVDisplaySearchResultsCommand );
			}
			break;
		}
	}

void CEmTubeAppUi::StartDownloadingImagesL()
	{
	CancelDownloadImageL();

	iCurrentIndex = 0;
	if( SearchEntries().Count() )
		{
		DownloadImageL( iCurrentIndex );
		}
	}

void CEmTubeAppUi::ImageLoadedL( TInt aError )
	{
	CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
	switch( aError )
		{
		case KErrInUse:
			iCurrentIndex = -1; //stop downloading images now!
		break;
		
		case KErrAlreadyExists:
		break;

		case KErrNone:
			{

			if( iCurrentIndex >= 0 )
				{
				CVideoEntry* entry = SearchEntries()[ iCurrentIndex ];
				entry->SetImageLoaded( ETrue );

				if( iSearchDisplayMode != CEmTubeAppUi::ESearchDisplaySearchEntries )
					sv->ImageLoadedL( iCurrentIndex );				
				else
					if( entry->PluginUid() == PluginManager().Uid() )
						sv->ImageLoadedL( iCurrentIndex );
				}
			}
		break;

		default:
Log("error loading image!");
			{
			if( iCurrentIndex >= 0 )
				{
				sv->ImageLoadedL( iCurrentIndex );
				}
			}
//		return;
		}

	if( iCurrentIndex >= 0 )
		{
		iCurrentIndex++;
		
		if( iSearchDisplayMode == CEmTubeAppUi::ESearchDisplaySearchEntries )
			{
			while( (iCurrentIndex < SearchEntries().Count()) &&
			  (SearchEntries()[ iCurrentIndex ]->PluginUid() != PluginManager().Uid()) )
				{
				iCurrentIndex++;			
				}			
			}
		
		if( iCurrentIndex >= SearchEntries().Count() )
			{
			iImageLoader->Close();
			return;
			}
		
		DownloadImageL( iCurrentIndex );
		}
	}

/*
LOCAL_C TInt CallbackText1(TAny* aAny)
	{
#ifdef EKA2	// S60 3rd Ed
	RApaLsSession apaLsSession;
//	const TUid KOSSBrowserUidValue = {0x1020724D};
	const TUid KOSSBrowserUidValue = {0x10008d39};

_LIT(KUrl, "www.emtube.yoyo.pl");

	HBufC* param = HBufC::NewLC(KUrl().Length( ) + 2);
	param->Des().Copy( _L("4 ") );
	param->Des().Append( KUrl() );

	TUid id(KOSSBrowserUidValue);
	TApaTaskList taskList(CEikonEnv::Static()->WsSession());
	TApaTask task = taskList.FindApp(id);
	if(task.Exists())
		{
		task.BringToForeground();
		HBufC8* param8 = HBufC8::NewLC(param->Length());
		param8->Des().Append(*param);
		task.SendMessage(TUid::Uid(0), *param8); // UID not used
		CleanupStack::PopAndDestroy(param8);
		}
	else
		{
		if(!apaLsSession.Handle())
			{
			User::LeaveIfError(apaLsSession.Connect());
			}
		TThreadId thread;
		User::LeaveIfError(apaLsSession.StartDocument(*param, KOSSBrowserUidValue, thread));
		apaLsSession.Close();
		}
	CleanupStack::PopAndDestroy(param);
#endif
	return EFalse;
	}

LOCAL_C TInt CallbackText2(TAny* aAny)
	{
#ifdef EKA2	// S60 3rd Ed
	RSendAs sendas;
	TInt res = sendas.Connect();
	CleanupClosePushL(sendas);

	RSendAsMessage message;
	TRAP(res, message.CreateL(sendas, KSenduiMtmSmtpUid));
	CleanupClosePushL(message);

	_LIT(KRecipient, "elf@frogger.rules.pl");
	_LIT(KSubject, "emTube");

	TRAP( res, message.AddRecipientL(KRecipient, RSendAsMessage::ESendAsRecipientTo) );

	if( res == KErrNone )
		{
		message.SetSubjectL(KSubject);

		CleanupStack::Pop( &message );
		TRAP(res, message.LaunchEditorAndCloseL());
		}
	else
		{
		CleanupStack::Pop( &message );
		}
	CleanupStack::PopAndDestroy( &sendas );
#endif
	return EFalse;
	}
*/
void CEmTubeAppUi::AboutL( )
	{
	CAknMessageQueryDialog* about = new (ELeave)CAknMessageQueryDialog();
	CleanupStack::PushL(about);

	RBuf txt;
	CleanupClosePushL( txt );

	about->PrepareLC( R_EMTV_ABOUT_DIALOG );
	HBufC* title = StringLoader::LoadLC( R_EMTV_ABOUT_HEADER_TXT );
	about->QueryHeading()->SetTextL( *title );
	CleanupStack::PopAndDestroy( title );

	HBufC* str = StringLoader::LoadLC( R_EMTV_ABOUT_1_TXT );
	txt.Create( str->Length() );
	txt.Append( *str );
	CleanupStack::PopAndDestroy( str );

	TBuf<24> version;
	version.Format(_L(" %d.%d.%d\n"), iVersion.iMajor, iVersion.iMinor, iVersion.iBuild );
	txt.ReAllocL( txt.Length() + version.Length() );
	txt.Append( version );

	TPtr8 date8((TUint8 *)__DATE__, strlen(__DATE__), strlen(__DATE__) );
	HBufC* date = HBufC::NewLC( date8.Length() );
	date->Des().Copy( date8 );

	txt.ReAllocL( txt.Length() + date->Length() + 1 );
	txt.Append( *date );
	txt.Append( _L("\n") );
	CleanupStack::PopAndDestroy( date );

	str = StringLoader::LoadLC( R_EMTV_ABOUT_2_TXT );
	txt.ReAllocL( txt.Length() + str->Length() );
	txt.Append( *str );
	CleanupStack::PopAndDestroy( str );

	about->SetMessageTextL( txt );

/*
_LIT(KLink1, "http://emTube.yoyo.pl/");
_LIT(KLink2, "elf@frogger.rules.pl");

	TCallBack callback1(CallbackText1);
	about->SetLink(callback1);
	about->SetLinkTextL(KLink1);

	TCallBack callback2(CallbackText2);
	about->SetLink(callback2);
	about->SetLinkTextL(KLink2);
*/
	about->RunLD();

	CleanupStack::PopAndDestroy( &txt );
	CleanupStack::Pop( about );
	}

TBool CEmTubeAppUi::MMCAvailable()
	{
	TBool res ( EFalse );

	TDriveInfo i;
	if ( iSession.Drive(i, EDriveE)==KErrNone)
		{
		if ( i.iType!=EMediaNotPresent &&
			 i.iType!=EMediaUnknown &&
			 i.iType!=EMediaCdRom &&
			 i.iType!=EMediaRom)
			{
			res = ETrue;
			}
		}

	return res;
	}

void CEmTubeAppUi::ChangeOrderL( TOrderBy aOrderBy )
	{
	if( aOrderBy != iOrderBy )
		{
		iOrderBy = aOrderBy;

		ClearSearchResults();
		PluginManager().ResetPluginStates();
		PluginManager().SetTemporaryPluginUid( 0 );

		CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
		sv->SetCurrentItemIndex( 0 );
		sv->StartProgressBarL( EFalse );

		iHttpEngine->SetClientObserver( NULL );

		CancelDownloadImageL();
		SearchL();
		}
	}

void CEmTubeAppUi::ClearSearchResults()
	{
	iSearchEntries.ResetAndDestroy();
	}

TBool CEmTubeAppUi::SearchDialogL()
	{
	if( iSearchString.Find( _L("http://") ) == 0 )
		iSearchString.Zero();


	TInt ret = 0;
	TInt idx = 0;
	if( iSearchHistory.Count() )
		{
		CAknSinglePopupMenuStyleListBox* plist = new(ELeave) CAknSinglePopupMenuStyleListBox;
		CleanupStack::PushL(plist);

		CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_MENU_LIST, AknPopupLayouts::EPopupSNotePopupWindow);
		CleanupStack::PushL(popupList);

		plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
		plist->CreateScrollBarFrameL(ETrue);
		plist->ScrollBarFrame()->SetScrollBarVisibilityL(
								   CEikScrollBarFrame::EOff,
								   CEikScrollBarFrame::EAuto);

		MDesCArray* itemList = plist->Model()->ItemTextArray();
		CDesCArray* items = (CDesCArray*) itemList;

		HBufC* txt = StringLoader::LoadLC( R_NEW_SEARCH_TXT );
		items->AppendL( *txt );
		CleanupStack::PopAndDestroy( txt );

		popupList->SetTitleL( KNullDesC() );

		for(TInt i=0;i<iSearchHistory.Count();i++)
			{
			items->AppendL( *(iSearchHistory[i]) );
			}

		TInt popupOk = popupList->ExecuteLD();
		if(popupOk)
			{
			idx = plist->CurrentItemIndex();
			switch( idx )
				{
				case 0:
				break;

				default:
					{
					iSearchString.Copy( *(iSearchHistory[idx - 1]) );
					HBufC* ptr = iSearchHistory[idx - 1];
					iSearchHistory.Remove( idx - 1 );
					iSearchHistory.Insert( ptr, 0 );
					ret = 1;
					}
				break;
				}
			}
		else
			{
			idx = -1;
			ret = 0;
			}
		CleanupStack::Pop( popupList );
		CleanupStack::PopAndDestroy( plist );
		}

	if( idx == 0 )
		{
		CAknTextQueryDialog* dlg = new(ELeave)CAknTextQueryDialog(iSearchString, CAknQueryDialog::ENoTone );
		dlg->SetPredictiveTextInputPermitted( ETrue );
		ret = dlg->ExecuteLD( R_EMTV_SEARCH_DIALOG );
		}

	if( ret )
		{
		PluginManager().SetTemporaryPluginUid( 0 );
		AddToSearchHistoryL( iSearchString );
		SetSearchNaviPaneId( R_SEARCH_RESULTS_TXT );
		StartSearchL( EFalse );
		return ETrue;
		}
	return EFalse;
	}

void CEmTubeAppUi::SearchL( TInt aFeature, TOrderBy aOrderBy, TPeriod aTimeFrame )
	{
	HBufC* url = NULL;

	iFeature = aFeature;
	iTimeFrame = aTimeFrame;

	ClearSearchResults();	
	PluginManager().ResetPluginStates();
	PluginManager().SetTemporaryPluginUid( 0 );

	switch( aFeature )
		{
			case EFeaturedClips:
				{
				SetSearchNaviPaneId( R_SEARCH_FEATURED_TXT );
				url = PluginManager().Plugin()->FeatureUrlL( (TFeature)aFeature, iSearchMaxResults, 1, (TOrderBy)aOrderBy, EPeriodIrrelevant );
				}
			break;

			case ETopRatedClips:
				{
				SetSearchNaviPaneId( R_SEARCH_TOPRATED_TXT );
				url = PluginManager().Plugin()->FeatureUrlL( (TFeature)aFeature, iSearchMaxResults, 1, (TOrderBy)aOrderBy, aTimeFrame );
				}
			break;

			case EMostViewedClips:
				{
				SetSearchNaviPaneId( R_SEARCH_MOSTWATCHED_TXT );
				url = PluginManager().Plugin()->FeatureUrlL( (TFeature)aFeature, iSearchMaxResults, 1, (TOrderBy)aOrderBy, aTimeFrame );
				}
			break;

			case ENewClips:
				{
				SetSearchNaviPaneId( R_SEARCH_NEWCLIPS_TXT );
				url = PluginManager().Plugin()->FeatureUrlL( (TFeature)aFeature, iSearchMaxResults, 1, (TOrderBy)aOrderBy, EPeriodIrrelevant );
				}
			break;

			case EUserClips:
				{
				PluginManager().SetTemporaryPluginUid( PluginManager().Uid() );
				SetSearchNaviPaneId( R_SEARCH_USERVIDEOS_TXT );
				iSearchStringBase.Copy( iSearchString );
				url = PluginManager().Plugin()->UserVideosUrlL( iSearchString, iSearchMaxResults, 1, (TOrderBy)aOrderBy );
				}
			break;

			case ERelatedClips:
				{
				PluginManager().SetTemporaryPluginUid( PluginManager().Uid() );
				SetSearchNaviPaneId( R_SEARCH_RELATED_TXT );
				iSearchStringBase.Copy( iSearchString );
				url = PluginManager().Plugin()->RelatedVideosUrlL( iSearchString, iSearchMaxResults, 1, (TOrderBy)aOrderBy );
				}
			break;

		}

	if( !GotFeature() )
		{
		delete url;
		iSearchDisplayMode = ESearchDisplaySearchEntries;
		HandleCommandL( EMTVActivateSearchViewCommand );
		
		return;			
		}

	iSearchString.Copy( *url );
	delete url;

	StartSearchL( ETrue );
	}

void CEmTubeAppUi::SearchL()
	{
	TInt searchStartIndex = SearchStartIndex();
				
	if( iFeature == -1 )
		{
		RBuf query;
		CleanupClosePushL( query );

		query.Create( iSearchString.Length() );
		for(TInt i=0;i<iSearchString.Length();i++)
			{
			if( iSearchString[i] == ' ' )
				query.Append( '+' );
			else
				query.Append( iSearchString[i] );
			}

		HBufC* url = PluginManager().Plugin()->SearchUrlL( query, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy );
		CleanupStack::PushL( url );

//		CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
		iHttpEngine->SearchL( *url, *this );

		CleanupStack::PopAndDestroy( url );

		CleanupStack::PopAndDestroy( &query );
		}
	else
		{
		HBufC* url = NULL;

		switch( iFeature )
			{
				case EFeaturedClips:
					{
					SetSearchNaviPaneId( R_SEARCH_FEATURED_TXT );
					url = PluginManager().Plugin()->FeatureUrlL( (TFeature)iFeature, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy, EPeriodIrrelevant );
					}
				break;

				case ETopRatedClips:
					{
					SetSearchNaviPaneId( R_SEARCH_TOPRATED_TXT );
					url = PluginManager().Plugin()->FeatureUrlL( (TFeature)iFeature, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy, iTimeFrame );
					}
				break;

				case EMostViewedClips:
					{
					SetSearchNaviPaneId( R_SEARCH_MOSTWATCHED_TXT );
					url = PluginManager().Plugin()->FeatureUrlL( (TFeature)iFeature, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy, iTimeFrame );
					}
				break;

				case ENewClips:
					{
					SetSearchNaviPaneId( R_SEARCH_NEWCLIPS_TXT );
					url = PluginManager().Plugin()->FeatureUrlL( (TFeature)iFeature, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy, EPeriodIrrelevant );
					}
				break;

				case EUserClips:
					{
					SetSearchNaviPaneId( R_SEARCH_USERVIDEOS_TXT );
					url = PluginManager().Plugin()->UserVideosUrlL( iSearchStringBase, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy );
					}
				break;

				case ERelatedClips:
					{
					SetSearchNaviPaneId( R_SEARCH_RELATED_TXT );
					url = PluginManager().Plugin()->RelatedVideosUrlL( iSearchStringBase, iSearchMaxResults, searchStartIndex, (TOrderBy)iOrderBy );
					}
				break;

			}

		iSearchString.Copy( *url );
		delete url;

//		CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
		iHttpEngine->SearchL( iSearchString, *this );
		}
	}

void CEmTubeAppUi::StartSearchL( TBool aByUrl )
	{
	iSearchByUrl = aByUrl;
	
	iNewSearch = ETrue;

	ClearSearchResults();	
	PluginManager().ResetPluginStates();
	

	iHttpEngine->SetClientObserver( NULL );

	CancelDownloadImageL();
//	iHttpEngine->CancelOperationL();
//	iImageLoader->Cancel();

	iSearchDisplayMode = ESearchDisplaySearchEntries;
	HandleCommandL( EMTVActivateSearchViewCommand );

	CEmTubeSearchView* sv = static_cast<CEmTubeSearchView*>( View( TUid::Uid(EMTVSearchViewId) ) );
	sv->ResetCurrentItems();
	
	if( !aByUrl )
		{
		iFeature = -1;
		SearchL();
		}
	else
		{
		iHttpEngine->SearchL( iSearchString, *this );
		}
	}

void CEmTubeAppUi::OpenFileL( const TDesC& aFilename, const TDesC& aTitle )
	{
	CEmTubePlayView* pv = static_cast<CEmTubePlayView*>( View( TUid::Uid(EMTVPlayViewId) ) );

	iPlaylistManager->UpdateStatsL( aFilename, aTitle, CEmTubePlaylistEntry::EPlaylistEntryLocal );

	if( iCurrentViewId == EMTVPlayViewId )
		{
		pv->OpenFileL( aFilename );
		}
	else
		{
		pv->SetFileNameL( aFilename );
		HandleCommandL( EMTVActivatePlayViewCommand );
		}
	}

void CEmTubeAppUi::OpenFileL( RFile& aFile )
	{
	CEmTubePlayView* pv = static_cast<CEmTubePlayView*>( View( TUid::Uid(EMTVPlayViewId) ) );

	if( iCurrentViewId == EMTVPlayViewId )
		{
		pv->OpenFileL( aFile );
		}
	else
		{
		pv->SetFileHandleL( aFile );
		HandleCommandL( EMTVActivatePlayViewCommand );
		}
	}

void CEmTubeAppUi::OpenFileL( CVideoEntry *aEntry )
	{
	CEmTubePlayView* pv = static_cast<CEmTubePlayView*>( View( TUid::Uid(EMTVPlayViewId) ) );

	iPlaylistManager->UpdateStatsL( aEntry->Url(), aEntry->MediaTitle(), CEmTubePlaylistEntry::EPlaylistEntryRemote );

	if( iCurrentViewId == EMTVPlayViewId )
		{
		pv->OpenFileL( aEntry );
		}
	else
		{
		pv->SetVideoEntry( aEntry );
		HandleCommandL( EMTVActivatePlayViewCommand );
		}
	}

void CEmTubeAppUi::OpenFileL( CEmTubePlaylistEntry *aEntry )
	{
	iPlaylistManager->UpdateStatsL( aEntry->Location(), aEntry->Name(), aEntry->Type() );

	if( aEntry->Type() == CEmTubePlaylistEntry::EPlaylistEntryLocal )
		{
		OpenFileL( aEntry->Location(), aEntry->Name() );
		}
	else
		{
		delete iPlaylistVideoEntry;
		iPlaylistVideoEntry = CVideoEntry::NewL();
		iPlaylistVideoEntry->SetUrlL( aEntry->Location() );
		iPlaylistVideoEntry->SetTitleL( aEntry->Name() );

		OpenFileL( iPlaylistVideoEntry );
		}
	}

void CEmTubeAppUi::SetTempMemory( TBool aTempMemory )
	{
	iTempMemory = aTempMemory;
	if( iTmpVideoFile )
		BaflUtils::DeleteFile( iSession, *iTmpVideoFile );

	InitTmpVideoFileNameL();
	}

void CEmTubeAppUi::InitTmpVideoFileNameL()
	{
	delete iTmpVideoFile;

	RBuf path;
	CleanupClosePushL( path );

	if( iTempMemory )
		{
		path.Create( KVideoDirectoryE().Length() + KVideoTmpName().Length() );
		path.Copy( KVideoDirectoryE );
		}
	else
		{
		path.Create( KVideoDirectoryC().Length() + KVideoTmpName().Length() );
		path.Copy( KVideoDirectoryC );
		}
	path.Append( KVideoTmpName );

	iTmpVideoFile = path.AllocL();

	CleanupStack::PopAndDestroy( &path );
	}

TInt CEmTubeAppUi::ConfirmationQueryL( TInt aResourceId )
	{
	TInt retCode;

	HBufC* txt = StringLoader::LoadLC( aResourceId );

	CAknQueryDialog* dlg = CAknQueryDialog::NewL();
	dlg->SetPromptL( *txt );
	retCode = dlg->ExecuteLD( R_YES_NO_QUERY );

	CleanupStack::PopAndDestroy( txt );

	return retCode;
	}

void CEmTubeAppUi::ShowErrorNoteL( const TDesC& aText )
	{
	CAknQueryDialog* dlg = CAknQueryDialog::NewL();
	dlg->SetPromptL( aText );
	dlg->ExecuteLD( R_ERROR_DIALOG );
	}

void CEmTubeAppUi::ShowErrorNoteL( TInt aResourceId )
	{
	HBufC* txt = StringLoader::LoadLC( aResourceId );
	ShowErrorNoteL( *txt );
	CleanupStack::PopAndDestroy( txt );
	}

_LIT( KFavsFilename, "favs.bin" );

void CEmTubeAppUi::LoadFavoritesL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KFavsFilename );

	TInt err = file.Open( iSession, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TInt count = 0;
		count = stream.ReadInt32L();
		for( TInt i=0; i<count; i++ )
			{
			CVideoEntry* e = CVideoEntry::NewLC();
			e->ImportL( stream );
			iFavsEntries.Append( e );
			CleanupStack::Pop( e );
			}

		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

void CEmTubeAppUi::SaveFavoritesL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KFavsFilename );

	TInt err = file.Replace( iSession, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		TInt count = iFavsEntries.Count();
		stream.WriteInt32L( count );

		for( TInt i=0; i<count; i++ )
			{
			iFavsEntries[i]->ExportL( stream );
			}

		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

TInt CEmTubeAppUi::FindInFavoritesL( CVideoEntry* aEntry )
	{
	for( TInt i=0; i<iFavsEntries.Count(); i++ )
		{
		CVideoEntry* f = iFavsEntries[i];
		if( !aEntry->Url().Compare( f->Url() ) )
			{
			return i;
			}
		}
	return KErrNotFound;
	}

void CEmTubeAppUi::AddToFavoritesL( CVideoEntry* aEntry )
	{
	if( FindInFavoritesL( aEntry ) == KErrNotFound )
		{
		CVideoEntry* e = CVideoEntry::NewL( aEntry );
		iFavsEntries.Append( e );

		SaveFavoritesL();
		}
	}

void CEmTubeAppUi::DeleteFromFavoritesL( CVideoEntry* aEntry )
	{
	TInt idx = FindInFavoritesL( aEntry );

	if( idx != KErrNotFound )
		{
		CVideoEntry* e = iFavsEntries[ idx ];
		iFavsEntries.Remove( idx );
		delete e;

		SaveFavoritesL();
		}
	}

//saved clips
_LIT( KSavedFilename, "saved.bin" );

void CEmTubeAppUi::LoadSavedClipsL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSavedFilename );

	TInt err = file.Open( iSession, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TFileName f;
		TInt length;

		TInt count = 0;
		count = stream.ReadInt32L();

		if( count == ( ('s' << 24) + ('c' << 16) + ('1' << 8) + ('0') ) )
			{
			//new format
			TInt count = stream.ReadInt32L();
			for( TInt i=0; i<count; i++ )
				{
				CVideoEntry* e = CVideoEntry::NewLC();
				e->ImportL( stream );

				length = stream.ReadInt32L();
				stream.ReadL( f, length );
				e->SetSavedFileName( f );

				length = stream.ReadInt32L();
				if( length )
					{
					HBufC* tmpb = HBufC::NewLC( length );
					TPtr tmp( tmpb->Des() );
					stream.ReadL( tmp, length );
					e->SetCategoryL( *tmpb );
					CleanupStack::PopAndDestroy( tmpb );
					}

				if( BaflUtils::FileExists( iSession, e->SavedFileName() ) )
					{
					TInt size = 0;
					RFile file;
					TInt err = file.Open( iSession, e->SavedFileName(), EFileStream | EFileRead | EFileShareAny );
					if( err == KErrNone )
						{
						file.Size( size );
						file.Close();
						}
					e->SetVideoFileSize( size );

					iSavedEntries.Append( e );
					CleanupStack::Pop( e );
					}
				else
					{
					BaflUtils::DeleteFile( iSession, e->ThumbnailFile() );
					CleanupStack::PopAndDestroy( e );
					}
				}
			}
		else
			{
			//old format
			for( TInt i=0; i<count; i++ )
				{
				CVideoEntry* e = CVideoEntry::NewLC();
				e->ImportL( stream );

				length = stream.ReadInt32L();
				stream.ReadL( f, length );
				e->SetSavedFileName( f );

				if( BaflUtils::FileExists( iSession, e->SavedFileName() ) )
					{
					TInt size = 0;
					RFile file;
					TInt err = file.Open( iSession, e->SavedFileName(), EFileStream | EFileRead | EFileShareAny );
					if( err == KErrNone )
						{
						file.Size( size );
						file.Close();
						}
					e->SetVideoFileSize( size );

					iSavedEntries.Append( e );
					CleanupStack::Pop( e );
					}
				else
					{
					BaflUtils::DeleteFile( iSession, e->ThumbnailFile() );
					CleanupStack::PopAndDestroy( e );
					}
				}
			}

		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

void CEmTubeAppUi::SaveSavedClipsL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSavedFilename );

	TInt err = file.Replace( iSession, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		TInt header = ('s' << 24) + ('c' << 16) + ('1' << 8) + ('0');
		stream.WriteInt32L( header );

		TInt count = iSavedEntries.Count();
		stream.WriteInt32L( count );

		for( TInt i=0; i<count; i++ )
			{
			iSavedEntries[i]->ExportL( stream );
			TInt length = iSavedEntries[i]->SavedFileName().Length();
			stream.WriteInt32L( length );
			if( length )
				stream.WriteL( iSavedEntries[i]->SavedFileName() );

			length = iSavedEntries[i]->Category().Length();
			stream.WriteInt32L( length );
			if( length )
				stream.WriteL( iSavedEntries[i]->Category() );
			}

		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

TInt CEmTubeAppUi::FindInSavedClipsL( CVideoEntry* aEntry )
	{
	for( TInt i=0; i<iSavedEntries.Count(); i++ )
		{
		CVideoEntry* f = iSavedEntries[i];
		if( !aEntry->SavedFileName().Compare( f->SavedFileName() ) )
			{
			return i;
			}
		}
	return KErrNotFound;
	}

void CEmTubeAppUi::AddToSavedClipsL( CQueueEntry* aEntry )
	{
	CVideoEntry* entry = CVideoEntry::NewLC();
	entry->SetSavedFileName( aEntry->Filename() );
	entry->SetVideoFileSize( aEntry->Size() );
	entry->SetDownloadFinished( ETrue );
	entry->SetTitleL( aEntry->Title() );
	entry->SetAuthorNameL( KNullDesC() );

	TFileName img;
	TParsePtrC parse( entry->SavedFileName() );
	img.Copy( iFullPrivatePath );
	img.Append( parse.Name() );
	img.Append( KCacheFileExtension() );
	entry->SetThumbnailFileL( img );
	entry->SetThumbnailUrlL( KNullDesC() );

	CThumbnailRetriever* tr = CThumbnailRetriever::NewLC();
	TRAPD( err, tr->RetrieveThumbnailL( entry ) );
	CleanupStack::PopAndDestroy( tr );

	if( err == KErrNone )
		{
		iPlaylistManager->UpdateSavedClipsStatsL( entry->SavedFileName(), entry->MediaTitle() );
		iSavedEntries.Append( entry );
		CleanupStack::Pop( entry );
		}
	else
		{
//TODO -> show error.
		CleanupStack::PopAndDestroy( entry );
		}
	}

void CEmTubeAppUi::AddToSavedClipsL( CVideoEntry* aEntry )
	{
	if( FindInSavedClipsL( aEntry ) == KErrNotFound )
		{
		CVideoEntry* e = CVideoEntry::NewLC( aEntry );
		TFileName img;
		TParsePtrC parse( e->SavedFileName() );
		img.Copy( iFullPrivatePath );
		img.Append( parse.Name() );
		img.Append( KCacheFileExtension() );
		e->SetThumbnailFileL( img );
		e->SetThumbnailUrlL( KNullDesC() );

		if( BaflUtils::FileExists( iSession, aEntry->ThumbnailFile()) )
			{
		    BaflUtils::CopyFile( iSession, aEntry->ThumbnailFile(), img );
			}
		else
	    	{
			CThumbnailRetriever* tr = CThumbnailRetriever::NewLC();
			TRAP_IGNORE( tr->RetrieveThumbnailL( e ) );
    		CleanupStack::PopAndDestroy( tr );
	    	}

		iPlaylistManager->UpdateSavedClipsStatsL( e->SavedFileName(), e->MediaTitle() );
		iSavedEntries.Append( e );
   		CleanupStack::Pop( e );

		SaveSavedClipsL();
		}
	}

void CEmTubeAppUi::DeleteFromSavedClipsL( CVideoEntry* aEntry )
	{
	TInt idx = FindInSavedClipsL( aEntry );

	if( idx != KErrNotFound )
		{
		CVideoEntry* e = iSavedEntries[ idx ];

		BaflUtils::DeleteFile( iSession, aEntry->ThumbnailFile() );
		BaflUtils::DeleteFile( iSession, aEntry->SavedFileName() );

		iSavedEntries.Remove( idx );
		delete e;

		SaveSavedClipsL();
		}
	}

void CEmTubeAppUi::RenameSavedClipL( CVideoEntry* aEntry, const TDesC& aName )
	{
	TParsePtrC parse( aEntry->SavedFileName() );
	TParsePtrC parse2( aName );

	TFileName m;
	m.Copy( parse.DriveAndPath() );
	m.Append( parse2.Name() );
	m.Append( KCacheFileExtension() );
	BaflUtils::RenameFile( iSession, aEntry->ThumbnailFile(), m );
	aEntry->SetThumbnailFileL( m );

	m.Copy( parse.DriveAndPath() );
	m.Append( parse2.Name() );
	m.Append( _L(".flv") );
	BaflUtils::RenameFile( iSession, aEntry->SavedFileName(), m );
	aEntry->SetSavedFileName( m );

	aEntry->SetTitleL( parse2.Name() );

	SaveSavedClipsL();
	}

_LIT( KSettingsFilename, "emtsettings.bin" );
void CEmTubeAppUi::LoadSettingsL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSettingsFilename );

	TInt err = file.Open( iSession, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		iSearchMaxResults = stream.ReadInt32L();
		iMaxCacheSize = stream.ReadInt32L();
		iVolume = stream.ReadInt32L();
		iStartPlaybackMode = (TStartPlaybackMode)stream.ReadInt32L();

		iAutoRotate = stream.ReadInt32L();
		iTempMemory = stream.ReadInt32L();

		int length = 0;
		length = stream.ReadInt32L();
		if( length )
			{
			stream.ReadL( iSaveLoadDirectory, length );
			}

		iOrderBy = (TOrderBy)stream.ReadInt32L();

		iAccessPoint = stream.ReadInt32L();
		iVideoScaleMode = (TVideoScaleMode)stream.ReadInt32L();
		iPluginUid = stream.ReadUint32L();
        iS60Ui = stream.ReadUint32L();

		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

void CEmTubeAppUi::SaveSettingsL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSettingsFilename );

	TInt err = file.Replace( iSession, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		stream.WriteInt32L( iSearchMaxResults );
		stream.WriteInt32L( iMaxCacheSize );
		stream.WriteInt32L( iVolume );
		stream.WriteInt32L( (TInt)iStartPlaybackMode );
		stream.WriteInt32L( (TInt)iAutoRotate );
		stream.WriteInt32L( (TInt)iTempMemory );

		if( iSaveLoadDirectory.Length() )
			{
			stream.WriteInt32L( iSaveLoadDirectory.Length() );
			stream.WriteL( iSaveLoadDirectory );
			}
		else
			{
			stream.WriteInt32L( 0 );
			}

		stream.WriteInt32L( (TInt)iOrderBy );

		stream.WriteInt32L( iAccessPoint );
		stream.WriteInt32L( (TInt) iVideoScaleMode );
		stream.WriteUint32L( PluginManager().DefaultPluginUid() );
		stream.WriteUint32L( (TInt) iS60Ui );

		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

//search history

_LIT( KSearchHistoryFilename, "searchhistory.bin" );

void CEmTubeAppUi::LoadSearchHistoryL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSearchHistoryFilename );

	TInt err = file.Open( iSession, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TFileName f;
		TInt length;

		TInt count = 0;
		count = stream.ReadInt32L();

		for( TInt i=0; i<count; i++ )
			{
			HBufC* str;
			length = stream.ReadInt32L();

			str = HBufC::NewLC( length );

			TPtr tmp( str->Des() );
			stream.ReadL( tmp, length );
			iSearchHistory.Append( str );
			CleanupStack::Pop( str );
			}
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

void CEmTubeAppUi::SaveSearchHistoryL()
	{
	TFileName fileName;
	RFile file;

	fileName.Copy( iFullPrivatePath );
	fileName.Append( KSearchHistoryFilename );

	TInt err = file.Replace( iSession, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		TInt count = iSearchHistory.Count();
		stream.WriteInt32L( count );

		for( TInt i=0; i<count; i++ )
			{
			TInt length = iSearchHistory[i]->Length();
			stream.WriteInt32L( length );
			if( length )
				stream.WriteL( *(iSearchHistory[i]) );
			}

		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	}

void CEmTubeAppUi::AddToSearchHistoryL( const TDesC& aString )
	{

	for(TInt i=0;i<iSearchHistory.Count();i++)
		{
		if( !aString.Compare( *(iSearchHistory[i]) ) )
			return; //already in search history
		}

	iSearchHistory.Insert( aString.AllocL(), 0 );

	while( iSearchHistory.Count() > MAX_SEARCH_HISTORY_ENTRIES )
		{
		TInt c = iSearchHistory.Count() - 1;
		HBufC* t = iSearchHistory[ c ];
		iSearchHistory.Remove( c );
		delete t;
		}
	}

TBool CEmTubeAppUi::CheckDiskSpaceL( const TDesC& aFileName, TInt aSize )
	{
	TParsePtrC parse( aFileName );
	TPtrC drive = parse.Drive();

	TVolumeInfo info;
	if( !drive.Compare( _L("C:") ) )
		{
		iSession.Volume( info ,EDriveC );
		}
	else if( !drive.Compare( _L("D:") ) )
		{
		iSession.Volume( info, EDriveD );
		}
	else if( !drive.Compare( _L("E:") ) )
		{
		iSession.Volume( info, EDriveE );
		}
	else
		{
		return EFalse;
		}

Log( info.iFree );
Log( "" );

	if( info.iFree > aSize )
		{
		return ETrue;
		}

	return EFalse;
	}

void CEmTubeAppUi::SetSaveLoadDirectory( TDesC& aDir )
	{
	iSaveLoadDirectory.Copy( aDir );
	SaveSettingsL();
	}

HBufC* CEmTubeAppUi::ConvertFileNameL( TDesC& aFileName )
	{
	RBuf name;
	CleanupClosePushL( name );

	for(TInt i=0;i<aFileName.Length();i++ )
		{
		switch( aFileName[i] )
			{
			case '<':
			case '>':
			case '\\':
			case '/':
			case '"':
			case '|':
			case ':':
			case ';':
			case '*':
			case '!':
			case '?':
				break;

			default:
				name.ReAllocL( name.Length() + 1 );
				name.Append( aFileName[i] );
			break;
			}
		}

	HBufC* tmp = name.AllocL();
	CleanupStack::PopAndDestroy( &name );
	return tmp;
	}

TBool CEmTubeAppUi::ProcessCommandParametersL( CApaCommandLine& /*aCommandLine*/ )
	{
//Log("->ProcessCommandParametersL");
//Log( aCommandLine.DocumentName() );
//Log("<-ProcessCommandParametersL");

/*	TApaTaskList taskList( CEikonEnv::Static()->WsSession() );

	if( aCommandLine.DocumentName().Length() )
		{
		SetEmbedded( ETrue );
		OpenFileL( aCommandLine.DocumentName() );
		}

Log("ProcessCommandParameters");
Log( aCommandLine.OpaqueData() );
Log( aCommandLine.DocumentName() );
*/
	return EFalse;
	}

CEmTubeAppUi::TMemoryChoice CEmTubeAppUi::SelectDriveL( )
	{
	CAknSingleGraphicPopupMenuStyleListBox* plist = new(ELeave) CAknSingleGraphicPopupMenuStyleListBox;
	CleanupStack::PushL(plist);

	CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_SELECT_CANCEL, AknPopupLayouts::EMenuWindow);
	CleanupStack::PushL(popupList);

	plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
	plist->CreateScrollBarFrameL(ETrue);
	plist->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

	MDesCArray* itemList = plist->Model()->ItemTextArray();
	CDesCArray* items = (CDesCArray*) itemList;

	CArrayPtr<CGulIcon>* icons = new(ELeave) CAknIconArray(4);
	CleanupStack::PushL(icons);

	CFbsBitmap *bitmap, *mask;

	AknIconUtils::CreateIconL( bitmap, mask, KSystemIconFile, EMbmAvkonQgn_prop_phone_memc_large, EMbmAvkonQgn_prop_phone_memc_large_mask );
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );

	AknIconUtils::CreateIconL( bitmap, mask, KSystemIconFile, EMbmAvkonQgn_prop_mmc_memc_large, EMbmAvkonQgn_prop_mmc_memc_large_mask );
	icons->AppendL( CGulIcon::NewL( bitmap, mask ) );

	CleanupStack::Pop( icons );

	plist->ItemDrawer()->ColumnData()->SetIconArray( icons );

	TInt mmcPresent = (TInt) MMCAvailable();

	_LIT( KListFormat, "%d\t%S");
	RBuf text;
	text.Create(100);

	HBufC* phoneMemory = StringLoader::LoadLC( R_DISKC_TXT );
	HBufC* memoryCard = StringLoader::LoadLC( R_DISKE_TXT );
	HBufC* inbox = StringLoader::LoadLC( R_INBOX_TXT );

	text.Format( KListFormat, 0, phoneMemory);
	items->AppendL( text );

	if( mmcPresent )
		{
		text.Format( KListFormat, 1, memoryCard );
		items->AppendL( text );
		}

	if( iInboxFlvFinder->FindFlvL() )
		{
		text.Format( KListFormat, 1, inbox );
		items->AppendL( text );
		}

	plist->SetCurrentItemIndex( 0 );

	popupList->SetTitleL( _L("Select Memory:") );

	TInt popupOk = popupList->ExecuteLD();
	CEmTubeAppUi::TMemoryChoice result;

	CleanupStack::PopAndDestroy( inbox );
	CleanupStack::PopAndDestroy( memoryCard );
	CleanupStack::PopAndDestroy( phoneMemory );

	text.Close();

	result = EMemoryNone;

	if(popupOk)
		{
		switch( plist->CurrentItemIndex() )
			{
			case 0:
				{
				result = EMemoryPhone;
				}
			break;

			case 1:
				{
				if(mmcPresent) result = EMemorySd;
				else result = EMemoryInbox;
				}
			break;

			case 2:
				{
				result = EMemoryInbox;
				}
			break;

			}
		}

	CleanupStack::Pop( ); //popupList
	CleanupStack::PopAndDestroy( );	//list

	return result;
}

MVideoEntry* CEmTubeAppUi::AddVideoEntryL()
	{
	CVideoEntry* ventry = CVideoEntry::NewL();
	ventry->SetPluginUid( PluginManager().Uid() );
	iTmpSearchEntries.Append( ventry );
	return ventry;
	}

void CEmTubeAppUi::StartDownloadByTML( )
    {
    if(iStartDownloadByTM)
        return;

    if( !iTransferManager->AllProcessed() )
        {
    	if( ConfirmationQueryL( R_START_DOWNLOAD_TXT ) )
    	    iTransferManager->StartL( ETrue );
        }

    iStartDownloadByTM = ETrue;
    }

void CEmTubeAppUi::ChangeScreenLayoutL( TBool aForceS60Ui )
    {
	MEikAppUiFactory* f = CEikonEnv::Static()->AppUiFactory();
	CEikButtonGroupContainer* toolBar;
	toolBar = f->ToolBar();

    CEikStatusPane* statusPane = StatusPane();
    if( S60Ui() || aForceS60Ui )
        {
		toolBar->MakeVisible( ETrue );
		if (statusPane->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_USUAL)
            statusPane->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
        }
    else
        {
		toolBar->MakeVisible( ETrue );
		if (statusPane->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL_WITH_SIGNAL_PANE)
            statusPane->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_SMALL_WITH_SIGNAL_PANE );
        }
    statusPane->DrawNow();
    }


TBool CEmTubeAppUi::GotFeature()
	{
	TBool result = EFalse;
	TInt caps = PluginManager().Plugin()->Capabilities();
	
	switch( iFeature )
		{
		case -1: // no Feature
			result = ETrue;
		break;

		case ETopRatedClips:
			result = ( caps & KCapsTopRated );
		break;

		case ENewClips:
			result = ( caps & KCapsRecentlyUploaded );
		break;

		case EMostViewedClips:
			result = ( caps & KCapsMostViewed );
		break;

		case EFeaturedClips:
			result = ( caps & KCapsFeatured );
		break;

		case EUserClips:
			if( PluginManager().TemporaryPluginUid() == PluginManager().Uid() )
				result = ( caps & KCapsUserClips );
			else
				result = EFalse;
		break;

		case ERelatedClips:
			if( PluginManager().TemporaryPluginUid() == PluginManager().Uid() )
				result = ( caps & KCapsRelatedClips );
			else
				result = EFalse;
		break;

		default:
			break;			
		}
	
	return result;
	}

TInt CEmTubeAppUi::SearchStartIndex()
	{
	if( iNewSearch )
		return 1;
	
	TInt searchStartIndex = 1;

	for( TInt i=0 ; i<iSearchEntries.Count() ; i++ )
		if( iSearchEntries[i]->PluginUid() == PluginManager().Uid() )
			searchStartIndex++;			
	
	return searchStartIndex;		
	}

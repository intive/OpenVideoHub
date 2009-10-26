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

#include <aknviewappui.h>
#include <akntitle.h>
#include <stringloader.h>
#include <aknmessagequerydialog.h>
#include <sendui.h>
#include <cmessagedata.h>
#include <senduiconsts.h>
#include <PluginInterface.h>

#ifdef __S60_50__
#include <AknToolbar.h>
#endif

#include "emTubeResource.h"

#include "emTube.hrh"

#include "emTubeSearchView.h"
#include "emTubeAppUi.h"
#include "emTubeSearchViewContainer.h"
#include "emTubeDetailsDialog.h"
#include "emTubeVideoEntry.h"
#include "emTubePlaylistManager.h"

CEmTubeSearchView* CEmTubeSearchView::NewL( CEmTubePlaylistManager* aManager )
	{
	CEmTubeSearchView* self = CEmTubeSearchView::NewLC( aManager );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeSearchView* CEmTubeSearchView::NewLC( CEmTubePlaylistManager* aManager )
	{
	CEmTubeSearchView* self = new (ELeave) CEmTubeSearchView( aManager );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeSearchView::~CEmTubeSearchView()
	{
	delete iFindBoxText;

	if(iContainer)
		{
		delete iContainer;
		iContainer = NULL;
		}
	ResetCurrentItems();
	iCurrentItems.Close();
	}

CEmTubeSearchView::CEmTubeSearchView( CEmTubePlaylistManager* aManager ) : iManager( aManager )
	{
	}

void CEmTubeSearchView::ConstructL()
	{
	BaseConstructL( R_EMTV_SEARCH_VIEW );
	iFindBoxText = KNullDesC().AllocL();
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());
	}

TUid CEmTubeSearchView::Id() const
	{
	return TUid::Uid( EMTVSearchViewId );
	}

void CEmTubeSearchView::HandleCommandL( TInt aCommand )
	{
	TInt which = CurrentToActual();

	switch(aCommand)
		{
		case EMTVSendViaBluetoothCommand:
			{
			CSendUi* sendUi = CSendUi::NewLC();
			CMessageData* mdata = CMessageData::NewLC();
			mdata->AppendAttachmentL( iAppUi->SearchEntries()[ which ]->SavedFileName() );
			sendUi->CreateAndSendMessageL( KSenduiMtmBtUid, mdata );
			CleanupStack::PopAndDestroy( mdata );
			CleanupStack::PopAndDestroy( sendUi );
			}
		break;

		case EMTVSearchViewDownloadCommand:
			{
//			iAppUi->CancelDownloadImageL();
			iContainer->DownloadL( );
			}
		break;

		case EMTVAddToPlaylistCommand:
			{
			RArray<TInt> indices;
			CleanupClosePushL( indices );
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

		    popupList->SetTitleL( KNullDesC() );

		    HBufC* txt = StringLoader::LoadLC( R_PLAYLIST_CREATE_NEW_TXT );
		    indices.Append( -1 );
		    items->AppendL( *txt );
		    CleanupStack::PopAndDestroy( txt );

		    TInt count = iManager->PlaylistsCount();
			for(TInt i=0;i<count;i++)
				{
				CEmTubePlaylist* pl = iManager->Playlist( i );
				if( pl->Editable() )
					{
					indices.Append( i );
			    	items->AppendL( pl->Name() );
					}
			    }

		    TInt popupOk = popupList->ExecuteLD();
		    if(popupOk)
		        {
		        TInt idx = indices[ plist->CurrentItemIndex() ];
				if( idx == -1 )
		        	{
					TBuf<64> name;
					CAknTextQueryDialog* dlg = new(ELeave)CAknTextQueryDialog( name, CAknQueryDialog::ENoTone );
					dlg->SetPredictiveTextInputPermitted( ETrue );
					TInt ret = dlg->ExecuteLD( R_EMTV_ADD_PLAYLIST_DIALOG );
					if( ret )
						{
						iManager->AddPlaylistL( name, CEmTubePlaylist::EPlaylistUserDefined );
						idx = iManager->PlaylistsCount() - 1;
						}
		        	}

				if( idx >= 0 )
					{
					CEmTubePlaylist* pl = iManager->Playlist( idx );
					CVideoEntry* e = iAppUi->SearchEntries()[ which ];
					if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries ||
							iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries )
						pl->AddEntryL( e->Url(), e->MediaTitle(), CEmTubePlaylistEntry::EPlaylistEntryRemote );
					else
						pl->AddEntryL( e->SavedFileName(), e->MediaTitle(), CEmTubePlaylistEntry::EPlaylistEntryLocal );
					}
		        }
		    CleanupStack::Pop( popupList );
		    CleanupStack::PopAndDestroy( plist );
		    CleanupStack::PopAndDestroy( &indices );
			}
		break;

		case EMTVSearchViewAddToFCommand:
			{
			iContainer->AddCurrentItemToFavsL();
			}
		break;

		case EMTVSearchViewRemoveFromFCommand:
			{
			iContainer->RemoveCurrentItemFromFavsL();
			}
		break;

		case EMTVSearchCommand:
			{
			if( iAppUi->SearchDialogL() )
				{
// TODO: when displaying featured (for example), new search (from menu) - change navi pane text
//				if(no tabs, one plugin)
//					iContainer->SetNaviPaneTextL();

				StartProgressBarL( EFalse );
				}
			}
		break;

		case EMTVSearchViewPlayCommand:
			{
			if( which != KErrNotFound )
				{
				iAppUi->CancelDownloadImageL();
				if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
					iAppUi->OpenFileL( iAppUi->SearchEntries()[ which ]->SavedFileName(), iAppUi->SearchEntries()[ which ]->MediaTitle() );
				else
					iAppUi->OpenFileL( iAppUi->SearchEntries()[ which ] );
				}
			}
		break;

		case EMTVDeleteClipCommand:
			{
			iContainer->DeleteSavedClipL();
			}
		break;

		case EMTVRenameClipCommand:
			{
			iContainer->RenameSavedClipL();
			}
		break;

		case EMTVShowRelatedCommand:
			{
			if( which != KErrNotFound )
				{
				iAppUi->CancelDownloadImageL();
				CVideoEntry* e = iAppUi->SearchEntries()[ which ];
				iAppUi->SearchString().Copy( e->RelatedUrl() );
				StartProgressBarL( EFalse );
				iAppUi->ClearSearchResults();
				iAppUi->SearchL( ERelatedClips, ERelevance, EPeriodIrrelevant );
				}
			}
		break;

		case EMTVShowUserCommand:
			{
			if( which != KErrNotFound )
				{
				iAppUi->CancelDownloadImageL();
				CVideoEntry* e = iAppUi->SearchEntries()[ which ];
				iAppUi->SearchString().Copy( e->AuthorVideosUrl() );
				StartProgressBarL( EFalse );
				iAppUi->ClearSearchResults();
				iAppUi->SearchL( EUserClips, EUpdated, EPeriodIrrelevant );
				}
			}
		break;

		case EMTVSearchViewDetailsCommand:
			{
			iContainer->ShowToolbar( EFalse );
			CEmTubeDetailsDialog::RunDialogL( iAppUi->SearchEntries()[ which ] );
			iContainer->ShowToolbar( ETrue );
			}
		break;

		case EMTVSearchViewBackCommand:
			{
			iContainer->ClearToolbar();
			iAppUi->CancelDownloadImageL();
			iAppUi->HandleCommandL( EMTVActivateMainViewCommand );
			}
		break;

		case EMTVProgressDialogCancel:
			{
			iAppUi->CancelDownloadImageL();
			if( iAppUi->SearchEntries().Count() )
				iContainer->CancelProgressL();
			else
				iAppUi->HandleCommandL( EMTVActivateMainViewCommand );
			}
		break;

		default:
			{
			iAppUi->HandleCommandL(aCommand);
			}
		break;
		}
	}


void CEmTubeSearchView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
										TUid /*aCustomMessageId*/,
										const TDesC8& /*aCustomMessage*/)
	{
	iAppUi->ChangeScreenLayoutL( EFalse );

#ifdef __S60_50__
    iAppUi->CurrentFixedToolbar()->SetToolbarObserver( this );
#endif

    iContainer = CEmTubeSearchViewContainer::NewL( *this, ClientRect(), *iFindBoxText );
	iAppUi->AddToStackL(iContainer);
	}


void CEmTubeSearchView::DoDeactivate()
	{
	HBufC* t = iContainer->FindBoxText();
	if( t )
		{
		delete iFindBoxText;
		iFindBoxText = t->AllocL();
		}
	
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;
	}

void CEmTubeSearchView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if ( aResourceId == R_EMTV_SEARCH_SORTBY_MENU_PANE )
        {
		if( iAppUi->SortResultsBy() == EViewCount )
			aMenuPane->SetItemButtonState( EMTVSortByViewCountCommand, EEikMenuItemSymbolOn );
		else if( iAppUi->SortResultsBy() == EUpdated )
			aMenuPane->SetItemButtonState( EMTVSortByDateAddedCommand, EEikMenuItemSymbolOn );
		else if( iAppUi->SortResultsBy() == ERating )
			aMenuPane->SetItemButtonState( EMTVSortByRatingCommand, EEikMenuItemSymbolOn );
		else if( iAppUi->SortResultsBy() == ERelevance )
			aMenuPane->SetItemButtonState( EMTVSortByRelevanceCommand, EEikMenuItemSymbolOn );
        }

	if( aResourceId == R_EMTV_SEARCH_VIEW_MENU_PANE )
		{
		if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
			{
			aMenuPane->SetItemDimmed(EMTVSendViaBluetoothCommand, ETrue );

			aMenuPane->SetItemDimmed(EMTVScanDirectoriesCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVDeleteClipCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVRenameClipCommand, ETrue );

			aMenuPane->SetItemDimmed(EMTVSearchCommand, EFalse );

//			if( iAppUi->Feature() == -1 )
//				aMenuPane->SetItemDimmed(EMTVSearchCommand, EFalse );
//			else
//				aMenuPane->SetItemDimmed(EMTVSearchCommand, ETrue );

			if( iContainer->CurrentItemIndex() == -1 || !iAppUi->GotFeature() )
				{
				aMenuPane->SetItemDimmed(EMTVSearchViewPlayCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVSortByCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, ETrue );
				}
			else
				{					
				aMenuPane->SetItemDimmed(EMTVSortByCommand, EFalse );

				if( iContainer->IsListboxLineMore() )
					{
					aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
					aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
					aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, ETrue );
					aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, ETrue );
					}
				else
					{
					TInt idx = CurrentToActual();
					CVideoEntry* e = iAppUi->SearchEntries()[ idx ];

					if( (iAppUi->PluginManager().Plugin()->Capabilities() & KCapsUserClips) && e->AuthorVideosUrl().Length() )
						{
						aMenuPane->SetItemDimmed(EMTVShowUserCommand, EFalse );
						}
					else
						{
						aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
						}

					if( (iAppUi->PluginManager().Plugin()->Capabilities() & KCapsRelatedClips) && e->RelatedUrl().Length() )
						{
						aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, EFalse );
						}
					else
						{
						aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
						}

					if( iContainer->IsCurrentItemInFavsL( ) )
						{
						aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, ETrue );
						aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, EFalse );
						}
					else
						{
						aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, EFalse );
						aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, ETrue );
						}
					}
				}
			}
		else if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries )
			{
			aMenuPane->SetItemDimmed(EMTVSendViaBluetoothCommand, ETrue );

			aMenuPane->SetItemDimmed(EMTVSortByCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVScanDirectoriesCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVDeleteClipCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVRenameClipCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, ETrue );
			if( !iAppUi->SearchEntries().Count() )
				{
				aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, ETrue );
				}
			else
				{
				aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, EFalse );
				}
			}
		else if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
			{
			if( !iAppUi->SearchEntries().Count() )
				{
				aMenuPane->SetItemDimmed(EMTVSendViaBluetoothCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVDeleteClipCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVRenameClipCommand, ETrue );
				}
			else
				{
				aMenuPane->SetItemDimmed(EMTVSendViaBluetoothCommand, EFalse );
				aMenuPane->SetItemDimmed(EMTVDeleteClipCommand, EFalse );
				aMenuPane->SetItemDimmed(EMTVRenameClipCommand, EFalse );
				}

			aMenuPane->SetItemDimmed(EMTVScanDirectoriesCommand, EFalse );
			aMenuPane->SetItemDimmed(EMTVSortByCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchViewAddToFCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchViewRemoveFromFCommand, ETrue );
			}


		if( !iAppUi->SearchEntries().Count() || !iAppUi->GotFeature() )
			{
			aMenuPane->SetItemDimmed(EMTVSearchViewDownloadCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchViewPlayCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVSearchViewDetailsCommand, ETrue );
			}
		else
			{
			if( iContainer->IsListboxLineMore() )
				{
				aMenuPane->SetItemDimmed(EMTVSearchViewDownloadCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVSearchViewPlayCommand, ETrue );
				aMenuPane->SetItemDimmed(EMTVSearchViewDetailsCommand, ETrue );
				}
			else
				{
				if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
					{
					aMenuPane->SetItemDimmed(EMTVSearchViewDownloadCommand, ETrue );
					}
				else
					{
					if( iAppUi->PluginManager().Plugin()->Capabilities() & KCapsAllowSavingVideos )
						aMenuPane->SetItemDimmed(EMTVSearchViewDownloadCommand, EFalse );
					else
						aMenuPane->SetItemDimmed(EMTVSearchViewDownloadCommand, ETrue );
					}
				aMenuPane->SetItemDimmed(EMTVSearchViewPlayCommand, EFalse );
				aMenuPane->SetItemDimmed(EMTVSearchViewDetailsCommand, EFalse );
				}
			}
		}
	}

void CEmTubeSearchView::DisplaySearchResultsL()
	{
	if( iContainer )
		iContainer->DisplaySearchResultsL();
	}

void CEmTubeSearchView::ImageLoadedL( TInt aIndex )
	{
	if( iContainer )
		iContainer->ImageLoadedL( aIndex );
	}

void CEmTubeSearchView::SetCbaL( TInt aCbaResource )
	{
	Cba()->SetCommandSetL( aCbaResource );
	Cba()->DrawDeferred();
	}

void CEmTubeSearchView::CancelProgressL()
	{
	if( iContainer )
		iContainer->CancelProgressL();
	}

void CEmTubeSearchView::StartProgressBarL( TBool aClear )
	{
	if( iContainer )
		iContainer->StartProgressBarL( aClear );
	}

#ifdef __S60_50__

void CEmTubeSearchView::DynInitToolbarL( TInt aResourceId, CAknToolbar *aToolbar )
	{
	if(aResourceId == R_EMTV_SEARCH_VIEW_TOOLBAR)
		{
/*		*if(iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries)
			{
			aToolbar->HideItem( EMTVSearchViewTabToLeftCommand, ETrue, ETrue );
			aToolbar->HideItem( EMTVSearchViewTabToRightCommand, ETrue, ETrue );
			}

		aToolbar->DrawNow();
*/		}
	}

void CEmTubeSearchView::OfferToolbarEventL( TInt aCommand )
	{
	if( iContainer )
		{
		switch( aCommand )
			{
			case EMTVSearchViewPlayCommand:
				HandleCommandL( EMTVSearchViewPlayCommand );
			break;

			case EMTVSearchViewTabToLeftCommand:
				iContainer->HandleCommandL( MHandleCommandObserver::EHandleKeyLeft );
			break;

			case EMTVSearchViewTabToRightCommand:
				iContainer->HandleCommandL( MHandleCommandObserver::EHandleKeyRight );
			break;

			default:
				break;
			}
		}
	}

#endif

TInt CEmTubeSearchView::CurrentToActual()
	{
	TInt which = -1;

	if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
		{
		RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();
		for( TInt i=0 ; i < entries.Count() ; i++ )
			{
			if( entries[i]->PluginUid() == iAppUi->PluginManager().Uid() )
				which++;

			if( which == iContainer->CurrentItemIndex() )
				{
				which = i;
				break;
				}
			}
		}
	else
		{
		RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();
		for( TInt i=0 ; i < entries.Count() ; i++ )
			{
			if( iContainer->IsEntryVisibleL( entries[i] ) )
				which++;

			if( which == iContainer->CurrentItemIndex() )
				{
				which = i;
				break;
				}
			}
		}

	return which;
	}

void CEmTubeSearchView::ResetCurrentItems()
	{
	iCurrentItems.Reset();
	}

TInt CEmTubeSearchView::FindCurrentItem( TUint32 aUid )
	{
	for(TInt i=0;i<iCurrentItems.Count();i++)
		{
		if( iCurrentItems[i].iPluginUid == aUid )
			return i;
		}
	return KErrNotFound;
	}

TInt CEmTubeSearchView::CurrentItemIndex()
	{
	TUint32 uid = iAppUi->PluginManager().Uid();
	TInt idx = FindCurrentItem( uid );
	TCurrentItem item;
	if( idx != KErrNotFound )
		{
		item = iCurrentItems[idx];
		}
	else
		{
		item.iPluginUid = uid;
		item.iCurrentItemIndex = 0;
		iCurrentItems.Append( item );
		}

	return item.iCurrentItemIndex;
	}

void CEmTubeSearchView::SetCurrentItemIndex( TInt aCurrent )
	{
	if( aCurrent == KErrNotFound )
		return;
	TUint32 uid = iAppUi->PluginManager().Uid();
	TInt idx = FindCurrentItem( uid );
	if( idx != KErrNotFound )
		{
		iCurrentItems[idx].iCurrentItemIndex = aCurrent;
		}
	else
		{
		TCurrentItem item;
		item.iPluginUid = uid;
		item.iCurrentItemIndex = aCurrent;
		iCurrentItems.Append( item );
		}
	}

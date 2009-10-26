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

#include <AknIconArray.h>
#include <GULICON.H>
#include <GULUTIL.H>
#include <stringloader.h>
#include <aknpopup.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <aknnavilabel.h>
#include <mgfetch.h>

#include <PluginInterface.h>

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubeMainViewContainer.h"
#include "emTubeUiCustom.h"
#include "emTubeUiSymbian.h"
#include "emTubeUiItem.h"
#include "emTubeUiItemGfx.h"
#include "emTubeTransferManager.h"
#include "emTubeVideoUploadDialog.h"

#include "emTubeTimeOutTimer.h"

#define ICON_ANIMATION_FRAME_TIME 250000

CEmTubeMainViewContainer* CEmTubeMainViewContainer::NewL(CEmTubeMainView& aView, const TRect& aRect)
	{
	CEmTubeMainViewContainer* self = CEmTubeMainViewContainer::NewLC(aView, aRect);
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeMainViewContainer* CEmTubeMainViewContainer::NewLC(CEmTubeMainView& aView, const TRect& aRect)
	{
	CEmTubeMainViewContainer* self = new (ELeave) CEmTubeMainViewContainer(aView);
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}

CEmTubeMainViewContainer::~CEmTubeMainViewContainer()
	{
	delete iTimer;
	delete iIconTimer;

	iCommands.Close();
	iIconIndices.Close();
	iCommandStrings.Close();
	delete iUi;

	if( iNaviPane )
		iNaviPane->Pop(NULL);

	delete iNaviLabelDecorator;

    if( iCustomUiFont )
    	CEikonEnv::Static()->ReleaseScreenFont( iCustomUiFont );
	}

CEmTubeMainViewContainer::CEmTubeMainViewContainer(CEmTubeMainView& aView) : iView(aView), iPreviousSelectedItem(0)
	{
	}

void CEmTubeMainViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextApplication;
    }

void CEmTubeMainViewContainer::ConstructL(const TRect& aRect)
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	CreateWindowL();
	SetRect(aRect);

	if( iAppUi->S60Ui() )
	    iUi = CEmTubeUiSymbian::NewL( *this, aRect );
	else
	    {
	    iUi = CEmTubeUiCustom::NewL( *this, aRect );
	    iCustomUiFont = UiItemGfx::CreateFontL( UiItemGfx::EFontLarge );

    	CEikStatusPane *sp = iAppUi->StatusPane();
    	iNaviPane = (CAknNavigationControlContainer*)sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
    	iNaviLabelDecorator = iNaviPane->CreateNavigationLabelL();
    	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( KNullDesC() );
    	iNaviPane->PushL( *iNaviLabelDecorator );

    	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( iAppUi->PluginManager().Plugin()->Name() );
    	iNaviLabelDecorator->DrawDeferred();
	    }

    iUi->InitializeL( MUiInterface::ETypeSingle, 5, this, EFalse, this, aRect );

    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_SEARCH );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_SAVEDCLIPS );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_FAVORITES );
    iCommandStrings.Append( R_MAIN_VIEW_RECENTLY_UPLOADED );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_FEATURED );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_TOP_RATED );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_MOST_VIEWED );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_TRANSFERMANAGER );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_UPLOADVIDEO );
    iCommandStrings.Append( R_MAIN_VIEW_ELEMENT_PLAYLISTS );
    
    iIconIndices.Append( 0 ); //ECommandSearch
	iIconIndices.Append( 10 ); //ECommandSavedClips
	iIconIndices.Append( 12 ); //ECommandFavorites
	iIconIndices.Append( 14 ); //ECommandNewClips
	iIconIndices.Append( 4 ); //ECommandFeaturedClips
	iIconIndices.Append( 2 ); //ECommandTopRatedClips
	iIconIndices.Append( 6 ); //ECommandMostViewedClips
	iIconIndices.Append( 8 ); //ECommandTransferManager
	iIconIndices.Append( 18 ); //ECommandUploadVideo
	iIconIndices.Append( 16 ); //ECommandPlaylists
	
	CFbsBitmap *bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_search, EMbmOpenvideohubIcn_search_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_search1, EMbmOpenvideohubIcn_search1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_toprated, EMbmOpenvideohubIcn_toprated_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_toprated1, EMbmOpenvideohubIcn_toprated1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_featured, EMbmOpenvideohubIcn_featured_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_featured1, EMbmOpenvideohubIcn_featured1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_mostviewed, EMbmOpenvideohubIcn_mostviewed_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_mostviewed1, EMbmOpenvideohubIcn_mostviewed1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_transfer, EMbmOpenvideohubIcn_transfer_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_transfer1, EMbmOpenvideohubIcn_transfer1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_savedclips, EMbmOpenvideohubIcn_savedclips_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_savedclips1, EMbmOpenvideohubIcn_savedclips1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_bookmarks, EMbmOpenvideohubIcn_bookmarks_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_bookmarks1, EMbmOpenvideohubIcn_bookmarks1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_hot, EMbmOpenvideohubIcn_hot_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_hot1, EMbmOpenvideohubIcn_hot1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_hot, EMbmOpenvideohubIcn_hot_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_hot1, EMbmOpenvideohubIcn_hot1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_upload, EMbmOpenvideohubIcn_upload_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_upload1, EMbmOpenvideohubIcn_upload1_mask);
	iUi->AppendIconL(CGulIcon::NewL(bitmap, mask), ETrue);

	HBufC* txt;

	txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_SEARCH );
	iUi->AppendItemL( iIconIndices[ ECommandSearch], *txt, KNullDesC, NULL );
	CleanupStack::PopAndDestroy( txt );
	iCommands.Append( ECommandSearch );

#ifdef ENABLE_TRANSFER_MANAGER
	TBool found = EFalse;
	RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();
	for(TInt i=0;i<plugins.Count();i++)
		{
		if( plugins[i]->Capabilities() & KCapsVideoUpload )
			{
			found = ETrue;
			break;
			}
	    }

	if( found )
		{
		txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_UPLOADVIDEO );
		iUi->AppendItemL( iIconIndices[ECommandUploadVideo], *txt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( txt );
		iCommands.Append( ECommandUploadVideo );
		}
#endif

	if( IsCapabilitySupported( KCapsRecentlyUploaded ) )
		{
		txt = StringLoader::LoadLC( R_MAIN_VIEW_RECENTLY_UPLOADED );
    	iUi->AppendItemL( iIconIndices[ECommandNewClips], *txt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( txt );
		iCommands.Append( ECommandNewClips );
		}

	if( IsCapabilitySupported( KCapsFeatured ) )
		{
		txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_FEATURED );
    	iUi->AppendItemL( iIconIndices[ECommandFeaturedClips], *txt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( txt );
		iCommands.Append( ECommandFeaturedClips );
		}

	if( IsCapabilitySupported( KCapsTopRated ) )
		{
		txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_TOP_RATED );
    	iUi->AppendItemL( iIconIndices[ECommandTopRatedClips], *txt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( txt );
		iCommands.Append( ECommandTopRatedClips );
		}

	if( IsCapabilitySupported( KCapsMostViewed ) )
		{
		txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_MOST_VIEWED );
    	iUi->AppendItemL( iIconIndices[ECommandMostViewedClips], *txt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( txt );
		iCommands.Append( ECommandMostViewedClips );
		}

#ifdef ENABLE_TRANSFER_MANAGER
	txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_TRANSFERMANAGER );
	iUi->AppendItemL( iIconIndices[ECommandTransferManager], *txt, KNullDesC, NULL );
	CleanupStack::PopAndDestroy( txt );
	iCommands.Append( ECommandTransferManager );
#endif

	txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_SAVEDCLIPS );
	iUi->AppendItemL( iIconIndices[ECommandSavedClips], *txt, KNullDesC, NULL );
	CleanupStack::PopAndDestroy( txt );
	iCommands.Append( ECommandSavedClips );

#if ENABLE_PLAYLISTS
	txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_PLAYLISTS );
	iUi->AppendItemL( iIconIndices[ECommandPlaylists], *txt, KNullDesC, NULL );
	CleanupStack::PopAndDestroy( txt );
	iCommands.Append( ECommandPlaylists );
#endif

	txt = StringLoader::LoadLC( R_MAIN_VIEW_ELEMENT_FAVORITES );
	iUi->AppendItemL( iIconIndices[ECommandFavorites], *txt, KNullDesC, NULL );
	CleanupStack::PopAndDestroy( txt );
	iCommands.Append( ECommandFavorites );

	SetRect(aRect);
	ActivateL();
	ReplaceCurrentItemIconL();
	iUi->RefreshL( iView.CurrentMenuItem() );
	iPreviousSelectedItem = iView.CurrentMenuItem();
	iTimer = CEmTubeTimeOutTimer::NewL( *this );
	iTimer->After( 1000000, ETimerModeTransferManager );
//TODO - enable to have icon animations.
//TODO start animation after inactivity time, reset inactivity and timer in offerkeyevent.
//	iIconTimer = CEmTubeTimeOutTimer::NewL( *this );
//	iIconTimer->After( ICON_ANIMATION_FRAME_TIME, ETimerModeIconAnimation );
//	iCurrentFrame = 0;
	}

void CEmTubeMainViewContainer::TimerExpired( TInt aMode )
	{
	switch ( aMode )
		{
		case ETimerModeTransferManager:
			{
			iAppUi->StartDownloadByTML();
			}
		break;
		
		case ETimerModeIconAnimation:
			{
			TInt time = ICON_ANIMATION_FRAME_TIME;
			iCurrentFrame++;
			if( iCurrentFrame == 4 )
				{
				iCurrentFrame = 0;
				time = ICON_ANIMATION_FRAME_TIME * 2;
				}
			
			TInt idx = CurrentItemIndex();
			HBufC* txt = StringLoader::LoadLC( iCommandStrings[ iCommands[ idx ] ] );
			iUi->RemoveItemL( idx );
			iUi->InsertItemL( idx, iIconIndices[ iCommands[ idx ] ] + iCurrentFrame, *txt, KNullDesC(), NULL );
			iUi->RefreshL( idx );
			CleanupStack::PopAndDestroy( txt );
			iIconTimer->After( time, ETimerModeIconAnimation );
			}
		break;
		}
	}

TInt CEmTubeMainViewContainer::CountComponentControls() const
	{
	return iUi->CountComponentControls();
	}

CCoeControl* CEmTubeMainViewContainer::ComponentControl( TInt aIndex ) const
	{
	return iUi->ComponentControl( aIndex );
	}

void CEmTubeMainViewContainer::SizeChanged()
	{
	if( iUi )
		{
		if( iAppUi->S60Ui() )
			iUi->SizeChanged( Rect(), iView.CurrentMenuItem() );
		else
			DrawNow();
		}
	}

void CEmTubeMainViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);
	if( aType == KEikDynamicLayoutVariantSwitch )
		{
		iView.SetCurrentMenuItem ( CurrentItemIndex() );

		SetRect( iView.ClientRect() );

		if( !iAppUi->S60Ui() )
			iUi->SizeChanged( Rect(), iView.CurrentMenuItem() );

		DrawDeferred();
		}
	}

TKeyResponse CEmTubeMainViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
	{
	TKeyResponse response = iUi->OfferKeyEventL( aKeyEvent, aType );

	if( (aType == EEventKey) && (response == EKeyWasConsumed) )
	{
		switch (aKeyEvent.iScanCode)
			{
	        case EStdKeyUpArrow:
	        case EStdKeyDownArrow:
	        	{
				ReplaceCurrentItemIconL();
				iUi->RefreshL( CurrentItemIndex() );
				iPreviousSelectedItem = CurrentItemIndex();
	        	}
			break;

			default:
				break;
			}
	}

	return response;
	}

void CEmTubeMainViewContainer::Draw(const TRect& /*aRect*/) const
	{
	iUi->Draw( Rect(), SystemGc() );
	}

TInt CEmTubeMainViewContainer::CurrentItemIndex()
    {
    return iUi->CurrentItemIndex();
    }

void CEmTubeMainViewContainer::HandleCommandL( TObserverEvent aEvent )
	{
	switch( aEvent )
		{
		case EHandleItemClicked:
			{
			TCommandId id = iCommands[ CurrentItemIndex() ];

			switch( id )
				{
				case ECommandSearch:
					{
					iView.HandleCommandL( EMTVSearchCommand );
					}
				break;

				case ECommandSavedClips:
					{
					iAppUi->SetSearchNaviPaneId( R_SEARCH_SAVEDCLIPS_TXT );
					iView.HandleCommandL( EMTVShowSavedClipsCommand );
					}
				break;

				case ECommandFavorites:
					{
					iAppUi->SetSearchNaviPaneId( R_SEARCH_FAVORITES_TXT );
					iView.HandleCommandL( EMTVShowFavoritesCommand );
					}
				break;

				case ECommandNewClips:
					{
					iView.SetCurrentMenuItem ( CurrentItemIndex() );
					iAppUi->SearchL( ENewClips, EUpdated, EPeriodIrrelevant );
					}
				break;

				case ECommandFeaturedClips:
					{
					iView.SetCurrentMenuItem ( CurrentItemIndex() );
					iAppUi->SearchL( EFeaturedClips, ERelevance, EPeriodIrrelevant );
					}
				break;

				case ECommandTopRatedClips:
					{
					iView.SetCurrentMenuItem ( CurrentItemIndex() );

					if( iAppUi->PluginManager().Plugin()->Capabilities() & KCapsTimeFrame )
						{
						TPeriod period;
						if( SelectTimePeriodL( period ) )
						    iAppUi->SearchL( ETopRatedClips, ERating, period );
						}
					else
						{
					    iAppUi->SearchL( ETopRatedClips, ERating, EPeriodIrrelevant );
						}
					}
				break;

				case ECommandMostViewedClips:
					{
					iView.SetCurrentMenuItem ( CurrentItemIndex() );

					if( iAppUi->PluginManager().Plugin()->Capabilities() & KCapsTimeFrame )
						{
						TPeriod period;
						if( SelectTimePeriodL( period ) )
    						iAppUi->SearchL( EMostViewedClips, EViewCount, period );
						}
					else
						{
   						iAppUi->SearchL( EMostViewedClips, EViewCount, EPeriodIrrelevant );
						}
					}
				break;

				case ECommandTransferManager:
					{
					iView.HandleCommandL( EMTVActivateTransferManagerViewCommand );
					}
				break;

				case ECommandPlaylists:
					{
					iView.HandleCommandL( EMTVActivatePlaylistsViewCommand );
					}
				break;

				case ECommandUploadVideo:
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

				    HBufC* title = StringLoader::LoadLC( R_UPLOAD_TO_TITLE_TXT );
				    popupList->SetTitleL( *title );
				    CleanupStack::PopAndDestroy( title );

					RArray<TUint32> uids;
					CleanupClosePushL( uids );

					RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();
					for(TInt i=0;i<plugins.Count();i++)
						{
						if( plugins[i]->Capabilities() & KCapsVideoUpload )
							{
							uids.Append( iAppUi->PluginManager().Uid( i ) );
							TPtrC name = plugins[i]->Name();
					    	items->AppendL( name ); //TODO -> add icons?
							}
					    }

				    TInt popupOk = popupList->ExecuteLD();
				    if(popupOk)
				        {
				        TInt idx = plist->CurrentItemIndex();

					 	CDesCArrayFlat* files = new (ELeave)CDesCArrayFlat(2);
					 	CleanupStack::PushL( files );

						TBool ret = MGFetch::RunL( *files, EVideoFile, EFalse );
						if( ret )
							{
							CQueueEntry* entry = CQueueEntry::NewLC();
							entry->SetFilename( files->MdcaPoint(0) );
							entry->SetUid( uids[ idx ] );

							CEmTubeVideoUploadDialog* uploadDialog = CEmTubeVideoUploadDialog::NewLC( *entry );
							TInt err = uploadDialog->ExecuteLD( R_UPLOAD_VIDEO_EDIT_DIALOG );
							CleanupStack::Pop( uploadDialog );

							if( err != 0 )
								{
			                    if( iAppUi->TransferManager().AllProcessed() )
			                        iAppUi->TransferManager().SetTMStarted( ETrue );

								iAppUi->TransferManager().AddToQueueL( *entry );
								CleanupStack::Pop( entry );
								}
							else
								{
								CleanupStack::PopAndDestroy( entry );
								}
							}
						CleanupStack::PopAndDestroy( files );
				        }

				    CleanupStack::PopAndDestroy( &uids );

				    CleanupStack::Pop( popupList );
				    CleanupStack::PopAndDestroy( plist );
					}
				break;
				}
			}
		break;

		default:
			break;
		}
    }

void CEmTubeMainViewContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
#ifndef __S60_50__
    return;
#else
    if( AknLayoutUtils::PenEnabled() )
    	{
        CCoeControl::HandlePointerEventL( aPointerEvent );

        if(aPointerEvent.iType == TPointerEvent::EButton1Up)
        	{
        	if( CurrentItemIndex() == iPreviousSelectedItem )
        		{
        		if( (iCommands[CurrentItemIndex()] == ECommandTopRatedClips)
        				|| (iCommands[CurrentItemIndex()] == ECommandMostViewedClips) )
        			FloatingTimePeriodDialogL( aPointerEvent.iPosition );
        		else
					HandleCommandL( EHandleItemClicked );
        		}
        	else
        		iPreviousSelectedItem = CurrentItemIndex();
        	}
    	}
#endif
	}

TBool CEmTubeMainViewContainer::SelectTimePeriodL( TPeriod& aPeriod )
	{
	CAknSinglePopupMenuStyleListBox* plist = new(ELeave) CAknSinglePopupMenuStyleListBox;
	CleanupStack::PushL(plist);

    CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_SELECT_CANCEL, AknPopupLayouts::EMenuWindow);
    CleanupStack::PushL(popupList);

    plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
    plist->CreateScrollBarFrameL(ETrue);
    plist->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    MDesCArray* itemList = plist->Model()->ItemTextArray();
    CDesCArray* items = (CDesCArray*) itemList;

    HBufC* line = StringLoader::LoadLC( R_PERIOD_TODAY_TXT );
    items->AppendL( *line );
    CleanupStack::PopAndDestroy( ); // line

    line = StringLoader::LoadLC( R_PERIOD_THIS_WEEK_TXT );
    items->AppendL( *line );
    CleanupStack::PopAndDestroy( ); // line

    line = StringLoader::LoadLC( R_PERIOD_THIS_MONTH_TXT );
    items->AppendL( *line );
    CleanupStack::PopAndDestroy( ); // line

    line = StringLoader::LoadLC( R_PERIOD_ALL_TIME_TXT );
    items->AppendL( *line );
    CleanupStack::PopAndDestroy( ); // line

	plist->SetCurrentItemIndex( 0 );

    HBufC* header = StringLoader::LoadLC( R_PERIOD_HEADER_TXT );
    popupList->SetTitleL( *header );
    CleanupStack::PopAndDestroy( ); // header

	TInt response = popupList->ExecuteLD();

	if( response )
		{
		aPeriod = (TPeriod) plist->CurrentItemIndex();
		}

    CleanupStack::Pop( ); //popupList
    CleanupStack::PopAndDestroy( );	//list

    return response;
	}

 void CEmTubeMainViewContainer::FloatingTimePeriodDialogL( TPoint aWhere )
 	{
#ifdef __S60_50__

	delete iPopup;

	iPopup = CAknStylusPopUpMenu::NewL( this, aWhere );

    HBufC* txt = StringLoader::LoadLC( R_PERIOD_TODAY_TXT );
	iPopup->AddMenuItemL( *txt, 0 );
    CleanupStack::PopAndDestroy(); // line

    txt = StringLoader::LoadLC( R_PERIOD_THIS_WEEK_TXT );
	iPopup->AddMenuItemL( *txt, 1 );
    CleanupStack::PopAndDestroy(); // line

    txt = StringLoader::LoadLC( R_PERIOD_THIS_MONTH_TXT );
	iPopup->AddMenuItemL( *txt, 2 );
    CleanupStack::PopAndDestroy(); // line

    txt = StringLoader::LoadLC( R_PERIOD_ALL_TIME_TXT );
	iPopup->AddMenuItemL( *txt, 3 );
    CleanupStack::PopAndDestroy(); // line

	iPopup->ShowMenu();

#endif
 	}

MItemDrawer::TItemState CEmTubeMainViewContainer::DrawItemL( CFbsBitmap* aDstBitmap, CFbsBitGc* aDstBitmapGc,
            TSize aNewSize, TPoint aDstPoint, TBool aShowText, TBool aSelected, CUiItem* aItem,
            TBool aScrollText )
    {
    MItemDrawer::TItemState result = EItemFit;

    if( aShowText )
        {
        TRgb textColor;

    	if( aSelected )
    	{
        	aDstBitmapGc->SetBrushColor( TRgb(15,80,122) );
        	aDstBitmapGc->SetPenColor( TRgb(15,80,122) );
        	aDstBitmapGc->SetBrushStyle( CGraphicsContext::ESolidBrush );
        	aDstBitmapGc->SetPenStyle(CGraphicsContext::ESolidPen);

            TInt textHeight = iCustomUiFont->HeightInPixels() / 2 + 4;
            TInt middle = aDstPoint.iY + aNewSize.iHeight / 2;
            TRect boxRect( 0, middle - textHeight, Rect().Width(), middle + textHeight );
            aDstBitmapGc->DrawRect(boxRect);

            aDstBitmapGc->SetBrushStyle( CGraphicsContext::ENullBrush );
        	aDstBitmapGc->SetPenColor( TRgb(110,190,242) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iTl.iY ), TPoint( Rect().Width(), boxRect.iTl.iY ) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iTl.iY - 1 ), TPoint( Rect().Width(), boxRect.iTl.iY - 1) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iBr.iY), TPoint( Rect().Width(), boxRect.iBr.iY ) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iBr.iY + 1), TPoint( Rect().Width(), boxRect.iBr.iY + 1) );

    	    textColor = KCustomUiSelectedTextColor;
    	}

        else
    	    textColor = KCustomUiTextColor;

	    aDstBitmapGc->SetPenColor( textColor );
    	aDstBitmapGc->UseFont( iCustomUiFont );

        TInt iconMargin = (TInt)( aNewSize.iWidth * 1.1 );
        TRect rect( aDstPoint.iX + iconMargin, aDstPoint.iY, Rect().Width(), aDstPoint.iY + aNewSize.iHeight );
        TInt baseline = rect.Height() / 2 + iCustomUiFont->AscentInPixels() / 2;

        TPtrC text( aItem->FirstLine() );
        RBuf textDots;

        TInt textWidth = iCustomUiFont->TextWidthInPixels( text );
        if( (rect.Width() - 5) < (textWidth  - iScrollAmount) )
            {
            if( aScrollText )
                {
                result = EItemScrolling;
                iScrollAmount++;
                }
            else
                {
                result = EItemLarger;
                iScrollAmount = 0;

                UiItemGfx::TextDotsL( text, rect.Width(), iCustomUiFont, textDots );
                text.Set( textDots );
                }
            }
        else if( iScrollAmount )
            {
            result = EItemScrolled;
            }

        aDstBitmapGc->SetClippingRect( rect );
        rect.iTl.iX -= iScrollAmount;

        UiItemGfx::DrawText( aDstBitmapGc, text, textColor, rect, baseline, ETrue );

        aDstBitmapGc->CancelClippingRect();
        textDots.Close();

        if( result == EItemScrolled )
            iScrollAmount = 0;
        }

    UiItemGfx::BltIconScale( aDstBitmap, iUi->Icon( aItem->IconIdx() ), aNewSize, aDstPoint );

    return result;
    }

TBool CEmTubeMainViewContainer::IsCapabilitySupported( TInt aCapabilty )
	{
	RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();

	for(TInt i=0;i<plugins.Count();i++)
		if( plugins[i]->Capabilities() & aCapabilty )
			return ETrue;

	return EFalse;
	}

#ifdef __S60_50__

void CEmTubeMainViewContainer::ProcessCommandL( TInt aCommandId )
	{
	if( aCommandId < 0 )
		return;

	TPeriod period = (TPeriod) aCommandId;

	switch( iCommands[CurrentItemIndex()] )
		{
		case ECommandTopRatedClips:
			iAppUi->SearchL( ETopRatedClips, ERating, period );
		break;

		case ECommandMostViewedClips:
			iAppUi->SearchL( EMostViewedClips, EViewCount, period );
		break;

		default:
			break;
		}
	}

void CEmTubeMainViewContainer::SetEmphasis( CCoeControl* /*aMenuControl*/, TBool /*aEmphasis*/ )
	{
	}

#endif

void CEmTubeMainViewContainer::ReplaceCurrentItemIconL()
	{
//TODO - fixme!
#if 0
	TInt idx = iPreviousSelectedItem;
	HBufC* txt = StringLoader::LoadLC( iCommandStrings[ iCommands[ idx ] ] );
	iUi->RemoveItemL( idx );
	iUi->InsertItemL( idx, iIconIndices[ iCommands[ idx ] ], *txt, KNullDesC(), NULL );
	CleanupStack::PopAndDestroy( txt );

	idx = CurrentItemIndex();
	txt = StringLoader::LoadLC( iCommandStrings[ iCommands[ idx ] ] );
	iUi->RemoveItemL( idx );
	iUi->InsertItemL( idx, iIconIndices[ iCommands[ idx ] ] + 1, *txt, KNullDesC(), NULL );
	CleanupStack::PopAndDestroy( txt );
#endif
	}

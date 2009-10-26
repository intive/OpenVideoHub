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
#include <aknnavilabel.h>
#include <AknCommonDialogs.h>
#include <CAknMemorySelectionDialog.h>
#include <CAknFileSelectionDialog.h>
#include <CAknFileNamePromptDialog.h>
#include <BAUTILS.H>
#include <avkon.mbg>
#include <aknsfld.h>  // SearchBox
#include <aknnotewrappers.h> // CAknInformationNote

#ifdef __S60_50__
#include <akntoolbar.h> // CAknToolbar
#endif

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubeSearchViewContainer.h"
#include "emTubeVideoEntry.h"
#include "emTubeImageLoader.h"
#include "emTubeTransferManager.h"

#include "emTubeUiItem.h"
#include "emTubeUiItemGfx.h"
#include "emTubeUiCustom.h"
#include "emTubeUiSymbian.h"

#define FIRST_VIDEO_ICON_IDX 3
//search view container::
CEmTubeSearchViewContainer* CEmTubeSearchViewContainer::NewL(CEmTubeSearchView& aView, const TRect& aRect, const TDesC& aFindBoxText )
	{
	CEmTubeSearchViewContainer* self = CEmTubeSearchViewContainer::NewLC(aView, aRect, aFindBoxText );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeSearchViewContainer* CEmTubeSearchViewContainer::NewLC(CEmTubeSearchView& aView, const TRect& aRect, const TDesC& aFindBoxText )
	{
	CEmTubeSearchViewContainer* self = new (ELeave) CEmTubeSearchViewContainer(aView);
	CleanupStack::PushL(self);
	self->ConstructL(aRect, aFindBoxText );
	return self;
	}

CEmTubeSearchViewContainer::~CEmTubeSearchViewContainer()
	{
	delete iTimeFormatString;

	iBitmaps.ResetAndDestroy();
	iBitmaps.Close();

	if( iTimer )
		iTimer->Cancel();
	delete iTimer;

//	delete iBackBitmap;
	delete iStarBitmapRed;
	delete iStarMaskRed;
	delete iStarBitmapGray;
	delete iStarMaskGray;
	delete iStarBitmapWhite;
	delete iStarMaskWhite;
	delete iHeartBitmap;
	delete iHeartMask;

	if( iNaviPane )
		iNaviPane->Pop(NULL);

	delete iNaviLabelDecorator;
	delete iUi;

    if( iCustomUiFont )
    	CEikonEnv::Static()->ReleaseScreenFont( iCustomUiFont );

    delete iAuthorLogo;
    delete iAuthorMask;
    delete iTimeLogo;
    delete iTimeMask;

#ifdef __S60_50__
    delete iPopup;
#endif
	}

CEmTubeSearchViewContainer::CEmTubeSearchViewContainer(CEmTubeSearchView& aView) : iView(aView)
	{
	}

void CEmTubeSearchViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextSearch;
    }

void CEmTubeSearchViewContainer::ConstructL(const TRect& aRect, const TDesC& aFindBoxText )
	{
	iTimeFormatString = CEikonEnv::Static()->AllocReadResourceL( R_QTN_TIME_DURAT_MIN_SEC );

	AknIconUtils::CreateIconL(iStarBitmapRed, iStarMaskRed, KBitmapFileName, EMbmOpenvideohubStar_red, EMbmOpenvideohubStar_red_mask);
	AknIconUtils::CreateIconL(iStarBitmapGray, iStarMaskGray, KBitmapFileName, EMbmOpenvideohubStar_gray, EMbmOpenvideohubStar_gray_mask);
	AknIconUtils::CreateIconL(iStarBitmapWhite, iStarMaskWhite, KBitmapFileName, EMbmOpenvideohubStar_white, EMbmOpenvideohubStar_white_mask);
	AknIconUtils::CreateIconL(iHeartBitmap, iHeartMask, KBitmapFileName, EMbmOpenvideohubIcn_bookmarks, EMbmOpenvideohubIcn_bookmarks_mask);

	iFrameCount = 8;
	for(TInt i=0;i<iFrameCount;i++)
		{
		CFbsBitmap *bitmap, *mask;		
#if 1
		AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohub11 + (i*2), EMbmOpenvideohub11 + (i*2) + 1);
#else
		AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubLoading1 + (i*2), EMbmOpenvideohubLoading1 + (i*2) + 1);
		TInt width = (aRect.Width() * 27) / 240;
		TSize size( width, width );
		AknIconUtils::SetSize( bitmap, size);
#endif
		iBitmaps.Append( bitmap );
		iBitmaps.Append( mask );
		}

	iTimer = CEmTubeTimeOutTimer::NewL( *this );

	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	iProgressEnabled = iAppUi->GotFeature();

	CEikStatusPane *sp = iAppUi->StatusPane();
	iNaviPane = (CAknNavigationControlContainer*)sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
	iNaviLabelDecorator = iNaviPane->CreateNavigationLabelL();
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( KNullDesC() );
	iNaviPane->PushL( *iNaviLabelDecorator );

	CreateWindowL();

	if( iAppUi->S60Ui() )
	    iUi = CEmTubeUiSymbian::NewL( *this, aRect );
	else
	    {
	    iUi = CEmTubeUiCustom::NewL( *this, aRect );
	    iCustomUiFont = UiItemGfx::CreateFontL( UiItemGfx::EFontSmall );
#ifdef ENABLE_CUSTOM_UI
    	AknIconUtils::CreateIconL(iAuthorLogo, iAuthorMask, KBitmapFileName, EMbmOpenvideohubIcn_author, EMbmOpenvideohubIcn_author_mask);
    	AknIconUtils::CreateIconL(iTimeLogo,iTimeMask, KBitmapFileName, EMbmOpenvideohubIcn_time, EMbmOpenvideohubIcn_time_mask);
#endif
	    }

#ifdef ENABLE_FINDBOX_IN_SAVEDCLIPS
	iUi->SetFindBoxTextL( aFindBoxText );
#endif

	if( iAppUi->S60Ui() && (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries) )
    	{
#ifdef ENABLE_TABS
		RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();
		RArray<TPtrC> names;
		TInt selected = 0;
		iPluginsCount = plugins.Count();

		for(TInt i=0;i<iPluginsCount;i++)
			{
			names.Append( plugins[i]->Name() );

			if( iAppUi->PluginManager().Uid() == iAppUi->PluginManager().Uid( i ) )
				selected = i;
			}

		if( names.Count() > 1 )
			iUi->AddTabsL( names, selected );
		else
			SetNaviPaneTextL();

		names.Close();
#else
			SetNaviPaneTextL();
#endif
    	}
	else
		SetNaviPaneTextL();

	HBufC* txt = StringLoader::LoadLC( R_SEARCH_VIEW_EMPTY_TXT );
    iUi->SetEmptyTextL( *txt );
	CleanupStack::PopAndDestroy( txt );

	UpdateToolbar();

	SetRect(aRect);
	ActivateL();

#ifdef ENABLE_FINDBOX_IN_SAVEDCLIPS
	if( iAppUi->S60Ui() )
		{
	    iFindBoxEnabled = ( (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries ) ||
    	    (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries) );
		}
	else
#endif
		{
		iFindBoxEnabled = EFalse;
		}

	switch( iAppUi->SearchDisplayMode() )
		{
		case CEmTubeAppUi::ESearchDisplayFavEntries:
		case CEmTubeAppUi::ESearchDisplaySavedEntries:
			{
			iView.SetCbaL( R_EMTV_SEARCH_VIEW_CBA );
			DisplaySearchResultsL();
			}
		break;

		default:
			{
			if( !iProgressEnabled || iAppUi->SearchEntries().Count() )
				{
				iView.SetCbaL( R_EMTV_SEARCH_VIEW_CBA );
				DisplaySearchResultsL();
				}
			else
				{
				iView.SetCbaL( R_EMTV_PROGRESS_DIALOG_CBA );
				TimerExpired( 0 );
				}
			}
		break;
		}
	}

TInt CEmTubeSearchViewContainer::CountComponentControls() const
	{
    if( iProgressEnabled )
		return 0;

    return iUi->CountComponentControls();
	}

CCoeControl* CEmTubeSearchViewContainer::ComponentControl( TInt aIndex ) const
	{
    if( iProgressEnabled )
		return NULL;

    return iUi->ComponentControl( aIndex );
	}

void CEmTubeSearchViewContainer::SizeChanged()
	{
	if( iUi && !iProgressEnabled )
		{
#if 1
		if( iUi->Initialized() )
			RescaleIconsL();
#endif
		if( iAppUi->S60Ui() )
			iUi->SizeChanged( Rect(), iView.CurrentItemIndex() );
		else
			DrawNow();
		}
	}

void CEmTubeSearchViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);

	if ( aType == KEikDynamicLayoutVariantSwitch )
		{
		SaveCurrentPosition();
//		delete iBackBitmap;
		iBackBitmap = NULL;
//		TRect rect;
//		AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, rect );
		SetRect( iView.ClientRect() );

		if( !iAppUi->S60Ui() )
			iUi->SizeChanged( Rect(), iView.CurrentItemIndex() );
		}
	}

TKeyResponse CEmTubeSearchViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	TKeyResponse response = EKeyWasNotConsumed;
	if( !iProgressEnabled )
		response = iUi->OfferKeyEventL( aKeyEvent, aType );

	if( (aType == EEventKey) && (response == EKeyWasConsumed) )
	{
		switch (aKeyEvent.iScanCode)
			{
	        case EStdKeyUpArrow:
	        case EStdKeyDownArrow:
	        	{
	        	HandleCommandL( MHandleCommandObserver::EHandleItemSelected );
	        	iPreviousSelectedItem = CurrentItemIndex();
	        	}
			break;

			default:
				break;
			}
	}

	return response;
	}

void CEmTubeSearchViewContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
#ifndef __S60_50__
    return;
#else
    if( AknLayoutUtils::PenEnabled() )
    	{
        CCoeControl::HandlePointerEventL( aPointerEvent );

        if(aPointerEvent.iType == TPointerEvent::EButton1Up)
        	{
        	if( CurrentItemIndex() == iPreviousSelectedItem  )
        		{
        		if( IsListboxLineMore() )
        			HandleCommandL( MHandleCommandObserver::EHandleItemClicked );
        		else
                	RunFloatingItemDialogL( aPointerEvent.iPosition );
        		}
        	else
        		iPreviousSelectedItem = CurrentItemIndex();
        	}
    	}
#endif
	}

TBool CEmTubeSearchViewContainer::IsListboxLineMore()
	{
	TBool more = EFalse;
	TInt count = iUi->ItemCount();

	if( count )
		{
		if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
			{
			if( (iAppUi->PluginManager().SearchState() == ESearching)
				&& count == ( CurrentItemIndex() + 1 ) )
				more = ETrue;
			}
		}
	return more;
	}

TBool CEmTubeSearchViewContainer::IsCurrentItemInFavsL()
	{
	TBool ret = ETrue;
	TInt idx = iView.CurrentToActual();

	if( idx != KErrNotFound )
		{
		if( iAppUi->FindInFavoritesL( iAppUi->SearchEntries()[ idx ] ) == KErrNotFound )
			ret = EFalse;
		else
			ret = ETrue;
		}
	return ret;
	}

void CEmTubeSearchViewContainer::AddCurrentItemToFavsL()
	{
	TInt idx = iView.CurrentToActual();

	if( idx != KErrNotFound )
		{
		iAppUi->AddToFavoritesL( iAppUi->SearchEntries()[ idx ] );

		RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();

		TInt width, height;
		iUi->GetIconSize( width, height );

		CVideoEntry* entry = entries[ idx ];
		if( entry->ImageLoaded() )
			{
			CGulIcon* icon = iUi->Icon( entry->IconIdx() );
			icon->SetBitmap( entry->ScaledBitmapL( width, height ) );
			ApplyRatingStarsL( entry );
			icon->SetBitmapsOwnedExternally( ETrue );
			}
		DrawNow();
		}
	}

void CEmTubeSearchViewContainer::DeleteSavedClipL()
	{
	TInt idx = CurrentItemIndex();
	if( idx != KErrNotFound )
		{
		if( iAppUi->ConfirmationQueryL( R_DELETE_SAVED_VIDEO_TXT ) )
			{
			iUi->RemoveItemL( idx );

			TInt count = iUi->ItemCount();
			if( count )
				{
				iUi->RefreshL( idx - 1 );
				}
			iAppUi->DeleteFromSavedClipsL( iAppUi->SearchEntries()[ idx ] );
			}
		}
	}

void CEmTubeSearchViewContainer::RenameSavedClipL()
	{
	TInt idx = CurrentItemIndex();
	if( idx != KErrNotFound )
		{
		TFileName name;
		CAknFileNamePromptDialog* dlg = CAknFileNamePromptDialog::NewL();

		CVideoEntry* e = iAppUi->SearchEntries()[ idx ];

	    TParsePtrC parse( e->SavedFileName() );

		dlg->SetPathL( parse.DriveAndPath() );

		name.Copy( parse.Name() );

		TInt ret = dlg->ExecuteL( name );
		delete dlg;

		if( ret )
			{
			iAppUi->RenameSavedClipL( e, name );

			iUi->RemoveItemL( idx );

			TInt id = 0;
			if( e->ImageLoaded() )
				{
				id = idx + FIRST_VIDEO_ICON_IDX;
				}

			RBuf line1;
			CleanupClosePushL( line1 );
			RBuf line2;
			CleanupClosePushL( line2 );

			CreateLineL( line1, line2, e );

			iUi->InsertItemL( idx, id, line1, line2, e );

			iUi->RefreshL( idx );

			CleanupStack::PopAndDestroy( &line2 );
			CleanupStack::PopAndDestroy( &line1 );
			}
		}
	}

void CEmTubeSearchViewContainer::RemoveCurrentItemFromFavsL()
	{
	TInt idx = iView.CurrentToActual();

	if( idx != KErrNotFound )
		{

		if( iAppUi->ConfirmationQueryL( R_REMOVE_FROM_FAVORITES_TXT ) )
			{
			iAppUi->DeleteFromFavoritesL( iAppUi->SearchEntries()[ idx ] );

			if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries )
				{

				iUi->RemoveItemL( idx );

				TInt count = iUi->ItemCount();
				if( count )
					{
					iUi->RefreshL( idx - 1 );
					}
				}
			else
				{
				RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();

				TInt width, height;
				iUi->GetIconSize(width, height );

				CVideoEntry* entry = entries[ idx ];
				if( entry->ImageLoaded() )
					{
					CGulIcon* icon = iUi->Icon( entry->IconIdx() );
					icon->SetBitmap( entry->ScaledBitmapL( width, height ) );
					ApplyRatingStarsL( entry );
					icon->SetBitmapsOwnedExternally( ETrue );
					}
				DrawNow();
				}
			UpdateToolbar();
			}
		}
	}

TInt CEmTubeSearchViewContainer::CurrentItemIndex()
	{
	TInt ret = KErrNotFound;

	ret = iUi->CurrentItemIndex();

	return ret;
	}

void CEmTubeSearchViewContainer::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case 0:
			{
			DrawNow();
			iTimer->After( 100000, 0 );
			iCurrentFrame++;
			if( iCurrentFrame > (iFrameCount - 1) )
				iCurrentFrame = 0;
			}
		break;

		case 1:
			{
			if( iAppUi->PluginManager().SearchState() == ESearchNotStarted )
				{
				StartProgressBarL( EFalse );
				UpdateToolbar();
				iAppUi->SearchL();
				}
			}
		break;

		default:
			break;
		}
	}

void CEmTubeSearchViewContainer::Draw(const TRect& /*aRect*/) const
	{
	if( iProgressEnabled )
		{
		User::ResetInactivityTime();

		CWindowGc& gc = SystemGc();
		if ( iBackBitmap )
			{
			gc.BitBlt( TPoint( 0, 0 ), iBackBitmap );
			}
		else
			{
			gc.SetBrushColor( KRgbBlack );
			gc.SetPenColor( KRgbBlack );
			gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
			gc.SetPenStyle( CGraphicsContext::ENullPen );
			gc.DrawRect( Rect() );
			}

		CFbsBitmap* bmp = iBitmaps[ iCurrentFrame * 2];
		CFbsBitmap* mask = iBitmaps[ iCurrentFrame * 2 + 1];

		TSize size = bmp->SizeInPixels();
		TInt x = (Rect().Width() / 2) - ( size.iWidth / 2 );
		TInt y = (Rect().Height() / 2) - ( size.iHeight / 2 );

	   	TPoint point = TPoint( x , y );
	   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );

		if ( iBackBitmap )
			{
		   	gc.BitBltMasked( point,
							bmp,
							sourceRect,
							mask,
							ETrue );
			}
		else
			{
			gc.BitBlt( TPoint( x, y ), bmp );
			}
		}
	else
		{
		iUi->Draw( Rect(), SystemGc() );
		}
	}

void CEmTubeSearchViewContainer::SaveCurrentPosition()
	{
	iView.SetCurrentItemIndex( CurrentItemIndex() );
	}

void CEmTubeSearchViewContainer::CreateLineL( RBuf& aLine, RBuf& aLine1, CVideoEntry* aEntry )
	{
	TBuf<128> duration;
	TTime ttime(0);
	ttime += (TTimeIntervalSeconds) aEntry->Duration();
	ttime.FormatL( duration, *iTimeFormatString);

	aLine.Create( aEntry->MediaTitle().Length() );
	aLine.Copy( aEntry->MediaTitle() );

	aLine1.Create( aEntry->AuthorName().Length() + 1 + duration.Length() + 3 );
	aLine1.Copy( duration );

	if( iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySavedEntries && aEntry->AuthorName().Length() )
		{
		aLine1.Append( _L(" - ") );
		aLine1.Append( aEntry->AuthorName() );
		}
	else if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
		{
		TParsePtrC parse( aEntry->SavedFileName() );
		TPtrC drive = parse.Drive();
//TODO -> pass second icon index to AddItemL();
/*
		if( !drive.Compare( _L("E:") ) )
			{
			aLine1.ReAllocL( aLine.Length() + 3 );
			aLine1.Append( _L("\t") );
			aLine1.Append( _L("2") );
			}
*/		}
	}

void CEmTubeSearchViewContainer::ImageLoadedL( TInt aIndex )
	{
	RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();
	
	if( aIndex < 0 || aIndex >= entries.Count() )
		return;
	
	CVideoEntry* entry = entries[aIndex];

	if( entry->ImageLoaded() )
		{
		TInt width, height;
		iUi->GetIconSize( width, height );

		entry->SetIconIdx( iUi->IconsCount() );

		CGulIcon* icon = CGulIcon::NewL();
		if( !entry->ScaledBitmap() )
			icon->SetBitmap( entry->ScaledBitmapL( width, height ) );
		else
			icon->SetBitmap( entry->ScaledBitmap() );

		if( iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySavedEntries )
			ApplyRatingStarsL( entry );

		icon->SetBitmapsOwnedExternally( ETrue );
		iUi->AppendIconL( icon, EFalse );

		if( IsEntryVisibleL( entry ) )
			{
			if( iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries )
				{
				iUi->ChangeItemIconIdxL( aIndex, iUi->IconsCount() - 1 );
				}
			else
				{
				TInt which = 0;

				for( TInt i=0 ; i < aIndex ; i++ )
					if( entry->PluginUid() == entries[i]->PluginUid() )
						which++;

				iUi->ChangeItemIconIdxL( which, iUi->IconsCount() - 1 );
				}
			}

		iView.SetCurrentItemIndex( CurrentItemIndex() );
		iUi->RefreshL( -1 );
		}
	}

void CEmTubeSearchViewContainer::StartProgressBarL( TBool aClear )
	{
	if( !iProgressEnabled )
		{
//	delete iBackBitmap;
		iBackBitmap = NULL;

		iBackBitmap = iUi->BackgroundBitmapL( aClear );
		iUi->DestroyL();

		iProgressEnabled = ETrue;
		iView.SetCbaL( R_EMTV_PROGRESS_DIALOG_CBA );
		TimerExpired( 0 );
		}
	}

void CEmTubeSearchViewContainer::CancelProgressL()
	{
	DisplaySearchResultsL();
	}

TInt CEmTubeSearchViewContainer::DisplaySearchResultsL()
	{
	TInt itemsAdded = 0;

	iTimer->Cancel();
	iProgressEnabled = EFalse;

	if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
		{
		TInt resource;

		if( iAppUi->GotFeature() )
			resource = R_SEARCH_VIEW_EMPTY_TXT;
		else
			resource = R_SEARCH_VIEW_FEATURE_NOT_SUPPORTED_TXT;

		HBufC* txt = StringLoader::LoadLC( resource );
	    iUi->SetEmptyTextL( *txt );
		CleanupStack::PopAndDestroy( txt );
		}

	if( !iUi->Initialized() )
	    iUi->InitializeL( MUiInterface::ETypeDoubleLarge, 5, this, iFindBoxEnabled, this, Rect() );

	iUi->ResetItems();
	iUi->ResetIcons();

	CFbsBitmap *bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubNo_image, EMbmOpenvideohubNo_image_mask);
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask), ETrue );
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubEmpty, EMbmOpenvideohubEmpty_mask);
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask), ETrue );
	AknIconUtils::CreateIconL( bitmap, mask, KSystemIconFile, EMbmAvkonQgn_prop_mmc_memc_large, EMbmAvkonQgn_prop_mmc_memc_large_mask );
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask ), ETrue );

/*
	if( !iCategory->Length() && iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
		{
		for(TInt i=0;i<iAppUi->SavedCategoriesEntries().Count(); i++)
			{
			CCategory *c = iAppUi->SavedCategoriesEntries()[i];
			RBuf line;
			CleanupClosePushL( line );
			CreateLineL( line, c, 1 );
			list->AppendL( line );
			CleanupStack::PopAndDestroy( &line );
			}
		}
*/
	TBool loadNewPictures = EFalse;

	TInt iconIdx = FIRST_VIDEO_ICON_IDX;
	for(TInt i=0;i<iAppUi->SearchEntries().Count(); i++)
		{
		CVideoEntry* entry = iAppUi->SearchEntries()[i];
		TBool add = ETrue;

        if( add )
            add = IsEntryVisibleL( entry );

		if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
			{
	        if( entry->PluginUid() != iAppUi->PluginManager().Uid() )
	        	add = EFalse;
			}

		if( add )
			{
			itemsAdded++;

			TInt idx = 0;

			if( entry->ImageLoaded() && entry->ScaledBitmap() )
				{
				CGulIcon* icon = CGulIcon::NewL();

			    icon->SetBitmap( entry->ScaledBitmap() );
				icon->SetBitmapsOwnedExternally( ETrue );

				iUi->AppendIconL( icon, EFalse );

				idx = iconIdx;
				entry->SetIconIdx( idx );
				iconIdx++;
				}
			else
				loadNewPictures = ETrue;

    		RBuf line1;
    		CleanupClosePushL( line1 );
    		RBuf line2;
    		CleanupClosePushL( line2 );

    		CreateLineL( line1, line2, entry );
			iUi->AppendItemL( idx, line1, line2, entry );


    		CleanupStack::PopAndDestroy( &line2 );
    		CleanupStack::PopAndDestroy( &line1 );
			}

		}

	if( (iAppUi->PluginManager().SearchState() == ESearching) && itemsAdded
		&& (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries) )
		{
		HBufC* moreTxt = StringLoader::LoadLC( R_SEARCH_MORE_TXT );
		iUi->AppendItemL( 1, *moreTxt, KNullDesC, NULL );
		CleanupStack::PopAndDestroy( moreTxt );
		}

	TInt idx = iView.CurrentItemIndex();
	if( idx >= iUi->ItemCount() )
		{
		idx -= 1;
		if( idx < 0 ) idx = 0;
		iView.SetCurrentItemIndex( idx );
		iPreviousSelectedItem = idx;
		}

#if 1
	SizeChanged();
#endif
	iUi->RefreshL( iView.CurrentItemIndex() );

    iView.SetCbaL( R_EMTV_SEARCH_VIEW_CBA );

	if(loadNewPictures)
		iAppUi->StartDownloadingImagesL();

	UpdateToolbar();

	return itemsAdded;
	}

void CEmTubeSearchViewContainer::ApplyRatingStarsL( CVideoEntry* aEntry )
	{
#define FRAME 1
#define MARGIN_X 1
#define MARGIN_Y 2
#define SPACE 1

#define USE_FRAME

	CFbsBitmap* scaledBitmap = aEntry->ScaledBitmap();
	TInt dstWidth = scaledBitmap->SizeInPixels().iWidth, dstHeight = scaledBitmap->SizeInPixels().iHeight;

	TInt width = dstWidth - ( (4 * SPACE) + (2 * MARGIN_X) );
	TSize size( width / 5, width / 5 );
	AknIconUtils::SetSize( iStarBitmapWhite, size);

#ifndef USE_FRAME
	AknIconUtils::SetSize( iStarBitmapRed, size);
	AknIconUtils::SetSize( iStarBitmapGray, size);
#else
	size.iHeight -= ( FRAME * 2 );
	size.iWidth -= ( FRAME * 2 );
	AknIconUtils::SetSize( iStarBitmapRed, size);
	AknIconUtils::SetSize( iStarBitmapGray, size);
#endif

	CFbsBitGc* gc;
	CFbsBitmapDevice* device = CFbsBitmapDevice::NewL( scaledBitmap );
	User::LeaveIfError( device->CreateContext( gc ) );

	size = iStarBitmapRed->SizeInPixels();
	TSize size1 = iStarBitmapWhite->SizeInPixels();
	TPoint point( MARGIN_X, dstHeight - (size1.iHeight + MARGIN_Y) );
   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
   	TRect sourceRect1( TPoint( 0, 0 ), TSize( size1.iWidth, size1.iHeight ) );

   	TInt val = (TInt)aEntry->AverageRating();
   	TInt p = (TInt)( (aEntry->AverageRating() - (float)(val)) * 100.0 );

   	TInt i;
   	for( i=0;i<val;i++ )
   		{
#ifndef USE_FRAME
   		gc->BitBltMasked( point, iStarBitmapRed, sourceRect1, iStarMaskRed, ETrue );
   		point.iX += ( MARGIN_X + size1.iWidth - 2);
#else
   		gc->BitBltMasked( point, iStarBitmapWhite, sourceRect1, iStarMaskWhite, ETrue );
   		point.iX += 2;
   		point.iY += 2;
   		gc->BitBltMasked( point, iStarBitmapRed, sourceRect, iStarMaskRed, ETrue );
 		point.iY -= 2;
   		point.iX += ( MARGIN_X + size1.iWidth - 2);
#endif
   		}

	if( val < 5 )
		{
#ifndef USE_FRAME
	   	gc->BitBltMasked( point, iStarBitmapGray, sourceRect1, iStarMaskGray, ETrue );
	   	TRect sourceRectTmp( TPoint( 0, 0 ), TSize( (size.iWidth * p) / 100, size.iHeight ) );
	   	gc->BitBltMasked( point, iStarBitmapRed, sourceRectTmp, iStarMaskRed, ETrue );
	   	point.iX += ( MARGIN_X + size1.iWidth - 2);
#else
	   	gc->BitBltMasked( point, iStarBitmapWhite, sourceRect1, iStarMaskWhite, ETrue );
		point.iX += 2;
		point.iY += 2;
		gc->BitBltMasked( point, iStarBitmapGray, sourceRect, iStarMaskGray, ETrue );
	  	TRect sourceRectTmp( TPoint( 0, 0 ), TSize( (size.iWidth * p) / 100, size.iHeight ) );
		gc->BitBltMasked( point, iStarBitmapRed, sourceRectTmp, iStarMaskRed, ETrue );
		point.iY -= 2;
		point.iX += ( MARGIN_X + size1.iWidth - 2);
#endif

		val++;
	   	for( i=val;i<5;i++ )
	   		{
#ifndef USE_FRAME
	  		gc->BitBltMasked( point, iStarBitmapGray, sourceRect1, iStarMaskGray, ETrue );
	   		point.iX += ( MARGIN_X + size1.iWidth - 2);
#else
	   		gc->BitBltMasked( point, iStarBitmapWhite, sourceRect1, iStarMaskWhite, ETrue );
	   		point.iX += 2;
	   		point.iY += 2;
	   		gc->BitBltMasked( point, iStarBitmapGray, sourceRect, iStarMaskGray, ETrue );
	  		point.iY -= 2;
	   		point.iX += ( MARGIN_X + size1.iWidth - 2);
#endif
	   		}
		}

	if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
		{
		if( iAppUi->FindInFavoritesL( aEntry ) != KErrNotFound )
			{
			TInt width = dstWidth / 5;
			TSize size( width , width );
			AknIconUtils::SetSize( iHeartBitmap, size);

			size = iHeartBitmap->SizeInPixels();
			TPoint point( 2 , 2 );
		   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
			gc->BitBltMasked( point, iHeartBitmap, sourceRect, iHeartMask, ETrue );
			}
		}

   	delete gc;
	delete device;
}

void CEmTubeSearchViewContainer::RescaleIconsL()
	{
	iUi->RefreshL( iView.CurrentItemIndex() );
	iUi->ResetIcons();

	CFbsBitmap *bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubNo_image, EMbmOpenvideohubNo_image_mask);
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask), ETrue );
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubEmpty, EMbmOpenvideohubEmpty_mask);
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask), ETrue );
	AknIconUtils::CreateIconL( bitmap, mask, KSystemIconFile, EMbmAvkonQgn_prop_mmc_memc_large, EMbmAvkonQgn_prop_mmc_memc_large_mask );
	iUi->AppendIconL( CGulIcon::NewL( bitmap, mask ), ETrue );

	RPointerArray<CVideoEntry>& entries = iAppUi->SearchEntries();

	TInt width, height;
	iUi->GetIconSize( width, height );

	TInt idx = FIRST_VIDEO_ICON_IDX;
	TInt j = 0;
	for(TInt i=0;i<entries.Count();i++)
		{
		CVideoEntry* entry = entries[i];

		if( (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries) &&
			 (entry->PluginUid() != iAppUi->PluginManager().Uid()) )
			continue;

		if( IsEntryVisibleL( entry ) )
			{
			if( entry->ImageLoaded() )
				{
				CGulIcon* icon = CGulIcon::NewL();
				if( width && height )
					icon->SetBitmap( entry->ScaledBitmapL( width, height ) );
				else
					icon->SetBitmap( entry->ScaledBitmap() );
	
				if( iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySavedEntries )
					ApplyRatingStarsL( entry );
				icon->SetBitmapsOwnedExternally( ETrue );
				iUi->AppendIconL( icon, EFalse );
				iUi->ChangeItemIconIdxL( j, idx );
				idx++;
				j++;
				}
			else
				{
				iUi->ChangeItemIconIdxL( j, 0 ); //no image icon
				j++;
				}
			}
		}
	}

void CEmTubeSearchViewContainer::DownloadL( )
	{
	TInt current = iView.CurrentToActual();
	CVideoEntry* entry = iAppUi->SearchEntries()[ current ];

    if( iAppUi->TransferManager().EntryAdded( entry->Url() ) )
        {
        HBufC* informationText = StringLoader::LoadLC( R_VIDEO_ALREADY_ADDED_TXT );
        CAknWarningNote* informationDlg = new ( ELeave ) CAknWarningNote(ETrue);
        informationDlg->ExecuteLD( *informationText );
        CleanupStack::PopAndDestroy( informationText );
        return;
        }

	TInt ret;
	TFileName fileName;

	TBool done = EFalse;
	TBool memDialog = ETrue;

	if( iAppUi->SaveLoadDirectory().Length() )
		memDialog = EFalse;

	while( !done )
		{

		if( memDialog )
			{
			fileName.Copy( KNullDesC() );

			CAknMemorySelectionDialog::TMemory memory = CAknMemorySelectionDialog::EPhoneMemory;
			CAknMemorySelectionDialog* memDlg = CAknMemorySelectionDialog::NewL( ECFDDialogTypeSelect, R_MEMORY_SELECTION_DIALOG );

			ret = memDlg->ExecuteL( memory );
			if( ret )
				{
				if( memory == CAknMemorySelectionDialog::EPhoneMemory )
					fileName.Copy( _L("C:\\") );
				else if( memory == CAknMemorySelectionDialog::EMemoryCard )
					fileName.Copy( _L("E:\\") );
				}
			delete memDlg;
			}
		else
			{
		    TParsePtrC parse( iAppUi->SaveLoadDirectory() );
			fileName.Copy( parse.Drive() );
			fileName.Append( _L("\\") );
			ret = 1;
			}

		memDialog = ETrue;

		if( ret )
			{
			CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL( ECFDDialogTypeSave, R_FILE_SELECTION_DIALOG );

			if( iAppUi->SaveLoadDirectory().Length() )
				{
			    TParsePtrC parse( iAppUi->SaveLoadDirectory() );
			    TPtrC path = parse.Path();
			    TPtrC path2 = path.Right( path.Length() - 1 );
			    dlg->SetDefaultFolderL( path.Right( path.Length() - 1 ) );
				}

			ret = dlg->ExecuteL( fileName );
			delete dlg;

			if( ret )
				{
				TFileName name;
				CAknFileNamePromptDialog* dlg = CAknFileNamePromptDialog::NewL();

				dlg->SetPathL( fileName );

				HBufC* tmp = iAppUi->ConvertFileNameL( entry->MediaTitle() );
				CleanupStack::PushL( tmp );
				if( tmp->Length() > KMaxFileName )
					name.Copy( tmp->Mid( 0, KMaxFileName ) );
				else
					name.Copy( *tmp );
				CleanupStack::PopAndDestroy( tmp );

				ret = dlg->ExecuteL( name );
				delete dlg;

				if( !ret )
					fileName.Copy( KNullDesC() );
				else
					fileName.Append( name );

				done = ETrue;
				}
			}
		else
			{
			done = ETrue;
			}
		}

	if( fileName.Length() )
		{
		iAppUi->SetSaveLoadDirectory( fileName );

		TFileName ext;

		TParsePtrC parse( fileName );
		ext = parse.Ext();

		if( ext.Compare( _L(".flv") ) )
			fileName.Append( _L(".flv") );

		entry->SetSavedFileName( fileName );

        TBool processing = iAppUi->TransferManager().Processing();
        TBool allProcessed = iAppUi->TransferManager().AllProcessed();

        if( (!processing) && (allProcessed) )
        {
            iAppUi->TransferManager().SetTMStarted( ETrue );
            iAppUi->TransferManager().RegisterObserverL( *this );

    		iProgressDialog = new ( ELeave ) CAknProgressDialog( reinterpret_cast<CEikDialog**> ( &iProgressDialog ), ETrue );
    		iProgressDialog->SetCallback( this );
    		iProgressDialog->PrepareLC( R_EMTV_DOWNLOAD_FILE_PROGRESS_DIALOG );
    		iProgressInfo = iProgressDialog->GetProgressInfoL();
    		iProgressDialog->RunLD();
        }
        else
        {
			HBufC* message = StringLoader::LoadLC( R_VIDEO_ADDED_TO_TM_TXT );
            CAknInformationNote* informationNote = new (ELeave) CAknInformationNote;
            informationNote->ExecuteLD( *message );
			CleanupStack::PopAndDestroy( message );
        }

        iAppUi->TransferManager().AddToQueueL( entry->MediaTitle(), entry->Url(), fileName );
		}
	}

void CEmTubeSearchViewContainer::RequestFinishedL( TInt /*aState*/, TDesC8& /*aResponseBuffer*/ )
	{
	}

void CEmTubeSearchViewContainer::RequestCanceledL( TInt /*aState*/ )
	{
	if( !iFromDialog )
		ProgressComplete();
	}

TBool CEmTubeSearchViewContainer::CheckDiskSpaceL( const TDesC& aFileName, TInt aSize )
	{
	return iAppUi->CheckDiskSpaceL( aFileName, aSize );
	}

void CEmTubeSearchViewContainer::ShowErrorNoteL( TInt aResourceId )
	{
	iAppUi->ShowErrorNoteL( aResourceId );
	}

void CEmTubeSearchViewContainer::ShowErrorNoteL( const TDesC& aText )
	{
	iAppUi->ShowErrorNoteL( aText );
	}

void CEmTubeSearchViewContainer::ProgressStart( TInt aCompleteSize )
	{
	if( iProgressDialog )
		iProgressInfo->SetFinalValue( aCompleteSize );
	}

void CEmTubeSearchViewContainer::ProgressUpdate( TInt aCurrent, TInt /*aDownloadSpeed*/ )
	{
	if( iProgressDialog )
		iProgressInfo->SetAndDraw( aCurrent );
	}

void CEmTubeSearchViewContainer::ProgressComplete()
	{
	if( iProgressDialog )
		{
		iProgressDialog->ProcessFinishedL();
		delete iProgressDialog;
		iProgressDialog = NULL;
		}

    iAppUi->TransferManager().UnRegisterObserver( *this );
	}

void CEmTubeSearchViewContainer::DialogDismissedL( TInt aButtonId )
	{
//TODO -> only allow to hide progress if ENABLE_TRANSFER_MANAGER is set
	if ( aButtonId == -1 )
		{
		iFromDialog = ETrue;
		TInt idx = iAppUi->TransferManager().StopL();
		iAppUi->TransferManager().RemoveFromQueueL( idx );
		iFromDialog = EFalse;
		}

	iAppUi->TransferManager().UnRegisterObserver( *this );
	}

TBool CEmTubeSearchViewContainer::IsEntryVisibleL( CVideoEntry* aEntry )
	{
	TBool res = iUi->IsEntryVisibleL( aEntry->MediaTitle() );
	return res;
	}

void CEmTubeSearchViewContainer::HandleCommandL( TObserverEvent aEvent )
	{
	switch( aEvent )
		{
		case MHandleCommandObserver::EHandleItemClicked:
			{
			if( IsListboxLineMore() )
				{
				SaveCurrentPosition();
				StartProgressBarL( EFalse );
				iAppUi->HandleCommandL( EMTVSearchNextCommand );
				}
			else
				RunItemDialogL();
			}
		break;

		case MHandleCommandObserver::EHandleItemSelected:
			{
#ifdef __S60_50__

			CAknToolbar *toolBar = iAppUi->CurrentFixedToolbar();

			if( IsListboxLineMore() || (CurrentItemIndex() == -1) )
				toolBar->SetItemDimmed( EMTVSearchViewPlayCommand, ETrue, EFalse );
			else
				toolBar->SetItemDimmed( EMTVSearchViewPlayCommand, EFalse, EFalse );

			toolBar->DrawNow();
#endif
			}
		break;

		case MHandleCommandObserver::EHandleItemDeleted:
			{
			if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries )
				RemoveCurrentItemFromFavsL();
			else if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySavedEntries )
				iView.HandleCommandL( EMTVDeleteClipCommand );
			}
		break;

		case MHandleCommandObserver::EHandleTabChanged:
			{
			if((iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries))
				break;

			TInt which = iUi->ActiveTab();
			ChangeTabL( which );
			}
		break;

		case MHandleCommandObserver::EHandleKeyLeft:
			{
			if((iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries) && iProgressEnabled)
				break;

			TInt which = iUi->ActiveTab();
			if( which > 0 )
				{
				which--;
				ChangeTabL( which );
				}
			}
		break;

		case MHandleCommandObserver::EHandleKeyRight:
			{
			if((iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries) && iProgressEnabled)
				break;

			TInt which = iUi->ActiveTab();
			if( which < (iPluginsCount - 1) )
				{
				which++;
				ChangeTabL( which );
				}
			}
		break;

		case MHandleCommandObserver::EHandleCreateItemList:
			{
			DisplaySearchResultsL();
			}
		break;

		default:
			break;
		}
	}

MItemDrawer::TItemState CEmTubeSearchViewContainer::DrawItemL( CFbsBitmap* aDstBitmap, CFbsBitGc* aDstBitmapGc,
            TSize aNewSize, TPoint aDstPoint, TBool aShowText, TBool aSelected, CUiItem* aItem,
            TBool aScrollText )
    {
    MItemDrawer::TItemState result = EItemFit;

    if( aShowText )
        {
    	aDstBitmapGc->UseFont( iCustomUiFont );

    	TInt fontHeight = iCustomUiFont->HeightInPixels();
    	TInt fontDescent = iCustomUiFont->DescentInPixels();

        if( aSelected )
            {
        	aDstBitmapGc->SetBrushColor( TRgb(15,80,122) );
        	aDstBitmapGc->SetPenColor( TRgb(15,80,122) );
        	aDstBitmapGc->SetBrushStyle( CGraphicsContext::ESolidBrush );
        	aDstBitmapGc->SetPenStyle(CGraphicsContext::ESolidPen);

            // big blue box
            TRect boxRect;
            if( IsListboxLineMore() )
                boxRect = TRect( 0, (Rect().Height() / 2) - fontHeight, Rect().Width(), (Rect().Height() / 2) + fontHeight );
            else
                boxRect = TRect( 0, aDstPoint.iY - 5, Rect().Width(), aDstPoint.iY + 5 + aNewSize.iHeight );

            aDstBitmapGc->DrawRect(boxRect);

            // light blue frame for blue box
            aDstBitmapGc->SetBrushStyle( CGraphicsContext::ENullBrush );
        	aDstBitmapGc->SetPenColor( TRgb(110,190,242) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iTl.iY ), TPoint( Rect().Width(), boxRect.iTl.iY ) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iTl.iY - 1 ), TPoint( Rect().Width(), boxRect.iTl.iY - 1) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iBr.iY), TPoint( Rect().Width(), boxRect.iBr.iY ) );
            aDstBitmapGc->DrawLine( TPoint( 0, boxRect.iBr.iY + 1), TPoint( Rect().Width(), boxRect.iBr.iY + 1) );

    	    aDstBitmapGc->SetPenColor( KCustomUiSelectedTextColor );
            TInt iconMargin = (TInt)( aNewSize.iWidth * 1.2 );

            if( IsListboxLineMore() )
                {
                aDstBitmapGc->DiscardFont();
                CFont* font = UiItemGfx::CreateFontL( UiItemGfx::EFontLarge );
            	aDstBitmapGc->UseFont( font );

                TInt baseline = boxRect.Height() / 2 + font->AscentInPixels() / 2;
    	        UiItemGfx::DrawText( aDstBitmapGc, aItem->FirstLine(), KCustomUiSelectedTextColor, boxRect, baseline, ETrue, CGraphicsContext::ECenter, 0 );
                CEikonEnv::Static()->ReleaseScreenFont( font );
                }
            else
                {
                if( !aScrollText )
                    {
                    iScrollAmount = 0;
                    iScrollAmount2 = 0;
                    }

                // title
                TRect titleRect( aDstPoint.iX + iconMargin, aDstPoint.iY, Rect().Width() - 5,
                    aDstPoint.iY + fontHeight + 2 );

                TPtrC title( aItem->FirstLine() );
                RBuf titleDots;

                TInt titleWidth = iCustomUiFont->TextWidthInPixels( title );
                if( (titleRect.Width() - 5) < (titleWidth - iScrollAmount) )
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

                        UiItemGfx::TextDotsL( title, titleRect.Width(), iCustomUiFont, titleDots );
                        title.Set( titleDots );
                        }
                    }
                else if( iScrollAmount )
                    {
                    result = EItemScrolled;
                    }

                aDstBitmapGc->SetClippingRect( titleRect );
                titleRect.iTl.iX -= iScrollAmount;

    	        UiItemGfx::DrawText( aDstBitmapGc, title, KCustomUiSelectedTextColor, titleRect, fontHeight - fontDescent, ETrue );

                aDstBitmapGc->CancelClippingRect();
                titleDots.Close();

                if( aItem->PrivateData() )
                    {
            	    aDstBitmapGc->SetPenColor( KCustomUiTextColor );
                    CVideoEntry* entry = static_cast<CVideoEntry*>( aItem->PrivateData() );

                	TSize logoSize( fontHeight + fontDescent, fontHeight + fontDescent );
                	TInt bottomLine = aDstPoint.iY + aNewSize.iHeight;

                    // line
                    aDstBitmapGc->DrawLine( TPoint( aDstPoint.iX + iconMargin, bottomLine - fontDescent - fontHeight - 2 ),
                        TPoint( Rect().Width() - 5 , bottomLine - fontDescent - fontHeight - 2 ) );

                    TRect durationRect( aDstPoint.iX, bottomLine - fontHeight,
                        Rect().Width(), bottomLine + 2 );

                	// video length
                	TBuf<128> duration;
                	TTime ttime(0);
                	ttime += (TTimeIntervalSeconds) entry->Duration();
                	ttime.FormatL( duration, *iTimeFormatString);

    		        UiItemGfx::DrawText( aDstBitmapGc, duration, KCustomUiTextColor, durationRect, fontHeight - fontDescent + 1, ETrue, CGraphicsContext::ERight, 5 );

                    // video length logo
                    TInt durationTextLength = iCustomUiFont->TextWidthInPixels( duration );
                    TPoint durationLogoPoint( Rect().Width() - 5 - durationTextLength - logoSize.iWidth - 2,
                        durationRect.iTl.iY + 2 - fontDescent );
                    AknIconUtils::SetSize( iTimeLogo, logoSize );
    			   	TRect timeLogoRect( TPoint( 0, 0 ), TSize( logoSize.iWidth, logoSize.iHeight ) );
                    aDstBitmapGc->BitBltMasked( durationLogoPoint, iTimeLogo, timeLogoRect, iTimeMask, ETrue );

                    if( entry->AuthorName().Length() )
                        {
                        // author name
                        TRect authorRect( aDstPoint.iX + iconMargin + logoSize.iWidth + 2, bottomLine - fontHeight,
                            Rect().Width() - 5 - durationTextLength - 2 - logoSize.iWidth -2, bottomLine + 2 );

                        TPtrC author( entry->AuthorName() );
                        RBuf authorDots;
                        MItemDrawer::TItemState resultAuthor = EItemFit;

                        TInt authorNameWidth = iCustomUiFont->TextWidthInPixels( author );
                        if( authorRect.Width() < (authorNameWidth - iScrollAmount2) )
                            {
                            if( aScrollText )
                                {
                                resultAuthor  = EItemScrolling;
                                iScrollAmount2++;
                                }
                            else
                                {
                                resultAuthor  = EItemLarger;
                                iScrollAmount2 = 0;

                                UiItemGfx::TextDotsL( author, authorRect.Width(), iCustomUiFont, authorDots );
                                author.Set( authorDots );
                                }
                            }
                        else if( iScrollAmount2 )
                            {
                            resultAuthor = EItemScrolled;
                            }

                        aDstBitmapGc->SetClippingRect( authorRect );
                        authorRect.iTl.iX -= iScrollAmount2;

            	        UiItemGfx::DrawText( aDstBitmapGc, author, KCustomUiTextColor, authorRect, fontHeight - fontDescent + 1, ETrue );

                        aDstBitmapGc->CancelClippingRect();
                        authorDots.Close();

                        switch( resultAuthor )
                            {
                            case MItemDrawer::EItemScrolled:
                                {
                                if( result != MItemDrawer::EItemScrolling )
                                    result = EItemScrolled;
                                break;
                                }

                            case MItemDrawer::EItemScrolling:
                                {
                                result = EItemScrolling;
                                break;
                                }

                            case MItemDrawer::EItemLarger:
                                {
                                result = EItemLarger;
                                break;
                                }

                            default:
                                break;
                            }

                        // author name logo
                        TPoint authorLogoPoint( aDstPoint.iX  + iconMargin, durationRect.iTl.iY + 2 - fontDescent );
                        AknIconUtils::SetSize( iAuthorLogo, logoSize );
        			   	TRect authorLogoRect( TPoint( 0, 0 ), TSize( logoSize.iWidth, logoSize.iHeight ) );
                        aDstBitmapGc->BitBltMasked( authorLogoPoint, iAuthorLogo, authorLogoRect, iAuthorMask, ETrue );
                        }

                    if( result == MItemDrawer::EItemScrolled )
                        {
                        iScrollAmount = 0;
                        iScrollAmount2 = 0;
                        }
                    }
                }
            }
        else
            {
    	    aDstBitmapGc->SetPenColor( KCustomUiTextColor );

            TInt iconMargin = (TInt)( aNewSize.iWidth * 1.2 );
            TRect rect( aDstPoint.iX + iconMargin, aDstPoint.iY, Rect().Width(), aDstPoint.iY + aNewSize.iHeight );
            TInt baseline = rect.Height() / 2 + iCustomUiFont->AscentInPixels() / 2;

            TPtrC title( aItem->FirstLine() );
            RBuf titleDots;

            TInt textWidth = iCustomUiFont->TextWidthInPixels( title );
            if( rect.Width() < textWidth )
                {
                if( !aScrollText )
                    {
                    TInt count = iCustomUiFont->TextCount( title, rect.Width() ) - 3;
                    titleDots.Create( count + 3 );
                    titleDots.Copy( title.Left( count ) );
                    titleDots.Append( _L("...") );
                    title.Set( titleDots );
                    }
                }

	        UiItemGfx::DrawText( aDstBitmapGc, title, KCustomUiTextColor, rect, baseline, ETrue );

            titleDots.Close();
            }

        aDstBitmapGc->DiscardFont();
        }

    UiItemGfx::BltIconScale( aDstBitmap, iUi->Icon( aItem->IconIdx() ), aNewSize, aDstPoint );

    return result;
    }

void CEmTubeSearchViewContainer::SetNaviPaneTextL()
	{
	HBufC* modeTxt = StringLoader::LoadLC( iAppUi->SearchNaviPaneId() );
	RBuf paneTxt;
	CleanupClosePushL( paneTxt );
    paneTxt.Create( modeTxt->Length() );
    paneTxt.Copy( *modeTxt );

	if( !iAppUi->S60Ui() && (iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries) )
    	{
        paneTxt.ReAlloc( paneTxt.Length() + iAppUi->PluginManager().Plugin()->Name().Length() + 3 );
        paneTxt.Insert( 0, _L(" - ") );
        paneTxt.Insert( 0, iAppUi->PluginManager().Plugin()->Name() );
    	}

	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( paneTxt );
	iNaviLabelDecorator->DrawDeferred();

	CleanupStack::PopAndDestroy( &paneTxt );
	CleanupStack::PopAndDestroy( modeTxt );
	}

void CEmTubeSearchViewContainer::ChangeTabL( TInt aWhich )
	{
	SaveCurrentPosition();

	iAppUi->CancelDownloadImageL();
	iUi->SetActiveTab( aWhich );
	iAppUi->PluginManager().SelectPlugin( aWhich );
	iAppUi->SetTitlePaneTextL( iAppUi->PluginManager().Plugin()->Name() );
	UpdateToolbar();

	if( !DisplaySearchResultsL() && iAppUi->GotFeature() )
		iTimer->After( 1000000, 1 );
	}

void CEmTubeSearchViewContainer::ClearToolbar()
	{
#ifdef __S60_50__
	CAknToolbar *toolBar = iAppUi->CurrentFixedToolbar();

	toolBar->HideItem( 1313, EFalse, ETrue ); // empty one
	toolBar->HideItem( 1314, EFalse, ETrue ); // empty one
	toolBar->HideItem( 1315, EFalse, ETrue ); // empty one

	toolBar->SetItemDimmed( 1313, ETrue, ETrue ); // empty one
	toolBar->SetItemDimmed( 1314, ETrue, ETrue ); // empty one
	toolBar->SetItemDimmed( 1315, ETrue, ETrue ); // empty one
#endif
	}

void CEmTubeSearchViewContainer::UpdateToolbar()
	{
#ifdef __S60_50__
	CAknToolbar *toolBar = iAppUi->CurrentFixedToolbar();
	toolBar->SetDimmed( EFalse );

	if(iAppUi->SearchDisplayMode() != CEmTubeAppUi::ESearchDisplaySearchEntries)
		{
		toolBar->HideItem( EMTVSearchViewTabToLeftCommand, ETrue, ETrue );
		toolBar->HideItem( EMTVSearchViewTabToRightCommand, ETrue, ETrue );
		toolBar->SetItemDimmed( 1313, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1314, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1315, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1316, EFalse, ETrue ); // empty one
		toolBar->SetItemDimmed( 1316, ETrue, ETrue ); // empty one
		}
	else if( iAppUi->PluginManager().Plugins().Count() > 1 )
		{
		toolBar->HideItem( EMTVSearchViewTabToLeftCommand, EFalse, ETrue );
		toolBar->HideItem( EMTVSearchViewTabToRightCommand, EFalse, ETrue );
		toolBar->HideItem( 1313, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1314, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1315, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1316, ETrue, ETrue ); // empty one

		// left arrow
		if( !iUi->ActiveTab() || iProgressEnabled )
			toolBar->SetItemDimmed( EMTVSearchViewTabToLeftCommand, ETrue, ETrue );
		else
			toolBar->SetItemDimmed( EMTVSearchViewTabToLeftCommand, EFalse, ETrue );

		// right arrow
		if( (iUi->ActiveTab() == (iAppUi->PluginManager().Plugins().Count() - 1)) || iProgressEnabled )
			toolBar->SetItemDimmed( EMTVSearchViewTabToRightCommand, ETrue, ETrue );
		else
			toolBar->SetItemDimmed( EMTVSearchViewTabToRightCommand, EFalse, ETrue );
		}
	else // when there is only one plugin, hide arrows
		{
		toolBar->HideItem( EMTVSearchViewTabToLeftCommand, ETrue, ETrue );
		toolBar->HideItem( EMTVSearchViewTabToRightCommand, ETrue, ETrue );
		toolBar->SetItemDimmed( 1313, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1314, ETrue, ETrue ); // empty one
		toolBar->HideItem( 1315, ETrue, ETrue ); // empty one
		toolBar->SetItemDimmed( 1316, ETrue, ETrue ); // empty one
		}

	// play button
	if( IsListboxLineMore() || (CurrentItemIndex() == -1) )
		toolBar->SetItemDimmed( EMTVSearchViewPlayCommand, ETrue, ETrue );
	else
		toolBar->SetItemDimmed( EMTVSearchViewPlayCommand, EFalse, ETrue );

	toolBar->DrawNow();
#endif
	}

void CEmTubeSearchViewContainer::ShowToolbar( TBool aShow )
	{
#ifdef __S60_50__
	CAknToolbar *toolBar = iAppUi->CurrentFixedToolbar();
	toolBar->SetToolbarVisibility( aShow );
	toolBar->DrawNow();
#endif
	}

#ifdef __S60_50__

void CEmTubeSearchViewContainer::ProcessCommandL( TInt aCommandId )
	{
	HandleItemDialogResponseL( aCommandId );
	}

void CEmTubeSearchViewContainer::SetEmphasis( CCoeControl* /*aMenuControl*/, TBool /*aEmphasis*/ )
	{
	}

#endif

void CEmTubeSearchViewContainer::RunItemDialogL()
	{
	if( CurrentItemIndex() != KErrNotFound )
		{
		RArray<TPopupCommand> commands;
		CleanupClosePushL( commands );
		RPointerArray<HBufC> texts;
		CleanupClosePushL( texts );

		ItemDialogCommandsL( texts, commands );

		CAknSinglePopupMenuStyleListBox* plist = new(ELeave) CAknSinglePopupMenuStyleListBox;
		CleanupStack::PushL(plist);

	    CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_MENU_LIST, AknPopupLayouts::EMenuWindow);
	    CleanupStack::PushL(popupList);

	    plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
	    plist->CreateScrollBarFrameL(ETrue);
	    plist->ScrollBarFrame()->SetScrollBarVisibilityL(
	                               CEikScrollBarFrame::EOff,
	                               CEikScrollBarFrame::EAuto);

	    MDesCArray* itemList = plist->Model()->ItemTextArray();
	    CDesCArray* items = (CDesCArray*) itemList;

	    for( TInt i=0 ; i < texts.Count() ; i++ )
			items->AppendL( *texts[i] );

	    popupList->SetTitleL( KNullDesC() );

	    TInt popupOk = popupList->ExecuteLD();
	    if(popupOk)
	        HandleItemDialogResponseL( commands[plist->CurrentItemIndex()] );

	    CleanupStack::Pop( popupList );
	    CleanupStack::PopAndDestroy( plist );
	    texts.ResetAndDestroy();
	    CleanupStack::PopAndDestroy( &texts );
	    CleanupStack::PopAndDestroy( &commands );
		}
	}

void CEmTubeSearchViewContainer::RunFloatingItemDialogL( TPoint aWhere )
	{
	if(CurrentItemIndex() != KErrNotFound)
		{
#ifdef __S60_50__

		delete iPopup;

		iPopup = CAknStylusPopUpMenu::NewL( this, aWhere );

		RArray<TPopupCommand> commands;
		CleanupClosePushL( commands );
		RPointerArray<HBufC> texts;
		CleanupClosePushL( texts );

		ItemDialogCommandsL( texts, commands );

	    for( TInt i=0 ; i < texts.Count() ; i++ )
			iPopup->AddMenuItemL( *texts[i], commands[i] );

	    texts.ResetAndDestroy();
	    CleanupStack::PopAndDestroy( &texts );
	    CleanupStack::PopAndDestroy( &commands );

		iPopup->ShowMenu();

#endif
		}
	}

void CEmTubeSearchViewContainer::ItemDialogCommandsL( RPointerArray<HBufC>& aTexts, RArray<TPopupCommand>& aCommands )
	{
	HBufC* txt = StringLoader::LoadLC( R_POPUP_VIEW_TXT );
	aCommands.Append( ECommandView );
	aTexts.AppendL( txt );
	CleanupStack::Pop( txt );

	txt = StringLoader::LoadLC( R_POPUP_DETAILS_TXT );
	aCommands.AppendL( ECommandDetails );
	aTexts.AppendL( txt );
	CleanupStack::Pop( txt );

#ifdef ENABLE_PLAYLISTS
	txt = StringLoader::LoadLC( R_POPUP_ADD_TO_PLAYLIST_TXT );
	aCommands.AppendL( ECommandAddToPlaylist );
	aTexts.AppendL( txt );
	CleanupStack::Pop( txt );
#endif

	if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries ||
		iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplayFavEntries )
		{
		if( iAppUi->PluginManager().Plugin()->Capabilities() & KCapsAllowSavingVideos )
			{
			txt = StringLoader::LoadLC( R_POPUP_DOWNLOAD_TXT );
			aCommands.Append( ECommandDownload );
			aTexts.AppendL( txt );
			CleanupStack::Pop( txt );
			}
		}

	iEntry = iAppUi->SearchEntries()[ iView.CurrentToActual() ];

	if( iAppUi->SearchDisplayMode() == CEmTubeAppUi::ESearchDisplaySearchEntries )
		{
		if( iAppUi->FindInFavoritesL( iEntry ) == KErrNotFound )
			{
			txt = StringLoader::LoadLC( R_POPUP_ADD_TO_FAVORITES_TXT );
			aCommands.Append( ECommandAddToFavs );
			aTexts.AppendL( txt );
			CleanupStack::Pop( txt );
			}
		else
			{
			txt = StringLoader::LoadLC( R_POPUP_REMOVE_FROM_FAVORITES_TXT );
			aCommands.Append( ECommandRemoveFromFavs );
			aTexts.AppendL( txt );
			CleanupStack::Pop( txt );
			}
		}
	}

void CEmTubeSearchViewContainer::HandleItemDialogResponseL( TInt aCommand )
	{
    switch( aCommand )
    	{
		case ECommandView:
			SaveCurrentPosition();
			iView.HandleCommandL( EMTVSearchViewPlayCommand );
		break;

		case ECommandDetails:
			iView.HandleCommandL( EMTVSearchViewDetailsCommand );
		break;

		case ECommandDownload:
			iView.HandleCommandL( EMTVSearchViewDownloadCommand );
		break;

		case ECommandAddToPlaylist:
			iView.HandleCommandL( EMTVAddToPlaylistCommand );
		break;

		case ECommandAddToFavs:
		case ECommandRemoveFromFavs:
			if( iAppUi->FindInFavoritesL( iEntry ) == KErrNotFound )
				{
				AddCurrentItemToFavsL();
				}
			else
				{
				RemoveCurrentItemFromFavsL();
				}
		break;

		default:
			break;
    	}
	}
HBufC* CEmTubeSearchViewContainer::FindBoxText()
	{
	if( iUi )
		return iUi->FindBoxText();
	else
		return NULL;
	}

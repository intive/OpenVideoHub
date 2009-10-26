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

#ifndef EMTUBE_SEARCHVIEW_CONTAINER_H
#define EMTUBE_SEARCHVIEW_CONTAINER_H

#include <coecntrl.h>
#include <aknlists.h> // list
#include <aknnavi.h>
#include <aknnavide.h>
#include <e32std.h>
#include <aknprogressdialog.h>
#include <eikprogi.h>
#include <coecobs.h> //MCoeControlObserver

#ifdef __S60_50__
#include <aknstyluspopupmenu.h>
#endif

#include "emtubeSearchView.h"
#include "emTubeMUiInterface.h"
#include "emTubeMItemDrawer.h"

class CEmTubeAppUi;
class CAknSearchField;
class CUiItem;
class CAknStylusPopUpMenu;

class CEmTubeSearchViewContainer : public CCoeControl, public MTimeOutObserver,
									public MProgressObserver, public MHttpEngineObserver,
									public MProgressDialogCallback, public MHandleCommandObserver,
									public MItemDrawer
#ifdef __S60_50__
									,public MEikMenuObserver
#endif
	{
private:
	enum TPopupCommand
		{
		ECommandView = 0,
		ECommandDetails,
		ECommandAddToPlaylist,
		ECommandDownload,
		ECommandAddToFavs,
		ECommandRemoveFromFavs
		};

public:
	static CEmTubeSearchViewContainer* NewL(CEmTubeSearchView& aView, const TRect& aRect, const TDesC& aFindBoxText );
	static CEmTubeSearchViewContainer* NewLC(CEmTubeSearchView& aView, const TRect& aRect, const TDesC& aFindBoxText );
	~CEmTubeSearchViewContainer();

private:
	CEmTubeSearchViewContainer(CEmTubeSearchView& aView);
	void ConstructL(const TRect& aRect, const TDesC& aFindBoxText );
	void SizeChanged();

public:
	void ImageLoadedL(TInt aIndex);
	void CancelProgressL();
	void SaveCurrentPosition();
	void StartProgressBarL( TBool aClear );
	TInt CurrentItemIndex();
	void UpdateNaviPane( TInt aPercentage );
	void DownloadL( );

public: // MProgressDialogCallback
	void DialogDismissedL( TInt aButtonId );

public: //from MHttpEngineObserver
	void RequestFinishedL( TInt aState, TDesC8& aResponseBuffer );
	void RequestCanceledL( TInt aState );
	TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize );
	void ShowErrorNoteL( TInt aResourceId );
	void ShowErrorNoteL( const TDesC& aText );

public: //from MProgressObserver
	void ProgressStart( TInt aCompleteSize );
	void ProgressUpdate( TInt aCurrent, TInt aDownloadSpeed );
	void ProgressComplete();

#ifdef __S60_50__

public: //from MEikMenuObserver
    void ProcessCommandL( TInt aCommandId );
    void SetEmphasis( CCoeControl* aMenuControl,TBool aEmphasis );

#endif

protected: // from CCoeControl
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void HandleResourceChange(TInt aType);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
	void GetHelpContext( TCoeHelpContext& aContext ) const;

public:  // from MTimeOutObserver
	void TimerExpired( TInt aMode );

public:  // from CCoeControl
	void Draw(const TRect& aRect) const;

public: // from MHandleCommandObserver
    void HandleCommandL( TObserverEvent aEvent );

public: // from MIemDrawer;
    TItemState DrawItemL( CFbsBitmap* aDstBitmap, CFbsBitGc* aDstBitmapGc, TSize aNewSize,
                          TPoint aDstPoint, TBool aShowText, TBool aSelected, CUiItem* aItem,
                          TBool aScrollText = EFalse );

public:
	// gives back number of added items to the list
	TInt DisplaySearchResultsL();
	void CreateLineL( RBuf& aLine, RBuf& aLine1, CVideoEntry* aEntry );
	void RescaleIconsL();
	void ApplyRatingStarsL( CVideoEntry* aEntry );

	void AddCurrentItemToFavsL();
	void RemoveCurrentItemFromFavsL();
	TBool IsCurrentItemInFavsL();
	TBool IsListboxLineMore();

	void DeleteSavedClipL();
	void RenameSavedClipL();

    CAknSearchField* CreateFindBoxL();
    TBool IsEntryVisibleL( CVideoEntry* aEntry );

	void SetNaviPaneTextL();

	void ChangeTabL( TInt aWhich );

	void UpdateToolbar();
	void ClearToolbar();
	void ShowToolbar( TBool aShow );

	void RunItemDialogL();
	void RunFloatingItemDialogL( TPoint aWhere );
	void ItemDialogCommandsL( RPointerArray<HBufC>& aTexts, RArray<TPopupCommand>& aCommands );
	void HandleItemDialogResponseL( TInt aCommand );
	
	HBufC* FindBoxText();
private:
	CEmTubeSearchView& iView;
	CEmTubeAppUi* iAppUi;
//	CAknDoubleLargeStyleListBox *iListBox;

	HBufC* iTimeFormatString;

//progress
	CFbsBitmap* iBackBitmap;

	CEmTubeTimeOutTimer* iTimer;

	TBool iProgressEnabled;
	TInt iFrameCount;
	RPointerArray<CFbsBitmap> iBitmaps;
	CFont* iFont;
	TInt iCurrentFrame;

//ratings engine
	CFbsBitmap* iStarBitmapRed;
	CFbsBitmap* iStarMaskRed;
	CFbsBitmap* iStarBitmapGray;
	CFbsBitmap* iStarMaskGray;
	CFbsBitmap* iStarBitmapWhite;
	CFbsBitmap* iStarMaskWhite;
	CFbsBitmap* iHeartBitmap;
	CFbsBitmap* iHeartMask;

//navipane
	CAknNavigationControlContainer* iNaviPane;
	CAknNavigationDecorator* iNaviLabelDecorator;

//progress
	CAknProgressDialog* iProgressDialog;
	CEikProgressInfo* iProgressInfo;
	TBool iFromDialog;

	TBool iFindBoxEnabled;

	MUiInterface* iUi;
	CFont* iCustomUiFont;
	TInt iScrollAmount;
	TInt iScrollAmount2;

	CFbsBitmap* iAuthorLogo;
	CFbsBitmap* iAuthorMask;
	CFbsBitmap* iTimeLogo;
	CFbsBitmap* iTimeMask;

	TInt iPluginsCount;

	CAknStylusPopUpMenu* iPopup;
	CVideoEntry* iEntry; // just a pointer, do not delete this
	TInt iPreviousSelectedItem;
	};

#endif // EMTUBE_SEARCHVIEW_CONTAINER_H

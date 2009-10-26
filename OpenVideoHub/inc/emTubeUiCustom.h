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

#ifndef EMTUBE_UI_CUSTOM_H
#define EMTUBE_UI_CUSTOM_H

#include <coecntrl.h>
#include <e32std.h>

#include "emTubeMUiInterface.h"
#include "emTubeTimeOutTimer.h"
#include "emTubeMItemDrawer.h"


class CEmTubeAppUi;
class CUiItem;

const TRgb KCustomUiTextColor = TRgb(255,255,255);
const TRgb KCustomUiSelectedTextColor = TRgb(255,255,0);

class CEmTubeUiCustom : public CBase, public MUiInterface, public MTimeOutObserver
	{
public:
	enum TTimerMode
		{
		ETimerModeRotate = 1,
		ETimerModeScrollText
		};

public: // construction / destruction
	static CEmTubeUiCustom* NewL(MHandleCommandObserver& aObserver, const TRect& aRect);
	static CEmTubeUiCustom* NewLC(MHandleCommandObserver& aObserver, const TRect& aRect);
	~CEmTubeUiCustom();
   
private: // construction
	CEmTubeUiCustom(MHandleCommandObserver& aObserver, const TRect& aRect);
	void ConstructL();

public:  // MTimeOutObserver
	void TimerExpired( TInt aMode );

public: // from MUiInterface   
	TInt CountComponentControls();
	CCoeControl* ComponentControl( TInt aIndex );
	void HandleResourceChange( TInt aType );
	TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent,TEventCode aType );
	void SizeChanged( const TRect& aRect, TInt aCurrentItem );
	void Draw( const TRect& aRect, CWindowGc& aGraphicContext );
	
    void InitializeL( MUiInterface::TType aType, TInt aVisibleItems, CCoeControl* aControl, TBool aEnableFindBox, MItemDrawer* aItemDrawer, TRect aRect );
    TBool Initialized();
	void DestroyL();

    void RefreshL( TInt aCurrentItem = 0 );
    
    void AppendItemL( TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData );

    void AppendIconL( CGulIcon* aIcon, TBool aResize );

    TInt CurrentItemIndex();

	TInt ItemCount();
	void ResetItems();
	void ResetIcons();
	CGulIcon* Icon( TInt aIdx );
	TInt IconsCount();
	void GetIconSize(TInt& aWidth, TInt& aHeight);
	void RemoveItemL( TInt aIdx );
	void InsertItemL( TInt aIdx, TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData );

	void ChangeItemIconIdxL( TInt aIdx, TInt aIconIdx );

	HBufC* FindBoxText();
	void SetFindBoxTextL( const TDesC& aText );

	TBool IsEntryVisibleL( const TDesC& aText );

	void HideFindBoxL();

    void SetEmptyTextL( const TDesC& aEmptyText );

    CFbsBitmap* BackgroundBitmapL( TBool aClear );

	void AddTabsL( RArray<TPtrC> aTabNames, TInt aActiveTab ){}	
	void DeleteTabs() {}
	void SetActiveTab( TInt aWhich ) {}
	TInt ActiveTab() { return 0; }

public:    	
	void SetAngleAndScale( TInt aIndex, TReal aAngle, TReal aScale );
	void UpdateAngleAndScale( TInt aIndex, TReal aDiff, TReal aScaleDiff );
	void SetupAngles();
	TBool UpdateAngles();

	void DrawElement( TInt aIdx, TBool aScrollText = EFalse );

private:
	void CreateBitmapL( const TRect& aRect );

private: // data 
	MHandleCommandObserver& iObserver;

	TRect iRect;
	CWindowGc* iGc;
	CCoeControl* iParent;	

	RPointerArray<CUiItem> iItems;
	RPointerArray<CGulIcon> iIcons;

	TInt iCurrentItem;
	TInt iDestinationItem;
	TInt iDirection;

	CEmTubeTimeOutTimer* iTimer;
	
	CFbsBitmap* iBitmap;
    CFbsBitGc* iBitmapGc;
    CFbsBitmapDevice* iDevice;

	TInt iSteps;
	TInt iVisibleItems;
	RArray<TReal> iAngles;
	TBool iInitialized;
	
	HBufC* iEmptyText;
	
	MItemDrawer* iItemDrawer;

	CFbsBitmap* iBackgroundBitmap;

    MItemDrawer::TItemState iItemState;
    
    TBool iBlitOnly;    

    TInt iMaxScrollRepeat;
    TInt iScrollCount;

	CFbsBitmap* iBackBitmap;
	};

#endif // EMTUBE_UI_CUSTOM_H

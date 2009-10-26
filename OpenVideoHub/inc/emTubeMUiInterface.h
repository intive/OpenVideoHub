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

#ifndef EMTUBE_UI_INTERFACE
#define EMTUBE_UI_INTERFACE

#include <EIKLBO.H>
#include <e32cmn.h>

class CEikListBox;
class CFbsBitmap;
class MItemDrawer;

class MUiInterface
	{
public:
	enum TType
		{
		ETypeSingle,
		ETypeDoubleLarge,
		ETypeDoubleGraphic
		};

public:
	inline virtual ~MUiInterface() {};

	virtual TInt CountComponentControls() = 0;
	virtual CCoeControl* ComponentControl( TInt aIndex ) = 0;
	virtual TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType ) = 0;
	virtual void SizeChanged( const TRect& aRect, TInt aCurrentItem ) = 0;
	virtual void Draw( const TRect& aRect, CWindowGc& aGraphicContext ) = 0;

// listbox handling
    virtual void InitializeL( MUiInterface::TType aType, TInt aVisibleItems, CCoeControl* aControl, TBool aEnableFindBox, MItemDrawer* aItemDrawer, TRect aRect ) = 0;
    virtual TBool Initialized() = 0;
	virtual void DestroyL() = 0;

    // draws all items and sets current item
    virtual void RefreshL( TInt aCurrentItem = 0 ) = 0;

    virtual void AppendItemL( TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData ) = 0;

    virtual void AppendIconL( CGulIcon* aIcon, TBool aResize ) = 0;

    virtual TInt CurrentItemIndex() = 0;

	virtual TInt ItemCount() = 0;
	virtual void ResetItems() = 0;
	virtual void ResetIcons() = 0;
	virtual CGulIcon* Icon( TInt aIdx ) = 0;
	virtual TInt IconsCount() = 0;
	virtual void GetIconSize(TInt& aWidth, TInt& aHeight) = 0;
	virtual void RemoveItemL( TInt aIdx ) = 0;
	virtual void InsertItemL( TInt aIdx, TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData ) = 0;

	virtual void ChangeItemIconIdxL( TInt aIdx, TInt aIconIdx ) = 0;

	virtual HBufC* FindBoxText() = 0;
	virtual void SetFindBoxTextL( const TDesC& aText ) = 0;
	virtual TBool IsEntryVisibleL( const TDesC& aText ) = 0;

    virtual void SetEmptyTextL( const TDesC& aEmptyText ) = 0;

    virtual CFbsBitmap* BackgroundBitmapL( TBool aClear ) = 0;

// tabs handling
	virtual void AddTabsL( RArray<TPtrC> aTabNames, TInt aActiveTab ) = 0;
	virtual void DeleteTabs() = 0;
	virtual void SetActiveTab( TInt aWhich ) = 0;
	virtual TInt ActiveTab() = 0;

	};

class MHandleCommandObserver
    {
public:

    enum TObserverEvent
    {
        EHandleItemClicked = 0,
        EHandleItemDeleted,
        EHandleItemSelected,
        EHandleCreateItemList,
        EHandleKeyLeft,
        EHandleKeyRight,
        EHandleKeyUp,
        EHandleKeyDown,
        EHandleTabChanged,
        EHandleTimerExpired
    };

public:

    virtual void HandleCommandL( TObserverEvent aEvent ) = 0;

    };

#endif // EMTUBE_UI_INTERFACE

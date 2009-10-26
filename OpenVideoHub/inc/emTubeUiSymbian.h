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

#ifndef EMTUBE_UI_SYMBIAN_H
#define EMTUBE_UI_SYMBIAN_H

#include <coecntrl.h>
#include <coecobs.h>
#include <aknlists.h> // list
#include <AknTabObserver.h>
#include <e32std.h>

#include "emTubeMUiInterface.h"

class CEmTubeAppUi;
class CAknTabGroup;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;

class CEmTubeListBoxInterface
	{
public:
	inline virtual ~CEmTubeListBoxInterface() {};

	virtual CEikListBox *ListBox() = 0;
	virtual MDesCArray *ItemTextArray() = 0;
	virtual CArrayPtr<CGulIcon> *Icons() = 0;
	virtual TSize SubCellSize( TInt aIdx ) = 0;
	virtual TMargins SubCellMargins( TInt aIdx ) = 0;
	};

//single line listbox
class CEmTubeListBoxSingle : public CBase, public CEmTubeListBoxInterface
	{
public: // construction / destruction
	static CEmTubeListBoxSingle* NewL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );
	static CEmTubeListBoxSingle* NewLC( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );
	~CEmTubeListBoxSingle();

public:
	CEikListBox *ListBox();
	MDesCArray *ItemTextArray();
	CArrayPtr<CGulIcon> *Icons();
	TSize SubCellSize( TInt aIdx );
	TMargins SubCellMargins( TInt aIdx );

private: // construction
	CEmTubeListBoxSingle();
	void ConstructL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );

private:
	CAknSingleLargeStyleListBox *iListBox;

	};

//double line listbox
class CEmTubeListBoxDouble : public CBase, public CEmTubeListBoxInterface
	{
public: // construction / destruction
	static CEmTubeListBoxDouble* NewL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );
	static CEmTubeListBoxDouble* NewLC( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );
	~CEmTubeListBoxDouble();

public:
	CEikListBox *ListBox();
	MDesCArray *ItemTextArray();
	CArrayPtr<CGulIcon> *Icons();
	TSize SubCellSize( TInt aIdx );
	TMargins SubCellMargins( TInt aIdx );

private: // construction
	CEmTubeListBoxDouble();
	void ConstructL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect );

private:
	CAknDoubleLargeStyleListBox *iListBox;

	};

class CEmTubeUiSymbian : public CBase, public MUiInterface, public MEikListBoxObserver,
						 public MCoeControlObserver, public MAknTabObserver
	{

public: // construction / destruction
	static CEmTubeUiSymbian* NewL(MHandleCommandObserver& aObserver, const TRect& aRect);
	static CEmTubeUiSymbian* NewLC(MHandleCommandObserver& aObserver, const TRect& aRect);
	~CEmTubeUiSymbian();

private: // construction
	CEmTubeUiSymbian(MHandleCommandObserver& aObserver, const TRect& aRect);
	void ConstructL();

public: // from MAknTabObserver
    void TabChangedL( TInt aIndex );

public: //from MEikListBoxObserver
	void HandleListBoxEventL(CEikListBox* aListBox, MEikListBoxObserver::TListBoxEvent aEventType);

public: // From MCoeControlObserver
    void HandleControlEventL(CCoeControl* aControl, TCoeEvent aEventType);

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
	void HandlePopupFindSizeChanged();
	void InsertItemL( TInt aIdx, TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData );

	void ChangeItemIconIdxL( TInt aIdx, TInt aIconIdx );

	HBufC* FindBoxText();
	void SetFindBoxTextL( const TDesC& aText );

	TBool IsEntryVisibleL( const TDesC& aText );

    void SetEmptyTextL( const TDesC& aEmptyText );

    CFbsBitmap* BackgroundBitmapL( TBool aClear );

	void AddTabsL( RArray<TPtrC> aTabNames, TInt aActiveTab );
	void DeleteTabs();
	void SetActiveTab( TInt aWhich );
	TInt ActiveTab();

private:
	CAknSearchField* CreateFindBoxL();

private: // data
	MHandleCommandObserver& iObserver;

	MUiInterface::TType iListType;

	CEmTubeListBoxInterface *iListBox;

	TRect iRect;
	CCoeControl* iParent;

	CArrayPtr<CGulIcon>* iIcons;

	// find box
	TBool iFindBoxEnabled;
    CAknSearchField* iFindBox;
	HBufC* iFindBoxText;

	HBufC* iEmptyText;
	CFbsBitmap* iBackBitmap;

	// tabs
	CAknTabGroup* iTabGroup;
	CAknNavigationControlContainer* iNaviPane;
	CAknNavigationDecorator* iNaviDecoratorForTabs;
	};

#endif // EMTUBE_UI_SYMBIAN_H

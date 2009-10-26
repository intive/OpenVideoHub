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
#include <stringloader.h>
#include <aknpopup.h>
#include <aknsfld.h>
#include <aknlists.h>
#include <eikclbd.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <akntabgrp.h>
#include <COEAUI.H>

#include "emTubeUiSymbian.h"

//single line listbox:
CEmTubeListBoxSingle* CEmTubeListBoxSingle::NewL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	CEmTubeListBoxSingle* self = CEmTubeListBoxSingle::NewLC( aEmptyText, aObserver, aParent, aIcons, aRect );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeListBoxSingle* CEmTubeListBoxSingle::NewLC( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	CEmTubeListBoxSingle* self = new (ELeave) CEmTubeListBoxSingle();
	CleanupStack::PushL(self);
	self->ConstructL( aEmptyText, aObserver, aParent, aIcons, aRect );
	return self;
	}

void CEmTubeListBoxSingle::ConstructL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	iListBox = new (ELeave) CAknSingleLargeStyleListBox;

	iListBox->SetContainerWindowL( *aParent );
	iListBox->ConstructL( aParent, EAknListBoxSelectionList );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
	iListBox->SetListBoxObserver( aObserver );
	iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL( ETrue );

	iListBox->ItemDrawer()->ColumnData()->SetIconArray( aIcons );
	iListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(3, 5, 1000000, 50000);
	iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);

	if( aEmptyText )
		iListBox->View( )->SetListEmptyTextL( *aEmptyText );

	iListBox->SetRect( aRect );
	iListBox->SetFocus( ETrue , EDrawNow );
	iListBox->ActivateL();
	}

CEmTubeListBoxSingle::CEmTubeListBoxSingle()
	{
	}

CEmTubeListBoxSingle::~CEmTubeListBoxSingle()
	{
	delete iListBox;
	}

CEikListBox *CEmTubeListBoxSingle::ListBox()
	{
	return iListBox;
	}

MDesCArray *CEmTubeListBoxSingle::ItemTextArray()
	{
	return iListBox->Model()->ItemTextArray();
	}

CArrayPtr<CGulIcon> *CEmTubeListBoxSingle::Icons()
	{
	return iListBox->ItemDrawer()->ColumnData()->IconArray();
	}

TSize CEmTubeListBoxSingle::SubCellSize( TInt /*aIdx*/ )
	{
	return iListBox->ItemDrawer()->ItemCellSize();
	}

TMargins CEmTubeListBoxSingle::SubCellMargins( TInt /*aIdx*/ )
	{
	TMargins m;
	m.iBottom = m.iLeft = m.iRight = m.iTop = 0;
	return m;
	}

//double line listbox
CEmTubeListBoxDouble* CEmTubeListBoxDouble::NewL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	CEmTubeListBoxDouble* self = CEmTubeListBoxDouble::NewLC( aEmptyText, aObserver, aParent, aIcons, aRect );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeListBoxDouble* CEmTubeListBoxDouble::NewLC( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	CEmTubeListBoxDouble* self = new (ELeave) CEmTubeListBoxDouble();
	CleanupStack::PushL(self);
	self->ConstructL( aEmptyText, aObserver, aParent, aIcons, aRect );
	return self;
	}

void CEmTubeListBoxDouble::ConstructL( HBufC* aEmptyText, MEikListBoxObserver* aObserver, CCoeControl* aParent, CArrayPtr<CGulIcon>* aIcons, TRect aRect )
	{
	iListBox = new (ELeave) CAknDoubleLargeStyleListBox;

	iListBox->SetContainerWindowL( *aParent );
	iListBox->ConstructL( aParent, EAknListBoxSelectionList );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
	iListBox->SetListBoxObserver( aObserver );
	iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );

	iListBox->ItemDrawer()->ColumnData()->SetIconArray( aIcons );
	iListBox->ItemDrawer()->FormattedCellData()->SetMarqueeParams(3, 5, 1000000, 50000);
	iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);

	if( aEmptyText )
		iListBox->View( )->SetListEmptyTextL( *aEmptyText );

	iListBox->SetRect( aRect );
	iListBox->SetFocus( ETrue , EDrawNow );
	iListBox->ActivateL();
	}

CEmTubeListBoxDouble::CEmTubeListBoxDouble()
	{
	}

CEmTubeListBoxDouble::~CEmTubeListBoxDouble()
	{
	delete iListBox;
	}

CEikListBox *CEmTubeListBoxDouble::ListBox()
	{
	return iListBox;
	}

MDesCArray *CEmTubeListBoxDouble::ItemTextArray()
	{
	return iListBox->Model()->ItemTextArray();
	}

CArrayPtr<CGulIcon> *CEmTubeListBoxDouble::Icons()
	{
	return iListBox->ItemDrawer()->ColumnData()->IconArray();
	}

TSize CEmTubeListBoxDouble::SubCellSize( TInt aIdx )
	{
	return iListBox->ItemDrawer()->ColumnData()->SubCellSize( aIdx );
	}

TMargins CEmTubeListBoxDouble::SubCellMargins( TInt aIdx )
	{
	return iListBox->ItemDrawer()->ColumnData()->SubCellMargins( aIdx );
	}

//symbian ui
CEmTubeUiSymbian* CEmTubeUiSymbian::NewL( MHandleCommandObserver& aObserver, const TRect& aRect )
	{
	CEmTubeUiSymbian* self = CEmTubeUiSymbian::NewLC( aObserver, aRect );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeUiSymbian* CEmTubeUiSymbian::NewLC( MHandleCommandObserver& aObserver, const TRect& aRect )
	{
	CEmTubeUiSymbian* self = new (ELeave) CEmTubeUiSymbian( aObserver, aRect );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeUiSymbian::~CEmTubeUiSymbian()
	{
	if( !iListBox )
		delete iIcons;
	else
		{
		delete iListBox;
		}

	delete iFindBox;
	delete iFindBoxText;

	delete iEmptyText;

	delete iBackBitmap;

	if( !iNaviPane )
		{
		CEikStatusPane *sp = ((CAknAppUi*)CEikonEnv::Static()->EikAppUi())->StatusPane();
		iNaviPane = (CAknNavigationControlContainer*)
		sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
		}

	if( iNaviPane )
		iNaviPane->Pop(NULL);

	delete iNaviDecoratorForTabs;
	}

CEmTubeUiSymbian::CEmTubeUiSymbian( MHandleCommandObserver& aObserver, const TRect& aRect ):
		iObserver(aObserver), iRect(aRect)
	{
	}

void CEmTubeUiSymbian::ConstructL( )
	{
	iIcons = new(ELeave) CAknIconArray(8);
	}

TInt CEmTubeUiSymbian::CountComponentControls()
	{
	TInt count = 0;

	if( iListBox )
		count++;

	if( iFindBox )
		count++;

	return count;
	}

CCoeControl* CEmTubeUiSymbian::ComponentControl( TInt aIndex )
	{
	CCoeControl *c = NULL;
	switch(aIndex)
		{
		case 0:
			c = iListBox->ListBox();
		break;

		case 1:
			c = iFindBox;
		break;

		default:
		break;
		}

	return c;
	}

void CEmTubeUiSymbian::SizeChanged( const TRect& aRect, TInt aCurrentItem )
	{
	if( iListBox )
		{
		iListBox->ListBox()->HandleItemAdditionL();
//		iListBox->ListBox()->SetCurrentItemIndex( aCurrentItem );
		iRect = aRect;
		iListBox->ListBox()->SetRect( iRect );

		if( iFindBox )
			AknFind::HandlePopupFindSizeChanged( iParent, iListBox->ListBox(), iFindBox );
		}
	}

TKeyResponse CEmTubeUiSymbian::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
	TKeyResponse ret = EKeyWasNotConsumed;
	if( !iListBox )
		return ret;

	if (aType == EEventKey)
		{
		switch (aKeyEvent.iScanCode)
			{
			case EStdKeyEnter:
			case EStdKeyDevice3:
				{
#ifdef __WINS__
				iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemClicked );
				ret = EKeyWasConsumed;
#endif
				}
			break;

			case EStdKeyBackspace:
   			case EStdKeyDelete:
   				{
   				if( !iFindBox )
   					{
   					iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemDeleted );
   					ret = EKeyWasConsumed;
   					}
   				}
			break;

			case EStdKeyLeftArrow:
				{
				iObserver.HandleCommandL( MHandleCommandObserver::EHandleKeyLeft );
				ret = EKeyWasConsumed;
				}
			break;

			case EStdKeyRightArrow:
				{
				iObserver.HandleCommandL( MHandleCommandObserver::EHandleKeyRight );
				ret = EKeyWasConsumed;
				}
			break;

			default:
				break;

			}
		}

	if( ret == EKeyWasNotConsumed && iFindBox )
		ret = iFindBox->OfferKeyEventL(aKeyEvent, aType);

	if( ret == EKeyWasNotConsumed && iListBox )
		ret = iListBox->ListBox()->OfferKeyEventL( aKeyEvent, aType );

	return ret;
	}

void CEmTubeUiSymbian::Draw( const TRect& /*aRect*/, CWindowGc& /*aGraphicContext*/ )
	{
	}

void CEmTubeUiSymbian::HandleListBoxEventL(CEikListBox* /*aListBox*/, MEikListBoxObserver::TListBoxEvent aEventType)
	{
	switch( aEventType )
		{
		case MEikListBoxObserver::EEventEnterKeyPressed:
		case MEikListBoxObserver::EEventItemClicked:
			{
#ifndef __S60_50__
			iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemClicked );
#else
			iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemSelected );
#endif
			}
		break;

		default:
			break;
		}
	}

void CEmTubeUiSymbian::HandleControlEventL(CCoeControl* aControl, TCoeEvent aEventType)
	{
	if ( aEventType == EEventStateChanged && aControl == iFindBox && iFindBoxEnabled )
		{
		TInt length = iFindBox->TextLength();

		RBuf tmp;
		CleanupClosePushL( tmp );
		tmp.Create( length );
		iFindBox->GetSearchText( tmp );
		tmp.LowerCase();

		delete iFindBoxText;
		iFindBoxText = tmp.AllocL();

		CleanupStack::PopAndDestroy( &tmp );
		iObserver.HandleCommandL( MHandleCommandObserver::EHandleCreateItemList );
		}
	}

CAknSearchField* CEmTubeUiSymbian::CreateFindBoxL()
	{
	CAknSearchField* findbox;
	CAknSearchField::TSearchFieldStyle style(CAknSearchField::EPopup);

	findbox = CAknSearchField::NewL(*iParent, style, NULL, 128);
	CAknFilteredTextListBoxModel* model = static_cast<CAknFilteredTextListBoxModel*>( iListBox->ListBox()->Model() );
	model->CreateFilterL(iListBox->ListBox(), findbox);

	if( iFindBoxText )
		findbox->SetSearchTextL( *iFindBoxText );

	return findbox;
	}

void CEmTubeUiSymbian::DestroyL()
	{
	iListBox->ListBox()->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EOff );
	iListBox->ListBox()->MakeVisible( EFalse );
	iListBox->ListBox()->DrawNow();
	if( iFindBox )
		{
		iFindBox->MakeVisible( EFalse );
		iFindBox->DrawNow();
		}
	}

void CEmTubeUiSymbian::InitializeL( MUiInterface::TType aType, TInt /*aVisibleItems*/, CCoeControl* aControl,
	TBool aEnableFindBox, MItemDrawer* /*aItemDrawer*/, TRect aRect )
	{
	if( !iIcons )
		iIcons = new(ELeave) CAknIconArray(8);

	iFindBoxEnabled = aEnableFindBox;
	iParent = aControl;

	iListType = aType;
	switch( aType )
		{
		case MUiInterface::ETypeSingle:
			iListBox = CEmTubeListBoxSingle::NewL( iEmptyText, this, aControl, iIcons, aRect );
		break;

		case MUiInterface::ETypeDoubleLarge:
			iListBox = CEmTubeListBoxDouble::NewL( iEmptyText, this, aControl, iIcons, aRect );
		break;
		}

	if( iFindBoxEnabled )
		{
		iFindBox = CreateFindBoxL();
		iFindBox->SetObserver( this );
		iFindBox->SetSkinEnabledL(ETrue);
		iFindBox->MakeVisible( ETrue );
		iFindBox->SetFocus( ETrue );
		}

	iParent->DrawNow();
	}

TBool CEmTubeUiSymbian::Initialized()
	{
	if( iListBox )
		return ETrue;

	return EFalse;
	}

void CEmTubeUiSymbian::RefreshL( TInt aCurrentItem )
	{
	if( iListBox )
		{
		iListBox->ListBox()->HandleItemAdditionL();
		if( aCurrentItem > ItemCount()-1 )
			aCurrentItem = ItemCount()-1;

		iListBox->ListBox()->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
		iListBox->ListBox()->MakeVisible( ETrue );

		if( iFindBox )
			{
			iFindBox->MakeVisible (ETrue );
			iFindBox->SetFocus( ETrue );
			}

		if( aCurrentItem >= 0 )
			{
			iListBox->ListBox()->SetCurrentItemIndex( aCurrentItem );
//TODO enable someday - this brings new searchs after "more" to the top of the list.
//			iListBox->ListBox()->SetTopItemIndex( aCurrentItem );
			}
		iListBox->ListBox()->DrawNow();
		}
	}

TInt CEmTubeUiSymbian::CurrentItemIndex()
	{
	if( iListBox )
		return iListBox->ListBox()->CurrentItemIndex();

	return KErrNotFound;
	}

void CEmTubeUiSymbian::AppendItemL( TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* /*aPrivateData*/ )
	{
	TBuf<10> icon;
	icon.Num( aIconNumber );

	RBuf item;
	CleanupClosePushL( item );
	item.Create( icon.Length() + aFirstLine.Length() + aSecondLine.Length() + 2 );

	item.Copy( icon );
	item.Append( _L("\t") );
	item.Append( aFirstLine );
	item.Append( _L("\t") );
	item.Append( aSecondLine );

	CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
	list->AppendL( item );

	CleanupStack::PopAndDestroy( &item );
	}

void CEmTubeUiSymbian::AppendIconL( CGulIcon* aIcon, TBool /*aResize*/ )
	{
	CArrayPtr<CGulIcon>* icons = iListBox->Icons();
	icons->AppendL( aIcon );
	}

TInt CEmTubeUiSymbian::ItemCount()
	{
	TInt count = 0;
	if( iListBox )
		{
		CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
		count = list->Count();
		}
	return count;
	}

void CEmTubeUiSymbian::ResetItems()
	{
	if( iListBox )
		{
		TInt c = ItemCount();
		CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
		list->Reset();
		iListBox->ListBox()->SetCurrentItemIndex( 0 );
		iListBox->ListBox()->HandleItemRemovalL();
		}
	}

void CEmTubeUiSymbian::ResetIcons()
	{
	if( iListBox )
		{
		CArrayPtr<CGulIcon>* icons = iListBox->Icons();
		icons->ResetAndDestroy();
		}
	}

CGulIcon* CEmTubeUiSymbian::Icon( TInt aIdx )
	{
	CArrayPtr<CGulIcon>* icons = iListBox->Icons();
	return icons->At( aIdx );
	}

TInt CEmTubeUiSymbian::IconsCount()
	{
	CArrayPtr<CGulIcon>* icons = iListBox->Icons();
	return icons->Count();
	}

void CEmTubeUiSymbian::GetIconSize(TInt& aWidth, TInt& aHeight)
	{
	aWidth = 0;
	aHeight = 0;
	if( iListBox )
		{
		TSize size = iListBox->SubCellSize( 0 );
		TMargins margins = iListBox->SubCellMargins( 0 );
		aWidth = size.iWidth - ( margins.iLeft + margins.iRight );
		aHeight = size.iHeight - ( margins.iTop + margins.iBottom );
		}
	}

void CEmTubeUiSymbian::RemoveItemL( TInt aIdx )
	{
	if( iListBox )
		{
		CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
		list->Delete( aIdx );
		iListBox->ListBox()->HandleItemRemovalL();
		}
	}

void CEmTubeUiSymbian::InsertItemL( TInt aIdx, TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* /*aPrivateData*/ )
	{
	TBuf<10> icon;
	icon.Num( aIconNumber );

	RBuf item;
	CleanupClosePushL( item );
	item.Create( icon.Length() + aFirstLine.Length() + aSecondLine.Length() + 2 );

	item.Copy( icon );
	item.Append( _L("\t") );
	item.Append( aFirstLine );
	item.Append( _L("\t") );
	item.Append( aSecondLine );

	CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
	list->InsertL( aIdx, item );

	CleanupStack::PopAndDestroy( &item );

	iListBox->ListBox()->HandleItemAdditionL();
	}

void CEmTubeUiSymbian::ChangeItemIconIdxL( TInt aIdx, TInt aIconIdx )
	{
	TBuf<10> icon;
	icon.Num( aIconIdx );

	CDesCArray* list = static_cast<CDesCArray*>( iListBox->ItemTextArray() );
	TPtrC16 line = list->MdcaPoint( aIdx );

	TLex t( line );
	TPtrC val = t.NextToken();

	RBuf item;
	CleanupClosePushL( item );
	item.Create( icon.Length() + line.Length() );

	item.Copy( icon );
	TPtrC ll = line.Mid( val.Length() );
	item.Append( ll );

	list->Delete( aIdx );
	list->InsertL( aIdx, item );

	CleanupStack::PopAndDestroy( &item );

	}

void CEmTubeUiSymbian::SetFindBoxTextL( const TDesC& aText )
	{
	delete iFindBoxText;
	iFindBoxText = aText.AllocL();
	if( iFindBox )
		iFindBox->SetSearchTextL( aText );
	}

HBufC* CEmTubeUiSymbian::FindBoxText()
	{
	HBufC* res = NULL;
	if ( iFindBoxText ) 
		res = iFindBoxText;
	return res;
	}

TBool CEmTubeUiSymbian::IsEntryVisibleL( const TDesC& aText )
	{
	TBool result = ETrue;

	if( iFindBox )
		{
		result = EFalse;
		TInt length = iFindBox->TextLength();
		if( length )
			{
			RBuf videoName;
			CleanupClosePushL( videoName );
			videoName.Create( aText.Length() );
			videoName.CopyLC( aText );

			if( videoName.Find( *iFindBoxText ) != KErrNotFound )
				{
				result = ETrue;
				}

			CleanupStack::PopAndDestroy(&videoName);
			}
		 else
		 	{
			result = ETrue;
		 	}
		}
	return result;
	}

void CEmTubeUiSymbian::SetEmptyTextL( const TDesC& aEmptyText )
	{
	delete iEmptyText;
	iEmptyText = NULL;

	iEmptyText = aEmptyText.AllocL();

	if( iListBox )
		iListBox->ListBox()->View( )->SetListEmptyTextL( *iEmptyText );
	}

CFbsBitmap* CEmTubeUiSymbian::BackgroundBitmapL( TBool aClear )
	{
	delete iBackBitmap;
	iBackBitmap = NULL;

	if( !aClear )
		{
		CWsScreenDevice* screenDevice = new (ELeave) CWsScreenDevice( CCoeEnv::Static()->WsSession() );
		screenDevice->Construct();
		CleanupStack::PushL(screenDevice);
		iBackBitmap = new (ELeave) CFbsBitmap();

		TRect rect;
		AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, rect );

		iBackBitmap->Create( rect.Size(), EColor16MU );
		iBackBitmap->SetSizeInTwips(screenDevice);
		screenDevice->CopyScreenToBitmap( iBackBitmap, rect );
		CleanupStack::PopAndDestroy( screenDevice );

		iBackBitmap->LockHeap(ETrue);
		TUint32* src = (TUint32*)iBackBitmap->DataAddress();
		TInt size = iBackBitmap->SizeInPixels().iWidth * iBackBitmap->SizeInPixels().iHeight;

		for(TInt i=0;i<size;i++)
			{
			TUint32 p = *src;

			TUint32 r = (p >> 16) & 0xff;
			TUint32 g = (p >> 8) & 0xff;
			TUint32 b = (p) & 0xff;
			p = (( ((r * 25) / 100) ) << 16) + (( ((g * 25) / 100) ) << 8) + ( ((b * 25) / 100) );

			*src++ = p;
			}

		iBackBitmap->UnlockHeap(ETrue);
		}
	return iBackBitmap;
	}

void CEmTubeUiSymbian::AddTabsL( RArray<TPtrC> aTabNames, TInt aActiveTab )
	{
	CEikStatusPane *sp = ((CAknAppUi*)CEikonEnv::Static()->EikAppUi())->StatusPane();

	iNaviPane = (CAknNavigationControlContainer*) sp->ControlL(TUid::Uid( EEikStatusPaneUidNavi ));

	CAknNavigationDecorator* iNaviDecoratorForTabsTemp;
	iNaviDecoratorForTabsTemp = iNaviPane->CreateTabGroupL();
	if( iNaviDecoratorForTabs )
		{
		delete iNaviDecoratorForTabs;
		iNaviDecoratorForTabs = NULL;
		}
	iNaviDecoratorForTabs = iNaviDecoratorForTabsTemp;
	iTabGroup = STATIC_CAST(CAknTabGroup*, iNaviDecoratorForTabs->DecoratedControl());

	if( aTabNames.Count() == 2 )
		iTabGroup->SetTabFixedWidthL( KTabWidthWithTwoLongTabs );
	else
		iTabGroup->SetTabFixedWidthL( KTabWidthWithThreeLongTabs );

	for( TInt i=0 ; i < aTabNames.Count() ; i++ )
		iTabGroup->AddTabL( i, aTabNames[i] );

	iTabGroup->SetActiveTabByIndex( aActiveTab );
	iTabGroup->SetObserver( this );
	iNaviPane->PushL( *iNaviDecoratorForTabs );
	}

void CEmTubeUiSymbian::DeleteTabs()
	{
	if( iNaviPane )
		{
		iNaviPane->Pop();
		iNaviPane = NULL;
		}
	}

void CEmTubeUiSymbian::SetActiveTab( TInt aWhich )
	{
	iTabGroup->SetActiveTabByIndex( aWhich );
	}

TInt CEmTubeUiSymbian::ActiveTab()
	{
	TInt count = 0;

	if( iTabGroup )
		count = iTabGroup->ActiveTabIndex();

	return count;
	}

void CEmTubeUiSymbian::TabChangedL( TInt aIndex )
	{
	iObserver.HandleCommandL( MHandleCommandObserver::EHandleTabChanged );
	}

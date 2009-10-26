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

#include <eikedwin.h>
#include <akntitle.h>
#include <akndialog.h>
#include <AknListQueryDialog.h>
#include <aknmessagequerydialog.h>
#include <aknnotewrappers.h>
#include <centralrepository.h>	  // CRepository
#include <ProfileEngineSDKCRKeys.h> // CRepository

#include "emTubeResource.h"
#include "emTube.hrh"

#include "emTubeSettingsListBox.h"
#include "emTubeSettingsData.h"
#include "emTubeSettingsView.h"
#include "emTubeAppUi.h"
#include "emTubeHttpEngine.h"

//popup item overloaded for access point

class CEmTubeAccessPointPopupSettingItem : public CAknEnumeratedTextPopupSettingItem
	{
	public:
		CEmTubeAccessPointPopupSettingItem( TInt aResourceId, TInt& aValue );

		void CompleteConstructionL();

		TInt iValue;
	};

CEmTubeAccessPointPopupSettingItem::CEmTubeAccessPointPopupSettingItem( TInt aResourceId, TInt& aValue ) : CAknEnumeratedTextPopupSettingItem( aResourceId, aValue )
	{
	iValue = aValue;
	}

void CEmTubeAccessPointPopupSettingItem::CompleteConstructionL()
	{
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

	CArrayPtr<CAknEnumeratedText>* texts = EnumeratedTextArray();

	TInt profileId = CEmTubeHttpEngine::ActiveProfileL();

	CCommsDatabase* CommDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
	CleanupStack::PushL( CommDb );
	CCommsDbTableView* table = CommDb->OpenTableLC( TPtrC(IAP) );

	if ( table->GotoFirstRecord() == KErrNone )
		{
		do
			{
			TUint32 bearerType;
			table->ReadUintL( TPtrC(IAP_BEARER), bearerType );

			if( (profileId == KOfflineProfile) && (bearerType == 2) )
				continue;

			CAknEnumeratedText* enumeratedText;

			TUint32 ap;
			table->ReadUintL( TPtrC(COMMDB_ID), ap );
			HBufC* apName = HBufC::NewLC( KCommsDbSvrMaxColumnNameLength );
			TPtr apNamePtr( apName->Des() );
			table->ReadTextL(TPtrC(COMMDB_NAME), apNamePtr );

			enumeratedText = new (ELeave) CAknEnumeratedText( ap, apName );
			CleanupStack::Pop( apName );

			CleanupStack::PushL( enumeratedText );
			texts->AppendL( enumeratedText );
			CleanupStack::Pop( enumeratedText );
			}
		while( table->GotoNextRecord() == KErrNone );
		}

	CleanupStack::PopAndDestroy( table );
	CleanupStack::PopAndDestroy( CommDb );
	}

//popup item overloaded for max results

class CEmTubeEnumeratedTextPopupSettingItem : public CAknEnumeratedTextPopupSettingItem
	{
	public:
		CEmTubeEnumeratedTextPopupSettingItem( TInt aResourceId, TInt& aValue );

		void CompleteConstructionL();

		TInt iValue;
	};

CEmTubeEnumeratedTextPopupSettingItem::CEmTubeEnumeratedTextPopupSettingItem( TInt aResourceId, TInt& aValue ) : CAknEnumeratedTextPopupSettingItem( aResourceId, aValue )
	{
	iValue = aValue;
	}

void CEmTubeEnumeratedTextPopupSettingItem::CompleteConstructionL()
	{
	CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

	if( iValue != 5 && iValue != 8 && iValue != 12 )
		{

		CArrayPtr<CAknEnumeratedText>* texts = EnumeratedTextArray();
		HBufC* selectionText;
		CAknEnumeratedText* enumeratedText;
		TBuf<4> num;
		num.Format( _L("%d"), iValue );
		selectionText = num.AllocLC();
		enumeratedText = new ( ELeave) CAknEnumeratedText( iValue, selectionText );
		CleanupStack::PushL( enumeratedText );
		texts->AppendL( enumeratedText );
		CleanupStack::Pop( enumeratedText );
		CleanupStack::Pop( selectionText );
		}
	}

CAknSettingItem* CEmTubeSettingsListBox::CreateSettingItemL( TInt aIdentifier )
	{
	CAknSettingItem* settingItem = NULL;

	switch (aIdentifier)
		{
		case EMTVSettingsVolume:
			settingItem = settingItemVolume = new (ELeave) CAknVolumeSettingItem(aIdentifier, iData->iVolume );
			break;

		case EMTVSettingsUpscaleVideo:
			settingItem = settingItemUpscaleVideo = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iVideoScaleMode );
			break;

		case EMTVSettingsStartPlayback:
			settingItem = settingItemStartPlayback = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iStartPlaybackMode );
			break;

		case EMTVSettingsMaxResults:
			settingItem = settingItemMaxResults = new (ELeave) CEmTubeEnumeratedTextPopupSettingItem(aIdentifier, iData->iMaxResults );
			break;

		case EMTVSettingsSortResultsBy:
			settingItem = settingItemSortResultsBy = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iSortResultsBy );
			break;

		case EMTVSettingsAccessPoint:
			settingItem = settingItemAccessPoint  = new (ELeave) CEmTubeAccessPointPopupSettingItem(aIdentifier, iData->iAccessPoint );
			break;

		case EMTVSettingsCacheSize:
			settingItem = settingItemCacheSize = new (ELeave) CAknSliderSettingItem(aIdentifier, iData->iCacheSize );
			break;

		case EMTVSettingsAutoRotate:
			settingItem = settingItemAutoRotate = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iAutoRotate );
			break;

		case EMTVSettingsTempMemory:
			settingItem = settingItemTempMemory = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iTempMemory );
			break;

#ifdef ENABLE_CUSTOM_UI
		case EMTVSettingsS60Ui:
			settingItem = settingItemS60Ui = new (ELeave) CAknEnumeratedTextPopupSettingItem(aIdentifier, iData->iS60Ui );
            break;
#endif

		default:
			break;
		}
	return settingItem;
	}

void CEmTubeSettingsListBox::EditItemL( TInt aIndex, TBool aCalledFromMenu )
	{
	if( EMTVSettingsVolume + aIndex == EMTVSettingsTempMemory )
		{
		CEmTubeAppUi* appUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

		if( appUi->MMCAvailable() )
			CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);
		}
	else if( EMTVSettingsVolume + aIndex == EMTVSettingsMaxResults )
		{
		if( iData->iMaxResults != 5 && iData->iMaxResults != 8 && iData->iMaxResults != 12 )
			iData->iMaxResults = 0;

		CAknEnumeratedTextPopupSettingItem *item = (CAknEnumeratedTextPopupSettingItem *)settingItemMaxResults;
		CArrayPtr<CAknEnumeratedText>* texts = item->EnumeratedTextArray();

		while( texts->Count() > 4 )
			{
			CAknEnumeratedText* enumeratedText = texts->At(4);
			texts->Delete( 4 );
			delete enumeratedText;
			}

		item->HandleTextArrayUpdateL();
		item->UpdateListBoxTextL();

		CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);
		}
	else
		{
		CAknSettingItemList::EditItemL(aIndex, aCalledFromMenu);
		}

	switch (EMTVSettingsVolume + aIndex)
		{
		case EMTVSettingsMaxResults:
			{
			settingItemMaxResults->StoreL();
			if( iData->iMaxResults != 5 && iData->iMaxResults != 8 && iData->iMaxResults != 12 )
				{
				TInt number = 15;
				if( iData->iMaxResults != 0 ) number = iData->iMaxResults;
				CAknNumberQueryDialog* dlg = new ( ELeave ) CAknNumberQueryDialog( number, CAknQueryDialog::ENoTone );
				dlg->SetEmergencyCallSupport( EFalse );
				if( dlg->ExecuteLD( R_SETTINGS_MAXRESULTS_DIALOG ) == EAknSoftkeyOk )
					{
					iData->iMaxResults = number;

					CAknEnumeratedTextPopupSettingItem *item = (CAknEnumeratedTextPopupSettingItem *)settingItemMaxResults;
					CArrayPtr<CAknEnumeratedText>* texts = item->EnumeratedTextArray();

					HBufC* selectionText;
					CAknEnumeratedText* enumeratedText;
					TBuf<4> num;
					num.Format( _L("%d"), number );
					selectionText = num.AllocLC();
					enumeratedText = new ( ELeave) CAknEnumeratedText( number, selectionText );
					CleanupStack::PushL( enumeratedText );
					texts->AppendL( enumeratedText );
					CleanupStack::Pop( enumeratedText );
					CleanupStack::Pop( selectionText );

					item->HandleTextArrayUpdateL();
					item->UpdateListBoxTextL();

					}
				}
			}
		break;

		case EMTVSettingsCacheSize:
			{
			settingItemCacheSize->StoreL();
			}
		break;

		case EMTVSettingsUpscaleVideo:
			{
			settingItemUpscaleVideo->StoreL();
			}
		break;

		case EMTVSettingsStartPlayback:
			{
			settingItemStartPlayback->StoreL();
			}
		break;

		case EMTVSettingsSortResultsBy:
			{
			settingItemSortResultsBy->StoreL();
			}
		break;

		case EMTVSettingsAccessPoint:
			{
			settingItemAccessPoint->StoreL();
			}
		break;

		case EMTVSettingsVolume:
			{
			settingItemVolume->StoreL();
			}
		break;

		case EMTVSettingsAutoRotate:
			{
			settingItemAutoRotate->StoreL();
			}
		break;

		case EMTVSettingsTempMemory:
			{
			settingItemTempMemory->StoreL();
			}
		break;

		case EMTVSettingsS60Ui:
			{
			settingItemS60Ui->StoreL();
			}
		break;

		default:
		break;
		}
	}

void CEmTubeSettingsListBox::StoreSettingsL()
	{
	CAknSettingItemList::StoreSettingsL();
	}

void CEmTubeSettingsListBox::SetData(CEmTubeSettingsData* aData)
	{
	iData = aData;
	}

void CEmTubeSettingsListBox::SizeChanged()
	{
	if (ListBox())
		{
		ListBox()->SetRect(Rect());
		}
	}

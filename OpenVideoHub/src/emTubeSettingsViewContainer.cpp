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

#include <barsread.h>
#include <eiklabel.h>
#include <avkon.hrh>
#include <aknlists.h>
#include <aknquerydialog.h>
#include <centralrepository.h>	  // CRepository
#include <ProfileEngineSDKCRKeys.h> // CRepository

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeAppUi.h"

#include "emTubeApplication.h"
#include "emTubeSettingsView.h"
#include "emTubeSettingsViewContainer.h"
#include "emTubeSettingsListBox.h"
#include "emTubeSettingsData.h"
#include "emTubeHttpEngine.h"

CEmTubeSettingsViewContainer* CEmTubeSettingsViewContainer::NewL( CEmTubeSettingsView& aView, const TRect& aRect )
	{
	CEmTubeSettingsViewContainer* self = CEmTubeSettingsViewContainer::NewLC(aView, aRect);
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeSettingsViewContainer* CEmTubeSettingsViewContainer::NewLC(CEmTubeSettingsView& aView, const TRect& aRect )
	{
	CEmTubeSettingsViewContainer* self = new (ELeave) CEmTubeSettingsViewContainer( aView );
	CleanupStack::PushL(self);
	self->ConstructL( aRect );
	return self;
	}

CEmTubeSettingsViewContainer::CEmTubeSettingsViewContainer( CEmTubeSettingsView& aView ) : iView( aView )
	{
	}

void CEmTubeSettingsViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
	{
	aContext.iMajor = KUidEmTubeApp;
	aContext.iContext = KContextSettings;
	}

void CEmTubeSettingsViewContainer::ConstructL( const TRect& aRect )
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	CreateWindowL();

	iData = CEmTubeSettingsData::NewL();
	iData->iVolume = iAppUi->Volume() == 0 ? 1 : iAppUi->Volume();
	iData->iStartPlaybackMode = iAppUi->StartPlaybackMode();
	iData->iMaxResults = iAppUi->MaxResults();
	iData->iCacheSize = iAppUi->MaxCacheSize() / 1024;
	iData->iAutoRotate = iAppUi->AutoRotate();
	iData->iTempMemory = iAppUi->TempMemory();
	iData->iSortResultsBy = iAppUi->SortResultsBy();
	iData->iVideoScaleMode = iAppUi->VideoScaleMode();
	iData->iS60Ui = iAppUi->S60Ui();

	TInt profileId = CEmTubeHttpEngine::ActiveProfileL();

	if( !CEmTubeHttpEngine::AccessPointExistsL( iAppUi->AccessPoint() ) ||
		( (profileId == KOfflineProfile) || !CEmTubeHttpEngine::IsAccessPointWLAN( iAppUi->AccessPoint() ) ) )
		iAppUi->SetAccessPoint( 0 );
	iData->iAccessPoint = iAppUi->AccessPoint();

	iListBox = new (ELeave) CEmTubeSettingsListBox();
	iListBox->SetData(iData);
	iListBox->SetContainerWindowL(*this);
	iListBox->SetMopParent( this );
	iListBox->ConstructFromResourceL( R_EMTV_SETTING_LIST );
	iListBox->ListBox()->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);
	iListBox->ListBox()->UpdateScrollBarsL();
	iListBox->SetRect(aRect);
	iListBox->MakeVisible(ETrue);
	iListBox->ActivateL();

	SetRect( aRect );
	ActivateL();
	}

void CEmTubeSettingsViewContainer::SaveSettingsL()
	{
	iAppUi->SetVolume( iData->iVolume );
	iAppUi->SetStartPlaybackMode( (CEmTubeAppUi::TStartPlaybackMode) iData->iStartPlaybackMode );
	iAppUi->SetMaxResults( iData->iMaxResults );
	iAppUi->SetMaxCacheSize( iData->iCacheSize * 1024 );
	iAppUi->SetAutoRotate( iData->iAutoRotate );
	iAppUi->SetTempMemory( iData->iTempMemory );
	iAppUi->SetSortResultsBy( (TOrderBy) iData->iSortResultsBy );

	iAppUi->SetVideoScaleMode( (CEmTubeAppUi::TVideoScaleMode)iData->iVideoScaleMode );

	iAppUi->SetAccessPoint( iData->iAccessPoint );
	iAppUi->SetSelectedAccessPoint( iData->iAccessPoint );

	iAppUi->SetS60Ui( iData->iS60Ui );
	}

CEmTubeSettingsViewContainer::~CEmTubeSettingsViewContainer()
	{
	delete iListBox;
	delete iData;
	}

void CEmTubeSettingsViewContainer::EditItemL()
	{
	if( iListBox )
		iListBox->EditItemL( iListBox->ListBox()->CurrentItemIndex(), ETrue );
	}

TKeyResponse CEmTubeSettingsViewContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
	if( iListBox )
		return iListBox->OfferKeyEventL( aKeyEvent, aType );

	return EKeyWasNotConsumed;
	}


void CEmTubeSettingsViewContainer::SizeChanged()
	{
	if( iListBox )
		{
		iListBox->SetRect( Rect() );
		}
	}

void CEmTubeSettingsViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange( aType );
	iListBox->HandleResourceChange( aType );

	if( aType == KEikDynamicLayoutVariantSwitch )
		{
		SetRect( iView.ClientRect() );
		iListBox->SetExtent( TPoint(0,0), iView.ClientRect().Size() );
		}
	DrawDeferred();
	}

TInt CEmTubeSettingsViewContainer::CountComponentControls() const
	{
	return 1;
	}

CCoeControl* CEmTubeSettingsViewContainer::ComponentControl( TInt /*aIndex*/ ) const
	{
	return iListBox;
	}

void CEmTubeSettingsViewContainer::Draw( const TRect& /*aRect*/ ) const
	{
	}

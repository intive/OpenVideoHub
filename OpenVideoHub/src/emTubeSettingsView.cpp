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

#include <avkon.hrh>
#include <avkon.rsg>
#include <eikclbd.h>
#include <aknlists.h>
#include <akntitle.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <aknviewappui.h>

#include "emTubeResource.h"

#include "emTube.hrh"

#include "emTubeAppUi.h"
#include "emTubeSettingsView.h"
#include "emTubeSettingsViewContainer.h"

CEmTubeSettingsView* CEmTubeSettingsView::NewL()
    {
    CEmTubeSettingsView* self = CEmTubeSettingsView::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CEmTubeSettingsView* CEmTubeSettingsView::NewLC()
    {
    CEmTubeSettingsView* self = new (ELeave) CEmTubeSettingsView();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CEmTubeSettingsView::CEmTubeSettingsView()
	{
	}

CEmTubeSettingsView::~CEmTubeSettingsView()
	{
	if(iContainer)
		{
		delete iContainer;
		iContainer = NULL;
		}
	}

void CEmTubeSettingsView::ConstructL()
    {
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

    BaseConstructL( R_EMTV_SETTINGS_VIEW );
    }

TUid CEmTubeSettingsView::Id() const
    {
    return TUid::Uid( EMTVSettingsViewId );
    }

void CEmTubeSettingsView::HandleCommandL( TInt aCommand )
	{
	switch ( aCommand )
		{
		case EMTVChangeSettingCommand:
			{
			if( iContainer )
				iContainer->EditItemL();
			}
		break;

		case EAknSoftkeyBack:
			{
			if( iContainer )
				iContainer->SaveSettingsL();
			iAppUi->SaveSettingsL();
			iAppUi->HandleCommandL( EAknSoftkeyBack );
			}
		break;

		default:
			iAppUi->HandleCommandL( aCommand );
		break;
		}
	}

void CEmTubeSettingsView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, TUid /*aListId*/, const TDesC8& /*aCustomMessage*/ )
	{
	iAppUi->ChangeScreenLayoutL( ETrue );

#ifdef __S60_50__
	iAppUi->StopDisplayingPopupToolbar();
#endif

	iContainer = CEmTubeSettingsViewContainer::NewL( *this, ClientRect() );
	iAppUi->AddToStackL( iContainer );
	}

void CEmTubeSettingsView::DoDeactivate()
	{
	if ( iContainer )
		{
		iAppUi->RemoveFromStack( iContainer );
		}

	delete iContainer;
	iContainer = NULL;
	}

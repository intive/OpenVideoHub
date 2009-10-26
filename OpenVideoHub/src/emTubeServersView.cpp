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

#include <aknviewappui.h>
#include <coecntrl.h>
#include <eikedwin.h>
#include <Avkon.mbg>
#include <aknutils.h>
#include <aknnotewrappers.h>

#include "emTubeServersView.h"
#include "emTubeAppUi.h"
#include "emTubeServersViewContainer.h"
#include "emTube.hrh"
#include "emTubeResource.h"

CEmTubeServersView* CEmTubeServersView::NewL()
	{
	CEmTubeServersView* self = CEmTubeServersView::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeServersView* CEmTubeServersView::NewLC()
	{
	CEmTubeServersView* self = new (ELeave) CEmTubeServersView();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;	
	}

CEmTubeServersView::~CEmTubeServersView()
	{
	if(iContainer)
		{
		delete iContainer;		  
		iContainer = NULL;
		}
	}

CEmTubeServersView::CEmTubeServersView()
	{
	
	}

void CEmTubeServersView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_SERVERS_VIEW );
	}

TUid CEmTubeServersView::Id() const
	{
	return TUid::Uid( EMTVServersViewId );
	}

void CEmTubeServersView::HandleCommandL( TInt aCommand )
	{
	switch(aCommand)
		{
		case EAknSoftkeyDone:
			{
			if( !iContainer->IsCurrentItemMore() )
				{
				if( !iAppUi->PluginManager().DefaultPluginUid() )
					{
					iContainer->SelectPluginL();
					}
				HandleCommandL( EMTVActivateMainViewCommand );
				}
			}
		break;

		case EMTVSetAsDefaultServerCommand:
			{
			iContainer->SelectPluginL();
			iContainer->FillListL( iAppUi->PluginManager().Uid( iContainer->CurrentItemIndex() ) );
			}
		break;

		case EMTVLoginCommand:
			{
			TBuf<128> username;
			TBuf<128> password;

			TUint32 uid = iAppUi->PluginManager().Uid( iContainer->CurrentItemIndex() );
			username.Copy( iAppUi->PluginManager().Username( uid ) );
			password.Copy( iAppUi->PluginManager().Password( uid ) );
 			CAknMultiLineDataQueryDialog* dlg = CAknMultiLineDataQueryDialog::NewL( username, password );
			if ( dlg->ExecuteLD( R_DIALOG_USERNAME_PASSWORD_QUERY ) )
				{
				iAppUi->PluginManager().SetUsernameL( uid, username );
				iAppUi->PluginManager().SetPasswordL( uid, password );
				iAppUi->LoginL( uid, EFalse );
				}
			}
		break;

		default:
			{
			iAppUi->HandleCommandL(aCommand);
			}
		break;
		}
	}	

void CEmTubeServersView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
										TUid /*aCustomMessageId*/,	
										const TDesC8& /*aCustomMessage*/)
	{
	iAppUi->ChangeScreenLayoutL( ETrue );
	
	iContainer = CEmTubeServersViewContainer::NewL( *this, ClientRect() );
	iAppUi->AddToStackL(iContainer);	
	}
	

void CEmTubeServersView::DoDeactivate()
	{
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;
		
	}

void CEmTubeServersView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if( aResourceId == R_EMTV_SERVERS_VIEW_MENU_PANE )
		{
		aMenuPane->SetItemDimmed(EMTVSetAsDefaultServerCommand, EFalse );
		if( !iContainer->IsCurrentItemMore() )
			{
			CPluginInterface* plugin = iAppUi->PluginManager().Plugin( iContainer->CurrentItemIndex() );

			if( !(plugin->Capabilities() & KCapsLogin) )
				{
				aMenuPane->SetItemDimmed(EMTVLoginCommand, ETrue );
				}
			else
				{
				aMenuPane->SetItemDimmed(EMTVLoginCommand, EFalse );
				}
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVSetAsDefaultServerCommand, ETrue );
			aMenuPane->SetItemDimmed(EMTVLoginCommand, ETrue );
			}
		}
	}

void CEmTubeServersView::UpdateListL()
	{
	if( iContainer )
		iContainer->FillListL( iAppUi->PluginManager().Uid() );
	}

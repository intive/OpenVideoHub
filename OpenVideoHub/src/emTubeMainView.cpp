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
#include <akntitle.h>
#include <stringloader.h>
#include <coecntrl.h>
#include <eikedwin.h>
#include <Avkon.mbg>
#include <aknutils.h>

#include "emTubeMainView.h"
#include "emTubeAppUi.h"
#include "emTubeMainViewContainer.h"
#include "emTube.hrh"

#include "emTubeResource.h"

CEmTubeMainView* CEmTubeMainView::NewL()
	{
	CEmTubeMainView* self = CEmTubeMainView::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeMainView* CEmTubeMainView::NewLC()
	{
	CEmTubeMainView* self = new (ELeave) CEmTubeMainView();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;	
	}

CEmTubeMainView::~CEmTubeMainView()
	{
	if(iContainer)
		{
		delete iContainer;		  
		iContainer = NULL;
		}
	}

CEmTubeMainView::CEmTubeMainView()
	{
	
	}

void CEmTubeMainView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_MAIN_VIEW );
	}

TUid CEmTubeMainView::Id() const
	{
	return TUid::Uid( EMTVMainViewId );
	}

void CEmTubeMainView::HandleCommandL( TInt aCommand )
	{
	if( iContainer )
		iCurrentMenuItem = iContainer->CurrentItemIndex();

	switch(aCommand)
		{
		default:
			{
			iAppUi->HandleCommandL(aCommand);
			}
		}
	}
		
void CEmTubeMainView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
										TUid /*aCustomMessageId*/,	
										const TDesC8& /*aCustomMessage*/)
	{

	iAppUi->ChangeScreenLayoutL( EFalse );

#ifdef __S60_50__
	iAppUi->StopDisplayingPopupToolbar();
#endif
	
	iContainer = CEmTubeMainViewContainer::NewL(*this, ClientRect());
	iAppUi->AddToStackL(iContainer);	
	}
	

void CEmTubeMainView::DoDeactivate()
	{
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;		
	}

void CEmTubeMainView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if( aResourceId == R_EMTV_MAIN_VIEW_MENU_PANE )
		{
#ifdef ENABLE_SITE_SELECTION
		aMenuPane->SetItemDimmed(EMTVActivateServersViewCommand, EFalse );
#else
		aMenuPane->SetItemDimmed(EMTVActivateServersViewCommand, ETrue );
#endif

#ifdef ENABLE_CHECK_FOR_UPDATES_MENU_ITEM
		aMenuPane->SetItemDimmed(EMTVCheckForUpdate, EFalse );
#else
		aMenuPane->SetItemDimmed(EMTVCheckForUpdate, ETrue );
#endif

		if( iAppUi->PluginManager().Plugin()->Capabilities() & KCapsAccessVideoById )
			{
#ifdef ENABLE_OPEN_BY_ID_MENU_ITEM
			aMenuPane->SetItemDimmed(EMTVOpenVideoByIdCommand, EFalse );
#else
			aMenuPane->SetItemDimmed(EMTVOpenVideoByIdCommand, ETrue );
#endif
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVOpenVideoByIdCommand, ETrue );
			}
		}
	}

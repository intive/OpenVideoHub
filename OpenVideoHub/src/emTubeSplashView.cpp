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

#include "emTubeSplashView.h"
#include "emTubeAppUi.h"
#include "emTubeSplashViewContainer.h"
#include "emTube.hrh"

#include "emTubeResource.h"

CEmTubeSplashView* CEmTubeSplashView::NewL()
	{
	CEmTubeSplashView* self = CEmTubeSplashView::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeSplashView* CEmTubeSplashView::NewLC()
	{
	CEmTubeSplashView* self = new (ELeave) CEmTubeSplashView();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;	
	}

CEmTubeSplashView::~CEmTubeSplashView()
	{
	if(iContainer)
		{
		delete iContainer;		  
		iContainer = NULL;
		}
	}

CEmTubeSplashView::CEmTubeSplashView()
	{
	
	}

void CEmTubeSplashView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_MAIN_VIEW );

	Cba()->SetCommandSetL( R_AVKON_SOFTKEYS_EMPTY );
	Cba()->DrawDeferred();
	}

TUid CEmTubeSplashView::Id() const
	{
	return TUid::Uid( EMTVSplashViewId );
	}

void CEmTubeSplashView::HandleCommandL( TInt aCommand )
	{
	switch(aCommand)
		{
		default:
			{
			iAppUi->HandleCommandL( aCommand );
			}
		}
	}
		

void CEmTubeSplashView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
										TUid /*aCustomMessageId*/,	
										const TDesC8& /*aCustomMessage*/)
	{
	iContainer = CEmTubeSplashViewContainer::NewL(*this, ClientRect());
	iAppUi->AddToStackL(iContainer);	

	}
	

void CEmTubeSplashView::DoDeactivate()
	{
	CEikButtonGroupContainer* iToolBar;
	CEikStatusPane* iStatusPane;

	MEikAppUiFactory* f = CEikonEnv::Static()->AppUiFactory();
	iStatusPane = f->StatusPane();
	iToolBar = f->ToolBar();

	iStatusPane->MakeVisible( ETrue );
	iToolBar->MakeVisible( ETrue );

	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;
	}

void CEmTubeSplashView::DynInitMenuPaneL(TInt /*aResourceId*/, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}
	}

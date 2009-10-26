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

#include "emTubePlaylistManager.h"
#include "emTubePlaylistView.h"
#include "emTubeAppUi.h"
#include "emTubePlaylistViewContainer.h"
#include "emTube.hrh"
#include "emTubeResource.h"

CEmTubePlaylistView* CEmTubePlaylistView::NewL( CEmTubePlaylistManager* aManager )
	{
	CEmTubePlaylistView* self = CEmTubePlaylistView::NewLC( aManager );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylistView* CEmTubePlaylistView::NewLC( CEmTubePlaylistManager* aManager )
	{
	CEmTubePlaylistView* self = new (ELeave) CEmTubePlaylistView( aManager );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePlaylistView::~CEmTubePlaylistView()
	{
	if(iContainer)
		{
		delete iContainer;
		iContainer = NULL;
		}
	}

CEmTubePlaylistView::CEmTubePlaylistView( CEmTubePlaylistManager* aManager ) : iManager( aManager )
	{
	}

void CEmTubePlaylistView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_PLAYLIST_VIEW );
	}

TUid CEmTubePlaylistView::Id() const
	{
	return TUid::Uid( EMTVPlaylistViewId );
	}

void CEmTubePlaylistView::HandleCommandL( TInt aCommand )
	{
	switch(aCommand)
		{
		case EMTVAddPlaylistCommand:
			{
			TBuf<64> name;
			CAknTextQueryDialog* dlg = new(ELeave)CAknTextQueryDialog( name, CAknQueryDialog::ENoTone );
			dlg->SetPredictiveTextInputPermitted( ETrue );
			TInt ret = dlg->ExecuteLD( R_EMTV_ADD_PLAYLIST_DIALOG );
			if( ret )
				{
				iManager->AddPlaylistL( name, CEmTubePlaylist::EPlaylistUserDefined );
				iContainer->FillListL();
				}
			}
		break;

		case EMTVEditPlaylistCommand:
			{
			TInt idx = iManager->CurrentPlaylist();
			if( idx == KErrNotFound )
				{
				iManager->SetCurrentPlaylist( iContainer->CurrentItemIndex() );
				iContainer->FillListL();
				}

			iContainer->SetMode( CEmTubePlaylistViewContainer::EModeEdit );
			SetCbaL( R_EMTV_PLAYLIST_EDIT_CBA );
			}
		break;

		case EMTVFinishEditingPlaylistCommand:
			{
			iContainer->SetMode( CEmTubePlaylistViewContainer::EModeNormal );
			SetCbaL( R_AVKON_SOFTKEYS_OPTIONS_BACK );
			}
		break;

		case EMTVGrabPlaylistElementCommand:
			{
			iContainer->SetMode( CEmTubePlaylistViewContainer::EModeDrag );
			SetCbaL( R_EMTV_PLAYLIST_EDITING_CBA );
			}
		break;

		case EMTVDropPlaylistElementCommand:
			{
			iContainer->SetMode( CEmTubePlaylistViewContainer::EModeEdit );
			SetCbaL( R_EMTV_PLAYLIST_EDIT_CBA );
			}
		break;

		case EAknSoftkeyBack:
			{
			TInt idx = iManager->CurrentPlaylist();
			iManager->SetCurrentPlaylist( KErrNotFound );
			if( idx != KErrNotFound )
				{
				iContainer->FillListL( idx );
				}
			else
				{
				iAppUi->SetStartPlaybackMode( iStartPlaybackMode );
				iAppUi->HandleCommandL( EMTVActivateMainViewCommand );
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


void CEmTubePlaylistView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
										TUid /*aCustomMessageId*/,
										const TDesC8& /*aCustomMessage*/)
	{
	iStartPlaybackMode = iAppUi->StartPlaybackMode();
	iAppUi->SetStartPlaybackMode( CEmTubeAppUi::EStartPlaybackAsap );

	iAppUi->ChangeScreenLayoutL( ETrue );

	iContainer = CEmTubePlaylistViewContainer::NewL( *this, ClientRect(), iManager );
	iAppUi->AddToStackL(iContainer);
	}


void CEmTubePlaylistView::DoDeactivate()
	{
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;
	}

void CEmTubePlaylistView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if( aResourceId == R_EMTV_PLAYLIST_VIEW_MENU_PANE )
		{
		TInt idx = iManager->CurrentPlaylist();
		if( idx != KErrNotFound )
			{
			CEmTubePlaylist *p = iManager->Playlist( idx );
			if( p->Editable() && p->EntriesCount() > 1 )
				aMenuPane->SetItemDimmed(EMTVEditPlaylistCommand, EFalse );
			else
				aMenuPane->SetItemDimmed(EMTVEditPlaylistCommand, ETrue );

			aMenuPane->SetItemDimmed( EMTVAddPlaylistCommand, ETrue );
			}
		else
			{
			CEmTubePlaylist *p = iManager->Playlist( iContainer->CurrentItemIndex() );
			if( p->Editable() && p->EntriesCount() > 1 )
				aMenuPane->SetItemDimmed(EMTVEditPlaylistCommand, EFalse );
			else
				aMenuPane->SetItemDimmed(EMTVEditPlaylistCommand, ETrue );

			aMenuPane->SetItemDimmed( EMTVAddPlaylistCommand, EFalse );
			}
		}
	}

void CEmTubePlaylistView::SetCbaL( TInt aCbaResource )
	{
	Cba()->SetCommandSetL( aCbaResource );
	Cba()->DrawDeferred();
	}

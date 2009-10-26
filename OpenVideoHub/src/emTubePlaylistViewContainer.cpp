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

//TODO - add "remove from playlist"

#include <AknLists.h>
#include <Avkon.hrh>
#include <AknIconArray.h>
#include <AknIconUtils.h>
#include <AknNoteWrappers.h>
#include <AknUtils.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <aknnavilabel.h>
#include <eikclbd.h>
#include <barsread.h>
#include <gulicon.h>
#include <f32file.h>
#include <GDI.H>
#include <stringloader.h>
#include <apgcli.h>

#include <avkon.mbg>

#include "emTubeResource.h"
#include <PluginInterface.h>

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubePlaylistView.h"
#include "emTubePlaylistViewContainer.h"
#include "emTubePlaylistManager.h"

CEmTubePlaylistViewContainer* CEmTubePlaylistViewContainer::NewL( CEmTubePlaylistView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager )
	{
	CEmTubePlaylistViewContainer* self = CEmTubePlaylistViewContainer::NewLC( aView, aRect, aManager );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubePlaylistViewContainer* CEmTubePlaylistViewContainer::NewLC( CEmTubePlaylistView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager )
	{
	CEmTubePlaylistViewContainer* self = new ( ELeave ) CEmTubePlaylistViewContainer( aView, aManager );
	CleanupStack::PushL( self );
	self->ConstructL( aRect );
	return self;
	}

CEmTubePlaylistViewContainer::~CEmTubePlaylistViewContainer()
	{
	delete iListBox;

	if( iNaviPane )
		iNaviPane->Pop(NULL);

	delete iNaviLabelDecorator;
	}

CEmTubePlaylistViewContainer::CEmTubePlaylistViewContainer(CEmTubePlaylistView& aView, CEmTubePlaylistManager* aManager ) : iView( aView ), iManager( aManager ), iMode( EModeNormal)
	{
	}

void CEmTubePlaylistViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
//TODO - create an entry in help file
//    aContext.iContext = KContextPlaylist;
    }

void CEmTubePlaylistViewContainer::HandleListBoxEventL( CEikListBox* /*aListBox*/, TListBoxEvent aListBoxEvent )
	{
	switch( aListBoxEvent )
		{
		case EEventEnterKeyPressed:
		case EEventItemClicked:
			{
			switch( iMode )
				{
				case EModeNormal:
					{
					TInt plidx = iManager->CurrentPlaylist();
					TInt idx = iListBox->CurrentItemIndex();

					if( plidx != KErrNotFound )
						{
						if( idx != KErrNotFound )
							{
							iView.SetSelectedEntry( idx );
							CEmTubePlaylist *pl = iManager->Playlist( plidx );
							pl->SetCurrentEntry( idx );
							iAppUi->OpenFileL( pl->CurrentEntry() );
							}
						}
					else
						{
						iManager->SetCurrentPlaylist( idx );
						FillListL();
						}
					}
				break;

				case EModeEdit:
					{
					TInt idx = iListBox->CurrentItemIndex();
					if( idx != KErrNotFound )
						{
						iView.HandleCommandL( EMTVEditPlaylistCommand );
						}
					}
				break;

				case EModeDrag:
					{
					iView.HandleCommandL( EMTVFinishEditingPlaylistCommand );
					}
				break;

				}
			}
		break;

		default:
		break;
		}
	}

void CEmTubePlaylistViewContainer::ConstructL( const TRect& aRect )
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	CEikStatusPane *sp = iAppUi->StatusPane();
	iNaviPane = (CAknNavigationControlContainer*)sp->ControlL( TUid::Uid(EEikStatusPaneUidNavi) );
	iNaviLabelDecorator = iNaviPane->CreateNavigationLabelL();
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( KNullDesC() );
	iNaviPane->PushL( *iNaviLabelDecorator );

	CreateWindowL();

	SetNaviPaneTextL( R_PLAYLISTS_TXT );

//create list box
	iListBox = new ( ELeave ) CAknSingleGraphicStyleListBox;
	iListBox->SetContainerWindowL( *this );
	iListBox->ConstructL( this, EAknListBoxSelectionList );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
	iListBox->SetListBoxObserver( this );
	iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL( ETrue );

	CFbsBitmap *bitmap, *mask;
	CArrayPtr<CGulIcon>* icons = new(ELeave) CAknIconArray(8);
	CleanupStack::PushL(icons);

	//TODO - change icon to something different - internal not editable playlist
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubEmpty, EMbmOpenvideohubEmpty_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubEmpty, EMbmOpenvideohubEmpty_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));

	CleanupStack::Pop(icons);
	iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );

	TInt idx = 0;
	if( iManager->CurrentPlaylist() != KErrNotFound )
		idx = iView.SelectedEntry();
	FillListL( idx );

	SetRect(aRect);
	ActivateL();
	}

void CEmTubePlaylistViewContainer::SizeChanged()
	{
	if( iListBox )
		{
		iListBox->SetRect( Rect() );
		}
	}

void CEmTubePlaylistViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);
	SetRect(iView.ClientRect());
	DrawDeferred();
	}

void CEmTubePlaylistViewContainer::Draw( const TRect& /*aRect*/ ) const
	{
	}

TInt CEmTubePlaylistViewContainer::CountComponentControls() const
	{
	return 1;
	}

CCoeControl* CEmTubePlaylistViewContainer::ComponentControl(TInt aIndex) const
	{
	switch ( aIndex )
		{
		case 0:
			return iListBox;
		default:
			return NULL;
		}
	}

TKeyResponse CEmTubePlaylistViewContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
	TKeyResponse res = EKeyWasNotConsumed;

	if( aType == EEventKey )
		{
		switch (aKeyEvent.iScanCode)
			{
			case EStdKeyDownArrow:
				if( iMode == EModeDrag )
					{
					TInt idx = iListBox->CurrentItemIndex();
					CEmTubePlaylist* pl = iManager->Playlist( iManager->CurrentPlaylist() );
					CEmTubePlaylistEntry* pe = pl->RemoveEntry( idx );
					if( idx == pl->EntriesCount() )
						idx = 0;
					else
						idx++;

					pl->InsertEntry( pe, idx );

					FillListL( idx );
					res = EKeyWasConsumed;
					}
			break;

			case EStdKeyUpArrow:
				if( iMode == EModeDrag )
					{
					TInt idx = iListBox->CurrentItemIndex();
					CEmTubePlaylist* pl = iManager->Playlist( iManager->CurrentPlaylist() );
					CEmTubePlaylistEntry* pe = pl->RemoveEntry( idx );
					if( idx == 0 )
						idx = pl->EntriesCount();
					else
						idx--;

					pl->InsertEntry( pe, idx );

					FillListL( idx );
					res = EKeyWasConsumed;
					}
			break;

			default:
			break;
			}
		}


	if( iListBox && res == EKeyWasNotConsumed )
		res = iListBox->OfferKeyEventL( aKeyEvent, aType );

	return res;
	}

void CEmTubePlaylistViewContainer::FillListL( TInt aIndex )
	{
	RBuf text;
	CDesCArray* list = static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
	list->Reset();

	TInt plidx = iManager->CurrentPlaylist();
	if( plidx != KErrNotFound )
		{
		CEmTubePlaylist* pl = iManager->Playlist( plidx, ETrue );
		SetNaviPaneTextL( pl->Name() );
		for(TInt i=0;i<pl->EntriesCount();i++)
			{
			const TDesC& name = pl->Entry(i)->Name();
			CleanupClosePushL( text );

			text.CreateL( name.Length() + 10 + 2 );

			TInt idx = 1;

//TODO icon ->remote or saved clip!
			text.Format( _L("%d\t%S\t"), idx, &name );

			list->AppendL( text );
			CleanupStack::PopAndDestroy( &text );
			}
		}
	else
		{
		SetNaviPaneTextL( R_PLAYLISTS_TXT );
		for(TInt i=0;i<iManager->PlaylistsCount();i++)
			{
			CEmTubePlaylist* p = iManager->Playlist( i );
			const TDesC& name = p->Name();
			CleanupClosePushL( text );

			text.CreateL( name.Length() + 10 + 2 );

			TInt idx = 0;
			if( p->Type() == CEmTubePlaylist::EPlaylistUserDefined )
				idx = 1;

			text.Format( _L("%d\t%S\t"), idx, &name );

			list->AppendL( text );
			CleanupStack::PopAndDestroy( &text );
			}
		}
	iListBox->SetFocus( ETrue , EDrawNow );
	iListBox->HandleItemAdditionL();

	iListBox->SetCurrentItemIndexAndDraw( aIndex );
	}

void CEmTubePlaylistViewContainer::SetNaviPaneTextL( TInt aResource )
	{
	HBufC* paneTxt = StringLoader::LoadLC( aResource );
	SetNaviPaneTextL( *paneTxt );
	CleanupStack::PopAndDestroy( paneTxt );
	}

void CEmTubePlaylistViewContainer::SetNaviPaneTextL( const TDesC& aTxt )
	{
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( aTxt );
	iNaviLabelDecorator->DrawDeferred();
	}

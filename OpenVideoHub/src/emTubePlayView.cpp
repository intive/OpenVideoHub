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

#ifndef EMTUBE_UIQ
#include <aknviewappui.h>
#include <akntitle.h>
#include <stringloader.h>
#else
#include <QikAppUi.h>
#endif

#ifdef __S60_50__
#include <akntoolbar.h>
#endif

#include "emTubePlayView.h"
#ifndef EMTUBE_UIQ
#include "emTubePlayViewContainer.h"
#else
#include <QikListBoxModel.h>
#include <QikListBox.h>
#include <QikListBoxData.h>
#endif
#include <PluginInterface.h>

#include "emTubeAppUi.h"
#include "emTubeVideoEntry.h"

#include "emTube.hrh"
#include "emTubeResource.h"

CEmTubePlayView* CEmTubePlayView::NewL( CEmTubePlaylistManager* aManager )
	{
	CEmTubePlayView* self = CEmTubePlayView::NewLC( aManager );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlayView* CEmTubePlayView::NewLC( CEmTubePlaylistManager* aManager )
	{
	CEmTubePlayView* self = new (ELeave) CEmTubePlayView( aManager );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePlayView::~CEmTubePlayView()
	{
	delete iStatusPaneText;

	if(iContainer)
		{
		delete iContainer;
		iContainer = NULL;
		}
	delete iFileName;
	}

CEmTubePlayView::CEmTubePlayView( CEmTubePlaylistManager* aManager ) : iManager( aManager )
	{

	}

void CEmTubePlayView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_PLAY_VIEW );
	}

TUid CEmTubePlayView::Id() const
	{
	return TUid::Uid( EMTVPlayViewId );
	}

void CEmTubePlayView::HandleCommandL( TInt aCommand )
	{
	switch(aCommand)
		{
		case EMTVProgressDialogCancel:
			iAppUi->CancelDownloadMovieL();
			HandleCommandL( EAknSoftkeyBack );
		break;

	    case EMTVVideoSwitchToFullScreenCommand:
		case EMTVVideoBackFromFullScreenCommand:
		    iContainer->ChangeDisplayModeL();
		break;

		case EMTVVideoSaveCommand:
			iContainer->SaveFileL();
		break;

		case EAknSoftkeyBack:
			iContainer->ClosePlayerL();
		break;

		case EMTVVideoPauseCommand:
		case EMTVVideoPlayCommand:
			iContainer->PlayPauseL( EFalse );
		break;

		case EMTVVideoPropertiesCommand:
			iContainer->ShowPropertiesL();
		break;

		case EMTVShowRelatedCommand:
			{
			iAppUi->CancelDownloadMovieL();
			iAppUi->CancelDownloadImageL();
			CVideoEntry* e = iContainer->VideoEntry();
			iAppUi->SearchString().Copy( e->RelatedUrl() );
			iAppUi->ClearSearchResults();
			iAppUi->SearchL( ERelatedClips, ERelevance, EPeriodIrrelevant );
			}
		break;

		case EMTVShowUserCommand:
			{
			iAppUi->CancelDownloadMovieL();
			iAppUi->CancelDownloadImageL();
			CVideoEntry* e = iContainer->VideoEntry();
			iAppUi->SearchString().Copy( e->AuthorVideosUrl() );
			iAppUi->ClearSearchResults();
			iAppUi->SearchL( EUserClips, EUpdated, EPeriodIrrelevant );
			}
		break;

		default:
			iAppUi->HandleCommandL(aCommand);
		break;
		}
	}


void CEmTubePlayView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
										TUid /*aCustomMessageId*/,
										const TDesC8& /*aCustomMessage*/)
	{
#ifdef __S60_50__
	iAppUi->SetOrientationL( CAknAppUiBase::EAppUiOrientationLandscape );
#endif

	if( !iAppUi->Embedded() )
		{
		iAppUi->ChangeScreenLayoutL( ETrue );

		TUid titlePaneUid = TUid::Uid( EEikStatusPaneUidTitle );
		CEikStatusPaneBase::TPaneCapabilities subPaneTitle = StatusPane()->PaneCapabilities( titlePaneUid );
		if ( subPaneTitle.IsPresent() && subPaneTitle.IsAppOwned() )
			{
			CAknTitlePane* title = static_cast< CAknTitlePane* >( StatusPane()->ControlL( titlePaneUid ) );
			delete iStatusPaneText;
			iStatusPaneText = title->Text()->AllocL();
			}
		}

	iContainer = CEmTubePlayViewContainer::NewL(*this, ClientRect(), iManager );
	iAppUi->AddToStackL(iContainer);
	}


void CEmTubePlayView::DoDeactivate()
	{
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;

#ifdef __S60_50__
	iAppUi->SetOrientationL( CAknAppUiBase::EAppUiOrientationAutomatic );
#endif

	if( !iAppUi->Embedded() )
		{
		TUid titlePaneUid = TUid::Uid( EEikStatusPaneUidTitle );
		CEikStatusPaneBase::TPaneCapabilities subPaneTitle = StatusPane()->PaneCapabilities( titlePaneUid );
		if ( subPaneTitle.IsPresent() && subPaneTitle.IsAppOwned() )
			{
			CAknTitlePane* title = static_cast< CAknTitlePane* >( StatusPane()->ControlL( titlePaneUid ) );
			TRAP_IGNORE( title->SetTextL( *iStatusPaneText ) );
			}
		}
	}

void CEmTubePlayView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if( aResourceId == R_EMTV_PLAY_VIEW_MENU_PANE )
		{
		if( iContainer->IsPlaybackReady() )
			{
			aMenuPane->SetItemDimmed(EMTVVideoPropertiesCommand, EFalse );
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVVideoPropertiesCommand, ETrue );
			}

		if( iContainer->VideoEntry() && (iAppUi->PluginManager().Plugin()->Capabilities() & KCapsUserClips) &&
			iContainer->VideoEntry()->AuthorVideosUrl().Length() )
			{
			aMenuPane->SetItemDimmed(EMTVShowUserCommand, EFalse );
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVShowUserCommand, ETrue );
			}

		if( iContainer->VideoEntry() && (iAppUi->PluginManager().Plugin()->Capabilities() & KCapsRelatedClips) &&
			iContainer->VideoEntry()->RelatedUrl().Length() )
			{
			aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, EFalse );
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVShowRelatedCommand, ETrue );
			}

		if( iContainer->FileDownloaded() && iAppUi->PluginManager().Plugin()->Capabilities() & KCapsAllowSavingVideos )
			{
			aMenuPane->SetItemDimmed(EMTVVideoSaveCommand, EFalse );
			}
		else
			{
			aMenuPane->SetItemDimmed(EMTVVideoSaveCommand, ETrue );
			}
		}
	}

void CEmTubePlayView::PlayPauseL( TBool aForegroundEvent )
	{
	if( iContainer )
		iContainer->PlayPauseL( aForegroundEvent );
	}

void CEmTubePlayView::SetVideoEntry( CVideoEntry* aEntry )
	{
	delete iFileName;
	iFileName = KNullDesC().AllocL();
	iVideoEntry = aEntry;
	}

void CEmTubePlayView::SetFileNameL( const TDesC& aFileName )
	{
	iVideoEntry = NULL;
	delete iFileName;
	iFileName = aFileName.AllocL();
	}

void CEmTubePlayView::SetFileHandleL( RFile& aFile )
	{
	iVideoEntry = NULL;
	delete iFileName;
	iFileName = KNullDesC().AllocL();
	iFile.Duplicate( aFile );
	}

void CEmTubePlayView::OpenFileL( CVideoEntry* aEntry )
	{
	delete iFileName;
	iFileName = KNullDesC().AllocL();
	iVideoEntry = aEntry;

	if( iContainer )
		iContainer->OpenFileL();
	}

void CEmTubePlayView::OpenFileL( const TDesC& aFileName )
	{
	iVideoEntry = NULL;
	delete iFileName;
	iFileName = aFileName.AllocL();

	if( iContainer )
		iContainer->OpenFileL();
	}

void CEmTubePlayView::OpenFileL( RFile& aFile )
	{
	iVideoEntry = NULL;
	delete iFileName;
	iFileName = KNullDesC().AllocL();

	SetFileHandleL( aFile );
	if( iContainer )
		iContainer->OpenFileL();
	}

void CEmTubePlayView::SetCbaL( TInt aCbaResource )
	{
	Cba()->SetCommandSetL( aCbaResource );
	Cba()->DrawDeferred();
	}

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
#include <avkon.hrh>
#include <aknlists.h>
#include <aknquerydialog.h>
#include <aknnotewrappers.h>

#include "emTubeTransferView.h"
#include "emTubeAppUi.h"
#include "emTubeTransferViewContainer.h"
#include "emTubeTransferManager.h"

#include "emTube.hrh"
#include "emTubeResource.h"

CEmTubeTransferView* CEmTubeTransferView::NewL()
	{
	CEmTubeTransferView* self = CEmTubeTransferView::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeTransferView* CEmTubeTransferView::NewLC()
	{
	CEmTubeTransferView* self = new (ELeave) CEmTubeTransferView();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeTransferView::~CEmTubeTransferView()
	{
	if(iContainer)
		{
		delete iContainer;
		iContainer = NULL;
		}
	}

CEmTubeTransferView::CEmTubeTransferView()
	{

	}

void CEmTubeTransferView::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	BaseConstructL( R_EMTV_TRANSFER_VIEW );
	}

TUid CEmTubeTransferView::Id() const
	{
	return TUid::Uid( EMTVTransferViewId );
	}

void CEmTubeTransferView::HandleCommandL( TInt aCommand )
	{
	switch(aCommand)
		{
		case EAknSoftkeyBack:
			{
			iAppUi->HandleCommandL( EMTVActivateMainViewCommand );
			}
		break;

		case EMTVRemoveFromQueueCommand:
			{
            iContainer->RemoveEntryL();
            }
		break;

		case EMTVMoveUpCommand:
			{
            iContainer->MoveUpInQueueL();
			}
		break;

		case EMTVMoveDownCommand:
			{
            iContainer->MoveDownInQueueL();
			}
		break;

		case EMTVTransferManagerStartCommand:
			{
            iContainer->StartTransferManagerL( );
			}
		break;

		case EMTVTransferManagerStopCommand:
			{
            iContainer->StopTransferManagerL( );
			}
		break;

		case EMTVRemoveFinishedEntriesCommand:
			{
            iContainer->RemoveFinishedEntriesL( );
			}
		break;

		case EMTVTransferManagerRetryCommand:
			{
            iContainer->MarkEntryAsQueued( );
			}
		break;

		default:
			{
			iAppUi->HandleCommandL(aCommand);
			}
		break;
		}
	}


void CEmTubeTransferView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
										TUid /*aCustomMessageId*/,
										const TDesC8& /*aCustomMessage*/)
	{
	iAppUi->ChangeScreenLayoutL( ETrue );

	iContainer = CEmTubeTransferViewContainer::NewL(*this, ClientRect());
	iAppUi->AddToStackL(iContainer);
	}


void CEmTubeTransferView::DoDeactivate()
	{
	iAppUi->RemoveFromStack(iContainer);
	delete iContainer;
	iContainer = NULL;
	}

void CEmTubeTransferView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
	{
	if(aMenuPane != NULL)
		{
		aMenuPane->EnableMarqueeL(ETrue);
		}

	if( aResourceId == R_EMTV_TRANSFER_VIEW_MENU_PANE )
		{
        TInt queueCount = iAppUi->TransferManager().Queue().Count();
        TInt queueFinished = iAppUi->TransferManager().FinishedEntries();
        TInt queueFailed = iAppUi->TransferManager().FailedEntries();

		if( queueCount )
    		{
    		CEmTubeTransferManager* transferManager = &iAppUi->TransferManager();

    		if( transferManager->Processing() )
    			aMenuPane->SetItemDimmed( EMTVTransferManagerStopCommand, EFalse );
    		else
    			aMenuPane->SetItemDimmed( EMTVTransferManagerStopCommand, ETrue );

    		if( !transferManager->Processing() && ((queueCount-queueFailed) > queueFinished) )
    			aMenuPane->SetItemDimmed( EMTVTransferManagerStartCommand, EFalse );
    		else
    			aMenuPane->SetItemDimmed( EMTVTransferManagerStartCommand, ETrue );

    		if( queueFinished )
    			aMenuPane->SetItemDimmed( EMTVRemoveFinishedEntriesCommand, EFalse );
    		else
    			aMenuPane->SetItemDimmed( EMTVRemoveFinishedEntriesCommand, ETrue );

    		if( transferManager->Queue()[iContainer->CurrentItemIndex()]->Status() == CQueueEntry::EEntryFinished )
    			{
    			aMenuPane->SetItemDimmed( EMTVMoveUpCommand, ETrue );
    			aMenuPane->SetItemDimmed( EMTVMoveDownCommand, ETrue );
    			}
			else
				{
	    		TInt currentItem = iContainer->CurrentItemIndex() + 1;
	    		if( currentItem > 1 )
	    			aMenuPane->SetItemDimmed( EMTVMoveUpCommand, EFalse );
	    		else
	    			aMenuPane->SetItemDimmed( EMTVMoveUpCommand, ETrue );

	    		if( currentItem < queueCount )
	    			aMenuPane->SetItemDimmed( EMTVMoveDownCommand, EFalse );
	    		else
	    			aMenuPane->SetItemDimmed( EMTVMoveDownCommand, ETrue );
				}

    		if( transferManager->Queue()[iContainer->CurrentItemIndex()]->Status()
    		        == CQueueEntry::EEntryFailed )
    			aMenuPane->SetItemDimmed( EMTVTransferManagerRetryCommand, EFalse );
    		else
    			aMenuPane->SetItemDimmed( EMTVTransferManagerRetryCommand, ETrue );

    		if( transferManager->Queue()[iContainer->CurrentItemIndex()]->Status()
    		        == CQueueEntry::EEntryFinished )
    			aMenuPane->SetItemDimmed( EMTVRemoveFromQueueCommand, ETrue );
    		else
    			aMenuPane->SetItemDimmed( EMTVRemoveFromQueueCommand, EFalse );

    		}
    	else
        	{
			aMenuPane->SetItemDimmed( EMTVRemoveFromQueueCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVTransferManagerStartCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVTransferManagerStopCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVRemoveFinishedEntriesCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVMoveUpCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVMoveDownCommand, ETrue );
			aMenuPane->SetItemDimmed( EMTVTransferManagerRetryCommand, ETrue );
        	}
		}
	}

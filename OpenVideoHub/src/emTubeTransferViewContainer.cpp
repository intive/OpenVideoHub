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

#include <AknIconArray.h>
#include <GULICON.H>
#include <GULUTIL.H>
#include <stringloader.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <aknnavilabel.h>

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubeTransferViewContainer.h"
#include "emTubeVideoEntry.h"
#include "emTubeImageLoader.h"
#include "emTubeTransferManager.h"
#include "emTubeTimeOutTimer.h"

CEmTubeTransferViewContainer* CEmTubeTransferViewContainer::NewL(CEmTubeTransferView& aView, const TRect& aRect)
	{
	CEmTubeTransferViewContainer* self = CEmTubeTransferViewContainer::NewLC(aView, aRect);
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeTransferViewContainer* CEmTubeTransferViewContainer::NewLC(CEmTubeTransferView& aView, const TRect& aRect)
	{
	CEmTubeTransferViewContainer* self = new (ELeave) CEmTubeTransferViewContainer(aView);
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}
	
CEmTubeTransferViewContainer::~CEmTubeTransferViewContainer()
	{
	if( iTimer )
		iTimer->Cancel();
	delete iTimer;

	delete iListBox;

	if( iNaviPane )
		iNaviPane->Pop(NULL); 
	
	delete iNaviLabelDecorator;
	}

CEmTubeTransferViewContainer::CEmTubeTransferViewContainer(CEmTubeTransferView& aView) : iView(aView),
																						 iEntry(NULL)
	{
	}

void CEmTubeTransferViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextTransferMgr;
    }

void CEmTubeTransferViewContainer::ConstructL(const TRect& aRect)
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

    iTransferManager = &iAppUi->TransferManager();

	CEikStatusPane *sp = iAppUi->StatusPane();
	iNaviPane = (CAknNavigationControlContainer*)sp->ControlL( TUid::Uid(EEikStatusPaneUidNavi) );
	iNaviLabelDecorator = iNaviPane->CreateNavigationLabelL();
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( KNullDesC() );
	iNaviPane->PushL( *iNaviLabelDecorator );

	iTimer = CEmTubeTimeOutTimer::NewL( *this );

	CreateWindowL();

	HBufC* paneTxt = StringLoader::LoadLC( R_TRANSFER_MANAGER_TXT );
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( *paneTxt );
	iNaviLabelDecorator->DrawDeferred();
	CleanupStack::PopAndDestroy( paneTxt );

//create list box
	iListBox = new ( ELeave ) CAknDoubleGraphicStyleListBox;
	iListBox->SetContainerWindowL( *this );
	iListBox->ConstructL( this, EAknListBoxSelectionList );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
	iListBox->SetListBoxObserver( this );
	iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );

	HBufC* txt = StringLoader::LoadLC( R_TRANSFER_MANAGER_QUEUE_EMPTY_TXT );
	iListBox->View( )->SetListEmptyTextL( *txt );
	CleanupStack::PopAndDestroy( txt );

	CFbsBitmap *bitmap, *mask;
	CArrayPtr<CGulIcon>* icons = new(ELeave) CAknIconArray(8);
	CleanupStack::PushL(icons);

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_upload, EMbmOpenvideohubIcn_upload_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));
	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubIcn_download, EMbmOpenvideohubIcn_download_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));
	CleanupStack::Pop(icons);
	iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );
	iListBox->SetFocus( ETrue , EDrawNow );		

//add elements!
	UpdateListL( 0 );
	
	SetRect(aRect);	
	ActivateL();

	iTimer->After( 2 * 1000000, 0 );
	}
		
TInt CEmTubeTransferViewContainer::CountComponentControls() const
	{
	return 1;
	}
	
CCoeControl* CEmTubeTransferViewContainer::ComponentControl( TInt aIndex ) const
	{
	if( aIndex == 0 )
		return iListBox;
	
	return NULL;
	}
	
void CEmTubeTransferViewContainer::SizeChanged()
	{
	if( iListBox )
		{
		iListBox->SetRect( Rect() );
		}
	}

void CEmTubeTransferViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);
	SetRect(iView.ClientRect());    
	DrawDeferred();
	}

TKeyResponse CEmTubeTransferViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
    if( aType == EEventKey )
        {
        switch (aKeyEvent.iScanCode)
            {
            case EStdKeyBackspace:
   			case EStdKeyDelete:
   			    {
                iView.HandleCommandL( EMTVRemoveFromQueueCommand );
   			    }
   			break;
            
            default:
                break;
            }
        }

	if( iListBox )
		return iListBox->OfferKeyEventL( aKeyEvent, aType );
	
	return EKeyWasNotConsumed;
	}

void CEmTubeTransferViewContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
	{
	switch( aEventType )
		{
		case EEventEnterKeyPressed:
		case EEventItemClicked:
			{
			switch( CurrentItemIndex() )
				{
				}
			}
		break;

		default:
			break;
		}

	}

void CEmTubeTransferViewContainer::Draw(const TRect& /*aRect*/) const
	{
	}

void CEmTubeTransferViewContainer::AddStatusTextL( RBuf& aBuf, TInt aResource, TInt aAdditionalLength )
    {
    HBufC* txt = StringLoader::LoadLC( aResource );
	aBuf.ReAllocL( aBuf.Length() + txt->Length() + aAdditionalLength );
	aBuf.Append( *txt );
    CleanupStack::PopAndDestroy( txt );                    
    }

void CEmTubeTransferViewContainer::CreateLineL( RBuf& aBuf, CQueueEntry* aEntry )
	{
	TBuf<6> icon;
	if( aEntry->Type() == CQueueEntry::EEntryDownload )
		icon.Format( _L("%d\t"), 1 );
	else
		icon.Format( _L("%d\t"), 0 );

	aBuf.Create( aEntry->Title().Length() + 1 + icon.Length() );
	aBuf.Append( icon );
	aBuf.Append( aEntry->Title() );
	aBuf.Append( _L("\t") );

	switch( aEntry->Status() )
		{
		case CQueueEntry::EEntryFailed:
			{
			AddStatusTextL( aBuf , R_MANAGER_ENTRY_FAILED_TXT );
			}
		break;

		case CQueueEntry::EEntryQueued:
			{
			AddStatusTextL( aBuf , R_MANAGER_ENTRY_QUEUED_TXT );
			}
		break;

		case CQueueEntry::EEntryFinished:
			{
			AddStatusTextL( aBuf , R_MANAGER_ENTRY_FINISHED_TXT );
			}
		break;

		case CQueueEntry::EEntryInitializing:
			{
			AddStatusTextL( aBuf , R_MANAGER_ENTRY_INITIALIZING_TXT );
			}
		break;

		case CQueueEntry::EEntryUploading:
		case CQueueEntry::EEntryDownloading:
			{
			TBuf<128> tmp;
			if( aEntry->Size() != -1 )
				{
				TUint32 percentage = (TUint32)( ( (TReal)aEntry->CurrentSize() * 100.0f ) / (TReal)aEntry->Size() );
				tmp.Format( _L(" %d%% of %.2f MB"), percentage, (float)aEntry->Size()/(1024.0*1024.0) );
				}
			else
				{
				tmp.Format( _L(" %.2f MB"), (float)aEntry->CurrentSize()/(1024.0*1024.0) );
				}

			if( aEntry->Status() == CQueueEntry::EEntryUploading )
				{
    			AddStatusTextL( aBuf , R_MANAGER_ENTRY_UPLOADING_TXT, tmp.Length() );
				}
			else if( aEntry->Status() == CQueueEntry::EEntryDownloading )
				{
    			AddStatusTextL( aBuf , R_MANAGER_ENTRY_DOWNLOADING_TXT, tmp.Length() );
				}

            aBuf.Append( tmp );
			}
		break;
		}
	}

void CEmTubeTransferViewContainer::UpdateListL( TInt aIdx )
	{
	CDesCArray* list = static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
	list->Reset();
	RPointerArray<CQueueEntry>& queue = iAppUi->TransferManager().Queue();
	for( TInt i=0;i<queue.Count();i++ )
		{
		CQueueEntry* e = queue[i];
		
		RBuf line;
		CleanupClosePushL(line);
		CreateLineL( line, e );
		list->AppendL(line);
		CleanupStack::PopAndDestroy(&line);
		}

	iListBox->HandleItemAdditionL();

	if( list->Count() )
		{
		if( aIdx < 0 )
			aIdx = 0;
		else if ( aIdx >= list->Count() )
			aIdx = list->Count() - 1;
		}
	else
		{
		aIdx = 0;
		}
	iListBox->SetCurrentItemIndexAndDraw( aIdx );
	iListBox->DrawNow();
	}

void CEmTubeTransferViewContainer::TimerExpired( TInt /*aMode*/ )
	{
	CDesCArray* list = static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
	CQueueEntry* entry = iAppUi->TransferManager().CurrentEntry();

	if( !list->Count() || iEntry != entry )
		{
		UpdateListL( CurrentItemIndex() );
		}
	iEntry = entry;

	if( entry )
		{
		TInt index = iAppUi->TransferManager().Queue().Find( entry );
		if( index != KErrNotFound )
			{
			list->Delete( index );
			RBuf line;
			CleanupClosePushL( line );
			CreateLineL( line, entry );
			list->InsertL( index, line );
			CleanupStack::PopAndDestroy( &line );

			iListBox->HandleItemAdditionL();
			iListBox->DrawNow();

			}
		}
	iTimer->After( 2 * 1000000, 0 );
	}

void CEmTubeTransferViewContainer::RemoveEntryL()
    {
    TBool userResponse = iAppUi->ConfirmationQueryL( R_REMOVE_ENTRY_FROM_MANAGER_TXT ); 
    
	if( userResponse )
        {
        TBool processing = iTransferManager->Processing();
        
        iTransferManager->RemoveFromQueueL( CurrentItemIndex() );
    	
    	UpdateListL( CurrentItemIndex() - 1 );

    	if( processing )
    	    iTransferManager->StartL( ETrue );
        }        
    }

void CEmTubeTransferViewContainer::StartTransferManagerL()
    {
    iTransferManager->StartL( ETrue );
	UpdateListL( CurrentItemIndex() );            
    }
        
void CEmTubeTransferViewContainer::StopTransferManagerL()
    {
    iTransferManager->StopL();
	UpdateListL( CurrentItemIndex() );
    }    

void CEmTubeTransferViewContainer::RemoveFinishedEntriesL()
    {
    iTransferManager->RemoveFinishedEntriesL();    
	UpdateListL( 0 );
    }

void CEmTubeTransferViewContainer::MoveUpInQueueL( )
	{
    TBool processing = iTransferManager->Processing();
    
    if( !iTransferManager->SafeToMoveEntry( CurrentItemIndex(), ETrue ) )
        {
    	if( iAppUi->ConfirmationQueryL( R_MOVE_ENTRY_WARNING_TXT ) )
            iTransferManager->StopL();    	    
    	else
    	    return;
        }
    
    iTransferManager->MoveUpInQueueL( CurrentItemIndex() );

	UpdateListL( CurrentItemIndex() - 1 );            
	
	if( processing )
        iTransferManager->StartL( ETrue );    	    
	}

void CEmTubeTransferViewContainer::MoveDownInQueueL( )
	{
    TBool processing = iTransferManager->Processing();
    
    if( !iTransferManager->SafeToMoveEntry( CurrentItemIndex(), EFalse) )
        {
    	if( iAppUi->ConfirmationQueryL( R_MOVE_ENTRY_WARNING_TXT ) )
            iTransferManager->StopL();    	    
    	else
    	    return;
        }
    
    iTransferManager->MoveDownInQueueL( CurrentItemIndex() );

	UpdateListL( CurrentItemIndex() + 1 );
	
	if( processing )
        iTransferManager->StartL( ETrue );    	    
	}

TInt CEmTubeTransferViewContainer::CurrentItemIndex() 
    { 
    return iListBox->CurrentItemIndex(); 
    }

void CEmTubeTransferViewContainer::MarkEntryAsQueued()
    {
    iTransferManager->Queue()[iListBox->CurrentItemIndex()]->SetStatus( CQueueEntry::EEntryQueued );    
	UpdateListL( CurrentItemIndex() );

	if( !iTransferManager->Processing() )
        iTransferManager->StartL( ETrue );    	    
    }

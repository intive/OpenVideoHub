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


#include <eikmenub.h>
#include <eikcapc.h>
#include <eikedwin.h>
#include <aknappui.h>
#include <aknselectionlist.h>
#include <aknpopupfieldtext.h>
#include <stringloader.h>
#include <aknnotewrappers.h> 

#include "emTubeResource.h"
#include "emTube.hrh"
#include "emTubeVideoUploadDialog.h"
#include "emTubeTransferManager.h"

CEmTubeVideoUploadDialog::CEmTubeVideoUploadDialog( CQueueEntry& aEntry ) : iQueueEntry(&aEntry)
	{
	}

void CEmTubeVideoUploadDialog::ConstructL()
	{
	CAknForm::ConstructL( R_UPLOAD_VIDEO_DIALOG_MENU_BAR );

	CEikStatusPane* statusPane = iAvkonAppUi->StatusPane();
	iTitlePane = (CAknTitlePane*) statusPane->ControlL( TUid::Uid(EEikStatusPaneUidTitle) );
	iOriginalTitle = iTitlePane->Text()->AllocL();
	HBufC* txt = StringLoader::LoadLC( R_UPLOAD_VIDEO_TXT );
	iTitlePane->SetTextL( *txt );
	CleanupStack::PopAndDestroy( txt );

	}

CEmTubeVideoUploadDialog* CEmTubeVideoUploadDialog::NewLC( CQueueEntry& aEntry )
	{
	CEmTubeVideoUploadDialog* self = new (ELeave) CEmTubeVideoUploadDialog( aEntry );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeVideoUploadDialog::~CEmTubeVideoUploadDialog()
	{
	if (iTitlePane && iOriginalTitle)
		{
		iTitlePane->SetText(iOriginalTitle);
		}
	}

void CEmTubeVideoUploadDialog::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
	{
	CAknForm::DynInitMenuPaneL(aResourceId, aMenuPane);
	if (aResourceId == R_AVKON_FORM_MENUPANE)
		{
		aMenuPane->SetItemDimmed(EAknFormCmdLabel, ETrue); 
		aMenuPane->SetItemDimmed(EAknFormCmdAdd, ETrue); 
		aMenuPane->SetItemDimmed(EAknFormCmdDelete, ETrue); 
		aMenuPane->SetItemDimmed(EAknFormCmdSave, ETrue);
		}
	}

void CEmTubeVideoUploadDialog::ProcessCommandL(TInt aCommandId)
	{
	CAknForm::ProcessCommandL(aCommandId);
	}

TBool CEmTubeVideoUploadDialog::OkToExitL(TInt aButtonId)
	{
	if (aButtonId == EAknSoftkeyDone)
		{
		SaveFormDataL();

	    if( iQueueEntry->Title().Length() == 0 )
	    	{
			HBufC* message = StringLoader::LoadLC( R_DIALOG_TITLE_AND_DESCRIPTION_NEEDED_TXT );
            CAknInformationNote* informationNote = new (ELeave) CAknInformationNote;
            informationNote->ExecuteLD( *message );                    
			CleanupStack::PopAndDestroy( message );
    	    return EFalse;    	    
		    }
		
	    if( iQueueEntry->Description().Length() == 0 )
	    	{
			HBufC* message = StringLoader::LoadLC( R_DIALOG_TITLE_AND_DESCRIPTION_NEEDED_TXT );
            CAknInformationNote* informationNote = new (ELeave) CAknInformationNote;
            informationNote->ExecuteLD( *message );                    
			CleanupStack::PopAndDestroy( message );
    	    return EFalse;    	    
		    }

		TBool res = ETrue;

	    if( iQueueEntry->Tags().Length() == 0 )
	        res = EFalse;
    
	    TLex lex = TLex( iQueueEntry->Tags() );            
	    TPtrC16 ptr( lex.NextToken() );
	    TInt tokenCount = 0;
    
	    while( ptr.Length() && (tokenCount < 2) )
    	    {
        	tokenCount++;
	        ptr.Set( lex.NextToken() );
    	    }

		if( tokenCount < 2 )
			res = EFalse;
		
		if( !res )
			{
			HBufC* message = StringLoader::LoadLC( R_DIALOG_TWO_TAGS_NEEDED_TXT );
            CAknInformationNote* informationNote = new (ELeave) CAknInformationNote;
            informationNote->ExecuteLD( *message );                    
			CleanupStack::PopAndDestroy( message );
    	    return EFalse;    	    
			}

  		return ETrue;
		}
	else
		{
		return CAknForm::OkToExitL(aButtonId);
		}
	}

void CEmTubeVideoUploadDialog::PreLayoutDynInitL()
	{
	SetEditableL(ETrue);
	}

void CEmTubeVideoUploadDialog::PostLayoutDynInitL()
	{
	SetChangesPending(ETrue);
	CAknForm::PostLayoutDynInitL();
	}

void CEmTubeVideoUploadDialog::SetInitialCurrentLine()
	{
	CAknForm::SetInitialCurrentLine();
	}

void CEmTubeVideoUploadDialog::HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType )
	{
	CAknForm::HandleControlEventL(aControl, aEventType);
	}

const TMovieCategory KCategories[] = 
	{
	EFilmAndAnimation,
	ECarsAndVehicles,
	EMusic,
	EPetsAndAnimals,
	ESports,
	ETravelAndEvents,
	EPeopleAndBlogs,
	EComedy,
	EEntertainment,
	ENewsAndPolitics,
	EHowtoAndStyle,
	EEducation,
	EScienceAndTechnology,
	ENonprofitsAndActivism
	};

TBool CEmTubeVideoUploadDialog::SaveFormDataL()
	{
	TBuf<KUploadVideoMaxFieldLength> data;

	CEikEdwin* editor;

	editor = static_cast <CEikEdwin*> (Control(EUploadVideoFieldTitle));
	editor->GetText( data );
	iQueueEntry->SetTitleL( data );

	editor = static_cast <CEikEdwin*> (Control(EUploadVideoFieldDescription));
	editor->GetText( data );
	iQueueEntry->SetDescriptionL( data );

	editor = static_cast <CEikEdwin*> (Control(EUploadVideoFieldTags));
	editor->GetText( data );
	iQueueEntry->SetTagsL( data );


	CAknPopupFieldText* popup = static_cast<CAknPopupFieldText*>(ControlOrNull(EUploadVideoFieldPublic));
	switch ( popup->CurrentValueIndex() )
		{
		case 0:
			iQueueEntry->SetPublic( ETrue );
		break;

		case 1:
			iQueueEntry->SetPublic( EFalse );
		break;
		}

	popup = static_cast<CAknPopupFieldText*>(ControlOrNull(EUploadVideoFieldCategory));
	iQueueEntry->SetCategory( KCategories[ popup->CurrentValueIndex() ] );

	return ETrue;
	}

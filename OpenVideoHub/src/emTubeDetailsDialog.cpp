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

#include <akntitle.h> 
#include <stringloader.h>
#include <aknquerydialog.h>
#include <aknlists.h>

#include "emTubeResource.h"

#include "emTubeDetailsDialog.h"
#include "emTubeVideoEntry.h"

CEmTubeDetailsDialog::CEmTubeDetailsDialog(TInt& aSelectedItem, CArrayFixFlat<TInt>* aMarkedItems, CDesCArray* aListItems)
	: CAknMarkableListDialog(aSelectedItem, aMarkedItems, aListItems, R_EMTV_DETAILS_DIALOG_MENUBAR, -1, NULL), iList(aListItems)
	{
	}

CEmTubeDetailsDialog::~CEmTubeDetailsDialog()
	{
	delete iList;
	}

void CEmTubeDetailsDialog::ConstructL(TUint aMenuId)
	{
	CAknMarkableListDialog::ConstructL(aMenuId);
	}

TInt CEmTubeDetailsDialog::RunDialogL( CVideoEntry* aEntry )
	{
	CDesCArray* list = new ( ELeave ) CDesCArrayFlat( 2 );
	CleanupStack::PushL(list);
	
	TInt selectedItem = 0;
	CEmTubeDetailsDialog* dialog = new (ELeave) CEmTubeDetailsDialog(selectedItem, NULL, list);
	CleanupStack::Pop(list);

	dialog->iVideoEntry = aEntry;

	dialog->FillListL();
	dialog->ConstructL( R_EMTV_DETAILS_DIALOG_MENUBAR );
	dialog->PrepareLC( R_EMTV_DETAILS_DIALOG );
	TInt res = dialog->RunLD();

	return res;
	}

void CEmTubeDetailsDialog::FillListL()
	{
	RBuf line;

	HBufC* header = StringLoader::LoadLC( R_DETAILS_DIALOG_TITLE_TXT );
	CleanupClosePushL(line);
	line.ReAllocL( header->Length() + iVideoEntry->MediaTitle().Length() + KDialogLineTextTxt().Length() );
	line.Format( KDialogLineTextTxt, header, &iVideoEntry->MediaTitle() );
	iList->AppendL(line);
	CleanupStack::PopAndDestroy( &line );
	CleanupStack::PopAndDestroy( header );

	header = StringLoader::LoadLC( R_DETAILS_DIALOG_AUTHOR_TXT );
	CleanupClosePushL(line);
	line.ReAllocL( header->Length() + iVideoEntry->AuthorName().Length() + KDialogLineTextTxt().Length() );
	line.Format( KDialogLineTextTxt, header, &iVideoEntry->AuthorName() );
	iList->AppendL(line);
	CleanupStack::PopAndDestroy( &line );
	CleanupStack::PopAndDestroy( header );

	HBufC* timeFormatString = CEikonEnv::Static()->AllocReadResourceLC( R_QTN_TIME_DURAT_MIN_SEC );
	header = StringLoader::LoadLC( R_DETAILS_DIALOG_DURATION_TXT );
	CleanupClosePushL(line);
	line.ReAllocL( header->Length() + 20 + timeFormatString->Length() );
	TBuf<128> durText;
	TTime etime(0);
	etime += (TTimeIntervalSeconds) iVideoEntry->Duration();
	etime.FormatL( durText, *timeFormatString);
	line.Format( KDialogLineTextTxt, header, &durText );
	iList->AppendL(line);
	CleanupStack::PopAndDestroy( &line );
	CleanupStack::PopAndDestroy( header );
	CleanupStack::PopAndDestroy( timeFormatString );

	header = StringLoader::LoadLC( R_DETAILS_DIALOG_VIEWCOUNT_TXT );
	CleanupClosePushL(line);
	line.ReAllocL( header->Length() + 20 + KDialogLineNumberTxt().Length() );
	line.Format( KDialogLineNumberTxt, header, iVideoEntry->ViewCount() );
	iList->AppendL(line);
	CleanupStack::PopAndDestroy( &line );
	CleanupStack::PopAndDestroy( header );

	header = StringLoader::LoadLC( R_DETAILS_DIALOG_RATINGS_TXT );
	CleanupClosePushL(line);
	line.ReAllocL( header->Length() + 20 + KDialogLineDoubleTxt().Length() );
	line.Format( KDialogLineDoubleTxt,  header, iVideoEntry->AverageRating() );
	iList->AppendL(line);
	CleanupStack::PopAndDestroy( &line );
	CleanupStack::PopAndDestroy( header );

	if( iVideoEntry->VideoFileSize() )
		{
		header = StringLoader::LoadLC( R_DETAILS_DIALOG_FILESIZE_TXT );
		CleanupClosePushL(line);
		line.ReAllocL( header->Length() + 20 + KDialogLineNumberTxt().Length() );
		line.Format( KDialogLineNumberTxt,  header, iVideoEntry->VideoFileSize() );
		iList->AppendL(line);
		CleanupStack::PopAndDestroy( &line );
		CleanupStack::PopAndDestroy( header );
		}
	}
	
void CEmTubeDetailsDialog::PostLayoutDynInitL()
	{
	CAknMarkableListDialog::PostLayoutDynInitL();
	}

TKeyResponse CEmTubeDetailsDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
	{
	TKeyResponse ret = EKeyWasNotConsumed;
	if( aType == EEventKey )
		{
		switch (aKeyEvent.iScanCode)
			{
			case EStdKeyYes:
				{
				HandleListBoxEventL( NULL, EEventItemClicked );
				ret = EKeyWasConsumed;
				}
			break;
			
			case EStdKeyEnter:
			case EStdKeyDevice3:
				{
				TryExitL(EAknSoftkeyOk);
				ret = EKeyWasConsumed;
				}
			break;
			
			default:
			break;
			}
		}
	if( ret == EKeyWasNotConsumed )
		return CAknMarkableListDialog::OfferKeyEventL(aKeyEvent, aType);
	else
		return ret;
	}

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

#ifndef EMTUBE_DETAILS_DIALOG_H
#define EMTUBE_DETAILS_DIALOG_H

#include <e32base.h>
#include <aknselectionlist.h>

_LIT(KDialogLineTextTxt,"\t%S\t%S\t\t");
_LIT(KDialogLineNumberTxt,"\t%S\t%d\t\t");
_LIT(KDialogLineDoubleTxt, "\t%S\t%.2f\t\t");

class CVideoEntry;

class CEmTubeDetailsDialog : public CAknMarkableListDialog
	{
public: // Constructors and destructor
	static TInt RunDialogL( CVideoEntry* aEntry );
	~CEmTubeDetailsDialog();

protected: // From CEikDialog
	void ConstructL( TUint aMenuId );
	void PostLayoutDynInitL();

	void FillListL();

	TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

private:
	CEmTubeDetailsDialog(TInt& aSelectedItem, CArrayFixFlat<TInt>* aMarkedItems, CDesCArray* aListItems);

private: //data

	CVideoEntry* iVideoEntry;
	CDesCArray* iList;
	};

#endif //EMTUBE_DETAILS_DIALOG_H

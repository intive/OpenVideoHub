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


#include <FBS.H>
#include <GULICON.H>
#include <BITSTD.H>
#include <BITDEV.H>
#include <aknutils.h> 

#include "emTubeUiItem.h"

CUiItem* CUiItem::NewL( TInt aIconIdx, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData  )
	{
	CUiItem* self = CUiItem::NewLC( aIconIdx, aFirstLine, aSecondLine, aPrivateData );
	CleanupStack::Pop(self);
	return self;
	}

CUiItem* CUiItem::NewLC( TInt aIconIdx, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData  )
	{
	CUiItem* self = new (ELeave) CUiItem( aIconIdx, aPrivateData );
	CleanupStack::PushL(self);
	self->ConstructL( aFirstLine, aSecondLine );
	return self;
	}

void CUiItem::ConstructL( const TDesC& aFirstLine, const TDesC& aSecondLine )
	{
	iFirstLine = aFirstLine.AllocL();
	iSecondLine = aSecondLine.AllocL();
	}	
	
CUiItem::~CUiItem()
	{
	delete iFirstLine;
	delete iSecondLine;
	}

CUiItem::CUiItem( TInt aIconIdx, TAny* aPrivateData ): iIconIdx(aIconIdx), iPrivateData(aPrivateData)
	{
	}

TDesC& CUiItem::FirstLine()
	{
	return *iFirstLine;
	}

void CUiItem::SetFirstLineL()
	{
	}

TDesC& CUiItem::SecondLine()
	{
	return *iSecondLine;
	}

void CUiItem::SetSecondLineL()
	{
	}


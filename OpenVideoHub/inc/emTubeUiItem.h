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

#ifndef EMTUBE_UI_ITEM_H
#define EMTUBE_UI_ITEM_H

#include <e32base.h>

class CUiItem : public CBase
	{
public:

	static CUiItem* NewL( TInt iIconIdx, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData );
	static CUiItem* NewLC( TInt iIconIdx, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData );
	
	~CUiItem();

private: 
	CUiItem( TInt iIconIdx, TAny* aPrivateData );
	void ConstructL( const TDesC& aFirstLine, const TDesC& aSecondLine );

public:
	TDesC& FirstLine();
	void SetFirstLineL();

	TDesC& SecondLine();
	void SetSecondLineL();

	TInt IconIdx() { return iIconIdx; }
	void SetIconIdx( TInt aIconIdx ) { iIconIdx = aIconIdx; }
		
   	TReal Angle() { return iAngle; }
   	void SetAngle( TReal aAngle ) { iAngle = aAngle; }

   	TReal Scale() { return iScale; }
   	void SetScale( TReal aScale ) { iScale = aScale; }
    
   	TAny* PrivateData() { return iPrivateData; }
   	void SetPrivateData( TAny* aPrivateData ) { iPrivateData = aPrivateData; }
   
private:
	HBufC* iFirstLine;
	HBufC* iSecondLine;
	TInt iIconIdx;
	TReal iAngle;
	TReal iScale;
	
	TAny* iPrivateData;

	};

#endif // EMTUBE_UI_ITEM_H

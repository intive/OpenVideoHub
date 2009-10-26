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

#ifndef EMTUBE_UI_ITEM_GFX_H
#define EMTUBE_UI_ITEM_GFX_H

#include <e32base.h>
#include <gdi.h>
#include <bitstd.h>

class CFbsBitmap;
class CGulIcon;
class CFont;

class UiItemGfx
	{
public:
    enum TCustomUiFont
    {
        EFontSmall = 0,
        EFontLarge
    };

public:   
	static void BltIconScale( CFbsBitmap* aDstBitmap, CGulIcon* aIcon, TSize aNewSize, TPoint aDstPoint );
	static void BltIconScale( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, TSize aNewSize, TPoint aDstPoint );

	static void BltIconScaleBilinear( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, CFbsBitmap* aMask, TSize aNewSize, TPoint aDstPoint );
	static void BltIconScaleNormal( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, CFbsBitmap* aMask, TSize aNewSize, TPoint aDstPoint );
	static void DrawText( CFbsBitGc* aGc, const TDesC& aText, TRgb aColor, TRect aRect, TInt aBaseline, TBool aShadow, CGraphicsContext::TTextAlign aHrz=CGraphicsContext::ELeft,TInt aMargin=0 );
	static CFont* CreateFontL( TCustomUiFont aFont );
    static void TextDotsL( const TDesC& aText, TInt aFieldLengtInPixels, CFont* aFont, RBuf& aNewText );
	};

#endif // EMTUBE_UI_ITEM_GFX_H

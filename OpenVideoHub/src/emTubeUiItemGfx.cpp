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
#include <gdi.h>

#include <math.h>

#include "emTubeUiItemGfx.h"

void UiItemGfx::DrawText( CFbsBitGc* aGc, const TDesC& aText, TRgb aColor, TRect aRect, TInt aBaseline, TBool aShadow, CGraphicsContext::TTextAlign aHrz, TInt aMargin )
	{
	if( aShadow )
		{
	    TRect rectBlack = aRect;
		rectBlack.iBr.iX += 1;
		rectBlack.iBr.iY += 1;
		rectBlack.iTl.iX += 1;
		rectBlack.iTl.iY += 1;

	    aGc->SetPenColor( KRgbBlack );
        aGc->DrawText( aText, rectBlack, aBaseline, aHrz, aMargin );
		}
    aGc->SetPenColor( aColor );
    aGc->DrawText( aText, aRect, aBaseline, aHrz, aMargin );
	}

void UiItemGfx::BltIconScale( CFbsBitmap* aDstBitmap, CGulIcon* aIcon, TSize aNewSize, TPoint aDstPoint )
	{
	BltIconScaleBilinear( aDstBitmap, aIcon->Bitmap(), aIcon->Mask(), aNewSize, aDstPoint );
//	BltIconScaleNormal( aDstBitmap, aIcon->Bitmap(), aIcon->Mask(), aNewSize, aDstPoint );
	}

void UiItemGfx::BltIconScale( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, TSize aNewSize, TPoint aDstPoint )
	{
	BltIconScaleBilinear( aDstBitmap, aBitmap, NULL, aNewSize, aDstPoint );
//	BltIconScaleNormal( aDstBitmap, aBitmap, NULL, aNewSize, aDstPoint );
	}

void UiItemGfx::BltIconScaleBilinear( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, CFbsBitmap* aMask, TSize aNewSize, TPoint aDstPoint )
	{
	TInt dx, dy, x, y = 0, i, j, startX = 0;

    if( !aNewSize.iWidth || !aNewSize.iHeight )
        return;
    
	TInt dstWidth = aDstBitmap->ScanLineLength(aDstBitmap->SizeInPixels().iWidth, EColor64K ) / 2;
	TInt dstHeight = aDstBitmap->SizeInPixels().iHeight;
    
	aDstBitmap->LockHeap(ETrue);
	aBitmap->LockHeap(ETrue);
	if( aMask )
		aMask->LockHeap(ETrue);

	TUint8* mask = NULL;
	TUint16* src = (TUint16*) aBitmap->DataAddress();
	if( aMask )
		 mask = (TUint8*) aMask->DataAddress();	
	TUint16* dst = (TUint16*) aDstBitmap->DataAddress();

	TInt srcWidth = aBitmap->ScanLineLength(aBitmap->SizeInPixels().iWidth, EColor64K ) / 2;
	TInt srcHeight = aBitmap->SizeInPixels().iHeight;
	
	dx = (srcWidth << 16) / aNewSize.iWidth;
	dy = (srcHeight << 16) / aNewSize.iHeight;	    

	TInt width = aNewSize.iWidth;
	TInt height = aNewSize.iHeight;
	if( aDstPoint.iX < 0 )
		{
		TInt v = (-aDstPoint.iX);
		width -= v;
		startX = dx * v;
		aDstPoint.iX = 0;
		}

	if( aDstPoint.iY < 0 )
		{
		TInt v = (-aDstPoint.iY);
		height -= v;
		y = dy * (-aDstPoint.iY);
		aDstPoint.iY = 0;
		}

	if( (aDstPoint.iX + width) > (dstWidth-1) )
		{
		width -= ((aDstPoint.iX + width) - dstWidth);
		}

	if( (aDstPoint.iY + height) > (dstHeight-1) )
		{
		height -= ((aDstPoint.iY + height) - dstHeight);
		}

	dst = dst + aDstPoint.iX + (dstWidth * aDstPoint.iY); 

	height--;
    for (i = 0; i < height; i++)
    	{
		x = startX;
		TInt newPos = ( y >> 16 ) * srcWidth;

        for (j = 0; j < width; j++)
	        {
#define GET_R( rgb ) ((rgb >> 11) & 0x1f)
#define GET_G( rgb ) ((rgb >> 5) & 0x3f)
#define GET_B( rgb ) (rgb & 0x1f)

#define SET_R( r ) ((r & 0x1f) << 11 )
#define SET_G( g ) ((g & 0x3f) << 5 )
#define SET_B( b ) ( b & 0x1f)

int h = (x & 0xff00) / 255;
int ii = (y & 0xff00) / 255;

TUint16 cr1, cr2, cr3, cr4;
TInt tmp = newPos + (x >> 16);

cr1 = src[tmp];
cr2 = src[tmp+1];
cr3 = src[tmp+1+srcWidth];
cr4 = src[tmp+srcWidth];

int a,b,c,d;

if( mask )
	{
	if( !mask[ tmp + 1 ] || !mask[ tmp +1 + srcWidth] || !mask[ tmp + srcWidth ] )
		{
		a = 1 << 16;
		b = c = d = 0;
		}
	else
		{
		a = (0x100 - h) * (0x100 - ii);
		b = (0x000 + h) * (0x100 - ii);
		c = (0x000 + h) * (0x000 + ii);
		d = 65536 - a - b - c;
		}
	}
else
	{
	a = (0x100 - h) * (0x100 - ii);
	b = (0x000 + h) * (0x100 - ii);
	c = (0x000 + h) * (0x000 + ii);
	d = 65536 - a - b - c;
	}

unsigned int R = ((GET_R(cr1) * a) + (GET_R(cr2) * b) + (GET_R(cr3) * c) + (GET_R(cr4) * d));
unsigned int G = ((GET_G(cr1) * a) + (GET_G(cr2) * b) + (GET_G(cr3) * c) + (GET_G(cr4) * d));
unsigned int B = ((GET_B(cr1) * a) + (GET_B(cr2) * b) + (GET_B(cr3) * c) + (GET_B(cr4) * d));

TUint16 rgb = SET_R(R>>16)+SET_G(G>>16)+SET_B(B>>16);

			if( mask )
				{
				if( mask[tmp] )
				    *(dst + j) = rgb;
				x += dx;
				}
			else
				{
			    *(dst + j) = rgb;
				x += dx;
				}
	        }
		dst += dstWidth;
		y += dy;
    	}

	if( height > 0 )
		{
		x = startX;
		TInt newPos = ( y >> 16 ) * srcWidth;
	    for (j = 0; j < width; j++)
	        {
			if( mask )
				{
				TInt tmp = newPos + (x>>16);
				if( mask[tmp] )
				    *dst = src[ tmp ];
				dst++;
				x += dx;
				}
			else
				{
			    *dst++ = src[ newPos + (x>>16) ];
				x += dx;
				}
	        }
		}


	if( aMask )
		aMask->UnlockHeap(ETrue);

	aBitmap->UnlockHeap(ETrue);
	aDstBitmap->UnlockHeap(ETrue);
	}

void UiItemGfx::BltIconScaleNormal( CFbsBitmap* aDstBitmap, CFbsBitmap* aBitmap, CFbsBitmap* aMask, TSize aNewSize, TPoint aDstPoint )
	{
	TInt dx, dy, x, y = 0, i, j, startX = 0;

    if( !aNewSize.iWidth || !aNewSize.iHeight )
        return;
    
	TInt dstWidth = aDstBitmap->ScanLineLength(aDstBitmap->SizeInPixels().iWidth, EColor64K ) / 2;
	TInt dstHeight = aDstBitmap->SizeInPixels().iHeight;
    
	aDstBitmap->LockHeap(ETrue);
	aBitmap->LockHeap(ETrue);
	if( aMask )
		aMask->LockHeap(ETrue);

	TUint8* mask = NULL;
	TUint16* src = (TUint16*) aBitmap->DataAddress();
	if( aMask )
		 mask = (TUint8*) aMask->DataAddress();	
	TUint16* dst = (TUint16*) aDstBitmap->DataAddress();

	TInt srcWidth = aBitmap->ScanLineLength(aBitmap->SizeInPixels().iWidth, EColor64K ) / 2;
	TInt srcHeight = aBitmap->SizeInPixels().iHeight;
	
	dx = (srcWidth << 16) / aNewSize.iWidth;
	dy = (srcHeight << 16) / aNewSize.iHeight;	    

	TInt width = aNewSize.iWidth;
	TInt height = aNewSize.iHeight;
	if( aDstPoint.iX < 0 )
		{
		TInt v = (-aDstPoint.iX);
		width -= v;
		startX = dx * v;
		aDstPoint.iX = 0;
		}

	if( aDstPoint.iY < 0 )
		{
		TInt v = (-aDstPoint.iY);
		height -= v;
		y = dy * (-aDstPoint.iY);
		aDstPoint.iY = 0;
		}

	if( (aDstPoint.iX + width) > (dstWidth-1) )
		{
		width -= ((aDstPoint.iX + width) - (dstWidth - 1));
		}

	if( (aDstPoint.iY + height) > (dstHeight-1) )
		{
		height -= ((aDstPoint.iY + height) - (dstHeight -1));
		}

	dst = dst + aDstPoint.iX + (dstWidth * aDstPoint.iY); 
		
	for(i=0;i<height;i++)
		{
		x = startX;
		
		TInt newPos = ( y >> 16 ) * srcWidth;
		
		if( mask )
			{
			for(j=0;j<width;j++)
				{
				TInt tmp = newPos + (x >> 16);
				if( mask[tmp] )
				    *(dst + j) = src[tmp];
				x += dx;
				}
			}
		else
			{
			for(j=0;j<width;j++)
				{
			    *(dst + j) = src[newPos + (x >> 16)];
				x += dx;
				}
			}

		dst += dstWidth;
		y += dy;
		}

	
	if( aMask )
		aMask->UnlockHeap(ETrue);

	aBitmap->UnlockHeap(ETrue);
	aDstBitmap->UnlockHeap(ETrue);
	}

CFont* UiItemGfx::CreateFontL( TCustomUiFont aFont )
    {
	CFont *font = NULL;
   
    _LIT(KStandardFont, "Series S60 Sans");

    switch( aFont )
    	{
        case EFontSmall:
            {
		    TFontSpec fontSpec = TFontSpec(KStandardFont, 125 );
		    fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
//			fontSpec.iFontStyle.SetEffects( (FontEffect::TEffect)(FontEffect::EAlgorithmicBold | FontEffect::EDropShadow | FontEffect::ESoftEdge), ETrue );
		    
			font = CCoeEnv::Static()->CreateScreenFontL( fontSpec );

//			font = (CFont *)AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont, NULL);

            break;
            }
        case EFontLarge:
            {
		    TFontSpec fontSpec = TFontSpec(KStandardFont, 145);
		    fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
		    fontSpec.iFontStyle.SetBitmapType(EAntiAliasedGlyphBitmap);
//			fontSpec.iFontStyle.SetEffects( (FontEffect::TEffect)(FontEffect::EAlgorithmicBold | FontEffect::EDropShadow | FontEffect::ESoftEdge), ETrue );
		    
			font = CCoeEnv::Static()->CreateScreenFontL( fontSpec );

//			font = (CFont *)AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont, NULL);

            break;
            }
        
        default:
            break;
	    }

    return font;
    }

void UiItemGfx::TextDotsL( const TDesC& aText, TInt aFieldLengtInPixels, CFont* aFont, RBuf& aNewText )
    {
    TInt count = aFont->TextCount( aText, aFieldLengtInPixels );
    aNewText.Create( count );
    aNewText.Copy( aText.Left( count - 3 ) );
    aNewText.Append( _L("...") );
    }

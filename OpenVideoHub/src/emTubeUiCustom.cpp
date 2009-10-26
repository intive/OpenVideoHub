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

#include <akniconutils.h>
#include <FBS.H>
#include <GULICON.H>
#include <stringloader.h>
#include <e32math.h>
#include <EIKENV.H>
#include <aknutils.h>
#include <akniconutils.h>

#include "emTubeResource.h"

#include "emTubeUiCustom.h"
#include "emTubeUiItem.h"
#include "emTubeUiItemGfx.h"
#include "emTubeAppUi.h"

#define FRAMES_PER_SECOND 30
#define SCROLL_TIME 125000

#define ICON_WIDTH 80

CEmTubeUiCustom* CEmTubeUiCustom::NewL( MHandleCommandObserver& aObserver, const TRect& aRect )
	{
	CEmTubeUiCustom* self = CEmTubeUiCustom::NewLC( aObserver, aRect );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeUiCustom* CEmTubeUiCustom::NewLC( MHandleCommandObserver& aObserver, const TRect& aRect )
	{
	CEmTubeUiCustom* self = new (ELeave) CEmTubeUiCustom( aObserver, aRect );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeUiCustom::~CEmTubeUiCustom()
	{
	delete iTimer;

	iItems.ResetAndDestroy();
	iItems.Close();

    iIcons.ResetAndDestroy();
    iIcons.Close();

	delete iBitmapGc;
	delete iDevice;
    delete iBitmap;

	iAngles.Reset();
	iAngles.Close();

	delete iEmptyText;

	delete iBackgroundBitmap;
	delete iBackBitmap;
	}

CEmTubeUiCustom::CEmTubeUiCustom( MHandleCommandObserver& aObserver, const TRect& aRect ):
        iObserver(aObserver), iRect(aRect), iMaxScrollRepeat(3)
	{
	}

void CEmTubeUiCustom::ConstructL( )
	{
	iTimer = CEmTubeTimeOutTimer::NewL( *this );

	CreateBitmapL( iRect );

	iCurrentItem = 0;
	iDestinationItem = 0;

#ifdef ENABLE_CUSTOM_UI
	iBackgroundBitmap = AknIconUtils::CreateIconL( KBitmapFileName, EMbmOpenvideohubBackground );
#endif
	}

void CEmTubeUiCustom::CreateBitmapL( const TRect& aRect )
	{
	delete iBitmapGc;
	delete iDevice;
	delete iBitmap;

	iBitmap = new (ELeave) CFbsBitmap;
	iBitmap->Create( TSize(aRect.Width(), aRect.Height()), EColor64K );

    iDevice = CFbsBitmapDevice::NewL( iBitmap );
    User::LeaveIfError( iDevice->CreateContext( iBitmapGc ) );

	iBitmapGc->Activate( iDevice );
	}

void CEmTubeUiCustom::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case ETimerModeRotate:
	        {
	        if( !UpdateAngles() )
	        	{
        	    iTimer->After( 1000000 / FRAMES_PER_SECOND, ETimerModeRotate );
	        	}
	        else
	        	{
	        	iCurrentItem = iDestinationItem;
	        	SetupAngles();
	        	iTimer->After( 1000000, ETimerModeScrollText );
	            iScrollCount = 0;
	        	}

	        iParent->DrawNow();
	        }
		break;

		case ETimerModeScrollText:
			{
	        switch( iItemState )
	            {
	            case MItemDrawer::EItemLarger:
	                {
	                DrawElement( iCurrentItem, ETrue );
	                iBlitOnly = ETrue;
	                iParent->DrawNow();
	        	    iTimer->After( 200000 / FRAMES_PER_SECOND, ETimerModeScrollText );
	                break;
	                }

	            case MItemDrawer::EItemScrolling:
	                {
	                DrawElement( iCurrentItem, ETrue );
	                iBlitOnly = ETrue;
	                iParent->DrawNow();
	        	    iTimer->After( 200000 / FRAMES_PER_SECOND, ETimerModeScrollText );
	                break;
	                }

	            case MItemDrawer::EItemScrolled:
	                {
	                iScrollCount++;

	                if( iScrollCount < iMaxScrollRepeat )
	                    {
	                    iItemState = MItemDrawer::EItemLarger;
	                	iTimer->After( 1000000, ETimerModeScrollText );
	                    }
	                else if( iScrollCount == iMaxScrollRepeat )
                        {
        	        	// just wait once more
        	        	iTimer->After( 1000000, ETimerModeScrollText );
    	                }
    	            else
	                	{
	                    // redraw all
	                    iParent->DrawNow();
	                	}
	                break;
	                }

	            default:
	                break;
	            }
			}
		break;
        }
	}

TInt CEmTubeUiCustom::CountComponentControls()
	{
	return 0;
	}

CCoeControl* CEmTubeUiCustom::ComponentControl( TInt /*aIndex*/ )
	{
	return NULL;
	}

void CEmTubeUiCustom::SizeChanged( const TRect& aRect, TInt aCurrentItem )
	{
	CreateBitmapL( aRect );
	}

TKeyResponse CEmTubeUiCustom::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
    TKeyResponse result = EKeyWasNotConsumed;

	if (aType == EEventKey)
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyDownArrow:
	    		{
				if( iItems.Count() >= iVisibleItems )
					{
					if( iCurrentItem <= 0 )
						iDestinationItem = iItems.Count() - 1;
					else
						iDestinationItem = iCurrentItem - 1;
					result = EKeyWasConsumed;
					}
				else
					{
					if( iCurrentItem > 0 )
						{
						iDestinationItem = iCurrentItem - 1;
						result = EKeyWasConsumed;
						}
					}
				iDirection = -1;
				if( result == EKeyWasConsumed )
					{
					iSteps = (SCROLL_TIME * FRAMES_PER_SECOND) / 1000000;
					TimerExpired( ETimerModeRotate );
					}
		    	}
			break;

			case EKeyUpArrow:
	    		{
				if( iItems.Count() >= iVisibleItems )
					{
					if( iCurrentItem >= iItems.Count()-1 )
					    iDestinationItem = 0;
					else
					    iDestinationItem = iCurrentItem + 1;
					result = EKeyWasConsumed;
					}
				else
					{
					if( iCurrentItem < iItems.Count()-1 )
						{
						iDestinationItem = iCurrentItem + 1;
						result = EKeyWasConsumed;
						}
					}
				iDirection = 1;
				if( result == EKeyWasConsumed )
					{
					iSteps = (SCROLL_TIME * FRAMES_PER_SECOND) / 1000000;
					TimerExpired( ETimerModeRotate );
					}
				}
			break;

			default:
			break;

			}

		switch (aKeyEvent.iScanCode)
			{
			case EStdKeyEnter:
			case EStdKeyDevice3:
				{
	        	iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemClicked );
	        	result = EKeyWasConsumed;
				}
			break;

			case EStdKeyBackspace:
			case EStdKeyDelete:
				{
	        	iObserver.HandleCommandL( MHandleCommandObserver::EHandleItemDeleted );
	        	result = EKeyWasConsumed;
				}
			break;

			default:
			break;
			}
		}

	return result;
	}


void CEmTubeUiCustom::Draw( const TRect& aRect, CWindowGc& aGraphicContext )
	{
	iRect = aRect;

	if( ItemCount() || iBlitOnly )
    	{
	    if( !iBlitOnly )
            {
            if( iBackgroundBitmap )
                iBitmapGc->BitBlt( TPoint(0,0), iBackgroundBitmap );

        	TInt elements = (iVisibleItems >> 1) + 1;
        	for(TInt i=elements;i>0;i--)
        		{
        		DrawElement( iCurrentItem - i);
        		DrawElement( iCurrentItem + i);
        		}
        	DrawElement( iCurrentItem );
            }

	    aGraphicContext.BitBlt( TPoint(0,0), iBitmap );

	    iBlitOnly = EFalse;

        if( !iTimer->IsActive() )
	        {
            if( (iItemState == MItemDrawer::EItemLarger) && (iScrollCount < iMaxScrollRepeat) )
                iTimer->After( 1000000, ETimerModeScrollText );
    	    }
    	}
	else
    	{
        if( iBackgroundBitmap )
            aGraphicContext.BitBlt( TPoint(0,0), iBackgroundBitmap );

    	aGraphicContext.SetBrushColor( TRgb(15,80,122) );
    	aGraphicContext.SetPenColor( TRgb(15,80,122) );
    	aGraphicContext.SetBrushStyle( CGraphicsContext::ESolidBrush );
    	aGraphicContext.SetPenStyle(CGraphicsContext::ESolidPen);

        TRect boxRect( 0, (iRect.Height() / 2) - 20, iRect.Width(), (iRect.Height() / 2) + 20 );

        // big blue box
        aGraphicContext.DrawRect(boxRect);

        // light blue frame for blue box
        aGraphicContext.SetBrushStyle( CGraphicsContext::ENullBrush );
    	aGraphicContext.SetPenColor( TRgb(110,190,242) );
        aGraphicContext.DrawLine( TPoint( 0, boxRect.iTl.iY ), TPoint( iRect.Width(), boxRect.iTl.iY ) );
        aGraphicContext.DrawLine( TPoint( 0, boxRect.iTl.iY - 1 ), TPoint( iRect.Width(), boxRect.iTl.iY - 1) );
        aGraphicContext.DrawLine( TPoint( 0, boxRect.iBr.iY), TPoint( iRect.Width(), boxRect.iBr.iY ) );
        aGraphicContext.DrawLine( TPoint( 0, boxRect.iBr.iY + 1), TPoint( iRect.Width(), boxRect.iBr.iY + 1) );

        if( iEmptyText )
            {
    	    aGraphicContext.SetPenColor( KCustomUiSelectedTextColor );
            CFont* font = UiItemGfx::CreateFontL( UiItemGfx::EFontLarge );
        	aGraphicContext.UseFont( font );

            TInt baseline = boxRect.Height() / 2 + font->AscentInPixels() / 2;
            aGraphicContext.DrawText( *iEmptyText, boxRect, baseline, CGraphicsContext::ECenter );
            aGraphicContext.DiscardFont();
            CEikonEnv::Static()->ReleaseScreenFont( font );
            }
    	}
	}

void CEmTubeUiCustom::InitializeL( MUiInterface::TType /*aType*/, TInt aVisibleItems, CCoeControl* aControl,
    TBool /*aEnableFindBox*/, MItemDrawer* aItemDrawer, TRect aRect )
    {
    iParent = aControl;
    iVisibleItems = aVisibleItems;
	iItemDrawer = aItemDrawer;
	iInitialized = ETrue;

	iAngles.Append( 0.0f );
	TReal start = 30.0f;
	TReal diff = 3.5f;
	iAngles.Append( start );

	TInt elements = (iVisibleItems >> 1) + 1;
	for(TInt i=0;i<elements;i++)
		{
		start += (15 + diff);
        diff -= diff;
		iAngles.Append( start );
		}
    }

void CEmTubeUiCustom::RefreshL( TInt aCurrentItem )
    {
	if( ItemCount() )
		{
		if( aCurrentItem > ItemCount()-1 )
		    aCurrentItem = ItemCount()-1;
		if( aCurrentItem < 0 )
		    aCurrentItem = 0;

		iCurrentItem = aCurrentItem;

		SetupAngles();
		iParent->DrawNow();
		}
    }

TInt CEmTubeUiCustom::CurrentItemIndex()
    {
    return iCurrentItem;
    }

void CEmTubeUiCustom::AppendItemL( TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData )
    {
    iItems.AppendL( CUiItem::NewL( aIconNumber, aFirstLine, aSecondLine, aPrivateData ) );
    }

void CEmTubeUiCustom::AppendIconL( CGulIcon* aIcon, TBool aResize )
    {
	if( aResize )
		{
		TInt bsize = (TInt)( (TReal)(iRect.Width() * ICON_WIDTH) / 320 );
		TSize size( bsize, bsize );

		AknIconUtils::SetSize( aIcon->Bitmap(), size );
		if( aIcon->Mask() )
			AknIconUtils::SetSize( aIcon->Mask(), size );

        CFbsBitmap* scaledBitmap = new (ELeave) CFbsBitmap();
        scaledBitmap->Create( size, aIcon->Bitmap()->DisplayMode() );
        CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL( scaledBitmap );
        CleanupStack::PushL( bitmapDevice );
        CFbsBitGc* bitmapGc = CFbsBitGc::NewL();
        CleanupStack::PushL( bitmapGc );
        bitmapGc->Activate( bitmapDevice );
        bitmapGc->DrawBitmap( size, aIcon->Bitmap() );

        aIcon->SetBitmap( scaledBitmap );

		CleanupStack::PopAndDestroy( bitmapGc );
		CleanupStack::PopAndDestroy( bitmapDevice );

		if( aIcon->Mask() )
    		{
            CFbsBitmap* scaledMask = new (ELeave) CFbsBitmap();
            scaledMask->Create( size, aIcon->Mask()->DisplayMode() );
            CFbsBitmapDevice* maskDevice = CFbsBitmapDevice::NewL( scaledMask );
            CleanupStack::PushL( maskDevice );
            CFbsBitGc* maskGc = CFbsBitGc::NewL();
            CleanupStack::PushL( maskGc );
            maskGc->Activate( maskDevice );
            TRect scaledRect( TPoint( 0, 0 ), TPoint( bsize, bsize ) );
            maskGc->DrawBitmap( scaledRect, aIcon->Mask(), scaledRect );

            aIcon->SetMask( scaledMask );

    		CleanupStack::PopAndDestroy( maskGc );
    		CleanupStack::PopAndDestroy( maskDevice );
    		}
		}
    iIcons.AppendL( aIcon );
    }

void CEmTubeUiCustom::SetAngleAndScale( TInt aIndex, TReal aAngle, TReal aScale )
	{
	if( iItems.Count() >= iVisibleItems )
		{
		if( aIndex < 0 )
			aIndex = iItems.Count() + aIndex;
		else if( aIndex >= iItems.Count() )
			aIndex = aIndex - iItems.Count();
		}
	else
		{
		if( aIndex < 0 )
			return;
		else if( aIndex >= iItems.Count() )
			return;
		}

	iItems[ aIndex ]->SetAngle( aAngle );
	iItems[ aIndex ]->SetScale( aScale );
	}

void CEmTubeUiCustom::UpdateAngleAndScale( TInt aIndex, TReal aDiff, TReal aScaleDiff )
	{
	if( iItems.Count() >= iVisibleItems )
		{
		if( aIndex < 0 )
			aIndex = iItems.Count() + aIndex;
		else if( aIndex >= iItems.Count() )
			aIndex = aIndex - iItems.Count();
		}
	else
		{
		if( aIndex < 0 )
			return;
		else if( aIndex >= iItems.Count() )
			return;
		}

	CUiItem* item = iItems[ aIndex ];
	item->SetAngle( item->Angle() + aDiff );
	item->SetScale( item->Scale() + aScaleDiff );
	}

void CEmTubeUiCustom::SetupAngles()
	{
	TInt elements = (iVisibleItems >> 1) + 1;
	for(TInt i=elements;i>0;i--)
		{
		TReal scale = (90.0f - iAngles[i]) / 90.0f;
		SetAngleAndScale( iCurrentItem - i, iAngles[i], scale );
		SetAngleAndScale( iCurrentItem + i, -iAngles[i],scale );
		}
	SetAngleAndScale( iCurrentItem - 0, 0, 1.0f );
	}

TBool CEmTubeUiCustom::UpdateAngles()
	{
	TInt elements = (iVisibleItems >> 1) + 1;
	TReal div = (TReal)( (SCROLL_TIME * FRAMES_PER_SECOND) / 1000000 );
	TReal diff;
	TReal scaleDiff;
	for(TInt i=elements;i>0;i--)
		{
		scaleDiff = (90.0f - iAngles[i]) / 90.0f;
		scaleDiff /= div;

		diff = iAngles[i] - iAngles[i-1];
		diff = diff / div;
		diff *= iDirection;
		if( iDirection == -1 )
			{
			UpdateAngleAndScale( iCurrentItem - i, diff, scaleDiff );
			UpdateAngleAndScale( iCurrentItem + i, diff, -scaleDiff );
			}
		else
			{
			UpdateAngleAndScale( iCurrentItem - i, diff, -scaleDiff );
			UpdateAngleAndScale( iCurrentItem + i, diff, scaleDiff );
			}
		}

	scaleDiff = (90.0f - iAngles[1]) / 90.0f;
	scaleDiff /= div;

	diff = iAngles[1];
	diff /= div;
	diff *= iDirection;
	UpdateAngleAndScale( iCurrentItem, diff, -scaleDiff );

	iSteps--;
	if( iSteps == 0 )
		return ETrue;

	return EFalse;
	}

void CEmTubeUiCustom::DrawElement( TInt aIndex, TBool aScrollText )
	{
    TInt element = aIndex;

	if( iItems.Count() >= iVisibleItems )
		{
		if( aIndex < 0 )
			aIndex = iItems.Count() + aIndex;
		else if( aIndex >= iItems.Count() )
			aIndex = aIndex - iItems.Count();
		}
	else
		{
		if( aIndex < 0 )
			return;
		else if( aIndex >= iItems.Count() )
			return;
		}

	TReal angle = -iItems[ aIndex ]->Angle();

    TReal trgc, trgs, src = (TReal)( (angle * 3.14f) / 180.0f );
	Math::Sin( trgs, src );
	Math::Cos( trgc, src );

	TReal x = -(iRect.Width() / 2);
	TReal x2 = x * trgc;
	TReal y2 = x * trgs;

	CGulIcon* icon = iIcons[ iItems[aIndex]->IconIdx() ];

	TSize bmpSize = icon->Bitmap()->SizeInPixels();
	TInt bsize_w = (TInt)((TReal)((iRect.Width() * ICON_WIDTH ) / 320 ) * iItems[ aIndex ]->Scale() );
	TInt bsize_h = (bmpSize.iHeight * bsize_w) / bmpSize.iWidth;
	TInt offset = (iRect.Width() * ICON_WIDTH) / 320;

	TSize size( bsize_w, bsize_h );

	y2 += ( iRect.Height() / 2 );
	x2 += ( iRect.Width() / 2 );
	y2 -= ( bsize_h / 2 );
	x2 -= ( bsize_w / 2 );
	x2 += offset;

	TPoint point( (TInt)x2, (TInt)y2 );

    TBool selected = ( ( element == iCurrentItem ) && (iTimer->TimerMode() != ETimerModeRotate ) );

    TBool showText = ( (iSteps == 0) && (iItems[ aIndex ]->Scale() > 0.35) );

    iItemState = iItemDrawer->DrawItemL( iBitmap, iBitmapGc, size, point, showText, selected, iItems[aIndex], aScrollText );
    }

void CEmTubeUiCustom::DestroyL()
	{
	iInitialized = EFalse;
	}

TBool CEmTubeUiCustom::Initialized()
	{
	return iInitialized;
	}

TInt CEmTubeUiCustom::ItemCount()
	{
	return iItems.Count();
	}

void CEmTubeUiCustom::ResetItems()
	{
	iItems.ResetAndDestroy();
	}

void CEmTubeUiCustom::ResetIcons()
	{
	iIcons.ResetAndDestroy();
	}

CGulIcon* CEmTubeUiCustom::Icon( TInt aIdx )
	{
	return iIcons[aIdx];
	}

TInt CEmTubeUiCustom::IconsCount()
	{
	return iIcons.Count();
	}

void CEmTubeUiCustom::GetIconSize(TInt& aWidth, TInt& aHeight)
	{
	aWidth = 0;
	aHeight = 0;
	}

void CEmTubeUiCustom::RemoveItemL( TInt aIdx )
	{
	CUiItem* i = iItems[aIdx];
	iItems.Remove( aIdx );
	delete i;
	}

void CEmTubeUiCustom::InsertItemL( TInt aIdx, TInt aIconNumber, const TDesC& aFirstLine, const TDesC& aSecondLine, TAny* aPrivateData )
	{
    iItems.InsertL( CUiItem::NewL( aIconNumber, aFirstLine, aSecondLine, aPrivateData  ), aIdx );
	}

void CEmTubeUiCustom::ChangeItemIconIdxL( TInt aIdx, TInt aIconIdx )
	{
	iItems[ aIdx ]->SetIconIdx( aIconIdx );
	}

void CEmTubeUiCustom::SetFindBoxTextL( const TDesC& /*aText*/ )
	{
	}

HBufC* CEmTubeUiCustom::FindBoxText()
	{
	return NULL;
	}

TBool CEmTubeUiCustom::IsEntryVisibleL( const TDesC& /*aText*/ )
	{
	return ETrue;
	}

void CEmTubeUiCustom::HideFindBoxL()
	{
	}

void CEmTubeUiCustom::SetEmptyTextL( const TDesC& aEmptyText )
	{
	delete iEmptyText;
	iEmptyText = NULL;

	iEmptyText = aEmptyText.AllocL();
	}

CFbsBitmap* CEmTubeUiCustom::BackgroundBitmapL( TBool /*aClear*/ )
	{
	delete iBackBitmap;
	iBackBitmap = NULL;

	iBackBitmap = new (ELeave) CFbsBitmap();
	iBackBitmap->Create( iBitmap->SizeInPixels(), EColor64K );

	iBitmap->LockHeap(ETrue);
	iBackBitmap->LockHeap(ETrue);
	TUint16* src = (TUint16*)iBitmap->DataAddress();
	TUint16* dst = (TUint16*)iBackBitmap->DataAddress();
	TInt size = iBackBitmap->SizeInPixels().iWidth * iBackBitmap->SizeInPixels().iHeight;

	for(TInt i=0;i<size;i++)
		{
		TUint16 p = *src++;

		TUint16 r = (p >> 11) & 0x1f;
		TUint16 g = (p >> 5) & 0x3f;
		TUint16 b = (p) & 0x1f;
		p = (( ((r * 25) / 100) ) << 16) + (( ((g * 25) / 100) ) << 8) + ( ((b * 25) / 100) );

		*dst++ = p;
		}

	iBackBitmap->UnlockHeap(ETrue);
	iBitmap->UnlockHeap(ETrue);
	return iBackBitmap;
	}

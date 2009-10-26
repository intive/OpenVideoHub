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
#ifdef __S60_50__
#include <touchfeedback.h>
#endif

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubeSplashViewContainer.h"
#include "emTubeVideoEntry.h"
#include "emTubeImageLoader.h"

CEmTubeSplashViewContainer* CEmTubeSplashViewContainer::NewL(CEmTubeSplashView& aView, const TRect& aRect)
	{
	CEmTubeSplashViewContainer* self = CEmTubeSplashViewContainer::NewLC(aView, aRect);
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeSplashViewContainer* CEmTubeSplashViewContainer::NewLC(CEmTubeSplashView& aView, const TRect& aRect)
	{
	CEmTubeSplashViewContainer* self = new (ELeave) CEmTubeSplashViewContainer(aView);
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}

CEmTubeSplashViewContainer::~CEmTubeSplashViewContainer()
	{
	delete iTimer;
	delete iFadeTimer;
	delete iBitmap;
	delete iTmpBitmap;
	}

CEmTubeSplashViewContainer::CEmTubeSplashViewContainer(CEmTubeSplashView& aView) : iView(aView)
	{
	}

void CEmTubeSplashViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextApplication;
    }

const TInt KFadePeriod = (1000000 / 30 );
const TInt KBitmapSize = 220;
const TInt KAlphaMax = 256;
const TInt KAlphaStep = 6;
const TInt KSplashTime = 6 * 1000000;

void CEmTubeSplashViewContainer::ConstructL(const TRect& aRect)
	{
	iAlpha = KAlphaMax;

	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	CreateWindowL();

	TSize size( KBitmapSize, KBitmapSize );
	CFbsBitmap *bmp = AknIconUtils::CreateIconL( KBitmapFileName, EMbmOpenvideohubLogo_white );
	CleanupStack::PushL( bmp );
	AknIconUtils::SetSize( bmp, size );

    iBitmap = new (ELeave) CFbsBitmap();
    iBitmap->Create( size, EColor16MU );
    CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL( iBitmap );
    CleanupStack::PushL( bitmapDevice );
    CFbsBitGc* bitmapGc = CFbsBitGc::NewL();
    CleanupStack::PushL( bitmapGc );
    bitmapGc->Activate( bitmapDevice );
    bitmapGc->DrawBitmap( size, bmp );

	CleanupStack::PopAndDestroy( bitmapGc );
	CleanupStack::PopAndDestroy( bitmapDevice );
	CleanupStack::PopAndDestroy( bmp );

	iTmpBitmap = new (ELeave) CFbsBitmap;
	iTmpBitmap->Create( TSize(KBitmapSize, KBitmapSize), EColor16MU );

	SetRect(aRect);
	iClientRect = aRect;
	ActivateL();

	SetExtentToWholeScreen();
	iAppUi->StopDisplayingPopupToolbar();

	iFadeTimer = CEmTubeTimeOutTimer::NewL( *this );
	iFadeTimer->After( KFadePeriod, ECommandFadeIn );

	iTimer = CEmTubeTimeOutTimer::NewL( *this );
	iTimer->After( KSplashTime, ECommandFinish );

#ifdef __S60_50__
	MTouchFeedback* feedback = MTouchFeedback::Instance();
	if ( feedback )
		{
		feedback->EnableFeedbackForControl( this, ETrue );
		}
#endif
	}

void CEmTubeSplashViewContainer::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case ECommandFadeIn:
			{
			iAlpha -= KAlphaStep;
			if( iAlpha < 0 )
				iAlpha = 0;
			DrawNow();
			if( iAlpha != 0 )
				iFadeTimer->After( KFadePeriod, ECommandFadeIn );
			}
		break;

		case ECommandFadeOut:
			{
			iAlpha += KAlphaStep;
			if( iAlpha > KAlphaMax )
				iAlpha = KAlphaMax;
			DrawNow();
			if( iAlpha ==  KAlphaMax )
				iView.HandleCommandL( EMTVActivateMainViewCommand );
			else
				iFadeTimer->After( KFadePeriod, ECommandFadeOut );
			}
		break;

		case ECommandFinish:
			{
			iAlpha = 0;
			iFadeTimer->After( KFadePeriod, ECommandFadeOut );
			}
		break;
		}
	}

TInt CEmTubeSplashViewContainer::CountComponentControls() const
	{
	return 0;
	}

CCoeControl* CEmTubeSplashViewContainer::ComponentControl( TInt /*aIndex*/ ) const
	{
	return NULL;
	}

void CEmTubeSplashViewContainer::SizeChanged()
	{
	DrawNow();
	}

void CEmTubeSplashViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);
	SetRect( iView.ClientRect() );
	iClientRect = iView.ClientRect();
	SetExtentToWholeScreen();
	iAppUi->StopDisplayingPopupToolbar();
	DrawNow();
	}

TKeyResponse CEmTubeSplashViewContainer::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
	{
	iView.HandleCommandL( EMTVActivateMainViewCommand );
	return EKeyWasConsumed;
	}

#ifdef __S60_50__
void CEmTubeSplashViewContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    // Check if touch is enabled or not
    if( !AknLayoutUtils::PenEnabled() )
        {
        return;
        }

    if (aPointerEvent.iType == TPointerEvent::EButton1Up)
        {
#ifdef __S60_50__
		MTouchFeedback* feedback = MTouchFeedback::Instance();
		if ( feedback )
			{
			feedback->InstantFeedback( this, ETouchFeedbackBasic );
			}
#endif
    	iView.HandleCommandL( EMTVActivateMainViewCommand );
        }

    // Call base class HandlePointerEventL()
    CCoeControl::HandlePointerEventL(aPointerEvent);
    }
#endif

void CEmTubeSplashViewContainer::Draw(const TRect& /*aRect*/) const
	{

    CWindowGc& gc = SystemGc();

    gc.SetClippingRect( Rect() );

	gc.SetBrushColor( KRgbWhite );
    gc.Clear( Rect() );

	TRect rect = Rect();
	TInt x, y;

	x = (rect.Width() / 2) - (KBitmapSize / 2);
	y = (rect.Height() / 2) - (KBitmapSize / 2);

	TInt s_width = iBitmap->ScanLineLength(iBitmap->SizeInPixels().iWidth, EColor16MU ) / 4;
	TInt s_height = iBitmap->SizeInPixels().iHeight;

	TInt d_width = iTmpBitmap->ScanLineLength(iTmpBitmap->SizeInPixels().iWidth, EColor16MU ) / 4;
//	TInt d_height = iTmpBitmap->SizeInPixels().iHeight;

	iBitmap->LockHeap(ETrue);
	iTmpBitmap->LockHeap(ETrue);

	TUint32* src = (TUint32*) iBitmap->DataAddress();
	TUint32* dst = (TUint32*) iTmpBitmap->DataAddress();

#define GET_R( rgb ) ((rgb >> 16) & 0xff)
#define GET_G( rgb ) ((rgb >> 8) & 0xff)
#define GET_B( rgb ) (rgb & 0xff)

#define SET_R( r ) ((r & 0xff) << 16 )
#define SET_G( g ) ((g & 0xff) << 8 )
#define SET_B( b ) ( b & 0xff)

	TUint32 alpha = iAlpha * 255;
	TUint32 alpha1 = (KAlphaMax - iAlpha);
	for (TInt i = 0; i < s_height; i++)
    	{
    	TUint32* d = dst;
    	TUint32* s = src;
        for (TInt j = 0; j < s_width; j++)
	        {
			TUint32 rgb = *s++;

			TUint32 r = ((GET_R( rgb ) * alpha1) + alpha) >> 8;
			TUint32 g = ((GET_G( rgb ) * alpha1) + alpha) >> 8;
			TUint32 b = ((GET_B( rgb ) * alpha1) + alpha) >> 8;

			rgb = SET_R( r ) + SET_G( g ) + SET_B( b );
			*d++ = rgb;
	        }
        dst += d_width;
        src += s_width;
		}

	iBitmap->UnlockHeap(ETrue);
	iTmpBitmap->UnlockHeap(ETrue);

	gc.BitBlt( TPoint( x, y ), iTmpBitmap);
    gc.CancelClippingRect();
	}

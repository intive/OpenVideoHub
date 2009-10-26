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

#include <aknutils.h>
#include <akniconutils.h>
#include <w32std.h>
#include <avkon.rsg>

#include "emTubeAppUi.h"
#include "emTubeOsd.h"
#include "emTubeResource.h"

const TInt KVolumeButtonWidth = 38;
const TInt KVolumeButtonHeight = 38;
const TInt KVolumeHeight = 210;
const TInt KVolumeWidth = 40;
const TInt KVolumeSliderHeight = 190;
const TInt KVolumeSliderWidth = 20;
const TInt KVolumeKnobWidth = 20;

const TInt KPlayButtonWidth = 44;
const TInt KPlayButtonHeight = 44;
const TInt KForwardRewindButtonWidth = 32;
const TInt KForwardRewindButtonHeight = 32;
const TInt KNextPrevButtonWidth = 32;
const TInt KNextPrevButtonHeight = 32;

const TInt KGoBackButtonWidth = 48;
const TInt KGoBackButtonHeight = 48;

const TInt KOsdMarginX = 20;
const TInt KOsdMarginY = 10;
const TInt KOsdHeight = 120;
const TInt KOsdWidth = 520;
const TInt KOsdButtonsSpace = 14;
const TInt KMainOsdWidth = 640;
const TInt KSelectedButtonScale = 130;

const TInt KPositionKnobWidth = 20;
const TInt KPositionHeight = 20;
const TInt KPositionWidth = ( KOsdWidth - (KOsdMarginX * 2 ) );

const TInt KSliderTouchAreaBorder = 16;

const TInt KRepeatTime = 100000;

const TInt KOsdTimeFontHeight = 20;

CEmTubeOsdControl* CEmTubeOsdControl::NewL( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative = EFalse )
	{
	CEmTubeOsdControl* self = CEmTubeOsdControl::NewLC( aPoint, aType, aCommand, aCurrentValue, aRelative );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeOsdControl* CEmTubeOsdControl::NewLC( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative = EFalse )
	{
	CEmTubeOsdControl* self = new ( ELeave ) CEmTubeOsdControl( aPoint, aType, aCommand, aCurrentValue, aRelative );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CEmTubeOsdControl::~CEmTubeOsdControl()
	{
	iBitmaps.ResetAndDestroy();
	iBitmaps.Close();
	iMasks.ResetAndDestroy();
	iMasks.Close();
	}

CEmTubeOsdControl::CEmTubeOsdControl( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative )
	{
	iRelative = aRelative;
	iPoint = aPoint;
	iType = aType;
	iCommand = aCommand;
	iCurrentValue = aCurrentValue;
	iDimmed = EFalse;
	iVisible = ETrue;
	}

void CEmTubeOsdControl::ConstructL()
	{
	CFbsBitmap *bitmap, *mask;

	switch( iCommand )
		{
		case CEmTubeOsd::EOsdControlPlayPause:
			{
			TSize size( KPlayButtonWidth, KPlayButtonHeight );
			TSize size2( KPlayButtonWidth * KSelectedButtonScale / 100, KPlayButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_play, EMbmOpenvideohubOsd_play_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_play, EMbmOpenvideohubOsd_play_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_pause, EMbmOpenvideohubOsd_pause_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_pause, EMbmOpenvideohubOsd_pause_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlForward:
			{
			TSize size( KForwardRewindButtonWidth, KForwardRewindButtonHeight );
			TSize size2( KForwardRewindButtonWidth * KSelectedButtonScale / 100, KForwardRewindButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_forward, EMbmOpenvideohubOsd_forward_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_forward, EMbmOpenvideohubOsd_forward_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlRewind:
			{
			TSize size( KForwardRewindButtonWidth, KForwardRewindButtonHeight );
			TSize size2( KForwardRewindButtonWidth * KSelectedButtonScale / 100, KForwardRewindButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_rewind, EMbmOpenvideohubOsd_rewind_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_rewind, EMbmOpenvideohubOsd_rewind_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlNext:
			{
			TSize size( KNextPrevButtonWidth, KNextPrevButtonHeight );
			TSize size2( KNextPrevButtonWidth * KSelectedButtonScale / 100, KNextPrevButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_next, EMbmOpenvideohubOsd_next_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_next, EMbmOpenvideohubOsd_next_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_next_dimmed, EMbmOpenvideohubOsd_next_dimmed_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlPrev:
			{
			TSize size( KNextPrevButtonWidth, KNextPrevButtonHeight );
			TSize size2( KNextPrevButtonWidth * KSelectedButtonScale / 100, KNextPrevButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_prev, EMbmOpenvideohubOsd_prev_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_prev, EMbmOpenvideohubOsd_prev_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_prev_dimmed, EMbmOpenvideohubOsd_prev_dimmed_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlPosition:
			{
			TSize size( KPositionWidth, KPositionHeight );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_slider_empty, EMbmOpenvideohubOsd_slider_empty_mask );
			AknIconUtils::SetSize( bitmap, size, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_slider_red, EMbmOpenvideohubOsd_slider_red_mask );
			AknIconUtils::SetSize( bitmap, size, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_slider_blue, EMbmOpenvideohubOsd_slider_blue_mask  );
			AknIconUtils::SetSize( bitmap, size, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size1( KPositionKnobWidth , KPositionHeight );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_knob, EMbmOpenvideohubOsd_knob_mask );
			AknIconUtils::SetSize( bitmap, size1 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size2( KPositionKnobWidth * KSelectedButtonScale / 100, KPositionHeight * KSelectedButtonScale / 100 );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_knob, EMbmOpenvideohubOsd_knob_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			}
		break;

		case CEmTubeOsd::EOsdControlVolume:
			{
			TSize size1( KVolumeSliderWidth, KVolumeSliderHeight );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume_slider_empty, EMbmOpenvideohubOsd_volume_slider_empty_mask );
			AknIconUtils::SetSize( bitmap, size1, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size2( KVolumeSliderWidth, KVolumeSliderHeight );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume_slider_full, EMbmOpenvideohubOsd_volume_slider_full_mask );
			AknIconUtils::SetSize( bitmap, size2, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size3( KVolumeKnobWidth , KVolumeKnobWidth );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_knob, EMbmOpenvideohubOsd_knob_mask );
			AknIconUtils::SetSize( bitmap, size3 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size4( KVolumeKnobWidth * KSelectedButtonScale / 100, KVolumeKnobWidth * KSelectedButtonScale / 100 );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_knob, EMbmOpenvideohubOsd_knob_mask );
			AknIconUtils::SetSize( bitmap, size4 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			TSize size5( KVolumeWidth, KVolumeHeight );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume_back, EMbmOpenvideohubOsd_volume_back_mask );
			AknIconUtils::SetSize( bitmap, size5, EAspectRatioNotPreserved );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlShowVolume:
			{
			TSize size( KVolumeButtonWidth, KVolumeButtonHeight );
			TSize size2( KVolumeButtonWidth * KSelectedButtonScale / 100, KVolumeButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume, EMbmOpenvideohubOsd_volume_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume, EMbmOpenvideohubOsd_volume_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume_mute, EMbmOpenvideohubOsd_volume_mute_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_volume_mute, EMbmOpenvideohubOsd_volume_mute_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlScaleVideo:
			{
			TSize size( KGoBackButtonWidth, KGoBackButtonHeight );
			TSize size2( KGoBackButtonWidth * KSelectedButtonScale / 100, KGoBackButtonHeight * KSelectedButtonScale / 100);

			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scalenone, EMbmOpenvideohubOsd_scalenone_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scalenone, EMbmOpenvideohubOsd_scalenone_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scalenormal, EMbmOpenvideohubOsd_scalenormal_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scalenormal, EMbmOpenvideohubOsd_scalenormal_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );

			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scaleextended, EMbmOpenvideohubOsd_scaleextended_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_scaleextended, EMbmOpenvideohubOsd_scaleextended_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		case CEmTubeOsd::EOsdControlFinish:
			{
			TSize size( KGoBackButtonWidth, KGoBackButtonHeight );
			TSize size2( KGoBackButtonWidth * KSelectedButtonScale / 100, KGoBackButtonHeight * KSelectedButtonScale / 100);
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_goback, EMbmOpenvideohubOsd_goback_mask );
			AknIconUtils::SetSize( bitmap, size );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			AknIconUtils::CreateIconL( bitmap, mask, KBitmapFileName, EMbmOpenvideohubOsd_goback, EMbmOpenvideohubOsd_goback_mask );
			AknIconUtils::SetSize( bitmap, size2 );
			iBitmaps.Append( bitmap );
			iMasks.Append( mask );
			}
		break;

		default:
		break;

		}
	}

void CEmTubeOsdControl::DrawL( TPoint aTl, CEmTubeYUV2RGB::TRotation aRotation, CBitmapContext& aContext, MOsdCallback &aOsdCallback  )
	{
	switch( iType )
		{
		case CEmTubeOsd::EOsdControlButton:
			{
			if( iCommand == CEmTubeOsd::EOsdControlPlayPause )
				iCurrentValue = aOsdCallback.PlaybackEnabled() ? 1 : 0;
			else if( iCommand == CEmTubeOsd::EOsdControlShowVolume )
				iCurrentValue = aOsdCallback.Volume() > 0 ? 0 : 1;
			else if( iCommand == CEmTubeOsd::EOsdControlScaleVideo )
				{
				iCurrentValue = aOsdCallback.ScaleMode() + 1;
				if( iCurrentValue > 2 )
					iCurrentValue = 0;
				}
			else
				iCurrentValue = 0;
			TInt idx = ( iCurrentValue * 2 ) + iActive;
			if( iDimmed )
				idx = iBitmaps.Count() - 1; 
			TSize s = iBitmaps[ idx ]->SizeInPixels();
			TRect srcRect( TPoint( 0, 0 ), s );
			
			TPoint p = iPoint;
			
			if( iRelative )
				p += aTl;

			aContext.BitBltMasked( p - TPoint( s.iWidth / 2, s.iHeight / 2), iBitmaps[ idx ], srcRect, iMasks[ idx ], ETrue );
			}
		break;

		case CEmTubeOsd::EOsdControlSliderV:
			{
#define MARGIN 16
			TPoint start = iPoint + aTl;

			if( iCommand == CEmTubeOsd::EOsdControlVolume )
				iCurrentValue = aOsdCallback.Volume();

			TSize ss = iBitmaps[ 4 ]->SizeInPixels();
			TRect srcRect12( TPoint( 0, 0 ), ss );
			aContext.BitBltMasked( start - TPoint( ss.iWidth / 2, ss.iHeight / 2), iBitmaps[ 4 ], srcRect12, iMasks[ 4 ], ETrue );

			
			TSize s = iBitmaps[ 0 ]->SizeInPixels();
			
			TInt height = s.iHeight - MARGIN;
			
			TInt pos = (height * (10-iCurrentValue) / 10);

			TRect srcRect1( TPoint( 0, 0 ), TSize( s.iWidth, pos + 8 ) );
			aContext.BitBltMasked( start - TPoint( s.iWidth / 2, s.iHeight / 2), iBitmaps[ 0 ], srcRect1, iMasks[ 0 ], ETrue );

			TRect srcRect2( TPoint( 0, pos + 8 ), TSize( s.iWidth, s.iHeight - pos + 8 ) );
			aContext.BitBltMasked( start - TPoint( s.iWidth / 2, s.iHeight / 2) + TPoint(0, pos + 8), iBitmaps[ 1 ], srcRect2, iMasks[ 1 ], ETrue );

			TInt idx = iActive ? 3 : 2;
			TSize ks = iBitmaps[ idx ]->SizeInPixels();
			TRect srcRect3( TPoint( 0, 0 ), ks );
			TPoint p1( start - TPoint( 0, height / 2) );
			TPoint p( p1 - TPoint( (ks.iWidth / 2), ks.iHeight / 2) + TPoint( 0, pos ) );

			aContext.BitBltMasked( p, iBitmaps[ idx ], srcRect3, iMasks[ idx ], ETrue );
			}
		break;

		case CEmTubeOsd::EOsdControlSliderH:
			{
			TPoint start = iPoint + aTl;

			TSize s = iBitmaps[ 0 ]->SizeInPixels();
			
			TInt width = s.iWidth - 16;
			
			TInt pos = width * iCurrentValue / 100;
			TInt progress = aOsdCallback.ProgressPercentage();
			TInt redpos = width * progress / 100;
			
			if( progress < 100 )
				{
				TRect srcRect( TPoint( 0, 0 ), TSize( redpos + 8, s.iHeight ) );
				aContext.BitBltMasked( start - TPoint( s.iWidth / 2, s.iHeight / 2), iBitmaps[ 1 ], srcRect, iMasks[ 1 ], ETrue );
				}

			TRect srcRect( TPoint( 0, 0 ), TSize( pos + 8, s.iHeight ) );
			aContext.BitBltMasked( start - TPoint( s.iWidth / 2, s.iHeight / 2), iBitmaps[ 2 ], srcRect, iMasks[ 2 ], ETrue );

			TRect srcRect1( TPoint( pos, 0 ), TSize( s.iWidth - pos, s.iHeight ) );
			aContext.BitBltMasked( start - TPoint( s.iWidth / 2, s.iHeight / 2) + TPoint(pos, 0), iBitmaps[ 0 ], srcRect1, iMasks[ 0 ], ETrue );

			TInt idx = iActive ? 4 : 3;
			TSize ks = iBitmaps[ idx ]->SizeInPixels();
			TRect srcRect2( TPoint( 0, 0 ), ks );
			TPoint p1( start - TPoint( width / 2, 0) );
			TPoint p( p1 - TPoint( (ks.iWidth / 2), ks.iHeight / 2) + TPoint( pos, 0) );

			aContext.BitBltMasked( p, iBitmaps[ idx ], srcRect2, iMasks[ idx ], ETrue );

			}
		break;

		default:
		break;
		}
	}

void CEmTubeOsdControl::SetSize( TInt aWidth )
	{
	TInt x = iPoint.iX * aWidth / KMainOsdWidth;
	TInt y = iPoint.iY * aWidth / KMainOsdWidth;
	iPoint.SetXY( x, y);
	for(TInt i=0;i<iBitmaps.Count();i++ )
		{
		TSize s = iBitmaps[i]->SizeInPixels();
		TInt w = s.iWidth * aWidth / KMainOsdWidth;
		TInt h = s.iHeight * aWidth / KMainOsdWidth;
		s.SetSize( w, h);
		AknIconUtils::SetSize( iBitmaps[i], s);
		}
	}

void CEmTubeOsdControl::SetActive( TInt aActive )
	{
	iActive = aActive;
	}

TRect CEmTubeOsdControl::Rect( TPoint aTl )
	{
	TSize s = iBitmaps[0]->SizeInPixels();
	TPoint tl( iPoint - TPoint(s.iWidth/2, s.iHeight/2) );

	if( iRelative )
		tl += aTl;

	if( iType == CEmTubeOsd::EOsdControlSliderH )
		{
//		tl.iX -= KSliderTouchAreaBorder;
		tl.iY -= KSliderTouchAreaBorder;
		s.iHeight += KSliderTouchAreaBorder * 2;
//		s.iWidth += KSliderTouchAreaBorder * 2;
		}
	
	return TRect( tl, s );
	}

TInt CEmTubeOsdControl::Command()
	{
	return iCommand;
	}

TInt CEmTubeOsdControl::Type()
	{
	return iType;
	}

TBool CEmTubeOsdControl::Active()
	{
	return iActive;
	}

//OSDControl implementation
CEmTubeOsd* CEmTubeOsd::NewL( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback )
	{
	CEmTubeOsd* self = CEmTubeOsd::NewLC( aHandler, aOsdCallback );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeOsd* CEmTubeOsd::NewLC( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback )
	{
	CEmTubeOsd* self = new ( ELeave ) CEmTubeOsd( aHandler, aOsdCallback );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CEmTubeOsd::~CEmTubeOsd()
	{
	delete 	iTimeFormatString;
	CCoeEnv::Static()->ScreenDevice()->ReleaseFont( iTimeFont );
	delete iTimer;
	delete iBackBitmap;
	delete iBackMask;
	iControls.ResetAndDestroy();
	iControls.Close();
	}

CEmTubeOsd::CEmTubeOsd( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback ) : iHandler ( aHandler ), iOsdCallback( aOsdCallback )
	{
	}

void CEmTubeOsd::ConstructL()
	{
	iTimeFormatString = CEikonEnv::Static()->AllocReadResourceL( R_QTN_TIME_DURAT_MIN_SEC_WITH_ZERO );

	iTimer = CEmTubeTimeOutTimer::NewL( *this );

_LIT(KStandardFont, "Nokia Sans Regular");
	TFontSpec spec;
	spec = TFontSpec(KStandardFont, 200);
	spec.iFontStyle.SetBitmapType( EAntiAliasedGlyphBitmap );
	spec.iHeight = KOsdTimeFontHeight;
//	spec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
	CCoeEnv::Static()->ScreenDevice()->GetNearestFontInPixels( iTimeFont, spec );

	iVisible = ETrue;
	iRect.SetRect( 0, 0, 0, 0 );

	AknIconUtils::CreateIconL( iBackBitmap, iBackMask, KBitmapFileName, EMbmOpenvideohubOsd_back, EMbmOpenvideohubOsd_back_mask );

	AddElementL( TPoint(KMainOsdWidth - KOsdMarginX - KGoBackButtonWidth/2, KOsdMarginY + KGoBackButtonHeight/2), EOsdControlButton, EOsdControlFinish, 0, EFalse );

	AddElementL( TPoint(KMainOsdWidth - KOsdMarginX - KGoBackButtonWidth/2, KOsdMarginY + KGoBackButtonHeight/2 + KGoBackButtonHeight + KGoBackButtonHeight/2 ), EOsdControlButton, EOsdControlScaleVideo, 0, EFalse );

	
	TInt midx = KOsdWidth / 2;
	TInt midy = KOsdMarginY + (KPlayButtonHeight / 2);

	TPoint p( midx, midy );
	AddElementL( p, EOsdControlButton, EOsdControlPlayPause, 0 );
	p.SetXY( midx - KPlayButtonWidth/2 - KOsdButtonsSpace - KForwardRewindButtonWidth / 2, midy);
	AddElementL( p, EOsdControlButton, EOsdControlRewind, 0 );
	p.SetXY( midx + KPlayButtonWidth/2 + KOsdButtonsSpace + KForwardRewindButtonWidth / 2, midy);
	AddElementL( p, EOsdControlButton, EOsdControlForward, 0 );

	p.SetXY( midx - (KPlayButtonWidth/2) - KOsdButtonsSpace - KForwardRewindButtonWidth - KOsdButtonsSpace - (KNextPrevButtonWidth / 2), midy);
	AddElementL( p, EOsdControlButton, EOsdControlPrev, 0 );
	p.SetXY( midx + (KPlayButtonWidth/2) + KOsdButtonsSpace + KForwardRewindButtonWidth + KOsdButtonsSpace + (KNextPrevButtonWidth / 2), midy);
	AddElementL( p, EOsdControlButton, EOsdControlNext, 0 );

	p.SetXY( KOsdMarginX + KVolumeButtonWidth, midy);
	AddElementL( p, EOsdControlButton, EOsdControlShowVolume, 0 );

	AddElementL( TPoint( KOsdMarginX + KVolumeButtonWidth, -( KOsdMarginY + (KVolumeHeight / 2) )), EOsdControlSliderV, EOsdControlVolume, 0 );
	CEmTubeOsdControl *c = Control( EOsdControlVolume );
	c->SetVisible( EFalse );
	
	AddElementL( TPoint( midx, midy + KOsdMarginY + KPlayButtonHeight), EOsdControlSliderH, EOsdControlPosition, 0 );
	
	}

void CEmTubeOsd::AddElementL( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative )
	{
	CEmTubeOsdControl *el = CEmTubeOsdControl::NewLC( aPoint, aType, aCommand, aCurrentValue, aRelative );
	iControls.AppendL( el );
	CleanupStack::Pop( el );
	}

CEmTubeOsdControl* CEmTubeOsd::Control( TPoint aPosition )
	{
//	aPosition -= iStart;
	for( TInt i=0;i<iControls.Count();i++ )
		{
		if( iControls[i]->Visible() && iControls[i]->Rect( iStart ).Contains( aPosition ) )
			{
			return iControls[i];
			}
		}
	return NULL;
	}

CEmTubeOsdControl* CEmTubeOsd::Control( TInt aCommand )
	{
	for( TInt i=0;i<iControls.Count();i++ )
		{
		if( iControls[i]->Command() == aCommand )
			{
			return iControls[i];
			}
		}
	return NULL;
	}

TBool CEmTubeOsd::HandleTouchEventL( const TPointerEvent& aPointerEvent )
	{
	TBool ret = EFalse;
	if( aPointerEvent.iType == TPointerEvent::EButton1Down && iVisible)
		{
		CEmTubeOsdControl* el = Control( aPointerEvent.iPosition );
		if( el && !el->Dimmed() )
			{
			iActiveControl = el;
			iHandler.HandleButtonPressedL( el->Command() );
			el->SetActive( 1 );
			iTimer->After( KRepeatTime, EOsdTimerRepeat );
			ret = ETrue;
			}
		}
	else if( aPointerEvent.iType == TPointerEvent::EButton1Up )
		{
		if( iVisible )
			{
			CEmTubeOsdControl* el = Control( aPointerEvent.iPosition );
			if( el && el->Active() )
				{
				iHandler.HandleButtonReleasedL( el->Command() );
				el->SetActive( 0 );
				if( el->Command() == EOsdControlShowVolume )
					{
					CEmTubeOsdControl *e = Control( EOsdControlVolume );
					if( e )
						{
						if( e->Visible() )
							e->SetVisible( EFalse );
						else
							e->SetVisible( ETrue );
						}
					}
				ret = ETrue;
				}
			else
				{
				if( iActiveControl )
					{
					iHandler.HandleButtonReleasedL( iActiveControl->Command() );
					iActiveControl->SetActive( 0 );
					iActiveControl = NULL;
					ret = ETrue;
					}
				}
			}
		else
			{
			iVisible = ETrue;
			ret = ETrue;
			}
		iActiveControl = NULL;
		if( iOsdCallback.PlaybackEnabled() )
			iTimer->After( 4*1000000, EOsdTimerHide );
		else
			iTimer->Cancel();
		}
	else if( aPointerEvent.iType == TPointerEvent::EDrag )
		{
		CEmTubeOsdControl* el = iActiveControl;
		if( el )
			{
			switch( el->Type() )
				{
				case EOsdControlSliderH:
					{
					TRect r = el->Rect( iStart );
					TInt val;
					if( aPointerEvent.iPosition.iX < r.iTl.iX )
						{
						val = 0;
						}
					else if( aPointerEvent.iPosition.iX > r.iBr.iX )
						{
						val = 100;
						}
					else
						{
						TInt x = aPointerEvent.iPosition.iX - r.iTl.iX;
						val = x * 100 / r.Width();
						}

					el->SetCurrentValue( val );
					ret = EFalse;
					if( iOsdCallback.PlaybackEnabled() )
						iTimer->After( 4*1000000, EOsdTimerHide );
					iHandler.HandleSliderPositionChangeL( el->Command(), val );
					}
				break;
				
				case EOsdControlSliderV:
					{
					TRect r = el->Rect( iStart );
					TInt val;
					if( aPointerEvent.iPosition.iY < r.iTl.iY )
						{
						val = 10;
						}
					else if( aPointerEvent.iPosition.iY > r.iBr.iY )
						{
						val = 0;
						}
					else
						{
						TInt x = r.iBr.iY - aPointerEvent.iPosition.iY;
						val = x * 10 / r.Height();
						}
					el->SetCurrentValue( val );
					ret = EFalse;
					if( iOsdCallback.PlaybackEnabled() )
						iTimer->After( 4*1000000, EOsdTimerHide );
					iHandler.HandleSliderPositionChangeL( el->Command(), val );
					}
				break;

				default:
				break;
				}
			}
		}
	return ret;
	}

void CEmTubeOsd::HandleKeyEventL( TInt aCommand, TEventCode aType )
	{	
	CEmTubeOsdControl* el = Control( aCommand );

	if( el && aCommand == EOsdControlShowVolume )
		{
		CEmTubeOsdControl* c = Control( EOsdControlVolume );
		if( c )
			{
			if( !c->Visible() )
				c->SetVisible( ETrue );
			c->SetCurrentValue( iOsdCallback.Volume() );
			if( c == iActiveControl )
				return;
			}
		}

	iVisible = ETrue;
	if( el )
		{
		iActiveControl = el;
		switch( aType )
			{
			case EEventKey:
			break;
			
			case EEventKeyUp:
				el->SetActive( 0 );
			break;
			
			case EEventKeyDown:
				el->SetActive( 1 );
			break;
			
			default:
			break;
			}
		}
	else
		{
		if( iActiveControl )
			{
			iActiveControl->SetActive( 0 );
			iActiveControl = NULL;
			}
		}
	if( iOsdCallback.PlaybackEnabled() )
		iTimer->After( 4*1000000, EOsdTimerHide );
	else
		iTimer->Cancel();
	}

void CEmTubeOsd::SetStreamPosition( TUint32 aDuration, TUint32 aPosition )
	{
	iStreamDuration = aDuration;
	iStreamPosition = aPosition;
	CEmTubeOsdControl *ctrl = Control( EOsdControlPosition );
	if( ctrl )
		{
		TInt val = 100 * aPosition / aDuration;
		ctrl->SetCurrentValue( val );
		}
	}

void CEmTubeOsd::SetNextButtonDimmed( TBool aDimmed )
	{
	CEmTubeOsdControl *ctrl = Control( EOsdControlNext );
	if( ctrl )
		{
		ctrl->SetDimmed( aDimmed );
		}
	}

void CEmTubeOsd::SetPrevButtonDimmed( TBool aDimmed )
	{
	CEmTubeOsdControl *ctrl = Control( EOsdControlPrev );
	if( ctrl )
		{
		ctrl->SetDimmed( aDimmed );
		}
	}

void CEmTubeOsd::DrawL( TRect aRect, CEmTubeYUV2RGB::TRotation aRotation, CBitmapContext& aContext )
	{
	if( aRect.Width() <= 320 || !iVisible )
		return;

	if( iRect != aRect )
		{
		iRect = aRect;
		//TODO -> convert to the new screen size!
		iStart.SetXY( (KMainOsdWidth - KOsdWidth) / 2, aRect.Height() - KOsdHeight - KOsdMarginY );
		AknIconUtils::SetSize( iBackBitmap, TSize( KOsdWidth, KOsdHeight), EAspectRatioNotPreserved );

		for( TInt i=0;i<iControls.Count();i++)
			{
			iControls[i]->SetSize( aRect.Width() );
			}
		}

	SetNextButtonDimmed( !iOsdCallback.NextTrackExists() );
	SetPrevButtonDimmed( !iOsdCallback.PrevTrackExists() );

	aContext.BitBltMasked( iStart, iBackBitmap, TRect( TPoint( 0,0), iBackBitmap->SizeInPixels()), iBackMask, ETrue );
	
	TBuf<128> time;
	TBuf<128> tmp;

	TTime ctime(0);
	ctime += (TTimeIntervalSeconds) iStreamPosition;
	ctime.FormatL( time, *iTimeFormatString);
	time.Append( _L(" / ") );
	
	TTime ttime(0);
	ttime += (TTimeIntervalSeconds) iStreamDuration;
	ttime.FormatL( tmp, *iTimeFormatString);
	time.Append( tmp );
	CEmTubeOsdControl* c1 = Control( EOsdControlPosition );
	CEmTubeOsdControl* c2 = Control( EOsdControlPlayPause );
	
	aContext.UseFont( iTimeFont );
	aContext.SetPenColor( KRgbWhite );
	aContext.SetPenStyle( CGraphicsContext::ENullPen );
    aContext.SetBrushStyle( CGraphicsContext::ENullBrush );

	TPoint p( c1->Rect( iStart ).iBr.iX - iTimeFont->TextWidthInPixels( time ), c2->Rect( iStart ).iTl.iY );
	TRect textBox( p, TSize( iTimeFont->TextWidthInPixels( time ), c2->Rect( iStart ).Height() ) );
    TInt baseline = textBox.Height() / 2 + iTimeFont->AscentInPixels() / 2;
    aContext.DrawText( time, textBox, baseline, CGraphicsContext::ELeft );

    aContext.DiscardFont();

	for( TInt i=0;i<iControls.Count();i++)
		{
		if( iControls[i]->Visible() )
			iControls[i]->DrawL( iStart, aRotation, aContext, iOsdCallback );
		}
	}

void CEmTubeOsd::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case EOsdTimerHide:
			iActiveControl = NULL;
			iVisible = EFalse;
			iHandler.RefreshL();
		break;
		
		case EOsdTimerRepeat:
			if( iActiveControl )
				{
				iHandler.HandleButtonRepeatL( iActiveControl->Command() );
				iTimer->After( KRepeatTime, EOsdTimerRepeat );
				}
		break;
		}
	}

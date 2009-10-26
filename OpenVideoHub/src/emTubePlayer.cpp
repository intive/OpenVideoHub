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

//#define SPEED_TEST

#include <e32math.h>
#include <hal.h>
#include <math.h>

#ifndef EMTUBE_UIQ
#include <AknIconUtils.h>
#endif

#include "emTubeResource.h"

#include "emTubeAppUi.h"
#include "emTubePlayer.h"
#include "emTubeYUV2RGB.h"

#define TRANSITION_REFRESH_TIME (1000000 / 25)

void CEmTubePlayer::rotateImage( CFbsBitmap* aSrc, CFbsBitmap* aDst, TInt aAngle, TInt aScale)
	{
	TInt srcWidth = aSrc->SizeInPixels().iWidth;
	TInt srcHeight = aSrc->SizeInPixels().iHeight;
	TInt srcWidth2 = srcWidth / 2;
	TInt srcHeight2 = srcHeight / 2;
	TInt srcMod = aSrc->ScanLineLength( aSrc->SizeInPixels().iWidth, EColor16MU ) / 4;

	aSrc->LockHeap(ETrue);
	aDst->LockHeap(ETrue);

	TUint32* src = (TUint32*)aSrc->DataAddress();
	TUint32* dst = (TUint32*)aDst->DataAddress();

	TInt dstWidth = aDst->SizeInPixels().iWidth;
	TInt dstMod = aDst->ScanLineLength( aDst->SizeInPixels().iWidth, EColor16MU ) / 4;
	dstMod -= dstWidth;

	TReal c, s, radians = (TReal)( (aAngle * KPi) / 180.0f );
	Math::Sin( s, radians );
	Math::Cos( c, radians );
	TInt32 cvalue = (TInt32)( c * 65536.0 );
	TInt32 svalue = (TInt32)( s * 65536.0 );

	for( int y = -srcHeight2; y < srcHeight2; y++ )
		{
		TInt32 ys = (y * svalue) >> 16;
		TInt32 yc = (y * cvalue) >> 16;
		for( int x = -srcWidth2; x < srcWidth2; x++ )
			{
			TInt32 xs = (x * svalue) >> 16;
			TInt32 xc = (x * cvalue) >> 16;

			int sourcex = (((xc + ys) * aScale) >> 16 ) + srcWidth2;
			int sourcey = (((yc - xs) * aScale) >> 16 ) + srcHeight2;
			if( sourcex >= 0 && sourcex < srcWidth && sourcey >= 0 && sourcey < srcHeight )
				{
				*dst++ = src[ sourcey*srcMod + sourcex ];
				}
			else
				{
				*dst++ = 0;
				}
			}
		dst += dstMod;
		}

	aDst->UnlockHeap(ETrue);
	aSrc->UnlockHeap(ETrue);
	}

CEmTubePlayer* CEmTubePlayer::NewL(const TDesC& aFileName,
											MVideoHandler *aHandler,
											RWsSession& aClient,
											CWsScreenDevice& aScreenDevice,
											RWindow& aWindow,
											TRect aRect,
											MOsdCallback *aOsdCallback,
											TBool aFullscreen)
	{
	CEmTubePlayer* self = CEmTubePlayer::NewLC(aFileName, aHandler, aClient, aScreenDevice, aWindow, aRect, aOsdCallback, aFullscreen );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubePlayer* CEmTubePlayer::NewLC(const TDesC& aFileName,
											MVideoHandler *aHandler,
											RWsSession& aClient,
											CWsScreenDevice& aScreenDevice,
											RWindow& aWindow,
											TRect aRect,
											MOsdCallback *aOsdCallback,
											TBool aFullscreen)
	{
	CEmTubePlayer* self = new ( ELeave ) CEmTubePlayer( aHandler, aClient, aScreenDevice, aWindow, aRect, aOsdCallback, aFullscreen);
	CleanupStack::PushL( self );
	self->ConstructL( aFileName );
	return self;
	}

CEmTubePlayer* CEmTubePlayer::NewL(RFile& aFile,
									MVideoHandler *aHandler,
									RWsSession& aClient,
									CWsScreenDevice& aScreenDevice,
									RWindow& aWindow,
									TRect aRect,
									MOsdCallback *aOsdCallback,
									TBool aFullscreen)
	{
	CEmTubePlayer* self = CEmTubePlayer::NewLC(aFile, aHandler, aClient, aScreenDevice, aWindow, aRect, aOsdCallback, aFullscreen );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubePlayer* CEmTubePlayer::NewLC(RFile& aFile,
									MVideoHandler *aHandler,
									RWsSession& aClient,
									CWsScreenDevice& aScreenDevice,
									RWindow& aWindow,
									TRect aRect,
									MOsdCallback *aOsdCallback,
									TBool aFullscreen)
	{
	CEmTubePlayer* self = new ( ELeave ) CEmTubePlayer( aHandler, aClient, aScreenDevice, aWindow, aRect, aOsdCallback, aFullscreen);
	CleanupStack::PushL( self );
	self->ConstructL( aFile );
	return self;
	}

#include "emTubeLog.h"

void CEmTubePlayer::LogFps()
	{
#ifdef ENABLE_LOG
	TInt ticksPerSec = 200;
#ifndef __WINS__
	HAL::Get(HALData::ENanoTickPeriod, ticksPerSec);
#endif

	TVideoProperties& prop = iDemuxer->VideoProperties();

	TUint32 t = CVideoTimer::Time();
	TBuf<512> buf;
	float seconds = (float)(t - iTicks) / (float)ticksPerSec;
	buf.Format(_L("frames: %d skipped: %d seconds: %f fps: %f / %f"), iFrames, iSkippedFrames, seconds, iFrames / seconds, prop.iFps );
	Log( buf );

	TUint32 adTicks = iAudioDecoder->TimeConsumed();

	buf.Format( _L("ALL: %d CC: %d VD: %d DI: %d AD: %d"), t - iTicks, iCCTicks, iVDTicks, iDITicks - iCCTicks, adTicks );
	Log( buf );
	buf.Format( _L("frames: I: %d ticks: %d P: %d ticks: %d"), iIFrames, iIFTicks, iPFrames, iPFTicks );
	Log( buf );
	Log( "" );
#endif
	}

float CEmTubePlayer::Fps()
	{
	TInt ticksPerSec = 200;
#ifndef __WINS__
	HAL::Get(HALData::ENanoTickPeriod, ticksPerSec);
#endif

	TUint32 t = CVideoTimer::Time();
	float seconds = (float)( t - iTicks ) / ticksPerSec;
	return (float)( iFrames ) / seconds;
	}

CEmTubePlayer::~CEmTubePlayer()
	{
	iPlaybackEnabled = EFalse;

	delete iTimeOutTimer;

	delete iAudioDecoder;
	delete iDemuxer;

	if( iHandle )
		VDECClose( iHandle );
	iHandle = NULL;

	delete iDirectScreenAccess;
	delete iCC;
	delete iBitmap;

	delete iRotateBitmap;

	delete iTimer;

	delete iVolumeBitmap;
	delete iVolumeMask;

	delete iVolumeMuteBitmap;
	delete iVolumeMuteMask;

	delete iPauseBitmap;
	delete iPauseMask;

	delete iPlayBitmap;
	delete iPlayMask;

	delete iOsdUi;
	}


CEmTubePlayer::CEmTubePlayer(MVideoHandler *aHandler,
							RWsSession& aClient,
							CWsScreenDevice& aScreenDevice,
							RWindow& aWindow,
							TRect aRect,
							MOsdCallback *aOsdCallback,
							TBool aFullscreen)
 :	iClient(aClient),
	iScreenDevice(aScreenDevice),
	iWindow(aWindow),
	iFileOpened(EFalse),
	iVideoHandler(aHandler),
	iOsdCallback(aOsdCallback),
	iPlaybackEnabled(EFalse),
	iFullscreen(aFullscreen),
	iClientRect(aRect),
	iVolumeIndicator(EFalse),
	iPositionIndicator(EFalse),
	iPauseIndicator(EFalse)
	{
	}

void CEmTubePlayer::ConstructL( RFile& aFile )
	{
	iFile.Duplicate( aFile );
	ConstructL( KNullDesC() );
	}

#ifdef EMTUBE_UIQ
_LIT(KTxtCDrive,"C:");
_LIT(KTxtEDrive,"E:");

void CEmTubePlayer::LoadBitmapL(CFbsBitmap** aBitmap, CFbsBitmap** aMask, const TDesC& aPathAndFile, TInt aId, TInt aMaskId)
	{
	CFbsBitmap* bmp = new (ELeave) CFbsBitmap();
	*aBitmap = bmp;
	CFbsBitmap* mask = new (ELeave) CFbsBitmap();
	*aMask = mask;

	TParse mbfn;

	mbfn.Set( aPathAndFile, &KTxtCDrive, NULL );
	if ( !bmp->Load( mbfn.FullName(), aId, ETrue ) )
		{
		mask->Load( mbfn.FullName(), aMaskId, ETrue );
		return;
		}

	mbfn.Set( aPathAndFile, &KTxtEDrive, NULL);
	bmp->Load( mbfn.FullName(), aId, ETrue );
	mask->Load( mbfn.FullName(), aMaskId, ETrue );
	}
#endif

void CEmTubePlayer::ConstructL( const TDesC& aFileName )
	{
	iSeekDone = ETrue;

	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

#ifdef __S60_50__
	iOsdUi = CEmTubeOsd::NewL( *this, *iOsdCallback );
#endif
	
#ifndef EMTUBE_UIQ
	AknIconUtils::CreateIconL( iVolumeBitmap, iVolumeMask, KBitmapFileName, EMbmOpenvideohubIcn_volume, EMbmOpenvideohubIcn_volume_mask);
	AknIconUtils::CreateIconL( iVolumeMuteBitmap, iVolumeMuteMask, KBitmapFileName, EMbmOpenvideohubIcn_volume_mute, EMbmOpenvideohubIcn_volume_mute_mask);
	AknIconUtils::CreateIconL( iPauseBitmap, iPauseMask, KBitmapFileName, EMbmOpenvideohubIcn_pause, EMbmOpenvideohubIcn_pause_mask);
	AknIconUtils::CreateIconL( iPlayBitmap, iPlayMask, KBitmapFileName, EMbmOpenvideohubIcn_play, EMbmOpenvideohubIcn_play_mask);
#else
	LoadBitmapL( iVolumeBitmap, iVolumeMask, KBitmapFileName, EMbmOpenvideohubIcn_volume, EMbmOpenvideohubIcn_volume_mask);
	LoadBitmapL( iVolumeMuteBitmap, iVolumeMuteMask, KBitmapFileName, EMbmOpenvideohubIcn_volume_mute, EMbmOpenvideohubIcn_volume_mute_mask);
	LoadBitmapL( iPauseBitmap, iPauseMask, KBitmapFileName, EMbmOpenvideohubIcn_pause, EMbmOpenvideohubIcn_pause_mask);
	LoadBitmapL( iPlayBitmap, iPlayMask, KBitmapFileName, EMbmOpenvideohubIcn_play, EMbmOpenvideohubIcn_play_mask);
#endif

	iRotation = CEmTubeYUV2RGB::ERotationNone;

	iLowres = 0;
	iGray = 0;

	iTimer = CVideoTimer::NewL();

	iCC = CEmTubeYUV2RGB::NewL();

	iDirectScreenAccess = CDirectScreenAccess::NewL(iClient, iScreenDevice, iWindow, *this);

	iFileName.Copy( aFileName );

	delete iTimeOutTimer;
	iTimeOutTimer = CEmTubeTimeOutTimer::NewL( *this );
	iDemuxer = CDemuxer::NewL(this);
	iAudioDecoder = CAudioDecoder::NewL( iDemuxer, iTimer, this );

	StartDSAL();

	iTimeOutTimer->After( 1000, ETimerModeOpenDemuxer );
	}

void CEmTubePlayer::StartDSAL()
	{
	if( iDSAStarted )
		return;

	iDSAStarted = ETrue;
	iClearBackground = ETrue;

	iDirectScreenAccess->StartL();

	iGc = iDirectScreenAccess->Gc();
	iRegion = iDirectScreenAccess->DrawingRegion();
	iGc->SetClippingRegion( iRegion );

	if( !iPlaybackEnabled )
		{
		Refresh( *iGc );
		}
	}

void CEmTubePlayer::Restart(RDirectScreenAccess::TTerminationReasons /*aReason*/)
	{
	StartDSAL();
	}

void CEmTubePlayer::AbortNow(RDirectScreenAccess::TTerminationReasons /*aReason*/)
	{
	iDirectScreenAccess->Cancel();
	iDSAStarted = EFalse;
	}

void CEmTubePlayer::Refresh( CBitmapContext& aContext )
	{
	iClearBackground = ETrue;
	if( iImg )
		{
		DisplayFrame( aContext, iImg  );
		}
	}

void CEmTubePlayer::SetResolutionL( TRect aRect, TBool aFullscreen, TInt aTransition, TBool aForce )
	{
	if( aTransition )
		{
		if( iAppUi->VideoScaleMode() == CEmTubeAppUi::EVideoScaleNone )
			aForce = EFalse; //TODO - fix scaling down of a bitmap!

		if( aForce || (!iFullscreen && aFullscreen) )
			{
			if( iBitmap )
				{
				iTransition = aTransition;
				iPreviousWidth = iBitmap->SizeInPixels().iWidth;
				}
			}
		else
			{
			iTransition = KTransitionNone;
			}
		}

	//AbortNow( RDirectScreenAccess::ETerminateRotation );
	delete iBitmap;
	iBitmap = NULL;

	delete iRotateBitmap;
	iRotateBitmap = NULL;

	iFullscreen = aFullscreen;
	iClientRect = aRect;

	if( !iFullscreen )
		{
		iVolumeIndicator = iPositionIndicator = EFalse;
		}

	if( !iPlaybackEnabled && !iPlayIndicator )
		{
		iPauseIndicator = ETrue;
		}

	if( aTransition && !iPlaybackEnabled )
		iTimeOutTimer->After( TRANSITION_REFRESH_TIME, ETimerModeTransition );

//	Refresh( *iGc );
	}

void CEmTubePlayer::CalculateImageSizeAndRotation( TRect aRect, TDisplayMode aMode, TInt aWidth, TInt aHeight )
	{
	iRotation = CEmTubeYUV2RGB::ERotationNone;
	TInt newWidth = 0, newHeight = 0;
	TInt rectWidth = aRect.Width(), rectHeight = aRect.Height();
	TInt x = 0, y = 0;

	TBool swapWH = EFalse;

	if( iFullscreen )
		{
		if( rectWidth >= rectHeight )
			{
			iRotation = CEmTubeYUV2RGB::ERotationNone;
			}
		else
			{
			iRotation = CEmTubeYUV2RGB::ERotation90;
			}
		}
	else
		{
		iRotation = CEmTubeYUV2RGB::ERotationNone;
		}

	if( iFullscreen )
	{
		switch(iRotation)
		{
			case CEmTubeYUV2RGB::ERotation90:
			{
			if( aWidth > rectHeight || aHeight > rectWidth )
				{
				//scale down
				newHeight = (rectHeight * aHeight) / aWidth;
				newWidth = rectHeight;

				if (newHeight > rectWidth)
					{
					newWidth = (rectWidth * aWidth) / aHeight;
					newHeight = rectWidth;
					}
				}
			else
				{
				CEmTubeAppUi::TVideoScaleMode mode = iAppUi->VideoScaleMode();
				switch( mode )
					{
					case CEmTubeAppUi::EVideoScaleNormal:
						{
						newHeight = (rectHeight * aHeight) / aWidth;
						newWidth = rectHeight;

						if (newHeight > rectWidth)
							{
							newWidth = (rectWidth * aWidth) / aHeight;
							newHeight = rectWidth;
							}

						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						if(newWidth > rectHeight || newHeight > rectWidth )
							{
							newWidth -= 16;
							newHeight -= 16;
							}
						}
					break;
					
					case CEmTubeAppUi::EVideoScaleExtended:
						{
						newHeight = (rectHeight * aHeight) / aWidth;
						newWidth = rectHeight;

						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						}
					break;

					case CEmTubeAppUi::EVideoScaleNone:
						{
						newWidth = aWidth;
						newHeight = aHeight;

						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						if(newWidth > rectHeight || newHeight > rectWidth )
							{
							newWidth -= 16;
							newHeight -= 16;
							}
						}
					break;
					}
				}

				y = (aRect.Height() - newWidth) / 2;
				x = (aRect.Width() - newHeight) / 2;
				swapWH = ETrue;
			}
			break;

			case CEmTubeYUV2RGB::ERotationNone:
			default:
			{
			if( aWidth > rectWidth || aHeight > rectHeight )
				{
				//scale down
				newHeight = (rectWidth * aHeight) / aWidth;
				newWidth = rectWidth;

				if (newHeight > rectHeight)
					{
					newWidth = (rectHeight * aWidth) / aHeight;
					newHeight = rectHeight;
					}
				}
			else
				{
				CEmTubeAppUi::TVideoScaleMode mode = iAppUi->VideoScaleMode();
				switch( mode )
					{
					case CEmTubeAppUi::EVideoScaleNormal:
						{
						newHeight = (rectWidth * aHeight) / aWidth;
						newWidth = rectWidth;

						if (newHeight > rectHeight)
							{
							newWidth = (rectHeight * aWidth) / aHeight;
							newHeight = rectHeight;
							}

						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						if(newWidth > rectWidth || newHeight > rectHeight )
							{
							newWidth -= 16;
							newHeight -= 16;
							}
						}
					break;
					
					case CEmTubeAppUi::EVideoScaleExtended:
						{
						newHeight = (rectWidth * aHeight) / aWidth;
						newWidth = rectWidth;
						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						}
					break;

					case CEmTubeAppUi::EVideoScaleNone:
						{
						newWidth = aWidth;
						newHeight = aHeight;

						newHeight = (newHeight + 15)&(~15);
						newWidth = (newWidth + 15)&(~15);
						if(newWidth > rectWidth || newHeight > rectHeight )
							{
							newWidth -= 16;
							newHeight -= 16;
							}
						}
					break;
					}
				}

				x = (aRect.Width() - newWidth) / 2;
				y = (aRect.Height() - newHeight) / 2;
			}
			break;
		}
	}
	else //no fullscreen
	{
		TInt round = (5<<16)/10;

		if (aWidth > rectWidth || aHeight > rectHeight)
		{
			newWidth = rectWidth;
			newHeight = rectHeight;
			TUint srcAspect = (aHeight<<16) / aWidth;
			TUint dstAspect = (rectHeight<<16) / rectWidth;
			if (srcAspect < dstAspect)
			{
				TUint scale = (aHeight<<16) / aWidth;
				newHeight = ((newWidth * scale)+round) >> 16;
			}
			else if (srcAspect > dstAspect)
			{
				TUint scale = (aWidth<<16) / aHeight;
				newWidth = ((newHeight * scale)+round) >> 16;
			}
			x = (rectWidth - newWidth)/2;
			y = ((rectHeight - newHeight)/2);
			newWidth = (newWidth + 1) & (~1);
			newHeight = (newHeight + 1) & (~1);
		}
		else if (aWidth < rectWidth || aHeight < rectHeight)
		{
			TUint srcAspect = (aHeight<<16) / aWidth;
			TUint dstAspect = 1 << 16;

			CEmTubeAppUi::TVideoScaleMode mode = iAppUi->VideoScaleMode();
			switch( mode )
				{
				case CEmTubeAppUi::EVideoScaleNormal:
					{
					dstAspect = (rectHeight<<16) / rectWidth;
					newWidth = rectWidth;
					newHeight = rectHeight;
					}
				break;
				
				case CEmTubeAppUi::EVideoScaleExtended:
				case CEmTubeAppUi::EVideoScaleNone:
					{
					dstAspect = (aHeight<<16) / aWidth;
					newWidth = aWidth;
					newHeight = aHeight;
					}
				break;
				}

			if (srcAspect < dstAspect)
			{
				TUint scale = (aHeight<<16) / aWidth;
				newHeight = ((newWidth * scale)+round) >> 16;
			}
			else if (srcAspect > dstAspect)
			{
				TUint scale = (aWidth<<16) / aHeight;
				newWidth = ((newHeight * scale)+round) >> 16;
			}
			newWidth = (newWidth + 1) & (~1);
			newHeight = (newHeight + 1) & (~1);
			x = (rectWidth - newWidth) / 2;
			y = (rectHeight - newHeight) / 2;
		}
		else
		{
			x = 0; y = 0;
			newWidth = (newWidth + 1) & (~1);
			newHeight = (newHeight + 1) & (~1);
		}
	}

	int bx = 0, by = 0; //bitmap coordinates
	if( x < 0 )
		{
		bx = -x;
		x = 0;
		}
	
	if( y < 0 )
		{
		by = -y;
		y = 0;
		}

	int width = newWidth;
	int height = newHeight;
	if( !swapWH )
		{
		if( newWidth > rectWidth )
			width = rectWidth;

		if( newHeight > rectHeight )
			height = rectHeight;

		iVideoRect.SetRect( x, y, x + newWidth, y + newHeight );
		iBitmapRect.SetRect( bx, by, bx + width, by + height );
		}
	else
		{
		if( newWidth > rectHeight )
			width = rectHeight;

		if( newHeight > rectWidth )
			height = rectWidth;


		iVideoRect.SetRect( x, y, x + newHeight, y + newWidth );
		iBitmapRect.SetRect( bx, by, bx + height, by + width );
		}

	CEmTubeYUV2RGB::TScale scale = CEmTubeYUV2RGB::EScaleNone;

	if( newWidth != aWidth || newHeight != aHeight )
		{
		if( newWidth > aWidth || newHeight > aHeight )
			scale = CEmTubeYUV2RGB::EScaleUp;
		else
			scale = CEmTubeYUV2RGB::EScaleDown;
		}

	iCC->Init( aMode, iRotation, scale );
	}

void CEmTubePlayer::DisplayFrame( CBitmapContext& aContext, IMAGEDATA * aImg )
	{
	if( !iDSAStarted || !aImg )
		return;

	TUint32 cc = CVideoTimer::Time();

	aContext.CancelClippingRect();

	if( iImg )
		{
		if( iImg->width != aImg->width || iImg->height != aImg->height )
			{
			delete iRotateBitmap;
			iRotateBitmap = NULL;

			delete iBitmap;
			iBitmap = NULL;
			}
		}

	TDisplayMode mode = EColor16MU; //iDirectScreenAccess->ScreenDevice()->DisplayMode();

	if( !iBitmap )
		{
		if( iClientRect.Width() && iClientRect.Height() )
			{
			CalculateImageSizeAndRotation( iClientRect, mode, aImg->width >> iLowres, aImg->height >> iLowres );
			iBitmap = new (ELeave) CFbsBitmap;
			iBitmap->Create( iVideoRect.Size(), mode );

			if( iTransition )
				{
				if( iTransition & KTransitionRotate )
					{
					iAngle = -90;
					iAngleStep = 18;
					iScale = (iBitmap->SizeInPixels().iHeight << 16) / iPreviousWidth;
					}
				else
					{
					iAngle = 0;
					iAngleStep = 0;
					iScale = (iBitmap->SizeInPixels().iWidth << 16) / iPreviousWidth;
					}
				iScaleStep = (iScale - (1<<16)) / 5;
				}

			iRotateBitmap = new (ELeave) CFbsBitmap;
			iRotateBitmap->Create( iVideoRect.Size(), mode );

			TInt pixelSize = 4;
			iScanlineLength = iBitmap->ScanLineLength(iBitmap->SizeInPixels().iWidth, mode ) / pixelSize;

			iBitmap->LockHeap(ETrue);
			Mem::FillZ( iBitmap->DataAddress(), iScanlineLength * iBitmap->SizeInPixels().iHeight * pixelSize );
			iBitmap->UnlockHeap(ETrue);

			}
		}

	if( iBitmap )
		{
		TUint32 t = CVideoTimer::Time();
		iBitmap->LockHeap(ETrue);
		iCC->Convert( aImg->output, aImg->width >> iLowres, aImg->height >> iLowres, aImg->stride, aImg->uv_stride,
					  iBitmap->SizeInPixels().iWidth, iBitmap->SizeInPixels().iHeight, iScanlineLength, iBitmap->DataAddress() );
		iBitmap->UnlockHeap(ETrue);
		iCCTicks += (CVideoTimer::Time() - t);

		if( iClearBackground )
			{
			aContext.SetBrushColor( KRgbBlack );
			aContext.SetPenColor( KRgbBlack );
			aContext.SetBrushStyle( CGraphicsContext::ESolidBrush );
			aContext.SetPenStyle( CGraphicsContext::ENullPen );
			aContext.Clear();
			iClearBackground = EFalse;
			}

		if( !iTransition )
			{
			aContext.BitBlt( iVideoRect.iTl, iBitmap, iBitmapRect );
			}
		else
			{
			rotateImage( iBitmap, iRotateBitmap, iAngle, iScale );
			aContext.BitBlt( iVideoRect.iTl, iRotateBitmap, iBitmapRect );

			iAngle += iAngleStep;
			iScale -= iScaleStep;
			if( iScaleStep > 0 )
				{
				if( iScale < (1 << 16) )
					iScale = (1 << 16 );
				}
			else
				{
				if( iScale > (1 << 16) )
					iScale = (1 << 16 );
				}

			if( iTransition & KTransitionRotate )
				{
				if (iAngle > 0)
					iTransition = KTransitionNone;
				}
			else
				{
				if( iScale == ( 1<<16 ) )
					iTransition = KTransitionNone;
				}
			}

#ifdef __S60_50__
		if( iFullscreen )
			{
			TVideoProperties& prop = iDemuxer->VideoProperties();
			iOsdUi->SetStreamPosition( prop.iDuration, iPts / TIMER_BASE );
			iOsdUi->DrawL( iClientRect, iRotation, aContext );
			}
#endif

#ifndef __S60_50__
		if( iPositionIndicator )
			{
			if( iFullscreen )
				{
				iVolumeIndicator = EFalse;
#define PROGRESS_BAR_HEIGHT 8
				TSize s = iClientRect.Size(); //Bitmap->SizeInPixels();
				TSize round( 4, 4 );
				TRect rect;
				TRect rect1;
				TRect rectProgress;

				TVideoProperties& prop = iDemuxer->VideoProperties();

				TInt progress = iOsdCallback->ProgressPercentage();

				if( prop.iDuration )
					{
					if( iRotation == CEmTubeYUV2RGB::ERotation90 )
						{
						TInt w = s.iHeight * 3 / 4;
						TInt w1 = ( w * ( iPts / TIMER_BASE ) ) / prop.iDuration;

						rect.iTl.iX = ( s.iWidth / 4 ) - ( PROGRESS_BAR_HEIGHT / 2 );
						rect.iTl.iY = ( s.iHeight / 2 ) - ( w / 2 );
						rect.iBr.iX = ( s.iWidth / 4 ) + ( PROGRESS_BAR_HEIGHT / 2 );
						rect.iBr.iY = ( s.iHeight / 2 ) + ( w / 2 );
						rect1 = rect;
						rect1.iBr.iY = rect.iTl.iY + w1;
						if( progress )
							{
							rectProgress = rect;
							rectProgress.iBr.iY = rect.iTl.iY + ( rect.Height() * progress / 100 );
							rectProgress.iTl.iX += 1;
							rectProgress.iBr.iX -= 1;
							}
						}
					else
						{
						TInt w = s.iWidth * 3 / 4;
						TInt w1 = ( w * ( iPts / TIMER_BASE ) ) / prop.iDuration;

						rect.iTl.iX = ( s.iWidth / 2 ) - ( w / 2 );
						rect.iTl.iY = s.iHeight - ( s.iHeight / 4 ) - ( PROGRESS_BAR_HEIGHT / 2 );
						rect.iBr.iX = ( s.iWidth / 2 ) + ( w / 2 );
						rect.iBr.iY = s.iHeight - ( s.iHeight / 4 ) + ( PROGRESS_BAR_HEIGHT / 2 );
						rect1 = rect;
						rect1.iBr.iX = rect.iTl.iX + w1;
						if( progress )
							{
							rectProgress = rect;
							rectProgress.iBr.iX = rect.iTl.iX + ( rect.Width() * progress / 100 );
							rectProgress.iTl.iY += 1;
							rectProgress.iBr.iY -= 1;
							}
						}

					aContext.SetDrawMode( CGraphicsContext::EDrawModePEN );

					aContext.SetPenColor( KRgbBlack );
					aContext.SetBrushColor( KRgbGray );
					aContext.SetBrushStyle( CGraphicsContext::ESolidBrush );
					aContext.SetPenStyle( CGraphicsContext::ESolidPen );
					aContext.DrawRoundRect( rect, round );

					aContext.SetPenStyle( CGraphicsContext::ENullPen );
					if( progress )
						{
						aContext.SetBrushColor( KRgbRed );
						aContext.DrawRoundRect( rectProgress, round );
						}

					aContext.SetBrushColor( KRgbWhite );
					aContext.DrawRoundRect( rect1, round );
					}
				}
			}
		else if( iVolumeIndicator )
			{
			if( iFullscreen )
				{
				iPositionIndicator = EFalse;

				aContext.SetDrawMode( CGraphicsContext::EDrawModePEN );

				aContext.SetPenColor( KRgbBlack );
				aContext.SetBrushColor( KRgbWhite );
				aContext.SetBrushStyle( CGraphicsContext::ESolidBrush );
				aContext.SetPenStyle( CGraphicsContext::ESolidPen );

#define BAR_HEIGHT 14
#define BAR_WIDTH 5

				TSize size( 32 , 32 );

				TSize s = iClientRect.Size(); //iBitmap->SizeInPixels();
				TSize round( 1, 1 );
				TRect rect;

				if( iRotation == CEmTubeYUV2RGB::ERotation90 )
					{
					rect.iTl.iX = ( s.iWidth / 2 ) - ( BAR_HEIGHT / 2 );
					rect.iTl.iY = ( s.iHeight / 2 ) - ( ( BAR_WIDTH + 1 ) * 5 );
					rect.iBr.iX = ( s.iWidth / 2 ) + ( BAR_HEIGHT / 2 );
					rect.iBr.iY = rect.iTl.iY + BAR_WIDTH;

#ifndef EMTUBE_UIQ
					AknIconUtils::SetSizeAndRotation( iVolumeBitmap, size, EAspectRatioPreserved, 90 );
					AknIconUtils::SetSizeAndRotation( iVolumeMuteBitmap, size, EAspectRatioPreserved, 90 );
#endif
					size = iVolumeBitmap->SizeInPixels();
					TPoint point( ( s.iWidth / 2 ) + ( BAR_HEIGHT / 2 ) + 4, ( s.iHeight / 2 ) - ( 32 / 2 ) );
				   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
					if( iVolume )
						aContext.BitBltMasked( point, iVolumeBitmap, sourceRect, iVolumeMask, ETrue );
					else
						aContext.BitBltMasked( point, iVolumeMuteBitmap, sourceRect, iVolumeMuteMask, ETrue );
					}
				else
					{
					rect.iTl.iY = ( s.iHeight / 2 ) - ( BAR_HEIGHT / 2 );
					rect.iTl.iX = ( s.iWidth / 2 ) - ( ( BAR_WIDTH + 1 ) * 5 );
					rect.iBr.iY = ( s.iHeight / 2 ) + ( BAR_HEIGHT / 2 );
					rect.iBr.iX = rect.iTl.iX + BAR_WIDTH;

#ifndef EMTUBE_UIQ
					AknIconUtils::SetSize( iVolumeBitmap, size );
					AknIconUtils::SetSize( iVolumeMuteBitmap, size );
#endif
					size = iVolumeBitmap->SizeInPixels();
					TPoint point( ( s.iWidth / 2 ) - ( 32 / 2 ), rect.iTl.iY - (32 + 4 ) );
				   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
					if( iVolume )
						aContext.BitBltMasked( point, iVolumeBitmap, sourceRect, iVolumeMask, ETrue );
					else
						aContext.BitBltMasked( point, iVolumeMuteBitmap, sourceRect, iVolumeMuteMask, ETrue );
					}

				if( iVolume )
					{
					for (TInt i=0; i<10; i++)
						{
						if (i == iVolume)
							{
							aContext.SetBrushColor( KRgbGreen );
							aContext.SetBrushStyle( CGraphicsContext::ESolidBrush );
		//					aContext.SetBrushStyle( CGraphicsContext::ENullBrush );
							if( iRotation == CEmTubeYUV2RGB::ERotation90 )
								{
								rect.iTl.iX += 5;
								rect.iBr.iX -= 5;
								}
							else
								{
								rect.iTl.iY += 5;
								rect.iBr.iY -= 5;
								}
							}

		//				rect.iTl.iY = BAR_HEIGHT - ( i + 4 );

						aContext.DrawRoundRect( rect, round );

						if( iRotation == CEmTubeYUV2RGB::ERotation90 )
							{
							rect.iTl.iY += ( BAR_WIDTH + 1 );
							rect.iBr.iY += ( BAR_WIDTH + 1 );
							}
						else
							{
							rect.iTl.iX += ( BAR_WIDTH + 1 );
							rect.iBr.iX += ( BAR_WIDTH + 1 );
							}
						}
					}
				}
			}
		else if( iPauseIndicator )
			{
			TSize size( 40 , 40 );
			TSize s = iClientRect.Size(); //iBitmap->SizeInPixels();
			TPoint point;

			if( iRotation == CEmTubeYUV2RGB::ERotation90 )
				{
#ifndef EMTUBE_UIQ
				AknIconUtils::SetSizeAndRotation( iPauseBitmap, size, EAspectRatioPreserved, 90 );
#endif
				size = iPauseBitmap->SizeInPixels();
				point.SetXY( ( s.iWidth / 2 ) - ( size.iWidth / 2 ), ( s.iHeight / 2 ) - ( size.iHeight / 2 ) );
				}
			else
				{
#ifndef EMTUBE_UIQ
				AknIconUtils::SetSize( iPauseBitmap, size );
#endif
				size = iPauseBitmap->SizeInPixels();
				point.SetXY( ( s.iWidth / 2 ) - ( size.iWidth / 2 ), ( s.iHeight / 2 ) - ( size.iHeight / 2 ) );
				}
		   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
			aContext.BitBltMasked( point, iPauseBitmap, sourceRect, iPauseMask, ETrue );
			}
		else if( iPlayIndicator )
			{
			TSize size( 40 , 40 );
			TSize s = iClientRect.Size(); //iBitmap->SizeInPixels();
			TPoint point;

			if( iRotation == CEmTubeYUV2RGB::ERotation90 )
				{
#ifndef EMTUBE_UIQ
				AknIconUtils::SetSizeAndRotation( iPlayBitmap, size, EAspectRatioPreserved, 90 );
#endif
				size = iPlayBitmap->SizeInPixels();
				point.SetXY( ( s.iWidth / 2 ) - ( size.iWidth / 2 ), ( s.iHeight / 2 ) - ( size.iHeight / 2 ) );
				}
			else
				{
#ifndef EMTUBE_UIQ
				AknIconUtils::SetSize( iPlayBitmap, size );
#endif
				size = iPlayBitmap->SizeInPixels();
				point.SetXY( ( s.iWidth / 2 ) - ( size.iWidth / 2 ), ( s.iHeight / 2 ) - ( size.iHeight / 2 ) );
				}
		   	TRect sourceRect( TPoint( 0, 0 ), TSize( size.iWidth, size.iHeight ) );
			aContext.BitBltMasked( point, iPlayBitmap, sourceRect, iPlayMask, ETrue );
			}
#endif
		iDirectScreenAccess->ScreenDevice()->Update();
		}
	iDITicks += (CVideoTimer::Time() - cc);
	}

TVideoProperties& CEmTubePlayer::Properties()
	{
	return iDemuxer->VideoProperties();
	}

void CEmTubePlayer::Close()
	{
	iTimeOutTimer->Cancel();
	iPlaybackEnabled = EFalse;

	if( iDemuxer->AudioEnabled() )
		iAudioDecoder->Pause();
	iAudioStarted = EFalse;

	delete iBitmap;
	iBitmap = NULL;
	iDSAStarted = EFalse;
	iDirectScreenAccess->Cancel();

	if( iHandle )
		VDECClose( iHandle );
	iHandle = NULL;

	if( iDemuxer->HasAudio() )
		iAudioDecoder->Stop();

	iDemuxer->CloseFileL();

	iTimeOutTimer->After( 1000, ETimerModeCloseFile );
	}

void CEmTubePlayer::OpenFileL( const TDesC& aName )
	{
	iDemuxer->OpenFileL( aName );

	StartDSAL();

	iTimeOutTimer->After( 100000, ETimerModeOpenFile );
	}

void CEmTubePlayer::OpenFileL( RFile& aFile )
	{
	iDemuxer->OpenFileL( aFile );
	StartDSAL();
	iTimeOutTimer->After( 100000, ETimerModeOpenFile );
	}

TBool CEmTubePlayer::Seekable()
	{
	return iDemuxer->Seekable();
	}

void CEmTubePlayer::PreviousKeyFrameL( CDemuxer::TSeekSpeed aSpeed )
	{
	iClearBackground = ETrue;
	if( iDemuxer->FindKeyFrameL( iPts, CDemuxer::EBackward, aSpeed ) )
		iTimeOutTimer->After( 1, ETimerModeVideoFrame );
	}

void CEmTubePlayer::NextKeyFrameL( CDemuxer::TSeekSpeed aSpeed )
	{
	iClearBackground = ETrue;
	if( iDemuxer->FindKeyFrameL( iPts, CDemuxer::EForward, aSpeed ) )
		iTimeOutTimer->After( 1, ETimerModeVideoFrame );
	}

void CEmTubePlayer::SeekL( TInt aPosition )
	{
	if( iSeekDone )
		{
		iSeekDone = EFalse;
		iClearBackground = ETrue;
		iDemuxer->SeekL( aPosition );
		iTimeOutTimer->After( 50, ETimerModeSeek );
		}
	}

void CEmTubePlayer::SeekPercentageL( TInt aPercentage )
	{
	iClearBackground = ETrue;
	iDemuxer->SeekPercentageL( aPercentage );
	iTimeOutTimer->After( 1, ETimerModeVideoFrame );
	}

void CEmTubePlayer::Prepare()
	{
	iDemuxer->SetEof( EFalse );
	}

void CEmTubePlayer::Play()
	{
	iPlayIndicator = EFalse;
	iFrames = iSkippedFrames = 0;
	iResyncWithAudio = EFalse;
	iAutoResyncTime = 0;
	
	if( !iPlaybackEnabled )
		{
		iDemuxer->SetEof( EFalse );
		if( iDemuxer->AudioEnabled() )
			{
			iAudioDecoder->Start();
			iTimeOutTimer->After( 1000, ETimerModeStartAudioPlayback );
			}
		else
			{
			iTimer->Start();
			iAudioStarted = ETrue;
			iPlaybackEnabled = ETrue;
			iTimeOutTimer->After( 1, ETimerModeStartAudioPlayback );
			}

		iPauseIndicator = EFalse;
		}
	}

void CEmTubePlayer::Pause()
	{
	Stop();
	}

void CEmTubePlayer::Stop()
	{
	if( iPlaybackEnabled )
		{
		iTimeOutTimer->Cancel();
		iPlaybackEnabled = EFalse;

		if( iDemuxer->AudioEnabled() )
			iAudioDecoder->Pause();
		iAudioStarted = EFalse;

		LogFps();

		if( !iPositionIndicator )
			iPauseIndicator = ETrue;
		}
	}

TInt CEmTubePlayer::Volume()
	{
	return iAudioDecoder->Volume();
	}

void CEmTubePlayer::SetVolume( TInt aVolume )
	{
	TInt vol;

	if( aVolume < 0 )
		vol = 0;
	else if( aVolume > 100 )
		vol = 100;
	else
		vol = aVolume;

	iAudioDecoder->SetVolume( vol );
	}

void CEmTubePlayer::MPOSeekDoneL()
	{
	iSeekDone = ETrue;
	}

void CEmTubePlayer::MPOFileOpenedL( TInt aError )
	{
	if( aError == KErrNone )
		{
		iPauseIndicator = EFalse;
		iPlayIndicator = ETrue;
		TVideoProperties& prop = iDemuxer->VideoProperties();

#ifndef SPEED_TEST
		if( iDemuxer->HasAudio() )
			{
			iDemuxer->EnableAudio();
			iTimer->SetupTimer( TIMER_TYPE_AUDIO, prop.iFps );
			}
		else
#endif
			{
			iDemuxer->DisableAudio();
			iTimer->SetupTimer( TIMER_TYPE_NOAUDIO, prop.iFps);
			}

		if( iDemuxer->HasVideo() )
			{
			iDemuxer->EnableVideo();
			}
		else
			{
			iDemuxer->DisableVideo();
			}

#ifndef SPEED_TEST
		if( iDemuxer->AudioEnabled() )
			{
			iAudioDecoder->Initialize();
			}
		else
#endif
			{
			iVideoHandler->FileOpenedL( aError );
			iFileOpened = ETrue;
			}
		}
	else
		{
		iVideoHandler->FileOpenedL( aError );
		}
	}

void CEmTubePlayer::MPOFileClosedL()
	{
	if( iHandle )
		VDECClose( iHandle );
	iHandle = NULL;

	iImg = NULL;
	iFileOpened = EFalse;
	iAudioStarted = EFalse;
	}

void CEmTubePlayer::MPODemuxerCreated()
	{
	iDemuxerCreated = ETrue;
	}

void CEmTubePlayer::MPOAudioDecoderCreated()
	{
	}

void CEmTubePlayer::MPOAudioDecoderInitialized( TInt aError )
	{
	if( aError != KErrNone )
		{
		//TODO -> display message, that audio is being disabled! only if iDemuxer->AudioEnabled();
		iDemuxer->DisableAudio();

		TVideoProperties& prop = iDemuxer->VideoProperties();
		iTimer->SetupTimer( TIMER_TYPE_NOAUDIO, prop.iFps);
		}

	iVideoHandler->FileOpenedL( aError );

	if( aError == KErrNone )
		{
		iFileOpened = ETrue;
		}
	}

void CEmTubePlayer::MPOAudioPlaybackStarted()
	{
	iAudioStarted = ETrue;
	iPlaybackEnabled = ETrue;
	}

void CEmTubePlayer::MPOAudioError( TInt /*aError*/ )
	{
//TODO - handle audio error here
//	iVideoHandler->FileOpenedL( aError );
	}

void CEmTubePlayer::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case ETimerModeOpenFile:
			{
			if( !iFileOpened )
				{
				iTimeOutTimer->After( 1000, ETimerModeOpenFile );
				}
			else
				{
				iTimeOutTimer->After( 1, ETimerModeVideoFrame );
				iVideoHandler->PlaybackReady();
				}
			}
		break;

		case ETimerModeVideoFrame:
			{
			TBool eof = EFalse;
			Packet* pkt = (Packet*)iDemuxer->ReadVideoPacket( eof );
			if( pkt )
				{

				iPts = pkt->iPts;

				TVideoProperties& prop = iDemuxer->VideoProperties();

				if( !iHandle )
					{
					iHandle = VDECInit( prop.iWidth, prop.iHeight, &prop.iVideoExtraData, prop.iVideoExtraDataSize, prop.iVideoCodecId );
					if( iHandle )
						{
						VIDEO_SetOptions( iHandle, VIDEO_OPTION_LOWRES, iLowres );
						VIDEO_SetOptions( iHandle, VIDEO_OPTION_GRAY, iGray );
						}
					else
						{
						iVideoHandler->FileOpenedL( KErrNotSupported );
					  	iDemuxer->DeletePacket( pkt );
						return;
						}
					}

				IMAGEDATA *img = NULL;
				TBool keyframe = pkt->iKeyframe;

				TUint32 t = CVideoTimer::Time();
				if( iHandle )
					img = (IMAGEDATA *)VDECDecode( iHandle, pkt->iData, pkt->iSize, 0);
				iVDTicks += ( CVideoTimer::Time() - t );
				iDemuxer->DeletePacket( pkt );

				TInt32 wait = 0;
#ifndef SPEED_TEST
				TInt32 skip = 0;
				if( iPlaybackEnabled )
					{
					iFrames++;
					wait = iTimer->Wait( iPts, skip );
				  	if( skip && keyframe )
						skip = 0;
					}

				if( !skip )
					DisplayFrame( *iGc, img );
				else
					iSkippedFrames++;
#else
				t = CVideoTimer::Time() - t;
				if( img )
					{
					if( keyframe )
						{
						iIFTicks += t;
						iIFrames++;
						}
					else
						{
						iPFTicks += t;
						iPFrames++;
						}
					iFrames++;
					DisplayFrame( *iGc, img );
					}
#endif
				iVideoHandler->PositionUpdate( iPts / 1000, prop.iDuration );

				iImg = img;
				if( iPlaybackEnabled )
			  		{
				  	if( wait <= 0 )
				  		wait = 1;
					iTimeOutTimer->After( wait, ETimerModeVideoFrame );
			  		}
				}
			else if( eof )
				{
		  		iVideoHandler->PlaybackFinished();
				}
			else
				{
				if( iPlaybackEnabled )
					iTimeOutTimer->After( 1000, ETimerModePacket );
				}
			}
		break;

		case ETimerModeStartAudioPlayback:
			{
			if( !iAudioStarted )
				{
				iTimeOutTimer->After( 1000, ETimerModeStartAudioPlayback );
				}
			else
				{
				iFrames = iPFrames = iIFrames = iSkippedFrames = 0;
				iTicks = CVideoTimer::Time();
				iCCTicks = iVDTicks = iDITicks = iPFTicks = iIFTicks = 0;

				iTimeOutTimer->After( 1, ETimerModeVideoFrame );
				iTimer->Start();
				}
			}
		break;

		case ETimerModeSeek:
			{
			if( iSeekDone )
				{
				iClearBackground = ETrue;
				iTimeOutTimer->After( 1, ETimerModeVideoFrame );
				}
			else
				{
				iTimeOutTimer->After( 50, ETimerModeSeek );
				}
			}
		break;

		case ETimerModePacket:
			{
			iTimeOutTimer->After( 1, ETimerModeVideoFrame );
			}
		break;

		case ETimerModeCloseFile:
			{
			if( !iFileOpened )
				{
				iVideoHandler->FileClosedL( KErrNone );
				}
			else
				{
				iTimeOutTimer->After( 1000, ETimerModeCloseFile );
				}
			}
		break;

		case ETimerModeOpenDemuxer:
			{
			if( iDemuxerCreated )
				{
				iDemuxer->EnableAudio();
				iDemuxer->EnableVideo();
				if( iFileName.Length() )
					iDemuxer->OpenFileL( iFileName );
				else
					iDemuxer->OpenFileL( iFile );

				iTimeOutTimer->After( 1000, ETimerModeOpenFile );
				}
			else
				{
				iTimeOutTimer->After( 1000, ETimerModeOpenDemuxer );
				}
			}
		break;

		case ETimerModeTransition:
			{
			Refresh( *iGc );

			if( iTransition )
				iTimeOutTimer->After( TRANSITION_REFRESH_TIME, ETimerModeTransition );
			else
				Refresh( *iGc );
			}
		break;

		}
	}

void CEmTubePlayer::HideVolumeIndicator()
	{
	iPositionIndicator = EFalse;
	iVolumeIndicator = EFalse;

	iClearBackground = ETrue;
	}

void CEmTubePlayer::ShowVolumeIndicator( TInt aVolume )
	{

	iVolume = aVolume / 10;
	iVolumeIndicator = ETrue;
	iPositionIndicator = EFalse;

	iClearBackground = ETrue;
	}

void CEmTubePlayer::HidePositionIndicator()
	{
	iVolumeIndicator = EFalse;
	iPositionIndicator = EFalse;

	iClearBackground = ETrue;
	}

void CEmTubePlayer::ShowPositionIndicator()
	{

	iPositionIndicator = ETrue;
	iVolumeIndicator = EFalse;

	iClearBackground = ETrue;
	}

void CEmTubePlayer::HandleButtonRepeatL( TInt aButtonCommand )
	{
	switch( aButtonCommand )
		{
		case CEmTubeOsd::EOsdControlForward:
		case CEmTubeOsd::EOsdControlRewind:
			iVideoHandler->HandleOsdButtonRepeatL( aButtonCommand );
		break;
		
		default:
		break;
		}
	}

void CEmTubePlayer::HandleButtonReleasedL( TInt aButtonCommand )
	{
	switch( aButtonCommand )
		{
		case CEmTubeOsd::EOsdControlPosition:
			{
			if( Seekable() )
				{
				if( iRestartAfter )
					{
					Play();
					}
				iRestartAfter = EFalse;
				}
			}
			iVideoHandler->HandleOsdButtonReleasedL( aButtonCommand );
		break;

		default:
			iVideoHandler->HandleOsdButtonReleasedL( aButtonCommand );
		break;
		}
	}

void CEmTubePlayer::HandleButtonPressedL( TInt aButtonCommand )
	{
	switch( aButtonCommand )
		{
		case CEmTubeOsd::EOsdControlPosition:
			if( Seekable() )
				{
				iRestartAfter = iPlaybackEnabled;
				if( iPlaybackEnabled )
					{
					Pause();
					iClearBackground = ETrue;
					Refresh( *iGc );
					}
				}
			iVideoHandler->HandleOsdButtonPressedL( aButtonCommand );
		break;
		
		default:
			iVideoHandler->HandleOsdButtonPressedL( aButtonCommand );
		break;
		}
	}

void CEmTubePlayer::HandleSliderPositionChangeL( TInt aSliderCommand, TInt aPosition )
	{
	switch ( aSliderCommand )
		{
		case CEmTubeOsd::EOsdControlPosition:
			{
			if( Seekable() )
				{
				iVideoHandler->HandleOsdButtonRepeatL( aSliderCommand );
				SeekPercentageL( aPosition );
				}
			}
		break;
		
		case CEmTubeOsd::EOsdControlVolume:
			{
			iVideoHandler->HandleOsdSliderPositionChangeL( aSliderCommand, aPosition );
			RefreshL();
			}
		break;
		
		default:
		break;
		}
	}

TBool CEmTubePlayer::HandleTouchEventL( const TPointerEvent& aPointerEvent )
	{
	TBool ret = EFalse;
	if( iOsdUi )
		{
		ret = iOsdUi->HandleTouchEventL( aPointerEvent );
		iClearBackground = ETrue;
		}
	return ret;
	}

void CEmTubePlayer::HandleKeyEventL( TInt aCommand, TEventCode aType )
	{
	if( iOsdUi )
		{
		iOsdUi->HandleKeyEventL( aCommand, aType );
		RefreshL();
		}
	}

void CEmTubePlayer::RefreshL()
	{
	iClearBackground = ETrue;
	if( !iPlaybackEnabled )
		{
		Refresh( *iGc );
		}
	}

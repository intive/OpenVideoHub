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

#ifndef EMTUBE_PLAYER_H
#define EMTUBE_PLAYER_H


#include <w32std.h>

#include "emTubePlayerDemuxer.h"
#include "emTubePlayerAudioDevSound.h"

#include "emTubeplayertimer.h"
#include "emTubeYUV2RGB.h"
#include "videohandler.h"

#include "avlibvideo.h"
#define VDECInit( w, h, d, ds, id ) VIDEO_Init( w, h, d, ds, id )
#define VDECDecode( h, d, s, skip ) VIDEO_Decode( h, d, s, skip )
#define VDECClose(h) VIDEO_Close( h )
#define IMAGEDATA IMAGE

#include "emtubetimeouttimer.h"
#include "emtubeidletimer.h"

#include "emTubeOsd.h"

class CEmTubeAppUi;


const TInt KTransitionNone = 0;
const TInt KTransitionRotate = (1 << 0);
const TInt KTransitionScale = (1 << 1);

class CEmTubePlayer : public CBase, public MPlayerObserver, public MTimeOutObserver, public MDirectScreenAccess, public MOsdEventHandler
	{
	public:
		enum TTimerMode
			{
			ETimerModeOpenFile = 0,
			ETimerModeOpenDemuxer,
			ETimerModeVideoFrame,
			ETimerModeStartAudioPlayback,
			ETimerModePacket,
			ETimerModeCloseFile,
			ETimerModeTransition,
			ETimerModeSeek
			};

	public:
		static CEmTubePlayer* NewL(const TDesC& aFileName,
											MVideoHandler *aHandler,
											RWsSession& aClient,
											CWsScreenDevice& aScreenDevice,
											RWindow& aWindow,
											TRect aRect,
											MOsdCallback *aOsdCallback,
											TBool aFullscreen);

		static CEmTubePlayer* NewLC(const TDesC& aFileName,
											MVideoHandler *aHandler,
											RWsSession& aClient,
											CWsScreenDevice& aScreenDevice,
											RWindow& aWindow,
											TRect aRect,
											MOsdCallback *aOsdCallback,
											TBool aFullscreen);

		static CEmTubePlayer* NewL(RFile& aFile,
									MVideoHandler *aHandler,
									RWsSession& aClient,
									CWsScreenDevice& aScreenDevice,
									RWindow& aWindow,
									TRect aRect,
									MOsdCallback *aOsdCallback,
									TBool aFullscreen);

		static CEmTubePlayer* NewLC(RFile& aFile,
									MVideoHandler *aHandler,
									RWsSession& aClient,
									CWsScreenDevice& aScreenDevice,
									RWindow& aWindow,
									TRect aRect,
									MOsdCallback *aOsdCallback,
									TBool aFullscreen);
		virtual ~CEmTubePlayer();

	private:
		CEmTubePlayer( MVideoHandler *aHandler,
							RWsSession& aClient,
							CWsScreenDevice& aScreenDevice,
							RWindow& aWindow,
							TRect aRect,
							MOsdCallback *aOsdCallback,
							TBool aFullscreen);

		void ConstructL( const TDesC& aFileName );
		void ConstructL( RFile& aFile );

	private: //MDirectScreenAccess
		void Restart(RDirectScreenAccess::TTerminationReasons aReason);
		void AbortNow(RDirectScreenAccess::TTerminationReasons aReason);

	public: //MOsdEventHandler
		void HandleButtonPressedL( TInt aButtonCommand );
		void HandleButtonReleasedL( TInt aButtonCommand );
		void HandleButtonRepeatL( TInt aButtonCommand );
		void HandleSliderPositionChangeL( TInt aSliderCommand, TInt aPosition );
		void RefreshL();

	public:
		TBool HandleTouchEventL( const TPointerEvent& aPointerEvent );
		void HandleKeyEventL( TInt aCommand, TEventCode aType );

	public:
		void LoadBitmapL(CFbsBitmap** aBitmap, CFbsBitmap** aMask, const TDesC& aPathAndFile, TInt aId, TInt aMaskId);

		void Play();
		void Pause();
		void Stop();
		TInt Volume();
		void SetVolume( TInt aVolume );
		void SeekL( TInt aPosition );
		void SeekPercentageL( TInt aPercentage );

		void Prepare();

		void PreviousKeyFrameL( CDemuxer::TSeekSpeed aSpeed );
		void NextKeyFrameL( CDemuxer::TSeekSpeed aSpeed );

		TBool Seekable();

		void Close();
		void OpenFileL( const TDesC& aName );
		void OpenFileL( RFile& aFile );

		void SetResolutionL( TRect aRect, TBool aFullscreen, TInt aTransition = KTransitionNone, TBool aForce = EFalse );
		
		TVideoProperties& Properties();

		void HideVolumeIndicator();
		void ShowVolumeIndicator( TInt aVolume );

		void HidePositionIndicator();
		void ShowPositionIndicator();

		void Refresh( CBitmapContext& aContext );

		float Fps();

		TBool DisplayRotated() { return iRotation == CEmTubeYUV2RGB::ERotationNone ? (TBool)EFalse : (TBool)ETrue; }

	public: //CTimeOutTimer
		void TimerExpired( TInt aMode );

	public:
		void MPOFileOpenedL( TInt aError );
		void MPOFileClosedL();
		void MPODemuxerCreated();
		void MPOAudioDecoderCreated();
		void MPOAudioDecoderInitialized( TInt aError );
		void MPOAudioPlaybackStarted();
		void MPOAudioError( TInt aError );
		void MPOSeekDoneL();

	private:
		void LogFps();
		void StartDSAL();		
		void DisplayFrame( CBitmapContext& aContext, IMAGEDATA * aImg );
		void CalculateImageSizeAndRotation( TRect aRect, TDisplayMode aMode, TInt aWidth, TInt aHeight );


	private: //DSA
		RWsSession& iClient;
		CWsScreenDevice& iScreenDevice;
		RWindow& iWindow;

		CDirectScreenAccess* iDirectScreenAccess;
		CFbsBitGc* iGc;
		RRegion* iRegion;

	private:
		CEmTubeAppUi* iAppUi;

		TBool iDemuxerCreated;

		CDemuxer* iDemuxer;
		CAudioDecoder* iAudioDecoder;
		TFileName iFileName;
		RFile iFile;

		TBool iFileOpened;

		TAny* iHandle;

		MVideoHandler *iVideoHandler;
		CEmTubeTimeOutTimer *iTimeOutTimer;
		MOsdCallback *iOsdCallback;

		TBool iPlaybackEnabled;
		TBool iAudioStarted;

		TBool iSeekDone;
		
		TInt iFrames;
		TInt iSkippedFrames;
		TInt iTicks;
		TInt iCCTicks;
		TInt iPFTicks;
		TInt iPFrames;
		TInt iIFTicks;
		TInt iIFrames;
		TInt iVDTicks;
		TInt iDITicks;

		CEmTubeYUV2RGB::TRotation iRotation;
		CFbsBitmap* iBitmap;
		TInt iScanlineLength;

		CEmTubeYUV2RGB* iCC;

		TBool iDSAStarted;
		TBool iClearBackground;

		TBool iFullscreen;
		TRect iVideoRect;
		TRect iBitmapRect;
		TRect iClientRect;

		IMAGEDATA *iImg;

		CVideoTimer* iTimer;
		TInt32 iPts;
		TBool iResyncWithAudio;
		TInt32 iAutoResyncTime;

		TInt iLowres;
		TBool iGray;

		TInt iVolume;
		TBool iVolumeIndicator;
		TBool iPositionIndicator;
		TBool iPauseIndicator;
		TBool iPlayIndicator;

		CFbsBitmap* iVolumeMuteBitmap;
		CFbsBitmap* iVolumeMuteMask;
		CFbsBitmap* iVolumeBitmap;
		CFbsBitmap* iVolumeMask;
		CFbsBitmap* iPauseBitmap;
		CFbsBitmap* iPauseMask;
		CFbsBitmap* iPlayBitmap;
		CFbsBitmap* iPlayMask;

		CEmTubeOsd *iOsdUi;

	private:
		void rotateImage(CFbsBitmap *aSrc, CFbsBitmap* aDst, TInt angle, TInt scale);

		CFbsBitmap* iRotateBitmap;
		TInt32 iAngle;
		TInt32 iScale;
		TInt32 iAngleStep;
		TInt32 iScaleStep;
		TInt32 iPreviousWidth;
		TInt iTransition;
		TBool iRestartAfter;
	};

#endif // EMTUBE_PLAYER_H

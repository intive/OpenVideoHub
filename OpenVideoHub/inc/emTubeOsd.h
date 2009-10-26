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

#ifndef EMTUBE_OSD_H
#define EMTUBE_OSD_H

#include <gdi.h>
#include <fbs.h>

#include "emTubeYUV2RGB.h"
#include "emTubeTimeOutTimer.h"

class MOsdCallback
	{
	public:
		virtual TInt ProgressPercentage() = 0;
		virtual TBool PlaybackEnabled() = 0;
		virtual TBool NextTrackExists() = 0;
		virtual TBool PrevTrackExists() = 0;
		virtual TInt Volume() = 0;
		virtual TInt ScaleMode() = 0;
	};

class CEmTubeOsdControl : public CBase
	{
	public:
		static CEmTubeOsdControl* NewL( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative );
		static CEmTubeOsdControl* NewLC( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative );
		virtual ~CEmTubeOsdControl();

		TInt Type();
		TInt Command();
		TBool Active();
		void SetActive( TBool aActive );
		void SetSize( TInt aWidth );
		TBool Dimmed() {return iDimmed; }
		void SetDimmed( TBool aDimmed ) { iDimmed = aDimmed; }
		void SetCurrentValue( TInt aVal ) {iCurrentValue = aVal;}
		TRect Rect( TPoint aTl );

		TBool Visible() { return iVisible; }
		void SetVisible( TBool aVisible ){ iVisible = aVisible; }

		void DrawL( TPoint aTl, CEmTubeYUV2RGB::TRotation aRotation, CBitmapContext& aContext, MOsdCallback &aOsdCallback );

	protected:
		CEmTubeOsdControl( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative );
		void ConstructL();

	public:
		TInt iType;
		TInt iCommand;
		TInt iCurrentValue;

		TBool iActive;
		TBool iRelative;
		TBool iDimmed;

		TBool iVisible;

		TPoint iPoint;
		RPointerArray<CFbsBitmap> iBitmaps;
		RPointerArray<CFbsBitmap> iMasks;
	};

class MOsdEventHandler
	{
	public:
		virtual void HandleButtonPressedL( TInt aButtonCommand ) = 0;
		virtual void HandleButtonReleasedL( TInt aButtonCommand ) = 0;
		virtual void HandleButtonRepeatL( TInt aButtonCommand ) = 0;

		virtual void HandleSliderPositionChangeL( TInt aSliderCommand, TInt aPosition ) = 0;
		virtual void RefreshL() = 0;
	};

class CEmTubeOsd : public CBase, public MTimeOutObserver
	{
	public:
		enum TOsdTimerMode
		{
			EOsdTimerHide = 0,
			EOsdTimerRepeat
		};

		enum TOsdControlType
		{
			EOsdControlButton = 0,
			EOsdControlSliderH,
			EOsdControlSliderV,
			EOsdControlText
		};

		enum TOsdControlCommand
		{
			EOsdControlVolume = 0,
			EOsdControlPlayPause,
			EOsdControlScaleVideo,
			EOsdControlNext,
			EOsdControlPrev,
			EOsdControlForward,
			EOsdControlRewind,
			EOsdControlPosition,
			EOsdControlFinish,
			EOsdControlShowVolume,
			EOsdControlResetEvents,
			EOsdControlShowOsd,
			EOsdControlNone
		};

	public:
		static CEmTubeOsd* NewL( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback );
		static CEmTubeOsd* NewLC( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback );
		virtual ~CEmTubeOsd();

	protected:
		CEmTubeOsd( MOsdEventHandler& aHandler, MOsdCallback &aOsdCallback );
		void ConstructL();

	public:
		void AddElementL( TPoint aPoint, TInt aType, TInt aCommand, TInt aCurrentValue, TBool aRelative = ETrue );

		CEmTubeOsdControl* Control( TPoint aPoint );
		CEmTubeOsdControl* Control( TInt aCommand );

		TBool HandleTouchEventL( const TPointerEvent& aPointerEvent );
		void HandleKeyEventL( TInt aCommand, TEventCode aType );

		void DrawL( TRect aRect, CEmTubeYUV2RGB::TRotation aRotation, CBitmapContext& aContext );
		void SetStreamPosition( TUint32 aDuration, TUint32 aPosition );
		void SetNextButtonDimmed( TBool aDimmed );
		void SetPrevButtonDimmed( TBool aDimmed );

public:
		void TimerExpired( TInt aMode );

	private:
		CEmTubeTimeOutTimer *iTimer;

		CEmTubeOsdControl* iActiveControl;
		
		RPointerArray<CEmTubeOsdControl> iControls;
		MOsdEventHandler &iHandler;
		MOsdCallback &iOsdCallback;
		CFbsBitmap* iBackBitmap;
		CFbsBitmap* iBackMask;
		TPoint iStart;
		TRect iRect;
		TBool iVisible;
		TUint32 iStreamDuration, iStreamPosition;
		CFont *iTimeFont;
		
		HBufC* iTimeFormatString;
	};
#endif //EMTUBE_OSD_H

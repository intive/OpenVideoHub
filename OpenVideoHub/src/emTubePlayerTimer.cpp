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

#include <e32std.h>
#include <e32base.h>
#include <hal.h>

#include "emTubePlayerTimer.h"

CVideoTimer* CVideoTimer::NewL()
	{
	CVideoTimer* self = CVideoTimer::NewLC();
	CleanupStack::Pop( self );
	return self;
	}

CVideoTimer* CVideoTimer::NewLC()
	{
	CVideoTimer* self = new ( ELeave ) CVideoTimer();
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CVideoTimer::~CVideoTimer()
	{
//	iMutex.Close();
	}


void CVideoTimer::ConstructL()
	{
//	User::LeaveIfError( iMutex.CreateLocal() );
	}

CVideoTimer::CVideoTimer()
	{
	}

TInt32 CVideoTimer::Time()
{
#ifdef EKA2	// S60 3rd Ed
	TInt32 t = User::NTickCount();
#else
	TInt32 t = User::TickCount();
#endif

#ifdef __WINS__
	TInt ticksPerSec = 200;
	if( ticksPerSec != TIMER_BASE )
		t = (t * TIMER_BASE) / ticksPerSec;
#endif

	return t;
}

void CVideoTimer::SetupTimer( TInt32 aType, TReal aFps)
	{
//Log("Setup timer");
	iFrameTime = (TInt32) ( (TReal)TIMER_BASE / aFps );
	iMaxFrameDiff = -( iFrameTime * 5 );
	iType = aType;
	}

TInt32 CVideoTimer::Wait( TInt32 aPts, TInt32& aSkip )
	{
	TInt32 res = 0;

	if( iVideoInitialPts == -1 )
		iVideoInitialPts = aPts;

	switch( iType )
		{
		case TIMER_TYPE_AUDIO:
			{
			TInt32 diff = aPts - AudioPosition();

//TBuf<256> a;
//a.Format( _L("%d %d %d"), aPts, AudioPosition(), diff );
//Log( a );

			if( diff > 0 )
				{
				res = diff * 1000;
				aSkip = 0;
				}
			else
				{
				if( diff < iMaxFrameDiff )
					{
					res = 0;
					aSkip = 1;
					}
				}
			}
		break;

		case TIMER_TYPE_NOAUDIO:
			{
			aPts -= iVideoInitialPts;
			if( Time() > (iVideoBlockStart + aPts + 60) )
				{
				res = 0;
				aSkip = 1;
				}
			else
				{
				aSkip = 0;
				res = ( aPts - ( Time() - iVideoBlockStart ) ) * 1000;
				}
			}
		break;
		}
	return res;
	}

void CVideoTimer::Start()
	{
//Log("Start timer");
//	iMutex.Wait();

	SetAudioPosition( 0 );

	iVideoBlockStart = Time();
	iVideoInitialPts = -1;
//	iMutex.Signal();
	}

void CVideoTimer::SetAudioPosition( TInt32 aPosition )
	{
//TBuf<64> a;
//a.Format( _L("set audio position %d"), aPosition);
//Log( a );
//	iMutex.Wait();
	iAudioBlockStart = Time();
	iAudioPosition = aPosition;
//	iMutex.Signal();
	}

void CVideoTimer::SetInitialAudioPts( TInt32 aPts )
	{
//TBuf<64> a;
//a.Format( _L("set initial audio pts %d"), aPts);
//Log( a );
//	iMutex.Wait();
	iAudioInitialPts = aPts;
	iAudioPosition = 0;
	iAudioBlockStart = 0;
//	iMutex.Signal();
	}

TInt32 CVideoTimer::AudioPosition()
	{
	return iAudioInitialPts + iAudioPosition + ( Time() - iAudioBlockStart );
	}

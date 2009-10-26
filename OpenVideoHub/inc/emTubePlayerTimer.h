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

#ifndef EMTUBE_PLAYER_TIMER_H
#define EMTUBE_PLAYER_TIMER_H

#define TIMER_TYPE_AUDIO		1
#define TIMER_TYPE_NOAUDIO		2

#define TIMER_BASE 1000

class CVideoTimer : public CBase
	{
	public:
		static CVideoTimer* NewLC();
		static CVideoTimer* NewL();
	
		~CVideoTimer();
		CVideoTimer();
		void ConstructL();
		
	public:
		static TInt32 Time();

		void SetupTimer(TInt32 aType, TReal aFps);
		TInt32 Wait( TInt32 aPts, TInt32& aSkip );
		void Start();

		void SetAudioPosition( TInt32 aPosition );
		void SetInitialAudioPts( TInt32 aPts );
		TInt32 AudioPosition();

	private:
	    RMutex	iMutex;

		TInt32	iType;

	    TInt32	iFrameTime;
	    TInt32	iMaxFrameDiff;

	    TInt32	iVideoBlockStart;
		TInt32	iVideoInitialPts;

	    TInt32	iAudioInitialPts;
	    TInt32	iAudioBlockStart;
		TInt32	iAudioPosition;
	};

#endif //EMTUBE_PLAYER_TIMER_H

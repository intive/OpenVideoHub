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

#include <e32svr.h>
#include <coemain.h>

#include "emTubeIdleTimer.h"
			
CEmTubeIdleTimer* CEmTubeIdleTimer::NewLC(MTimeOutObserver& aObserver)
	{
	CEmTubeIdleTimer* self = new (ELeave) CEmTubeIdleTimer( aObserver );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeIdleTimer* CEmTubeIdleTimer::NewL(MTimeOutObserver& aObserver)
	{
	CEmTubeIdleTimer* self = CEmTubeIdleTimer::NewLC( aObserver );
	CleanupStack::Pop();
	return self;
	}

CEmTubeIdleTimer::CEmTubeIdleTimer( MTimeOutObserver& aObserver ) : iObserver( aObserver )
	{
	}

TInt CallbackL( TAny* aThis )
    {
    return static_cast<CEmTubeIdleTimer*>( aThis )->TimerCalback();
    }

void CEmTubeIdleTimer::ConstructL()
	{
    iIdle = CIdle::NewL( CActive::EPriorityStandard );
    TCallBack callback( CallbackL, this );
    iIdle->Start( callback );
	}

void CEmTubeIdleTimer::After( TTimeIntervalMicroSeconds32 aInterval, TInt aMode )
	{
	iMode = aMode;
	User::After( aInterval );
	}

CEmTubeIdleTimer::~CEmTubeIdleTimer()
	{
	iIdle->Cancel();
	delete iIdle;
	}

TInt CEmTubeIdleTimer::TimerCalback()
    {
	iObserver.TimerExpired( iMode );
    return 1;
    }


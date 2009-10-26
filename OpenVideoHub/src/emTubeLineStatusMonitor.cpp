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

#include "emTubeLineStatusMonitor.h"

CEmTubeLineStatusMonitor* CEmTubeLineStatusMonitor::NewL( MLineStatusObserver& aObserver )
	{
	CEmTubeLineStatusMonitor* self = NewLC( aObserver );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeLineStatusMonitor* CEmTubeLineStatusMonitor::NewLC( MLineStatusObserver& aObserver )
	{
	CEmTubeLineStatusMonitor* self = new (ELeave) CEmTubeLineStatusMonitor( aObserver );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CEmTubeLineStatusMonitor::CEmTubeLineStatusMonitor( MLineStatusObserver& aObserver ) : CActive( CActive::EPriorityStandard ),
																					 iLineStatusPckg( iLineStatus ),
																					 iObserver( aObserver )
	{
	}

CEmTubeLineStatusMonitor::~CEmTubeLineStatusMonitor()
	{
	Cancel();
	delete iTelephony;
	}

void CEmTubeLineStatusMonitor::ConstructL()
	{
	CActiveScheduler::Add( this );

	iTelephony = CTelephony::NewL();

	StartNotifier();
	}

void CEmTubeLineStatusMonitor::RunL()
	{
	if ( iStatus == KErrNone )
		{
		iObserver.HandleLineStatusChangeL( iLineStatus.iStatus );

		StartNotifier();
		}
	}

void CEmTubeLineStatusMonitor::DoCancel()
	{
	iTelephony->CancelAsync( CTelephony::EVoiceLineStatusChangeCancel );
	}

void CEmTubeLineStatusMonitor::StartNotifier()
	{
	Cancel();

	iLineStatus.iStatus = CTelephony::EStatusUnknown;
	iTelephony->NotifyChange( iStatus, CTelephony::EVoiceLineStatusChange, iLineStatusPckg );

	SetActive();
	}

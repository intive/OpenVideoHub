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

#include <EIKENV.H>

#include "emTubePhoneInfo.h"


CEmTubePhoneInfo* CEmTubePhoneInfo::NewL()
	{
	CEmTubePhoneInfo* self = CEmTubePhoneInfo::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePhoneInfo* CEmTubePhoneInfo::NewLC()
	{
	CEmTubePhoneInfo* self = new (ELeave) CEmTubePhoneInfo();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;  
	}

CEmTubePhoneInfo::CEmTubePhoneInfo() : CActive(EPriorityHigh)
	{
	}

void CEmTubePhoneInfo::ConstructL()
	{
	iTelephony = CTelephony::NewL();
	CActiveScheduler::Add(this); // Add to scheduler
	}

CEmTubePhoneInfo::~CEmTubePhoneInfo()
	{
	Cancel(); // Cancel any request, if outstanding
	delete iTelephony;
	}

void CEmTubePhoneInfo::DoCancel()
	{
	iTelephony->CancelAsync(CTelephony::EGetPhoneIdCancel);
	}

void CEmTubePhoneInfo::RunL()
	{
	if ( iActiveSchedulerWait.IsStarted() )
		{
		iActiveSchedulerWait.AsyncStop();
		if(iStatus == KErrNone)
			{
			iImei.Append( iPhoneId.iSerialNumber );
			}
		else
			{
			} 
		}
	}

const TPtrC CEmTubePhoneInfo::GetImeiL()
	{
	iImei.Zero();

	Cancel();

	CTelephony::TPhoneIdV1Pckg phoneIdPckg( iPhoneId ); 
	iTelephony->GetPhoneId( iStatus, phoneIdPckg );
	SetActive();
	iActiveSchedulerWait.Start();

	TPtrC ptr(iImei.Ptr());
	return ptr;
	}

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

#ifndef EMTUBE_LINE_STATUS_MONITOR_H
#define EMTUBE_LINE_STATUS_MONITOR_H

#include <e32base.h>
#include <etel3rdparty.h>

class MLineStatusObserver
{
public:
	virtual void HandleLineStatusChangeL( CTelephony::TCallStatus& aStatus ) = 0;
};
 
class CEmTubeLineStatusMonitor : public CActive 
{
public:
	static CEmTubeLineStatusMonitor* NewL( MLineStatusObserver& aObserver );
	static CEmTubeLineStatusMonitor* NewLC( MLineStatusObserver& aObserver );
	~CEmTubeLineStatusMonitor();

protected: // from CActive
	void RunL();
	void DoCancel();	

private:
	void ConstructL();
	CEmTubeLineStatusMonitor( MLineStatusObserver& aObserver );
	void StartNotifier();

private: //data
	CTelephony  *iTelephony;
	
	CTelephony::TCallStatusV1 iLineStatus;
	CTelephony::TCallStatusV1Pckg iLineStatusPckg;
	
	MLineStatusObserver& iObserver;
};

#endif // EMTUBE_LINE_STATUS_MONITOR_H

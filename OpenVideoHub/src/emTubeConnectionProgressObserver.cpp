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

#include <in_iface.h>
#include <aknglobalnote.h>

#include "emTubeConnectionProgressObserver.h"
#include "emTubeLog.h"

CEmTubeConnectionProgressObserver* CEmTubeConnectionProgressObserver::NewL( MConnectionProgressObserver* aObserver, RConnection& aConnection )
	{
	CEmTubeConnectionProgressObserver* self = NewLC( aObserver, aConnection );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeConnectionProgressObserver* CEmTubeConnectionProgressObserver::NewLC( MConnectionProgressObserver* aObserver, RConnection& aConnection )
	{
	CEmTubeConnectionProgressObserver* self = new (ELeave) CEmTubeConnectionProgressObserver( aObserver, aConnection );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CEmTubeConnectionProgressObserver::CEmTubeConnectionProgressObserver( MConnectionProgressObserver* aObserver, RConnection& aConnection )
: CActive(CActive::EPriorityStandard),
  iConnection(aConnection), 
  iObserver(aObserver)
	{
	CActiveScheduler::Add(this);		
	}

CEmTubeConnectionProgressObserver::~CEmTubeConnectionProgressObserver()
	{
	Cancel();
	}

void CEmTubeConnectionProgressObserver::ConstructL()
	{
	iConnection.ProgressNotification(iProgress, iStatus);
	SetActive();
	}

void CEmTubeConnectionProgressObserver::DoCancel()
	{
	iConnection.CancelProgressNotification();
	}

void CEmTubeConnectionProgressObserver::RunL()
	{
	Log( "->CEmTubeConnectionProgressObserver::RunL()" );
	Log( iStatus.Int() );
	
	switch ( iProgress().iStage )
		{
		case KConnectionUninitialised:
Log("connection uninitialized");
			break;

		case KStartingSelection:
Log("starting selection");
			break;

		case KFinishedSelection:
Log("finished selection");
			if (iProgress().iError == KErrNone)
				{
Log("selected");
				// The user successfully selected an IAP to be used
				}
			else
				{
Log("canceled");
				// The user pressed e.g. "Cancel" and did not select an IAP
				}
			break;

		case KConnectionFailure:
Log("connection faiure");
			break;

		// Prepearing connection (e.g. dialing)
		case KPsdStartingConfiguration:
		case KPsdFinishedConfiguration:
		case KCsdFinishedDialling:
		case KCsdScanningScript:
		case KCsdGettingLoginInfo:
		case KCsdGotLoginInfo:
Log("preparing connection");
			break;
		
		// Creating connection (e.g. GPRS activation)
		case KCsdStartingConnect:
		case KCsdFinishedConnect:
Log("creating connection");
			break;

		// Starting log in
		case KCsdStartingLogIn:
Log("starting log in");
			break;

		// Finished login
		case KCsdFinishedLogIn:
Log("finished log in");
			break;

		// Connection open
		case KConnectionOpen:
		case KLinkLayerOpen:
Log("connection open");
			break;
			
		// Connection blocked or suspended
		case KDataTransferTemporarilyBlocked:
Log("data transfer temporarily blocked");
			break;

		// Hangup or GRPS deactivation
		case KConnectionStartingClose:
Log("connection starting closed");
			break;

		// Connection closed
		case KConnectionClosed:
		case KLinkLayerClosed:
			iObserver->HandleConnectionEventL( iProgress().iStage );
Log("connection closed");
			break;

		// Unhandled state
		default:
Log(iProgress().iStage);
Log("unknown stage");
			break;
		}

	iConnection.ProgressNotification(iProgress, iStatus);
	SetActive();
	}

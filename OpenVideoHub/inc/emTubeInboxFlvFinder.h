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

#ifndef EMTUBE_INBOXFLVFINDER_H
#define EMTUBE_INBOXFLVFINDER_H


#include <e32base.h>

#include <msvapi.h>

class TMsvSessionEvent;
class CMsvSession;

_LIT( KFlvExtension, ".flv" );

class CEmTubeInboxFlvFinder : public CBase, public MMsvSessionObserver
	{	
public:
    enum TFlvFinderState
    {
        EFlvFinderNotReady = 0,
        EFlvFinderReady
    };

public: // construction / destruction
	static CEmTubeInboxFlvFinder* NewL();
	static CEmTubeInboxFlvFinder* NewLC(); 	   
	~CEmTubeInboxFlvFinder();
	
private: // construction
	CEmTubeInboxFlvFinder();
	void ConstructL();
	
public: // from MMsvSessionObserver

	void HandleSessionEventL(TMsvSessionEvent aEvent, TAny *aArg1, TAny *aArg2, TAny *aArg3);

public:

	TBool FindFlvL( );

    TBool IsItFlvL( TMsvId aMessageId );
    
    TBool GetFlvL( TMsvId aMessageId, RFile& aFile );

    RArray<TFileName>* GetNamesArray( );

    RArray<TMsvId>* GetIdsArray( ); 	

private: // data
	CMsvSession* iCMsvSession;
	TFlvFinderState iState;    
    RArray<TFileName> iFileNamesArray;
    RArray<TMsvId> iMessagesIds; 	
	};

#endif // EMTUBE_INBOXFLVFINDER_H


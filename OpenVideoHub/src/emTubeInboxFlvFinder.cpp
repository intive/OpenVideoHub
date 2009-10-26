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


#include <MMsvAttachmentManager.h>
#include <Msvids.h>  //KMsvGlobalInBoxIndexEntryId

#include "emTubeInboxFlvFinder.h"
#include "emTubeLog.h"

CEmTubeInboxFlvFinder* CEmTubeInboxFlvFinder::NewL()
	{
	CEmTubeInboxFlvFinder* self = CEmTubeInboxFlvFinder::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeInboxFlvFinder* CEmTubeInboxFlvFinder::NewLC()
	{
	CEmTubeInboxFlvFinder* self = new (ELeave) CEmTubeInboxFlvFinder();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;	
	}

CEmTubeInboxFlvFinder::~CEmTubeInboxFlvFinder()
	{
    delete iCMsvSession;
    iFileNamesArray.Close( );
    iMessagesIds.Close( ); 	
	}

CEmTubeInboxFlvFinder::CEmTubeInboxFlvFinder()//: iState(EFlvFinderNotReady)
	{
	
	}

void CEmTubeInboxFlvFinder::ConstructL()
	{
	iCMsvSession = CMsvSession::OpenAsyncL(*this);    
	}

// from MMsvSessionObserver
void CEmTubeInboxFlvFinder::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/)
	{
	switch(aEvent)
		{
	    case EMsvServerReady:
	    	{
	        iState = EFlvFinderReady;
	        break;
	    	}

	    case KErrNotReady:
	    	{
	        iState = EFlvFinderNotReady;
            break;
		    }

	    default:
	        break;
		}
	}

TBool CEmTubeInboxFlvFinder::FindFlvL( )
    {
Log("--->CEmTubeInboxFlvFinder::FindFlv( )");	        
	CMsvEntry* inboxMessages = iCMsvSession->GetEntryL(KMsvGlobalInBoxIndexEntryId);
	CleanupStack::PushL(inboxMessages);
	
	TBool result = EFalse;

    iFileNamesArray.Reset( );
    iMessagesIds.Reset( ); 	

	if(inboxMessages->Count())
	{		
		CMsvEntrySelection* entries = inboxMessages->ChildrenL();
		CleanupStack::PushL(entries);
 
		for(TInt i=0 ; i < inboxMessages->Count() ; i++)
		{
			if( IsItFlvL( entries->At(i) ) )
			{
			    result = ETrue;
			}
		}
 
		CleanupStack::PopAndDestroy(entries);	
	}
	
	CleanupStack::PopAndDestroy( inboxMessages ); //inboxMessages    

Log("CEmTubeInboxFlvFinder::FindFlv( )--->");	        
    
    return result;
    }

TBool CEmTubeInboxFlvFinder::IsItFlvL(TMsvId aMessageId)
    {    
Log("--->CEmTubeInboxFlvFinder::IsItFlvL(TMsvId aMessageId)");

    CMsvEntry* message = iCMsvSession->GetEntryL(aMessageId);
    CMsvEntrySelection* entries = message->ChildrenL();
    CleanupStack::PushL(entries);
    
    TBool result = EFalse;

    if(entries->Count() >= 1)
    	{
        message->SetEntryL( (*entries)[0] );

        if( message->HasStoreL() )
        	{
            CMsvStore* store = message->ReadStoreL();
            CleanupStack::PushL(store);

            if( !store->HasBodyTextL() )
            	{
                MMsvAttachmentManager& attachmentManager = store->AttachmentManagerL();
                TFileName fileName;
                attachmentManager.GetAttachmentFileL(0).Name( fileName );
                
                if( !KFlvExtension().Compare( fileName.Right(4)) )
                	{
                    iFileNamesArray.Append( fileName );
                    iMessagesIds.Append( aMessageId ); 	
                    result = ETrue;
                    Log( fileName );                    
                	}
            	}

            CleanupStack::PopAndDestroy(store);
        	}
    	}
    CleanupStack::PopAndDestroy(entries);	

Log("CEmTubeInboxFlvFinder::IsItFlvL(TMsvId aMessageId)--->");

    return result;
    }

TBool CEmTubeInboxFlvFinder::GetFlvL( TMsvId aMessageId, RFile& aFile  )
    {
    CMsvEntry* message = iCMsvSession->GetEntryL(aMessageId);
    CMsvEntrySelection* entries = message->ChildrenL();
    CleanupStack::PushL(entries);
    
    TBool result = EFalse;

    if(entries->Count() >= 1)
    	{
        message->SetEntryL( (*entries)[0] );

        if( message->HasStoreL() )
        	{
            CMsvStore* store = message->ReadStoreL();
            CleanupStack::PushL(store);

            if( !store->HasBodyTextL() )
            	{
                MMsvAttachmentManager& attachmentManger = store->AttachmentManagerL();
                TFileName fileName;
                attachmentManger.GetAttachmentFileL(0).Name( fileName );
                
                if( !KFlvExtension().Compare( fileName.Right(4)) )
                	{
                    
                    aFile = attachmentManger.GetAttachmentFileL(0);
                    result = ETrue;
                    Log("GetFlvL");                    
                    Log( fileName );                    
                	}
            	}

            CleanupStack::PopAndDestroy(store);
        	}
    	}
    CleanupStack::PopAndDestroy(entries);	

    return result;        
    }

RArray<TFileName>* CEmTubeInboxFlvFinder::GetNamesArray( )
    {
    return &iFileNamesArray;        
    }

RArray<TMsvId>* CEmTubeInboxFlvFinder::GetIdsArray( )	
    {
    return &iMessagesIds;
    }

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

#include <coeaui.h>
#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <BAUTILS.H>
#include <aknnotewrappers.h>

#include "emTubeResource.h"

#include "emTubeTransferManager.h"
#include "emTubeHttpEngine.h"
#include "emTubeVideoEntry.h"

#include "emTubeAppUi.h"
#include "emTube.hrh"
//queue entry!
CQueueEntry* CQueueEntry::NewL()
	{
	CQueueEntry* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CQueueEntry* CQueueEntry::NewLC()
	{
	CQueueEntry* self = new (ELeave) CQueueEntry();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CQueueEntry::CQueueEntry()
	{
	}

CQueueEntry::~CQueueEntry()
	{
	delete iTitle;
	delete iDescription;
	delete iTags;
	delete iUrl;
	}

void CQueueEntry::ConstructL()
	{
	SetStatus( CQueueEntry::EEntryQueued );
	}

void CQueueEntry::SetUid( TUint32 aUid )
	{
	iUid = aUid;
	}

void CQueueEntry::SetTitleL( const TDesC& aTitle )
	{
	delete iTitle;
	iTitle = aTitle.AllocL();
	}

void CQueueEntry::SetUrlL( const TDesC& aUrl )
	{
	delete iUrl;
	iUrl = aUrl.AllocL();
	}

void CQueueEntry::SetTagsL( const TDesC& aTags )
	{
	delete iTags;
	iTags = aTags.AllocL();
	}

void CQueueEntry::SetDescriptionL( const TDesC& aDescription )
	{
	delete iDescription;
	iDescription = aDescription.AllocL();
	}

void CQueueEntry::SetFilename( const TDesC& aFilename )
	{
	iFilename.Copy( aFilename );
	}

void CQueueEntry::ExternalizeL( RWriteStream& aStream )
    {
    if( iUrl )
        {
        aStream.WriteInt32L( iUrl->Length( ) );
        aStream.WriteL( *iUrl, iUrl->Length( ) );         
        }
     else
        aStream.WriteInt32L( 0 );

    if( iTitle )
        {
        aStream.WriteInt32L( iTitle->Length( ) );
        aStream.WriteL( *iTitle, iTitle->Length( ) ); 
        }
     else
        aStream.WriteInt32L( 0 );

    if( iTags )
        {
        aStream.WriteInt32L( iTags->Length( ) );
        aStream.WriteL( *iTags, iTags->Length( ) ); 
        }
     else
        aStream.WriteInt32L( 0 );

    if( iDescription )
        {
        aStream.WriteInt32L( iDescription->Length( )  );
        aStream.WriteL( *iDescription, iDescription->Length( ) ); 
        }
     else
        aStream.WriteInt32L( 0 );

    aStream.WriteInt32L( iFilename.Length( ) );
    aStream.WriteL( iFilename, iFilename.Length( ) ); 

    aStream.WriteInt32L( iType );

    // in case of aborted transaction
	if( (iStatus == CQueueEntry::EEntryInitializing) || (iStatus == CQueueEntry::EEntryUploading) 
	    || (iStatus == CQueueEntry::EEntryDownloading) )
    	{
        aStream.WriteInt32L( CQueueEntry::EEntryQueued );
    	}
    else
        aStream.WriteInt32L( iStatus );

    aStream.WriteInt32L( iCategory );

    aStream.WriteInt32L( iPublic );

    aStream.WriteInt32L( iSize );

    aStream.WriteInt32L( iUid );
    }

void CQueueEntry::InternalizeL( RReadStream& aStream )
    {
    TInt size;
    RBuf text;
	CleanupClosePushL( text );
    RBuf8 text8;
	CleanupClosePushL( text8 );

    size =  aStream.ReadInt32L( );
    text.ReAllocL( size );
    aStream.ReadL( text, size ); 
    SetUrlL(text);

    size =  aStream.ReadInt32L( );
    if( size > text.Length() ) text.ReAllocL( size );
    aStream.ReadL( text, size ); 
    SetTitleL(text);

    size =  aStream.ReadInt32L( );
    if( size > text.Length() ) text.ReAllocL( size );
    aStream.ReadL( text, size ); 
    SetTagsL(text);

    size =  aStream.ReadInt32L( );
    if( size > text.Length() ) text.ReAllocL( size );
    aStream.ReadL( text, size ); 
    SetDescriptionL( text );

    size =  aStream.ReadInt32L( );
    if( size > text.Length() ) text.ReAllocL( size );
    aStream.ReadL( text, size ); 
    SetFilename( text );

	CleanupStack::PopAndDestroy( &text8 );
	CleanupStack::PopAndDestroy( &text );
    
    iType = (TEntryType) aStream.ReadInt32L( );

	iStatus = (TEntryStatus) aStream.ReadInt32L( );

	iCategory = (TMovieCategory) aStream.ReadInt32L( );

	iPublic = aStream.ReadInt32L( );
		
	iSize = aStream.ReadInt32L( );

	iUid = aStream.ReadInt32L( );
    }

//manager!
CEmTubeTransferManager* CEmTubeTransferManager::NewL()
	{
	CEmTubeTransferManager* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeTransferManager* CEmTubeTransferManager::NewLC()
	{
	CEmTubeTransferManager* self = new (ELeave) CEmTubeTransferManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeTransferManager::CEmTubeTransferManager()
	{
	}

CEmTubeTransferManager::~CEmTubeTransferManager()
	{
	iTMStarted = EFalse;

	iHttpEngine->CancelOperationL();
	delete iHttpEngine;

	iQueue.ResetAndDestroy();
	iQueue.Close();	
	
	iObserverArray.Close();
	}

void CEmTubeTransferManager::ConstructL()
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());
	iHttpEngine = CEmTubeHttpEngine::NewL( iAppUi->PluginManager(), EFalse );
	LoadQueueL();
	}

void CEmTubeTransferManager::AddToQueueL( const TDesC& aName, const TDesC& aUrl, const TDesC& aFilename )
	{
	CQueueEntry* e = CQueueEntry::NewLC();
	e->SetType( CQueueEntry::EEntryDownload );
	e->SetTitleL( aName );
	e->SetFilename( aFilename );
	e->SetUrlL( aUrl );
	iQueue.Append( e );
	CleanupStack::Pop( e );

    SaveQueueL();

    if( iTMStarted ) StartL();
	}

void CEmTubeTransferManager::AddToQueueL( CQueueEntry& aEntry )
	{
	aEntry.SetType( CQueueEntry::EEntryUpload );
	iQueue.Append( &aEntry );
	
    SaveQueueL();

    if( iTMStarted ) StartL();
	}

void CEmTubeTransferManager::StartL( CQueueEntry* aEntry )
	{
	if( iAppUi->SelectedAccessPoint() )
		{
		iHttpEngine->SetAccessPoint( iAppUi->SelectedAccessPoint() );
		}

	switch( aEntry->Type() )
		{
		case CQueueEntry::EEntryUpload:
			iCurrentEntry = aEntry;
			iCurrentEntry->SetStatus( CQueueEntry::EEntryInitializing );

			if( !iAppUi->PluginManager().LoggedIn( aEntry->Uid() ) )
				{
				TBuf<128> userName;
				TBuf<128> password;

				TUint32 uid = aEntry->Uid();
				userName.Copy( iAppUi->PluginManager().Username( uid ) );
				password.Copy( iAppUi->PluginManager().Password( uid ) );

				if( userName.Length() == 0 || password.Length() == 0 )
					{
	 				CAknMultiLineDataQueryDialog* dlg = CAknMultiLineDataQueryDialog::NewL( userName, password );
					if ( dlg->ExecuteLD( R_DIALOG_USERNAME_PASSWORD_QUERY ) )
						{
						iAppUi->PluginManager().SetUsernameL( uid, userName );
						iAppUi->PluginManager().SetPasswordL( uid, password );
						iAppUi->LoginL( aEntry->Uid(), *this, EFalse );
						}
					else
						{
						iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
						iCurrentEntry = NULL;
						StartL();   
						}
					}
				else
					{
					iAppUi->LoginL( aEntry->Uid(), *this, EFalse );
					}
				}
			else
				{
				iAppUi->LoginL( aEntry->Uid(), *this, EFalse );
				}
		break;

		case CQueueEntry::EEntryDownload:
			iCurrentEntry = aEntry;
			iCurrentEntry->SetStatus( CQueueEntry::EEntryInitializing );
			CVideoEntry *e = CVideoEntry::NewLC();
			e->SetUrlL( aEntry->Url() );
			iHttpEngine->DownloadMovieL( aEntry->Filename(), *e, 0, *this, *this );
			CleanupStack::PopAndDestroy( e );
		break;
		}
	}
	
void CEmTubeTransferManager::RemoveFromQueueL( TInt aEntryIndex )
	{
	if( aEntryIndex != KErrNotFound )
		{
	    CQueueEntry::TEntryStatus entryStatus = iQueue[aEntryIndex]->Status();

		if( (entryStatus == CQueueEntry::EEntryInitializing) || ( entryStatus == CQueueEntry::EEntryUploading) 
		    || (entryStatus == CQueueEntry::EEntryDownloading) )
	    	{
	        CancelOperationL();
	        iCurrentEntry = NULL;
	    	}

		CQueueEntry* entry = iQueue[aEntryIndex];
		iQueue.Remove( aEntryIndex );
	    delete entry;

	    SaveQueueL();
	    }
	}

void CEmTubeTransferManager::MoveUpInQueueL( TInt aEntryIndex )
	{
	CQueueEntry* entry = iQueue[aEntryIndex];
	iQueue.Remove( aEntryIndex );
	iQueue.InsertL( entry, aEntryIndex-1 );

    SaveQueueL();        
	}

void CEmTubeTransferManager::MoveDownInQueueL( TInt aEntryIndex )
	{
	CQueueEntry* entry = iQueue[aEntryIndex];
	iQueue.Remove( aEntryIndex );
	iQueue.InsertL( entry, aEntryIndex+1 );

    SaveQueueL();        
	}

RPointerArray<CQueueEntry>& CEmTubeTransferManager::Queue()
	{
	return iQueue;
	}

void CEmTubeTransferManager::RequestFinishedL( TInt aRequest, TDesC8& aResponseBuffer )
	{
	switch( aRequest )
		{
		case CEmTubeHttpEngine::ERequestPreLogin:
			{
			if( !iAppUi->PluginManager().Plugin( iCurrentEntry->Uid() )->ParseLoginResponseL( aResponseBuffer ) )
				{
				iAppUi->ShowErrorNoteL( R_INCORRECT_LOGIN_TXT );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
				iCurrentEntry = NULL;
				}
			else
				{
				iAppUi->LoginL( iCurrentEntry->Uid(), *this, ETrue );
				}
			}
		break;

		case CEmTubeHttpEngine::ERequestLogin:
			if( iAppUi->PluginManager().LoggedIn( iCurrentEntry->Uid() ) )
				{
				iHttpEngine->UploadMovieStep0L( iCurrentEntry->Uid(), *this );
				}
			else
				{
				iAppUi->ShowErrorNoteL( R_INCORRECT_LOGIN_TXT );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
				iCurrentEntry = NULL;
				}
		break;

		case CEmTubeHttpEngine::ERequestMovieUploadStep0:
			{
			TInt ret = iAppUi->PluginManager().Plugin( iCurrentEntry->Uid() )->ParseUploadMovieResponseL( aResponseBuffer );
			if( ret != KErrNone )
				{
				iAppUi->ShowErrorNoteL( R_ERROR_UPLOADING_FILE_TXT );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
				iCurrentEntry = NULL;

				StartL();   
				}
			else
				{
				iHttpEngine->UploadMovieStep1L( iCurrentEntry->Uid(), *iCurrentEntry, *this, *this );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryUploading );
				}
			}
		break;

		case CEmTubeHttpEngine::ERequestMovieUploadStep1:
			{
			TInt ret = iAppUi->PluginManager().Plugin( iCurrentEntry->Uid() )->ParseUploadMovieResponseStep1L( aResponseBuffer );
			if( ret != KErrNone )
				{
				iAppUi->ShowErrorNoteL( R_ERROR_UPLOADING_FILE_TXT );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
				iCurrentEntry = NULL;

				StartL();   
				}
			else
				{
				iHttpEngine->UploadMovieStep2L( iCurrentEntry->Uid(), *iCurrentEntry, *this, *this );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryUploading );
				}
			}
		break;


		case CEmTubeHttpEngine::ERequestMovieUploadStep2:
			{
			TInt ret = iAppUi->PluginManager().Plugin( iCurrentEntry->Uid() )->ParseUploadMovieResponseStep2L( aResponseBuffer );
			if( ret != KErrNone )
				{
				iAppUi->ShowErrorNoteL( R_ERROR_UPLOADING_FILE_TXT );
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
				}
			else
				{
				iCurrentEntry->SetStatus( CQueueEntry::EEntryFinished );
				}
			iCurrentEntry = NULL;
			StartL();   
			}
		break;

		case CEmTubeHttpEngine::ERequestMovieDownload:
		case CEmTubeHttpEngine::ERequestMovieDownload2:
    		{
        	iAppUi->AddToSavedClipsL( iCurrentEntry );
			iCurrentEntry = NULL;

    		StartL();   
    		}
		break;

		default:
			iCurrentEntry = NULL;
		break;
		}
	}

void CEmTubeTransferManager::RequestCanceledL( TInt /*aRequest*/ )
	{
    if(iCurrentEntry)
        {
        CQueueEntry::TEntryStatus entryStatus = iCurrentEntry->Status();    	
    	if( (entryStatus == CQueueEntry::EEntryInitializing) || (entryStatus == CQueueEntry::EEntryUploading) 
    	    || (entryStatus == CQueueEntry::EEntryDownloading) )
        	{
        	if( iTMStarted )
                iCurrentEntry->SetStatus( CQueueEntry::EEntryFailed );
            else    	
                iCurrentEntry->SetStatus( CQueueEntry::EEntryQueued );
            
            iCurrentEntry->SetSize( 0 );                    	    
        	}

    	if( iCurrentEntry->Type() == CQueueEntry::EEntryDownload )
        	{
        	RFs session;
        	session.Connect();
        	BaflUtils::DeleteFile( session, iCurrentEntry->Filename() );                    
            session.Close();        	    
        	}

        iCurrentEntry = NULL;     
        }                    	        
    
    StartL();
	}

TBool CEmTubeTransferManager::CheckDiskSpaceL( const TDesC& aFileName, TInt aSize )
	{
	return iAppUi->CheckDiskSpaceL( aFileName, aSize );
	}

void CEmTubeTransferManager::ShowErrorNoteL( TInt aResourceId )
	{
    for(TInt i=0 ; i < iObserverArray.Count() ; i++)
        {
        iObserverArray[i]->ProgressComplete();
        }

	if( iAppUi->CurrentViewId() == EMTVTransferViewId )
	    iAppUi->ShowErrorNoteL( aResourceId );
	}

void CEmTubeTransferManager::ShowErrorNoteL( const TDesC& aText )
	{
    for(TInt i=0 ; i < iObserverArray.Count() ; i++)
        {
        iObserverArray[i]->ProgressComplete();
        }

	if( iAppUi->CurrentViewId() == EMTVTransferViewId )
    	iAppUi->ShowErrorNoteL( aText );
	}

void CEmTubeTransferManager::ProgressStart( TInt aCompleteSize )
	{
    for(TInt i=0 ; i < iObserverArray.Count() ; i++)
        {
        iObserverArray[i]->ProgressStart( aCompleteSize );
        }

	switch( iCurrentEntry->Type() )
		{
		case CQueueEntry::EEntryUpload:
			iCurrentEntry->SetSize( aCompleteSize );
		break;

		case CQueueEntry::EEntryDownload:
			iCurrentEntry->SetStatus( CQueueEntry::EEntryDownloading );
			iCurrentEntry->SetSize( aCompleteSize );
		break;
		}
	}

void CEmTubeTransferManager::ProgressUpdate( TInt aCurrent, TInt aDownloadSpeed )
	{
    for(TInt i=0 ; i < iObserverArray.Count() ; i++)
        {
        iObserverArray[i]->ProgressUpdate( aCurrent, aDownloadSpeed );
        }

	switch( iCurrentEntry->Type() )
		{
		case CQueueEntry::EEntryUpload:
			iCurrentEntry->SetCurrentSize( aCurrent );
		break;

		case CQueueEntry::EEntryDownload:
			iCurrentEntry->SetCurrentSize( aCurrent );
		break;
		}
	}

void CEmTubeTransferManager::ProgressComplete()
	{
    for(TInt i=0 ; i < iObserverArray.Count() ; i++)
        {
        iObserverArray[i]->ProgressComplete();
        }

	if( iCurrentEntry )
		{
		switch( iCurrentEntry->Type() )
			{
			case CQueueEntry::EEntryUpload:
//do nothing here, status will be set in RequestFinishedL
			break;

			case CQueueEntry::EEntryDownload:
				if( iCurrentEntry->Status() == CQueueEntry::EEntryDownloading )
					{
					if( iCurrentEntry->Size() == iCurrentEntry->CurrentSize() )
						iCurrentEntry->SetStatus( CQueueEntry::EEntryFinished );
					else
						iCurrentEntry->SetStatus( CQueueEntry::EEntryQueued );
					}
			break;
			}
		}
	}

_LIT( KDManagerFilename, "emtdmanager.bin" );
void CEmTubeTransferManager::LoadQueueL( )
{    
	TFileName fileName;
	RFile file;

	RFs session;
	session.Connect();

#ifndef __WINS__
	TParsePtrC parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
	fileName.Copy( parse.Drive() );
	TPath privateDir;
	User::LeaveIfError( session.PrivatePath( privateDir ) );
	fileName.Append( privateDir );
#else
	fileName.Copy( _L("C:\\Data\\") );
#endif

	fileName.Append( KDManagerFilename );
	TInt err = file.Open( session, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );
	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TInt count = 0;
		TRAPD(err, count = stream.ReadInt32L() );
		
		if(err == KErrNone)
    		{
    		for( TInt i=0; i<count; i++ )
    			{
    			CQueueEntry* e = CQueueEntry::NewLC();
    			e->InternalizeL( stream );
    			iQueue.Append( e );
    			CleanupStack::Pop( e );
    			}    		    
    		}
			
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	session.Close();
}

void CEmTubeTransferManager::SaveQueueL()
	{
	TFileName fileName;
	RFile file;

	RFs session;
	session.Connect();

#ifndef __WINS__
	TParsePtrC parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
	fileName.Copy( parse.Drive() );
	TPath privateDir;
	User::LeaveIfError( session.PrivatePath( privateDir ) );
	fileName.Append( privateDir );
#else
	fileName.Copy( _L("C:\\Data\\") );
#endif
	fileName.Append( KDManagerFilename );

	TInt err = file.Replace( session, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );
		
	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		TInt count = iQueue.Count();
		stream.WriteInt32L( count );

		for( TInt i=0; i<count; i++ )
			{
			iQueue[i]->ExternalizeL( stream );
			}
			
		stream.CommitL();	   
		CleanupStack::PopAndDestroy( &stream );	
		}
	CleanupStack::PopAndDestroy( &file );
	session.Close();
	}

void CEmTubeTransferManager::StartL( TBool aStartManager )
    {
    if( aStartManager ) iTMStarted = ETrue;

    if( !iCurrentEntry  && iTMStarted )
        {        
        if( iQueue.Count() > FinishedEntries() )
            {
            TInt which = -1;
            for(TInt i=0 ; i < iQueue.Count() ; i++)
                {
                 if( iQueue[i]->Status() == CQueueEntry::EEntryQueued )
                     {
                      which = i;
                      break;  
                     }             
                }
    		if( which != -1) StartL( iQueue[which] );                        
            }
        }
    }

TInt CEmTubeTransferManager::StopL( )
    {
    TInt idx = KErrNotFound;
	iTMStarted = EFalse;

    if( iCurrentEntry )
    	{
    	idx = iQueue.Find( iCurrentEntry );
        CancelOperationL();
        iCurrentEntry->SetSize(0);
        iCurrentEntry->SetStatus( CQueueEntry::EEntryQueued );
        iCurrentEntry = NULL;
    	}
    
    SaveQueueL();
    return idx;
    }

TBool CEmTubeTransferManager::Processing()
    {
    TBool result = EFalse;
    
    if( iCurrentEntry ) 
        result = ETrue;

    return result;
    }

void CEmTubeTransferManager::CancelOperationL()
    {
	if( iHttpEngine )
		iHttpEngine->CancelOperationL();
    }

void CEmTubeTransferManager::RemoveFinishedEntriesL()
    {
    for(TInt i=iQueue.Count()-1 ; i >= 0  ; i--)
        {
        if( iQueue[i]->Status() == CQueueEntry::EEntryFinished )
            {
        	CQueueEntry* entry = iQueue[i];
        	iQueue.Remove( i );
            delete entry;                
            }
        }    
    SaveQueueL();    
    }

TInt CEmTubeTransferManager::FinishedEntries()
    {
    TInt result = 0;
    
    for(TInt i=0 ; i < iQueue.Count() ; i++)
        if( iQueue[i]->Status() == CQueueEntry::EEntryFinished )
            result++;

    return result;
    }

TInt CEmTubeTransferManager::FailedEntries()
    {
    TInt result = 0;

    for(TInt i=0 ; i < iQueue.Count() ; i++)
        if( iQueue[i]->Status() == CQueueEntry::EEntryFailed )
            result++;

    return result;
    }

TBool CEmTubeTransferManager::AllProcessed()
    {
    TInt result = 0;
    
    for(TInt i=0 ; i < iQueue.Count() ; i++)
        if(  (iQueue[i]->Status() == CQueueEntry::EEntryFinished ) ||
             (iQueue[i]->Status() == CQueueEntry::EEntryFailed ) )
            result++;
    
    return ( result == iQueue.Count() );
    }

TBool CEmTubeTransferManager::SafeToMoveEntry( TInt aEntryIndex, TBool aMoveUp )
    {
    CQueueEntry::TEntryStatus entry = iQueue[aEntryIndex]->Status();

    CQueueEntry::TEntryStatus replacedEntry;
    if( aMoveUp ) 
        replacedEntry = iQueue[aEntryIndex-1]->Status();
    else
        replacedEntry = iQueue[aEntryIndex+1]->Status();

    if( ( (entry == CQueueEntry::EEntryInitializing) ||
        (entry == CQueueEntry::EEntryUploading) ||
        (entry == CQueueEntry::EEntryDownloading) ) && 
        (replacedEntry == CQueueEntry::EEntryQueued) )
        {
        return EFalse;
        }
            
    if( ( (replacedEntry == CQueueEntry::EEntryInitializing) ||
        (replacedEntry == CQueueEntry::EEntryUploading) ||
        (replacedEntry == CQueueEntry::EEntryDownloading) ) &&
        (entry == CQueueEntry::EEntryQueued) )
        {
        return EFalse;
        }
    
    return ETrue;
    }

TBool CEmTubeTransferManager::EntryAdded( TDesC& aEntryUrl )
    {
    for(TInt i=0; i < iQueue.Count() ; i++)
        {
        if( iQueue[i]->Url().Compare( aEntryUrl ) == 0 )
            return ETrue;   
        }

    return EFalse;   
    }

void CEmTubeTransferManager::RegisterObserverL( MProgressObserver& aObserver )
    {
    TInt where = iObserverArray.Find( &aObserver ) ;
    if( where == KErrNotFound )
        iObserverArray.AppendL( &aObserver );
    }

void CEmTubeTransferManager::UnRegisterObserver( MProgressObserver& aObserver )
    {
    TInt where = iObserverArray.Find( &aObserver ) ;
    if( where != KErrNotFound )
        iObserverArray.Remove( where );
    }

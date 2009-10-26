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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <e32svr.h>
#include <e32std.h>
#include <e32base.h>

#include "emTubePlayerDemuxer.h"
#include "emTubePlayerTimer.h"
#include "emTubevideoEntry.h"

#include "avlibvideo.h"
#include "avlibaudio.h"

#define KMaxAudioBuffers 32
#define KMaxVideoBuffers 16

CDemuxer* CDemuxer::NewL(MPlayerObserver *aObserver)
	{
	 CDemuxer* self = CDemuxer::NewLC(aObserver);
	 CleanupStack::Pop( self );
	 return self;
	}

CDemuxer* CDemuxer::NewLC(MPlayerObserver *aObserver)
	{
	 CDemuxer* self = new ( ELeave ) CDemuxer();
	 CleanupStack::PushL( self );
	 self->ConstructL( aObserver );
	 return self;
	}

void CDemuxer::ConstructL( MPlayerObserver *aObserver )
	{
	iShared.iObserver = aObserver;
	User::LeaveIfError( iShared.iAliveMutex.CreateLocal( 1 ) );
	User::LeaveIfError( iShared.iMutex.CreateLocal() );
	User::LeaveIfError( iDemuxerThread.Create( _L("Demuxer"),
						CDemuxerThread::ThreadFunction,
						0x5000,
						NULL,
						&iShared));

	iDemuxerThread.SetPriority( EPriorityRealTime );
	iDemuxerThread.Resume();
	}

CDemuxer::CDemuxer()
	{
	}

CDemuxer::~CDemuxer()
	{
	SendCmd( EDemuxerCmdDestroy );
	iShared.iAliveMutex.Wait();
	iShared.iAliveMutex.Close();
	iShared.iMutex.Close();
	iDemuxerThread.Close();
	}

TBool CDemuxer::HasAudio()
	{
	return iShared.iVideoProperties.iHasAudio;
	}

TBool CDemuxer::HasVideo()
	{
	return iShared.iVideoProperties.iHasVideo;
	}

TBool CDemuxer::FindKeyFrameL( TUint32 aPts, TSearchMode aMode, TSeekSpeed aSpeed )
	{
	TBool res = EFalse;

	switch( aMode )
		{
		case EForward:
			{
			TInt count = iShared.iIndices.Count();

			aPts += ( aSpeed * TIMER_BASE );
				
			TInt idx = count - 1;
			for( TInt i=0;i<count;i++ )
				{
				if( iShared.iIndices[i].iPts > aPts )
					{
					idx = i;
					break;
					}
				}
			SeekL( iShared.iIndices[ idx ].iPosition - 4 );
			res = ETrue;
			}
		break;

		case EBackward:
			{
			TInt count = iShared.iIndices.Count();

			TInt32 pts = (TInt32)aPts - ( aSpeed * TIMER_BASE );
			if( pts < 0 ) pts = 0;

			TInt idx = 0;
			for( TInt i=count-1;i>=0;i-- )
				{
				if( (TInt32)iShared.iIndices[ i ].iPts < pts )
					{
					idx = i;
					break;
					}
				}
			SeekL( iShared.iIndices[ idx ].iPosition - 4 );
			res = ETrue;
			}
		break;
		
		case EExact:
			{
			TInt count = iShared.iIndices.Count();
			for( TInt i=0;i<count;i++ )
				{
				if( iShared.iIndices[i].iPts == aPts )
					{
					SeekL( iShared.iIndices[i].iPosition - 4 );
					res = ETrue;
					break;
					}
				}
			}
		break;
		}
	return res;
	}

void CDemuxer::SeekPercentageL( TInt aPercentage )
	{
	TInt idx  = (iShared.iIndices.Count() - 1 ) * aPercentage / 100;
	SeekL( iShared.iIndices[ idx ].iPosition - 4 );
	}

void CDemuxer::SeekL( TInt aPosition )
	{
	if( Seekable() || aPosition == 0 )
		{
		if( aPosition == 0 )
			{
			if( iShared.iIndices.Count() )
				aPosition = iShared.iIndices[0].iPosition - 4;
			else
				aPosition = iShared.iVideoProperties.iStartOfData;
			}
			
		iShared.iMutex.Wait();
		iShared.iEof = EFalse;
		iShared.iSeekPosition = aPosition;
		iShared.iMutex.Signal();

		SendCmd( EDemuxerCmdSeek );
		}
	}

void CDemuxer::SetEof( TBool aEof )
	{
	iShared.iMutex.Wait();
	iShared.iEof = aEof;
	iShared.iMutex.Signal();

	SendCmd( EDemuxerCmdSetEof );
	}

TBool CDemuxer::Seekable()
	{
	if( iShared.iIndices.Count() )
		return ETrue;
	else
		return EFalse;
	}

void CDemuxer::EnableAudio()
	{
	iShared.iMutex.Wait();
	iShared.iEnableAudio = ETrue;
	iShared.iMutex.Signal();
	}
	
void CDemuxer::EnableVideo()
	{
	iShared.iMutex.Wait();
	iShared.iEnableVideo = ETrue;
	iShared.iMutex.Signal();
	}

TBool CDemuxer::AudioEnabled()
	{
	return iShared.iEnableAudio;
	}
	
TBool CDemuxer::VideoEnabled()
	{
	return iShared.iEnableVideo;
	}

void CDemuxer::DisableAudio()
	{
	iShared.iMutex.Wait();
	iShared.iEnableAudio = EFalse;
	iShared.iMutex.Signal();
	}
	
void CDemuxer::DisableVideo()
	{
	iShared.iMutex.Wait();
	iShared.iEnableVideo = EFalse;
	iShared.iMutex.Signal();
	}

void CDemuxer::DeletePacket( TAny* aPacket )
	{
	Packet* pkt = (Packet*)aPacket;
	if( pkt )
		{
		if( pkt->iData )
			free( pkt->iData );
		free( pkt );
		}
	}

TAny* CDemuxer::ReadAudioPacket( TBool& aEof )
	{
	TAny* pkt = NULL;
	TBool needMoreData = EFalse;

	iShared.iMutex.Wait();
	
	TInt count = iShared.iAudioBuffers.Count();
	
	if( iShared.iEof && !count)
		{
 		aEof = ETrue;
		iShared.iMutex.Signal();
		return pkt; 		
 		}
	
	if( count )
		{
		pkt = iShared.iAudioBuffers[0];
		iShared.iAudioBuffers.Remove(0);
		}
	if( iShared.iAudioBuffers.Count() < (KMaxAudioBuffers/2) )
		needMoreData = ETrue;
	
	iShared.iMutex.Signal();

	if(needMoreData)
		{
		SendCmd( EDemuxerCmdFillAudio );
		}

	return pkt;
	}

TAny* CDemuxer::ReadVideoPacket( TBool& aEof )
	{
	TAny* pkt = NULL;
	TBool needMoreData = EFalse;

	iShared.iMutex.Wait();
	
	TInt count = iShared.iVideoBuffers.Count();
	
	if( iShared.iEof && !count)
		{
 		aEof = ETrue;
		iShared.iMutex.Signal();
		return pkt; 		
 		}

	if( count )
		{
		pkt = iShared.iVideoBuffers[0];
		iShared.iVideoBuffers.Remove(0);
		}
	if( iShared.iVideoBuffers.Count() < (KMaxVideoBuffers/2) )
		needMoreData = ETrue;
	
	iShared.iMutex.Signal();

	if(needMoreData)
		{
		SendCmd( EDemuxerCmdFillVideo );
		}

	return pkt;
	}

void CDemuxer::OpenFileL( const TDesC& aFile )
	{
	iShared.iFilename.Copy( aFile );
	SendCmd( EDemuxerCmdOpenFile );
	}

void CDemuxer::OpenFileL( RFile& aFile )
	{
	iShared.iFile.Duplicate( aFile );
	iShared.iFilename.Copy( KNullDesC() );
	SendCmd( EDemuxerCmdOpenFile );
	}

void CDemuxer::CloseFileL()
	{
	SendCmd( EDemuxerCmdCloseFile );
	}

void CDemuxer::SendCmd( TDemuxerCmd aCmd )
	{
	iShared.iMutex.Wait();
	iShared.iCmd = aCmd;
	iShared.iMutex.Signal();

	iShared.iExc = EExcUserInterrupt;
	 
	TRequestStatus* status = iShared.iStatusPtr;
	if( status && status->Int() == KRequestPending )
		{
		iDemuxerThread.RequestComplete(status, KErrNone);
		}
	else
		{
		}
	}

TInt CDemuxerThread::ThreadFunction( TAny* aData )
	{
	TDemuxerShared& shared = *((TDemuxerShared*)aData);
	shared.iAliveMutex.Wait();

	CDemuxerThread* demuxerThread = NULL;
  
	TRAPD(error, demuxerThread = CDemuxerThread::CreateL( aData ));
	if(error!=KErrNone)
		{
		return error;
		}

	if( demuxerThread == NULL )
		{
		return KErrGeneral;
		}

	shared.iStatusPtr = &(demuxerThread->iActive->iStatus);

	shared.iObserver->MPODemuxerCreated();

	// if we're still here, activescheduler has been constructed
	// start wait loop which runs until it's time to end the thread
	CActiveScheduler::Start();
	delete demuxerThread;

	// tell owning thread it's safe to exit
	shared.iAliveMutex.Signal();

	 return KErrNone;
	}


CDemuxerThread* CDemuxerThread::CreateL( TAny* aData )
	{
	CDemuxerThread* self = new( ELeave )CDemuxerThread( aData );
	if( self == NULL ) return self;
	if( self->Construct() != KErrNone )
		{
		delete self;
		return NULL;
		}

	return self;
	}

TInt CDemuxerThread::Construct()
	{
	iCleanupStack = CTrapCleanup::New();
	if( iCleanupStack == NULL )
		{
		return KErrNoMemory;
		}

	TInt err = KErrNone;
	TRAP( err, ConstructL() );
	return err;
	}

void CDemuxerThread::ConstructL()
	{
	iFlv = CFlvDemuxer::NewL();

	// create active scheduler
	iActiveScheduler = new( ELeave )CActiveScheduler;
	CActiveScheduler::Install( iActiveScheduler );

	iActive = new(ELeave) MyActive(this);
	iActive->Request();
	}

CDemuxerThread::CDemuxerThread( TAny* aData )
	: iShared( *((TDemuxerShared*)aData) )
	{
	}

void CDemuxerThread::CloseFileL()
	{
	iShared.iMutex.Wait();

	while( iShared.iAudioBuffers.Count() )
		{
		Packet* pkt = (Packet*)iShared.iAudioBuffers[0];
		iShared.iAudioBuffers.Remove( 0 );
		iFlv->FreePacket( pkt );
		}

	while( iShared.iVideoBuffers.Count() )
		{
		Packet* pkt = (Packet*)iShared.iVideoBuffers[0];
		iShared.iVideoBuffers.Remove( 0 );
		iFlv->FreePacket( pkt );
		}

	iShared.iAudioBuffers.Close();
	iShared.iVideoBuffers.Close();

	iShared.iIndices.Close();

	if( iFlv )
		iFlv->CloseFileL();

	iShared.iMutex.Signal();  
	}

CDemuxerThread::~CDemuxerThread()
	{
	CloseFileL();
	delete iFlv;

	delete iActiveScheduler;
	delete iCleanupStack;

	if(iActive)
		{
		iActive->Cancel();
		}
	delete iActive;
	}

void CDemuxerThread::HandleExceptionL()
	{

	switch( iShared.iExc )
		{
		case EExcUserInterrupt:				// Command from client
			{
			switch( iShared.iCmd )
				{
				case EDemuxerCmdOpenFile:
					{
					TInt ret = KErrNone;
					TInt err = KErrNone;
					iShared.iEof = EFalse;
					iShared.iIndices.Close();
					TRAP( err, ret = iFlv->OpenFileL( iShared.iFilename, iShared.iFile, iShared.iIndices, iShared.iVideoProperties ) );
					if( ret == KErrNone )
						{
						if( !iShared.iVideoProperties.iHasAudio )
							iShared.iEnableAudio = EFalse;

						if( !iShared.iVideoProperties.iHasVideo )
						 	iShared.iEnableVideo = EFalse;

						if( iShared.iVideoProperties.iHasAudio )
							iShared.iEof = iFlv->FillBuffersL( iShared.iVideoBuffers, iShared.iAudioBuffers, CFlvDemuxer::EAudio );

						if( iShared.iVideoProperties.iHasVideo )
							iShared.iEof = iFlv->FillBuffersL( iShared.iVideoBuffers, iShared.iAudioBuffers, CFlvDemuxer::EVideo );
						else
							err = KErrNotSupported;
						}

					if( err != KErrNone )
						ret = KErrNotSupported;

					iShared.iObserver->MPOFileOpenedL( ret );
					}
				break;

				case EDemuxerCmdSetEof:
					{
					iFlv->SetEof( EFalse );
					}
				break;

				case EDemuxerCmdFillAudio:
					{
					iShared.iMutex.Wait();
					TRAP_IGNORE( iShared.iEof = iFlv->FillBuffersL( iShared.iVideoBuffers, iShared.iAudioBuffers, CFlvDemuxer::EAudio ) );
					iShared.iMutex.Signal();
					}
				break;

				case EDemuxerCmdFillVideo:
					{
					iShared.iMutex.Wait();
					TRAP_IGNORE( iShared.iEof = iFlv->FillBuffersL( iShared.iVideoBuffers, iShared.iAudioBuffers, CFlvDemuxer::EVideo ) );
					iShared.iMutex.Signal();
					}
				break;

				case EDemuxerCmdCloseFile:
					{
					CloseFileL();
					iShared.iObserver->MPOFileClosedL();
					}
				break;

				case EDemuxerCmdSeek:
					{
					iShared.iMutex.Wait();
					while( iShared.iAudioBuffers.Count() )
						{
						Packet* pkt = (Packet*)iShared.iAudioBuffers[0];
						iShared.iAudioBuffers.Remove( 0 );
						iFlv->FreePacket( pkt );
						}

					while( iShared.iVideoBuffers.Count() )
						{
						Packet* pkt = (Packet*)iShared.iVideoBuffers[0];
						iShared.iVideoBuffers.Remove( 0 );
						iFlv->FreePacket( pkt );
						}

					iFlv->Seek( iShared.iSeekPosition );
					iShared.iEof = iFlv->FillBuffersL( iShared.iVideoBuffers, iShared.iAudioBuffers, CFlvDemuxer::EVideo );

					iShared.iMutex.Signal();
					iShared.iObserver->MPOSeekDoneL();
					}
				break;

				case EDemuxerCmdDestroy:
					{
					CActiveScheduler::Stop();		// Exit
					}
				break;

				}
			break;
			}
		default:
			{
			// if unknown exception, just exit this thread
			CActiveScheduler::Stop();				// Exit
			break;
			}
		}
	}

MyActive::MyActive(MThreadHandler* aThread): CActive(CActive::EPriorityHigh), iThread(aThread)
	{
	CActiveScheduler::Add(this) ; 
	}

void MyActive::DoCancel()
	{
	TRequestStatus* status = &iStatus;
	RThread().RequestComplete(status,KErrCancel);
	}

void MyActive::Request()
	{
	SetActive();  
	iStatus = KRequestPending;
	}

void MyActive::RunL()
	{
	if(iStatus == KErrNone)
		{
		iThread->HandleExceptionL();
		Request();
		}
	}

TInt MyActive::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}


//file buffer
CFileBuffer* CFileBuffer::NewL( RFile& aFile )
	{
	CFileBuffer* self = CFileBuffer::NewLC( aFile );
	CleanupStack::Pop( self );
	return self;
	}

CFileBuffer* CFileBuffer::NewLC( RFile& aFile )
	{
	CFileBuffer* self = new ( ELeave ) CFileBuffer( aFile );
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

void CFileBuffer::ConstructL()
	{
	iFile.Size( iFileSize );
	ptr = FileBuffer.Ptr();
	BytesInBuffer = 0;
	pos = 0;
	endOfFile = 0;
	}

CFileBuffer::CFileBuffer( RFile& aFile ) : iFile(aFile)
	{
	}

CFileBuffer::~CFileBuffer()
	{
	}

void CFileBuffer::SkipData(TInt size)
	{
	if(size <= 0) return;

	BytesInBuffer -= size;
	ptr += size;
	pos += size;
	while(BytesInBuffer < 0)
		{
		int t = 0 - BytesInBuffer;
		ptr = FileBuffer.Ptr();
		
		iFile.Read( FileBuffer );
		size = FileBuffer.Length();

		if(size == 0)
			{
			endOfFile = 1;
			return;
			}

		BytesInBuffer = (size - t);
		ptr += t;
	}
}

void CFileBuffer::GetData(TAny* where, TInt size)
{
	if(BytesInBuffer >= size)
	{
		memcpy(where, ptr, size);
		BytesInBuffer -= size;
		ptr += size;
		pos += size;
	}
	else
	{
		char *t = (char *)where;
		memcpy(t, ptr, BytesInBuffer);
		t += BytesInBuffer;
		size -= BytesInBuffer;
		ptr = FileBuffer.Ptr();
		pos += BytesInBuffer;

		iFile.Read( FileBuffer );
		BytesInBuffer = FileBuffer.Length();
		if(BytesInBuffer == 0)
			{
			endOfFile = 1;
			return;
			}
		GetData(t,size);
		}
	}

TUint8 CFileBuffer::GetByte()
	{
	if(BytesInBuffer <= 0)
		{
		ptr = FileBuffer.Ptr();
		iFile.Read( FileBuffer );
		BytesInBuffer = FileBuffer.Length();
		if(BytesInBuffer == 0)
			{
			endOfFile = 1;
			return 0;
			}
		}
	pos++;
	BytesInBuffer -= 1;
	return *ptr++;
	}

TInt16 CFileBuffer::GetWord()
	{
	TUint8 c, c1;
	TUint16 r;

	c = GetByte();
	c1 = GetByte();
	r = (c1<<8) + c;
	return r;
	}

TInt32 CFileBuffer::GetLong()
	{
	TUint8 c, c1, c2, c3;
	TUint32 r;

	c = GetByte();
	c1 = GetByte();
	c2 = GetByte();
	c3 = GetByte();
	r = (c3<<24) + (c2<<16) + (c1<<8) + c;
	return r;
	}

TInt16 CFileBuffer::GetWordBE()
	{
	TUint8 c, c1;
	TUint16 r;

	c = GetByte();
	c1 = GetByte();
	r = (c<<8) + c1;
	return r;
	}

TInt32 CFileBuffer::GetLongBE()
	{
	TUint8 c, c1, c2, c3;
	TUint32 r;

	c = GetByte();
	c1 = GetByte();
	c2 = GetByte();
	c3 = GetByte();
	r = (c<<24) + (c1<<16) + (c2<<8) + c3;
	return r;
	}

TInt CFileBuffer::Position()
	{
	return pos;
	}

void CFileBuffer::Seek( TInt aPosition )
	{
	iFile.Seek( ESeekStart, aPosition );
	BytesInBuffer = 0;
	pos = aPosition;
	endOfFile = 0;
	}

void CFileBuffer::SetPosition( TInt aPosition )
	{
	iFile.Seek( ESeekStart, aPosition );
	BytesInBuffer = 0;
	pos = aPosition;
	}

//FlvDemuxer
CFlvDemuxer* CFlvDemuxer::NewL()
	{
	CFlvDemuxer* self = CFlvDemuxer::NewLC();
	CleanupStack::Pop( self );
	return self;
	}

CFlvDemuxer* CFlvDemuxer::NewLC()
	{
	CFlvDemuxer* self = new ( ELeave ) CFlvDemuxer();
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

void CFlvDemuxer::ConstructL()
	{
	iSession.Connect();
	}

CFlvDemuxer::CFlvDemuxer()
	{
	}

CFlvDemuxer::~CFlvDemuxer()
	{
	iFile.Close();
	iSession.Close();
	}

void CFlvDemuxer::SetEof(TBool aEof )
	{
	iFileBuffer->SetEof( aEof );
	}
	
TInt CFlvDemuxer::FileSize()
	{
	return iFileBuffer->FileSize();
	}

void CFlvDemuxer::Seek( TInt aPos )
	{
	iFileBuffer->Seek( aPos );
	}

void CFlvDemuxer::CloseFileL()
	{
	delete iFileBuffer;
	iFileBuffer = NULL;
	}

TBool CFlvDemuxer::IsFlv( TUint8* aBuf )
	{
	 if (aBuf[0] == 'F' && aBuf[1] == 'L' && aBuf[2] == 'V' && aBuf[3] < 5 )
		{
		return ETrue;
		}
	return EFalse;
	}

TReal64 CFlvDemuxer::GetDoubleValFromMetadata( const TUint8* aPtr, TInt& aOffset )
	{
	aPtr += aOffset;
	aOffset += 8;

	TUint64 val = 0;
	for(TInt i=0;i<8;i++)
	  	{
		val <<= 8;
		val |= *(aPtr + i);
		}

	union
		{
		TUint64 src;
		TReal64 dst;
		}conv;
	conv.src = val;
	return conv.dst;
	}

TUint32 CFlvDemuxer::GetIntValFromMetadata( const TUint8* aPtr, TInt& aOffset )
	{
	aPtr += aOffset;
	aOffset += 4;

	TUint8 c, c1, c2, c3;
	TUint32 r;

	c = aPtr[0];
	c1 = aPtr[1];
	c2 = aPtr[2];
	c3 = aPtr[3];
	r = (c<<24) + (c1<<16) + (c2<<8) + c3;
	return r;
	}

TUint32 CFlvDemuxer::GetDurationFromMetadata( const TDesC8& aMetadata )
	{
	TUint32 duration = 0;

	TInt offset = aMetadata.Find( _L8("duration") );
	if( offset != KErrNotFound )
		{
		offset += 8; //"duration"
		if( aMetadata.Length() > (offset + 9)  )
			{
			const TUint8* ptr = aMetadata.Ptr();
			offset++; //type
			duration = (TUint32)GetDoubleValFromMetadata( ptr, offset );
			}
		}
	return duration;
	}

void CFlvDemuxer::GetIndicesFromMetadata( const TDesC8& aMetadata, const TDesC8& aType, RArray<TIndexEntry>& aIndices )
	{
	TInt offset = 0;
	const TUint8* ptr = aMetadata.Ptr();
	
	while(1)
		{
		TInt idx = 0;
		TPtrC8 data = aMetadata.Mid( offset );
		
		idx = data.Find( aType );
		if( idx != KErrNotFound )
			{
			offset += idx;
			if( ptr[ offset + aType.Length() ] == 0x0A )
				{
				break;
				}
			}
		else
			{
			return;
			}
		offset++;
		}

	offset += aType.Length();
	TUint8 type = ptr[offset++]; //type == array
	TInt32 count = GetIntValFromMetadata( ptr, offset );

	TInt add = 1;
	if( aIndices.Count() )
		add = 0;

	for (TInt i=0;i< count;i++)
		{
		type =  ptr[offset++]; //type == number

		TReal64 val = GetDoubleValFromMetadata( ptr, offset );

		if( !add )
			{
			if( !aType.Compare(_L8("times" ) ) )
				aIndices[i].iPts = (TUint32)( val * (TReal)TIMER_BASE );
			else
				aIndices[i].iPosition = (TUint32)val;
			}
		else
			{
			TIndexEntry entry;
			if( !aType.Compare(_L8("times" ) ) )
				entry.iPts = (TUint32)( val * (TReal)TIMER_BASE );
			else
				entry.iPosition = (TUint32)val;
			aIndices.Append( entry );
			}
		}
	}

 TInt CFlvDemuxer::OpenFileL( TDesC& aFileName, RFile& aFile, RArray<TIndexEntry>& aIndices, TVideoProperties& aProperties )
	{
	delete iFileBuffer;
	iFileBuffer = NULL;

	iFileName.Copy( aFileName );

	if( aFileName.Length() )
		{
		TInt err = iFile.Open( iSession, aFileName, EFileStream | EFileRead | EFileShareAny );
		if( err != KErrNone )
			return err;
		iFileBuffer = CFileBuffer::NewL( iFile );
		}
	else
		{
		iFileBuffer = CFileBuffer::NewL( aFile );
		}

	iFileBuffer->Seek( 0 );

	TUint8 hdr[5];
	iFileBuffer->GetData( hdr, 5 );

	if( !IsFlv( hdr ) )
		{
		return KErrNotFound;
	 	}

	aProperties.iHasAudio = aProperties.iHasVideo = EFalse;

#define INT24(p) ((p[0] << 16) + (p[1] << 8) + p[2])
#define FLV_TYPE_AUDIO 8
#define FLV_TYPE_VIDEO 9
#define FLV_TYPE_OTHER 18

	int prevPTS = 0;
	aProperties.iFps = 0.0f;
	aProperties.iDuration = 0;
	aProperties.iVideoExtraDataSize = 0;

	TInt offset = iFileBuffer->GetLongBE();

	iFileBuffer->Seek( offset );
	aProperties.iStartOfData = iFileBuffer->Position();

	TBool done = EFalse;
	
	while( !done )
	{
		int size, pts;
		unsigned char tmp[3];
		unsigned char type;

		if( iFileBuffer->EndOfFile() ) break;

		iFileBuffer->GetLongBE(); //previous packet size
		type = iFileBuffer->GetByte();
		iFileBuffer->GetData(tmp, 3);
		size = INT24(tmp);

		iFileBuffer->GetData(tmp, 3);
		pts = INT24(tmp);
		pts |= ( iFileBuffer->GetByte() << 24 ); //extendedPTS

		iFileBuffer->SkipData(3); //stream id

		switch(type)
		{
			case FLV_TYPE_VIDEO:
			{
				unsigned char flags;

				if(prevPTS && aProperties.iFps == 0.0f)
				{
					float diff = pts - prevPTS;
					aProperties.iFps = 1.0f / (diff / (float)TIMER_BASE );
					aProperties.iFrametime = (TUint32)( (float)TIMER_BASE / aProperties.iFps );
					aProperties.iHasVideo = 1;
					done = ETrue;
				}
				prevPTS = pts;

				flags = iFileBuffer->GetByte();
				size -= 1;

				switch(flags & 0xF)
				{
					case 2:
					{
						unsigned char tmp[9];
						int format, width, height;

						iFileBuffer->GetData(tmp, 9);
						size -= 9;


						aProperties.iVideoCodecId = VIDEO_CODEC_FLV;

						format = ((tmp[3] & 0x3) << 1) | ((tmp[4] >> 7) & 1);
						switch (format)
						{
							case 0:
								width = (tmp[4] & 0x7f) << 1;
								width += ((tmp[5] >> 7) & 1);

								height = (tmp[5] & 0x7f) << 1;
								height += ((tmp[6] >> 7) & 1);
							break;

							case 1:
								width = (tmp[4] & 0x7f) << 9;
								width += (tmp[5] << 1);
								width += ((tmp[6] >> 7) & 1);

								height = (tmp[6] & 0x7f) << 9;
								height += (tmp[7] << 1);
								height += ((tmp[8] >> 7) & 1);
							break;

							case 2:
								width = 352;
								height = 288;
							break;

							case 3:
								width = 176;
								height = 144;
							break;

							case 4:
								width = 128;
								height = 96;
							break;

							case 5:
								width = 320;
								height = 240;
							break;

							case 6:
								width = 160;
								height = 120;
							break;
						
							default:
								width = height = 0;
							break;
						}

						aProperties.iWidth = width;
						aProperties.iHeight = height;
					}
					break;
					
					case 3:
					{
						TUint16 tmp[2];
						aProperties.iVideoCodecId = VIDEO_CODEC_FLASHSV;

						iFileBuffer->GetData(tmp, 4);
						size -= 4;

						aProperties.iWidth = tmp[0] & 0x0fff;
						aProperties.iHeight = tmp[1] & 0x0fff;
					}
					break;

					case 4:
					case 5:
					{
						unsigned char tmp[4];

						if( (flags & 0xF) == 5 )
							aProperties.iVideoCodecId = VIDEO_CODEC_VP6A;
						else
							aProperties.iVideoCodecId = VIDEO_CODEC_VP6F;
						
						TUint8 extraData = iFileBuffer->GetByte();
						size -= 1;

						iFileBuffer->GetData(tmp, 4);
						size -= 4;

						aProperties.iVideoExtraData = extraData;
						aProperties.iVideoExtraDataSize = 1;

						if(!(tmp[0] & 0x80))
						{
							aProperties.iWidth = tmp[3] * 16;
							aProperties.iHeight = tmp[2] * 16;

							aProperties.iWidth -= extraData >> 4;
							aProperties.iHeight -= extraData & 0x0F;
						}
					}
					break;

					default:
					return KErrNotFound;
				}
				iFileBuffer->SkipData(size);
			}
			break;

			case FLV_TYPE_AUDIO:
			{
				unsigned char flags;
				int channels;
				int rate;
				int bitspersample;

				flags = iFileBuffer->GetByte();
				size -= 1;

				if((flags >> 4) == 5)
					rate = 8000;
				else
					rate = (44100<<((flags>>2)&3))>>3;

				channels = (flags&1)+1;
				bitspersample = (flags & 2) ? 16 : 8;

				aProperties.iHasAudio = 1;
				aProperties.iSampleRate = rate;
				aProperties.iBitsPerSample = bitspersample;
				aProperties.iChannels = channels;

				switch(flags >> 4)
				{
/*						case 0:
						if (flags&2)
							d->Properties.aTracks[0].codecID = MAKE_ID('t','w','o','s');
						else
							d->Properties.aTracks[0].codecID = MAKE_ID('p','c','m',' ');
					break;

					case 1:
						d->Properties.aTracks[0].codecID = MAKE_ID('A','S','W','F');
					break;
*/
					case 2:
						aProperties.iAudioCodecId = AUDIO_CODEC_MP3;
					break;
						
/*					case 3:
						if (flags&2)
							d->Properties.aTracks[0].codecID = MAKE_ID('s','o','w','t');
						else
							d->Properties.aTracks[0].codecID = MAKE_ID('p','c','m',' ');
					break;
*/					 
					default:
					return KErrNotFound;
					}

				iFileBuffer->SkipData(size);
			}
			break;

			case FLV_TYPE_OTHER:
			{
				unsigned char *buf = (unsigned char *)calloc(1, size);
				iFileBuffer->GetData( buf, size );

				if( iFileBuffer->EndOfFile() )
					break;
				
				TPtrC8 metadata( buf, size );

				aProperties.iDuration = GetDurationFromMetadata( metadata );
				GetIndicesFromMetadata( metadata, _L8("times"), aIndices );
				GetIndicesFromMetadata( metadata, _L8("filepositions"), aIndices );

				free(buf);

			}
			break;

			default:
				iFileBuffer->SkipData(size);
			break;
		}
	}

	if( aProperties.iDuration == 0 && aFileName.Length() && aFileName.Compare( KVideoTmpName() ) )
	{
		unsigned char tmp[3];
		TInt offset;
		TInt size;
		iFileBuffer->Seek( iFileBuffer->FileSize() - 4 );

		offset = iFileBuffer->GetLongBE();

		iFileBuffer->Seek( iFileBuffer->FileSize() - 3 - offset );

		iFileBuffer->GetData( tmp, 3);
		size = INT24(tmp);
		if( size + 11 == offset )
		{
			iFileBuffer->GetData( tmp, 3);
			TInt pts = INT24(tmp);
			aProperties.iDuration = pts / 1000;
		}
	}

	iFileBuffer->Seek( aProperties.iStartOfData );



	return KErrNone;
	}

void CFlvDemuxer::FreePacket( Packet *pkt )
	{
	if( pkt )
		{
		if( pkt->iData )
			free( pkt->iData );
		free( pkt );
		}
	}

TInt CFlvDemuxer::ReadPacket( Packet **pkt )
	{
	TInt rollBackPosition = iFileBuffer->Position();
	
	if( iFileBuffer->EndOfFile() )
		{
		return -1;
		}

	int size, pts;
	unsigned char tmp[3];
	unsigned char type;

	iFileBuffer->GetLongBE(); //previous packet size
	type = iFileBuffer->GetByte();
	iFileBuffer->GetData( tmp, 3);
	size = INT24(tmp);

	if( size == 0 )
		return -1;

	iFileBuffer->GetData( tmp, 3);
	pts = INT24(tmp);
	pts |= ( iFileBuffer->GetByte() << 24 ); //extendedPTS

	iFileBuffer->SkipData(3); //stream id

	Packet *p = (Packet *)calloc( 1, sizeof (Packet) );
	*pkt = p;
	p->iPosition = iFileBuffer->Position();

	p->iPts = pts;
	p->iType = type;

	switch(type)
		{
		case FLV_TYPE_VIDEO:
		case FLV_TYPE_AUDIO:
			{
			unsigned char flags;
			flags = iFileBuffer->GetByte();
			size -= 1;

			p->iKeyframe = (( flags & 0xf0 ) == (1 << 4) );

			if((flags & 0xF) == 4)
				{
				iFileBuffer->SkipData(1);
				size -= 1;
				}
			p->iData = (TUint8 *)calloc( 1, size );
			iFileBuffer->GetData(p->iData, size);
			p->iSize = size;
			}
		break;
	
		default:
			iFileBuffer->SkipData(size);
		break;
		}

	if( iFileBuffer->EndOfFile() )
		{
		iFileBuffer->SetPosition( rollBackPosition );
		FreePacket( p );
		return -1;
		}

	return 0;
	}

TBool CFlvDemuxer::FillBuffersL( RPointerArray<void>& aVideoBuffers, RPointerArray<void>& aAudioBuffers, TInt aType )
	{
	if( iFileBuffer->EndOfFile() )
		{
		return ETrue;
		}

	if( aType == EAudio )
		{
		while( aAudioBuffers.Count() < KMaxAudioBuffers )
			{
			 Packet *pkt;
			 if( ReadPacket( &pkt ) < 0 )
			 	{
				return ETrue;
			 	}
			 else
			 	{
			 	if( pkt->iType == FLV_TYPE_AUDIO )
			 		{
					aAudioBuffers.Append( pkt );
			 		}
			 	else if( pkt->iType == FLV_TYPE_VIDEO )
			 		{
					aVideoBuffers.Append( pkt );
			 		}
			 	else
			 		{
			 		FreePacket( pkt );
			 		}
				}
			}
		}
	else if( aType == EVideo )
		{
		while( aVideoBuffers.Count() < KMaxVideoBuffers )
			{
			 Packet *pkt;
			 if( ReadPacket( &pkt ) < 0 )
			 	{
				return ETrue;
			 	}
			 else
			 	{
			 	if( pkt->iType == FLV_TYPE_AUDIO )
			 		{
					aAudioBuffers.Append( pkt );
			 		}
			 	else if( pkt->iType == FLV_TYPE_VIDEO )
			 		{
					aVideoBuffers.Append( pkt );
					}
			 	else
			 		{
			 		FreePacket( pkt );
			 		}
				}
			}
		}
	return EFalse;
	}

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

#define USE_NOKIA_MP3_DECODER
#include <e32svr.h>
#include <e32svr.h>

#include "emTubePlayerAudioDevSound.h"
#include "emTubePlayerDemuxer.h"

#include "avlibaudio.h"

CAudioDecoder* CAudioDecoder::NewL(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver *aObserver)
	{
	 CAudioDecoder* self = CAudioDecoder::NewLC( aDemuxer, aTimer, aObserver );
	 CleanupStack::Pop( self );
	 return self;
	}

CAudioDecoder* CAudioDecoder::NewLC(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver *aObserver)
	{
	 CAudioDecoder* self = new ( ELeave ) CAudioDecoder();
	 CleanupStack::PushL( self );
	 self->ConstructL( aDemuxer, aTimer, aObserver );
	 return self;
	}

void CAudioDecoder::ConstructL(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver *aObserver)
	{
	iShared.iDemuxer = aDemuxer;
	iShared.iTimer = aTimer;
	iShared.iObserver = aObserver;
	iShared.iVolume = 50;

	User::LeaveIfError( iShared.iAliveMutex.CreateLocal( 1 ) );
	User::LeaveIfError( iShared.iMutex.CreateLocal() );
	User::LeaveIfError( iThread.Create( _L("AudioDecoder"),
			CAudioDecoderThread::ThreadFunction,
			0x5000,
			NULL,
			&iShared));
	
	iThread.SetPriority( EPriorityRealTime );
	iThread.Resume();
	iPaused = ETrue;
	}


CAudioDecoder::CAudioDecoder()
	{
	}


CAudioDecoder::~CAudioDecoder()
	{
	SendCmd( EAudioCmdDestroy );
	iShared.iAliveMutex.Wait();
	iShared.iAliveMutex.Close();
	iShared.iMutex.Close();
	iThread.Close();
	}

void CAudioDecoder::Initialize()
	{
	SendCmd( EAudioCmdInitialize );
	}

void CAudioDecoder::Start()
	{
	SendCmd( EAudioCmdStart );
	}

void CAudioDecoder::Stop()
	{
	SendCmd( EAudioCmdStop );
	}

void CAudioDecoder::Pause()
	{
	SendCmd( EAudioCmdPause );
	}

void CAudioDecoder::SetVolume( TInt aVolume )
	{
	iShared.iMutex.Wait();
	iShared.iVolume = aVolume;
	iShared.iMutex.Signal();	
	}

TInt CAudioDecoder::Volume()
	{
	return iShared.iVolume;
	}

void CAudioDecoder::SendCmd( TAudioDecoderCmd aCmd )
	{
	iShared.iMutex.Wait();
	iShared.iCmd = aCmd;
	iShared.iMutex.Signal();
	iShared.iExc = EExcUserInterrupt;
	TBool done = EFalse;
	while( !done )
		{
		TRequestStatus* status = iShared.iStatusPtr;
		if( status )
			{
			if(status->Int() == KRequestPending )
				{
				done = ETrue;
				iThread.RequestComplete(status, KErrNone);
				}
			else
				{
				User::After( 100000 );
				}
			}
		else
			{
			done = ETrue;
			}
		}
	}

TInt CAudioDecoderThread::ThreadFunction( TAny* aData )
	{
	TAudioDecoderShared& shared = *((TAudioDecoderShared*)aData);

	// tell client we're alive
	// signaled off when destroying this thread
	shared.iAliveMutex.Wait();
	CAudioDecoderThread* mixerThread = NULL;
	
	TRAPD(error, mixerThread = CAudioDecoderThread::CreateL( aData ));
	if(error!=KErrNone)
	 {
	 return error;
	 }
	
	if( mixerThread == NULL )
	 {
	 return KErrGeneral;
	 }

	shared.iStatusPtr = &(mixerThread->iActive->iStatus);

	shared.iObserver->MPOAudioDecoderCreated();

	// if we're still here, activescheduler has been constructed
	// start wait loop which runs until it's time to end the thread
	CActiveScheduler::Start();
	delete mixerThread;

	// tell owning thread it's safe to exit
	shared.iAliveMutex.Signal();

	return KErrNone;
	}


CAudioDecoderThread* CAudioDecoderThread::CreateL( TAny* aData )
	{
	CAudioDecoderThread* self = new( ELeave )CAudioDecoderThread( aData );
	if( self == NULL ) return self;
	if( self->Construct() != KErrNone )
	 {
	 delete self;
	 return NULL;
	 }

	return self;
	}


TInt CAudioDecoderThread::Construct()
	{
	// create cleanup stack
	iCleanupStack = CTrapCleanup::New();
	if( iCleanupStack == NULL )
	 {
	 return KErrNoMemory;
	 }

	TInt err = KErrNone;
	TRAP( err, ConstructL() );
	return err;
	}


void CAudioDecoderThread::ConstructL()
	{
	// create active scheduler
	iActiveScheduler = new( ELeave )CActiveScheduler;
	CActiveScheduler::Install( iActiveScheduler );

	iTimeOutTimer = CEmTubeTimeOutTimer::NewL( *this );

	iActive = new(ELeave) MyActive(this);
	iActive->Request();

	AV_AudioRegister();
	}


CAudioDecoderThread::CAudioDecoderThread( TAny* aData )
	: iShared( *((TAudioDecoderShared*)aData))
	{
	}

CAudioDecoderThread::~CAudioDecoderThread()
	{
	StopMixer();
	delete iTimeOutTimer;

	delete iActiveScheduler;
	delete iCleanupStack;
	if(iActive)
	 {
	 iActive->Cancel();	
	 }
	delete iActive;
	}


void CAudioDecoderThread::HandleExceptionL()
	{
	// handle exceptions
	
	// Note: After adding support for 3rd Ed, there is no longer actual
	// thread exception handling - instead, inter-thread communication is done 
	// via active object/scheduler. This function is now called from MyActive.
	
	switch( iShared.iExc )
	 {
	 case EExcUserInterrupt:		 // Command from client
		{
		switch( iShared.iCmd )
			{

			case EAudioCmdInitialize:
			 {
			 InitializeL();
			 break;
			 }

			case EAudioCmdStart:
			 {
			 StartMixerL();
			 break;
			 }

			case EAudioCmdStop:
			 {
			 StopMixer();
			 break;
			 }

			case EAudioCmdPause:
			{
			PauseMixer();
			break;
			}

			case EAudioCmdDestroy:
			 {
			 CActiveScheduler::Stop();	// Exit
			 break;
			 }
			}
		break;
		}
	 default:
		{
		// if unknown exception, just exit this thread
		CActiveScheduler::Stop();		 // Exit
		break;
		}
	 }
	}

void CAudioDecoderThread::TimerExpired( TInt /*aMode*/ )
	{
	FillBufferL();
	}

TInt CAudioDecoderThread::RealVolume( TInt aVolume )
	{
 	TInt volume = ( aVolume * iVolumeScale ) >> 10;
 	if( volume > iShared.iMaxVolume )
 		volume = iShared.iMaxVolume;
 	else if ( volume < 0 )
 		volume = 0;
 	
 	return volume;
 	}

void CAudioDecoderThread::InitializeL()
	{
	TVideoProperties& prop = iShared.iDemuxer->VideoProperties();

	if( !iHandle )
		{
		iHandle = AUDIO_Init( prop.iSampleRate, prop.iChannels, prop.iBitsPerSample, prop.iAudioCodecId);
		}

	if( iHandle )
		{
		iMMFDevSound = CMMFDevSound::NewL();

		RArray<TFourCC> supported;
		TMMFPrioritySettings prioritySettings;
		prioritySettings.iPref = EMdaPriorityPreferenceTime;
		prioritySettings.iPriority = EMdaPriorityMax;
		prioritySettings.iState = EMMFStatePlaying;

		iMMFDevSound->GetSupportedInputDataTypesL( supported, prioritySettings );

		TUint32 fourCC = KMMFFourCCCodePCM16;
		iAudioDecodingRoutine = &CAudioDecoderThread::DecodeAudioSW;

#ifdef USE_NOKIA_MP3_DECODER
		if( supported.Find( KMMFFourCCCodeMP3 ) != KErrNotFound )
			{
			fourCC = KMMFFourCCCodeMP3;
			iAudioDecodingRoutine = &CAudioDecoderThread::DecodeAudioHW;
			}
		else
			{
			fourCC = KMMFFourCCCodePCM16;
			iAudioDecodingRoutine = &CAudioDecoderThread::DecodeAudioSW;
			}
#endif
		supported.Close();

		TInt initError;
		TRAP(initError,iMMFDevSound->InitializeL(*this, fourCC, EMMFStatePlaying));
		if( initError != KErrNone )
			iShared.iObserver->MPOAudioError( KErrNotSupported );
		}
	else
		{
		iShared.iObserver->MPOAudioError( KErrNotSupported );
		}
	}

void CAudioDecoderThread::StartMixerL()
	{
	iBuffer = NULL;
	iFirstBuffer = ETrue;

	iQueueSize = 4096;
	iDecoded = 0;
	iBufPtr = &iPcmout1;
	iBufPtr->Zero();

	iCurrentSample = 0;
	iMMFDevSound->PlayInitL();
	}

void CAudioDecoderThread::PauseMixer()
	{

	if( iTimeOutTimer )
		iTimeOutTimer->Cancel();
	
	if( iMMFDevSound )
		{
		iMMFDevSound->Stop();
		}
	}

void CAudioDecoderThread::StopMixer()
	{
	if( iTimeOutTimer )
		iTimeOutTimer->Cancel();
	
	if( iMMFDevSound )
		{
		iMMFDevSound->Stop();
		delete iMMFDevSound;
		}
	iMMFDevSound = NULL;

	if( iHandle )
		{
		AUDIO_Close( iHandle );
		iHandle = NULL;
		}
	}

void CAudioDecoderThread::DecodeAudioSW( TAny* aData, TInt aSize )
	{
	TUint32 t = CVideoTimer::Time();
	TUint8* ptr = (TUint8* )iBufPtr->Ptr();
	ptr += iDecoded;
	SAMPLE* s = (SAMPLE *)AUDIO_Decode( iHandle, aData, (void*)ptr, aSize);
	iShared.iADTicks += (CVideoTimer::Time() - t);
	iDecoded += s->outsize;
	iBufPtr->SetLength( iDecoded );
	}

void CAudioDecoderThread::DecodeAudioHW( TAny* aData, TInt aSize )
	{
	TUint32 t = CVideoTimer::Time();
	TUint8* ptr = (TUint8* )iBufPtr->Ptr();
	ptr += iDecoded;
	Mem::Copy( ptr, aData, aSize );
	iShared.iADTicks += (CVideoTimer::Time() - t);
	iDecoded += aSize;
	iBufPtr->SetLength( iDecoded );
	}

TInt32 CAudioDecoderThread::DecodeAudio()
	{
	TBool eof = EFalse;
	TInt32 pts = -1;
	while( iDecoded < iQueueSize )
		{
		Packet* pkt = (Packet*)iShared.iDemuxer->ReadAudioPacket( eof );
		if( pkt )
			{
			if( pts == -1 )
				pts = pkt->iPts;

			(this->*iAudioDecodingRoutine)( pkt->iData, pkt->iSize );
			iShared.iDemuxer->DeletePacket( pkt );
			}
		else
			{
			break;
			}
		}
	return pts;
	}

TUint CAudioDecoderThread::SamplesToTime( TUint aSamples, TUint /*aPts*/ )
	{
	TVideoProperties& prop = iShared.iDemuxer->VideoProperties();

	TUint32 val = 0;
	if( iSamplesCount < aSamples )
		{
		val = aSamples - iSamplesCount;
		iCurrentSample += val;
		}

	iSamplesCount = aSamples;

    TUint64 ret = ((TUint64)iCurrentSample * 1000) / prop.iSampleRate;
	return (TUint32)ret;
	}

TInt CAudioDecoderThread::FillBufferL()
	{
	iShared.iMutex.Wait();
	TInt volume = iShared.iVolume;
	iShared.iMutex.Signal();	

	TInt pts = -1;

	while( 1 )
		{

		if( iDecoded )
			{
			if( iDecoded <= iRequestedSize )
				{
				iBuffer->Data().Append( *iBufPtr );
				iRequestedSize -= iDecoded;
				iDecoded = 0;
				iBufPtr = &iPcmout1;
				iBufPtr->Zero();
				}
			else
				{
				iBuffer->Data().Append( iBufPtr->Left( iRequestedSize ) );
				iBufPtr->Delete( 0, iRequestedSize );
				iDecoded -= iRequestedSize;
				iRequestedSize = 0;
				}
			}
		else
			{
			iBufPtr = &iPcmout1;
			iBufPtr->Zero();
			}

		if( iRequestedSize )
			{
			TBool eof = EFalse;
			Packet* pkt = (Packet*)iShared.iDemuxer->ReadAudioPacket( eof );

			if( pkt )
				{
				if( pts == -1 )
					pts = pkt->iPts;
				
				(this->*iAudioDecodingRoutine)( pkt->iData, pkt->iSize );
				iShared.iDemuxer->DeletePacket( pkt );

				}
			else if( !eof )
				{
				if( iBuffer->Data().Length() )
					{
					iMMFDevSound->SetVolume( RealVolume( volume ) );
					iShared.iTimer->SetAudioPosition( SamplesToTime( iMMFDevSound->SamplesPlayed(), pts) );
					iMMFDevSound->PlayData();
					iBuffer = NULL;
					break;
					}
				else
					{
					if( !iTimeOutTimer->IsActive() )
						{
						iTimeOutTimer->After( 2000, 0 );
						return pts;
						}
					}
				}
			else
				{
				iMMFDevSound->SetVolume( RealVolume( volume ) );
				iShared.iTimer->SetAudioPosition( SamplesToTime( iMMFDevSound->SamplesPlayed(), pts) );
			   	iBuffer->SetLastBuffer( ETrue );
				iMMFDevSound->PlayData();
				iBuffer = NULL;
				return pts;
				}
			}
		else
			{
			iMMFDevSound->SetVolume( RealVolume( volume ) );
			iShared.iTimer->SetAudioPosition( SamplesToTime( iMMFDevSound->SamplesPlayed(), pts) );
			iMMFDevSound->PlayData();
			iBuffer = NULL;
			break;
			}
		}
	DecodeAudio(); //fill up internal buffer!
	return pts;
	}

void CAudioDecoderThread::InitializeComplete(TInt aError)
{
	if(aError == KErrNone)
        {

		TVideoProperties& prop = iShared.iDemuxer->VideoProperties();

        TMMFCapabilities conf = iMMFDevSound->Config();

		conf.iEncoding = EMMFSoundEncoding16BitPCM;
//		conf.iBufferSize = 1024;

		switch( prop.iChannels )
			{
				case 1:
					conf.iChannels = EMMFMono;
				break;
				
				case 2:
					conf.iChannels = EMMFStereo;
				break;
			}

		switch( prop.iSampleRate )
			{
				case 8000:
					conf.iRate = EMMFSampleRate8000Hz;
				break;
				case 11025:
					conf.iRate = EMMFSampleRate11025Hz;
				break;
				case 12000:
					conf.iRate = EMMFSampleRate12000Hz;
				break;
				case 16000:
					conf.iRate = EMMFSampleRate16000Hz;
				break;
				case 22050:
					conf.iRate = EMMFSampleRate22050Hz;
				break;
				case 24000:
					conf.iRate = EMMFSampleRate24000Hz;
				break;
				case 32000:
					conf.iRate = EMMFSampleRate32000Hz;
				break;
				case 44100:
					conf.iRate = EMMFSampleRate44100Hz;
				break;
				case 48000:
					conf.iRate = EMMFSampleRate48000Hz;
				break;
				case 96000:
					conf.iRate = EMMFSampleRate96000Hz;
				break;
				case 64000:
					conf.iRate = EMMFSampleRate64000Hz;
				break;
			}
        TRAPD( err, iMMFDevSound->SetConfigL(conf) );
        if( err != KErrNone )
        	{
			iShared.iObserver->MPOAudioDecoderInitialized( KErrNotSupported );
			return;
	        }

        // priority and preference settings    
        iPrioritySettings.iPref = EMdaPriorityPreferenceQuality;
        iPrioritySettings.iPriority = EMdaPriorityNormal;
        iMMFDevSound->SetPrioritySettings(iPrioritySettings);

		if( aError == KErrNone )
			{
			iShared.iMaxVolume = iMMFDevSound->MaxVolume();
			iVolumeScale = ( ( iShared.iMaxVolume << 10 ) / 100 ) + 1;
			iShared.iADTicks = 0;
			}
        }

	iShared.iObserver->MPOAudioDecoderInitialized( aError );
}

void CAudioDecoderThread::BufferToBeFilled(CMMFBuffer* aBuffer)
{
	if( iBuffer == NULL )
		{
		iBuffer = static_cast<CMMFDataBuffer*>(aBuffer);
	   	iBuffer->SetLastBuffer( EFalse );
	   	iBuffer->Data().Zero();
		iQueueSize = iRequestedSize = iBuffer->RequestSize();
		}

	TInt32 pts = FillBufferL();
	if( iFirstBuffer )
		{
		iShared.iTimer->SetInitialAudioPts( pts );
		iShared.iObserver->MPOAudioPlaybackStarted();
		}
	iFirstBuffer = EFalse;
}

void CAudioDecoderThread::PlayError(TInt aError) 
{
	
 	switch(aError)
        {
        case KErrNone:
        	break;
        case KErrAbort:
			break;
        case KErrCancel:
        	break;
        case KErrUnderflow:        
            {
			iMMFDevSound->PlayInitL();
            break;
            }
        case KErrDied:                                   
        	break;
        case KErrAccessDenied:
        	break;	        	        	
        default: 
            break;  
       }
}

void CAudioDecoderThread::ToneFinished(TInt /*aError*/) 
{
}

void CAudioDecoderThread::BufferToBeEmptied(CMMFBuffer* /*aBuffer*/) 
{
}

void CAudioDecoderThread::RecordError(TInt /*aError*/) 
{
}

void CAudioDecoderThread::ConvertError(TInt /*aError*/)
{
}

void CAudioDecoderThread::DeviceMessage (TUid /*aMessageType*/,const TDesC8& /*aMsg*/) 
{
}

void CAudioDecoderThread::SendEventToClient(const TMMFEvent& /*aEvent*/) 
{
}

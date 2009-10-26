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

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <e32std.h>
#include <mdaaudiooutputstream.h>
#include <mda\common\audio.h>
#include <mmf\server\SoundDevice.h>

#include "emtubetimeouttimer.h"
#include "emTubePlayerTimer.h"
#include "threadhandler.h"
#include "emTubePlayerDemuxer.h"

enum TAudioDecoderCmd
	{
	EAudioCmdStart = 0,
	EAudioCmdInitialize,
	EAudioCmdPause,
	EAudioCmdStop,
	EAudioCmdDestroy
	};

class TAudioDecoderShared
	{
  public:
    RSemaphore  iAliveMutex;
    RMutex  iMutex;

    TBool iPaused;

    TExcType        iExc;
    TRequestStatus* iStatusPtr;
    
    CDemuxer* iDemuxer;
    
    CVideoTimer* iTimer;
    
    TInt  iVolume;
    TInt  iMaxVolume;
    
    MPlayerObserver* iObserver;

    TUint32 iADTicks;
    
    TAudioDecoderCmd iCmd;
  };

#define AUDIO_BUFFER_SIZE (44100*2*2)

class CAudioDecoderThread : public CBase,
							public MDevSoundObserver,
							public MThreadHandler,
							public MTimeOutObserver
  {
  public:
    static TInt ThreadFunction( TAny* aData );
    ~CAudioDecoderThread();
    void HandleExceptionL();
    
  private:

    CAudioDecoderThread( TAny* aData );
    static CAudioDecoderThread* CreateL( TAny* aData );
    TInt Construct();
    void ConstructL();
	void InitializeL();
    void StartMixerL();
    void StopMixer();
    void PauseMixer();

  public: //MTimeOuttimerExpired
  	void TimerExpired( TInt aMode );

  private: // MDevSoundObserver
	virtual void InitializeComplete(TInt aError);
	virtual void BufferToBeFilled(CMMFBuffer* aBuffer);
	virtual void PlayError(TInt aError); 
	virtual void ToneFinished(TInt /*aError*/); 
	virtual void BufferToBeEmptied(CMMFBuffer* /*aBuffer*/); 
	virtual void RecordError(TInt /*aError*/); 
	virtual void ConvertError(TInt /*aError*/) ;
	virtual void DeviceMessage (TUid /*aMessageType*/,const TDesC8& /*aMsg*/);
	virtual void SendEventToClient(const TMMFEvent& /*aEvent*/); 

  private:
	TUint SamplesToTime( TUint aSamples, TUint aPts );
    TInt FillBufferL();
	TInt RealVolume( TInt aVolume );

	TInt32 DecodeAudio();
	void DecodeAudioSW( TAny* aData, TInt aSize );
	void DecodeAudioHW( TAny* aData, TInt aSize );
    
  private:

    CTrapCleanup* iCleanupStack;
    CActiveScheduler* iActiveScheduler;

	TBuf8<AUDIO_BUFFER_SIZE> iPcmout1;
	TAny* iHandle;
	TAudioDecoderShared& iShared;  // reference to shared data with client

	CEmTubeTimeOutTimer *iTimeOutTimer;

	MyActive* iActive;
	TUint32 iVolumeScale;

	TMMFPrioritySettings  iPrioritySettings;
	CMMFDevSound* iMMFDevSound;
	CMMFDataBuffer* iBuffer;
	TBool iFirstBuffer;
	TUint32 iCurrentSample;
	TUint32 iSamplesCount;
	TInt32 iDecoded;
	TInt32 iRequestedSize;
	TDes8 *iBufPtr;

    TInt32 iQueueSize;

	void (CAudioDecoderThread::*iAudioDecodingRoutine)( TAny* aData, TInt aSize );

	};
  
class CAudioDecoder : public CBase
	{
	public:
	static CAudioDecoder* NewL(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver* aObserver);
	static CAudioDecoder* NewLC(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver* aObserver);
    ~CAudioDecoder();

	private:
		CAudioDecoder();
		void ConstructL(CDemuxer *aDemuxer, CVideoTimer* aTimer, MPlayerObserver* aObserver);

	public:
		void Initialize();
		void Start();
		void Stop();
		void Pause();

		void SetVolume( TInt aVolume );
		TInt Volume();

		TUint32 TimeConsumed() { return iShared.iADTicks; }

	private:
		void SendCmd( TAudioDecoderCmd aCmd );

	private:
		TAudioDecoderShared  iShared;        // shared data with mixer thread
		RThread       iThread;   // handle to mixer thread
		TBool         iPaused;        // paused flag
	};
  
#endif //AUDIO_DECODER_H

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

#ifndef DEMUXER_H
#define DEMUXER_H

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <s32file.h>

#include "threadhandler.h"

class MPlayerObserver
{
	public:
		virtual void MPOFileOpenedL( TInt aError ) = 0;
		virtual void MPOFileClosedL() = 0;
		virtual void MPODemuxerCreated() = 0;
		virtual void MPOAudioDecoderCreated() = 0;
		virtual void MPOAudioDecoderInitialized( TInt aError ) = 0;
		virtual void MPOAudioPlaybackStarted() = 0;
		virtual void MPOAudioError( TInt aError ) = 0;
		virtual void MPOSeekDoneL() = 0;
};

enum TDemuxerCmd
	{
	EDemuxerCmdOpenFile = 0,
	EDemuxerCmdFillAudio,
	EDemuxerCmdFillVideo,
	EDemuxerCmdCloseFile,
	EDemuxerCmdSeek,
	EDemuxerCmdSetEof,
	EDemuxerCmdDestroy
	};

class TVideoProperties
	{
	public:
		TBool iHasVideo;
		TBool iHasAudio;
		TUint32 iDuration;

		TInt iWidth;
		TInt iHeight;
		float iFps;
		TUint32 iFrametime;
		TInt iVideoCodecId;
		TUint8 iVideoExtraData;
		TInt iVideoExtraDataSize;

		TUint iSampleRate;
		TUint iChannels;
		TUint iBitsPerSample;
		TUint iAudioCodecId;

		TInt32 iStartOfData;
	};

class TIndexEntry
	{
	public:
		TUint32 iPts;
		TUint32 iPosition;
	};

class TDemuxerShared
	{
	public:
		RSemaphore iAliveMutex;
		RMutex iMutex;

		TFileName iFilename;
		TInt iError;

		RPointerArray<void> iVideoBuffers;
		RPointerArray<void> iAudioBuffers;

		RArray<TIndexEntry> iIndices;

		TExcType        iExc;
		TRequestStatus* iStatusPtr;

		MPlayerObserver* iObserver;

		TBool iEof;

		TDemuxerCmd iCmd;
		
		TVideoProperties iVideoProperties;
		
		TBool iEnableVideo;
		TBool iEnableAudio;
		
		TInt32 iSeekPosition;
		
		RFile iFile;
	};

class CFileBuffer;
class CFlvDemuxer;

typedef struct Packet
	{
	TUint8 *iData;
	TUint32 iPts;
	TUint32 iSize;
	TUint32 iType;
	TBool iKeyframe;
	TInt iPosition;
	} PACKET;

class CDemuxerThread : CBase, public MThreadHandler
	{
	public:
		static TInt ThreadFunction( TAny* aData );
		~CDemuxerThread();
		
	    void HandleExceptionL();

	private:
		void CloseFileL();

		CDemuxerThread( TAny* aData );
		static CDemuxerThread* CreateL( TAny* aData );
		TInt Construct();
		void ConstructL();

	private:
	private:

		CTrapCleanup*			iCleanupStack;
		CActiveScheduler*		iActiveScheduler;
		TDemuxerShared&			iShared;
		MyActive* iActive;
		CFlvDemuxer *iFlv;
	};

class CDemuxer : public CBase
	{
	public:
		enum TSeekSpeed
		{
		ESeekSpeedNormal = 0,
		ESeekSpeedMedium = 3,
		ESeekSpeedHigh = 15
		};
	
		enum TSearchMode
		{
		EBackward = 0,
		EForward,
		EExact
		};
	
	public:
		static CDemuxer* NewL(MPlayerObserver *aObserver);
		static CDemuxer* NewLC(MPlayerObserver *aObserver);
		~CDemuxer();

	private:
		CDemuxer();
		void ConstructL(MPlayerObserver *aObserver);

	public:
		void OpenFileL( const TDesC& aFile );
		void OpenFileL( RFile& aFile );
		void CloseFileL();
		TAny* ReadAudioPacket( TBool& aEof );
		TAny* ReadVideoPacket( TBool& aEof );
		
		void SeekL( TInt aPosition );
		void SeekPercentageL( TInt aPercentage );

		void EnableAudio();
		void EnableVideo();
		void DisableAudio();
		void DisableVideo();

		TBool AudioEnabled();
		TBool VideoEnabled();

		TBool HasAudio();
		TBool HasVideo();
		TBool Seekable();

		void SetEof( TBool aEof );

		TBool FindKeyFrameL( TUint32 aPts, TSearchMode aMode, TSeekSpeed aSpeed );

		void DeletePacket( TAny* aPacket );

		TVideoProperties& VideoProperties() { return iShared.iVideoProperties; }
	
	private:
		void SendCmd( TDemuxerCmd aCmd );

	private:
		TDemuxerShared	iShared;
		RThread			iDemuxerThread;
	};

class CFileBuffer : public CBase
	{
	public:
		static CFileBuffer* NewL( RFile& aFile );
		static CFileBuffer* NewLC( RFile& aFile );
		~CFileBuffer();

	public:
		void SkipData(TInt size);
		void GetData(TAny *where, TInt size);
		TInt Position();

		TUint8 GetByte();
		TInt16 GetWord();
		TInt32 GetLong();

		TInt16 GetWordBE();
		TInt32 GetLongBE(); 
		double GetDoubleBE();

		TInt FileSize() { return iFileSize; }

		void Seek( TInt aPosition );

		void SetPosition( TInt aPosition );

		void SetEof( TBool aEof ) { endOfFile = aEof; }

		TBool EndOfFile() { return endOfFile; }

	private:
		CFileBuffer( RFile& aFile );
		void ConstructL();

	private:
		RFile &iFile;

		TBuf8<65536 / 8> FileBuffer;

		const TUint8 *ptr;
		TInt BytesInBuffer;

		TInt pos;

		TInt iFileSize;

		TBool endOfFile;
	};

class CFlvDemuxer : public CBase
	{
	public:
		enum TBufferType
		{
			EAudio = 0,
			EVideo
		};
	
	public:
		static CFlvDemuxer* NewL();
		static CFlvDemuxer* NewLC();
		~CFlvDemuxer();

	public:
		TInt OpenFileL( TDesC& aFileName, RFile& aFile, RArray<TIndexEntry>& aIndices, TVideoProperties& aProperties );
		void CloseFileL();
		TBool FillBuffersL( RPointerArray<void>& aAudio, RPointerArray<void>& aVideo, TInt aType );

		TBool IsFlv( TUint8* aBuf );
		TInt ProcessMetaData( TUint8 * aBuf, TInt aSize, TInt8 *aKey, TInt aKeylen, RArray<TIndexEntry>& aIndices );

		void FreePacket( Packet *pkt );
		TInt ReadPacket( Packet **pkt );

		void SetEof(TBool aEof );
		void Seek( TInt aPos );

		TInt FileSize();

	private:
		CFlvDemuxer();
		void ConstructL();

		TReal64 GetDoubleValFromMetadata( const TUint8* aPtr, TInt& aOffset );
		TUint32 GetIntValFromMetadata( const TUint8* aPtr, TInt& aOffset );
		TUint32 GetDurationFromMetadata( const TDesC8& aMetadata );
		void GetIndicesFromMetadata( const TDesC8& aMetadata, const TDesC8& aType, RArray<TIndexEntry>& aIndices );

	private:
		CFileBuffer *iFileBuffer;
		RFile iFile;
		RFs iSession;
		TFileName iFileName;
	};

#endif //DEMUXER_H

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

#include <eikapp.h> 
#include <eikappui.h> 

#include "emTubeThumbnail.h"
#include "emTubeVideoEntry.h"
#include "emTubePlayerDemuxer.h"
#include "emTubeYUV2RGB.h"
#include "avlibvideo.h"

CThumbnailRetriever::CThumbnailRetriever()
	: CActive(CActive::EPriorityStandard),
	  iEncoder(NULL)
	{
	}

void CThumbnailRetriever::ConstructL()
	{
	iCC = CEmTubeYUV2RGB::NewL();
	iFs.Connect();
	CActiveScheduler::Add(this);
	}

CThumbnailRetriever* CThumbnailRetriever::NewL()
	{
	CThumbnailRetriever* self = NewLC();
	CleanupStack::Pop();
	return self;
	}

CThumbnailRetriever* CThumbnailRetriever::NewLC()
	{
	CThumbnailRetriever* self = new (ELeave) CThumbnailRetriever();
	CleanupStack::PushL( self );
	self->ConstructL();

	return self;
	}

CThumbnailRetriever::~CThumbnailRetriever()
	{
	delete iCC;
	delete iBitmap;
	delete iEncoder;
	iFs.Close();
	}

void CThumbnailRetriever::RetrieveThumbnailL( CVideoEntry* aEntry )
	{
	delete iBitmap;
	iBitmap = NULL;
	
	delete iEncoder;
	iEncoder = NULL;

	CFlvDemuxer* flv = CFlvDemuxer::NewLC();

	TVideoProperties prop;
	TAny* handle;
	RArray<TIndexEntry> indices;
	RPointerArray<void> audioB;
	RPointerArray<void> videoB;

	TInt ret = KErrNone;
	RFile file;
	TRAPD( err, ret = flv->OpenFileL( aEntry->SavedFileName(), file, indices, prop ) );

	if( ret == KErrNone && err == KErrNone )
		{
		handle = VIDEO_Init( prop.iWidth, prop.iHeight, &prop.iVideoExtraData, prop.iVideoExtraDataSize, prop.iVideoCodecId );
		if( handle )
			{
			aEntry->SetDuration( prop.iDuration );

			if( indices.Count() )
				{
				TInt c = indices.Count() / 2 ;
				TInt position = indices[c].iPosition - 4;
				if( position >= flv->FileSize() )
					{
					while( position >= flv->FileSize() )
						{
						c--;
						position = indices[c].iPosition - 4;
						if( c == 0 )
							break;
						}
					}

				flv->Seek( position);
				}
			flv->FillBuffersL( videoB, audioB, CFlvDemuxer::EVideo );
			aEntry->SetVideoFileSize( flv->FileSize() );

			if( videoB.Count() )
				{
				Packet* pkt = (Packet*)videoB[0];
				if( pkt )
					{
					IMAGE *img = (IMAGE *)VIDEO_Decode( handle, pkt->iData, pkt->iSize, 0);
					if( img )
						{
						iBitmap = new (ELeave) CFbsBitmap;
						TInt w = prop.iWidth;
						TInt h = prop.iHeight;

						w = (w + 1) & (~1);
						if( h & 1 )
							h--;

						TSize size( w, h );
						iBitmap->Create( size, EColor16MU );
						TInt scanlineLength = iBitmap->ScanLineLength(iBitmap->SizeInPixels().iWidth, EColor16MU ) / 4;

						iCC->Init( EColor16MU, CEmTubeYUV2RGB::ERotationNone, CEmTubeYUV2RGB::EScaleNone );

						iBitmap->LockHeap( ETrue );
						TInt height = img->height;
						if( height & 1 )
							height--;

						iCC->Convert( img->output, img->width, height, img->stride, img->uv_stride,
									iBitmap->SizeInPixels().iWidth, iBitmap->SizeInPixels().iHeight, scanlineLength, iBitmap->DataAddress() );
						iBitmap->UnlockHeap( ETrue );

						iEncoder = CImageEncoder::FileNewL( iFs, aEntry->ThumbnailFile(), _L8("image/jpeg"), CImageEncoder::EOptionNone);
						iEncoder->Convert( &iStatus, *iBitmap );
						SetActive();
						iWait.Start();
						}
					}
				}

			while( audioB.Count() )
				{
				Packet* pkt = (Packet*)audioB[0];
				audioB.Remove( 0 );
				flv->FreePacket( pkt );
				}

			while( videoB.Count() )
				{
				Packet* pkt = (Packet*)videoB[0];
				videoB.Remove( 0 );
				flv->FreePacket( pkt );
				}

			videoB.Close();
			audioB.Close();
			indices.Close();
			VIDEO_Close( handle );
			}
		}

	flv->CloseFileL();
	CleanupStack::PopAndDestroy( flv );
	}

void CThumbnailRetriever::Close()
	{
	delete iEncoder;
	iEncoder = NULL;
	}

void CThumbnailRetriever::RunL()
	{
	if ( iWait.IsStarted() )
		{
		iWait.AsyncStop();
		}
	}
	 
void CThumbnailRetriever::DoCancel()
	{
	if ( iEncoder ) 
		{
		iEncoder->Cancel();
		}
	}

void CThumbnailRetriever::StopSavingImage()
	{
	DoCancel();
	}

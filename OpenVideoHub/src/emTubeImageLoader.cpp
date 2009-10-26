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
#include "emTubeImageLoader.h"
#include "emTubeVideoEntry.h"

CImageLoader::CImageLoader( MImageLoaderCallback& aCallback )
	: CActive(CActive::EPriorityStandard),
	  iDecoder(NULL),
	  iCallback(aCallback),
	  iDesc(NULL, 0, 0)
	{
	}

void CImageLoader::ConstructL()
	{
	iFs.Connect();
	CActiveScheduler::Add(this);
	}

CImageLoader* CImageLoader::NewL( MImageLoaderCallback& aCallback )
	{
	CImageLoader* self = NewLC( aCallback );
	CleanupStack::Pop();
	return self;
	}

CImageLoader* CImageLoader::NewLC( MImageLoaderCallback& aCallback )
	{
	CImageLoader* self = new (ELeave) CImageLoader( aCallback );
	CleanupStack::PushL( self );
	self->ConstructL();

	return self;
	}

CImageLoader::~CImageLoader()
	{
	delete iDecoder;
	delete iFileData;
	iFs.Close();
	}

void CImageLoader::LoadFileL( CVideoEntry *aEntry )
	{
	if( IsActive() )
		{
		iCallback.ImageLoadedL( KErrInUse );
		return;
		}
	
	if( aEntry->ImageLoaded() )
		{
		iCallback.ImageLoadedL( KErrAlreadyExists );
		return;
		}
		
	iBitmap = aEntry->Bitmap();

	delete iDecoder;
	iDecoder = NULL;
	
	delete iFileData;
	iFileData = NULL;

	RFile file;
	TInt err = file.Open( iFs, aEntry->ThumbnailFile(), EFileRead | EFileStream | EFileShareAny );
	if( err == KErrNone )
		{
		TBuf8<4> hdr;
		file.Read( hdr );
		if( !hdr.Compare( _L8("emtb") ) )
			{
			TInt pos = 4;
			file.Seek( ESeekCurrent, pos );

			TInt size;
			file.Size( size );
			iFileData = HBufC8::NewL( size - 4 );
			iDesc.Set( iFileData->Des() );
			file.Read( iDesc );
			file.Close();
			TRAP( err, iDecoder = CImageDecoder::DataNewL( iFs, iDesc ) );
			}
		else
			{
			file.Close();
			TRAP( err, iDecoder = CImageDecoder::FileNewL( iFs, aEntry->ThumbnailFile() ) );
			}
		}

	if( err != KErrNone )
		{
		iCallback.ImageLoadedL( err );
		return;
		}

	iFrameInfo = iDecoder->FrameInfo( 0 );
	TRect bitmapSize = iFrameInfo.iFrameCoordsInPixels;
	iBitmap->Resize(bitmapSize.Size());

	// Decode as bitmap.
	iDecoder->Convert(&iStatus, *iBitmap, 0 );
	SetActive();

#if 0
	iWait.Start();
//	iCallback.ImageLoadedL( iStatus.Int() );
#endif
	}

void CImageLoader::Close()
	{
	delete iDecoder;
	iDecoder = NULL;
	
	delete iFileData;
	iFileData = NULL;
	}

void CImageLoader::RunL()
	{
#if 0
	if ( iWait.IsStarted() )
		{
		iWait.AsyncStop();
		}
#endif
	iCallback.ImageLoadedL( iStatus.Int() );
	}
	 
void CImageLoader::DoCancel()
	{
	if ( iDecoder ) 
		{
		iDecoder->Cancel();
		}
	}

void CImageLoader::StopLoadingImage()
	{
	Cancel();
	}

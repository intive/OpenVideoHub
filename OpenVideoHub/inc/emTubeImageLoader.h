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

#ifndef EMTUBE_IMAGE_LOADER_H
#define EMTUBE_IMAGE_LOADER_H

#include <fbs.h>
#include <icl/imagedata.h>
#include <imageconversion.h>

class CVideoEntry;

class MImageLoaderCallback
	{
	public:
		virtual void ImageLoadedL(TInt aError) = 0;
	};

class CImageLoader : public CActive
	{
	public:
		static CImageLoader* NewL( MImageLoaderCallback& aCallback );
		static CImageLoader* NewLC( MImageLoaderCallback& aCallback );
		virtual ~CImageLoader();

	public:
		void LoadFileL( CVideoEntry* aEntry );
		void StopLoadingImage();
		void Close();

	private:
		void RunL(); 
		void DoCancel();

	protected:
		CImageLoader( MImageLoaderCallback& aCallback );
		void ConstructL();

	private: // Data
		CImageDecoder* iDecoder;
		MImageLoaderCallback& iCallback;
		CFbsBitmap *iBitmap; 
		RFs iFs;
		TFrameInfo iFrameInfo;
		TPtr8 iDesc;
		HBufC8 *iFileData;

		CActiveSchedulerWait iWait;
	};

#endif //EMTUBE_IMAGE_LOADER_H

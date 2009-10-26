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

#ifndef EMTUBE_IHUMBNAIL_RETRIEVER_H
#define EMTUBE_IHUMBNAIL_RETRIEVER_H

#include <fbs.h>
#include <icl/imagedata.h>
#include <imageconversion.h>

class CVideoEntry;
class CEmTubeYUV2RGB;

class CThumbnailRetriever : public CActive
	{
	public:
		static CThumbnailRetriever* NewL();
		static CThumbnailRetriever* NewLC();
		virtual ~CThumbnailRetriever();

	public:
		void RetrieveThumbnailL( CVideoEntry* aEntry );

		void StopSavingImage();
		void Close();

	private:
		void RunL(); 
		void DoCancel();

	protected:
		CThumbnailRetriever();
		void ConstructL();

	private: // Data
		CImageEncoder* iEncoder;
		RFs iFs;
		
		CFbsBitmap* iBitmap;
		CEmTubeYUV2RGB* iCC;

		CActiveSchedulerWait iWait;

	};

#endif //EMTUBE_IHUMBNAIL_RETRIEVER_H

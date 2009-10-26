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

#ifndef EMTUBE_VIDEO_ENTRY_H
#define EMTUBE_VIDEO_ENTRY_H

#include <e32base.h>
#include <s32file.h>
#include <fbs.h>

#include <PluginInterface.h>

_LIT( KVideoDirectoryC, "C:\\Data\\" );

#ifdef __WINS__
_LIT( KVideoDirectoryE, "D:\\" );
#else
_LIT( KVideoDirectoryE, "E:\\" );
#endif

_LIT( KVideoTmpName, "tmpvideo.flv" );

class CVideoEntry: public CBase, public MVideoEntry
	{
	public:
		static CVideoEntry* NewL();
		static CVideoEntry* NewLC();

		static CVideoEntry* NewL( CVideoEntry* aEntry );
		static CVideoEntry* NewLC( CVideoEntry* aEntry );

		~CVideoEntry();

	protected:
		CVideoEntry();

		void ConstructL();
		void ConstructL( CVideoEntry* aEntry );

	public: //from MVideoEntry
		void SetVideoIdL( const TDesC& aId );
		void SetTitleL( const TDesC& aTitle );
		void SetUrlL( const TDesC& aUrl );
		void SetThumbnailUrlL( const TDesC& aUrl );
		void SetDuration( TInt aSeconds );
		void SetAverageRating( TReal32 aAverage );
		void SetViewCount( TInt aCount );
		void SetAuthorNameL( const TDesC& aName );
		void SetRelatedUrlL( const TDesC& aUrl );
		void SetAuthorVideosUrlL( const TDesC& aUrl );

	public:
		void SetVideoUrlL( const TDesC& aUrl );
		void SetVideoFileSize( TInt aSize ) { iVideoFileSize = aSize; }
		void SetDownloadFinished( TBool aFinished ) { iDownloadFinished = aFinished; }
		void SetThumbnailFileL( const TDesC& aFilename );
		void SetCategoryL( const TDesC& aCategory );
		void GenerateThumbnailFileL( const TDesC& aCacheDirectory );


		TDesC& MediaTitle() { return *iMediaTitle; }
		TDesC& Url() { return *iUrl; }
		TDesC& ThumbnailUrl() { return *iThumbnailUrl; }
		TDesC& ThumbnailFile() { return *iThumbnailFile; }
		TDesC& Category() { return *iCategory; }
		TInt Duration() { return iDuration; }
		TReal32 AverageRating() { return iAverageRating; }
		TInt ViewCount() { return iViewCount; }
		TDesC& AuthorName() { return *iAuthorName; }
		TDesC& AuthorUrl() { return *iAuthorUrl; }
		TDesC& VideoId() { return *iVideoId; }
		TDesC& RelatedUrl() { return *iRelatedUrl; }
		TDesC& AuthorVideosUrl() { return *iAuthorVideosUrl; }
		TInt VideoFileSize() { return iVideoFileSize; }
		TBool DownloadFinished() { return iDownloadFinished; }

		void SetSavedFileName( const TDesC& aFilename ) { iSavedFileName.Copy( aFilename ); }
		TDesC& SavedFileName() { return iSavedFileName; }

		void SetIconIdx( TInt aIdx ) { iIconIdx = aIdx; }
		TInt IconIdx() { return iIconIdx; }

		CFbsBitmap *Bitmap();
		void SetImageLoaded( TBool aLoaded );
		TBool ImageLoaded();

		CFbsBitmap* ScaledBitmapL( TInt aDstWidth, TInt aDstHeight );
		void ScaleImageL( TInt aDstWidth, TInt aDstHeight );
		CFbsBitmap* ScaledBitmap();

		void ExportL( RFileWriteStream &aStream );
		void ImportL( RFileReadStream &aStream );

		static HBufC* ConvertUtfToUnicodeL(const TDesC8& aUtf7);

		TUint32 PluginUid() { return iPluginUid; }
		void SetPluginUid( TUint32 aUid ) { iPluginUid = aUid; }

	private:
		TBool iImageLoaded;
	
		HBufC* iMediaTitle;
		HBufC* iUrl;
		HBufC* iThumbnailUrl;
		HBufC* iThumbnailFile;
		HBufC* iVideoId;
		HBufC* iAuthorName;
		HBufC* iAuthorUrl;
		HBufC* iRelatedUrl;
		HBufC* iAuthorVideosUrl;
		HBufC* iCategory;
		TInt iThumbnailHeight;
		TInt iThumbnailWidth;
		TInt iDuration;
		TReal32 iAverageRating;
		TInt iViewCount;
		CFbsBitmap* iBitmap;
		CFbsBitmap* iScaledBitmap;
		
		TInt iVideoFileSize;
		TBool iDownloadFinished;
		
		TFileName iSavedFileName;
		
		TInt iIconIdx;
		
		TUint32 iPluginUid;		
	};

#endif //EMTUBE_VIDEO_ENTRY_H

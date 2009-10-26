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

#include <e32base.h>
#include <e32const.h>
#include <S32FILE.H>

#include "emTubeVideoEntry.h"
#include "emTubeUiItemGfx.h"
#include "emTubeCache.h"

CVideoEntry* CVideoEntry::NewL()
	{
	CVideoEntry* self = CVideoEntry::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CVideoEntry* CVideoEntry::NewLC()
	{
	CVideoEntry* self = new (ELeave) CVideoEntry();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;	
	}

CVideoEntry* CVideoEntry::NewL( CVideoEntry* aEntry )
	{
	CVideoEntry* self = CVideoEntry::NewLC( aEntry );
	CleanupStack::Pop(self);
	return self;
	}

CVideoEntry* CVideoEntry::NewLC( CVideoEntry* aEntry )
	{
	CVideoEntry* self = new (ELeave) CVideoEntry();
	CleanupStack::PushL(self);
	self->ConstructL( aEntry );
	return self;	
	}

CVideoEntry::~CVideoEntry()
	{
	delete iBitmap;
	delete iScaledBitmap;
	delete iMediaTitle;
	delete iCategory;
	delete iUrl;
	delete iThumbnailUrl;
	delete iThumbnailFile;
	delete iVideoId;
	delete iAuthorName;
	delete iAuthorUrl;
	delete iRelatedUrl;
	delete iAuthorVideosUrl;
	}

CVideoEntry::CVideoEntry()
	{
	}
	
void CVideoEntry::ConstructL()
	{
	iSavedFileName.Copy( KNullDesC() );
	iThumbnailUrl = KNullDesC().AllocL();
	iThumbnailFile = KNullDesC().AllocL();

	iUrl = KNullDesC().AllocL();
	iVideoId = KNullDesC().AllocL();
	iAuthorName = KNullDesC().AllocL();
	iAuthorUrl = KNullDesC().AllocL();
	iRelatedUrl = KNullDesC().AllocL();
	iAuthorVideosUrl = KNullDesC().AllocL();
	iMediaTitle = KNullDesC().AllocL();
	iCategory = KNullDesC().AllocL();
	}

void CVideoEntry::ConstructL( CVideoEntry* aEntry )
	{
	iMediaTitle = aEntry->MediaTitle().AllocL();
	iCategory = aEntry->Category().AllocL();
	iUrl = aEntry->Url().AllocL();
	iThumbnailUrl = aEntry->ThumbnailUrl().AllocL();
	iThumbnailFile = aEntry->ThumbnailFile().AllocL();
	iVideoId = aEntry->VideoId().AllocL();
	iAuthorName = aEntry->AuthorName().AllocL();
	iAuthorUrl = aEntry->AuthorUrl().AllocL();
	iRelatedUrl = aEntry->RelatedUrl().AllocL();
	iAuthorVideosUrl = aEntry->AuthorVideosUrl().AllocL();

	SetDuration( aEntry->Duration() );
	SetAverageRating( aEntry->AverageRating() );
	SetViewCount( aEntry->ViewCount() );
	SetVideoFileSize( aEntry->VideoFileSize() );
	
	if( aEntry->SavedFileName().Length() )
		iSavedFileName.Copy( aEntry->SavedFileName() );
	else
		iSavedFileName.Copy( KNullDesC() );
	}


void CVideoEntry::SetTitleL( const TDesC& aTitle )
	{
	delete iMediaTitle;
	iMediaTitle = aTitle.AllocL();
	}

void CVideoEntry::SetCategoryL( const TDesC& aCategory )
	{
	delete iCategory;
	iCategory = aCategory.AllocL();
	}

void CVideoEntry::SetAuthorNameL( const TDesC& aName )
	{
	delete iAuthorName;
	iAuthorName = aName.AllocL();
	}

void CVideoEntry::SetRelatedUrlL( const TDesC& aUrl )
	{
	delete iRelatedUrl;
	iRelatedUrl = aUrl.AllocL();
	}

void CVideoEntry::SetAuthorVideosUrlL( const TDesC& aName )
	{
	delete iAuthorVideosUrl;
	iAuthorVideosUrl = aName.AllocL();
	}

void CVideoEntry::SetUrlL( const TDesC& aUrl )
	{
	delete iUrl;
	iUrl = aUrl.AllocL();
	}

void CVideoEntry::SetVideoIdL( const TDesC& aId )
	{
	delete iVideoId;
	iVideoId = aId.AllocL();
	}

void CVideoEntry::SetVideoUrlL( const TDesC& aUrl )
	{
	delete iUrl;
	delete iVideoId;

	iUrl = aUrl.AllocL();

	TInt index = iUrl->LocateReverse( '=' );
	if( index != KErrNotFound )
		{
		TPtrC name = iUrl->Right( iUrl->Length() - index - 1 );
		iVideoId = name.AllocL();
		}
	else
		{
		iVideoId = KNullDesC().AllocL();
		}
	
	SetRelatedUrlL( *iVideoId );
	}

void CVideoEntry::GenerateThumbnailFileL( const TDesC& aCacheDirectory )
	{
	delete iThumbnailFile;

	RBuf path;
	CleanupClosePushL( path );
	path.Create( aCacheDirectory.Length() + iVideoId->Length() + KCacheFileExtension().Length() );
	path.Copy( aCacheDirectory );
	path.Append( *iVideoId );
	path.Append( KCacheFileExtension() );
	iThumbnailFile = path.AllocL();
	CleanupStack::PopAndDestroy( &path );
	}

void CVideoEntry::SetThumbnailUrlL( const TDesC& aUrl )
	{
	delete iThumbnailUrl;
	iThumbnailUrl = aUrl.AllocL();
	}

void CVideoEntry::SetThumbnailFileL( const TDesC& aFilename )
	{
	delete iThumbnailFile;
	iThumbnailFile = aFilename.AllocL();
	}

void CVideoEntry::SetDuration( TInt aSeconds )
	{
	iDuration = aSeconds;
	}

void CVideoEntry::SetAverageRating( TReal32 aAverage )
	{
	if( aAverage < 0.0f )
		iAverageRating = 0.0f;
	else if( aAverage > 5.0f )
		iAverageRating = 5.0f;
	else
		iAverageRating = aAverage;
	}

void CVideoEntry::SetViewCount( TInt aCount )
	{
	iViewCount = aCount;
	}
	
CFbsBitmap* CVideoEntry::Bitmap()
	{
	if( !iBitmap )
		{
		iBitmap = new (ELeave) CFbsBitmap;
		iBitmap->Create(TSize(10, 10), EColor64K);
		}
	return iBitmap;
	}

TBool CVideoEntry::ImageLoaded()
	{
	return iImageLoaded;
	}

void CVideoEntry::SetImageLoaded( TBool aLoaded )
	{
	iImageLoaded = aLoaded;
	}

CFbsBitmap* CVideoEntry::ScaledBitmapL( TInt aDstWidth, TInt aDstHeight )
	{
	if( iScaledBitmap )
		delete iScaledBitmap;
	iScaledBitmap = NULL;
	
	ScaleImageL( aDstWidth, aDstHeight );
	return iScaledBitmap;
	}

CFbsBitmap* CVideoEntry::ScaledBitmap()
	{
	return iScaledBitmap;
	}

void CVideoEntry::ScaleImageL( TInt aDstWidth, TInt aDstHeight )
	{
	iScaledBitmap = new (ELeave) CFbsBitmap;

	TBool scale = (aDstWidth != 0 && aDstHeight != 0);
	if( !scale )
		{
		aDstWidth = iBitmap->SizeInPixels().iWidth;
		aDstHeight = iBitmap->SizeInPixels().iHeight;
		}

	TSize size( aDstWidth, aDstHeight );
	iScaledBitmap->Create( size, EColor64K );
	
	UiItemGfx::BltIconScale( iScaledBitmap, iBitmap, size, TPoint(0,0) );

	}

void CVideoEntry::ExportL( RFileWriteStream &aStream )
	{
	aStream.WriteInt16L( iMediaTitle->Length() );
	aStream.WriteL( *iMediaTitle, iMediaTitle->Length() );

	aStream.WriteInt16L( iUrl->Length() );
	aStream.WriteL( *iUrl, iUrl->Length() );

	aStream.WriteInt16L( iThumbnailUrl->Length() );
	aStream.WriteL( *iThumbnailUrl, iThumbnailUrl->Length() );

	aStream.WriteInt16L( iThumbnailFile->Length() );
	aStream.WriteL( *iThumbnailFile, iThumbnailFile->Length() );

	aStream.WriteInt16L( iVideoId->Length() );
	aStream.WriteL( *iVideoId, iVideoId->Length() );

	aStream.WriteInt16L( iAuthorName->Length() );
	aStream.WriteL( *iAuthorName, iAuthorName->Length() );

	aStream.WriteInt16L( iAuthorUrl->Length() );
	aStream.WriteL( *iAuthorUrl, iAuthorUrl->Length() );

	aStream.WriteInt16L( iRelatedUrl->Length() );
	aStream.WriteL( *iRelatedUrl, iRelatedUrl->Length() );

	aStream.WriteInt16L( iAuthorVideosUrl->Length() );
	aStream.WriteL( *iAuthorVideosUrl, iAuthorVideosUrl->Length() );

	aStream.WriteInt32L( iThumbnailHeight );
	aStream.WriteInt32L( iThumbnailWidth );
	aStream.WriteInt32L( iDuration );
	aStream.WriteReal32L( iAverageRating );

	aStream.WriteInt32L( iViewCount );

	aStream.CommitL();
	}

void CVideoEntry::ImportL( RFileReadStream &aStream )
	{
	delete iBitmap;
	iBitmap = NULL;
	delete iScaledBitmap;
	iScaledBitmap = NULL;
	delete iMediaTitle;
	iMediaTitle = NULL;
	delete iUrl;
	iUrl = NULL;
	delete iThumbnailUrl;
	iThumbnailUrl = NULL;
	delete iThumbnailFile;
	iThumbnailFile = NULL;
	delete iVideoId;
	iVideoId = NULL;
	delete iAuthorName;
	iAuthorName = NULL;
	delete iAuthorUrl;
	iAuthorUrl = NULL;
	delete iRelatedUrl;
	iRelatedUrl = NULL;
	delete iAuthorVideosUrl;
	iAuthorVideosUrl = NULL;

	TInt len;

	len = aStream.ReadInt16L();
	if( len )
		{
		iMediaTitle = HBufC::NewL( len );
		TPtr tmp(iMediaTitle->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iMediaTitle = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iUrl = HBufC::NewL( len );
		TPtr tmp(iUrl->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iUrl = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iThumbnailUrl = HBufC::NewL( len );
		TPtr tmp(iThumbnailUrl->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iThumbnailUrl = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iThumbnailFile = HBufC::NewL( len );
		TPtr tmp(iThumbnailFile->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iThumbnailFile = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iVideoId = HBufC::NewL( len );
		TPtr tmp(iVideoId->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iVideoId = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iAuthorName = HBufC::NewL( len );
		TPtr tmp(iAuthorName->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iAuthorName = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iAuthorUrl = HBufC::NewL( len );
		TPtr tmp(iAuthorUrl->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iAuthorUrl = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iRelatedUrl = HBufC::NewL( len );
		TPtr tmp(iRelatedUrl->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iRelatedUrl = KNullDesC().AllocL();
		}

	len = aStream.ReadInt16L();
	if( len )
		{
		iAuthorVideosUrl = HBufC::NewL( len );
		TPtr tmp(iAuthorVideosUrl->Des());
		aStream.ReadL(tmp, len);
		}
	else
		{
		iAuthorVideosUrl = KNullDesC().AllocL();
		}

	iThumbnailHeight = aStream.ReadInt32L();
	iThumbnailWidth = aStream.ReadInt32L();
	iDuration = aStream.ReadInt32L();
	iAverageRating = aStream.ReadReal32L();
	iViewCount = aStream.ReadInt32L();
	}

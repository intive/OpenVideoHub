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

#ifndef EMTUBE_CACHE_H
#define EMTUBE_CACHE_H

#include <e32base.h>

_LIT( KCacheFileExtension, ".ytci" );

class CEmTubeCacheEntry : public CBase
{
	public:
		static CEmTubeCacheEntry* NewL( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData );
		static CEmTubeCacheEntry* NewLC( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData );
		~CEmTubeCacheEntry();

	public:
		TDesC& String() { return *iString; }
		TDesC& FileName() { return iFileName; }
		TInt FileSize() { return iFileSize; }

	private:
		CEmTubeCacheEntry();
		void ConstructL( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData );

	private:
		TFileName iFileName;
		HBufC* iString;
		TInt iFileSize;
};

class CEmTubeCache : public CBase
{
	public:
		static CEmTubeCache* NewL( TInt aMaxSize );
		static CEmTubeCache* NewLC( TInt aMaxSize );
		~CEmTubeCache();

	public:
		TDesC& CacheDirectory() { return iCacheDirectory; }

		TBool IsFileInCache( const TDesC& aFile );
		void UpdateCacheL();

		TInt MaxCacheSize() { return iMaxCacheSize; }
		void SetMaxCacheSize( TInt aMaxSize ) { iMaxCacheSize = aMaxSize; }

		void SaveDataInCacheL( const TDesC& aUrl, const TDesC8& aData );
		HBufC8* GetDataFromCacheL( const TDesC& aUrl );

	private:
		CEmTubeCache( TInt aMaxSize );
		void ConstructL();

		CEmTubeCacheEntry* FindEntryByStringL( const TDesC& aString );
		TInt FindEntryByFileL( const TDesC& aName );

	private:
		TFileName iCacheDirectory;
		RFs iSession;
		TInt iMaxCacheSize;

		RPointerArray<CEmTubeCacheEntry> iCacheEntries;
};

#endif //EMTUBE_CACHE_H

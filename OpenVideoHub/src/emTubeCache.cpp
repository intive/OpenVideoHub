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

#include <coeaui.h>
#include <eikenv.h>
#include <eikappui.h>
#include <eikapp.h>
#include <BAUTILS.H>

#ifdef EKA2	// S60 3rd Ed
#include <hash.h>
#endif

#include "emTubeCache.h"

CEmTubeCacheEntry* CEmTubeCacheEntry::NewL( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData )
	{
	CEmTubeCacheEntry* self = NewLC( aFs, aString, aFileName, aData );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeCacheEntry* CEmTubeCacheEntry::NewLC( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData )
	{
	CEmTubeCacheEntry* self = new (ELeave) CEmTubeCacheEntry();
	CleanupStack::PushL(self);
	self->ConstructL( aFs, aString, aFileName, aData );
	return self;
	}

CEmTubeCacheEntry::~CEmTubeCacheEntry()
	{
	delete iString;
	}

CEmTubeCacheEntry::CEmTubeCacheEntry()
	{
	}

void CEmTubeCacheEntry::ConstructL( RFs& aFs, const TDesC& aString, const TDesC& aFileName, const TDesC8& aData )
	{
	iString = aString.AllocL();
	iFileName.Copy( aFileName );

	RFile file;
	CleanupClosePushL( file );

	file.Replace( aFs, aFileName, EFileWrite | EFileStream );
	file.Write( aData );
	iFileSize = aData.Length();

	CleanupStack::PopAndDestroy( &file );
	}

//cache

CEmTubeCache* CEmTubeCache::NewL( TInt aMaxSize )
	{
	CEmTubeCache* self = NewLC( aMaxSize );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeCache* CEmTubeCache::NewLC( TInt aMaxSize )
	{
	CEmTubeCache* self = new (ELeave) CEmTubeCache( aMaxSize );
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeCache::CEmTubeCache( TInt aMaxSize ) : iMaxCacheSize( aMaxSize )
	{
	}

CEmTubeCache::~CEmTubeCache()
	{
	iCacheEntries.ResetAndDestroy();
	iCacheEntries.Close();

	iSession.Close();
	}

void CEmTubeCache::ConstructL()
	{
	iSession.Connect();

#ifndef __WINS__
    TParsePtrC Parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
    iCacheDirectory = Parse.DriveAndPath();
	TPtrC16 driveLetter = iCacheDirectory.Left( 2 );
	iCacheDirectory.Copy( driveLetter );
	TPath privateDir;
	User::LeaveIfError( iSession.PrivatePath( privateDir ) );
	iCacheDirectory.Append( privateDir );
	iCacheDirectory.Append( _L("cache\\") );
#else
	iCacheDirectory.Copy( _L("C:\\Data\\Cache\\") );
#endif
	iSession.MkDirAll( iCacheDirectory );
	}

TBool CEmTubeCache::IsFileInCache( const TDesC& aFile )
	{
	return BaflUtils::FileExists( iSession, aFile );
	}

void CEmTubeCache::UpdateCacheL()
	{
	TInt currentSize = 0;

	CDir* dirList;
	CDir* fileList;

	User::LeaveIfError( iSession.GetDir( iCacheDirectory, KEntryAttNormal, ESortByDate|EAscending,fileList,dirList) );

	CleanupStack::PushL(fileList);
	CleanupStack::PushL(dirList);

	for(TInt i=0;i<fileList->Count();i++ )
		{
		if ( !( (*fileList)[i].iName.Right(5).Compare( KCacheFileExtension() ) ) )
			{
			if( iMaxCacheSize )
				{
				currentSize += (*fileList)[i].iSize;
				}
			else
				{
				RBuf fullPath;
				CleanupClosePushL( fullPath );
				fullPath.Create( iCacheDirectory.Length() + (*fileList)[i].iName.Length() );
				fullPath.Append( iCacheDirectory );
				fullPath.Append( (*fileList)[i].iName );

				TInt idx = FindEntryByFileL( fullPath );
				if( idx != KErrNotFound )
					{
					CEmTubeCacheEntry* cData = iCacheEntries[ idx ];
					iCacheEntries.Remove( idx );
					delete cData;
					}

				BaflUtils::DeleteFile( iSession, fullPath );
				CleanupStack::PopAndDestroy( &fullPath );
				}
			}
		}

	if( currentSize >= iMaxCacheSize )
		{
		TInt maxSize = iMaxCacheSize / 2;
		TInt cur = 0;
		while( currentSize > maxSize )
			{
			if ( !( (*fileList)[cur].iName.Right(5).Compare( KCacheFileExtension() ) ) )
				{
				RBuf fullPath;
				CleanupClosePushL( fullPath );

				fullPath.Create( iCacheDirectory.Length() + (*fileList)[cur].iName.Length() );
				fullPath.Append( iCacheDirectory );
				fullPath.Append( (*fileList)[cur].iName );

				TInt idx = FindEntryByFileL( fullPath );
				if( idx != KErrNotFound )
					{
					CEmTubeCacheEntry* cData = iCacheEntries[ idx ];
					iCacheEntries.Remove( idx );
					delete cData;
					}

				BaflUtils::DeleteFile( iSession, fullPath );
				currentSize -= (*fileList)[cur].iSize;

				CleanupStack::PopAndDestroy( &fullPath );
				}
			cur++;
			if( cur >= fileList->Count() )
				break;
			}
		}

	CleanupStack::PopAndDestroy(dirList);
	CleanupStack::PopAndDestroy(fileList);
	}

CEmTubeCacheEntry* CEmTubeCache::FindEntryByStringL( const TDesC& aString )
	{
	for( TInt i=0; i<iCacheEntries.Count(); i++ )
		{
		CEmTubeCacheEntry* c = iCacheEntries[i];
		if( !c->String().Compare( aString ) )
			{
			return c;
			}
		}
	return NULL;
	}

TInt CEmTubeCache::FindEntryByFileL( const TDesC& aName )
	{
	for( TInt i=0; i<iCacheEntries.Count(); i++ )
		{
		CEmTubeCacheEntry* c = iCacheEntries[i];
		if( !c->FileName().Compare( aName ) )
			{
			return i;
			}
		}
	return KErrNotFound;
	}

void CEmTubeCache::SaveDataInCacheL( const TDesC& aUrl, const TDesC8& aData )
	{
//TODO -> cache disabled!
	return;

	if( aData.Length() > 64*1024 )
		return;

	UpdateCacheL();

	if( !FindEntryByStringL( aUrl ) )
		{
		TFileName name;

	//filename generation
		_LIT( KDigestFormat, "%02x" );

#ifdef EKA2	// S60 3rd Ed
		CMD5 *md5 = CMD5::NewL();
		CleanupStack ::PushL( md5 );

	 	TPtrC8 ptr( (TUint8*)aUrl.Ptr(), aUrl.Length() << 1 );
		TPtrC8 ptrHash = md5->Hash( ptr );

	 	name.Copy( iCacheDirectory );

		for( TInt i=0; i < ptrHash.Length(); i++ )
			{
			name.AppendFormat( KDigestFormat, ptrHash[i] );
			}
		CleanupStack::PopAndDestroy( md5 );
#endif
		name.Append( KCacheFileExtension() );

		CEmTubeCacheEntry *data = CEmTubeCacheEntry::NewL( iSession, aUrl, name, aData );
		iCacheEntries.Append( data );
		}
	}

HBufC8* CEmTubeCache::GetDataFromCacheL( const TDesC& aUrl )
	{
	HBufC8* data = NULL;

//TODO - cache disabled!
	return data;

	CEmTubeCacheEntry* cData = FindEntryByStringL( aUrl );
	if( cData )
		{
		RFile file;
		TInt res = file.Open( iSession, cData->FileName(), EFileRead | EFileStream );
		CleanupClosePushL( file );
		if( res == KErrNone )
			{
			data = HBufC8::NewL( cData->FileSize() );
			TPtr8 tmp( data->Des() );
			file.Read( tmp );
			}
		CleanupStack::PopAndDestroy( &file );
		}
	return data;
	}

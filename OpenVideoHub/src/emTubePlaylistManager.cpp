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
#include <S32FILE.H>

#include <stringloader.h>

#include "emTubePlaylistManager.h"
#include "emTubeResource.h"

//playlist entry
CEmTubePlaylistEntry* CEmTubePlaylistEntry::NewL( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType )
	{
	CEmTubePlaylistEntry* self = NewLC( aLocation, aName, aType );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylistEntry* CEmTubePlaylistEntry::NewLC( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType )
	{
	CEmTubePlaylistEntry* self = new (ELeave) CEmTubePlaylistEntry( aType );
	CleanupStack::PushL(self);
	self->ConstructL( aLocation, aName );
	return self;
	}

CEmTubePlaylistEntry* CEmTubePlaylistEntry::NewL()
	{
	CEmTubePlaylistEntry* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylistEntry* CEmTubePlaylistEntry::NewLC()
	{
	CEmTubePlaylistEntry* self = new (ELeave) CEmTubePlaylistEntry();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePlaylistEntry::~CEmTubePlaylistEntry()
	{
	delete iName;
	delete iLocation;
	}

CEmTubePlaylistEntry::CEmTubePlaylistEntry( CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType ) : iType( aType )
	{
	}

void CEmTubePlaylistEntry::ConstructL( const TDesC& aLocation, const TDesC& aName )
	{
	iLocation = aLocation.AllocL();
	iName = aName.AllocL();
	iTime.HomeTime();
	}

CEmTubePlaylistEntry::CEmTubePlaylistEntry()
	{
	}

void CEmTubePlaylistEntry::ConstructL()
	{
	}

void CEmTubePlaylistEntry::ImportL( RFileReadStream& aStream )
	{
	TInt l = aStream.ReadInt32L();
	if( l )
		{
		iLocation = HBufC::NewL( l );
		TPtr pLocation( iLocation->Des() );
		aStream.ReadL( pLocation, l );
		}
	else
		{
		iLocation = KNullDesC().AllocL();
		}

	l = aStream.ReadInt32L();
	if( l )
		{
		iName = HBufC::NewL( l );
		TPtr pName( iName->Des() );
		aStream.ReadL( pName, l );
		}
	else
		{
		iName = KNullDesC().AllocL();
		}

	iPlayCount = aStream.ReadInt32L();
	iType = (TEmTubePlaylistEntryType)aStream.ReadInt32L();
	TReal t = aStream.ReadReal64L();
	iTime = TTime( Int64( t ) );
	}

void CEmTubePlaylistEntry::ExportL( RFileWriteStream& aStream )
	{
	TInt l = iLocation->Length();
	aStream.WriteInt32L( l );
	if( l )
		{
		aStream.WriteL( *iLocation );
		}

	l = iName->Length();
	aStream.WriteInt32L( l );
	if( l )
		{
		aStream.WriteL( *iName );
		}

	aStream.WriteInt32L( iPlayCount );
	aStream.WriteInt32L( (TInt)iType );
	aStream.WriteReal64L( TReal(iTime.Int64()) );
	}

//playlist
CEmTubePlaylist* CEmTubePlaylist::NewL( const TDesC& aName, TEmTubePlaylistType aType )
	{
	CEmTubePlaylist* self = CEmTubePlaylist::NewLC( aName, aType );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylist* CEmTubePlaylist::NewLC( const TDesC& aName, TEmTubePlaylistType aType )
	{
	CEmTubePlaylist* self = new (ELeave) CEmTubePlaylist( aType );
	CleanupStack::PushL(self);
	self->ConstructL( aName );
	return self;
	}

CEmTubePlaylist::~CEmTubePlaylist()
	{
	iEntries.ResetAndDestroy();
	iEntries.Close();
	delete iName;
	}

CEmTubePlaylist::CEmTubePlaylist( TEmTubePlaylistType aType ) : iType (aType )
	{
	if( aType == EPlaylistUserDefined )
		SetEditable( ETrue );
	else
		SetEditable( EFalse );
	SetDirty( EFalse );
	}

void CEmTubePlaylist::ConstructL( const TDesC& aName )
	{
	iName = aName.AllocL();
	}

CEmTubePlaylist* CEmTubePlaylist::NewL( RFileReadStream& aStream )
	{
	CEmTubePlaylist* self = CEmTubePlaylist::NewLC( aStream );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylist* CEmTubePlaylist::NewLC( RFileReadStream& aStream )
	{
	CEmTubePlaylist* self = new (ELeave) CEmTubePlaylist();
	CleanupStack::PushL(self);
	self->ConstructL( aStream );
	return self;
	}

CEmTubePlaylist::CEmTubePlaylist()
	{
	}

void CEmTubePlaylist::ConstructL( RFileReadStream& aStream )
	{
	TInt l = aStream.ReadInt32L();
	if( l )
		{
		iName = HBufC::NewL( l );
		TPtr pName( iName->Des() );
		aStream.ReadL( pName, l );
		}
	else
		{
		iName = KNullDesC().AllocL();
		}

	iType = (TEmTubePlaylistType)aStream.ReadInt32L();
	iFlags = aStream.ReadInt32L();

	TInt count = aStream.ReadInt32L();
	for(TInt i=0;i<count;i++)
		{
		CEmTubePlaylistEntry* e = CEmTubePlaylistEntry::NewLC();
		e->ImportL( aStream );
		iEntries.AppendL( e );
		CleanupStack::Pop( e );
		}
	}

void CEmTubePlaylist::ExportL( RFileWriteStream& aStream )
	{
	if( Name().Length() )
		{
		aStream.WriteInt32L( Name().Length() );
		aStream.WriteL( Name() );
		}
	else
		{
		aStream.WriteInt32L( 0 );
		}

	aStream.WriteInt32L( (TInt)iType );
	aStream.WriteInt32L( iFlags );

	aStream.WriteInt32L( iEntries.Count() );
	for(TInt i=0;i<iEntries.Count();i++)
		{
		iEntries[i]->ExportL( aStream );
		}
	}

void CEmTubePlaylist::SetEditable( TBool aEditable )
	{
	if ( aEditable )
		iFlags |= KPlaylistEditable;
	else
		iFlags &= (~KPlaylistEditable);
	}

void CEmTubePlaylist::SetDirty( TBool aDirty )
	{
	if ( aDirty )
		iFlags |= KPlaylistDirty;
	else
		iFlags &= (~KPlaylistDirty);
	}

void CEmTubePlaylist::SetLooped( TBool aLooped )
	{
	if ( aLooped )
		iFlags |= KPlaylistLooped;
	else
		iFlags &= (~KPlaylistLooped);
	}

CEmTubePlaylistEntry* CEmTubePlaylist::PrevEntry()
	{
	iCurrentEntry--;
	if( iCurrentEntry < 0 )
		{
		if ( iFlags & KPlaylistLooped )
			{
			iCurrentEntry = iEntries.Count() - 1;
			return iEntries[ iCurrentEntry ];
			}
		else
			{
			return NULL;
			}
		}
	else
		{
		return iEntries[ iCurrentEntry ];
		}
	}

TBool CEmTubePlaylist::CheckEntry( TBool aNext )
	{
	TBool ret = ETrue;
	TInt idx = iCurrentEntry;
	if( aNext )
		{
		idx++;
		if( idx >= iEntries.Count() )
			{
			if ( !(iFlags & KPlaylistLooped) )
				{
				ret = EFalse;
				}
			}
		}
	else
		{
		idx--;
		if( idx < 0 )
			{
			if ( !(iFlags & KPlaylistLooped) )
				{
				ret = EFalse;
				}
			}
		}
	return ret;
	}

CEmTubePlaylistEntry* CEmTubePlaylist::NextEntry()
	{
	iCurrentEntry++;
	if( iCurrentEntry >= iEntries.Count() )
		{
		if ( iFlags & KPlaylistLooped )
			{
			iCurrentEntry = 0;
			return iEntries[ iCurrentEntry ];
			}
		else
			{
			return NULL;
			}
		}
	else
		{
		return iEntries[ iCurrentEntry ];
		}
	}

CEmTubePlaylistEntry* CEmTubePlaylist::AddEntryL( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType, TInt aIdx )
	{
	CEmTubePlaylistEntry* e = CEmTubePlaylistEntry::NewL( aLocation, aName, aType );
	InsertEntry( e, aIdx );
	return e;
	}

CEmTubePlaylistEntry* CEmTubePlaylist::RemoveEntry( TInt aIdx )
	{
	CEmTubePlaylistEntry* e = iEntries[aIdx];
	iEntries.Remove( aIdx );
	return e;
	}

CEmTubePlaylistEntry* CEmTubePlaylist::RemoveEntry( CEmTubePlaylistEntry* aEntry )
	{
	TInt idx = iEntries.Find( aEntry );
	return RemoveEntry( idx );
	}

void CEmTubePlaylist::DeleteEntry( TInt aIdx )
	{
	CEmTubePlaylistEntry* e = iEntries[aIdx];
	iEntries.Remove( aIdx );
	delete e;
	}

void CEmTubePlaylist::InsertEntry( CEmTubePlaylistEntry* aEntry, TInt aIdx )
	{
	if( aIdx == -1 )
		iEntries.Append( aEntry );
	else
		iEntries.Insert( aEntry, aIdx );
	}

CEmTubePlaylistEntry* CEmTubePlaylist::Find( const TDesC& aLocation )
	{
	CEmTubePlaylistEntry *e = NULL;

	for(TInt i=0;i<EntriesCount();i++)
		{
		if( !aLocation.Compare( Entry(i)->Location() ))
			{
			e = Entry( i );
			break;
			}
		}
	return e;
	}

//manager
CEmTubePlaylistManager* CEmTubePlaylistManager::NewL()
	{
	CEmTubePlaylistManager* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlaylistManager* CEmTubePlaylistManager::NewLC()
	{
	CEmTubePlaylistManager* self = new (ELeave) CEmTubePlaylistManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePlaylistManager::~CEmTubePlaylistManager()
	{
	iPlaylists.ResetAndDestroy();
	iPlaylists.Close();
	}

CEmTubePlaylistManager::CEmTubePlaylistManager()
	{
	}

void CEmTubePlaylistManager::ConstructL()
	{
	SetCurrentPlaylist( KErrNotFound );
	ImportPlaylistsL();
	}

_LIT( KPlaylistsFilename, "emtplaylists.bin" );
void CEmTubePlaylistManager::ExportPlaylistsL()
	{
	TFileName fileName;
	RFile file;

	RFs session;
	session.Connect();
	CleanupClosePushL( session );

#ifndef __WINS__
	TParsePtrC parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
	fileName.Copy( parse.Drive() );
	TPath privateDir;
	User::LeaveIfError( session.PrivatePath( privateDir ) );
	fileName.Append( privateDir );
#else
	fileName.Copy( _L("C:\\Data\\") );
#endif

	fileName.Append( KPlaylistsFilename );

	TInt err = file.Replace( session, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		stream.WriteInt32L( iPlaylists.Count() );

		for(TInt i=0;i<iPlaylists.Count();i++)
			{
			CEmTubePlaylist* pl = iPlaylists[i];
			pl->ExportL( stream );
			}
		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	CleanupStack::PopAndDestroy( &session );
	}

void CEmTubePlaylistManager::ImportPlaylistsL()
	{
	TFileName fileName;
	RFile file;
	RFs session;
	session.Connect();
	CleanupClosePushL( session );

#ifndef __WINS__
	TParsePtrC parse( CEikonEnv::Static()->EikAppUi()->Application()->AppFullName() );
	fileName.Copy( parse.Drive() );
	TPath privateDir;
	User::LeaveIfError( session.PrivatePath( privateDir ) );
	fileName.Append( privateDir );
#else
	fileName.Copy( _L("C:\\Data\\") );
#endif

	fileName.Append( KPlaylistsFilename );

	TInt err = file.Open( session, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TInt count = stream.ReadInt32L();
		for( TInt i=0;i<count;i++)
			{
			CEmTubePlaylist *pl = CEmTubePlaylist::NewL( stream );
			iPlaylists.Append( pl );
			}
		CleanupStack::PopAndDestroy( &stream );
		}

	if( iPlaylists.Count() == 0 )
		{
		HBufC* name = StringLoader::LoadLC( R_PLAYLIST_MOST_PLAYED_TXT );
		AddPlaylistL( *name, CEmTubePlaylist::EPlaylistInternal );
		CleanupStack::PopAndDestroy( name );

		name = StringLoader::LoadLC( R_PLAYLIST_RECENTLY_PLAYED_TXT );
		AddPlaylistL( *name, CEmTubePlaylist::EPlaylistInternal );
		CleanupStack::PopAndDestroy( name );

		name = StringLoader::LoadLC( R_PLAYLIST_RECENTLY_SAVED_TXT );
		AddPlaylistL( *name, CEmTubePlaylist::EPlaylistInternal );
		CleanupStack::PopAndDestroy( name );
		}

	CleanupStack::PopAndDestroy( &file );
	CleanupStack::PopAndDestroy( &session );
	}

void CEmTubePlaylistManager::AddPlaylistL( const TDesC& aName, CEmTubePlaylist::TEmTubePlaylistType aType )
	{
	CEmTubePlaylist *p = CEmTubePlaylist::NewL( aName, aType );
	iPlaylists.Append( p );
	}

void CEmTubePlaylistManager::UpdateStatsL( const TDesC& aLocation, const TDesC& aTitle, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType )
	{
	CEmTubePlaylist *mostPlayed = iPlaylists[0];
	CEmTubePlaylistEntry *e = mostPlayed->Find( aLocation );
	if( e )
		{
		e->IncreasePlayCount();

		if( mostPlayed->EntriesCount() > 1 )
			{
			TBool inserted = EFalse;
			e = mostPlayed->RemoveEntry( e );
			for(TInt i=0;i<mostPlayed->EntriesCount(); i++)
				{
				if( mostPlayed->Entry( i )->PlayCount() < e->PlayCount() )
					{
					mostPlayed->InsertEntry( e, i );
					inserted = ETrue;
					break;
					}
				}
			if( !inserted )
				mostPlayed->InsertEntry( e );
			}
		mostPlayed->SetDirty( ETrue );
		}
	else
		{
		CEmTubePlaylistEntry* e = mostPlayed->AddEntryL( aLocation, aTitle, aType );
		e->IncreasePlayCount();
		}

//recently played
	CEmTubePlaylist *recentlyPlayed = iPlaylists[1];
	e = recentlyPlayed->Find( aLocation );
	if( !e )
		{
		e = CEmTubePlaylistEntry::NewL( aLocation, aTitle, aType );
		recentlyPlayed->InsertEntry( e, 0 );
		}
	else
		{
		e = recentlyPlayed->RemoveEntry( e );
		recentlyPlayed->InsertEntry( e, 0 );
		TTime now;
		now.HomeTime();
		e->SetTime( now );
		}
	}

void CEmTubePlaylistManager::UpdateSavedClipsStatsL( const TDesC& aLocation, const TDesC& aTitle )
	{
	CEmTubePlaylist* ls = iPlaylists[2];
	CEmTubePlaylistEntry *e = ls->Find( aLocation );
	if( !e )
		{
		e = CEmTubePlaylistEntry::NewL( aLocation, aTitle, CEmTubePlaylistEntry::EPlaylistEntryLocal );
		ls->InsertEntry( e, 0 );
		}
	}

CEmTubePlaylist* CEmTubePlaylistManager::Playlist( TInt aIdx, TBool aRefresh )
	{
	CEmTubePlaylist* pl;

	switch( aIdx )
		{
		case EStatsMostPlayed:
			pl = iPlaylists[ 0 ];
		break;

		case EStatsRecentlyPlayed:
			pl = iPlaylists[ 1 ];
			if( aRefresh )
				RemoveOutdatedEntries( pl );
		break;

		case EStatsRecentlySaved:
			pl = iPlaylists[ 2 ];
			if( aRefresh )
				RemoveOutdatedEntries( pl );
		break;

		default:
			pl = iPlaylists[ aIdx ];
		break;
		}
	return pl;
	}

void CEmTubePlaylistManager::RemoveOutdatedEntries( CEmTubePlaylist* aPlaylist )
	{
	TTime weekago;
	weekago.HomeTime();
	TTimeIntervalHours week( 24*7 );
	weekago -= week;
	TInt count = aPlaylist->EntriesCount();
	for(TInt i=count-1;i>0;i--)
		{
		CEmTubePlaylistEntry *e = aPlaylist->Entry( i );
		TTimeIntervalHours diff;
		e->Time().HoursFrom( weekago, diff );
		if( diff > week )
			{
			aPlaylist->RemoveEntry( i );
			delete e;
			}
		else
			{
			break;
			}
		}
	}

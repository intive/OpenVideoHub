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

#ifndef EMTUBE_PLAYLIST_MANAGER_H
#define EMTUBE_PLAYLIST_MANAGER_H

#include <e32base.h>
#include <S32FILE.H>

class CEmTubePlaylistEntry : public CBase
{
public:
	enum TEmTubePlaylistEntryType
		{
		EPlaylistEntryLocal = 0,
		EPlaylistEntryRemote
		};

public:
	static CEmTubePlaylistEntry* NewL( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType );
	static CEmTubePlaylistEntry* NewLC( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType );
	static CEmTubePlaylistEntry* NewL();
	static CEmTubePlaylistEntry* NewLC();
	~CEmTubePlaylistEntry();

public:
	const TDesC& Location() { return *iLocation; }
	const TDesC& Name() { return *iName; }
	TEmTubePlaylistEntryType Type() { return iType; }
	TInt PlayCount() { return iPlayCount; }
	void SetTime( TTime& aTime ) { iTime = aTime; }
	TTime& Time() { return iTime; }
	void IncreasePlayCount() { iPlayCount++; }

	void ImportL( RFileReadStream& aStream );
	void ExportL( RFileWriteStream& aStream );

private:
	CEmTubePlaylistEntry( CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType );
	void ConstructL( const TDesC& aLocation, const TDesC& aName );
	CEmTubePlaylistEntry();
	void ConstructL();

private: //data
	HBufC* iLocation;
	HBufC* iName;
	TEmTubePlaylistEntryType iType;
	TInt iPlayCount;
	TTime iTime;
};

#define KPlaylistEditable		( 1 << 0 ) // user can move entries around
#define KPlaylistDirty			( 1 << 1 ) // list has been changed and has to be sorted before it can be shown.
#define KPlaylistLooped			( 1 << 2 ) // loop the whole playlist.

class CEmTubePlaylist : public CBase
{
	public:
		enum TEmTubePlaylistType
			{
			EPlaylistInternal = 0,
			EPlaylistUserDefined
			};

	public:
		static CEmTubePlaylist* NewL( const TDesC& aName, TEmTubePlaylistType aType );
		static CEmTubePlaylist* NewLC( const TDesC& aName, TEmTubePlaylistType aType );
		static CEmTubePlaylist* NewL( RFileReadStream& aStream );
		static CEmTubePlaylist* NewLC( RFileReadStream& aStream );
		~CEmTubePlaylist();

	public:
		const TDesC& Name() {return *iName;}
		void SetName( const TDesC& aName ) {delete iName; iName = aName.AllocL(); }

		TEmTubePlaylistType Type() {return iType;}

		TInt EntriesCount() {return iEntries.Count(); }
		CEmTubePlaylistEntry *Entry( TInt aIdx ) {return iEntries[aIdx];}

		CEmTubePlaylistEntry* AddEntryL( const TDesC& aLocation, const TDesC& aName, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType, TInt aIdx = -1 );
		CEmTubePlaylistEntry* RemoveEntry( TInt aIdx );
		CEmTubePlaylistEntry* RemoveEntry( CEmTubePlaylistEntry* aEntry );
		void DeleteEntry( TInt aIdx );
		void InsertEntry( CEmTubePlaylistEntry* aEntry, TInt aIdx = -1 );

		CEmTubePlaylistEntry* Find( const TDesC& aLocation );

		void SetEditable( TBool aEditable );
		void SetDirty( TBool aDirty );
		void SetLooped( TBool aLooped );

		TBool Editable() {return (iFlags & KPlaylistEditable) ? (TBool)ETrue : (TBool)EFalse; }
		TBool Dirty() {return (iFlags & KPlaylistDirty) ? (TBool)ETrue : (TBool)EFalse; }
		TBool Looped() {return (iFlags & KPlaylistLooped) ? (TBool)ETrue : (TBool)EFalse; }

		void ExportL( RFileWriteStream& aStream );

		CEmTubePlaylistEntry* CurrentEntry() { return iEntries[ iCurrentEntry ]; }
		CEmTubePlaylistEntry* PrevEntry();
		CEmTubePlaylistEntry* NextEntry();
		TBool CheckEntry( TBool aNext );
		void SetCurrentEntry( TInt aIdx ) { iCurrentEntry = aIdx; }

	private:
		CEmTubePlaylist( TEmTubePlaylistType aType );
		CEmTubePlaylist();
		void ConstructL( const TDesC& aName );
		void ConstructL( RFileReadStream& aStream );

	private: //data
		TInt iFlags;
		RPointerArray<CEmTubePlaylistEntry> iEntries;
		HBufC* iName;
		TEmTubePlaylistType iType;
		TInt iCurrentEntry;
};

class CEmTubePlaylistManager : public CBase
{
	public:
		enum TStatsPlaylist
			{
			EStatsMostPlayed = 0,
			EStatsRecentlyPlayed,
			EStatsRecentlySaved
			};
	public:
		static CEmTubePlaylistManager* NewL();
		static CEmTubePlaylistManager* NewLC();
		~CEmTubePlaylistManager();

	public: //methods
		CEmTubePlaylist* Playlist( TInt aIdx, TBool aRefresh = EFalse );
		TInt PlaylistsCount() {return iPlaylists.Count(); }
		TInt CurrentPlaylist() { return iCurrentPlaylist; }
		void SetCurrentPlaylist( TInt aIdx ) { iCurrentPlaylist = aIdx; }

		TInt Find( CEmTubePlaylist* aPlaylist ) { return iPlaylists.Find( aPlaylist ); }

		void ExportPlaylistsL();
		void ImportPlaylistsL();

		void AddPlaylistL( const TDesC& aName, CEmTubePlaylist::TEmTubePlaylistType aType );

		void UpdateStatsL( const TDesC& aLocation, const TDesC& aTitle, CEmTubePlaylistEntry::TEmTubePlaylistEntryType aType );
		void UpdateSavedClipsStatsL( const TDesC& aLocation, const TDesC& aTitle );
		CEmTubePlaylist* StatsPlaylistL( CEmTubePlaylistManager::TStatsPlaylist aType );

	private:
		void RemoveOutdatedEntries( CEmTubePlaylist* aPlaylist );

	private:
		CEmTubePlaylistManager();
		void ConstructL();

	private: //data
		RPointerArray<CEmTubePlaylist> iPlaylists;
		TInt iCurrentPlaylist;
};

#endif //EMTUBE_PLAYLIST_MANAGER_H

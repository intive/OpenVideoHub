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

#ifndef EMTUBE_PLUGIN_MANAGER_H
#define EMTUBE_PLUGIN_MANAGER_H

#include <e32base.h>
#include <PluginInterface.h>

class CEmTubeCookie : public CBase
{
	public:
		static CEmTubeCookie* NewL( const TDesC8& aName, const TDesC8& aValue );
		static CEmTubeCookie* NewLC( const TDesC8& aName, const TDesC8& aValue );
		static CEmTubeCookie* NewL( const TDesC8& aValue );
		static CEmTubeCookie* NewLC( const TDesC8& aValue );
		~CEmTubeCookie();
	
	public:
		const TDesC8& DesC();

	private:
		CEmTubeCookie();
		void ConstructL( const TDesC8& aName, const TDesC8& aValue );
		void ConstructL( const TDesC8& aValue );

	private: //data
		HBufC8* iValue;
};

enum TSearchState
	{
	ESearchNotStarted = 0,
	ESearching,
	ESearchFinished	
	};
		
class CEmTubePluginCredentials : public CBase
{
	public:
		static CEmTubePluginCredentials* NewL();
		static CEmTubePluginCredentials* NewLC();
		~CEmTubePluginCredentials();
	
	public: //methods
		void SetUid( TUint32 aUid ) { iUid = aUid; }
		TUint32 Uid() { return iUid; }

		void SetUsernameL( const TDesC& aUsername );
		const TDesC& Username() { return *iUsername; }

		void SetPasswordL( const TDesC& aPassword );
		const TDesC& Password() { return *iPassword; }

		void AddCookieL( const TDesC8& aName, const TDesC8& aValue );
		void AddCookieL( const TDesC8& aValue );
		void ClearCookies();
		RPointerArray<CEmTubeCookie>& Cookies() { return iCookies; }

	private:
		CEmTubePluginCredentials();
		void ConstructL();

	private: //data
		TUint32 iUid;
		HBufC* iUsername;
		HBufC* iPassword;
		RPointerArray<CEmTubeCookie> iCookies;
};

class CEmTubePluginManager : public CBase
{
	public:
		static CEmTubePluginManager* NewL();
		static CEmTubePluginManager* NewLC();
		~CEmTubePluginManager();
	
	public: //methods
		RPointerArray<CPluginInterface>& Plugins() { return iPlugins; }

		void SelectPlugin( TInt aIndex );
		TInt SelectPlugin( TUint32 aUid );
		TInt SelectPlugin( const TDesC& aUrl );

		TInt FindPlugin( const TDesC& aUrl );

		void RestorePlugin();
		CPluginInterface* Plugin( TUint32 aUid );
		CPluginInterface* Plugin( TInt aId );
		CPluginInterface* Plugin();
		TUint32 Uid();
		TUint32 Uid( TInt aId );

		TUint32 DefaultPluginUid();
		void SetDefaultPluginUid( TUint32 aUid );
		TUint32 TemporaryPluginUid();
		void SetTemporaryPluginUid( TUint32 aUid );

		void LoadPluginsL();

		const TDesC& Username();
		const TDesC& Password();
		const TDesC& Username( TUint32 aUid );
		const TDesC& Password( TUint32 aUid );
		RPointerArray<CEmTubeCookie>& Cookies();
		void ClearCookies();
		void ClearCookies( TUint32 aUid );
		RPointerArray<CEmTubeCookie>& Cookies( TUint32 aUid );

		void SetUsernameL( const TDesC& aUsername );
		void SetPasswordL( const TDesC& aPassword );
		void AddCookieL( const TDesC8& aName, const TDesC8& aValue );

		void SetUsernameL( TUint32 aUid, const TDesC& aUsername );
		void SetPasswordL( TUint32 aUid, const TDesC& aPassword );
		void AddCookieL( TUint32 aUid, const TDesC8& aName, const TDesC8& aValue );

		TBool LoggedIn();
		TBool LoggedIn( TUint32 aUid );

		void ExportCredentialsL();
		void ImportCredentialsL();
		
		void ResetPluginStates();

		TSearchState SearchState( TInt aWhich = -1 );
		void SetSearchState( TSearchState aState, TInt aWhich = -1 );		

	private:
		CEmTubePluginManager();
		void ConstructL();

		TInt FindCredentials( TUint32 aUid );

	private: //data
		RPointerArray<CPluginInterface> iPlugins;
		RPointerArray<CEmTubePluginCredentials> iPluginCredentials;

		RArray<TUint32> iPluginUids;
		TInt iSelectedPluginId;
		TInt32 iDefaultPluginUid;
		TUint32 iTemporaryPluginUid;
		RArray<TSearchState> iPluginState;
};

#endif //EMTUBE_PLUGIN_MANAGER_H

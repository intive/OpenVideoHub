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

#include "emTubePluginManager.h"

CEmTubeCookie* CEmTubeCookie::NewL( const TDesC8& aName, const TDesC8& aValue )
	{
	CEmTubeCookie* self = NewLC( aName, aValue );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeCookie* CEmTubeCookie::NewLC( const TDesC8& aName, const TDesC8& aValue )
	{
	CEmTubeCookie* self = new (ELeave) CEmTubeCookie();
	CleanupStack::PushL(self);
	self->ConstructL( aName, aValue );
	return self;
	}

CEmTubeCookie* CEmTubeCookie::NewL( const TDesC8& aValue )
	{
	CEmTubeCookie* self = NewLC( aValue );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeCookie* CEmTubeCookie::NewLC( const TDesC8& aValue )
	{
	CEmTubeCookie* self = new (ELeave) CEmTubeCookie();
	CleanupStack::PushL(self);
	self->ConstructL( aValue );
	return self;
	}

CEmTubeCookie::~CEmTubeCookie()
	{
	delete iValue;
	}

CEmTubeCookie::CEmTubeCookie()
	{
	}

void CEmTubeCookie::ConstructL( const TDesC8& aName, const TDesC8& aValue )
	{
	iValue = HBufC8::NewL( aName.Length() + 1 + aValue.Length() );
	iValue->Des().Copy( aName );
	iValue->Des().Append( _L8( "=") );
	iValue->Des().Append( aValue );
	}

void CEmTubeCookie::ConstructL( const TDesC8& aValue )
	{
	iValue = aValue.AllocL();
	}

const TDesC8& CEmTubeCookie::DesC()
	{
	return *iValue;
	}

//credentials
CEmTubePluginCredentials* CEmTubePluginCredentials::NewL()
	{
	CEmTubePluginCredentials* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePluginCredentials* CEmTubePluginCredentials::NewLC()
	{
	CEmTubePluginCredentials* self = new (ELeave) CEmTubePluginCredentials();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePluginCredentials::~CEmTubePluginCredentials()
	{
	delete iUsername;
	delete iPassword;
	iCookies.ResetAndDestroy();
	iCookies.Close();
	}

CEmTubePluginCredentials::CEmTubePluginCredentials()
	{
	}

void CEmTubePluginCredentials::ConstructL()
	{
	iUsername = KNullDesC().AllocL();
	iPassword = KNullDesC().AllocL();
	}

void CEmTubePluginCredentials::SetUsernameL( const TDesC& aUsername )
	{
	delete iUsername;
	iUsername = aUsername.AllocL();
	}

void CEmTubePluginCredentials::SetPasswordL( const TDesC& aPassword )
	{
	delete iPassword;
	iPassword = aPassword.AllocL();
	}

void CEmTubePluginCredentials::AddCookieL( const TDesC8& aName, const TDesC8& aValue )
	{
	CEmTubeCookie *c = CEmTubeCookie::NewL( aName, aValue );
	iCookies.Append( c );
	}

void CEmTubePluginCredentials::AddCookieL( const TDesC8& aValue )
	{
	CEmTubeCookie *c = CEmTubeCookie::NewL( aValue );
	iCookies.Append( c );
	}

void CEmTubePluginCredentials::ClearCookies()
	{
	iCookies.ResetAndDestroy();
	}

//plugin manager implementation
CEmTubePluginManager* CEmTubePluginManager::NewL()
	{
	CEmTubePluginManager* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePluginManager* CEmTubePluginManager::NewLC()
	{
	CEmTubePluginManager* self = new (ELeave) CEmTubePluginManager();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubePluginManager::~CEmTubePluginManager()
	{
	iPlugins.ResetAndDestroy();
	iPlugins.Close();

	iPluginCredentials.ResetAndDestroy();
	iPluginCredentials.Close();

	iPluginUids.Reset();
	iPluginUids.Close();
	
	iPluginState.Reset();
	iPluginState.Close();	
	}

CEmTubePluginManager::CEmTubePluginManager()
	{
	}

void CEmTubePluginManager::ConstructL()
	{
	LoadPluginsL();

	if( iPlugins.Count() == 1 )
		{
		iSelectedPluginId = 0;
		}
	}

_LIT( KCredentialsFilename, "emtcredentials.bin" );
void CEmTubePluginManager::ExportCredentialsL()
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

	fileName.Append( KCredentialsFilename );

	TInt err = file.Replace( session, fileName, EFileStream | EFileWrite );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileWriteStream stream( file );
		CleanupClosePushL( stream );

		stream.WriteInt32L( iPluginCredentials.Count() );

		for(TInt i=0;i<iPluginCredentials.Count();i++)
			{
			stream.WriteInt32L( iPluginCredentials[i]->Uid() );

			if( iPluginCredentials[i]->Username().Length() )
				{
				stream.WriteInt32L( iPluginCredentials[i]->Username().Length() );
				stream.WriteL( iPluginCredentials[i]->Username() );
				}
			else
				{
				stream.WriteInt32L( 0 );
				}

			if( iPluginCredentials[i]->Password().Length() )
				{
				stream.WriteInt32L( iPluginCredentials[i]->Password().Length() );
				stream.WriteL( iPluginCredentials[i]->Password() );
				}
			else
				{
				stream.WriteInt32L( 0 );
				}

			RPointerArray<CEmTubeCookie>& cookies = iPluginCredentials[i]->Cookies();
			stream.WriteInt32L( cookies.Count() );
			for(TInt i=0;i<cookies.Count();i++ )
				{
				stream.WriteInt32L( cookies[i]->DesC().Length() );
				stream.WriteL( cookies[i]->DesC() );
				}
			}

		stream.CommitL();
		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	CleanupStack::PopAndDestroy( &session );
	}

void CEmTubePluginManager::ImportCredentialsL()
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

	fileName.Append( KCredentialsFilename );

	TInt err = file.Open( session, fileName, EFileStream | EFileRead );
	CleanupClosePushL( file );

	if( err == KErrNone )
		{
		RFileReadStream stream( file );
		CleanupClosePushL( stream );

		TInt count = stream.ReadInt32L();

		for( TInt i=0;i<count;i++)
			{
			CEmTubePluginCredentials *c = CEmTubePluginCredentials::NewL();
			iPluginCredentials.Append( c );

			TUint32 uid = stream.ReadInt32L();
			c->SetUid( uid );

			TInt len = stream.ReadInt32L();
			if( len )
				{
				HBufC* username = HBufC::NewLC( len );
				TPtr pUsername( username->Des() );
				stream.ReadL( pUsername, len );
				c->SetUsernameL( *username );
				CleanupStack::PopAndDestroy( username );
				}

			len = stream.ReadInt32L();
			if( len )
				{
				HBufC* password = HBufC::NewLC( len );
				TPtr pPassword( password->Des() );
				stream.ReadL( pPassword, len );
				c->SetPasswordL( *password );
				CleanupStack::PopAndDestroy( password );
				}

			TInt ccount = stream.ReadInt32L();
			for(TInt j=0;j<ccount;j++ )
				{
				len = stream.ReadInt32L();
				HBufC8* cookie = HBufC8::NewLC( len );
				TPtr8 pCookie( cookie->Des() );
				stream.ReadL( pCookie, len );
				c->AddCookieL( *cookie );
				CleanupStack::PopAndDestroy( cookie );
				}
			}

		CleanupStack::PopAndDestroy( &stream );
		}
	CleanupStack::PopAndDestroy( &file );
	CleanupStack::PopAndDestroy( &session );
	}

void CEmTubePluginManager::LoadPluginsL()
	{
	iPlugins.ResetAndDestroy();
	iPlugins.Close();

	iPluginUids.Reset();
	iPluginUids.Close();

	iPluginState.Reset();
	iPluginState.Close();

	iPluginCredentials.ResetAndDestroy();
	iPluginCredentials.Close();

	ImportCredentialsL();

	iSelectedPluginId = -1;
	RImplInfoPtrArray infoArray;


	CPluginInterface::ListAllImplementationsL( infoArray );

	if( !infoArray.Count() )
		User::Leave( KErrNotFound );

	for ( TInt i = 0; i < infoArray.Count(); i++ )
		{
		CImplementationInformation* info = infoArray[i];

		if( info->Version() == PLUGIN_API_VERSION )
			{
			TUid implementationUid = info->ImplementationUid();
			CPluginInterface* plugin = NULL;
			TRAPD( err, plugin = CPluginInterface::NewL( implementationUid ) );
			if ( err == KErrNone && plugin )
				{
				iPlugins.Append( plugin );
				iPluginUids.Append( implementationUid.iUid );
				iPluginState.Append( ESearchNotStarted );
				if( FindCredentials( implementationUid.iUid ) == KErrNotFound )
					{
					SetUsernameL( implementationUid.iUid, KNullDesC() );
					}
				}
			}
		else
			{
			//TODO -> display info, that plugin is outdated?
			}
		}
	infoArray.ResetAndDestroy();
	}

CPluginInterface* CEmTubePluginManager::Plugin( TUint32 aUid )
	{
	TInt idx = iPluginUids.Find( aUid );
	return iPlugins[ idx ];
	}

CPluginInterface* CEmTubePluginManager::Plugin( TInt aId )
	{
	return iPlugins[ aId ];
	}

TUint32 CEmTubePluginManager::Uid( TInt aId )
	{
	return iPluginUids[ aId ];
	}

TUint32 CEmTubePluginManager::Uid()
	{
	TUint32 ret = 0;
	if( iSelectedPluginId != -1 )
		ret = iPluginUids[ iSelectedPluginId ];

	return ret;
	}

TUint32 CEmTubePluginManager::DefaultPluginUid()
	{
	return iDefaultPluginUid;
	}

void CEmTubePluginManager::SetDefaultPluginUid( TUint32 aUid )
	{
	iDefaultPluginUid = aUid;	
	}

TUint32 CEmTubePluginManager::TemporaryPluginUid()
	{
	return iTemporaryPluginUid;
	}

void CEmTubePluginManager::SetTemporaryPluginUid( TUint32 aUid )
	{
	iTemporaryPluginUid = aUid;	
	}

TInt CEmTubePluginManager::SelectPlugin( TUint32 aUid )
	{
	TInt idx = iPluginUids.Find( aUid );
	if( idx != KErrNotFound )
		SelectPlugin( idx );

	return idx;
	}

TInt CEmTubePluginManager::FindPlugin( const TDesC& aUrl )
	{
	for( TInt i=0;i<iPlugins.Count(); i++ )
		{
		if( iPlugins[i]->CanHandleUrlL( aUrl ) )
			{
			return i;
			}
		}
	return KErrNotFound;
	}

TInt CEmTubePluginManager::SelectPlugin( const TDesC& aUrl )
	{
	for( TInt i=0;i<iPlugins.Count(); i++ )
		{
		if( iPlugins[i]->CanHandleUrlL( aUrl ) )
			{
			SelectPlugin( i );
			return i;
			}
		}
	return KErrNotFound;
	}

void CEmTubePluginManager::SelectPlugin( TInt aIndex )
	{
	iSelectedPluginId = aIndex;
	}

CPluginInterface* CEmTubePluginManager::Plugin()
	{
	return iPlugins[ iSelectedPluginId ];
	}

TInt CEmTubePluginManager::FindCredentials( TUint32 aUid )
	{
	for( TInt i=0;i<iPluginCredentials.Count();i++ )
		{
		if( iPluginCredentials[ i ]->Uid() == aUid )
			return i;
		}
	return KErrNotFound;
	}

const TDesC& CEmTubePluginManager::Username()
	{
	TInt idx = FindCredentials( iPluginUids[ iSelectedPluginId ] );
	return iPluginCredentials[ idx ]->Username();
	}

const TDesC& CEmTubePluginManager::Password()
	{
	TInt idx = FindCredentials( iPluginUids[ iSelectedPluginId ] );
	return iPluginCredentials[ idx ]->Password();
	}

void CEmTubePluginManager::ClearCookies()
	{
	ClearCookies( iPluginUids[ iSelectedPluginId ] );
	}

RPointerArray<CEmTubeCookie>& CEmTubePluginManager::Cookies()
	{
	return Cookies( iPluginUids[ iSelectedPluginId ] );
	}

void CEmTubePluginManager::ClearCookies( TUint32 aUid )
	{
	TInt idx = FindCredentials( aUid );
	iPluginCredentials[ idx ]->ClearCookies();
	}

RPointerArray<CEmTubeCookie>& CEmTubePluginManager::Cookies( TUint32 aUid )
	{
	TInt idx = FindCredentials( aUid );
	return iPluginCredentials[ idx ]->Cookies();
	}

const TDesC& CEmTubePluginManager::Username( TUint32 aUid )
	{
	TInt idx = FindCredentials( aUid );
	return iPluginCredentials[ idx ]->Username();
	}

const TDesC& CEmTubePluginManager::Password( TUint32 aUid )
	{
	TInt idx = FindCredentials( aUid );
	return iPluginCredentials[ idx ]->Password();
	}

void CEmTubePluginManager::SetUsernameL( const TDesC& aUsername )
	{
	SetUsernameL( iPluginUids[ iSelectedPluginId ], aUsername );
	}

void CEmTubePluginManager::SetPasswordL( const TDesC& aPassword )
	{
	SetPasswordL( iPluginUids[ iSelectedPluginId ], aPassword );
	}

void CEmTubePluginManager::AddCookieL( const TDesC8& aName, const TDesC8& aValue )
	{
	AddCookieL( iPluginUids[ iSelectedPluginId ], aName, aValue );
	}

void CEmTubePluginManager::SetUsernameL( TUint32 aUid, const TDesC& aUsername )
	{
	TInt idx = FindCredentials( aUid );
	if( idx != KErrNotFound )
		{
		iPluginCredentials[ idx ]->SetUsernameL( aUsername );
		}
	else
		{
		CEmTubePluginCredentials *c = CEmTubePluginCredentials::NewL();
		iPluginCredentials.Append( c );
		c->SetUsernameL( aUsername );
		c->SetUid( aUid );
		}
	ExportCredentialsL();
	}

void CEmTubePluginManager::SetPasswordL( TUint32 aUid, const TDesC& aPassword )
	{
	TInt idx = FindCredentials( aUid );
	if( idx != KErrNotFound )
		{
		iPluginCredentials[ idx ]->SetPasswordL( aPassword );
		}
	else
		{
		CEmTubePluginCredentials *c = CEmTubePluginCredentials::NewL();
		iPluginCredentials.Append( c );
		c->SetPasswordL( aPassword );
		c->SetUid( aUid );
		}
	ExportCredentialsL();
	}

void CEmTubePluginManager::AddCookieL( TUint32 aUid, const TDesC8& aName, const TDesC8& aValue )
	{
	TInt idx = FindCredentials( aUid );
	if( idx != KErrNotFound )
		{
		iPluginCredentials[ idx ]->AddCookieL( aName, aValue );
		}
	else
		{
		CEmTubePluginCredentials *c = CEmTubePluginCredentials::NewL();
		iPluginCredentials.Append( c );
		c->AddCookieL( aName, aValue );
		c->SetUid( aUid );
		}
	ExportCredentialsL();
	}

TBool CEmTubePluginManager::LoggedIn()
	{
	return LoggedIn( Uid() );
	}

TBool CEmTubePluginManager::LoggedIn( TUint32 aUid )
	{
	TInt idx = FindCredentials( aUid );
	return iPluginCredentials[ idx ]->Cookies().Count() ? (TBool)ETrue : (TBool)EFalse;
	}

void CEmTubePluginManager::ResetPluginStates()
	{
	iPluginState.Reset();

	for( TInt i=0;i<iPlugins.Count(); i++ )
		iPluginState.Append( ESearchNotStarted );
	}

TSearchState CEmTubePluginManager::SearchState( TInt aWhich )
	{
	if( aWhich == -1 )
		aWhich = iSelectedPluginId;

	return iPluginState[aWhich];
	}

void CEmTubePluginManager::SetSearchState( TSearchState aState, TInt aWhich )
	{
	if( aWhich == -1 )
		aWhich = iSelectedPluginId;
		
	iPluginState.Remove( aWhich );
	iPluginState.Insert( aState, aWhich );		
	}

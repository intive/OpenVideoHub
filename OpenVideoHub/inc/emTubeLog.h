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

#ifndef EMTUBE_LOG_H
#define EMTUBE_LOG_H

#ifdef ENABLE_LOG

#include <e32base.h>
#include <f32file.h>
#include <bautils.h>

_LIT( EMTVLog, "C:\\data\\emTube.txt" );
_LIT8( KEnter, "\r\n" );

inline void Log( const TDesC8& aDes )
{
	RFs fs;
	fs.Connect();
	RFile file;
	TInt err = file.Open( fs, EMTVLog, EFileWrite );

	if( err == KErrPathNotFound )
		{

		fs.MkDirAll(EMTVLog);
		err = file.Open( fs, EMTVLog, EFileWrite );

		}

	if ( err == KErrNotFound )
	{
		err = file.Replace( fs, EMTVLog, EFileWrite );
	}

	if ( err == KErrNone )
	{
		TInt pos( 0 );
		file.Seek( ESeekEnd, pos );
		file.Write( aDes );
		file.Write( KEnter() );
		file.Flush();
	}
	file.Close();
	fs.Close();
}

inline void Log( const TDesC16& aDes )
{
	TBufC8<1024> buffer;
	if(aDes.Length() > 1024 )
		Log( _L("TO LONG VALUE TO LOG!!!") );
	else
		{
		buffer.Des().Copy( aDes );
		Log( buffer );
		}
}

inline void Log( const char* aDes )
{
	TBufC8<1024> buffer;
	buffer.Des().Copy( (const TUint8* )aDes );
	Log( buffer );
}

inline void Log( const TInt aIntNum )
{
	TBuf8<14> num;
	num.AppendNum( aIntNum );
	Log( num );
}
#else
#define Log( a )
#endif

#endif //EMTUBE_LOG_H

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

#include <fbs.h>

#include "emTubeFileScanner.h"
#include "emTubeVideoEntry.h"


CEmTubeFileScanner* CEmTubeFileScanner::NewL()
	{
	CEmTubeFileScanner* self = NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeFileScanner* CEmTubeFileScanner::NewLC()
	{
	CEmTubeFileScanner* self = new (ELeave) CEmTubeFileScanner();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeFileScanner::CEmTubeFileScanner()
	{
	}

void CEmTubeFileScanner::ConstructL()
	{
	User::LeaveIfError( iFs.Connect() );
	}

CEmTubeFileScanner::~CEmTubeFileScanner()
	{
	iFs.Close();
	}

TInt CEmTubeFileScanner::Find( const TDesC& aName, RPointerArray<CVideoEntry>& aEntries )
	{
	for( TInt i=0; i<aEntries.Count(); i++ )
		{
		CVideoEntry* f = aEntries[i];
		if( !aName.Compare( f->SavedFileName() ) )
			{
			return i;
			}
		}
	return KErrNotFound;
	}

void CEmTubeFileScanner::ScanDirectoryL( const TDesC& aDir, RPointerArray<CVideoEntry>& aEntries )
	{
	TInt i=0;
	CDir* dirList;
	CDir* fileList;

	TInt err = iFs.GetDir(aDir, KEntryAttNormal, ESortByDate|EDescending,fileList,dirList);
	if( err == KErrNone )
		{
		CleanupStack::PushL(fileList);
		CleanupStack::PushL(dirList);
	
		while ( i < fileList->Count() )
			{
			TBuf<4> ext;
			ext.Copy( (*fileList)[i].iName.Right(4) );
			ext.LowerCase();

			if ( !ext.Compare( _L(".flv") ) )
				{
				if ( (*fileList)[i].iName.Compare( KVideoTmpName() ) )
					{
					TFileName file;
					file.Copy(aDir);
					file.Append((*fileList)[i].iName);

					if( Find( file, aEntries ) == KErrNotFound )
						{
						CVideoEntry *e = CVideoEntry::NewLC();
						TParsePtrC parse( file );
							
						e->SetSavedFileName( file );
						e->SetVideoFileSize( (*fileList)[i].iSize );
						e->SetDownloadFinished( ETrue );
						e->SetTitleL( parse.Name() );
						e->SetAuthorNameL( KNullDesC() );
						aEntries.Append( e );

						CleanupStack::Pop( e );
						}
					}
				}
			i++;
			}
	
		TInt j=0;
		while ( j<dirList->Count() )
			{
			TFileName newPath;
			newPath.Copy(aDir);
			newPath.Append((*dirList)[j].iName);
			newPath.Append( _L("\\") );
			ScanDirectoryL( newPath, aEntries );
			j++;
			}

		CleanupStack::PopAndDestroy(dirList);
		CleanupStack::PopAndDestroy(fileList);
		}
	}

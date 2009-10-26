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

#ifndef EMTUBE_FILE_SCANNER_H
#define EMTUBE_FILE_SCANNER_H

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

class CVideoEntry;

class CEmTubeFileScanner : public CBase
{
public:
	static CEmTubeFileScanner* NewL();
	static CEmTubeFileScanner* NewLC();
	~CEmTubeFileScanner( );

public:
	void ScanDirectoryL( const TDesC& aDir, RPointerArray<CVideoEntry>& aEntries );
	static TInt Find( const TDesC& aName, RPointerArray<CVideoEntry>& aEntries );

private: // internal methods
	CEmTubeFileScanner();
	void ConstructL();

private:
	RFs iFs;
};

#endif //EMTUBE_FILE_SCANNER_H

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

#ifndef EMTUBE_DOCUMENT_H
#define EMTUBE_DOCUMENT_H

#ifdef EMTUBE_UIQ
#include <qikdocument.h>
#else
#include <akndoc.h>
#endif

class CEmTubeAppUi;

#ifdef EMTUBE_UIQ
class CEmTubeDocument : public CQikDocument
#else
class CEmTubeDocument : public CAknDocument
#endif
	{
public:

	static CEmTubeDocument* NewL(CEikApplication& aApp);
	static CEmTubeDocument* NewLC(CEikApplication& aApp);
	~CEmTubeDocument();

public: // from CAknDocument

	CEikAppUi* CreateAppUiL();
	void OpenFileL(CFileStore*& aFileStore, RFile& aFile);
	CFileStore* OpenFileL(TBool aDoOpen, const TDesC& aFilename, RFs& aFs);

private:

	void ConstructL();
	CEmTubeDocument(CEikApplication& aApp);
	CEmTubeAppUi& AppUi();

	};


#endif // EMTUBE_DOCUMENT_H

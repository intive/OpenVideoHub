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

#include "emTubeAppUi.h"
#include "emTubeDocument.h"

#include <EIKENV.H>

#ifdef EMTUBE_UIQ
#include <QikApplication.h>
#endif

// Standard Symbian OS construction sequence
CEmTubeDocument* CEmTubeDocument::NewL(CEikApplication& aApp)
	{
	CEmTubeDocument* self = NewLC(aApp);
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeDocument* CEmTubeDocument::NewLC(CEikApplication& aApp)
	{
	CEmTubeDocument* self = new (ELeave) CEmTubeDocument(aApp);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CEmTubeDocument::ConstructL()
	{
	// no implementation required
	}	 

#ifndef EMTUBE_UIQ
CEmTubeDocument::CEmTubeDocument(CEikApplication& aApp) : CAknDocument(aApp)
#else
CEmTubeDocument::CEmTubeDocument(CEikApplication& aApp) : CQikDocument( aApp )
#endif
	{
	// no implementation required
	}

CEmTubeDocument::~CEmTubeDocument()
	{
	// no implementation required
	}

CEikAppUi* CEmTubeDocument::CreateAppUiL()
	{
	// Create the application user interface, and return a pointer to it,
	// the framework takes ownership of this object
	CEikAppUi* appUi = new (ELeave) CEmTubeAppUi;
	return appUi;
	}

CEmTubeAppUi& CEmTubeDocument::AppUi()
	{
	return *(static_cast<CEmTubeAppUi*>(CEikonEnv::Static()->EikAppUi()));
	}

CFileStore* CEmTubeDocument::OpenFileL(TBool /*aDoOpen*/, const TDesC& /*aFilename*/, RFs& /*aFs*/)
	{
/*	Log("document: open file by name");
	Log(aFilename);
	AppUi().SetEmbedded( ETrue );
	AppUi().OpenFileL( aFilename );
*/
	return NULL;
	}

void CEmTubeDocument::OpenFileL(CFileStore*& /*aFileStore*/, RFile& aFile)
	{
//	Log("document: open file by RFile");
//	TFileName fileName;
//	User::LeaveIfError( aFile.FullName(fileName) );
//	Log(fileName);

	AppUi().SetEmbedded( ETrue );
	AppUi().OpenFileL( aFile );
	}

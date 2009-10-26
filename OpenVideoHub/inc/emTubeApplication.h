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

#ifndef EMTUBE_APPLICATION_H
#define EMTUBE_APPLICATION_H

#ifdef EMTUBE_UIQ
#include <QikApplication.h>
#else
#include <aknapp.h>
#endif

// UID for the application, this should correspond to the uid defined in the mmp file
static const TUid KUidEmTubeApp = {0x2001A7DC};

/**
* CEmTubeApplication
*	 An instance of CEmTubeApplication is the application part of the AVKON
*	 application framework for the emTube example application
*/
#ifndef EMTUBE_UIQ
class CEmTubeApplication : public CAknApplication
#else
class CEmTubeApplication : public CQikApplication
#endif
	{

public:  // from CAknApplication

	/**
	* AppDllUid
	* @return the UID of this Application/Dll
	*/
	TUid AppDllUid() const;

protected: // from CAknApplication

	/**
	* CreateDocumentL
	* @return a pointer to the created CApaDocument object
	*/
	CApaDocument* CreateDocumentL();
	};

#endif // EMTUBE_APPLICATION_H

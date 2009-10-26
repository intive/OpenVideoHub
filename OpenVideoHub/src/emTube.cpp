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

#ifdef __SERIES60_3X__
#include 	<eikstart.h>
#endif

#include "emTubeApplication.h"

#ifdef __UI_FRAMEWORKS_V2__

// Create an application, and return a pointer to it
CApaApplication* NewApplication()
	{
	return new CEmTubeApplication;
	}

TInt E32Main()
	{
	// FIX: prevent dummy TRAP/LEAVE leak on EKA2
#if defined(__WINS__) && defined(EKA2)
	TRAPD(Err, User::Leave(1));
#endif // defined(__WINS__) && defined(EKA2)
	return EikStart::RunApplication(NewApplication);
	
	}

///////////////////////////////////////////////////////////////////////////////
//
// The following is required for wins on EKA1 (using the exedll target)
//
#if defined(__WINS__) && !defined(EKA2)
EXPORT_C TInt WinsMain(TDesC* aCmdLine)
	{
	return EikStart::RunApplication(NewApplication, aCmdLine);
	}

TInt E32Dll(TDllReason)
	{
	return KErrNone;
	}
#endif

#else // __UI_FRAMEWORKS_V2__

// Create an application, and return a pointer to it
EXPORT_C CApaApplication* NewApplication()
  {
  return new CEmTubeApplication;
  }

// DLL entry point, return that everything is ok
GLDEF_C TInt E32Dll(TDllReason)
  {
  return KErrNone;
  }

#endif // __UI_FRAMEWORKS_V2__


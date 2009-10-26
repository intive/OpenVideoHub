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

#ifndef EMTUBE_SERVERSVIEW_H
#define EMTUBE_SERVERSVIEW_H

#include <aknview.h>

class CEmTubeServersViewContainer;
class CEmTubeAppUi;
class CVideoEntry;

class CEmTubeServersView : public CAknView
	{
	
public: // construction / destruction
	static CEmTubeServersView* NewL();
	static CEmTubeServersView* NewLC(); 	   
	~CEmTubeServersView();
	
private: // construction
	CEmTubeServersView();
	void ConstructL();
	
public:

	TUid Id() const;
	void HandleCommandL( TInt aCommand );
	void DoActivateL(const TVwsViewId& aPrevViewId,  
				TUid aCustomMessageId,	
				const TDesC8& aCustomMessage);
	
	void DoDeactivate();

	void UpdateListL();
	
protected: // from CAknView
	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);		 

private: // data
	CEmTubeServersViewContainer* iContainer;
	CEmTubeAppUi* iAppUi;

	};

#endif // EMTUBE_SERVERSVIEW_H

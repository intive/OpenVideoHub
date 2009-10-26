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

#ifndef EMTUBE_MAINVIEW_H
#define EMTUBE_MAINVIEW_H

#include <aknview.h>

class CEmTubeMainViewContainer;
class CEmTubeAppUi;
class CVideoEntry;

class CEmTubeMainView : public CAknView
	{
	
public: // construction / destruction
	static CEmTubeMainView* NewL();
	static CEmTubeMainView* NewLC(); 	   
	~CEmTubeMainView();
	
private: // construction
	CEmTubeMainView();
	void ConstructL();
	
public:
	TUid Id() const;
	void HandleCommandL( TInt aCommand );
	void DoActivateL(const TVwsViewId& aPrevViewId,  
				TUid aCustomMessageId,	
				const TDesC8& aCustomMessage);
	
	void DoDeactivate();

public:		
	TInt CurrentMenuItem() { return iCurrentMenuItem; }
	void SetCurrentMenuItem( TInt aIndex ) { iCurrentMenuItem = aIndex; }

protected: // from CAknView
	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);		 

private: // data
	CEmTubeMainViewContainer* iContainer;
	CEmTubeAppUi* iAppUi;
	
    TInt iCurrentMenuItem;

	};

#endif // EMTUBE_MAINVIEW_H


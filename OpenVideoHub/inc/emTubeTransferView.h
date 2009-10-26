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

#ifndef EMTUBE_TRANSFERVIEW_H
#define EMTUBE_TRANSFERVIEW_H

#include <aknview.h>

class CEmTubeTransferViewContainer;
class CEmTubeAppUi;
class CAknTitlePane;

class CEmTubeTransferView : public CAknView
	{
	
public: // construction / destruction
	static CEmTubeTransferView* NewL();
	static CEmTubeTransferView* NewLC(); 	   
	~CEmTubeTransferView();
	
private: // construction
	CEmTubeTransferView();
	void ConstructL();
	
public:

	TUid Id() const;
	void HandleCommandL( TInt aCommand );
	void DoActivateL(const TVwsViewId& aPrevViewId,  
				TUid aCustomMessageId,	
				const TDesC8& aCustomMessage);
	
	void DoDeactivate();
	
protected: // from CAknView
	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);		 

private: // data
	CEmTubeTransferViewContainer* iContainer;
	CEmTubeAppUi* iAppUi;
	};

#endif // EMTUBE_TRANSFERVIEW_H


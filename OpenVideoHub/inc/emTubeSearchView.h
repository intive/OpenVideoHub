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

#ifndef EMTUBE_SEARCHVIEW_H
#define EMTUBE_SEARCHVIEW_H

#include <aknview.h>
#include "emTubeProgress.h"

#ifdef __S60_50__
#include <AknToolbarObserver.h>
#endif

class CEmTubeSearchViewContainer;
class CEmTubeAppUi;
class CVideoEntry;
class CObexFileObject;
class CEmTubePlaylistManager;

class TCurrentItem
	{
public:
	TUint32 iPluginUid;
	TInt iCurrentItemIndex;
	};

class CEmTubeSearchView : public CAknView

#ifdef __S60_50__
					,public MAknToolbarObserver
#endif

{

public: // construction / destruction
	static CEmTubeSearchView* NewL( CEmTubePlaylistManager* aManager );
	static CEmTubeSearchView* NewLC( CEmTubePlaylistManager* aManager );
	~CEmTubeSearchView();

private: // construction
	CEmTubeSearchView( CEmTubePlaylistManager* aManager );
	void ConstructL();

public:
	void StartProgressBarL( TBool aClear );
	TInt CurrentItemIndex();
	void SetCurrentItemIndex( TInt aCurrent );
	TInt FindCurrentItem( TUint32 aUid );
	void ResetCurrentItems();

public:

	TUid Id() const;
	void HandleCommandL( TInt aCommand );
	void DoActivateL(const TVwsViewId& aPrevViewId,
				TUid aCustomMessageId,
				const TDesC8& aCustomMessage);

	void DoDeactivate();

protected: // from CAknView
	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);

#ifdef __S60_50__
public: // from MAknToolbarObserver
	void  DynInitToolbarL(TInt aResourceId, CAknToolbar *aToolbar);
	void  OfferToolbarEventL(TInt aCommand);
#endif

public:
	void DisplaySearchResultsL();
	void ImageLoadedL( TInt aIndex );
	void SetCbaL( TInt aCbaResource );
	void CancelProgressL();

	TInt CurrentToActual();

private: // data
	CEmTubeSearchViewContainer* iContainer;
	CEmTubeAppUi* iAppUi;

	TInt iProgressCompleteSize;
	RArray<TCurrentItem> iCurrentItems;

	CObexFileObject* iObexFileObject;
	CEmTubePlaylistManager* iManager;
	
	HBufC* iFindBoxText;
	};

#endif // EMTUBE_SEARCHVIEW_H


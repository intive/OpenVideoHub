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

#ifndef EMTUBE_TRANSFERVIEW_CONTAINER_H
#define EMTUBE_TRANSFERVIEW_CONTAINER_H

#include <coecntrl.h>
#include <aknlists.h> // list
#include <e32std.h>

#include "emTubeTransferView.h"

class CEmTubeAppUi;
class CEmTubeTimeOutTimer;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CEmTubeTransferManager;

class CEmTubeTransferViewContainer : public CCoeControl, MEikListBoxObserver, public MTimeOutObserver
	{

public: // construction / destruction
	static CEmTubeTransferViewContainer* NewL(CEmTubeTransferView& aView, const TRect& aRect);
	static CEmTubeTransferViewContainer* NewLC(CEmTubeTransferView& aView, const TRect& aRect);
	~CEmTubeTransferViewContainer();
   
private: // construction
	CEmTubeTransferViewContainer(CEmTubeTransferView& aView);
	void ConstructL(const TRect& aRect);
	void SizeChanged();

	void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

	void UpdateListL( TInt aIdx );
	void CreateLineL( RBuf& aBuf, CQueueEntry* aEntry );
	void AddStatusTextL( RBuf& aBuf, TInt aResource, TInt aAdditionalLength = 0 );

protected: // from CCoeControl

	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void HandleResourceChange(TInt aType);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	void GetHelpContext( TCoeHelpContext& aContext ) const;

public:  // from CCoeControl
	void Draw(const TRect& aRect) const;

public: //from MTimeOutObserver
	void TimerExpired( TInt aMode );

public:
    void RemoveEntryL();

    void StartTransferManagerL();
    void StopTransferManagerL();
    
    void RemoveFinishedEntriesL();

    void MoveUpInQueueL( );
    void MoveDownInQueueL( );

    TInt CurrentItemIndex();    

    void MarkEntryAsQueued();
    
private: // data
	CEmTubeTransferView& iView;
	CAknDoubleGraphicStyleListBox *iListBox;
	CEmTubeAppUi* iAppUi;
	CEmTubeTimeOutTimer* iTimer;
    CEmTubeTransferManager* iTransferManager;	

//navipane
	CAknNavigationControlContainer* iNaviPane;
	CAknNavigationDecorator* iNaviLabelDecorator;
	
	CQueueEntry* iEntry;
	};

#endif // EMTUBE_TRANSFERVIEW_CONTAINER_H

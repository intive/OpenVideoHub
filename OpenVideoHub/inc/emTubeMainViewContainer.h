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

#ifndef EMTUBE_MAINVIEW_CONTAINER_H
#define EMTUBE_MAINVIEW_CONTAINER_H

#include <coecntrl.h>
#include <aknlists.h> // list
#include <e32std.h>

#ifdef __S60_50__
#include <aknstyluspopupmenu.h>
#endif

#include <PluginInterface.h>

#include "emTubeMainView.h"
#include "emTubeMUiInterface.h"
#include "emTubeMItemDrawer.h"

#include "emTubeTimeOutTimer.h"

class CEmTubeAppUi;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;
class CAknStylusPopUpMenu;

class CEmTubeMainViewContainer : public CCoeControl, MHandleCommandObserver, MItemDrawer,
								 public MTimeOutObserver
#ifdef __S60_50__
								,public MEikMenuObserver
#endif
	{
public:
	enum TCommandId
		{
		ECommandSearch = 0,
		ECommandSavedClips,
		ECommandFavorites,
		ECommandNewClips,
		ECommandFeaturedClips,
		ECommandTopRatedClips,
		ECommandMostViewedClips,
		ECommandTransferManager,
		ECommandUploadVideo,
		ECommandPlaylists
		};

	enum TTimerMode
		{
		ETimerModeTransferManager = 0,
		ETimerModeIconAnimation
		};
	
public: // construction / destruction
	static CEmTubeMainViewContainer* NewL(CEmTubeMainView& aView, const TRect& aRect);
	static CEmTubeMainViewContainer* NewLC(CEmTubeMainView& aView, const TRect& aRect);
	~CEmTubeMainViewContainer();

private: // construction
	CEmTubeMainViewContainer(CEmTubeMainView& aView);
	void ConstructL(const TRect& aRect);
	void SizeChanged();

	void ReplaceCurrentItemIconL();

protected: // from CCoeControl
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void HandleResourceChange(TInt aType);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
	void GetHelpContext( TCoeHelpContext& aContext ) const;
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);

public:  // from CCoeControl
	void Draw(const TRect& aRect) const;

public:  // from MTimeOutObserver
	void TimerExpired( TInt aMode );

public: // from MHandleCommandObserver
    void HandleCommandL( TObserverEvent aEvent );

public: // from MItemDrawer
    TItemState DrawItemL( CFbsBitmap* aDstBitmap, CFbsBitGc* aDstBitmapGc, TSize aNewSize,
                          TPoint aDstPoint, TBool aShowText, TBool aSelected, CUiItem* aItem,
						  TBool aScrollText = EFalse );

#ifdef __S60_50__

public: //from MEikMenuObserver
    void ProcessCommandL( TInt aCommandId );
    void SetEmphasis( CCoeControl* aMenuControl,TBool aEmphasis );

#endif

public:
    TInt SelectTimePeriodL( TPeriod& aPeriod );
    void FloatingTimePeriodDialogL( TPoint aWhere );

public:
    TInt CurrentItemIndex();

	TBool IsCapabilitySupported( TInt aCapabilty );

private: // data
	CEmTubeMainView& iView;
	CEmTubeAppUi* iAppUi;

	RArray<TCommandId> iCommands;
	RArray<TInt> iIconIndices;
	RArray<TInt> iCommandStrings;

//navipane
	CAknNavigationControlContainer* iNaviPane;
	CAknNavigationDecorator* iNaviLabelDecorator;

	MUiInterface* iUi;
	CFont* iCustomUiFont;
	TInt iScrollAmount;

	CEmTubeTimeOutTimer* iTimer;
	CEmTubeTimeOutTimer* iIconTimer;

	CAknStylusPopUpMenu* iPopup;
	TInt iPreviousSelectedItem;
	TInt iCurrentFrame;
	};

#endif // EMTUBE_MAINVIEW_CONTAINER_H

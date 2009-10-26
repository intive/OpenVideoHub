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

#ifndef EMTUBE_SPLASHVIEW_CONTAINER_H
#define EMTUBE_SPLASHVIEW_CONTAINER_H

#include <coecntrl.h>
#include <aknlists.h> // list
#include <e32std.h>

#include "emTubeSplashView.h"
#include "emTubeTimeOutTimer.h"

class CEmTubeAppUi;

class CEmTubeSplashViewContainer : public CCoeControl, public MTimeOutObserver
	{
public:
	enum TTimerCommand
		{
		ECommandFadeIn = 1,
		ECommandFadeOut,
		ECommandFinish
		};

public: // construction / destruction
	static CEmTubeSplashViewContainer* NewL(CEmTubeSplashView& aView, const TRect& aRect);
	static CEmTubeSplashViewContainer* NewLC(CEmTubeSplashView& aView, const TRect& aRect);
	~CEmTubeSplashViewContainer();

private: // construction
	CEmTubeSplashViewContainer(CEmTubeSplashView& aView);
	void ConstructL(const TRect& aRect);
	void SizeChanged();

public: //from MTimeOutObserver
	void TimerExpired( TInt aMode );

protected: // from CCoeControl
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void HandleResourceChange(TInt aType);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
#ifdef __S60_50__
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
#endif
	void GetHelpContext( TCoeHelpContext& aContext ) const;

public:  // from CCoeControl
	void Draw(const TRect& aRect) const;

private: // data
	CEmTubeSplashView& iView;
	CEmTubeAppUi* iAppUi;
    CFbsBitmap *iBitmap;
    CFbsBitmap *iTmpBitmap;

	CEmTubeTimeOutTimer* iTimer;
	CEmTubeTimeOutTimer* iFadeTimer;

	TRect iClientRect;
	TInt iAlpha;
	};

#endif // EMTUBE_SPLASHVIEW_CONTAINER_H

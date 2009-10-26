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

#ifndef EMTUBE_SERVERSVIEW_CONTAINER_H
#define EMTUBE_SERVERSVIEW_CONTAINER_H

#include <coecntrl.h>
#include <eiklbo.h>
#include <akngrid.h>
#include <aknsbasicbackgroundcontrolcontext.h> 
#include <aknlists.h> // list

class CEmTubeServersView;

class CEmTubeServersViewContainer : public CCoeControl, MEikListBoxObserver
	{
	public:
		static CEmTubeServersViewContainer* NewL( CEmTubeServersView& aView, const TRect& aRect );
		static CEmTubeServersViewContainer* NewLC( CEmTubeServersView& aView, const TRect& aRect );
		~CEmTubeServersViewContainer();

	public:
		void SelectPluginL();
		TInt CurrentItemIndex();
		TBool IsCurrentItemMore();
		void FillListL( TUint32 aUid );

	private: //from MEikListBoxObserver
		void GetHelpContext( TCoeHelpContext& aContext ) const;

		void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aListBoxEvent );
		CEmTubeServersViewContainer( CEmTubeServersView& aView );
		void ConstructL( const TRect& aRect );
		void SizeChanged();
		void Draw( const TRect& aRect ) const;
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl( TInt aIndex ) const;
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
		void HandleResourceChange( TInt aType );

	private:
		CAknDoubleLargeStyleListBox *iListBox;
		CEmTubeAppUi* iAppUi;
		CEmTubeServersView& iView;
	};

#endif //EMTUBE_SERVERSVIEW_CONTAINER_H

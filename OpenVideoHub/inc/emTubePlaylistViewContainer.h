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

#ifndef EMTUBE_PLAYLISTVIEW_CONTAINER_H
#define EMTUBE_PLAYLISTVIEW_CONTAINER_H

#include <coecntrl.h>
#include <eiklbo.h>
#include <akngrid.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <aknlists.h> // list

class CEmTubePlaylistView;
class CAknNavigationControlContainer;
class CAknNavigationDecorator;

class CEmTubePlaylistViewContainer : public CCoeControl, MEikListBoxObserver
	{
	public:
		enum TMode
			{
			EModeNormal = 0,
			EModeEdit,
			EModeDrag
			};

	public:
		static CEmTubePlaylistViewContainer* NewL( CEmTubePlaylistView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager );
		static CEmTubePlaylistViewContainer* NewLC( CEmTubePlaylistView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager );
		~CEmTubePlaylistViewContainer();

	private: //from MEikListBoxObserver
		void GetHelpContext( TCoeHelpContext& aContext ) const;

		CEmTubePlaylistViewContainer( CEmTubePlaylistView& aView, CEmTubePlaylistManager* aManager );

		void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aListBoxEvent );
		void ConstructL( const TRect& aRect );
		void SizeChanged();
		void Draw( const TRect& aRect ) const;
		TInt CountComponentControls() const;
		CCoeControl* ComponentControl( TInt aIndex ) const;
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
		void HandleResourceChange( TInt aType );

	public:
		void FillListL( TInt aIndex = 0 );
		void SetMode( CEmTubePlaylistViewContainer::TMode aMode ) {iMode = aMode; }
		TInt CurrentItemIndex() {return iListBox->CurrentItemIndex(); }

	private:
		void SetNaviPaneTextL( TInt aResource );
		void SetNaviPaneTextL( const TDesC& aTxt );

	private:
		CAknSingleGraphicStyleListBox *iListBox;
		CEmTubeAppUi* iAppUi;
		CEmTubePlaylistView& iView;
		CEmTubePlaylistManager *iManager;

		TMode iMode;
//navipane
		CAknNavigationControlContainer* iNaviPane;
		CAknNavigationDecorator* iNaviLabelDecorator;
	};

#endif //EMTUBE_PLAYLISTVIEW_CONTAINER_H

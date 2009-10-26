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

#ifndef EMTUBE_SETTINGSVIEW_CONTAINER_H
#define EMTUBE_SETTINGSVIEW_CONTAINER_H

#include <coecntrl.h>
#include <eiklbo.h>

class CEmTubeSettingsView;
class CEmTubeSettingsListBox;
class CEmTubeSettingsItemData;

class CEmTubeSettingsViewContainer : public CCoeControl
{
public: // Constructors and destructor
	static CEmTubeSettingsViewContainer* NewL( CEmTubeSettingsView& aView, const TRect& aRect );
	static CEmTubeSettingsViewContainer* NewLC( CEmTubeSettingsView& aView, const TRect& aRect );
	~CEmTubeSettingsViewContainer();

public:
	void EditItemL();
	void SaveSettingsL();

private: // construction
	CEmTubeSettingsViewContainer( CEmTubeSettingsView& aView );
	void ConstructL( const TRect& aRect );

private: // From CCoeControl
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType );
	void SizeChanged();
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void Draw( const TRect& aRect ) const;
	void GetHelpContext( TCoeHelpContext& aContext ) const;

private: // Data
	CEmTubeAppUi* iAppUi;

	CEmTubeSettingsView& iView;
	CEmTubeSettingsListBox* iListBox;
	CEmTubeSettingsData* iData;
};

#endif //EMTUBE_SETTINGSVIEW_CONTAINER_H

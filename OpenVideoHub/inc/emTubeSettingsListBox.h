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

#ifndef EMTUBE_SETTINGSVIEW_LISTBOX_H
#define EMTUBE_SETTINGSVIEW_LISTBOX_H

#include <AknSettingItemList.h>

class CEmTubeSettingsData;

class CEmTubeSettingsListBox : public CAknSettingItemList
{
public:
	CAknSettingItem* CreateSettingItemL( TInt identifier );
	void StoreSettingsL();
	void EditItemL( TInt aIndex, TBool aCalledFromMenu );
	void SetData(CEmTubeSettingsData* aData);

private:
	void SizeChanged();

private:
	CEmTubeSettingsData* iData;

	CAknSettingItem* settingItemVolume;
	CAknSettingItem* settingItemMaxResults;
	CAknSettingItem* settingItemCacheSize;
	CAknSettingItem* settingItemStartPlayback;
	CAknSettingItem* settingItemSortResultsBy;
	CAknSettingItem* settingItemAccessPoint;
	CAknSettingItem* settingItemAutoRotate;
	CAknSettingItem* settingItemTempMemory;
	CAknSettingItem* settingItemUsername;
	CAknSettingItem* settingItemPassword;
	CAknSettingItem* settingItemUpscaleVideo;
	CAknSettingItem* settingItemPlus18Content;
	CAknSettingItem* settingItemS60Ui;

};

#endif // EMTUBE_SETTINGSVIEW_LISTBOX_H

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

#ifndef EMTUBE_SETTINGS_DATA_H
#define EMTUBE_SETTINGS_DATA_H

#include <e32base.h>

#include <aknsettingitemlist.h>

class CEmTubeSettingsData : public CBase
{
public:
	static CEmTubeSettingsData* NewL();
	~CEmTubeSettingsData();

private:
	CEmTubeSettingsData();
	void ConstructL();

public:
	TInt iVolume;
	TInt iStartPlaybackMode;
	TInt iMaxResults;
	TInt iCacheSize;
	TBool iAutoRotate;
	TInt iTempMemory;
	TInt iSortResultsBy;

	TBuf<128> iUsername;
	TBuf<128> iPassword;

	TInt iVideoScaleMode;

	TInt iAccessPoint;

	TBool iS60Ui;
};

#endif // EMTUBE_SETTINGS_ITEMDATA_H

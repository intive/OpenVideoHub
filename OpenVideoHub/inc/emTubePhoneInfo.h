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

#ifndef EMTUBE_PHONE_INFO_H
#define EMTUBE_PHONE_INFO_H

#include <Etel3rdParty.h>

class CEmTubePhoneInfo : public CActive
{
public:
	static CEmTubePhoneInfo* NewL();
	static CEmTubePhoneInfo* NewLC();

	~CEmTubePhoneInfo();

public:

	const TPtrC GetImeiL();

private: 

	CEmTubePhoneInfo(); 

	void ConstructL();

private: //CActive
	void RunL();
	void DoCancel();

private:
	CTelephony* iTelephony;
	CActiveSchedulerWait iActiveSchedulerWait;
	TBuf<CTelephony::KPhoneSerialNumberSize> iImei;
	CTelephony::TPhoneIdV1 iPhoneId;
};

#endif // EMTUBE_PHONE_INFO_H

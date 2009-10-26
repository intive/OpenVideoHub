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

#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <e32base.h>

_LIT( KBoundary, "------------GI3ae0GI3KM7Ef1Ij5Ef1ei4GI3GI3" );
_LIT( KDisposition, "Content-Disposition: form-data; name=\"" );
_LIT( KCrLf, "\r\n" );

_LIT8( KContentTypeUrlEncoded, "application/x-www-form-urlencoded");
_LIT8( KContentTypeMultipart, "multipart/form-data; boundary=------------GI3ae0GI3KM7Ef1Ij5Ef1ei4GI3GI3");

class CPluginUtils
	{

public:
	static void AddUrlParamL( RBuf& aBuf, const TDesC& aParam, const TDesC& aValue, TBool aLast = EFalse);
	static void AddUrlParamL( RBuf& aBuf, const TDesC& aParam, TInt aValue, TBool aLast = EFalse );

	static void AddMultipartParamL( RBuf& aBuf, const TDesC& aParam, const TDesC& aValue );
	static void AddMultipartParamL( RBuf& aBuf, const TDesC& aParam, TInt aValue );

	static HBufC* ConvertUtfToUnicodeL( const TDesC8& aUtf7 );
	static HBufC* sConvertUtfToUnicodeL( const char *aUtf7 );
	};  

#endif //PLUGIN_UTILS_H


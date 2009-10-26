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

#ifndef EXAMPLE_PLUGIN_H
#define EXAMPLE_PLUGIN_H

#include <PluginInterface.h>

class CPlugin : public CPluginInterface
	{

public:
	static CPlugin* NewL();
	virtual ~CPlugin();

private:
	CPlugin();
	void ConstructL();

public:
	TPtrC16 Name();
	void IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask );

	TInt Capabilities();
	
	TBool CanHandleUrlL( const TDesC& aUrl );

	HBufC* FeatureUrlL( TFeature aFeature, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime );
	HBufC* UserVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );
	HBufC* RelatedVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );
	HBufC* CreateUrlFromIdL( const TDesC& aId );

	TInt SearchResultsPerPage();

	HBufC* SearchUrlL( const TDesC& aString, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );
	TInt ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler );

	HBufC* VideoUrlL( const TDesC8& aResponse, TBool& aLast );

	void LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword );
	TBool ParseLoginResponseL( TDesC8& aResponse );

	HBufC* UploadMovieUrlL();
	TInt ParseUploadMovieResponseL( const TDesC8& aResponse );
	TBool UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData );
	TInt ParseUploadMovieResponseStep1L( const TDesC8& aResponse );
	TBool UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData );
	TInt ParseUploadMovieResponseStep2L( const TDesC8& aResponse );

private:
	HBufC* ConvertCharToHBufCL( const char *aString );
	HBufC* ConvertCharToHBufCL( const unsigned char *aString, TInt aLength );
	HBufC* VideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime );

private:
	HBufC8* iToken;
	HBufC* iAddresser;
	HBufC* iActionUrl;

	};  

#endif //EXAMPLE_PLUGIN_H


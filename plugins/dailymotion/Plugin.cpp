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

#include <string.h>
#include <utf.h>
#include <akniconutils.h>
#include <EscapeUtils.h>

#include "pluginUtils.h"

#include <DailyMotionPlugin.mbg>
#include "Plugin.h"

CPlugin* CPlugin::NewL()
	{
	CPlugin* self=new(ELeave) CPlugin();
	CleanupStack::PushL(self);
	self->ConstructL(); 
	CleanupStack::Pop();
	return self;
	}

CPlugin::~CPlugin()
	{
	delete iActionUrl;
	}

CPlugin::CPlugin()
	{
	}

void CPlugin::ConstructL()
	{
	}

TInt CPlugin::Capabilities()
	{
	return KCapsAccessVideoById | KCapsRelatedClips | KCapsUserClips |
		KCapsFeatured | KCapsTopRated | KCapsRecentlyUploaded | KCapsMostViewed |
		KCapsConstResultsPerPage | KCapsLogin | KCapsVideoUpload |
		KCapsVideoDownload;
	}

TBool CPlugin::CanHandleUrlL( const TDesC& aUrl )
	{
	TBool ret = EFalse;

	RBuf url;
	CleanupClosePushL( url );

	url.Create( aUrl.Length() );
	url.CopyLC( aUrl );

	if( url.Find(_L("dailymotion.") ) != KErrNotFound )
		ret = ETrue;

	CleanupStack::PopAndDestroy( &url );
	return ret;
	}

_LIT( KDailyMotionUrl, "http://iphone.dailymotion.com/" );
_LIT( KDailyMotionMainUrl, "http://www.dailymotion.com/" );
_LIT( KDailyMotionVideo, "video/" );

HBufC* CPlugin::CreateUrlFromIdL( const TDesC& aId )
	{
	RBuf url;
	CleanupClosePushL( url );
	url.Create( KDailyMotionMainUrl().Length() + KDailyMotionVideo().Length() + aId.Length() );
	url.Copy( KDailyMotionMainUrl() );
	url.Append( KDailyMotionVideo() );
	url.Append( aId );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	
	return ret;
	}

HBufC* CPlugin::UserVideosUrlL( const TDesC& aBaseUrl, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy /*aOrderBy*/ )
	{
	RBuf url;
	CleanupClosePushL( url );

	url.Create( KDailyMotionUrl().Length() + aBaseUrl.Length() + 1 );
	url.Copy( KDailyMotionUrl() );
	url.Append( aBaseUrl );
	url.Append( _L("/") );

	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<16> num;
	num.Format( _L("%d"), page );
	url.ReAllocL( url.Length() + num.Length() );
	url.Append( num );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	
	return ret;
	}

_LIT( KDailyMotionVideoRelated, "related/" );

HBufC* CPlugin::RelatedVideosUrlL( const TDesC& aBaseUrl, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy /*aOrderBy*/ )
	{
	RBuf url;
	CleanupClosePushL( url );

	url.Create( KDailyMotionUrl().Length() + KDailyMotionVideoRelated().Length() + aBaseUrl.Length() + 1 );
	url.Copy( KDailyMotionUrl() );
	url.Append( KDailyMotionVideoRelated() );
	url.Append( aBaseUrl );
	url.Append( _L("/") );

	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<16> num;
	num.Format( _L("%d"), page );
	url.ReAllocL( url.Length() + num.Length() );
	url.Append( num );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	
	return ret;
	}

_LIT( KPluginName, "DailyMotion" );
TPtrC16 CPlugin::Name()
	{
	return KPluginName();
	}

void CPlugin::IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask )
	{
	CFbsBitmap* bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, _L("\\resource\\apps\\dailymotionplugin.mbm"), EMbmDailymotionpluginLogo, EMbmDailymotionpluginLogo_mask);
	aBitmap = bitmap;
	aMask = mask;
	}

TInt CPlugin::SearchResultsPerPage()
	{
	return 13;
	}

_LIT( KUrlFeatured, "featured/" );
_LIT( KUrlRated, "rated-week/" );
_LIT( KUrlViewed, "visited-week/" );

HBufC* CPlugin::FeatureUrlL( TFeature aFeature, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy /*aOrderBy*/, TPeriod /*aTime*/ )
	{
	RBuf url;
	CleanupClosePushL( url );

	url.CreateL( KDailyMotionUrl().Length() );
	url.Copy( KDailyMotionUrl() );

	switch( aFeature )
		{
			case EFeaturedClips:
				{
				url.ReAllocL( url.Length() + KUrlFeatured().Length() );
				url.Append( KUrlFeatured() );
				}
			break;

			case ETopRatedClips:
				{
				url.ReAllocL( url.Length() + KUrlRated().Length() );
				url.Append( KUrlRated() );
				}
			break;

			case EMostViewedClips:
				{
				url.ReAllocL( url.Length() + KUrlViewed().Length() );
				url.Append( KUrlViewed() );
				}
			break;

			case ENewClips:
			default:
			break;
		}

	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<16> num;
	num.Format( _L("%d"), page );
	url.ReAllocL( url.Length() + num.Length() );
	url.Append( num );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	
	return ret;
	}

_LIT( KUrlViewCount, "http://iphone.dailymotion.com/visited/search/" );
_LIT( KUrlViewUpdated, "http://iphone.dailymotion.com/search/" );
_LIT( KUrlViewRelevance, "http://iphone.dailymotion.com/relevance/search/" );
_LIT( KUrlViewTopRated, "http://iphone.dailymotion.com/popular/search/" );

HBufC* CPlugin::SearchUrlL( const TDesC& aString, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy aOrderBy )
	{
	RBuf url;
	CleanupClosePushL( url );

	switch( aOrderBy )
		{
		case EViewCount:
			url.Create( KUrlViewCount().Length() );
			url.Copy( KUrlViewCount() );
		break;

		case EUpdated:
			url.Create( KUrlViewUpdated().Length() );
			url.Copy( KUrlViewUpdated() );
		break;
		
		case ERating:
			url.Create( KUrlViewTopRated().Length() );
			url.Copy( KUrlViewTopRated() );
		break;

		case ERelevance:
			url.Create( KUrlViewRelevance().Length() );
			url.Copy( KUrlViewRelevance() );
		break;

		default:
			url.Create( KUrlViewCount().Length() );
			url.Copy( KUrlViewCount() );
		break;
		}

		HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( aString );
		CleanupStack::PushL( utf8 );

		HBufC8* encode = EscapeUtils::EscapeEncodeL( *utf8 , EscapeUtils::EEscapeUrlEncoded );
		CleanupStack::PushL( encode );

		HBufC16* unicode = EscapeUtils::ConvertToUnicodeFromUtf8L( *encode );
		CleanupStack::PushL( unicode );

	url.ReAllocL( url.Length() + unicode->Length() + 1 );
	url.Append( *unicode );
	url.Append( _L("/") );

		CleanupStack::PopAndDestroy( unicode );
		CleanupStack::PopAndDestroy( encode );
		CleanupStack::PopAndDestroy( utf8 );


	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<16> num;
	num.Format( _L("%d"), page );
	url.ReAllocL( url.Length() + num.Length() );
	url.Append( num );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	return ret;
	}

HBufC* CPlugin::ConvertUtfToUnicodeL( const TDesC8& aString )
	{
	RBuf output;
	CleanupClosePushL( output );
	
	TBuf16<20> outputBuffer;
	TPtrC8 remainderOfUtf7( aString );

	for(;;)
		{
		const TInt returnValue = CnvUtfConverter::ConvertToUnicodeFromUtf8(outputBuffer, remainderOfUtf7);
		if (returnValue==CnvUtfConverter::EErrorIllFormedInput)
			return NULL;
		else if (returnValue<0)
			return NULL;
        
		output.ReAllocL( output.Length() + outputBuffer.Length() );
		output.Append( outputBuffer );

        if (returnValue == 0)
            break;

        remainderOfUtf7.Set(remainderOfUtf7.Right(returnValue));
		}

	HBufC* ret = output.AllocL();
	
	CleanupStack::PopAndDestroy( &output );
	return ret;
	}

TInt CPlugin::ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler )
	{
	TInt count = 0;
	TInt index = aResponse.Find( _L8("dm_widget_iphone_list") );
	const TPtrC8 rem = aResponse.Mid( index + 23 );
	TInt index2 = rem.Find( _L8("dm_widget_iphone_footer") );
	if( index == KErrNotFound || index2 == KErrNotFound )
		return KErrNotSupported;
	index2 = index2 + index + 23;
	
	while(1)
		{
		const TPtrC8 data = aResponse.Mid( index, index2 - index );
		TInt start = data.Find( _L8("dm_widget_iphone_videoitem") );
		if( start == KErrNotFound )
			break;
		
		index += ( start + 9 );
		const TPtrC8 data1 = aResponse.Mid( index, index2 - index );
		index++;

		MVideoEntry* entry = aHandler.AddVideoEntryL();

		start = data1.Find( _L8("a href=\"") ) + 8;
		TInt end = data1.Find( _L8("\" class=\"preview\"") );
		const TPtrC8 url = data1.Mid( start, end - start );
		HBufC* tmp = HBufC::NewLC( url.Length() + KDailyMotionMainUrl().Length() );
		HBufC* tmp1 = HBufC::NewLC( url.Length() );
		tmp1->Des().Copy( url );
		tmp->Des().Copy( KDailyMotionMainUrl() );
		tmp->Des().Append( *tmp1 );
		CleanupStack::PopAndDestroy( tmp1 );

		TInt index = tmp->LocateReverse( '/' );
		TPtrC id = tmp->Right( tmp->Length() - index - 1 );
		entry->SetVideoIdL( id );
		entry->SetRelatedUrlL( id );
		HBufC* vurl = CreateUrlFromIdL( id );
		CleanupStack::PushL( vurl );
		entry->SetUrlL( *vurl );
		CleanupStack::PopAndDestroy( vurl );
		CleanupStack::PopAndDestroy( tmp );

		_LIT8(KImg, "<img src=\"" );
		start = data1.Find( KImg() );
		end = start + KImg().Length() + data1.Mid( start + KImg().Length() ).Find( _L8("?") );
		const TPtrC8 imgurl = data1.Mid( start + KImg().Length(), end - (start + KImg().Length() ) );
		tmp = HBufC::NewLC( imgurl.Length() );
		tmp->Des().Copy( imgurl );
		entry->SetThumbnailUrlL( *tmp );
		CleanupStack::PopAndDestroy( tmp );

		_LIT8(KTitle, "class=\"title\">" );
		start = data1.Find( KTitle() );
		start += KTitle().Length();
		end = data1.Mid( start ).Find( _L8("</a>") );
		const TPtrC8 title = data1.Mid( start, end );

		HBufC* mediaTitle = ConvertUtfToUnicodeL( title );
		CleanupStack::PushL( mediaTitle );
		entry->SetTitleL( *mediaTitle );
		CleanupStack::PopAndDestroy( mediaTitle );

		
		_LIT8(KViews, "<div class=\"views\">" );
		start = data1.Find( KViews() );
		TInt viewsCount;
		TLex8 views( TLex8( data1.Mid( start + KViews().Length() ) ).NextToken() );
		views.Val( viewsCount );
		entry->SetViewCount( viewsCount );

		_LIT8(KDuration, "<div class=\"duration\">" );
		start = data1.Find( KDuration() );
		TLex8 duration( TLex8( data1.Mid( start + KDuration().Length() ) ).NextToken() );
		TPtrC8 d = duration.NextToken();
		TLex8 min( d.Mid( 0, 2 ) );
		TLex8 sec( d.Mid( 3, 2 ) );
		TInt mm;
		TInt ss;
		min.Val( mm );
		sec.Val( ss );
		entry->SetDuration( (mm*60) + ss );

		_LIT8(KOwner, "class=\"owner\"" );
		start = data1.Find( KOwner() );
		const TPtrC8 ownerString = data1.Mid( start );

		_LIT8(KOwnerHref, " href=\"/" );
		start = ownerString.Find( KOwnerHref() );
		start += KOwnerHref().Length();
		end = ownerString.Mid( start ).Find( _L8("\"" ) );
		const TPtrC8 owner = ownerString.Mid( start, end );
		tmp = HBufC::NewLC( owner.Length() );
		tmp->Des().Copy( owner );
		entry->SetAuthorNameL( *tmp );
		entry->SetAuthorVideosUrlL( *tmp );
		CleanupStack::PopAndDestroy( tmp );

		count++;
		}

	return count;
	}

HBufC* CPlugin::VideoUrlL( const TDesC8& aResponse, TBool& aLast )
	{
	aLast = ETrue;
	HBufC* ret = NULL;

	TInt start = aResponse.Find( _L8("\"video\", \"") );
	if( start != KErrNotFound )
		{
		RBuf url;
		CleanupClosePushL( url );

		start += 10;

		TInt end = aResponse.Mid( start ).Find( _L8("\"") );
		const TPtrC8 encoded = aResponse.Mid( start, end );
		HBufC8* decoded = EscapeUtils::EscapeDecodeL( encoded );
		CleanupStack::PushL( decoded );

		end = decoded->Find( _L8("@@") );
		
		const TPtrC8 flvurl = decoded->Mid( 0, end );

#if 0
		url.CreateL( flvurl.Length() - 1 + KDailyMotionMainUrl().Length() );
		url.Copy( KDailyMotionMainUrl() );

		HBufC* decoded16 = HBufC::NewLC( flvurl.Length() );
		decoded16->Des().Copy( flvurl );
		url.Append( decoded16->Mid( 1 ) );
		CleanupStack::PopAndDestroy( decoded16 );
#else
		url.CreateL( flvurl.Length() );
		url.Copy( flvurl );
#endif

		CleanupStack::PopAndDestroy( decoded );

		ret = url.AllocL();
		CleanupStack::PopAndDestroy( &url );
		}

	return ret;
	}

//login / video upload

_LIT( KDailyMotionLoginUrl, "http://www.dailymotion.com/login?urlback=/" );
_LIT( KDailyMotionUsername, "&username=" );
_LIT( KDailyMotionPassword, "&password=" );
_LIT( KDailyMotionLoginData, "&rememberme=on" );

void CPlugin::LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword )
	{
	aUrl = KDailyMotionLoginUrl().AllocL();
	aContentType = KContentTypeUrlEncoded().AllocL();

	HBufC8* body = HBufC8::NewL( KDailyMotionLoginData().Length() + aUserName.Length() + aPassword.Length() + KDailyMotionPassword().Length() + KDailyMotionUsername().Length() );
	body->Des().Copy( KDailyMotionUsername() );
	body->Des().Append( aUserName );
	body->Des().Append( KDailyMotionPassword() );
	body->Des().Append( aPassword );
	body->Des().Append( KDailyMotionLoginData() );
	aBody = body;
	}

TBool CPlugin::ParseLoginResponseL( TDesC8& aResponse )
	{
	return ETrue;
	}

//upload video
_LIT( KDailyMotionUploadVideoUrl, "http://www.dailymotion.com/upload" );

HBufC* CPlugin::UploadMovieUrlL()
	{
	return KDailyMotionUploadVideoUrl().AllocL();
	}

_LIT8(KUrlStart, "dm_widget_uploadweb\" ><form action=\"");
_LIT( KFlashModeTxt, "&flash_mode=1" );

TInt CPlugin::ParseUploadMovieResponseL( const TDesC8& aResponse )
	{
	TInt offset = aResponse.Find( KUrlStart );
	if( offset != KErrNotFound )
		{
		TPtrC8 buf( aResponse.Mid( offset + KUrlStart().Length() ) );
		TInt end = buf.Locate( '"' );
		const TPtrC8 aurl = buf.Mid( 0, end );
		delete iActionUrl;
		iActionUrl = HBufC::NewL( aurl.Length() + KFlashModeTxt().Length() );
		iActionUrl->Des().Copy( aurl );
		iActionUrl->Des().Append( KFlashModeTxt() );
		return KErrNone;
		}

	return KErrNotFound;
	}

TBool CPlugin::UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	aUrl = iActionUrl->AllocL();
	aContentType = KContentTypeMultipart().AllocL();


//part1
	RBuf data;
	CleanupClosePushL( data );

	data.Create( 1 );

_LIT( KFiledataDisposition, "Content-Disposition: form-data; name=\"Filedata\"; filename=\"");
_LIT( KOctetStreamContentType, "Content-Type: application/octet-stream" );

	TParsePtrC parse( aData.Filename() );

	CPluginUtils::AddMultipartParamL( data, _L("Filename"), parse.NameAndExt() );
	data.ReAllocL( data.Length() + KBoundary().Length() + 2 + 2 + 2 + 2 + 2 + 2 + 1 + KFiledataDisposition().Length() + parse.NameAndExt().Length() + KOctetStreamContentType().Length() );
	data.Append( _L("--") );
	data.Append( KBoundary() );
	data.Append( KCrLf );
	data.Append( KFiledataDisposition() );
	data.Append( parse.NameAndExt() );
	data.Append( _L("\"") );
	data.Append( KCrLf );
	data.Append( KOctetStreamContentType() );
	data.Append( KCrLf );
	data.Append( KCrLf );
	
	HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( data );
	aBodyStart = utf8;
	CleanupStack::PopAndDestroy( &data );

//part2

	CleanupClosePushL( data );
_LIT( KUploadDisposition, "Content-Disposition: form-data; name=\"Upload\"\r\n\r\nSub");
	data.Create( 2 + 2 + KBoundary().Length() + 2 + KUploadDisposition().Length() + 2 + KBoundary().Length() + 2 + 2 + 2 );
	data.Append( KCrLf );
	data.Append( _L("--") );
	data.Append( KBoundary() );
	data.Append( KCrLf );
	data.Append( KUploadDisposition() );
	data.Append( KCrLf );
	data.Append( KBoundary() );
	data.Append( _L("--") );
	data.Append( KCrLf );
	data.Append( KCrLf );
	
	utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( data );
	aBodyEnd = utf8;
	CleanupStack::PopAndDestroy( &data );

	return ETrue;
	}

TInt CPlugin::ParseUploadMovieResponseStep1L( const TDesC8& aResponse )
	{
//{"url":"http:\/\/www.dailymotion.com\/upload?error_msg=Error: The uploaded file was only partially uploaded.&upload_id=62579636348781814258534221787164"}
//{"url":"http:\/\/www.dailymotion.com\/video\/edit\/x5ggx9_1"}

	TInt idx = aResponse.Find( _L8("error_msg") );
	if( idx != KErrNotFound )
		return KErrNotFound;
	
	idx = aResponse.Find( _L8("{\"url\":\"") );
	if( idx == KErrNotFound )
		return KErrNotFound;
	
	idx = aResponse.Find( _L8("http:") );
	TInt idx1 = aResponse.Find( _L8("}") );
	if( idx1 == KErrNotFound )
		return KErrNotFound;
	
	delete iActionUrl;
	iActionUrl = HBufC::NewL( idx1 - idx - 1 );
	for(TInt i=idx;i<idx1-1;i++)
		{
		if( aResponse[i] != '\\' )
			iActionUrl->Des().Append( aResponse[i] );
		}
	
	return KErrNone;

	}

TBool CPlugin::UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	aUrl = iActionUrl->AllocL();
	aContentType = KContentTypeUrlEncoded().AllocL();

	RBuf data;
	CleanupClosePushL( data );
	
	CPluginUtils::AddUrlParamL( data, _L("new"), 1 );
	CPluginUtils::AddUrlParamL( data, _L("title"), aData.Title() );
	CPluginUtils::AddUrlParamL( data, _L("tags"), aData.Tags() );
	CPluginUtils::AddUrlParamL( data, _L("language"), _L("en") );
	CPluginUtils::AddUrlParamL( data, _L("description"), aData.Description() );

	if( aData.Public() )
		CPluginUtils::AddUrlParamL( data, _L("privacy"), _L("public") );
	else
		CPluginUtils::AddUrlParamL( data, _L("privacy"), _L("private") );

	CPluginUtils::AddUrlParamL( data, _L("relation_group_1"), KNullDesC() );
	CPluginUtils::AddUrlParamL( data, _L("relation_group_2"), KNullDesC() );

	CPluginUtils::AddUrlParamL( data, _L("allow_comments"), 1 );
	CPluginUtils::AddUrlParamL( data, _L("user_featured"), KNullDesC() );

	switch( aData.Category() )
		{
		case EFilmAndAnimation:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Ashortfilms") );
		break;

		case ECarsAndVehicles:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Aauto") );
		break;

		case EMusic:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Amusic") );
		break;

		case EPetsAndAnimals:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Aanimals") );
		break;

		case ESports:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Asport") );
		break;

		case ETravelAndEvents:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Atravel") );
		break;

		case EPeopleAndBlogs:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Apeople") );
		break;

		case EComedy:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Afun") );
		break;

		case EEntertainment:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Afun") );
		break;

		case ENewsAndPolitics:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Anews") );
		break;

		case EHowtoAndStyle:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Alifestyle") );
		break;

		case EEducation:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Aschool") );
		break;

		case EScienceAndTechnology:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Atech") );
		break;

		case ENonprofitsAndActivism:
			CPluginUtils::AddUrlParamL( data, _L("user-category%5B%5D"), _L("user-category%3Acreation") );
		break;
		}

	CPluginUtils::AddUrlParamL( data, _L("save"), _L("Saving..."), ETrue );

	HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( data );
	aBodyStart = utf8;
	aBodyEnd = NULL;
	CleanupStack::PopAndDestroy( &data );

	return EFalse;
	}

TInt CPlugin::ParseUploadMovieResponseStep2L( const TDesC8& aResponse )
	{
	TInt offset = aResponse.Find( _L8("<div id=\"skip_step\">") );
	if( offset == KErrNotFound )
		{
		return KErrNotFound;
		}
	return KErrNone;
	}


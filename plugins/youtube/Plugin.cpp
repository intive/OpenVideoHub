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

#include "tinyxml.h"
#include "Plugin.h"
#include <YouTubePlugin.mbg>

_LIT( KFileDisposition, "Content-Disposition: form-data; name=\"field_uploadfile\"; filename=\"" );
_LIT( KVideoContentType, "Content-Type: video/mp4" );

_LIT( KYouTubeVideoUrl, "http://www.youtube.com/watch?v=" );
//_LIT( KYouTubeVideoUrl, "http://www.youtube.com/v/" );

_LIT( KYouTubeVideoUrlNoMobile, "&nomobile=1" );

_LIT( KFeaturedUrl, "http://gdata.youtube.com/feeds/standardfeeds/recently_featured?" );
_LIT( KTopRatedUrl, "http://gdata.youtube.com/feeds/standardfeeds/top_rated?" );
_LIT( KMostViewedUrl, "http://gdata.youtube.com/feeds/standardfeeds/most_viewed?" );
_LIT( KRecentlyAddedUrl, "http://gdata.youtube.com/feeds/api/standardfeeds/most_recent?" );

_LIT( KYTubeUrl, "http://gdata.youtube.com/feeds/videos?" );
_LIT( KAuthorVideosUrl, "http://gdata.youtube.com/feeds/videos?author=" );
_LIT( KImgUrl, "http://img.youtube.com/vi/" );
_LIT( KImgDefault, "/default.jpg" );
_LIT( KRelatedUrl, "http://gdata.youtube.com/feeds/videos/" );
_LIT( KRelatedUrlEnd, "/related?" );
_LIT( KGetVideoUrl, "http://www.youtube.com/get_video?video_id=" );


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
	delete iToken;
	delete iAddresser;
	delete iActionUrl;
	}

CPlugin::CPlugin()
	{
	}

void CPlugin::ConstructL()
	{
	iToken = KNullDesC8().AllocL();
	}

HBufC* CPlugin::ConvertCharToHBufCL( const char *aString )
	{
	TInt len = strlen( aString );
	TPtr8 string((TUint8 *)aString, len, len);
	HBufC* u = HBufC::NewL( len );
	u->Des().Copy( string );
	return u;
	}

HBufC* CPlugin::ConvertCharToHBufCL( const unsigned char *aString, TInt aLength )
	{
	TPtr8 string((TUint8 *)aString, aLength, aLength);
	HBufC* u = HBufC::NewL( aLength );
	u->Des().Copy( string );
	return u;
	}


TInt CPlugin::Capabilities()
	{
	return KCapsAccessVideoById | KCapsRelatedClips | KCapsUserClips |
		KCapsFeatured | KCapsTopRated | KCapsMostViewed |
		KCapsRecentlyUploaded | KCapsTimeFrame | KCapsLogin |
		KCapsVideoUpload | KCapsVideoDownload;
	}

TBool CPlugin::CanHandleUrlL( const TDesC& aUrl )
	{
	TBool ret = EFalse;

	RBuf url;
	CleanupClosePushL( url );

	url.Create( aUrl.Length() );
	url.CopyLC( aUrl );

	if( url.Find( _L("youtube.com") ) != KErrNotFound )
		ret = ETrue;

	CleanupStack::PopAndDestroy( &url );
	return ret;
	}

HBufC* CPlugin::CreateUrlFromIdL( const TDesC& aId )
	{
	RBuf vParam;
	CleanupClosePushL( vParam );

	vParam.ReAllocL( KYouTubeVideoUrl().Length() );
	vParam.Copy( KYouTubeVideoUrl() );

	TInt pos = aId.Locate( '?' );
	if( pos != KErrNotFound )
		{
		while( pos > 0 && pos < aId.Length() )
			{
			TPtrC right = aId.Right( aId.Length() - pos - 1 );
			TInt pos2 = right.Locate( '=' );
			TPtrC param = right.Mid( 0, pos2 );
			TInt pos3 = right.Locate( '&' );
			if( pos3 == KErrNotFound )
				pos3 = right.Length();
			TPtrC value = right.Mid( pos2 + 1, pos3 - pos2 - 1 );

			if( !param.Compare( _L("v") ) )
				{
				vParam.ReAllocL( vParam.Length() + value.Length() );
				vParam.Copy( value );
				}
			pos += (pos3 + 1);
			}
		}
	else
		{
		if( !aId.Mid( 0, 7).Compare( _L("http://") ) )
			{
			vParam.Zero();
			vParam.ReAllocL( aId.Length() );
			vParam.Copy( aId );
			}
		else
			{
			vParam.ReAllocL( vParam.Length() + aId.Length() );
			vParam.Append( aId );
			}
		}

	vParam.ReAllocL( vParam.Length() + KYouTubeVideoUrlNoMobile().Length() );
	vParam.Append( KYouTubeVideoUrlNoMobile() );

	HBufC* ret = vParam.AllocL();
	CleanupStack::PopAndDestroy( &vParam );

	return ret;
	}

HBufC* CPlugin::VideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime )
	{
	RBuf url;
	CleanupClosePushL( url );

	url.Create( aBaseUrl.Length() );
	url.Copy( aBaseUrl );

	if( url[ url.Length() - 1 ] != '?' )
		{
		url.ReAllocL( url.Length() + 1 );
		url.Append( _L("&") );
		}

	CPluginUtils::AddUrlParamL( url, _L("start-index"), aStartIndex );
	CPluginUtils::AddUrlParamL( url, _L("max-results"), aMaxResults );

	switch( aOrderBy )
		{
		case EViewCount:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("viewCount") );
		break;

		case EUpdated:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("published") );
		break;

		case ERating:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("rating") );
		break;

		case ERelevance:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("relevance") );
		break;

		default:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("viewCount") );
		break;
		}

	switch( aTime )
		{
		case EToday:
			CPluginUtils::AddUrlParamL( url, _L("time"), _L("today") );
		break;

		case EThisWeek:
			CPluginUtils::AddUrlParamL( url, _L("time"), _L("this_week") );
		break;

		case EThisMonth:
			CPluginUtils::AddUrlParamL( url, _L("time"), _L("this_month") );
		break;

		case EAllTime:
			CPluginUtils::AddUrlParamL( url, _L("time"), _L("all_time") );
		break;

		default:
		break;
		}

	CPluginUtils::AddUrlParamL( url, _L("alt"), _L("atom") );
	CPluginUtils::AddUrlParamL( url, _L("racy"), _L("include"), ETrue );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );

	return ret;
	}

HBufC* CPlugin::UserVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy )
	{
	return VideosUrlL( aBaseUrl, aMaxResults, aStartIndex, aOrderBy, EPeriodIrrelevant );
	}

HBufC* CPlugin::RelatedVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy )
	{
	return UserVideosUrlL( aBaseUrl, aMaxResults, aStartIndex, aOrderBy );
	}

_LIT( KPluginName, "YouTube" );
TPtrC16 CPlugin::Name()
	{
	return KPluginName();
	}

void CPlugin::IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask )
	{
	CFbsBitmap* bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, _L("\\resource\\apps\\youtubeplugin.mbm"), EMbmYoutubepluginLogo, EMbmYoutubepluginLogo_mask);
	aBitmap = bitmap;
	aMask = mask;
	}

TInt CPlugin::SearchResultsPerPage()
	{
	return 0;
	}

HBufC* CPlugin::FeatureUrlL( TFeature aFeature, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime )
	{
	HBufC* ret = NULL;
	switch( aFeature )
		{
			case ENewClips:
				{
				ret = VideosUrlL( KRecentlyAddedUrl(), aMaxResults, aStartIndex, EIrrelevant, EPeriodIrrelevant );
				}
			break;

			case EFeaturedClips:
				{
				ret = VideosUrlL( KFeaturedUrl(), aMaxResults, aStartIndex, aOrderBy, EPeriodIrrelevant );
				}
			break;

			case ETopRatedClips:
				{
				ret = VideosUrlL( KTopRatedUrl(), aMaxResults, aStartIndex, aOrderBy, aTime );
				}
			break;

			case EMostViewedClips:
				{
				ret = VideosUrlL( KMostViewedUrl(), aMaxResults, aStartIndex, aOrderBy, aTime );
				}
			break;

			default:
			break;
		}

	return ret;
	}

HBufC* CPlugin::SearchUrlL( const TDesC& aString, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy )
	{
	RBuf url;
	CleanupClosePushL( url );

	url.Create( KYTubeUrl().Length() );
	url.Copy( KYTubeUrl() );

		HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( aString );
		CleanupStack::PushL( utf8 );

		HBufC8* encode = EscapeUtils::EscapeEncodeL( *utf8 , EscapeUtils::EEscapeUrlEncoded );
		CleanupStack::PushL( encode );

		HBufC16* unicode = EscapeUtils::ConvertToUnicodeFromUtf8L( *encode );
		CleanupStack::PushL( unicode );

	CPluginUtils::AddUrlParamL( url, _L("vq"), *unicode );

		CleanupStack::PopAndDestroy( unicode );
		CleanupStack::PopAndDestroy( encode );
		CleanupStack::PopAndDestroy( utf8 );

	CPluginUtils::AddUrlParamL( url, _L("start-index"), aStartIndex );
	CPluginUtils::AddUrlParamL( url, _L("max-results"), aMaxResults );

	switch( aOrderBy )
		{
		case EViewCount:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("viewCount") );
		break;

		case EUpdated:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("published") );
		break;

		case ERating:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("rating") );
		break;

		case ERelevance:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("relevance") );
		break;

		case EIrrelevant:
		break;

		default:
			CPluginUtils::AddUrlParamL( url, _L("orderby"), _L("viewCount") );
		break;
		}

	CPluginUtils::AddUrlParamL( url, _L("alt"), _L("atom") );
	CPluginUtils::AddUrlParamL( url, _L("racy"), _L("include"), ETrue );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );
	return ret;
	}

TInt CPlugin::ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler )
	{
	TInt count = 0;

	TiXmlDocument doc( "" );
	doc.Parse( (const char *)aResponse.Ptr() );
	if ( doc.Error() )
		{
		return KErrNotSupported;
		}
	else
		{
		TiXmlElement* root = doc.FirstChildElement( "feed" );
		if( root )
			{
			TiXmlElement* entry = root->FirstChildElement( "entry" );
			while( entry )
				{
				count++;
				entry = entry->NextSiblingElement();
				}

			if( !count )
				{
				return 0;
				}

			entry = root->FirstChildElement( "entry" );
			while( entry )
				{
				MVideoEntry* ventry = aHandler.AddVideoEntryL();

				TiXmlElement* author = entry->FirstChildElement( "author" );
				if( author )
					{
					TiXmlElement* name = author->FirstChildElement( "name" );
					if( name )
						{
						const char *t = name->GetText();
						if( t )
							{

							HBufC* authorName = ConvertCharToHBufCL( t );
							CleanupStack::PushL( authorName );
							ventry->SetAuthorNameL( *authorName );

							RBuf authorVideosUrl;
							CleanupClosePushL( authorVideosUrl );
							authorVideosUrl.Create( KAuthorVideosUrl().Length() + authorName->Length() );
							authorVideosUrl.Copy( KAuthorVideosUrl );
							authorVideosUrl.Append( *authorName );
							ventry->SetAuthorVideosUrlL( authorVideosUrl );
							CleanupStack::PopAndDestroy( &authorVideosUrl );

							CleanupStack::PopAndDestroy( authorName );

							}
						else
							{
							ventry->SetAuthorNameL( KNullDesC() );
							}
						}
					}

				TiXmlElement* media = entry->FirstChildElement( "media:group" );
				if( media )
					{
					TiXmlElement* media_title = media->FirstChildElement( "media:title" );
					if( media_title )
						{
						const char *t = media_title->GetText();
						if( t )
							{

							TInt len = strlen( t );
							TPtr8 string((TUint8 *)t, len, len);

							HBufC* mediaTitle = CPluginUtils::ConvertUtfToUnicodeL( string );
							CleanupStack::PushL( mediaTitle );
							ventry->SetTitleL( *mediaTitle );
							CleanupStack::PopAndDestroy( mediaTitle );

							}
						else
							{
							ventry->SetTitleL( KNullDesC() );
							}
						}

					TiXmlElement* media_player = media->FirstChildElement( "media:player" );
					if( media_player )
						{
						const char *url = media_player->Attribute( "url" );
						if( url )
							{
							//watch?v=5bsXOcK9_Cw&feature=youtube_gdata
									
							HBufC* u = ConvertCharToHBufCL( url );
							CleanupStack::PushL( u );
							TInt index = u->Find( _L("v=") );
							TInt end = u->Mid( index ).Locate( '&' );
							
							if( end == KErrNotFound )
								{
								end = u->Mid( index ).Length();
								}
							TPtrC id = u->Mid( index + 2, end - 2 );
							ventry->SetVideoIdL(  id );

							RBuf url;
							CleanupClosePushL( url );
							url.ReAllocL( KYouTubeVideoUrl().Length() + id.Length() + KYouTubeVideoUrlNoMobile().Length() );
							url.Copy( KYouTubeVideoUrl );
							url.Append( id );
							url.Append( KYouTubeVideoUrlNoMobile() );
							ventry->SetUrlL( url );
							CleanupStack::PopAndDestroy( &url );

							RBuf thumbnailUrl;
							CleanupClosePushL( thumbnailUrl );
							thumbnailUrl.Create( KImgUrl().Length() + id.Length() + KImgDefault().Length() );
							thumbnailUrl.Copy( KImgUrl );
							thumbnailUrl.Append( id );
							thumbnailUrl.Append( KImgDefault() );
							ventry->SetThumbnailUrlL( thumbnailUrl );
							CleanupStack::PopAndDestroy( &thumbnailUrl );

							RBuf relatedUrl;
							CleanupClosePushL( relatedUrl );
							relatedUrl.Create( KRelatedUrl().Length() + id.Length() + KRelatedUrlEnd().Length() );
							relatedUrl.Copy( KRelatedUrl );
							relatedUrl.Append( id );
							relatedUrl.Append( KRelatedUrlEnd );
							ventry->SetRelatedUrlL( relatedUrl );
							CleanupStack::PopAndDestroy( &relatedUrl );

							CleanupStack::PopAndDestroy( u );
							}
						else
							{
							ventry->SetUrlL( KNullDesC() );
							ventry->SetVideoIdL( KNullDesC() );
							ventry->SetThumbnailUrlL( KNullDesC() );
							ventry->SetRelatedUrlL( KNullDesC() );
							}

					TiXmlElement* media_duration = media->FirstChildElement( "yt:duration" );
					if( media_duration )
						{
						int seconds = 0;
						if( media_duration->QueryIntAttribute( "seconds", &seconds ) == TIXML_SUCCESS )
							ventry->SetDuration( seconds );
						}
					}

					TiXmlElement* rating = entry->FirstChildElement( "gd:rating" );
					if( rating )
						{
						double average = 0.0;
						if( rating->QueryDoubleAttribute( "average", &average ) == TIXML_SUCCESS )
							ventry->SetAverageRating( average );
						}

					TiXmlElement* stats = entry->FirstChildElement( "yt:statistics" );
					if( stats )
						{
						int c = 0;
						if( stats->QueryIntAttribute( "viewCount", &c ) == TIXML_SUCCESS )
							ventry->SetViewCount( c );
						}
					}
				entry = entry->NextSiblingElement();
				}
			}
		else
			{
			return KErrNotSupported;
			}
		}
	return count;
	}

_LIT8( KVideoIdParam, "\"video_id\":" );
_LIT8( KTParam, "\"t\":" );
HBufC* CPlugin::VideoUrlL( const TDesC8& aResponse, TBool& aLast )
	{
	aLast = ETrue;

	RBuf newUrl;
	CleanupClosePushL( newUrl );

	newUrl.Create( KGetVideoUrl().Length() );
	newUrl.Copy( KGetVideoUrl() );

	TInt idx = aResponse.Find( KVideoIdParam() );
	if( idx != KErrNotFound )
		{
		idx += KVideoIdParam().Length();
		const TPtrC8 data = aResponse.Mid( idx );
		TInt start = data.Locate( '"' );
		TInt end = data.Mid( start + 1 ).Locate( '"' ) + 1;
		const TPtrC8 videoId = data.Mid( start + 1, end - start );

		newUrl.ReAllocL( newUrl.Length() + videoId.Length() );

		HBufC* tmp = ConvertCharToHBufCL( videoId.Ptr(), videoId.Length() );
		newUrl.Append( *tmp );
		delete tmp;
		}
	else
		{
		CleanupStack::PopAndDestroy( &newUrl );
		return NULL;
		}

	idx = aResponse.Find( KTParam() );
	if( idx != KErrNotFound )
		{
		idx += KTParam().Length();
		const TPtrC8 data = aResponse.Mid( idx );
		TInt start = data.Locate( '"' );
		TInt end = data.Mid( start + 1 ).Locate( '"' ) + 1;
		const TPtrC8 tParam = data.Mid( start + 1, end - start );

		newUrl.ReAllocL( newUrl.Length() + 3 + tParam.Length() );
		newUrl.Append( _L("&t=") );

		HBufC* tmp = ConvertCharToHBufCL( tParam.Ptr(), tParam.Length() );
		newUrl.Append( *tmp );
		delete tmp;
		}
	else
		{
		CleanupStack::PopAndDestroy( &newUrl );
		return NULL;
		}

	newUrl.ReAllocL( newUrl.Length() + KYouTubeVideoUrlNoMobile().Length() );
	newUrl.Append( KYouTubeVideoUrlNoMobile() );
	HBufC* ret = newUrl.AllocL();
	CleanupStack::PopAndDestroy( &newUrl );

	return ret;
	}

//login
_LIT( KYouTubeLoginUrl, "http://www.youtube.com/login" );
_LIT( KYouTubeUsername, "&username=" );
_LIT( KYouTubePassword, "&password=" );
_LIT( KYouTubeToken, "&session_token=" );
_LIT( KYoTubeLoginData, "&current_form=loginForm&action_login=Log+In" );

void CPlugin::LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword )
	{
	aUrl = KYouTubeLoginUrl().AllocL();
	aContentType = KContentTypeUrlEncoded().AllocL();

	HBufC8* body = HBufC8::NewL( KYoTubeLoginData().Length() + aUserName.Length() + aPassword.Length() + KYouTubeUsername().Length() + KYouTubePassword().Length() + KYouTubeToken().Length() + iToken->Length() );
	body->Des().Copy( KYoTubeLoginData() );
	body->Des().Append( KYouTubeUsername() );
	body->Des().Append( aUserName );
	body->Des().Append( KYouTubePassword() );
	body->Des().Append( aPassword );
	body->Des().Append( KYouTubeToken() );
	body->Des().Append( *iToken );

	aBody = body;

	}

TBool CPlugin::ParseLoginResponseL( TDesC8& aResponse )
	{
	TInt idx = 0;
	TBool done = EFalse;

	while( !done )
		{
		const TPtrC8 rem = aResponse.Mid( idx );
		
		TInt start = rem.Find( _L8("gXSRF_token") );
		if( start == KErrNotFound )
			return EFalse;
		
		idx += (start + 1);
		
		TPtrC8 rest = rem.Mid( start );
		start = rest.Locate( '\'' );
		if( start == KErrNotFound )
			return EFalse;

		TInt end = rest.Mid( start + 1 ).Locate( '\'' );
		const TPtrC8 token = rest.Mid( start + 1, end );
		if( token.Length() )
			{
			delete iToken;
			iToken = token.AllocL();
			return ETrue;
			}
		}

	return EFalse;
	}

//upload video
_LIT( KYouTubeUploadVideoUrl, "http://www.youtube.com/my_videos_upload" );

HBufC* CPlugin::UploadMovieUrlL()
	{
	return KYouTubeUploadVideoUrl().AllocL();
	}

TInt CPlugin::ParseUploadMovieResponseL( const TDesC8& aResponse )
	{
	//fixme - parse session token and pass it as aPrivateData
	return KErrNone;
	}

TBool CPlugin::UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	aUrl = KYouTubeUploadVideoUrl().AllocL();
	aContentType = KContentTypeUrlEncoded().AllocL();

	RBuf data;
	CleanupClosePushL( data );

	data.Create( 1 );

	CPluginUtils::AddUrlParamL( data, _L("field_myvideo_title"), aData.Title() );
	CPluginUtils::AddUrlParamL( data, _L("field_myvideo_keywords"), aData.Tags() );
	CPluginUtils::AddUrlParamL( data, _L("field_myvideo_descr"), aData.Description() );
	CPluginUtils::AddUrlParamL( data, _L("language"), _L("EN") );
	CPluginUtils::AddUrlParamL( data, _L("field_myvideo_categories"), aData.Category() );
	CPluginUtils::AddUrlParamL( data, _L("ignore_broadcast_settings"), _L("0") );
	CPluginUtils::AddUrlParamL( data, _L("action_upload"), _L("1") );
	CPluginUtils::AddUrlParamL( data, _L("field_privacy"), aData.Public() ? _L("public") : _L("private") ) ;
//fixme -> add session_token

//fixme - encode each string separately!
	HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( data );
//	HBufC8* encode = EscapeUtils::EscapeEncodeL( *utf8 , EscapeUtils::EEscapeUrlEncoded );
//	CleanupStack::PushL( encode );
//	iBody->PrepareL( *encode );
	aBodyStart = utf8;
	aBodyEnd = NULL;
//	CleanupStack::PopAndDestroy( encode );
	CleanupStack::PopAndDestroy( &data );

	return EFalse;
	}

TInt CPlugin::ParseUploadMovieResponseStep1L( const TDesC8& aResponse )
	{
	TInt found = 0;
	TInt offset = aResponse.Find( _L8("id=\"theForm\"") );
	if( offset != KErrNotFound )
		{
		TPtrC8 buf( aResponse.Mid( 0, offset ) );
		offset = buf.LocateReverse( '<' );
		if( offset != KErrNotFound )
			{
			TPtrC8 buf1( aResponse.Mid( offset ) );
			offset = buf1.Find( _L8("action=\"") );
			if( offset != KErrNotFound )
				{
				TPtrC8 buf2( buf1.Mid( offset + 8 ) );
				TInt end = buf2.Locate( '"' );

				const TPtrC8 aurl = buf2.Mid( 0, end );
				delete iActionUrl;
				iActionUrl = HBufC::NewL( aurl.Length() );
				iActionUrl->Des().Copy( aurl );
				found++;
				}

			offset = buf1.Find( _L8("name=\"addresser\" value=\"") );
			if( offset != KErrNotFound )
				{
				TPtrC8 buf2( buf1.Mid( offset + 24 ) );
				TInt end = buf2.Locate( '"' );

				const TPtrC8 aurl = buf2.Mid( 0, end );
				delete iAddresser;
				iAddresser = HBufC::NewL( aurl.Length() );
				iAddresser->Des().Copy( aurl );
				found++;
				}

			}
		}

	if( found != 2 )
		{
		return KErrNotFound;
		}

	return KErrNone;
	}

TBool CPlugin::UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	aUrl = iActionUrl->AllocL();
	aContentType = KContentTypeMultipart().AllocL();

	RBuf data;
	CleanupClosePushL( data );

	data.Create( 1 );

	CPluginUtils::AddMultipartParamL( data, _L("field_command"), _L("myvideo_submit") );
	CPluginUtils::AddMultipartParamL( data, _L("field_myvideo_title"), aData.Title() );
	CPluginUtils::AddMultipartParamL( data, _L("field_myvideo_keywords"), aData.Tags() );
	CPluginUtils::AddMultipartParamL( data, _L("field_myvideo_descr"), aData.Description());
	CPluginUtils::AddMultipartParamL( data, _L("language"), _L("EN") );
	CPluginUtils::AddMultipartParamL( data, _L("field_myvideo_categories"), aData.Category() );
	CPluginUtils::AddMultipartParamL( data, _L("action_upload"), _L("1") );
	CPluginUtils::AddMultipartParamL( data, _L("addresser"), *iAddresser );
	CPluginUtils::AddMultipartParamL( data, _L("field_privacy"), aData.Public() ? _L("public") : _L("private") );

	TParsePtrC parse( aData.Filename() );
	data.ReAllocL( data.Length() + KBoundary().Length() + 2 + 2 + 2 + 2 + 2 + 2 + 1 + KFileDisposition().Length() + parse.NameAndExt().Length() + KVideoContentType().Length() );
	data.Append( _L("--") );
	data.Append( KBoundary() );
	data.Append( KCrLf );
	data.Append( KFileDisposition() );
	data.Append( parse.NameAndExt() );
	data.Append( _L("\"") );
	data.Append( KCrLf );
	data.Append( KVideoContentType() );
	data.Append( KCrLf );
	data.Append( KCrLf );

//fixme - encode each string separately!
	HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( data );
//	HBufC8* encode = EscapeUtils::EscapeEncodeL( *utf8 , EscapeUtils::EEscapeUrlEncoded );
//	CleanupStack::PushL( encode );
//	iBody->PrepareL( *encode );
	aBodyStart = utf8;
	aBodyEnd = NULL;
//	CleanupStack::PopAndDestroy( encode );
	CleanupStack::PopAndDestroy( &data );

	return ETrue;
	}

TInt CPlugin::ParseUploadMovieResponseStep2L( const TDesC8& aResponse )
	{
	return KErrNone;
	}

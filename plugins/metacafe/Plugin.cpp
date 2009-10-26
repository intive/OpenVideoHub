//#include <string.h>
#include <utf.h>
#include <akniconutils.h>
#include <MetacafePlugin.mbg>
#include <EIKENV.H>
#include <f32file.h>
#include <EscapeUtils.h>

#include "Plugin.h"

#include "pluginUtils.h"

_LIT( KPluginName, "Metacafe" );
_LIT(KUrlTopVideo, "http://www.metacafe.com/f/top_videos/");
_LIT(KUrlMostViewed, "http://www.metacafe.com/f/top_videos/most_popular/");
_LIT(KUrlNewClips, "http://www.metacafe.com/f/top_videos/newest/");

_LIT(KSlash,"/");
_LIT(KUrlMetacafe,"http://metacafe.com");
_LIT8(KUlFlagStart,"<ul class=\"Default Items\"");
_LIT8(KUlFlagStart1,"<ul id=\"Catalog1\"");
_LIT8(KUlFlagEnd,"</ul>");

_LIT(KLiFlagStart,"<li id=");
_LIT(KLiFlagEnd,"</li>");

_LIT(KVideo,"href=\"/watch/");
_LIT(KTitle,"title=");

_LIT(KThumb,"<img src=");

_LIT(KDurationStart,"<span>");
_LIT(KDurationEnd,"</span>");
_LIT(KDurationStart1, "<small class=\"ItemDuration\">");
_LIT(KDurationEnd1, "</small>");

_LIT(KRatingStart,"<em >Rated ");
_LIT(KRatingEnd,"</em>");
_LIT(KRatingStart1,"Rated <em>");

_LIT(KViewsStart,"<em >");
_LIT(KViewsStart1,"ItemViews\"><em>");
_LIT(KViewsEnd,"</em>");


_LIT(KWatch,"/watch/");
_LIT(KTags,"/tags/");

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
	}

CPlugin::CPlugin()
	{
	}

void CPlugin::ConstructL()
	{
	}

TInt CPlugin::Capabilities()
	{
 
	return KCapsAccessVideoById |
		KCapsTimeFrame | /*KCapsUserClips |
		KCapsFeatured | */ KCapsTopRated | 
		KCapsMostViewed | KCapsRecentlyUploaded |
		KCapsConstResultsPerPage | KCapsLogin | KCapsAllowSavingVideos | 
		KCapsVideoDownload;
	}


TPtrC16 CPlugin::Name()
	{
	return KPluginName();
	}

void CPlugin::IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask )
	{
	CFbsBitmap* bitmap, *mask;
	AknIconUtils::CreateIconL(bitmap, mask, _L("\\resource\\apps\\MetacafePlugin.mbm"), EMbmMetacafepluginLogo, EMbmMetacafepluginLogo_mask);

	aBitmap = bitmap;
	aMask = mask;
	}

TBool CPlugin::CanHandleUrlL( const TDesC& aUrl )
	{
	TBool ret = EFalse;

	RBuf url;
	CleanupClosePushL( url );

	url.Create( aUrl.Length() );
	url.CopyLC( aUrl );

	if( url.Find( _L("metacafe.com") ) != KErrNotFound )
		ret = ETrue;

	if( url.Find( _L("mcstatic.com") ) != KErrNotFound )
		ret = ETrue;
	
	if( url.Find( _L("mccont.com") ) != KErrNotFound )
		ret = ETrue;
	
	if( url.Find( _L("img.youtube.com") ) != KErrNotFound )
		ret = ETrue;

	CleanupStack::PopAndDestroy( &url );
	return ret;
	}


HBufC* CPlugin::CreateUrlFromIdL( const TDesC& aId )
	{	
		
		 HBufC* VideoId;
	
		if( aId.FindC( _L("http://" )) ==	KErrNotFound )
		{
			VideoId = HBufC::NewL(KUrlMetacafe().Length() + KWatch().Length() + aId.Length() + KSlash().Length());	
			 	VideoId->Des().Copy(KUrlMetacafe);
		 	VideoId->Des().Append(KWatch);
		 	VideoId->Des().Append(aId);
			 	VideoId->Des().Append(KSlash);
		}
		else
		{
			VideoId = HBufC::NewL(aId.Length());	
			 	VideoId->Des().Copy(aId);
	
		}


	return VideoId;
	}

_LIT( KMostPopular, "most_popular" );
_LIT( KTopRated, "rated" );
_LIT( KUpdated, "newest" );

_LIT( KTimeEver, "ever" );
_LIT( KTimeMonth, "month" );

HBufC* CPlugin::FeatureUrlL( TFeature aFeature, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime )
	{
	HBufC* url;
	RBuf data;
	CleanupClosePushL( data );
	
	iResultsPerPage = 15;

	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<22> num;
	num.Format( _L("page-%d"), page );

	TBool append0 = EFalse;

	switch( aFeature )
		{
		case ETopRatedClips:
			{
			data.Create( KUrlTopVideo().Length() );
	 		data.Copy( KUrlTopVideo() );
			}
		break;
		
		case EMostViewedClips:
			{
			data.Create( KUrlMostViewed().Length() );
	 		data.Copy( KUrlMostViewed() );
			}
		break;

		case ENewClips:
			{
			data.Create( KUrlNewClips().Length() );
	 		data.Copy( KUrlNewClips() );
			append0 = ETrue;
			}
		break;
		
		default:
		break;

		}

	switch( aOrderBy )
		{
		case EViewCount:
			{
			data.ReAllocL( data.Length() + KMostPopular().Length() + KSlash().Length() );
			data.Append( KMostPopular() );
			data.Append( KSlash() );
			}
		break;

		case EUpdated:
			{
			if( aFeature != ENewClips )
				{
				data.ReAllocL( data.Length() + KUpdated().Length() + KSlash().Length() );
				data.Append( KUpdated() );
				data.Append( KSlash() );
				}
			}
		break;

		case ERating:
			{
			data.ReAllocL( data.Length() + KTopRated().Length() + KSlash().Length() );
			data.Append( KTopRated() );
			data.Append( KSlash() );
			}
		break;

		case ERelevance:
		default:
		break;
		}

	switch( aTime )
		{
			case EToday:
			case EThisWeek:
			break;

			case EThisMonth:
				data.ReAllocL( data.Length() + KTimeMonth().Length() + KSlash().Length() );
				data.Append( KTimeMonth() );
				data.Append( KSlash() );
			break;

			case EAllTime:
				data.ReAllocL( data.Length() + KTimeEver().Length() + KSlash().Length() );
				data.Append( KTimeEver() );
				data.Append( KSlash() );
			break;

			default:
			break;
		}

	data.ReAllocL( data.Length() + num.Length() + KSlash().Length() );
	data.Append( num );
	data.Append( KSlash() );

	if( append0 )
		{
		data.ReAllocL( data.Length() + 2 );
		data.AppendNum( 0 );
		data.Append( KSlash() );
		}

	url = data.AllocL();
	CleanupStack::PopAndDestroy( &data );

	return url;
	}

HBufC* CPlugin::SearchUrlL( const TDesC& aString, TInt /*aMaxResults*/, TInt aStartIndex, TOrderBy aOrderBy )
	{
	iResultsPerPage = 15;

	TInt page = ( aStartIndex / SearchResultsPerPage() ) + 1;
	TBuf<22> num;
	num.Format( _L("page-%d"), page );

	RBuf url;
	CleanupClosePushL( url );

	url.Create( KUrlMetacafe().Length() + KTags().Length() +	aString.Length() + KSlash().Length() );
	url.Copy( KUrlMetacafe() );
	url.Append( KTags() );
	url.Append( aString );
	url.Append( KSlash() );

	switch( aOrderBy )
		{
			case EViewCount:
				url.ReAllocL( url.Length() + KMostPopular().Length() + KSlash().Length() );
				url.Append( KMostPopular() );
				url.Append( KSlash() );		
			break;

			case EUpdated:
				url.ReAllocL( url.Length() + KUpdated().Length() + KSlash().Length() );
				url.Append( KUpdated() );
				url.Append( KSlash() );		
			break;

			case ERating:
				url.ReAllocL( url.Length() + KTopRated().Length() + KSlash().Length() );
				url.Append( KTopRated() );
				url.Append( KSlash() );		
			break;

			case ERelevance:
			default:
			break;
		}

	url.ReAllocL( url.Length() + num.Length() + KSlash().Length() );
	url.Append( num );
	url.Append( KSlash() );

	HBufC* ret = url.AllocL();
	CleanupStack::PopAndDestroy( &url );

	return ret;
	}
	

TInt CPlugin::ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler )
	{
	
	TInt	count = 0;
	TInt indexStart = 0 , indexEnd = 0;
	RBuf response;
	
	 
	indexStart = aResponse.Find(KUlFlagStart);
 	if ( indexStart == KErrNotFound )
		{
		indexStart = aResponse.Find(KUlFlagStart1);
	 	if ( indexStart == KErrNotFound )
	 		{
	 		return KErrNotSupported;
	 		}
		}

	TPtrC8	remainingData = aResponse.Mid(indexStart);
	indexEnd	 = remainingData.Find(KUlFlagEnd);

	CleanupClosePushL( response );

	response.Create( indexEnd + KUlFlagEnd().Length() );
	response.Copy( remainingData.Left(indexEnd + KUlFlagEnd().Length() ));

	TInt i , index = 0;
	TPtr tmpPtr =	response.MidTPtr(0);


		while( (i = tmpPtr.Find(KLiFlagStart)) != KErrNotFound )
		{
		
			MVideoEntry* videoEntry = aHandler.AddVideoEntryL();
			
			TInt end = tmpPtr.Find( KLiFlagEnd );
			TPtr block = tmpPtr.MidTPtr( i, end - i + 1 );
			 
		//VideoUrl and VideoId				 
				i = block.Find(KVideo);
				block = block.Mid(i + KVideo().Length());// + 1);
				index = block.Locate('"');
				RBuf VideosUrl;
				CleanupClosePushL( VideosUrl ); 
				VideosUrl.Create( KUrlMetacafe().Length() + KWatch().Length() + index ); 
		 			VideosUrl.Copy( KUrlMetacafe() );
		 			VideosUrl.Append( KWatch() );
		 			VideosUrl.Append( block.Left(index) );
		 			videoEntry->SetUrlL( VideosUrl );
		
				RBuf VideoId;
				CleanupClosePushL( VideoId );	 			 			
				GetVideoId( VideosUrl, VideoId );
		 			videoEntry->SetVideoIdL(VideoId );
				CleanupStack::PopAndDestroy( &VideoId );
				CleanupStack::PopAndDestroy( &VideosUrl );


			//Title	
	 				i = block.Find(KTitle);
				block = block.Mid(i + KTitle().Length() + 1);		 			
		 			index = block.Locate('"');
				RBuf8 Title;
				CleanupClosePushL( Title );	 		 
					Title.Create( index ); 
		 			Title.Append( block.Left(index) );

				HBufC* mediaTitle = CPluginUtils::ConvertUtfToUnicodeL( Title );
				CleanupStack::PushL( mediaTitle );
				videoEntry->SetTitleL( *mediaTitle );
				CleanupStack::PopAndDestroy( mediaTitle );

				CleanupStack::PopAndDestroy( &Title );

		//Thumb
 				i = block.Find(KThumb);
				block = block.Mid(i + KThumb().Length() + 1);		 			
		 			index = block.Find(_L(".jpg")) + 4;
				RBuf Thumb;
				CleanupClosePushL( Thumb );	 		 
					Thumb.Create( index ); 
		 			Thumb.Append( block.Left(index) );
				videoEntry->SetThumbnailUrlL( Thumb );
				CleanupStack::PopAndDestroy( &Thumb );	 			
		 		
		//Duration
	 				i = block.Find(KDurationStart1);
	 				if(i != KErrNotFound)
				{
					block = block.Mid(i + KDurationStart1().Length() + 1);									 			 
					index = block.Find(KDurationEnd1); 
					RBuf Duration;
					CleanupClosePushL( Duration);	 		 
						Duration.Create( index ); 
			 			Duration.Append( block.Left(index ) );	

					videoEntry->SetDuration( ConvertStringTimeToSecond( Duration ) );
			 			CleanupStack::PopAndDestroy( &Duration );
				}
				else
			{
 				i = block.Find(KDurationStart);
 				if(i != KErrNotFound)
				{
					block = block.Mid(i + KDurationStart().Length() + 1);									 			 
					index = block.Find(KDurationEnd);
					RBuf Duration;
					CleanupClosePushL( Duration);	 		 
						Duration.Create( index ); 
			 			Duration.Append( block.Left(index ) );	

					videoEntry->SetDuration( ConvertStringTimeToSecond( Duration ) );
			 			CleanupStack::PopAndDestroy( &Duration );
				}
 				else
 					videoEntry->SetDuration( 0 );
			}
				
		//Rating
				videoEntry->SetAverageRating( 0.0 );
	 				i = block.Find(KRatingStart);
	 				if( i != KErrNotFound )
	 					{
	 					block = block.Mid(i + KRatingStart().Length() );									 			 
	 					index = block.Find(KRatingEnd);
	 					if(index != KErrNotFound)
	 						{
	 						RBuf Rating;
	 						CleanupClosePushL( Rating );	 		 
	 						Rating.Create( index ); 
	 						Rating.Append( block.Left(index ) );						
	 						videoEntry->SetAverageRating( ConvertStringToTReal( Rating ) );
	 						CleanupStack::PopAndDestroy( &Rating );					
	 						}
	 					}
 				else
 					{
 		 				i = block.Find(KRatingStart1);
 		 				if( i != KErrNotFound )
 		 					{
 		 					block = block.Mid(i + KRatingStart1().Length() );									 			 
 		 					index = block.Find(KRatingEnd);
 		 					if(index != KErrNotFound)
 		 						{
 		 						RBuf Rating;
 		 						CleanupClosePushL( Rating );	 		 
 		 						Rating.Create( index ); 
 		 						Rating.Append( block.Left(index ) );						
 		 						videoEntry->SetAverageRating( ConvertStringToTReal( Rating ) );
 		 						CleanupStack::PopAndDestroy( &Rating );					
 		 						}
 		 					}
 					}
		//Views
				i = block.Find(KViewsStart);
 				if(i != KErrNotFound)
 					{
					block = block.Mid(i + KViewsStart().Length() );									 			 
					index = block.Find(KViewsEnd);				
					RBuf Views;
					CleanupClosePushL( Views );	 		 
					Views.Create( index ); 
		 			Views.Append( block.Left(index ) );						
					videoEntry->SetViewCount( ConvertStringViewCountToInt(Views) );
		 			CleanupStack::PopAndDestroy( &Views );
 					}
				else
					{
					i = block.Find(KViewsStart1);
	 				if(i != KErrNotFound)
	 					{
						block = block.Mid(i + KViewsStart1().Length() );									 			 
						index = block.Find(KViewsEnd);				
						RBuf Views;
						CleanupClosePushL( Views );	 		 
						Views.Create( index ); 
			 			Views.Append( block.Left(index ) );						
						videoEntry->SetViewCount( ConvertStringViewCountToInt(Views) );
			 			CleanupStack::PopAndDestroy( &Views );
	 					}
	 				else
	 					videoEntry->SetViewCount( 0 );
					}
		 		
		 	//Author
_LIT(KAuthorBlockStart, "class=\"ItemSubmitter\">");
_LIT(KAuthorBlockStart1, "class=\"By\"");

			i = block.Find( KAuthorBlockStart );
			if( i == KErrNotFound )
				i = block.Find( KAuthorBlockStart1 );
			if( i == KErrNotFound )
				{
				videoEntry->SetAuthorNameL( _L("") );
				}
			else
				{
				block = block.Mid( i );
				i = block.Find(KTitle);
				if(i != KErrNotFound)
					{
					block = block.Mid(i + KTitle().Length() + 1);									 			 
					index = block.Locate('"');				
					RBuf Author;
					CleanupClosePushL(Author);					 		 
					Author.Create( index ); 
					Author.Append( block.Left(index ) );						
					videoEntry->SetAuthorNameL( Author);
					CleanupStack::PopAndDestroy( &Author );
					}
				else
					{
					videoEntry->SetAuthorNameL( _L("") );
					}
				}
			count++;
			tmpPtr = tmpPtr.Mid( end + 1 );
		}
	
	CleanupStack::PopAndDestroy( &response );
	return count;
	}
	
_LIT8(KMediaUrl,"mediaURL=");
_LIT8(KFlv,".flv");
_LIT8(KGdaKey, "gdaKey" );
_LIT( KGdaParam, "?__gda__");
_LIT( KUrlStart, "http://v.mccont.com/ItemFiles/" );

HBufC* CPlugin::VideoUrlL( const TDesC8& aResponse, TBool& aLast )
	{
	aLast = ETrue;
	HBufC* handle = NULL;
	
	TInt positionStart = 0, positionEnd = 0;
	positionStart = aResponse.Find( KMediaUrl );

	if( positionStart != KErrNotFound )
		{
		TPtrC8 tmpPtr = aResponse.Mid( positionStart + KMediaUrl().Length() );
		positionEnd = tmpPtr.Find( KFlv );


		positionStart = aResponse.Find( KGdaKey );
		if( positionStart != KErrNotFound )
			{
			const TPtrC8 tmp = tmpPtr.Left( positionEnd + KFlv().Length() );

			TPtrC8 tmpPtr = aResponse.Mid( positionStart + KGdaKey().Length() );
			positionEnd = tmpPtr.Locate( '"' );
			TInt positionEnd1 = tmpPtr.Locate( '&' );
			if( positionEnd1 < positionEnd )
				positionEnd = positionEnd1;
			const TPtrC8 tmp2 = tmpPtr.Left( positionEnd );
		
			handle = HBufC::NewL( KUrlStart().Length() + tmp.Length() + tmp2.Length() + KGdaParam().Length() );

//			for(TInt i=0;i<KUrlStart().Length();i++)
//				{
//		 		handle->Des().Append( KUrlStart()[i] );
//				}

			HBufC8* decoded = EscapeUtils::EscapeDecodeL( tmp );
			CleanupStack::PushL( decoded );

//			handle->Append( *decoded );
			for(TInt i=0;i<decoded->Length();i++)
				{
				if( (*decoded)[i] != '\\' )
			 		handle->Des().Append( (*decoded)[i] );
				}

			CleanupStack::PopAndDestroy( decoded );
			
			for(TInt i=0;i<KGdaParam().Length();i++)
				{
		 		handle->Des().Append( KGdaParam()[i] );
				}

//			handle->Des().Append( KGdaParam() );

			for(TInt i=0;i<tmp2.Length();i++)
				{
		 		handle->Des().Append( tmp2[i] );
				}
			
			}
		}
	return handle;
	}


void CPlugin::GetVideoId( RBuf& aVideosUrl, RBuf& aVideoId )
{
	TInt i = aVideosUrl.Find( KWatch() );
	TPtr tPtr = aVideosUrl.MidTPtr( i +	KWatch().Length() );
	i = tPtr.Locate('/'); 
	aVideoId.Create( i );
	aVideoId.Copy( tPtr.Left(i) ); 
}

	
TReal CPlugin::ConvertStringToTReal( TDesC& aRes )
{
	TReal number = 0;
	TLex lex;	
					
	lex.Assign(aRes);					
					 		
	if( lex.Val(number) == KErrNone )
			return number;	
		else
			return 0.0;
}

	
TInt CPlugin::ConvertStringTimeToSecond( TDesC& aRes )
 {
	TInt i, minute = 0, seconds = 0, sec = 0;
	TLex lex_minute, lex_second;
	
	RBuf n_min;
	CleanupClosePushL(n_min);
	n_min.Create(2);	

	RBuf n_sec;
	CleanupClosePushL( n_sec);
	n_sec.Create(2);	

		for( i = 0; (( i < aRes.Length() ) && aRes[i] != ':'); i++ )
				n_min.Append(aRes[i]);
	++i;
			
	while( i < aRes.Length() )
		n_sec.Append(aRes[i++]);
					
	lex_minute.Assign(n_min);					
	lex_second.Assign(n_sec);	
	
	if( (lex_minute.Val(minute) == KErrNone) && (lex_second.Val(sec) == KErrNone) )
			seconds = (minute * 60) + sec;
	else
		seconds = 0;	
	
	CleanupStack::PopAndDestroy( &n_sec );	
	CleanupStack::PopAndDestroy( &n_min );				
	
	return seconds;
 }
 

 TInt CPlugin::ConvertStringViewCountToInt(TDesC& aRes)
 {
	TInt i, number = 0;
	TLex lex;	

	RBuf numb;
	CleanupClosePushL( numb );
	numb.Create(aRes.Length());	

		for( i = 0; i < aRes.Length(); i++ )
		{		
			if( aRes[i] != ',' )
				numb.Append(aRes[i]);			
		}
					
	lex.Assign(numb);					
			
	if( lex.Val(number) != KErrNone )
		number = 0;	
		
	CleanupStack::PopAndDestroy(&numb);
	
	return number;
	}
	
TInt CPlugin::SearchResultsPerPage()
	{
	return iResultsPerPage;
	}
	
HBufC* CPlugin::UserVideosUrlL( const TDesC& /*aBaseUrl*/, TInt /*aMaxResults*/, TInt /*aStartIndex*/, TOrderBy aOrderBy )
	{	
	return NULL;
	}

HBufC* CPlugin::RelatedVideosUrlL( const TDesC& /*aBaseUrl*/, TInt /*aMaxResults*/, TInt /*aStartIndex*/, TOrderBy aOrderBy )
	{
	
	return NULL;
	}

//login
_LIT( KMetacafeLoginUrl, "https://secure.metacafe.com/account/login" );
_LIT( KMetacafeUsername, "&email=" );
_LIT( KMetacafePassword, "&password=" );
_LIT( KMetacafeRemember, "&remember=on" );
_LIT( KMetacafePage, "&pageToLoad=1" );

void CPlugin::LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword )
	{
	aUrl = KMetacafeLoginUrl().AllocL();
	aContentType = KContentTypeUrlEncoded().AllocL();

	HBufC8* body = HBufC8::NewL( KMetacafeUsername().Length() + aUserName.Length() + KMetacafePassword().Length() + aPassword.Length() + KMetacafeRemember().Length() + KMetacafePage().Length() );
	body->Des().Copy( KMetacafeUsername() );
	body->Des().Append( aUserName );
	body->Des().Append( KMetacafePassword() );
	body->Des().Append( aPassword );
	body->Des().Append( KMetacafeRemember() );
	body->Des().Append( KMetacafePage() );
	aBody = body;
	}

TBool CPlugin::ParseLoginResponseL( TDesC8& aResponse )
	{
	return ETrue;
	}

//upload video
HBufC* CPlugin::UploadMovieUrlL()
	{
	return NULL;
	}

TInt CPlugin::ParseUploadMovieResponseL( const TDesC8& aResponse )
	{
	return KErrNone;
	}

TBool CPlugin::UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	return EFalse;
	}

TInt CPlugin::ParseUploadMovieResponseStep1L( const TDesC8& aResponse )
	{
	return KErrNone;
	}

TBool CPlugin::UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData )
	{
	return ETrue;
	}

TInt CPlugin::ParseUploadMovieResponseStep2L( const TDesC8& aResponse )
	{
	return KErrNone;
	}


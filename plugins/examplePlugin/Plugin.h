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

/**
  How to creat new plugin:
  - You will need 2 UIDs, one for dll and one for ecom implementation.
    Search for my_dll_uid and my_implementation_uid and replace them with your uids.
    Note, that the dll resource file should be renamed too, to match your dll uid value.
  - Search for "Example" text end replace it with your plugin name. This includes
    dll name, mbm/mif names, pkg files (and its content).
  - Fill in informations in dll resource (.rsc) file. Uid, plugin name and plugin description should be changed
    there.
  - Provide logo. Logo should be 24bpp with 24bpp mask.
  - Implement all required methods from plugin interface in Plugin.cpp.
    See below for the decription of each method.

    The most important methods are SearchUrlL() and ParseSearchResponseL(). FeatureUrlL(), UserVideosUrlL() and RelatedVideosUrlL()
    methods do not have to be implemented, as long as corresponding flags in Capabilities() method are not set.
    Same thing applies to the the login support and videos uploading - the methods can be left unimplemented as long
    as the capabilities are not enabled.
    Once you have these 2 methods ready and working, you should be able to build the plugin, create sis file, install the plugin and see it on
    the plugins list in the application. You should also be able to perform search using this plugin and see the search results.
    If the url to .flv file is direct and flag for that is set in Capabilities() you should also be able to watch the video.
    If url is indirect you have to implement VideoUrlL() method which provides direct url to the .flv file.
*/

class CPlugin : public CPluginInterface
	{

public:
	static CPlugin* NewL();
	virtual ~CPlugin();

private:
	CPlugin();
	void ConstructL();

public:
	/**
	Provides user readable plugin name. The name is used on the plugin selection list,
	and on navipane to indicate currently selected plugin.
	Keep it short.
	@returns name of the plugin.
	*/
	TPtrC16 Name();

	/**
	Creates bitmap & mask. Bitmap is used on the plugin selection list.
	This method _must_ create valid bitmap&mask.

	Ownership is transfered to the caller.

	@param aBitmap on return contains logo bitmap.
	@param aMask on return contains logo mask.
	*/
	void IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask );

	/**
	Plugin capabilities. A combination of KCaps* flags that describe what kind of features this plugin supports.
	
	<B>KCapsRecentlyUploaded</B> - Set this flag if the site can provide list of recently uploaded clips.

	<B>KCapsFeatured</B> - Set this flag if the site can provide list of featured clips.

	<B>KCapsTopRated</B> - Set this flag if the site can provide list of top rated clips.

	<B>KCapsMostViewed</B> - Set this flag if the site can provide list of most watched clips.

	<B>KCapsUserClips</B> - Set this flag if there is a way to get a list of clips uploaded by user.

	<B>KCapsRelatedClips</B> - Set this flag if there is a way to get a list of clips related to the original video.

	<B>KCapsConstResultsPerPage</B> - Set this flag if tere is no way to specify how many videos should be returned in one search call.

	<B>KCapsAccessVideoById</B> - Set this flag if there is a way to build an url pointing to a video from videoid.

	<B>KCapsVideoUrlIsDirect</B> - Set this flag if url to video is direct. Url is considered to be direct if it either points to a video:
	http://somehost.com/file.flv or to a script/file that will redirect (via 3xx responses) to a server with .flv file.
	Indirect url is an url to a xml/html file that has to be downloaded and parsed (see VideoUrlL() method) before direct
	url to the video can be	obtained.

	<B>KCapsTimeFrame</B> - Set this flag if the site supports selecting a time period in
	toprated/mostviewed calls.

	<B>KCapsLogin</B> - Set if the site supports loging in and plugin login functions are implemented.

	<B>KCapsVideoUpload</B> - Set this flag if plugin implements methods for video uploading.

	<B>KCapsAllowSavingVideos</B> - Set this flag if you want to allow user to save movies on his mobile phone.
	You may want to disable this feature if the site youre working with do not allow to save files locally.

	<B>KCapsVideoDownload</B> - Set if the plugin only supports video uploading, and video playback is not possible.

	@returns capabilities.
	*/
	TInt Capabilities();
	
	/**
	This method is used to decide if this plugin can handle given url.
	@param aUrl url to be checked may or may not start with http://
	@returns ETrue if this plugin can handle this url, EFalse otherwise.
	*/
	TBool CanHandleUrlL( const TDesC& aUrl );

	/**
	Provides search url for given site feature.
	Feature can be one of:

	ETopRatedClips, 
	ENewClips,
	EMostViewedClips,
	EFeaturedClips,

	This method will only be called if capability flag for a feature is set (see Capabilities method).
	Returned url should contain order parameters as well as start/number of results/page number (depends on api provided by video site).

	Ownership is transfered to the caller.

	Response received from server using provided url will be send to ParseSearchResponseL().
	
	@param aFeature feature for which url should be prepared.
	@param aMaxResults number of results video site should return. Ignore if KCapsConstResultsPerPage is set.
	@param aStartIndex index of first video that should be included in response. Starts with 1.
	@param aOrderBy Defines how should the results be ordered. If video site doesn't provide a way to use specified sort type use EViewCount. Or ignore completly.
	@param aTime Time period. One of TPeriod. Ignore this parameter if site you are writting plugin for doesn't support that kind of parameters.
	@returns An url to a video site for given feature.
	*/
	HBufC* FeatureUrlL( TFeature aFeature, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime );

	/**
	Provides url that allows to list videos uploaded by particullar user.

	This method will only be called if capability flag KCapsUserClips set (see Capabilities method).
	Returned url should contain order parameters as well as start/number of results/page number (depends on api provided by video site).

	Ownership is transfered to the caller.

	Response received from server using provided url will be send to ParseSearchResponseL().
	
	@param aBaseUrl This is a string that was set via SetAuthorVideosUrlL() call in MVideoEntry. It is up to the plugin author how to handle this string.
	@param aMaxResults number of results video site should return. Ignore if KCapsConstResultsPerPage is set.
	@param aStartIndex index of first video that should be included in response. Starts with 1.
	@param aOrderBy Defines how should the results be ordered. If video site doesn't provide a way to use specified sort type use EViewCount. Or ignore completly.
	@returns An url to a video site for given feature.
	*/
	HBufC* UserVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );

	/**
	Provides url that allows to list videos that are related to particullar video.
	The original video is specified by aBaseUrl.

	This method will only be called if capability flag KCapsRelatedClips set (see Capabilities method).
	Returned url should contain order parameters as well as start/number of results/page number (depends on api provided by video site).

	Ownership is transfered to the caller.

	Response received from server using provided url will be send to ParseSearchResponseL().
	
	@param aBaseUrl This is a string that was set via SetRelatedUrlL() call in MVideoEntry. It is up to the plugin author how to handle this string.
	@param aMaxResults number of results video site should return. Ignore if KCapsConstResultsPerPage is set.
	@param aStartIndex index of first video that should be included in response. Starts with 1.
	@param aOrderBy Defines how should the results be ordered. If video site doesn't provide a way to use specified sort type use EViewCount. Or ignore completly.
	@returns An url to a video site for given feature.
	*/
	HBufC* RelatedVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );

	/**
	Creates an url from video id.

	This method will only be called if capability flag KCapsAccessVideoById set (see Capabilities method).

	Ownership is transfered to the caller.

	@param aId video id for which an url should be created.
	@returns An url to a specified video.
	*/
	HBufC* CreateUrlFromIdL( const TDesC& aId );

	/**
	Provides number of search results per page. Only if KCapsConstResultsPerPage is set.
	If the site supports TInt aMaxResults, TInt aStartIndex parameters of all the url methods (ie it is possible to specify start
        video and number of videos per search call) return value of this method will be ignored. Otherwise it will be used to determine
        how many videos will be presented to the user, and when there is no more videos available.
	When this method returns value !=0 and KCapsConstResultsPerPage is set the parameter aStartIndex in all the url methods
	will contain correct value which can be used to determine which search result should be obtained from the server.
	@returns Number of results per page. 0 if this plugin is able to use non const number of search results.
	*/
	TInt SearchResultsPerPage();

	/**
	Provides url that allows to list videos that matches given string.

	Returned url should contain order parameters as well as start/number of results/page number (depends on api provided by video site).

	Ownership is transfered to the caller.

	Response received from server using provided url will be send to ParseSearchResponseL().
	
	@param aString tags the search url should be created for. This string is not url escaped, so if your site reuiqres it to be escaped -> convert it to utf8 and escape before using it!
	@param aMaxResults number of results video site should return. Ignore if KCapsConstResultsPerPage is set.
	@param aStartIndex index of first video that should be included in response. Starts with 1.
	@param aOrderBy Defines how should the results be ordered. If video site doesn't provide a way to use specified sort type use EViewCount. Or ignore completly.
	@returns An url to a video site for given feature.
	*/
	HBufC* SearchUrlL( const TDesC& aString, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy );

	/**
	Parses response received from server, adds all the videos found in the response to the results list via aHandler.

	aHandler provides only one method, MVideoEntry* AddVideoEntryL(); returned interface can be used to set all the video informations for
        particular clip.

	Plugin _MUST_ call these methods:

	void SetVideoIdL( const TDesC& aId );<BR>
	void SetTitleL( const TDesC& aTitle );<BR>
	void SetUrlL( const TDesC& aUrl );<BR>
	void SetThumbnailUrlL( const TDesC& aUrl );<BR>


	Other methods do not have to be called, as default values are set in the main application.
	But of course providing these additional informations would ne a nice thing to do.

	void SetRelatedUrlL( const TDesC& aUrl ); aUrl provided for this method will later on be send to RelatedVideosUrlL()
                                                  as aBaseUrl.

	void SetAuthorVideosUrlL( const TDesC& aUrl ); aUrl provided for this method will later on be send to UserVideosUrlL()
                                                  as aBaseUrl.

	You may set whathever you want there, as long as it will be possible to create a valid url from that information later on in
	the RelatedVideosUrlL()/UserVideosUrlL() methods.

	@returns Number of videos found, results per page. 0 if this plugin is able to use non const number of search results.
	*/
	TInt ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler );

	/**
	This method is called only if flag KCapsVideoUrlIsDirect (see Capabilities() method) is not set (ie the url to video is not direct).
	Plugin should parse provided response and return url that points either directly to the flv file or to another page where such url
	can be found. In the later case aLast parameter should be set to EFalse. If the returned url is pointing to the flv file
	the aLast parameter should be set to ETrue.

	Ownership is transfered to the caller.
	
	@param aResponse string containing response from a server that might contain url to a flv video file.
			This method should parse the string and return new url.
	@param aLast set to ETrue if the returned url points to video file, EFalse if it points to another page
			that might contain such url.
	@returns Url to the video file. NULL if the plugin was unable to find an url in provided response.
	*/
	HBufC* VideoUrlL( const TDesC8& aResponse, TBool& aLast );

	/**
	Creates url and body for a login call using provided credentials. Might be left unimplemented if the KCapsLogin flags not set.
	Ownership of the aBody, aUrl, aContentType is transfered to the caller.
	
	@param aUrl url the main application should call to login into the site.
	@param aBody body of the post call that will be sent to the site.
	@param aContentType content type that should be used with the login call.
	@param aUserName user name for the login call. Provided by user in tha main application.
	@param aPassword password for the login call provided by user.
	*/
	void LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword );

	/**
	Parses response from the server after calling the url provided in the LoginUrlAndBodyL() function.
	@param aResponse response received from the server.
	@returns ETrue if user was logged succesfully EFalse if login failed.
	*/
	TBool ParseLoginResponseL( TDesC8& aResponse );

	/**
	@returns url to a page where user can upload movie. Ownership is transfered to the caller.
	*/
	HBufC* UploadMovieUrlL();

	/**
	Parses the response received from a call to the url provided by UploadMovieUrlL() function.
	@returns KErrNone if the response is ok, any other error if the response is not valid.
	*/
	TInt ParseUploadMovieResponseL( const TDesC8& aResponse );

	/**
	The actual movie uploading is done in two steps. Depending on the site the protocol might be:
	1) upload the video first, the in second call provide informations about the video
	2)first provide video title and other info, then in second call provide the video file itself.
	@param aUrl url where the call to the server should be made
	@param aBodyStart first part of the body to be used in the call to the server. Multipart encoded.
	@param aBodyEnd second part of the body. The file data will be inserted between aBodyStart and aBodyEnd. aBodyEnd might be NULL.
	@param aContentType content type to be used with this call.
	@param aData interface that provides informations abou the video upload. Title, description, filename, etc.
	@returns ETrue if this call should contain actual video information embedded in the post. EFalse if the video data should not be included.
	*/
	TBool UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData );

	/**
	Parse response from the server after executing the first part of movie upload process. Store any important imformations that might
	be used in the second part of the movie upload process.

	@param aResponse response from the server.
	@returns KErrNone if there was no errors, any other error if there was something wrong with hte response.
	*/
	TInt ParseUploadMovieResponseStep1L( const TDesC8& aResponse );

	/**
	Second part of the movie upload process.
	@param aUrl url where the call to the server should be made
	@param aBodyStart first part of the body to be used in the call to the server. Multipart encoded.
	@param aBodyEnd second part of the body. The file data will be inserted between aBodyStart and aBodyEnd. aBodyEnd might be NULL.
	@param aContentType content type to be used with this call.
	@param aData interface that provides informations abou the video upload. Title, description, filename, etc.
	@returns ETrue if this call should contain actual video information embedded in the post. EFalse if the video data should not be included.
	*/
	TBool UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData );

	/**
	Parse response from the server after executing the second part of movie upload process.

	@param aResponse response from the server.
	@returns KErrNone if there was no errors, any other error if there was something wrong with hte response.
	*/
	TInt ParseUploadMovieResponseStep2L( const TDesC8& aResponse );
	};  

#endif //_EXAMPLE_PLUGIN_H

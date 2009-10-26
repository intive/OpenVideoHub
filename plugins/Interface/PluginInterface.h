#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <e32base.h>
#include <ECom.h>

#include "PluginUids.h"

const TUid KPluginInterfaceUid = {KPluginInterfaceUidInt};

enum TFeature
	{
	ETopRatedClips = 0,
	ENewClips,
	EMostViewedClips,
	EFeaturedClips,
	EUserClips,
	ERelatedClips
	};

enum TOrderBy
	{
	EViewCount = 0,
	EUpdated,
	ERating,
	ERelevance,
	EIrrelevant
	};

enum TPeriod
	{
	EToday = 0,
	EThisWeek,
	EThisMonth,
	EAllTime,
	EPeriodIrrelevant
	};

enum TMovieCategory
	{
	EFilmAndAnimation = 1,
	ECarsAndVehicles = 2,
	EMusic = 10,
	EPetsAndAnimals = 15,
	ESports = 17,
	ETravelAndEvents = 19,
	EPeopleAndBlogs = 22,
	EComedy = 23,
	EEntertainment = 24,
	ENewsAndPolitics = 25,
	EHowtoAndStyle = 26,
	EEducation = 27,
	EScienceAndTechnology = 28,
	ENonprofitsAndActivism = 29
	};

class MVideoEntry
{
public:
	virtual void SetVideoIdL( const TDesC& aId ) = 0;
	virtual void SetTitleL( const TDesC& aTitle ) = 0;
	virtual void SetUrlL( const TDesC& aUrl ) = 0;
	virtual void SetThumbnailUrlL( const TDesC& aUrl ) = 0;
	virtual void SetDuration( TInt aSeconds ) = 0;
	virtual void SetAverageRating( TReal32 aAverage ) = 0;
	virtual void SetViewCount( TInt aCount ) = 0;
	virtual void SetAuthorNameL( const TDesC& aName ) = 0;
	virtual void SetRelatedUrlL( const TDesC& aUrl ) = 0;
	virtual void SetAuthorVideosUrlL( const TDesC& aUrl ) = 0;
};

class MVideoData
{
public:
	virtual TDesC& Title() = 0;
	virtual TDesC& Tags() = 0;
 	virtual TDesC& Description() = 0;
	virtual TMovieCategory Category() = 0;
 	virtual TDesC& Filename() = 0;
	virtual TBool Public() = 0;
	virtual TReal64 Latitude() = 0;
	virtual TReal64 Longitude() = 0;
};

class MVideoEntryHandler
{
public:
	virtual MVideoEntry* AddVideoEntryL() = 0;
};

//capabilities flags
#define KCapsRecentlyUploaded		( 1 << 0 ) //plugin can retrieve new videos
#define KCapsFeatured			( 1 << 1 )
#define KCapsTopRated			( 1 << 2 )
#define KCapsMostViewed			( 1 << 3 )
#define KCapsUserClips			( 1 << 4 )
#define KCapsRelatedClips		( 1 << 5 )
#define KCapsConstResultsPerPage	( 1 << 6 ) //there is no way to change number of videos returned in one search call.
#define KCapsAccessVideoById		( 1 << 7 )
#define KCapsVideoUrlIsDirect		( 1 << 8 ) //url to video is direct
#define KCapsTimeFrame			( 1 << 9 ) //TPeriod is supported in toprated/mostviewed calls

#define KCapsLogin			( 1 << 10 ) //Plugin supports login operation
#define KCapsVideoUpload		( 1 << 11 ) //Plugin supports video uploading

#define KCapsAllowSavingVideos		( 1 << 12 ) //user can store videos from this site locally

#define KCapsVideoDownload		( 1 << 13 ) //Plugin supports video downloading/playback

class CFbsBitmap;
class CPluginInterface : public CBase
{
public:

	static CPluginInterface* NewL( TUid aUid );
	static CPluginInterface* NewL( const TDesC8& aMatch, TUid aUid = KPluginInterfaceUid );
	virtual ~CPluginInterface();

	static void ListAllImplementationsL( RImplInfoPtrArray& aImplInfoArray );

	virtual TPtrC16 Name() = 0;
	virtual void IconL( CFbsBitmap*& aBitmap, CFbsBitmap*& aMask ) = 0;

	virtual TInt Capabilities() = 0;
	
	virtual TBool CanHandleUrlL( const TDesC& aUrl ) = 0;

	virtual HBufC* FeatureUrlL( TFeature aFeature, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy, TPeriod aTime ) = 0;
	virtual HBufC* UserVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy ) = 0;
	virtual HBufC* RelatedVideosUrlL( const TDesC& aBaseUrl, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy ) = 0;
	virtual HBufC* CreateUrlFromIdL( const TDesC& aId ) = 0;

	virtual TInt SearchResultsPerPage() = 0;

	virtual HBufC* SearchUrlL( const TDesC& aString, TInt aMaxResults, TInt aStartIndex, TOrderBy aOrderBy) = 0;
	virtual TInt ParseSearchResponseL( const TDesC8& aResponse, MVideoEntryHandler& aHandler ) = 0;

	virtual HBufC* VideoUrlL( const TDesC8& aResponse, TBool& aLast ) = 0;

//
	virtual void LoginUrlAndBodyL( HBufC*& aUrl, HBufC8*& aBody, HBufC8*& aContentType, const TDesC& aUserName, const TDesC& aPassword ) = 0;
	virtual TBool ParseLoginResponseL( TDesC8& aResponse ) = 0;
	
	virtual HBufC* UploadMovieUrlL() = 0;
	virtual TInt ParseUploadMovieResponseL( const TDesC8& aResponse ) = 0;
	virtual TBool UploadMovieStep1L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData ) = 0;
	virtual TInt ParseUploadMovieResponseStep1L( const TDesC8& aResponse ) = 0;
	virtual TBool UploadMovieStep2L( HBufC*& aUrl, HBufC8*& aBodyStart, HBufC8*& aBodyEnd, HBufC8*& aContentType, MVideoData& aData ) = 0;
	virtual TInt ParseUploadMovieResponseStep2L( const TDesC8& aResponse ) = 0;

protected:
	inline CPluginInterface();

private:
	TUid iDtor_ID_Key;
};
	
#include "PluginInterface.inl"
	
#endif //PLUGIN_INTERFACE_H

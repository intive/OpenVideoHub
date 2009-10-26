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
	TInt ConvertStringTimeToSecond(TDesC& aRes);
	TInt ConvertStringViewCountToInt(TDesC& aRes);
	TReal ConvertStringToTReal( TDesC& aRes );
	//RBuf GetNameAuthor( TDesC& aUrl );
	void GetVideoId(RBuf& aVideoUrl, RBuf& aVideoId );

	TInt iResultsPerPage;
	};  

#endif //EXAMPLE_PLUGIN_H

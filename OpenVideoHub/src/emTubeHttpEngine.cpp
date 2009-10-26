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

#include <uri8.h>
#include <http.h>
#include <eikenv.h>
#include <stringpool.h>
#include <f32file.h>
#include <http\thttphdrfielditer.h>
#include <bautils.h>
#include <eikenv.h>
#ifndef EMTUBE_UIQ
#include <stringloader.h>
#endif
#include <EscapeUtils.h>
#include <centralrepository.h>	  // CRepository
#include <ProfileEngineSDKCRKeys.h> // CRepository

#include "emTubeResource.h"

#include "emTubeVideoEntry.h"
#include "emTubeProgress.h"
#include "emTubeAppUi.h"
#include "emTubeHttpEngine.h"
#include "emTubeTimeOutTimer.h"
#include "emTubeLog.h"
#include "emTubeTransferManager.h"
#include "emTubePlayerTimer.h"

_LIT8( KAccept, "*/*");
_LIT8( KCache, "no-cache");
_LIT8( KTxtUserAgent, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)" );
_LIT8( KConnection, "keep-Alive");

_LIT(  KAccessPointParam, "IAP\\Id" );

//filestream seems to be faster.
#define USE_FILE_STREAM

CEmTubeHttpEngine* CEmTubeHttpEngine::NewL( CEmTubePluginManager& aManager, TBool aRemoveRedirectFilter )
	{
	CEmTubeHttpEngine* self = CEmTubeHttpEngine::NewLC( aManager, aRemoveRedirectFilter );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeHttpEngine* CEmTubeHttpEngine::NewLC( CEmTubePluginManager& aManager, TBool aRemoveRedirectFilter )
	{
	CEmTubeHttpEngine* self = new (ELeave)CEmTubeHttpEngine( aManager );
	CleanupStack::PushL(self);
	self->ConstructL( aRemoveRedirectFilter );
	return self;
	}

CEmTubeHttpEngine::CEmTubeHttpEngine( CEmTubePluginManager& aManager ) : CActive(EPriorityIdle),
#ifdef __WINS__
										 iAccessPoint(3),
#else
										 iAccessPoint(0),
#endif
										 iPluginManager(&aManager),
										 iProfileId( -1 ),
										 iForceIAPSelectionDialog( EFalse ),
										 iForceCloseConnection( EFalse )
	{
	}

void CEmTubeHttpEngine::ConstructL( TBool aRemoveRedirectFilter )
	{
	iBody = CEmTubeHTTPDataSupplier::NewL();

	iResponse = HBufC8::NewL(0);

	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	iFsSession.Connect();

	SetHttpBufferSize( HTTP_BUFFER_SIZE );

	CActiveScheduler::Add(this);

	User::LeaveIfError(iSockServ.Connect());

	iTimer = CEmTubeTimeOutTimer::NewL( *this );

	iSession.OpenL();

//remove redirection filter
	if( aRemoveRedirectFilter )
		{
		RHTTPFilterCollection filtColl = iSession.FilterCollection();
		RStringF filterName = iSession.StringPool().StringF( HTTP::ERedirect, RHTTPSession::GetTable() );
		filtColl.RemoveFilter( filterName );
		}
	}

CEmTubeHttpEngine::~CEmTubeHttpEngine()
	{
	Cancel();

	delete iProgressObserver;

	delete iSearchString;
	delete iResponse;
	delete iUrl;

	if( iTimer )
		iTimer->Cancel();
	delete iTimer;

	if( iFileOpened )
		{
#ifndef USE_FILE_STREAM
		iFile.Close();
#else
		iFileStream.Close();
#endif
		}
	iFileOpened = EFalse;

	iFsSession.Close();

	delete iFileName;
	delete iMovieFileName;

	iHttpBuffer.Close();

	iSession.Close();
	iConn.Close();

	delete iBody;

	iSockServ.Close();
	}

void CEmTubeHttpEngine::HandleConnectionEventL( TInt /*aStage*/ )
	{
//aStage is ignored, as this method is only colled by connection monitor in selected cases.
Log("->HandleConnectionEventL");
Log(iRequestType);

	if( iRequest == CEmTubeHttpEngine::ERequestMovieDownload || iRequest == CEmTubeHttpEngine::ERequestMovieDownload2 )
		{
		RequestCanceledL( iRequest );
		HandleErrorL( CEmTubeHttpEngine::ECouldNotConnect );
		}

	CancelOperationL();

	iForceIAPSelectionDialog = ETrue;
	iForceCloseConnection = ETrue;
Log("<-HandleConnectionEventL");
	}

TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset )
	{
Log( "->TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset )" );
	TInt idx = iPluginManager->FindPlugin( aUrl );
	if( idx == KErrNotFound )
		{
		HandleErrorL( ENoPlugin );
		return -1;
		}
	else
		{
		iPluginUid = iPluginManager->Uid( idx );
		}

	delete iUrl;
	iUrl = HBufC8::NewL(aUrl.Length());
	iUrl->Des().Copy(aUrl);

	if( iRequest == ERequestMovieDownload && !(iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
		{
		delete iMovieFileName;
		iMovieFileName = aFileName.AllocL();

		iSaveToFile = EFalse;
		}
	else
		{
		iSaveToFile = ETrue;
		}

	iStartOffset = aStartOffset;
	iRequestType = ERequestGet;

	delete iFileName;
	iFileName = aFileName.AllocL();

	TInt ret = iUri.Parse(*iUrl);
	if( ret != KErrNone )
		{
		HandleErrorL( ERequestFailed );
		return ret;
		}
	else
		{
		TRAP( ret, IssueRequestL() )
		if(ret != KErrNone)
			{
			CloseRequestL();
			HandleErrorL( ret );
			return ret;
			}
		}
Log( "TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset )->" );
	return KErrNone;
	}

TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl )
	{
Log( "->TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset )" );
	TInt idx = iPluginManager->FindPlugin( aUrl );
	if( idx == KErrNotFound )
		{
		HandleErrorL( ENoPlugin );
		return -1;
		}
	else
		{
		iPluginUid = iPluginManager->Uid( idx );
		}

	delete iUrl;
	iUrl = HBufC8::NewL(aUrl.Length());
	iUrl->Des().Copy(aUrl);

	iSaveToFile = EFalse;
	iStartOffset = 0;
	iRequestType = ERequestGet;

	TInt ret = iUri.Parse(*iUrl);
	if( ret != KErrNone )
		{
		HandleErrorL(ERequestFailed);
		}
	else
		{
		TRAP( ret, IssueRequestL())
		if(ret != KErrNone)
			{
			CloseRequestL();
			HandleErrorL( ret );
			return ret;
			}
		}
Log( "TInt CEmTubeHttpEngine::StartGetRequestL( const TDesC& aUrl, const TDesC& aFileName, TInt aStartOffset )->" );
	return KErrNone;
	}

TInt CEmTubeHttpEngine::StartPostRequestL( const TDesC& aUrl )
	{
	TInt idx = iPluginManager->FindPlugin( aUrl );
	if( idx == KErrNotFound )
		{
		HandleErrorL( ENoPlugin );
		return -1;
		}
	else
		{
		iPluginUid = iPluginManager->Uid( idx );
		}

	delete iUrl;
	iUrl = HBufC8::NewL(aUrl.Length());
	iUrl->Des().Copy(aUrl);

	iSaveToFile = EFalse;
	iStartOffset = 0;
	iRequestType = ERequestPost;

	TInt ret = iUri.Parse(*iUrl);
	if( ret != KErrNone )
		{
		HandleErrorL(ERequestFailed);
		}
	else
		{
		TRAP( ret, IssueRequestL())
		if(ret != KErrNone)
			{
			CloseRequestL();
			HandleErrorL( ret );
			return ret;
			}
		}
	return KErrNone;
	}

void CEmTubeHttpEngine::CancelRequestL()
	{
Log( "->void CEmTubeHttpEngine::CancelRequestL()" );
	Cancel();

	if( iTransactionOpened )
		{
		MHFRunL( iTransaction, THTTPEvent::ECancel );
		}
	else
		{
		CloseRequestL();
		}

	if( iFileName )
		{
		if( BaflUtils::FileExists( iFsSession, *iFileName) )
			BaflUtils::DeleteFile( iFsSession, *iFileName );

		delete iFileName;
		iFileName = NULL;
		}
Log( "void CEmTubeHttpEngine::CancelRequestL()->" );
	}

void CEmTubeHttpEngine::AddHttpHeaderL( TInt aParam, const TDesC8& aValue )
	{
	RHTTPHeaders headers = iTransaction.Request().GetHeaderCollection();

	RStringPool stringPool = iSession.StringPool();
	RStringF valStr = stringPool.OpenFStringL(aValue);
	CleanupClosePushL(valStr);

	THTTPHdrVal headerVal(valStr);
	headers.SetFieldL(stringPool.StringF(aParam, RHTTPSession::GetTable()), headerVal);

	CleanupStack::PopAndDestroy(&valStr);
	}

void CEmTubeHttpEngine::RunL()
	{
Log( "->RunL" );
Log( iStatus.Int() );

	if( iStatus != KErrNone )
		{
		delete iProgressObserver;
		iProgressObserver = NULL;

		iConn.Stop();
		iConnectionStarted = EFalse;
		CloseRequestL();
		HandleErrorL( iStatus.Int() );
		}
	else
		{
Log("connection started");

		iConnectionStarted = ETrue;

		TUint32 accessPoint;
		TInt error;
		error = iConn.GetIntSetting( KAccessPointParam(), accessPoint );

Log( "ap:" );
Log( error );
Log( accessPoint );

		if( error == KErrNone )
			iAccessPoint = accessPoint;
		else
			iAccessPoint = 0;

		iAppUi->SetSelectedAccessPoint( iAccessPoint );
		SubmitTransactionL();
		}
Log( "RunL->" );
	}

void CEmTubeHttpEngine::DoCancel()
	{
	delete iProgressObserver;
	iProgressObserver = NULL;

	if( iConnectionStarted )
		iConn.Stop();
	iConnectionStarted = EFalse;

	if( iConnectionOpened )
		iConn.Close();
	iConnectionOpened = EFalse;
	}

void CEmTubeHttpEngine::AddCookiesL( RHTTPHeaders aHttpHeaders )
	{
	RStringF cookieField = iSession.StringPool().StringF(HTTP::ECookie, RHTTPSession::GetTable());

	RPointerArray<CEmTubeCookie>& cookies = iPluginManager->Cookies( iPluginUid );
	for(TInt i=0;i<cookies.Count();i++ )
		{
		aHttpHeaders.SetRawFieldL(cookieField, cookies[i]->DesC(), _L8(";"));
		}

	cookieField.Close();
	}


void CEmTubeHttpEngine::SetAccessPoint( TInt aAccessPoint )
	{
	if( iAccessPoint != (TUint)aAccessPoint )
		{
		iAccessPoint = aAccessPoint;

		delete iProgressObserver;
		iProgressObserver = NULL;

		if( iConnectionStarted )
			iConn.Stop();
		iConnectionStarted = EFalse;

		if( iConnectionOpened )
			iConn.Close();
		iConnectionOpened = EFalse;
		}
	}

TBool CEmTubeHttpEngine::IsAccessPointWLAN( TUint32 aAp )
	{
	CCommsDatabase* CommDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
	CleanupStack::PushL( CommDb );
	CCommsDbTableView* tableIAP = CommDb->OpenTableLC( TPtrC(IAP) );

	if ( tableIAP->GotoFirstRecord() == KErrNone )
		{
		do
			{
			TUint32 ap;
			tableIAP->ReadUintL( TPtrC(COMMDB_ID), ap );

			if( ap == aAp )
				{
				TUint32 bearerType;
				tableIAP->ReadUintL( TPtrC(IAP_BEARER), bearerType );

				CleanupStack::PopAndDestroy( tableIAP );
				CleanupStack::PopAndDestroy( CommDb );

				if( bearerType == 1 )
					return ETrue;
				else
					return EFalse;
				}
			}
		while( tableIAP->GotoNextRecord() == KErrNone );
		}

	CleanupStack::PopAndDestroy( tableIAP );
	CleanupStack::PopAndDestroy( CommDb );

	return EFalse;
	}

TBool CEmTubeHttpEngine::AccessPointExistsL( TInt aAp )
	{
	TBool found = EFalse;

	CCommsDatabase* CommDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
	CleanupStack::PushL( CommDb );
	CCommsDbTableView* tableIAP = CommDb->OpenTableLC( TPtrC(IAP) );
	if ( tableIAP->GotoFirstRecord() == KErrNone )
		{
			do
			{
			TUint32 ap;
			tableIAP->ReadUintL( TPtrC(COMMDB_ID), ap );

			if ( ap == (TUint)aAp )
				{
				found = ETrue;
				break;
				}
			}
		while( tableIAP->GotoNextRecord()==KErrNone );
		}
	CleanupStack::PopAndDestroy( tableIAP );
	CleanupStack::PopAndDestroy( CommDb );

	return found;
	}

void CEmTubeHttpEngine::SubmitTransactionL()
	{
	if( iRequestType == ERequestGet )
		{
		RStringF method = iSession.StringPool().StringF(HTTP::EGET, RHTTPSession::GetTable());
		iTransaction = iSession.OpenTransactionL(iUri, *this, method);
		}
	else
		{
		RStringF method = iSession.StringPool().StringF(HTTP::EPOST, RHTTPSession::GetTable());
		iTransaction = iSession.OpenTransactionL(iUri, *this, method);
		iBody->SetTransaction( iTransaction );
		iTransaction.Request().SetBody( *iBody );
		AddHttpHeaderL( HTTP::EContentType, iBody->ContentType() );

		AddHttpHeaderL( HTTP::EKeepAlive, _L8("300") );
		AddHttpHeaderL( HTTP::EAcceptCharset, _L8("ISO-8859-2,utf-8;q=0.7,*;q=0.7") );
		AddHttpHeaderL( HTTP::EAcceptLanguage, _L8("pl,en-us;q=0.7,en;q=0.3") );
		}
	iTransactionOpened = ETrue;

	RHTTPHeaders httpHeaders = iTransaction.Request().GetHeaderCollection();
	RStringF connField = iSession.StringPool().StringF(HTTP::EConnection, RHTTPSession::GetTable());
	httpHeaders.SetRawFieldL(connField, KConnection(), KNullDesC8() );
	connField.Close();

	AddCookiesL( httpHeaders );

	AddHttpHeaderL( HTTP::EUserAgent, KTxtUserAgent );
	AddHttpHeaderL( HTTP::EAccept, KAccept);
	AddHttpHeaderL( HTTP::ECacheControl, KCache );

#if 0 //TODO -> implememt resume
	if( iStartOffset )
		{
		TBuf8<64> num;
		num.Format( _L8("bytes=%d-"), iStartOffset );
		AddHttpHeaderL( HTTP::ERange, num ); //Range: bytes=92456745-
		}
#endif
	if( iRequestType == ERequestGet )
		{
		iTimer->Cancel();
		iTimer->After( 30 * 1000000, 0 );
		}

Log("submitting transaction" );
	iTransaction.SubmitL();
	}

TInt CEmTubeHttpEngine::ActiveProfileL()
	{
	TInt profileId;
	CRepository* repository = CRepository::NewL( KCRUidProfileEngine );
	repository->Get( KProEngActiveProfile, profileId );
	delete repository;

	return profileId;
	}

void CEmTubeHttpEngine::IssueRequestL()
	{
Log("->CEmTubeHttpEngine::IssueRequestL()");

	TInt profileId = ActiveProfileL();

	if( iAppUi->AccessPoint() && !AccessPointExistsL( iAppUi->AccessPoint() ) )
		{
		iAppUi->SetAccessPoint( 0 );
		iForceIAPSelectionDialog = ETrue;
		iForceCloseConnection = ETrue;
		}

	if ( iProfileId != -1 && iProfileId != profileId && profileId == KOfflineProfile )
		{
		iForceIAPSelectionDialog = ETrue;
		iForceCloseConnection = ETrue;
		}
	iProfileId = profileId;

	if ( iAppUi->AccessPoint() && profileId == KOfflineProfile && !IsAccessPointWLAN( iAppUi->AccessPoint() ) )
		{
		iAppUi->SetAccessPoint( 0 );
		iForceIAPSelectionDialog = ETrue;
		iForceCloseConnection = ETrue;
		}

	if( iForceCloseConnection )
		{
		delete iProgressObserver;
		iProgressObserver = NULL;

		if( iConnectionStarted )
			iConn.Stop();
		iConnectionStarted = EFalse;

		if( iConnectionOpened )
			iConn.Close();
		iConnectionOpened = EFalse;
		iForceCloseConnection = EFalse;
		}

	if( !iForceIAPSelectionDialog && iAppUi->AccessPoint() )
		{
		SetAccessPoint( iAppUi->AccessPoint() );
		}
	else if( !iForceIAPSelectionDialog && iAppUi->SelectedAccessPoint() )
		{
		SetAccessPoint( iAppUi->SelectedAccessPoint() );
		}
	else
		{
		SetAccessPoint( 0 );
		}

	iForceIAPSelectionDialog = EFalse;

	if( !iConnectionOpened )
		{
		User::LeaveIfError(iConn.Open(iSockServ));

		iSession.ConnectionInfo().SetPropertyL(iSession.StringPool().StringF(HTTP::EHttpSocketServ, RHTTPSession::GetTable() ), THTTPHdrVal (iSockServ.Handle()) );
		TInt connPtr = REINTERPRET_CAST(TInt, &(iConn));
		iSession.ConnectionInfo().SetPropertyL(iSession.StringPool().StringF(HTTP::EHttpSocketConnection, RHTTPSession::GetTable() ), THTTPHdrVal (connPtr) );

		iConnectionOpened = ETrue;
		}

	if( !iConnectionStarted )
		{
		iConnPref.SetDirection( ECommDbConnectionDirectionOutgoing );
		if( iProfileId == KOfflineProfile )
			iConnPref.SetBearerSet( KCommDbBearerWLAN );
		else
			iConnPref.SetBearerSet( KCommDbBearerWcdma | KCommDbBearerCSD );
		if ( !iAccessPoint )
			{
			iConnPref.SetDialogPreference( ECommDbDialogPrefPrompt );
			}
		else
			{
			iConnPref.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
			}
		iConnPref.SetIapId( iAccessPoint );

		iProgressObserver = CEmTubeConnectionProgressObserver::NewL( this, iConn );

		iConn.Start( iConnPref, iStatus );
Log( "SetActive()" );
		SetActive();
		}
	else
		{
		SubmitTransactionL();
		}
Log( "issue request - out" );
	}

void CEmTubeHttpEngine::MHFRunL(RHTTPTransaction aTransaction,const THTTPEvent& aEvent)
	{
	iTimer->Cancel();

	switch(aEvent.iStatus)
	{
		case THTTPEvent::ERedirectedTemporarily:
			{
Log("redirected");
Log( aTransaction.Request().URI().UriDes() );

			if( !aTransaction.Request().URI().UriDes().Compare( _L8("http://www.t-zones.co.uk/cop/adult") ) )
				{
				MHFRunL(iTransaction, THTTPEvent::ECancel);
				HandleErrorL( EContentLock );
				}
			}
		break;

		case THTTPEvent::EGotResponseHeaders:
			{
			TInt status = aTransaction.Response().StatusCode();
			RHTTPHeaders headersCollection = aTransaction.Response().GetHeaderCollection();

Log( "status" );
Log( status );
Log( "uri" );
Log( aTransaction.Request().URI().UriDes() );

DumpHeadersL( aTransaction );

			if( status == 200 )
				{
				RStringF lengthString = iSession.StringPool().StringF(HTTP::EContentLength, RHTTPSession::GetTable());
				CleanupClosePushL(lengthString);
				THTTPHdrVal contentLength;
				headersCollection.GetField(lengthString, 0, contentLength);

				if(contentLength.KTIntVal == contentLength.Type())
					{
					iBodySize = contentLength.Int();
					}
				else
					{
					iBodySize = -1;
					}

Log( iBodySize );

				iDataReceived = 0;
Log("progress start");
Log( iBodySize );
				if( iRequest != ERequestMovieDownload || (iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
					{
					if(iProgress)
						{
						iDownloadSpeed = 0;
						iDownloadSpeedTicks = CVideoTimer::Time();
						iProgress->ProgressStart( iBodySize );
						}
					}
Log("progress start finished");

				CleanupStack::PopAndDestroy(&lengthString);
				delete iResponse;
				iResponse = NULL;
				}
			else if ( status == 301 || status == 302 || status == 303 )
				{
				TBuf<KMaxHeaderNameLen>  fieldName16;
				TBuf<KMaxHeaderValueLen> fieldVal16;

				RStringF location = iSession.StringPool().StringF(HTTP::ELocation,RHTTPSession::GetTable());
				RHTTPHeaders responseHeaders( aTransaction.Response().GetHeaderCollection() );
				THTTPHdrVal locationValue;
				TInt res = responseHeaders.GetField(location, 0, locationValue);
				if( res == KErrNone && iRequest == ERequestLogin )
					{
					RStringPool strP = aTransaction.Session().StringPool();
					RHTTPResponse response = aTransaction.Response();
					RStringF fieldName = iSession.StringPool().StringF(HTTP::ESetCookie, RHTTPSession::GetTable());
					THTTPHdrVal val;
					if (response.GetHeaderCollection().GetField(fieldName, 0, val) != KErrNotFound)
						{
						RStringF cookieValueName = iSession.StringPool().StringF(HTTP::ECookieValue, RHTTPSession::GetTable());
						RStringF cookieNameName = iSession.StringPool().StringF(HTTP::ECookieName, RHTTPSession::GetTable());

						if (val.StrF() == iSession.StringPool().StringF(HTTP::ECookie, RHTTPSession::GetTable()))
							{
							THTTPHdrVal cookieValue;
							THTTPHdrVal cookieName;

							TInt parts = response.GetHeaderCollection().FieldPartsL(fieldName);
							for (TInt i = 0; i < parts; i++)
								{
								response.GetHeaderCollection().GetParam(fieldName, cookieValueName, cookieValue, i);
								response.GetHeaderCollection().GetParam(fieldName, cookieNameName, cookieName, i);

								RString fieldNameStr = strP.String(cookieName.Str());
								RString fieldValStr = strP.String(cookieValue.Str());

								iPluginManager->AddCookieL( iPluginUid, fieldNameStr.DesC(), fieldValStr.DesC() );
								}
							}
						}
					AddCookiesL( aTransaction.Request().GetHeaderCollection() );

					if( status == 301 )
						{
						TUriParser8 uri;
						RStringF uriStrF = strP.StringF( locationValue.StrF() );
						uri.Parse( uriStrF.DesC() );
						iBody->PrepareL();
						aTransaction.Request().SetURIL( uri );
						aTransaction.Cancel();
						aTransaction.SubmitL();
						}
					else
						{
						RequestFinishedL( iRequest, *iResponse );
						}
					}
				else
					{
					HandleErrorL( ENoLocationHeader );
					Log( "error getting redirection url" );
					Log( res );
					Log( "" );
					}
				}
			else if( status == 410 )
				{
				HandleErrorL( EVideoRemoved );
				MHFRunL(iTransaction, THTTPEvent::EClosed);
				}
			else
				{
				HandleErrorL( status );
				MHFRunL(iTransaction, THTTPEvent::EClosed);
				}
			}
		break;

		case THTTPEvent::EGotResponseBodyData:
			{
			GetResponseBodyDataL();
			}
		break;

		case THTTPEvent::EResponseComplete:
			{
Log( "EResponseComplete" );

Log("progress completed");
			if( iRequest != ERequestMovieDownload || (iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
				{
				if(iProgress)
					iProgress->ProgressComplete();
				}
Log("progress completed finished");

			if( iFileOpened )
				{
				if( iHttpBuffer.Length() )
					{
#ifndef USE_FILE_STREAM
					iFile.Write( iHttpBuffer );
#else
					iFileStream.WriteL( iHttpBuffer );
#endif

					iHttpBuffer.Zero();
					}

#ifndef USE_FILE_STREAM
				iFile.Close();
#else
				iFileStream.CommitL();
				iFileStream.Close();
#endif
				}
			iFileOpened = EFalse;

			delete iFileName;
			iFileName = NULL;
			}
		break ;

		case THTTPEvent::ECancel:
			{
Log( "ECancel" );
			iTransaction.Cancel();
			CloseRequestL();
			}
		break;

		case THTTPEvent::ESucceeded:
			{
Log( "ESucceeded" );
			CloseRequestL();
			RequestCompletedL();
			}
		break;

		case THTTPEvent::EClosed:
			{
Log( "EClosed" );
			if( iRequest != ERequestMovieDownload || (iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
				{
				if(iProgress)
					iProgress->ProgressComplete();
				}
			CloseRequestL();
			}
		break;

		case THTTPEvent::EFailed:
			{
Log( "EFailed" );
			MHFRunL(iTransaction, THTTPEvent::EClosed);
			}
		break;

		case KErrDisconnected:
Log( "KErrDisconnected" );
			RequestCanceledL( iRequest );
			MHFRunL(iTransaction, THTTPEvent::EClosed);
			HandleErrorL( CEmTubeHttpEngine::ECouldNotConnect );
		break;

		case KErrCouldNotConnect:
Log( "KErrCouldNotConnect" );
			RequestCanceledL( iRequest );
			MHFRunError(aEvent.iStatus, aTransaction, aEvent);
			MHFRunL(iTransaction, THTTPEvent::EClosed);
			HandleErrorL( CEmTubeHttpEngine::ECouldNotConnect );
		break;

		default:
Log( "default" );
Log( aEvent.iStatus );
			if(aEvent.iStatus < 0)
				{
				MHFRunError(aEvent.iStatus, aTransaction, aEvent);
				MHFRunL(iTransaction, THTTPEvent::EClosed);
				HandleErrorL( aEvent.iStatus );
				}
		break;
		}
	}

TInt CEmTubeHttpEngine::MHFRunError(TInt /*aError*/, RHTTPTransaction /*aTransaction*/, const THTTPEvent& /*aEvent*/ )
	{
	iTimer->Cancel();
	return KErrNone;
	}

void CEmTubeHttpEngine::GetResponseBodyDataL()
	{
	TBool commit = EFalse;

	MHTTPDataSupplier* body = iTransaction.Response().Body();

	TPtrC8 ptr;
	body->GetNextDataPart(ptr);
	iDataReceived += ptr.Length();

	if( iDataReceived < HTTP_BUFFER_SIZE )
		commit = ETrue;

	if( iSaveToFile && !iFileOpened )
		{

		TBool res = ETrue;
		if( iBodySize > 0 && iObserver )
			res = iObserver->CheckDiskSpaceL( *iFileName, iBodySize );

		if( !res )
			{
			body->ReleaseData();
			if( iObserver )
				iObserver->ShowErrorNoteL( R_NOT_ENOUGH_SPACE_TXT );

			MHFRunL(iTransaction, THTTPEvent::ECancel);
			HandleErrorL( ERequestCanceled );
			return;
			}

		iFsSession.MkDirAll( *iFileName );
#ifndef USE_FILE_STREAM
		iFile.Replace( iFsSession, *iFileName, EFileStream | EFileWrite | EFileShareAny );
#else
		iFileStream.Replace( iFsSession, *iFileName, EFileStream | EFileWrite | EFileShareAny );
#endif
		iFileOpened = ETrue;
		iHttpBuffer.Zero();
		}

	if( !iSaveToFile )
		{
		if(!iResponse)
			{
			iResponse = ptr.AllocL();
			}
		else
			{
			TInt len = iResponse->Length() + ptr.Length();
			iResponse = iResponse->ReAllocL( len );
			iResponse->Des().Append(ptr);
			}
		}
	else
		{
		if( commit )
			{
#ifndef USE_FILE_STREAM
			iFile.Write( ptr );
#else
			iFileStream.WriteL( ptr );
			iFileStream.CommitL();
#endif
			}
		else
			{
			if( ptr.Length() >= iHttpBuffer.Size() )
				{
#ifndef USE_FILE_STREAM
				iFile.Write( iHttpBuffer );
#else
				iFileStream.WriteL( iHttpBuffer );
#endif
				iHttpBuffer.Zero();

#ifndef USE_FILE_STREAM
				iFile.Write( ptr );
#else
				iFileStream.WriteL( ptr );
#endif

				}
			else
				{
				if( (iHttpBuffer.Length() + ptr.Length()) > HTTP_BUFFER_SIZE )
					{
#ifndef USE_FILE_STREAM
					iFile.Write( iHttpBuffer );
#else
					iFileStream.WriteL( iHttpBuffer );
#endif
					iHttpBuffer.Zero();
					}
				iHttpBuffer.Append( ptr );
				}
			}
		}

	body->ReleaseData();
	if( iRequest != ERequestMovieDownload || (iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
		{
		if(iProgress)
			{
			TUint32 t = CVideoTimer::Time();
#ifndef __WINS__
	#define TICKS 1000.0f
#else
	#define TICKS 200.0f
#endif
			TInt seconds = (TInt)( (float)(t - iDownloadSpeedTicks) / TICKS );
			if( seconds )
				iDownloadSpeed = iDataReceived / 1024 / seconds;

			iProgress->ProgressUpdate( iDataReceived, iDownloadSpeed );
			}
		}
	}

void CEmTubeHttpEngine::CloseRequestL()
	{
	iTimer->Cancel();

	if( iFileOpened )
		{
		if( iHttpBuffer.Length() )
			{
#ifndef USE_FILE_STREAM
			iFile.Write( iHttpBuffer );
#else
			iFileStream.WriteL( iHttpBuffer );
#endif
			iHttpBuffer.Zero();
			}

#ifndef USE_FILE_STREAM
		iFile.Close();
#else
		iFileStream.CommitL();
		iFileStream.Close();
#endif
		}
	iFileOpened = EFalse;

	if( iTransactionOpened )
		iTransaction.Close();
	iTransactionOpened = EFalse;

//	delete iProgressObserver;
//	iProgressObserver = NULL;

//	if( iConnectionStarted )
//		iConn.Stop();
//	iConnectionStarted = EFalse;

//	if( iConnectionOpened )
//		iConn.Close();
//	iConnectionOpened = EFalse;
	}

void CEmTubeHttpEngine::TimerExpired( TInt /*aMode*/ )
	{
	CancelRequestL();
	HandleErrorL( EConnectionTimedOut );
	}

void CEmTubeHttpEngine::HandleErrorL( TInt aError )
	{
Log("Handle error");
Log( aError );

	switch( aError )
		{
		case EVideoRemoved:
		if( iObserver )
			iObserver->ShowErrorNoteL( R_HTTP_VIDEO_HAS_BEEN_REMOVED_TXT );
		break;

		case EContentLock:
		if( iObserver )
			iObserver->ShowErrorNoteL( _L("Site is marked as 18+, contact your operator to remove the content lock.") );
		break;

		case KErrCancel:
		case ERequestCanceled:
		break;

		case EConnectionTimedOut:
		if( iObserver )
			iObserver->ShowErrorNoteL( R_HTTP_CONNECTION_TIMED_OUT_TXT );
		break;

		case ENoLocationHeader:
		if( iObserver )
			iObserver->ShowErrorNoteL( R_HTTP_NO_LOCATION_HEADER_TXT );
		break;

		case 404:
			if( iObserver )
				{
				if( iRequest == ERequestMovieDownload || iRequest == ERequestMovieDownload2 )
					{
					iObserver->ShowErrorNoteL( R_HTTP_VIDEO_NOT_AVAILABLE_TXT );
					}
				else
					{
					HBufC* prompt = StringLoader::LoadLC( R_HTTP_ERROR_TXT );
					RBuf text;

					CleanupClosePushL( text );
					text.ReAllocL( prompt->Length() + 10 );
					text.Format( *prompt, aError );
	
					iObserver->ShowErrorNoteL( text );
	
					CleanupStack::PopAndDestroy( &text );
					CleanupStack::PopAndDestroy( prompt );
					}
				}
		break;

		case EVideoNotAvailable:
		if( iObserver )
			iObserver->ShowErrorNoteL( R_HTTP_VIDEO_NOT_AVAILABLE_TXT );
		break;

		case ENoPlugin:
		if( iObserver )
			iObserver->ShowErrorNoteL( R_HTTP_NO_PLUGIN_AVAILABLE_TXT );
		break;

		case -30170:
		case -30171:
		case -30172:
		case -30173:
		case -30174:
		case -30175:
		case -30176:
		case -30177:
		case -30178:
		case -30179:
		case -30180:
		case -4180:
		case -5120:
		case ECouldNotConnect:
			{
			if( iObserver )
				iObserver->ShowErrorNoteL( R_HTTP_COULD_NOT_CONNECT_TXT );
			iForceCloseConnection = ETrue;
			iForceIAPSelectionDialog = ETrue;
			}
		break;

		case -18:
		case -4155:
		case EConnectionLost:
		case EWrongAccessPoint:
			iForceCloseConnection = ETrue;
			iForceIAPSelectionDialog = ETrue;

		default:
			{
			if( iRequest != ERequestImageDownload )
				{

#ifndef EMTUBE_UIQ
				HBufC* prompt = StringLoader::LoadLC( R_HTTP_ERROR_TXT );
#else
				HBufC* prompt = CEikonEnv::Static()->AllocReadResourceLC( R_HTTP_ERROR_TXT );
#endif
				RBuf text;
				CleanupClosePushL( text );
				text.ReAllocL( prompt->Length() + 10 );
				text.Format( *prompt, aError );

				if( iObserver )
					iObserver->ShowErrorNoteL( text );

				CleanupStack::PopAndDestroy( &text );
				CleanupStack::PopAndDestroy( prompt );
				}
			}
		break;
		}

//	if( iRequest != ERequestImageDownload )
		{
		RequestCanceledL( iRequest );
		}
//	else
		{
//		RequestCompletedL();
		}
	}

void CEmTubeHttpEngine::SetHttpBufferSize( TInt aSize )
	{
	iHttpBufferSize = aSize;
	iHttpBuffer.Close();
	iHttpBuffer.Create( iHttpBufferSize );
	}

void CEmTubeHttpEngine::CancelOperationL()
	{
	iObserver = NULL;
	CancelRequestL();
	RequestCanceledL( iRequest );
	}

void CEmTubeHttpEngine::LoginL( TUint32 aPluginUid, MHttpEngineObserver& aObserver, TBool aPost )
	{
	iPluginUid = aPluginUid;
	iPluginManager->ClearCookies( iPluginUid );
	SetProgressObserver( NULL );
	iObserver = &aObserver;

	HBufC* url;
	HBufC8* body;
	HBufC8* cType;
	iPluginManager->Plugin( iPluginUid )->LoginUrlAndBodyL( url, body, cType, iPluginManager->Username( iPluginUid ), iPluginManager->Password( iPluginUid ) );
	CleanupStack::PushL( cType );
	CleanupStack::PushL( url );
	CleanupStack::PushL( body );

	if( !aPost )
		{
		iRequest = ERequestPreLogin;

		TInt ret = StartGetRequestL( *url );
		if( ret != KErrNone )
			{
			RequestCanceledL( iRequest );
			}
		}
	else
		{
		iRequest = ERequestLogin;

		iBody->PrepareL( *body, *cType );
	
		TInt ret = StartPostRequestL( *url );
		if( ret != KErrNone )
			{
			RequestCanceledL( iRequest );
			}
		}
	CleanupStack::PopAndDestroy( body );
	CleanupStack::PopAndDestroy( url );
	CleanupStack::PopAndDestroy( cType );
	}

void CEmTubeHttpEngine::UploadMovieStep0L( TUint32 aPluginUid, MHttpEngineObserver& aObserver )
	{
	iRequest = ERequestMovieUploadStep0;
	SetProgressObserver( NULL );
	iObserver = &aObserver;

	iPluginUid = aPluginUid;
	HBufC* url = iPluginManager->Plugin( aPluginUid )->UploadMovieUrlL();
	CleanupStack::PushL( url );

	TInt ret = StartGetRequestL( *url );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}

	CleanupStack::PopAndDestroy( url );
	}

void CEmTubeHttpEngine::UploadMovieStep1L( TUint32 aPluginUid, CQueueEntry& aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	iRequest = ERequestMovieUploadStep1;
	SetProgressObserver( NULL );
	iObserver = &aObserver;

	HBufC* url;
	HBufC8* body1;
	HBufC8* body2;
	HBufC8* cType;

	iPluginUid = aPluginUid;
	TBool res = iPluginManager->Plugin( aPluginUid )->UploadMovieStep1L( url, body1, body2, cType, aEntry );
	CleanupStack::PushL( cType );
	CleanupStack::PushL( body1 );
	CleanupStack::PushL( body2 );
	CleanupStack::PushL( url );

	if( res )
		{
		if( body2 )
			iBody->PrepareL( aEntry.Filename(), *body1, *body2, *cType, aProgressObserver );
		else
			iBody->PrepareL( aEntry.Filename(), *body1, *cType, aProgressObserver );
		}
	else
		{
		if( body2 )
			iBody->PrepareL( *body1, *body2, *cType );
		else
			iBody->PrepareL( *body1, *cType );
		}

	TInt ret = StartPostRequestL( *url );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}

	CleanupStack::PopAndDestroy( url );
	CleanupStack::PopAndDestroy( body2 );
	CleanupStack::PopAndDestroy( body1 );
	CleanupStack::PopAndDestroy( cType );
	}

void CEmTubeHttpEngine::UploadMovieStep2L( TUint32 aPluginUid, CQueueEntry& aEntry, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	iRequest = ERequestMovieUploadStep2;
	SetProgressObserver( NULL );
	iObserver = &aObserver;

	HBufC* url;
	HBufC8* body1;
	HBufC8* body2;
	HBufC8* cType;

	iPluginUid = aPluginUid;
	TBool res = iPluginManager->Plugin( aPluginUid )->UploadMovieStep2L( url, body1, body2, cType, aEntry );
	CleanupStack::PushL( cType );
	CleanupStack::PushL( body1 );
	CleanupStack::PushL( body2 );
	CleanupStack::PushL( url );

	if( res )
		{
		if( body2 )
			iBody->PrepareL( aEntry.Filename(), *body1, *body2, *cType,  aProgressObserver );
		else
			iBody->PrepareL( aEntry.Filename(), *body1, *cType,  aProgressObserver );
		}
	else
		{
		if( body2 )
			iBody->PrepareL( *body1, *body2, *cType );
		else
			iBody->PrepareL( *body1, *cType );
		}

	TInt ret = StartPostRequestL( *url );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}

	CleanupStack::PopAndDestroy( url );
	CleanupStack::PopAndDestroy( body2 );
	CleanupStack::PopAndDestroy( body1 );
	CleanupStack::PopAndDestroy( cType );
	}

void CEmTubeHttpEngine::CheckForUpdateL( MHttpEngineObserver& aObserver )
	{
	iRequest = ERequestCheckForUpdate;
	SetProgressObserver( NULL );
	iObserver = &aObserver;

	TInt ret = StartGetRequestL( KAppUpdateUrl() );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}
	}

void CEmTubeHttpEngine::DownloadMovieL( TDesC& aOutput, TDesC& aUrl, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	iRequest = ERequestMovieDownload2;
	SetProgressObserver( &aProgressObserver );
	iObserver = &aObserver;

Log( "download movie");
Log( aUrl );

	TInt ret = StartGetRequestL( aUrl, aOutput, 0 );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}
	}

void CEmTubeHttpEngine::DownloadMovieL( TDesC& aOutput, CVideoEntry& aEntry, TInt aStartOffset, MHttpEngineObserver& aObserver, MProgressObserver& aProgressObserver )
	{
	iRequest = ERequestMovieDownload;
	SetProgressObserver( &aProgressObserver );
	iObserver = &aObserver;

Log( "download movie");
Log( aEntry.Url() );

  	TInt ret = StartGetRequestL( aEntry.Url(),  aOutput, aStartOffset );
   	if( ret != KErrNone )
   		{
		RequestCanceledL( iRequest );
   		}
	}

void CEmTubeHttpEngine::DownloadMovieL( TDesC& aUrl, MHttpEngineObserver& aObserver )
	{
	iRequest = ERequestMovieDownload;
	iObserver = &aObserver;

Log( "download movie");
Log( aUrl );

  	TInt ret = StartGetRequestL( aUrl );
   	if( ret != KErrNone )
   		{
		RequestCanceledL( iRequest );
   		}
	}

HBufC* CEmTubeHttpEngine::EscapeUrlL( const TDesC& aUrl )
	{
	TInt rev = aUrl.LocateReverse( '/' );
	const TPtrC name = aUrl.Right( aUrl.Length() - rev - 1 );
	const TPtrC host = aUrl.Mid( 0, rev + 1 );

	HBufC8* utf8 = EscapeUtils::ConvertFromUnicodeToUtf8L( name );
	CleanupStack::PushL( utf8 );

	HBufC8* encode = EscapeUtils::EscapeEncodeL( *utf8 , EscapeUtils::EEscapeUrlEncoded );
	CleanupStack::PushL( encode );

	HBufC16* unicode = EscapeUtils::ConvertToUnicodeFromUtf8L( *encode );
	CleanupStack::PushL( unicode );

	RBuf newUrl;
	CleanupClosePushL( newUrl );
	newUrl.Create( host.Length() + unicode->Length() );
	newUrl.Copy( host );
	newUrl.Append( *unicode );

	HBufC* ret = newUrl.AllocL();

	CleanupStack::PopAndDestroy( &newUrl );

	CleanupStack::PopAndDestroy( unicode );
	CleanupStack::PopAndDestroy( encode );
	CleanupStack::PopAndDestroy( utf8 );

	return ret;
	}

void CEmTubeHttpEngine::DownloadImageL( CVideoEntry *aEntry, MHttpEngineObserver& aObserver )
	{
Log("download image");
Log( aEntry->ThumbnailUrl() );
Log( aEntry->ThumbnailFile() );

	iRequest = ERequestImageDownload;
	SetProgressObserver( NULL );

	iObserver = &aObserver;

	HBufC* url = EscapeUrlL( aEntry->ThumbnailUrl() );
	CleanupStack::PushL( url );

Log("escaped url:");
Log( *url );
Log("");

	TInt ret = StartGetRequestL( *url, aEntry->ThumbnailFile() );

	CleanupStack::PopAndDestroy( url );


	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		return;
		}
	}

void CEmTubeHttpEngine::SearchL( const TDesC& aUrl, MHttpEngineObserver &aObserver )
	{
Log( "search" );
Log( aUrl );
	iRequest = ERequestSearch;
	iObserver = &aObserver;

	SetProgressObserver( NULL );

	delete iSearchString;
	iSearchString = aUrl.AllocL();

	HBufC8* data = iAppUi->Cache().GetDataFromCacheL( *iSearchString );
	if( data )
		{
		iResponse = data;
		RequestCompletedL();
		return;
		}

	TInt ret = StartGetRequestL( aUrl );
	if( ret != KErrNone )
		{
		RequestCanceledL( iRequest );
		}
	}

void CEmTubeHttpEngine::RequestCompletedL()
	{
Log("Request completed");
Log( iRequest );
Log( "" );
	TInt request = iRequest;
	iRequest = ERequestNone;

	if( request == ERequestSearch )
		{
		iAppUi->Cache().SaveDataInCacheL( *iSearchString, *iResponse );
		}

if( iResponse )
	{
	Log( *iResponse );
	}

	if( request == ERequestMovieDownload && !(iPluginManager->Plugin( iPluginUid )->Capabilities() & KCapsVideoUrlIsDirect ) )
		{
		TBool lastOne;
		HBufC* url = iPluginManager->Plugin( iPluginUid )->VideoUrlL( *iResponse, lastOne );
		CleanupStack::PushL( url );
		if( !url )
			{
			HandleErrorL(EVideoNotAvailable);
//			RequestFinishedL( request, *iResponse );
			}
		else
			{
			if( iObserver )
				{
				if( lastOne )
					DownloadMovieL( *iMovieFileName, *url, *iObserver, *iProgress );
				else
					DownloadMovieL( *url, *iObserver );
				}
			}
		CleanupStack::PopAndDestroy( url );
		}
	else
		{
		RequestFinishedL( request, *iResponse );
		}

	delete iResponse;
	iResponse = NULL;

	}

//data supplier -> buffer
CEmTubeDataSupplierBuffer* CEmTubeDataSupplierBuffer::NewL(const TDesC8& aData)
	{
	CEmTubeDataSupplierBuffer* self = CEmTubeDataSupplierBuffer::NewLC( aData );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeDataSupplierBuffer* CEmTubeDataSupplierBuffer::NewLC(const TDesC8& aData)
	{
	CEmTubeDataSupplierBuffer* self = new (ELeave)CEmTubeDataSupplierBuffer();
	CleanupStack::PushL(self);
	self->ConstructL( aData );
	return self;
	}

CEmTubeDataSupplierBuffer::CEmTubeDataSupplierBuffer()
	{
	}

void CEmTubeDataSupplierBuffer::ConstructL(const TDesC8& aData)
	{
	iData = aData.AllocL();
	}

CEmTubeDataSupplierBuffer::~CEmTubeDataSupplierBuffer()
	{
	delete iData;
	}

TInt CEmTubeDataSupplierBuffer::Length()
	{
	return iData->Length();
	}

void CEmTubeDataSupplierBuffer::DataBlock( TPtrC8& aData )
	{
	aData.Set( *iData );
	}

TInt CEmTubeDataSupplierBuffer::NextDataBlock()
	{
	return iData->Length();
	}

//data supplier file
CEmTubeDataSupplierFile* CEmTubeDataSupplierFile::NewL(const TDesC& aFilename)
	{
	CEmTubeDataSupplierFile* self = CEmTubeDataSupplierFile::NewLC( aFilename );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeDataSupplierFile* CEmTubeDataSupplierFile::NewLC(const TDesC& aFilename)
	{
	CEmTubeDataSupplierFile* self = new (ELeave)CEmTubeDataSupplierFile();
	CleanupStack::PushL(self);
	self->ConstructL( aFilename );
	return self;
	}

CEmTubeDataSupplierFile::CEmTubeDataSupplierFile()
	{
	}

void CEmTubeDataSupplierFile::ConstructL(const TDesC& aFilename)
	{
	iFs.Connect();

	iFile.Open( iFs, aFilename, EFileStream | EFileRead | EFileShareAny );
	iFile.Size (iFileSize );

	iLastBlock = EFalse;

	iFile.Read( iFileBuffer );
	if( iFileBuffer.Length() != CHUNK_SIZE )
		iLastBlock = ETrue;
	}

CEmTubeDataSupplierFile::~CEmTubeDataSupplierFile()
	{
	iFile.Close();
	iFs.Close();
	}

TInt CEmTubeDataSupplierFile::Length()
	{
	return iFileSize;
	}

void CEmTubeDataSupplierFile::DataBlock( TPtrC8& aData )
	{
	aData.Set( iFileBuffer );
	}

TInt CEmTubeDataSupplierFile::NextDataBlock()
	{
	iFile.Read( iFileBuffer );
	if( iFileBuffer.Length() != CHUNK_SIZE )
		iLastBlock = ETrue;
	return iFileBuffer.Length();
	}

//

CEmTubeHTTPDataSupplier* CEmTubeHTTPDataSupplier::NewL()
	{
	CEmTubeHTTPDataSupplier* self = CEmTubeHTTPDataSupplier::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CEmTubeHTTPDataSupplier* CEmTubeHTTPDataSupplier::NewLC()
	{
	CEmTubeHTTPDataSupplier* self = new (ELeave)CEmTubeHTTPDataSupplier();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CEmTubeHTTPDataSupplier::CEmTubeHTTPDataSupplier()
	{
	}

void CEmTubeHTTPDataSupplier::ConstructL()
	{
	}

CEmTubeHTTPDataSupplier::~CEmTubeHTTPDataSupplier()
	{
	delete iContentType;

	iSuppliers.ResetAndDestroy();
	iSuppliers.Close();
	}

TBool CEmTubeHTTPDataSupplier::GetNextDataPart(TPtrC8& aDataPart)
	{
	TBool last = EFalse;

	iSuppliers[ iCurrentChunk ]->DataBlock( aDataPart );

	if( iCurrentChunk == ( iSuppliers.Count() - 1 ) )
		{
		if( iSuppliers[ iCurrentChunk ]->EndOfData() )
			{
			last = ETrue;
			}
		}

	return last;
	}

void CEmTubeHTTPDataSupplier::ReleaseData()
	{
	if( iSuppliers[iCurrentChunk]->EndOfData() )
		{
		iCurrentChunk++;
		}
	else
		{
		iCurrentSize += iSuppliers[iCurrentChunk]->NextDataBlock();
		if( iProgressObserver )
			iProgressObserver->ProgressUpdate( iCurrentSize, 0 ); //TODO - add speed for upload too
		}

	if( iCurrentChunk < iSuppliers.Count()  )
		{
		iTransaction->NotifyNewRequestBodyPartL();
		}
	else
		{
		if( iProgressObserver )
			iProgressObserver->ProgressComplete();
		}
	}

TInt CEmTubeHTTPDataSupplier::Reset()
	{
	iCurrentChunk = 0;
	return KErrNone;
	}

TInt CEmTubeHTTPDataSupplier::OverallDataSize()
	{
	TInt size = 0;
	for( TInt i=0;i<iSuppliers.Count();i++ )
		{
		size += iSuppliers[i]->Length();
		}

	if(size >= 0)
		return size;
	else
		return KErrNotFound;
	}

void CEmTubeHTTPDataSupplier::PrepareL()
	{
	iCurrentChunk = 0;
	iCurrentSize = 0;
	}

void CEmTubeHTTPDataSupplier::PrepareL( const TDesC8& aData, const TDesC8& aContentType )
	{
	iProgressObserver = NULL;
	iCurrentChunk = 0;
	iCurrentSize = 0;

	delete iContentType;
	iSuppliers.ResetAndDestroy();

	CEmTubeDataSupplierBuffer* buf = CEmTubeDataSupplierBuffer::NewL( aData );
	iSuppliers.Append( buf );

	iContentType = aContentType.AllocL();
	}

void CEmTubeHTTPDataSupplier::PrepareL( const TDesC8& aData1, const TDesC8& aData2, const TDesC8& aContentType )
	{
	iProgressObserver = NULL;
	iCurrentChunk = 0;
	iCurrentSize = 0;

	delete iContentType;
	iSuppliers.ResetAndDestroy();

	CEmTubeDataSupplierBuffer* buf = CEmTubeDataSupplierBuffer::NewL( aData1 );
	iSuppliers.Append( buf );
	buf = CEmTubeDataSupplierBuffer::NewL( aData2 );
	iSuppliers.Append( buf );

	iContentType = aContentType.AllocL();
	}

void CEmTubeHTTPDataSupplier::PrepareL( const TDesC& aFilename, const TDesC8& aData, const TDesC8& aContentType, MProgressObserver& aObserver )
	{
	iProgressObserver = &aObserver;
	iCurrentChunk = 0;
	iCurrentSize = 0;

	delete iContentType;
	iSuppliers.ResetAndDestroy();

	CEmTubeDataSupplierBuffer* buf = CEmTubeDataSupplierBuffer::NewL( aData );
	iSuppliers.Append( buf );
	CEmTubeDataSupplierFile* file = CEmTubeDataSupplierFile::NewL( aFilename );
	iSuppliers.Append( file );

	if( iProgressObserver )
		iProgressObserver->ProgressStart( OverallDataSize() );

	iContentType = aContentType.AllocL();
	}

void CEmTubeHTTPDataSupplier::PrepareL( const TDesC& aFilename, const TDesC8& aData1, const TDesC8& aData2, const TDesC8& aContentType, MProgressObserver& aObserver )
	{
	iProgressObserver = &aObserver;
	iCurrentChunk = 0;
	iCurrentSize = 0;

	delete iContentType;
	iSuppliers.ResetAndDestroy();

	CEmTubeDataSupplierBuffer* buf = CEmTubeDataSupplierBuffer::NewL( aData1 );
	iSuppliers.Append( buf );
	CEmTubeDataSupplierFile* file = CEmTubeDataSupplierFile::NewL( aFilename );
	iSuppliers.Append( file );
	buf = CEmTubeDataSupplierBuffer::NewL( aData2 );
	iSuppliers.Append( buf );

	if( iProgressObserver )
		iProgressObserver->ProgressStart( OverallDataSize() );

	iContentType = aContentType.AllocL();
	}

#if 1
void CEmTubeHttpEngine::DumpHeadersL( RHTTPTransaction& aTrans )
{
	RHTTPResponse resp = aTrans.Response();
	RStringPool strP = aTrans.Session().StringPool();
	RHTTPHeaders hdr = resp.GetHeaderCollection();
	THTTPHdrFieldIter it = hdr.Fields();

	Log("->dumping headers:");
	Log( resp.StatusCode() );
	Log( resp.StatusText().DesC() );

	TBuf<KMaxHeaderNameLen>  fieldName16;
	TBuf<KMaxHeaderValueLen> fieldVal16;

	while (it.AtEnd() == EFalse)
	{
		RStringTokenF fieldName = it();
		RStringF fieldNameStr = strP.StringF(fieldName);
		THTTPHdrVal fieldVal;
		if (hdr.GetField(fieldNameStr,0,fieldVal) == KErrNone)
		{
			const TDesC8& fieldNameDesC = fieldNameStr.DesC();
			fieldName16.Copy(fieldNameDesC.Left(KMaxHeaderNameLen));
			switch (fieldVal.Type())
			{
				case THTTPHdrVal::KTIntVal:
				{
					Log( fieldName16 );
					Log( fieldVal.Int() );
				}
				break;

				case THTTPHdrVal::KStrFVal:
				{
					RStringF fieldValStr = strP.StringF(fieldVal.StrF());
					const TDesC8& fieldValDesC = fieldValStr.DesC();
					fieldVal16.Copy(fieldValDesC.Left(KMaxHeaderValueLen));
					Log( fieldName16 );
					Log( fieldVal16 );
				}
				break;

				case THTTPHdrVal::KStrVal:
				{
					RString fieldValStr = strP.String(fieldVal.Str());
					const TDesC8& fieldValDesC = fieldValStr.DesC();
					fieldVal16.Copy(fieldValDesC.Left(KMaxHeaderValueLen));
					Log( fieldName16 );
					Log( fieldVal16 );
				}
				break;

				case THTTPHdrVal::KDateVal:
				{
					TDateTime date = fieldVal.DateTime();
					TBuf<40> dateTimeString;
					TTime t(date);
//					t.FormatL( dateTimeString, KDateFormat);
					Log( fieldName16 );
//					Log( dateTimeString );
				}
				break;

				default:
				{
					Log( "unknown header type");
					Log( fieldName16 );
				}
				break;
			}
		}
	Log("");
	++it;
	}

Log("dumping response cookie:");
	RHTTPResponse response = aTrans.Response();
	RStringF fieldName = iSession.StringPool().StringF(HTTP::ESetCookie, RHTTPSession::GetTable());
	THTTPHdrVal val;
	if (response.GetHeaderCollection().GetField(fieldName, 0, val) != KErrNotFound)
		{
		RStringF cookieValueName = iSession.StringPool().StringF(HTTP::ECookieValue,
		RHTTPSession::GetTable());
		RStringF cookieNameName = iSession.StringPool().StringF(HTTP::ECookieName,
		RHTTPSession::GetTable());

		if (val.StrF() == iSession.StringPool().StringF(HTTP::ECookie, RHTTPSession::GetTable()))
			{
			THTTPHdrVal cookieValue;
			THTTPHdrVal cookieName;

			TInt parts = response.GetHeaderCollection().FieldPartsL(fieldName);
			for (TInt i = 0; i < parts; i++)
				{
				response.GetHeaderCollection().GetParam(fieldName, cookieValueName, cookieValue, i);
				response.GetHeaderCollection().GetParam(fieldName, cookieNameName, cookieName, i);

				RString fieldNameStr = strP.String(cookieName.Str());
				const TDesC8& fieldNameDesC = fieldNameStr.DesC();
				fieldName16.Copy(fieldNameDesC.Left(KMaxHeaderNameLen));

				RString fieldValStr = strP.String(cookieValue.Str());
				const TDesC8& fieldValDesC = fieldValStr.DesC();
				fieldVal16.Copy(fieldValDesC.Left(KMaxHeaderValueLen));

				Log( fieldName16 );
				Log( fieldVal16 );
				}
			}
		}
	Log("->end of headers");
	}
#endif

void CEmTubeHttpEngine::RequestFinishedL( TInt aState, TDesC8& aResponseBuffer )
	{
	if( iObserver )
		iObserver->RequestFinishedL( aState, aResponseBuffer );
	}

void CEmTubeHttpEngine::RequestCanceledL( TInt aState )
	{
Log("->requestcanceledL()");
	iRequest = ERequestNone;

	MHttpEngineObserver* observer = iObserver;
	iObserver = NULL;

Log((TInt)observer);

	if( observer )
		observer->RequestCanceledL( aState );
Log("<-requestcanceledL()");
	}

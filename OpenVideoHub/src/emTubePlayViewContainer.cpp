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

#include <AknIconArray.h>
#include <GULICON.H>
#include <GULUTIL.H>
#include <stringloader.h>
#include <aknnavilabel.h>
#include <hal.h>
#include <aknnotewrappers.h>
#include <aknutils.h>
#include <aknmessagequerydialog.h>
#include <AknCommonDialogs.h> // For single function calls
#include <CAknMemorySelectionDialog.h>
#include <CAknFileSelectionDialog.h>
#include <CAknFileNamePromptDialog.h>
#include <BAUTILS.H>
#include <apgcli.h>
#include <akntitle.h>
#include <aknappui.h>
#ifdef __S60_50__
#include <akntoolbar.h>
#include <touchfeedback.h>
#endif

#include "emTubeResource.h"

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeHttpEngine.h"
#include "emTubeAppUi.h"
#include "emTubePlayViewContainer.h"
#include "emTubePlayer.h"
#include "emTubeYUV2RGB.h"

#include "emTubePlaylistManager.h"

#include "emTubeVideoEntry.h"

#include "avlibaudio.h"
#include "avlibvideo.h"

#ifndef __S60_50__

//rotation sensor ID
const TInt Kacc = 12350;

#else

const TInt KGoBackButtonWidth = 48;
const TInt KGoBackButtonHeight = 48;

const TInt KSelectedButtonScale = 130;

const TInt KMainOsdWidth = 640;
const TInt KOsdMarginX = 20;
const TInt KOsdMarginY = 10;

#endif

CEmTubePlayViewContainer* CEmTubePlayViewContainer::NewL(CEmTubePlayView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager )
	{
	CEmTubePlayViewContainer* self = CEmTubePlayViewContainer::NewLC( aView, aRect, aManager );
	CleanupStack::Pop(self);
	return self;
	}

CEmTubePlayViewContainer* CEmTubePlayViewContainer::NewLC(CEmTubePlayView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager )
	{
	CEmTubePlayViewContainer* self = new (ELeave) CEmTubePlayViewContainer( aView, aManager );
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}

CEmTubePlayViewContainer::~CEmTubePlayViewContainer()
	{
	delete iLineStatusMonitor;
	
	delete iConnectingTxt;
	delete iBufferingTxt;
	CCoeEnv::Static()->ScreenDevice()->ReleaseFont( iFont );

#ifndef __S60_50__
	if( iAccSensor )
		{
		iAccSensor->RemoveDataListener();
		delete iAccSensor;
		iAccSensor = NULL;
		}
	iSensorLibrary.Close();
#else
	delete iGoBackBitmap;
	delete iGoBackMask;
	delete iGoBackBitmapPressed;
	delete iGoBackMaskPressed;
#endif

	delete iTimeFormatString;

	delete iInterfaceSelector;

	iBitmaps.ResetAndDestroy();
	iBitmaps.Close();

	delete iPlayer;

	if (!iNaviPane)
		{
		iNaviPane = (CAknNavigationControlContainer*)iStatusPane->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
		}
	if(iNaviPane && ( iNaviVolumeDecorator || iNaviImageDecorator ) )
		iNaviPane->Pop(NULL);

	iNaviPane->Pop(NULL);

	delete iNaviVolumeDecorator;
	delete iNaviImageDecorator;
	delete iNaviLabelDecorator;

	delete iIdle;

	iSession.Close();

	if( iTimer )
		iTimer->Cancel();
	delete iTimer;

	}

CEmTubePlayViewContainer::CEmTubePlayViewContainer(CEmTubePlayView& aView, CEmTubePlaylistManager* aManager ) : iView(aView), iManager( aManager )
	{
	}

void CEmTubePlayViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextPlayer;
    }

void CEmTubePlayViewContainer::ConstructL(const TRect& aRect)
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	iLineStatusMonitor = CEmTubeLineStatusMonitor::NewL( *this );
	
	iBufferingTxt = StringLoader::LoadL( R_BUFFERING_TXT );
	iConnectingTxt = StringLoader::LoadL( R_CONNECTING_TXT );

	_LIT(KStandardFont, "Nokia Sans Regular");
	TFontSpec spec;
	spec = TFontSpec(KStandardFont, 200);
	spec.iFontStyle.SetBitmapType( EAntiAliasedGlyphBitmap );
	spec.iHeight = 18;
//	spec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
	CCoeEnv::Static()->ScreenDevice()->GetNearestFontInPixels( iFont, spec );

	MEikAppUiFactory* f = CEikonEnv::Static()->AppUiFactory();
	iStatusPane = f->StatusPane();
	iToolBar = f->ToolBar();

	iSession.Connect();

	iFileOpened = EFalse;

	iTimeFormatString = CEikonEnv::Static()->AllocReadResourceL( R_QTN_TIME_DURAT_MIN_SEC );

	iMode = EIdle;

#ifndef __S60_50__
typedef CRRSensorApi* (*TSensorNewLFunc)( TRRSensorInfo aSensor );
typedef void (*TFindSensorsLFunc)( RArray<TRRSensorInfo>& aSensorInfoArray );
_LIT(KSensorLibrary, "z:\\sys\\bin\\RRSensorApi.dll");

	TInt error = iSensorLibrary.Load( KSensorLibrary );

	if( error == KErrNone )
		{
		TFindSensorsLFunc FindSensorsLFunc = reinterpret_cast<TFindSensorsLFunc>(iSensorLibrary.Lookup(1));
		RArray <TRRSensorInfo> sensorList;
		FindSensorsLFunc( sensorList );
		TInt sensorCount = sensorList.Count();
		for( TInt i = 0 ; i != sensorCount ; i++ )
			{
			if( sensorList[i].iSensorId == Kacc )
				{
				TSensorNewLFunc SensorNewLFunc = reinterpret_cast<TSensorNewLFunc>(iSensorLibrary.Lookup(2));
			 	iAccSensor = SensorNewLFunc( sensorList[i] );
				iAccSensor->AddDataListener( this );
				}
			}
		}
#endif

	iInterfaceSelector = CRemConInterfaceSelector::NewL();
	iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *this);
	iInterfaceSelector->OpenTargetL();

	CreateWindowL();

	AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EScreen, iFullscreenRect );

#ifdef __S60_50__
	iDefaultRect = iFullscreenRect;
	iStatusPane->MakeVisible( EFalse );
	iToolBar->MakeVisible( EFalse );
	iAppUi->StopDisplayingPopupToolbar();
	SetExtentToWholeScreen();
	iCurrentCba = R_AVKON_SOFTKEYS_OK_EMPTY;
	iFullscreen = ETrue;
	AknIconUtils::CreateIconL( iGoBackBitmap, iGoBackMask, KBitmapFileName, EMbmOpenvideohubOsd_goback, EMbmOpenvideohubOsd_goback_mask );
	AknIconUtils::SetSize( iGoBackBitmap, TSize( KGoBackButtonWidth, KGoBackButtonHeight ) );
	AknIconUtils::CreateIconL( iGoBackBitmapPressed, iGoBackMaskPressed, KBitmapFileName, EMbmOpenvideohubOsd_goback, EMbmOpenvideohubOsd_goback_mask );
	AknIconUtils::SetSize( iGoBackBitmapPressed, TSize( KGoBackButtonWidth * KSelectedButtonScale / 100, KGoBackButtonHeight * KSelectedButtonScale / 100) );
	iGoBackButtonPoint = TPoint(KMainOsdWidth - KOsdMarginX - KGoBackButtonWidth/2, KOsdMarginY + KGoBackButtonHeight/2);
	iGoBackRect = TRect( iGoBackButtonPoint, TSize( KGoBackButtonWidth, KGoBackButtonHeight ) );
#else
	iDefaultRect = aRect;
	SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
	iFullscreen = EFalse;
#endif

	SetRect(iDefaultRect);

	iCurrentTime = -1;
	iNaviPane = (CAknNavigationControlContainer*)iStatusPane->ControlL(TUid::Uid(EEikStatusPaneUidNavi));
	iNaviLabelDecorator = iNaviPane->CreateNavigationLabelL();
	static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( KNullDesC() );
	iNaviPane->PushL( *iNaviLabelDecorator );

	iTimer = CEmTubeTimeOutTimer::NewL( *this );

	iPause = ETrue;
	iPlaybackReady = EFalse;

	iFileError = KErrNone;

	EnableDragEvents();
    ActivateL();
	DrawNow();

	iFrameCount = 8;
	for(TInt i=0;i<iFrameCount;i++)
		{
		CFbsBitmap *bitmap, *mask;
		AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohub11 + (i*2), EMbmOpenvideohub11 + (i*2) + 1);
		iBitmaps.Append( bitmap );
		iBitmaps.Append( mask );
		}

	iBufferingPercentage = iBufferingStartPosition = 0;
	iProgressCompleteSize = -1;
	if( iView.VideoEntry() )
		{
		iProgressPreviousPercentage = iProgressPercentage = 0;
		iProgressPreviousSize = 0;
		iFileDownloaded = EFalse;
		iProgressEnabled = ETrue;
		SetCurrentCbaL( R_EMTV_PROGRESS_DIALOG_CBA );

		iVideoEntry = iView.VideoEntry();

		iTotalTime = iVideoEntry->Duration();
		iView.SetVideoEntry( NULL );

		if( !iVideoEntry->Url().Compare( iAppUi->LastVideoUrl() ) && iVideoEntry->DownloadFinished() )
			{
			iFileDownloaded = ETrue;
			iWaitingForFile = ETrue;
			iProgressPercentage = 100;
			iPlayer = CEmTubePlayer::NewL( iAppUi->VideoFileName(),
											this,
											iEikonEnv->WsSession(),
											*(CCoeEnv::Static()->ScreenDevice()),
											Window(), iDefaultRect, this, iFullscreen );
			}
		else
			{
			iWaitingForFile = EFalse;
			iAppUi->DownloadMovieL( iVideoEntry, *this, *this );
			}
		iTimer->After( 1, EModeProgress );
		}
	else
		{
		iFileDownloaded = EFalse;

		iWaitingForFile = ETrue;

		iVideoEntry = NULL;
		iTotalTime = 0;
		iProgressPercentage = 100;

		iProgressEnabled = ETrue;
		SetCurrentCbaL( R_EMTV_PROGRESS_DIALOG_CBA );
		iTimer->After( 1, EModeProgress );

		if( iView.FileName().Length() )
			{
			iPlayer = CEmTubePlayer::NewL( iView.FileName(),
											this,
											iEikonEnv->WsSession(),
											*(CCoeEnv::Static()->ScreenDevice()),
											Window(), iDefaultRect, this, iFullscreen );
			}
		else
			{
			iPlayer = CEmTubePlayer::NewL( iView.FileHandle(),
											this,
											iEikonEnv->WsSession(),
											*(CCoeEnv::Static()->ScreenDevice()),
											Window(), iDefaultRect, this, iFullscreen );
			}
		}

#ifdef __S60_50__
	MTouchFeedback* feedback = MTouchFeedback::Instance();
	if ( feedback )
		{
		feedback->EnableFeedbackForControl( this, ETrue );
		}
#endif
    }

void CEmTubePlayViewContainer::OpenFileL()
	{
	if( iPlayer )
		{
		iMode = EOpenNewFile;
		iPlayer->Close();
		}
	}

TInt CEmTubePlayViewContainer::CountComponentControls() const
	{
	return 0;
	}

CCoeControl* CEmTubePlayViewContainer::ComponentControl( TInt aIndex ) const
	{
	return NULL;
	}

void CEmTubePlayViewContainer::SizeChanged()
	{
	DrawNow();
	}

void CEmTubePlayViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);

	if( aType == KEikDynamicLayoutVariantSwitch )
		{
		SetRect( iView.ClientRect() );
		if( iFullscreen )
			SetExtentToWholeScreen();

		iDefaultRect = iView.ClientRect();
		AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EScreen, iFullscreenRect );
		if( iPlayer )
			{
			if( !iFullscreen )
				{
				iPlayer->SetResolutionL( iDefaultRect, EFalse );
				}
			else
				{
				iPlayer->SetResolutionL( iFullscreenRect, ETrue );
				}
			}
#ifdef __S60_50__
		iStatusPane->MakeVisible( EFalse );
		iToolBar->MakeVisible( EFalse );
		iAppUi->StopDisplayingPopupToolbar();
#endif
		DrawDeferred();
		}
	}

void CEmTubePlayViewContainer::HideVolumeIndicator()
	{
	if( iNaviVolumeDecorator || iNaviImageDecorator )
		iNaviPane->Pop(NULL);

	delete iNaviImageDecorator;
	iNaviImageDecorator = NULL;
	delete iNaviVolumeDecorator;
	iNaviVolumeDecorator = NULL;

	if( iFullscreen )
		{
		iPlayer->HideVolumeIndicator();
		}
	}

void CEmTubePlayViewContainer::ShowVolumeIndicator( TInt aVolume )
	{  
	if( iTimer->IsActive() )
		iTimer->Cancel();
	if( !iFullscreen )
		{
		HideVolumeIndicator();
		if( !aVolume )
			{
			CFbsBitmap* volumeMuteBitmap;
			CFbsBitmap* volumeMuteMask;
			AknIconUtils::CreateIconL( volumeMuteBitmap, volumeMuteMask, KBitmapFileName, EMbmOpenvideohubIcn_volume_mute, EMbmOpenvideohubIcn_volume_mute_mask);
			TRect rect = iNaviPane->Rect();
			TSize size( rect.Height(), rect.Height() );
			AknIconUtils::SetSize( volumeMuteBitmap, size);
			iNaviImageDecorator = iNaviPane->CreateNavigationImageL( volumeMuteBitmap, volumeMuteMask );

			iNaviPane->PushL( *iNaviImageDecorator );
			}
		else
			{
			iNaviVolumeDecorator = iNaviPane->CreateVolumeIndicatorL(R_AVKON_NAVI_PANE_VOLUME_INDICATOR);
			STATIC_CAST(CAknVolumeControl*, iNaviVolumeDecorator->DecoratedControl())->SetValue( aVolume / 10 );
			iNaviPane->PushL(*iNaviVolumeDecorator);
			}
		}
	else
		{
		iPlayer->ShowVolumeIndicator( aVolume );
		}

	iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlShowVolume, EEventKeyUp );
	iTimer->After( 1500000, EModeVolume ); //1.5 seconds
	}

void CEmTubePlayViewContainer::MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct)
	{
	switch (aOperationId)
		{
		case ERemConCoreApiMute:
			{
			}
		break;

		case ERemConCoreApiPlay:
			{
			}
		break;

		case ERemConCoreApiPause:
			{
			}
		break;

		case ERemConCoreApiRewind:
			{
			}
		break;

		case ERemConCoreApiForward:
			{
			}
		break;

		case ERemConCoreApiBackward:
			{
			}
		break;

		case ERemConCoreApiPausePlayFunction:
			{
			}
		break;

		case ERemConCoreApiVolumeUp:
			{
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
				break;

				case ERemConCoreApiButtonRelease:
				break;

				case ERemConCoreApiButtonClick:
					if( iPlaybackReady )
						{
#if 0
						AknLayoutUtils::TAknCbaLocation location = AknLayoutUtils::CbaLocation();
						if( location == AknLayoutUtils::EAknCbaLocationRight ||
							location == AknLayoutUtils::EAknCbaLocationLeft ||
							iPlayer->DisplayRotated() )
							{
							iPlayer->SetVolume( iPlayer->Volume() - 10 );
							}
						else
							{
							iPlayer->SetVolume( iPlayer->Volume() + 10 );
							}
#else
						iPlayer->SetVolume( iPlayer->Volume() + 10 );
#endif
						iMuted = EFalse;
						iAppUi->SetVolume( iPlayer->Volume() / 10 );
						ShowVolumeIndicator( iPlayer->Volume() );
						}
				break;

				default:
				break;
				}
 			}
		break;

		case ERemConCoreApiVolumeDown:
			{
			switch (aButtonAct)
				{
				case ERemConCoreApiButtonPress:
				break;

				case ERemConCoreApiButtonRelease:
				break;

				case ERemConCoreApiButtonClick:
					if( iPlaybackReady )
						{
#if 0
						AknLayoutUtils::TAknCbaLocation location = AknLayoutUtils::CbaLocation();
						if( location == AknLayoutUtils::EAknCbaLocationRight ||
							location == AknLayoutUtils::EAknCbaLocationLeft ||
							iPlayer->DisplayRotated() )
							{
							iPlayer->SetVolume( iPlayer->Volume() + 10 );
							}
						else
							{
							iPlayer->SetVolume( iPlayer->Volume() - 10 );
							}
#else
						iPlayer->SetVolume( iPlayer->Volume() - 10 );
#endif
						iMuted = EFalse;
						iAppUi->SetVolume( iPlayer->Volume() / 10 );
						ShowVolumeIndicator( iPlayer->Volume() );
						}
				break;

				default:
				break;
				}
			}
		break;

		default:
		break;
		}
	}

void CEmTubePlayViewContainer::ClosePlayerL()
	{
	iMode = EClosePlayer;

	iStatusPane->MakeVisible( ETrue );
	iToolBar->MakeVisible( ETrue );

	iAppUi->CancelDownloadMovieL();
	if( iPlayer )
		{
		iPlayer->Close();
		}
	else
		{
		if( iAppUi->Embedded() )
			iAppUi->HandleCommandL( EAknCmdExit );
		else
			iAppUi->HandleCommandL( EAknSoftkeyBack );
		}
	}

void CEmTubePlayViewContainer::PlayPauseL( TBool aForegroundEvent )
	{
	if( iPlaybackReady )
		{
		if( iPause && !aForegroundEvent )
			{
			iPause = EFalse;

			HideVolumeIndicator();

			if( iTimer->IsActive() )
				iTimer->Cancel();

			iPlayer->Play();
			if( !iFullscreen )
				SetCurrentCbaL( R_EMTV_PLAY_VIEW_PLAYING_CBA );
			else
				iCurrentCba = R_EMTV_PLAY_VIEW_PLAYING_CBA;
			}
		else if( !iPause )
			{
			if( !iFullscreen )
				SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
			else
				iCurrentCba = R_EMTV_PLAY_VIEW_CBA;
			iPause = ETrue;
			iPlayer->Pause();

			iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlShowOsd, EEventKeyUp );

			if( iVideoEntry && !iFileDownloaded )
				iTimer->After( 1000000, EModeUpdateNaviPane );

			DrawNow();
			}
		}
	}

void CEmTubePlayViewContainer::SetCurrentCbaL( TInt aCba )
	{
	iCurrentCba = aCba;
	iView.SetCbaL( aCba );
	}

TKeyResponse CEmTubePlayViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	TKeyResponse ret = EKeyWasNotConsumed;
	TInt code = aKeyEvent.iCode;
	TInt scanCode = aKeyEvent.iScanCode;

	if( iFullscreen && iPlayer && iPlayer->DisplayRotated() )
		{
		switch (code)
			{
			case EKeyDownArrow:
				code = EKeyRightArrow;
				break;
			case EKeyUpArrow:
				code = EKeyLeftArrow;
				break;
			case EKeyLeftArrow:
				code = EKeyDownArrow;
				break;
			case EKeyRightArrow:
				code = EKeyUpArrow;
				break;
			}

		switch (scanCode)
			{
			case EStdKeyDownArrow:
				scanCode = EStdKeyRightArrow;
				break;
			case EStdKeyUpArrow:
				scanCode = EStdKeyLeftArrow;
				break;
			case EStdKeyLeftArrow:
				scanCode = EStdKeyDownArrow;
				break;
			case EStdKeyRightArrow:
				scanCode = EStdKeyUpArrow;
				break;
			}
		}


	if( aType == EEventKey )
		{
		switch (aKeyEvent.iCode)
			{
			case EKeyDevice3:
			case EKeyEnter:
				{
				PlayPauseL( EFalse );
				if( iPlayer )
					iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlPlayPause, EEventKeyUp );
				ret = EKeyWasConsumed;
				}
			break;

			default:
			break;

			}

		switch( aKeyEvent.iScanCode )
			{
			case 53:
				{
				CEmTubeAppUi::TVideoScaleMode up = iAppUi->VideoScaleMode();

				up = (CEmTubeAppUi::TVideoScaleMode)(((TInt)up) + 1);
				if( up > CEmTubeAppUi::EVideoScaleExtended )
					up = CEmTubeAppUi::EVideoScaleNone;
				
				iAppUi->SetVideoScaleMode( up );
				TRect rect;
				if( iFullscreen )
					rect = iFullscreenRect;
				else
					rect = iDefaultRect;
				iPlayer->SetResolutionL( rect, iFullscreen, KTransitionScale, ETrue );
				}
			break;

			case 49:
			case 82:
				{
				if( iPlaybackReady )
					{
                    ChangeDisplayModeL( );

                    if( iPause )
                    	DrawNow();
					ret = EKeyWasConsumed;
					}
				}
			break;

			case 51:
			case 89:
				{
				if( iPlaybackReady )
					{
					iRestartAfter = !iPause;
					if( !iPause )
						{
						PlayPauseL( EFalse );
						}
					iPlayer->SeekL ( 0 );

					if( iRestartAfter )
						PlayPauseL( EFalse );
					iRestartAfter = EFalse;

					iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlShowOsd, EEventKeyUp );
					ret = EKeyWasConsumed;
					}
				}
			break;

			case 52:
			case 70:
				{
				if( iPlaybackReady )
					{
					if( iMuted )
						{
						iPlayer->SetVolume( iAppUi->Volume() * 10 );
						ShowVolumeIndicator( iPlayer->Volume() );
						iMuted = EFalse;
						}
					else
						{
						iPlayer->SetVolume( 0 );
						ShowVolumeIndicator( 0 );
						iMuted = ETrue;
						}
					ret = EKeyWasConsumed;
					}
				}
			break;
			}

		switch( code )
			{
			case EKeyRightArrow:
				{
				if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
					{
					iPlayer->NextKeyFrameL( iSeekSpeed );
					}
				ret = EKeyWasConsumed;
				}
			break;

			case EKeyLeftArrow:
				{
				if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
					{
					iPlayer->PreviousKeyFrameL( iSeekSpeed );
					}
				ret = EKeyWasConsumed;
				}
			break;

			case EKeyDownArrow:
				{
				if( iPlaybackReady )
					{
					iMuted = EFalse;
					iPlayer->SetVolume( iPlayer->Volume() - 10 );
					iAppUi->SetVolume( iPlayer->Volume() / 10 );
					ShowVolumeIndicator( iPlayer->Volume() );
					ret = EKeyWasConsumed;
					}
				}
			break;

			case EKeyUpArrow:
				{
				if( iPlaybackReady )
					{
					iMuted = EFalse;
					iPlayer->SetVolume( iPlayer->Volume() + 10 );
					iAppUi->SetVolume( iPlayer->Volume() / 10 );
					ShowVolumeIndicator( iPlayer->Volume() );
					ret = EKeyWasConsumed;
					}
				}
			break;

			default:
			break;
			}
		}
	else if( aType == EEventKeyDown )
		{
		switch( scanCode )
			{
			case EKeyDevice3:
			case EKeyEnter:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlPlayPause, EEventKeyDown );
				}
			break;

			case EStdKeyRightArrow:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlForward, EEventKeyDown );
				ret = EKeyWasConsumed;
				StartSeekL();
				}
			break;

			case EStdKeyLeftArrow:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlRewind, EEventKeyDown );
				ret = EKeyWasConsumed;
				StartSeekL();
				}
			break;
			}
		}
	else if( aType == EEventKeyUp )
		{
		switch( scanCode )
			{
			case EKeyDevice3:
			case EKeyEnter:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlPlayPause, EEventKeyUp );
				}
			break;

			case EStdKeyRightArrow:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlForward, EEventKeyUp );
				ret = EKeyWasConsumed;
				StopSeekL();
				}
			break;

			case EStdKeyLeftArrow:
				{
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlRewind, EEventKeyUp );
				ret = EKeyWasConsumed;
				StopSeekL();
				}
			break;
			}
		}
	return ret;
	}

void CEmTubePlayViewContainer::TimerExpired( TInt aMode )
	{
	switch( aMode )
		{
		case EModeVolume:
			{
			HideVolumeIndicator();
			if( iVideoEntry && !iFileDownloaded && iPause )
				iTimer->After( 1000000, EModeUpdateNaviPane );
			}
		break;

		case EModeProgress:
			{
			UpdateNaviPane();

			if( iFileOpened && iWaitingForFile )
				{
				if( iFileError != KErrNone )
					{
					ShowErrorNoteL( R_PLAYER_CANNOT_OPEN_FILE_TXT );
					ClosePlayerL();
					}
				else
					{
					SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
					iProgressEnabled = EFalse;

					if( iVideoEntry && !iFileDownloaded && iPause )
						iTimer->After( 1000000, EModeUpdateNaviPane );
					}
				}
			else
				{
				if( iVideoEntry )
					{
					TBool start = EFalse;
					TInt sizeStart = 768*1024;
					TInt percentStart = 5;

					TInt percent = iProgressPercentage - iProgressPreviousPercentage;
					if( iDownloadSpeed > 30 )
						{
						percentStart = 1;
						sizeStart = 128*1024;
						}
					else if( iDownloadSpeed > 20 )
						{
						percentStart = 2;
						sizeStart = 368*1024;
						}
					else if( iDownloadSpeed > 10 )
						{
						percentStart = 3;
						sizeStart = 512*1024;
						}
					TInt sizeStartP = iProgressCompleteSize * percentStart / 100;
					iBufferingPercentage = sizeStartP ? (iProgressCurrentSize - iBufferingStartPosition) * 100 / sizeStartP : 0;

					if( iProgressCompleteSize != -1 )
						{
						if( (iProgressCurrentSize >= HTTP_BUFFER_SIZE) && (percent >= percentStart) )
							start = ETrue;
						}
					else
						{
						TInt bytes = iProgressCurrentSize - iProgressPreviousSize;

						if( bytes >= sizeStart )
							start = ETrue;
						}

					if( start )
						{
						ProgressComplete();
						}
					else
						{
						DrawNow();
						iTimer->After( 100000, EModeProgress );
						}
					}
				else
					{
					DrawNow();
					iTimer->After( 100000, EModeProgress );
					}

				iCurrentFrame++;
				if( iCurrentFrame > (iFrameCount - 1) )
					iCurrentFrame = 0;
				}
			}
		break;

		case EModeSeek:
			{
			if( iSeekSpeed != CDemuxer::ESeekSpeedHigh )
				{
				if( iSeekSpeed == CDemuxer::ESeekSpeedNormal )
					{
					iSeekSpeed = CDemuxer::ESeekSpeedMedium;
					iTimer->After( 3 * 1000000, EModeSeek );
					}
				else if( iSeekSpeed == CDemuxer::ESeekSpeedMedium )
					{
					iSeekSpeed = CDemuxer::ESeekSpeedHigh;
					}
				}
			}
		break;

		case EModeOpenFile:
			{
			if( iFileOpened )
				{
				if( iFileError != KErrNone )
					{
					ShowErrorNoteL( R_PLAYER_CANNOT_OPEN_FILE_TXT );
					ClosePlayerL();
					}
				else
					{
					SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
					iProgressEnabled = EFalse;

					switch( iAppUi->StartPlaybackMode() )
						{
						case CEmTubeAppUi::EStartPlaybackAfterDownload:
							if( iFileDownloaded && iPause )
								{
								PlayPauseL( EFalse );
								}
						break;

						default:
						break;
						}
					if( iVideoEntry && !iFileDownloaded && iPause )
						iTimer->After( 1000000, EModeUpdateNaviPane );
					}
				}
			else
				{
				iTimer->After( 1000, EModeOpenFile );
				}
			}
		break;

		case EModeUpdateNaviPane:
			{
			UpdateNaviPane();

			if( iVideoEntry && !iFileDownloaded && iPause )
				iTimer->After( 1000000, EModeUpdateNaviPane );
			}
		break;

		default:
		break;
		}
	}

void CEmTubePlayViewContainer::Draw(const TRect& /*aRect*/) const
	{
	if( iProgressEnabled )
		{
		CWindowGc& gc = SystemGc();

		gc.SetBrushColor( KRgbBlack );
		gc.SetPenColor( KRgbBlack );
		gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
		gc.SetPenStyle( CGraphicsContext::ENullPen );
		gc.DrawRect( Rect() );

		CFbsBitmap* bmp = iBitmaps[ iCurrentFrame * 2];
//		CFbsBitmap* mask = iBitmaps[ iCurrentFrame * 2 + 1];

		TSize size = bmp->SizeInPixels();
		TInt x = (Rect().Width() / 2) - ( size.iWidth / 2 );
		TInt y = (Rect().Height() / 2) - ( size.iHeight / 2 );
		if( iVideoEntry )
			y -= iFont->HeightInPixels();
		gc.BitBlt( TPoint( x, y ), bmp );

#ifdef __S60_50__
		if(iGoBackPressed)
			{
			TSize buttonSize = TSize( KGoBackButtonWidth * KSelectedButtonScale / 100, KGoBackButtonHeight * KSelectedButtonScale / 100);
			iGoBackRect = TRect( TPoint(iGoBackButtonPoint.iX - buttonSize.iWidth/2, iGoBackButtonPoint.iY - buttonSize.iHeight/2), buttonSize );
			gc.BitBltMasked( iGoBackRect.iTl, iGoBackBitmapPressed, TRect( TPoint(0,0), buttonSize), iGoBackMaskPressed, ETrue);
			}
		else
			{
			TSize buttonSize = TSize( KGoBackButtonWidth, KGoBackButtonHeight );
			iGoBackRect = TRect( TPoint(iGoBackButtonPoint.iX - buttonSize.iWidth/2, iGoBackButtonPoint.iY - buttonSize.iHeight/2), buttonSize );
			gc.BitBltMasked( iGoBackRect.iTl, iGoBackBitmap, TRect( TPoint(0,0), buttonSize), iGoBackMask, ETrue);
			}
#endif
		if( iVideoEntry )
			{
			gc.UseFont( iFont );
			gc.SetPenColor( KRgbWhite );
			gc.SetPenStyle( CGraphicsContext::ENullPen );
			gc.SetBrushStyle( CGraphicsContext::ENullBrush );

			TBuf<128> buf;
			if( iProgressCompleteSize == -1 )
				{
				buf.Copy( *iConnectingTxt );
				}
			else
				{
				buf.Copy( *iBufferingTxt );
				buf.AppendNum( (TInt)iBufferingPercentage );
				buf.Append( _L("%") );
				}
			TPoint p( x + (size.iWidth / 2) - (iFont->TextWidthInPixels( buf ) / 2), y + size.iHeight + (iFont->HeightInPixels() * 2) );
			gc.DrawText( buf, p );

			gc.DiscardFont();
			}
		}
	else if( iPlayer && iPause )
		{
		iPlayer->Refresh( SystemGc() );
		}
	else
		{
//TODO when switching to fullscreen using acelerometer this will be called when playback is enabled resulting in
//small blink.
		CWindowGc& gc = SystemGc();
		gc.SetBrushColor( KRgbBlack );
		gc.SetPenColor( KRgbBlack );
		gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
		gc.SetPenStyle( CGraphicsContext::ENullPen );
		gc.DrawRect( Rect() );
		}
	}

void CEmTubePlayViewContainer::HandleOsdButtonRepeatL( TInt aCommand )
	{
	TBool fb = EFalse;
	switch( aCommand )
		{
		case CEmTubeOsd::EOsdControlPosition:
		case CEmTubeOsd::EOsdControlVolume:
			{
			fb = ETrue;
			}
		break;

		case CEmTubeOsd::EOsdControlForward:
			{
			if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
				{
				iPlayer->NextKeyFrameL( iSeekSpeed );
				fb = ETrue;
				}
			}
		break;

		case CEmTubeOsd::EOsdControlRewind:
			{
			if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
				{
				iPlayer->PreviousKeyFrameL( iSeekSpeed );
				fb = ETrue;
				}
			}
		break;

		default:
		break;
		}

	if( fb )
		{
#ifdef __S60_50__
		MTouchFeedback* feedback = MTouchFeedback::Instance();
		if ( feedback )
			{
			feedback->InstantFeedback( this, ETouchFeedbackBasic );
			}
#endif
		}
	}

void CEmTubePlayViewContainer::HandleOsdButtonPressedL( TInt aCommand )
	{
	switch( aCommand )
		{
		case CEmTubeOsd::EOsdControlForward:
		case CEmTubeOsd::EOsdControlRewind:
			StartSeekL();
		break;

		case CEmTubeOsd::EOsdControlPosition:
			iRestartAfter = !iPause;
			iPause = ETrue;
		break;
		
		default:
		break;
		}
	}

void CEmTubePlayViewContainer::HandleOsdButtonReleasedL( TInt aCommand )
	{
	switch( aCommand )
		{
		case CEmTubeOsd::EOsdControlScaleVideo:
			{
			CEmTubeAppUi::TVideoScaleMode up = iAppUi->VideoScaleMode();

			up = (CEmTubeAppUi::TVideoScaleMode)(((TInt)up) + 1);
			if( up > CEmTubeAppUi::EVideoScaleExtended )
				up = CEmTubeAppUi::EVideoScaleNone;
			
			iAppUi->SetVideoScaleMode( up );
			TRect rect;
			if( iFullscreen )
				rect = iFullscreenRect;
			else
				rect = iDefaultRect;
			iPlayer->SetResolutionL( rect, iFullscreen, KTransitionScale, ETrue );
			}
		break;
		
		case CEmTubeOsd::EOsdControlFinish:
			iStatusPane->MakeVisible( ETrue );
			iToolBar->MakeVisible( ETrue );
			ClosePlayerL();
		break;

		case CEmTubeOsd::EOsdControlPosition:
			if( iRestartAfter )
				iPause = EFalse;
			iRestartAfter = EFalse;
		break;

		case CEmTubeOsd::EOsdControlPlayPause:
			PlayPauseL( EFalse );
		break;

		case CEmTubeOsd::EOsdControlForward:
		case CEmTubeOsd::EOsdControlRewind:
			StopSeekL();
		break;

		case CEmTubeOsd::EOsdControlNext:
#ifdef ENABLE_PLAYLISTS
		if( iManager->CurrentPlaylist() != KErrNotFound )
			{
			CEmTubePlaylistEntry* pe = iManager->Playlist( iManager->CurrentPlaylist() )->NextEntry();
			if( pe )
				iAppUi->OpenFileL( pe );
			}
#endif
		break;
		
		case CEmTubeOsd::EOsdControlPrev:
#ifdef ENABLE_PLAYLISTS
		if( iManager->CurrentPlaylist() != KErrNotFound )
			{
			CEmTubePlaylistEntry* pe = iManager->Playlist( iManager->CurrentPlaylist() )->PrevEntry();
			if( pe )
				iAppUi->OpenFileL( pe );
			}
#endif
		break;

		default:
		break;
		}
	}

void CEmTubePlayViewContainer::HandleOsdSliderPositionChangeL( TInt aCommand, TInt aValue )
	{
	switch( aCommand )
		{
		case CEmTubeOsd::EOsdControlVolume:
			{
			iPlayer->SetVolume( aValue * 10 );
			iMuted = EFalse;
			iAppUi->SetVolume( iPlayer->Volume() / 10 );
#ifdef __S60_50__
			MTouchFeedback* feedback = MTouchFeedback::Instance();
			if ( feedback )
				{
				feedback->InstantFeedback( this, ETouchFeedbackBasic );
				}
#endif
			}
		break;
		
		default:
		break;
		}
	}

void CEmTubePlayViewContainer::PlaybackFinished()
	{
	TBool end = ETrue;

	iRestartAfter = !iPause;

	if( iVideoEntry && !iFileDownloaded )
		end = EFalse;

	iPlayer->Pause();
	if( !iFullscreen )
		SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
	else
		iCurrentCba = R_EMTV_PLAY_VIEW_CBA;
	iPause = ETrue;

	if( end )
		{
		if( iFullscreen )
			{
#ifndef __S60_50__
//			CEikStatusPane *sp = iAppUi->StatusPane();
			iPlayer->SetResolutionL( iDefaultRect, EFalse, KTransitionScale );
			SetRect( iDefaultRect );
			iStatusPane->MakeVisible( ETrue );
			iToolBar->MakeVisible( ETrue );
			iView.SetCbaL( R_EMTV_PLAY_VIEW_CBA );
			iFullscreen = EFalse;
#endif
			}

#ifdef ENABLE_PLAYLISTS
		if( iManager->CurrentPlaylist() != KErrNotFound )
			{
			CEmTubePlaylistEntry* pe = iManager->Playlist( iManager->CurrentPlaylist() )->NextEntry();
			if( pe )
				iAppUi->OpenFileL( pe );
			else
				{
				iPlayer->SeekL( 0 );
				iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlShowOsd, EEventKeyUp );
				}
			}
		else
#endif
			{
			iPlayer->SeekL( 0 );
			iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlShowOsd, EEventKeyUp );
			}
		iPlayer->RefreshL();
		}
	else
		{
		iDownloadSpeed = 0;
		iBufferingStartPosition = iProgressCurrentSize;
		iProgressPreviousSize = iProgressCurrentSize;
		iProgressPreviousPercentage	= iProgressPercentage;
		iProgressEnabled = ETrue;
		iWaitingForFile = EFalse;

		iPlayer->HandleKeyEventL( CEmTubeOsd::EOsdControlResetEvents, EEventKeyUp );

		iTimer->After( 1, EModeProgress );
		}
	}

void CEmTubePlayViewContainer::FileOpenedL( TInt aError )
	{
	iFileError = aError;

	if( aError == KErrNone )
		{
		TVideoProperties& prop = iPlayer->Properties();
		if( prop.iDuration )
			iTotalTime = prop.iDuration;

		if( !iMuted )
			{
			iPlayer->SetVolume( iAppUi->Volume() * 10 );
			}
		else
			{
			iPlayer->SetVolume( 0 );
			}
		}

	iFileOpened = ETrue;
	}

void CEmTubePlayViewContainer::FileClosedL( TInt /*aError*/ )
	{
	switch( iMode )
		{
		case EIdle:
		break;

		case EOpenNewFile:
			{
			iPause = ETrue;
			iPlaybackReady = EFalse;
			iCurrentTime = -1;
			iFullscreen = EFalse;
			iFileOpened = EFalse;

			iWaitingForFile = ETrue;

			iProgressEnabled = ETrue;
			SetCurrentCbaL( R_EMTV_PROGRESS_DIALOG_CBA );
			iTimer->After( 1, EModeProgress );

			if( iView.FileName().Length() )
				iPlayer->OpenFileL( iView.FileName() );
			else
				iPlayer->OpenFileL( iView.FileHandle() );
			}
		break;

		case EClosePlayer:
			{
			iStatusPane->MakeVisible( ETrue );
			iToolBar->MakeVisible( ETrue );

			if( iAppUi->Embedded() )
				iAppUi->HandleCommandL( EAknCmdExit );
			else
				iAppUi->HandleCommandL( EAknSoftkeyBack );
			}
		break;
		}
	}

void CEmTubePlayViewContainer::PlaybackReady()
	{
	iPlaybackReady = ETrue;

	switch( iAppUi->StartPlaybackMode() )
		{
		case CEmTubeAppUi::EStartPlaybackAsap:
			if( iPause )
				{
				PlayPauseL( EFalse );
				}
		break;

		default:
		break;
		}
	}

void CEmTubePlayViewContainer::UpdateNaviPane()
	{
	if( iFullscreen )
		return;

	if( !iNaviVolumeDecorator && !iNaviImageDecorator )
		{
		TBuf<128> duration;
		TBuf<128> tmp;

		TTime ctime(0);
		ctime += (TTimeIntervalSeconds) (iCurrentTime == -1 ? 0 : iCurrentTime);
		ctime.FormatL( tmp, *iTimeFormatString);
		duration.Append( tmp );

		duration.Append( _L("/") );

		TTime ttime(0);
		ttime += (TTimeIntervalSeconds) iTotalTime;
		ttime.FormatL( tmp, *iTimeFormatString);
		duration.Append( tmp );

//		if( iProgressPercentage < 100 )
//			{
//			duration.Append( _L(" ") );
//			tmp.Format( _L("%d%% of %.2f MB"), iProgressPercentage, (float)iProgressCompleteSize/(1024.0*1024.0) );
//			duration.Append( tmp );
//			}

		static_cast<CAknNaviLabel*>( iNaviLabelDecorator->DecoratedControl())->SetTextL( duration );
		iNaviLabelDecorator->DrawDeferred();
		}

	TUid titlePaneUid = TUid::Uid( EEikStatusPaneUidTitle );
	CEikStatusPaneBase::TPaneCapabilities subPaneTitle = iStatusPane->PaneCapabilities( titlePaneUid );
	if ( subPaneTitle.IsPresent() && subPaneTitle.IsAppOwned() )
		{
		CAknTitlePane* title = static_cast< CAknTitlePane* >( iStatusPane->ControlL( titlePaneUid ) );

		TBuf<128> tmp;
		tmp.Copy( KNullDesC() );

		if( iProgressPercentage < 100 )
			{
			if( iProgressCompleteSize != -1 )
				tmp.Format( _L("%d%% of %.2f MB (%d kB/s)"), iProgressPercentage, (float)iProgressCompleteSize/(1024.0*1024.0), iDownloadSpeed );
			else
				tmp.Format( _L("%.2f of ??? MB (%d kB/s)"), iProgressCurrentSize/(1024.0*1024.0), iDownloadSpeed );
			}

		title->SetTextL( tmp ); //, ETrue );
		}
	}

void CEmTubePlayViewContainer::PositionUpdate( TInt32 aCurrent, TInt32 aTotal )
	{
	if( aCurrent != iCurrentTime )
		{
		User::ResetInactivityTime();
		iCurrentTime = aCurrent;
		if( aTotal )
			iTotalTime = aTotal;

		UpdateNaviPane();

		}
	}

_LIT(KFormat, "FlashVideo");
_LIT(KVCodecFLV1, "Sorenson H263");
_LIT(KVCodecVP6, "VP6 Flash");
_LIT(KVCodecVP6A, "VP6 Flash + Alpha");
_LIT(KCodecUnknown, "Unknown");
_LIT(KACodecMP3, "MPEG Audio");

_LIT( KDoubleListFormatTxt, "%S\t%S");

void CEmTubePlayViewContainer::ShowPropertiesL()
	{
	CAknDoublePopupMenuStyleListBox* plist = new(ELeave) CAknDoublePopupMenuStyleListBox;
	CleanupStack::PushL(plist);

	CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_OK_EMPTY, AknPopupLayouts::EMenuDoubleWindow);
	CleanupStack::PushL(popupList);

	plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
	plist->CreateScrollBarFrameL(ETrue);
	plist->ScrollBarFrame()->SetScrollBarVisibilityL(
							   CEikScrollBarFrame::EOff,
							   CEikScrollBarFrame::EAuto);

	MDesCArray* itemList = plist->Model()->ItemTextArray();
	CDesCArray* items = (CDesCArray*) itemList;

	HBufC* txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_TITLE_TXT );
	popupList->SetTitleL( *txt );
	CleanupStack::PopAndDestroy( txt );

	TVideoProperties& prop = iPlayer->Properties();

	RBuf item;
	CleanupClosePushL( item );
	txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_FORMAT_TXT );
	item.Create( txt->Length() + KFormat().Length() + 1 );
	item.Format( KDoubleListFormatTxt, txt, &KFormat() );
	items->AppendL( item );
	CleanupStack::PopAndDestroy( txt );
	CleanupStack::PopAndDestroy( &item );

	TBuf<16> duration;
	TTime etime(0);
	etime += (TTimeIntervalSeconds) iTotalTime;
	etime.FormatL( duration, *iTimeFormatString);
	CleanupClosePushL( item );
	txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_DURATION_TXT );
	item.Create( txt->Length() + duration.Length() + 1 );
	item.Format( KDoubleListFormatTxt, txt, &duration );
	items->AppendL( item );
	CleanupStack::PopAndDestroy( txt );
	CleanupStack::PopAndDestroy( &item );

	if( prop.iHasVideo )
		{
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_VIDEO_COMPRESSION_TXT );
		switch( prop.iVideoCodecId )
			{
			case VIDEO_CODEC_FLV:
				item.Create( txt->Length() + KVCodecFLV1().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KVCodecFLV1() );
			break;

			case VIDEO_CODEC_VP6F:
				item.Create( txt->Length() + KVCodecVP6().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KVCodecVP6() );
			break;

			case VIDEO_CODEC_VP6A:
				item.Create( txt->Length() + KVCodecVP6A().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KVCodecVP6A() );
			break;

			default:
				item.Create( txt->Length() + KCodecUnknown().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KCodecUnknown() );
			break;
			}
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		TBuf<16> width;
		width.Format( _L("%d px"), prop.iWidth );
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_WIDTH_TXT );
		item.Create( txt->Length() + width.Length() + 1 );
		item.Format( KDoubleListFormatTxt, txt, &width );
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		TBuf<16> height;
		height.Format( _L("%d px"), prop.iHeight );
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_HEIGHT_TXT );
		item.Create( txt->Length() + height.Length() + 1 );
		item.Format( KDoubleListFormatTxt, txt, &height );
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		TBuf<10> fps;
		fps.Format( _L("%.2f"), prop.iFps );
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_FPS_TXT );
		item.Create( txt->Length() + fps.Length() + 1 );
		item.Format( KDoubleListFormatTxt, txt, &fps );
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );
		}

	if( prop.iHasAudio )
		{
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_AUDIO_COMPRESSION_TXT );
		switch( prop.iAudioCodecId )
			{
			case AUDIO_CODEC_MP3:
				item.Create( txt->Length() + KACodecMP3().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KACodecMP3() );
			break;

			default:
				item.Create( txt->Length() + KCodecUnknown().Length() + 1 );
				item.Format( KDoubleListFormatTxt, txt, &KCodecUnknown() );
			break;
			}
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		TBuf<16> rate;
		rate.Format( _L("%d"), prop.iSampleRate );
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_SAMPLERATE_TXT );
		item.Create( txt->Length() + rate.Length() + 1 );
		item.Format( KDoubleListFormatTxt, txt, &rate );
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		TBuf<16> channels;
		channels.Format( _L("%d"), prop.iChannels );
		CleanupClosePushL( item );
		txt = StringLoader::LoadLC( R_PROPERTIES_DIALOG_CHANNELS_TXT );
		item.Create( txt->Length() + channels.Length() + 1 );
		item.Format( KDoubleListFormatTxt, txt, &channels );
		items->AppendL( item );
		CleanupStack::PopAndDestroy( txt );
		CleanupStack::PopAndDestroy( &item );

		}

	popupList->ExecuteLD();
	CleanupStack::Pop( popupList );
	CleanupStack::PopAndDestroy( plist );
	}

//http/progress interface
void CEmTubePlayViewContainer::RequestFinishedL( TInt /*aRequest*/, TDesC8& /*aResponseBuffer*/ )
	{
	iFileDownloaded = ETrue;

	if( iVideoEntry )
		{
		iVideoEntry->SetDownloadFinished( ETrue );
		switch( iAppUi->StartPlaybackMode() )
			{
			case CEmTubeAppUi::EStartPlaybackAfterDownload:
				if( iPause )
					{
					PlayPauseL( EFalse );
					}
			break;

			default:
			break;
			}
		}
	}

void CEmTubePlayViewContainer::RequestCanceledL( TInt /*aRequest*/ )
	{
	if( iMode != EClosePlayer )
		{
		iFileDownloaded = ETrue; //not true actually, but we do not want to continue downloading.

//		SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
		if( iTimer->IsActive() )
			iTimer->Cancel();
		iProgressEnabled = EFalse;

		iStatusPane->MakeVisible( ETrue );
		iToolBar->MakeVisible( ETrue );

		if( iAppUi->Embedded() )
			iAppUi->HandleCommandL( EAknCmdExit );
		else
			iAppUi->HandleCommandL( EMTVActivatePreviousViewCommand );
		}
	}

TBool CEmTubePlayViewContainer::CheckDiskSpaceL( const TDesC& aFileName, TInt aSize )
	{
	return iAppUi->CheckDiskSpaceL( aFileName, aSize );
	}

void CEmTubePlayViewContainer::ShowErrorNoteL( TInt aResourceId )
	{
	iAppUi->ShowErrorNoteL( aResourceId );
	}

void CEmTubePlayViewContainer::ShowErrorNoteL( const TDesC& aText )
	{
	iAppUi->ShowErrorNoteL( aText );
	}

void CEmTubePlayViewContainer::ProgressStart( TInt aCompleteSize )
	{
	iProgressTicks = CVideoTimer::Time();
	iProgressCompleteSize = aCompleteSize;
	}

void CEmTubePlayViewContainer::ProgressUpdate( TInt aCurrent, TInt aDownloadSpeed )
	{
	iProgressCurrentSize = aCurrent;
	iDownloadSpeed = aDownloadSpeed;

	if( iProgressCompleteSize != -1 )
		iProgressPercentage = (TUint32)( ((float)aCurrent * 100.0f) / (float)iProgressCompleteSize );

	if( iPlayer && iPause )
		DrawNow();
	}

void CEmTubePlayViewContainer::ProgressComplete()
	{
	UpdateNaviPane();
	if( !iPlayer )
		{
		SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
		if( iTimer->IsActive() )
			iTimer->Cancel();
		iProgressEnabled = EFalse;

		RFile file;
		TInt ret = file.Open( iSession, iAppUi->VideoFileName(), EFileRead | EFileStream | EFileShareAny );

		TBuf8<9> buf;
		buf.Copy( KNullDesC8() );
		TInt size = 0;

		if( ret == KErrNone )
			{
			file.Size( size );
			file.Read( buf );
			file.Close();
			}

		if( !buf.Compare(_L8("<html")) || !buf.Compare( _L8("<!DOCTYPE")) )
			{
 			ShowErrorNoteL( R_HTTP_HTML_RESPONSE_TXT );
			ClosePlayerL();
			return;
			}

		if( size == 0 )
			{
//			ShowErrorNoteL( R_HTTP_EMPTY_RESPONSE_TXT );
			ClosePlayerL();
			return;
			}

		iPlayer = CEmTubePlayer::NewL( iAppUi->VideoFileName(),
											this,
											iEikonEnv->WsSession(),
											*(CCoeEnv::Static()->ScreenDevice()),
											Window(), Rect(), this, iFullscreen );

		iTimer->After( 1000, EModeOpenFile );
		}
	else
		{
		if( iPause )
			SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
		else
			SetCurrentCbaL( R_EMTV_PLAY_VIEW_PLAYING_CBA );

		if( iTimer->IsActive() )
			iTimer->Cancel();
		iProgressEnabled = EFalse;

		if( iRestartAfter )
			{
			iPause = EFalse;
			iPlayer->Play();
			if( !iFullscreen )
				SetCurrentCbaL( R_EMTV_PLAY_VIEW_PLAYING_CBA );
			else
				iCurrentCba = R_EMTV_PLAY_VIEW_PLAYING_CBA;
			}
		else
			{
			iPlayer->Prepare();
			DrawNow();
			}
		}
	}

void CEmTubePlayViewContainer::SaveFileL()
	{
	TInt ret;
	TFileName fileName;

	TBool done = EFalse;
	TBool memDialog = ETrue;

	if( iAppUi->SaveLoadDirectory().Length() )
		memDialog = EFalse;

	while( !done )
		{

		if( memDialog )
			{
			fileName.Copy( KNullDesC() );

			CAknMemorySelectionDialog::TMemory memory = CAknMemorySelectionDialog::EPhoneMemory;
			CAknMemorySelectionDialog* memDlg = CAknMemorySelectionDialog::NewL( ECFDDialogTypeSelect, R_MEMORY_SELECTION_DIALOG );

			ret = memDlg->ExecuteL( memory );
			if( ret )
				{
				if( memory == CAknMemorySelectionDialog::EPhoneMemory )
					fileName.Copy( _L("C:\\") );
				else if( memory == CAknMemorySelectionDialog::EMemoryCard )
					fileName.Copy( _L("E:\\") );
				}
			delete memDlg;
			}
		else
			{
			TParsePtrC parse( iAppUi->SaveLoadDirectory() );
			fileName.Copy( parse.Drive() );
			fileName.Append( _L("\\") );
			ret = 1;
			}

		memDialog = ETrue;

		if( ret )
			{
			CAknFileSelectionDialog* dlg = CAknFileSelectionDialog::NewL( ECFDDialogTypeSave, R_FILE_SELECTION_DIALOG );

			if( iAppUi->SaveLoadDirectory().Length() )
				{
				TParsePtrC parse( iAppUi->SaveLoadDirectory() );
				TPtrC path = parse.Path();
				TPtrC path2 = path.Right( path.Length() - 1 );
				dlg->SetDefaultFolderL( path.Right( path.Length() - 1 ) );
				}

			ret = dlg->ExecuteL( fileName );
			delete dlg;

			if( ret )
				{
				TFileName name;
				CAknFileNamePromptDialog* dlg = CAknFileNamePromptDialog::NewL();

				dlg->SetPathL( fileName );

				HBufC* tmp = iAppUi->ConvertFileNameL( iVideoEntry->MediaTitle() );
				CleanupStack::PushL( tmp );
				if( tmp->Length() > KMaxFileName )
					name.Copy( tmp->Mid( 0, KMaxFileName ) );
				else
					name.Copy( *tmp );
				CleanupStack::PopAndDestroy( tmp );

				ret = dlg->ExecuteL( name );
				delete dlg;

				if( !ret )
					fileName.Copy( KNullDesC() );
				else
					fileName.Append( name );

				done = ETrue;
				}
			}
		else
			{
			done = ETrue;
			}
		}

	if( fileName.Length() )
		{
		iAppUi->SetSaveLoadDirectory( fileName );

		TFileName ext;

		TParsePtrC parse( fileName );
		ext = parse.Ext();

		if( ext.Compare( _L(".flv") ) )
			fileName.Append( _L(".flv") );

		TInt err = iInFile.Open( iSession, iAppUi->VideoFileName(), EFileStream | EFileRead | EFileShareAny );
		if( err == KErrNone )
			{
			TInt size;
			iInFile.Size( size );

			if( iAppUi->CheckDiskSpaceL( fileName, size ) )
				{
				ret = iOutFile.Replace( iSession, fileName, EFileStream | EFileWrite | EFileShareAny);
				if( ret != KErrNone )
					{
					ShowErrorNoteL( R_CANNOT_COPY_FILE_TXT );
					}
				else
					{
					iVideoEntry->SetSavedFileName( fileName );

					iProgressDialog = new ( ELeave ) CAknProgressDialog( reinterpret_cast<CEikDialog**> ( &iProgressDialog ), ETrue );

					iProgressDialog->SetCallback( this );
					iProgressDialog->PrepareLC( R_EMTV_SAVE_FILE_PROGRESS_DIALOG );
					iProgressInfo = iProgressDialog->GetProgressInfoL();
					iProgressInfo->SetFinalValue( size );
					iProgressDialog->RunLD();

					delete iIdle;
					iIdle = CIdle::NewL( CActive::EPriorityStandard );
					TCallBack callback( CallbackIncrementProgressNoteL, this );
					iIdle->Start( callback );

					}
				}
			else
				{
				ShowErrorNoteL( R_NOT_ENOUGH_SPACE_TXT );
				}
			}
		else
			{
			ShowErrorNoteL( R_CANNOT_COPY_FILE_TXT );
			}
		}
	}

TInt CEmTubePlayViewContainer::CallbackIncrementProgressNoteL( TAny* aThis )
	{
	return static_cast<CEmTubePlayViewContainer*>( aThis )->UpdateProgressNote();
	}

TInt CEmTubePlayViewContainer::UpdateProgressNote()
	{
	iInFile.Read( iFileBuf );
	TInt bytes = iFileBuf.Length();
	iOutFile.Write( iFileBuf );

	iProgressInfo->IncrementAndDraw( bytes );

	if( bytes == 0 )
		{
		iAppUi->AddToSavedClipsL( iVideoEntry );
		iInFile.Close();
		iOutFile.Close();
		iProgressDialog->ProcessFinishedL();
		delete iProgressDialog;
		iProgressDialog = NULL;
		return 0;
		}
	return 1;
	}

void CEmTubePlayViewContainer::DialogDismissedL( TInt aButtonId )
	{
	if ( aButtonId == -1 )
		{
		delete iIdle;
		iIdle = NULL;

		TFileName name;
 		iInFile.FullName( name );

		iInFile.Close();
		iOutFile.Close();

		BaflUtils::DeleteFile( iSession, name );
		}
	}

#ifndef __S60_50__
void CEmTubePlayViewContainer::HandleDataEventL( TRRSensorInfo aSensor, TRRSensorEvent aEvent )
	{
	if( iAppUi->AutoRotate() )
		{
		switch ( aSensor.iSensorId )
			{
			case Kacc:
				{
				if( iPlaybackReady )
					{
					if( aEvent.iSensorData1 == 90 && iFullscreen )
						{

						if( iDefaultRect.Width() > iDefaultRect.Height() )
							iPlayer->SetResolutionL( iDefaultRect, EFalse, KTransitionScale );
						else
							iPlayer->SetResolutionL( iDefaultRect, EFalse, KTransitionScale | KTransitionRotate );

						SetRect( iDefaultRect );
						iStatusPane->MakeVisible( ETrue );
						iToolBar->MakeVisible( ETrue );
						iView.SetCbaL( CurrentCba() );
						iFullscreen = EFalse;
						if( iPause )
							DrawNow();
						}
					else if( aEvent.iSensorData1 == 0 && !iFullscreen )
						{
//						iStatusPane->MakeVisible( EFalse );
//						iToolBar->MakeVisible( EFalse );
						iFullscreen = ETrue;

						AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EScreen, iFullscreenRect );
						if( iDefaultRect.Width() > iDefaultRect.Height() )
							iPlayer->SetResolutionL( iFullscreenRect, ETrue, KTransitionScale );
						else
							iPlayer->SetResolutionL( iFullscreenRect, ETrue, KTransitionScale | KTransitionRotate );

						SetExtentToWholeScreen();
						iView.SetCbaL( R_EMTV_PLAY_VIEW_FULL_SCREEN_CBA );

						if( iPause )
							DrawNow();
						}

					User::ResetInactivityTime();
					}
				}
			break;

			default:
		  	break;

			}
		}
	}
#endif

TInt CEmTubePlayViewContainer::ProgressPercentage()
	{
	return iProgressPercentage;
	}

TBool CEmTubePlayViewContainer::PlaybackEnabled()
	{
	return !iPause;
	}

TInt CEmTubePlayViewContainer::Volume()
	{
	TInt vol = 0;
	if( iPlayer )
		vol = iPlayer->Volume() / 10;
	return vol;
	}

TInt CEmTubePlayViewContainer::ScaleMode()
	{
	return (TInt)iAppUi->VideoScaleMode();
	}

TBool CEmTubePlayViewContainer::NextTrackExists()
	{
#ifdef ENABLE_PLAYLISTS
		if( iManager->CurrentPlaylist() != KErrNotFound )
			{
			if( iManager->Playlist( iManager->CurrentPlaylist() )->CheckEntry( ETrue ) )
				{
				return ETrue;
				}
			}
#endif
	return EFalse;
	}

TBool CEmTubePlayViewContainer::PrevTrackExists()
	{
#ifdef ENABLE_PLAYLISTS
		if( iManager->CurrentPlaylist() != KErrNotFound )
			{
			if( iManager->Playlist( iManager->CurrentPlaylist() )->CheckEntry( EFalse ) )
				{
				return ETrue;
				}
			}
#endif
	return EFalse;
	}

void CEmTubePlayViewContainer::ChangeDisplayModeL( )
    {
	if( iFullscreen )
		{
		iPlayer->SetResolutionL( iDefaultRect, EFalse );
		SetRect( iDefaultRect );
		iStatusPane->MakeVisible( ETrue );
		iToolBar->MakeVisible( ETrue );
		iView.SetCbaL( CurrentCba() );
		iFullscreen = EFalse;
		}
	else
		{
		iFullscreen = ETrue;

		if( iDefaultRect.Width() > iDefaultRect.Height() )
			iPlayer->SetResolutionL( iFullscreenRect, ETrue, KTransitionScale );
		else
			iPlayer->SetResolutionL( iFullscreenRect, ETrue );

		SetExtentToWholeScreen();
		iView.SetCbaL( R_EMTV_PLAY_VIEW_FULL_SCREEN_CBA );

//		iStatusPane->MakeVisible( EFalse );
//		iToolBar->MakeVisible( EFalse );
		}
	}

#ifdef __S60_50__
void CEmTubePlayViewContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
	{
	// Check if touch is enabled or not
	if( !AknLayoutUtils::PenEnabled() )
		{
		return;
		}

	TBool consumed = EFalse;
	TBool exit= EFalse;

	if( iProgressEnabled )
		{
		if( ( (aPointerEvent.iType == TPointerEvent::EButton1Down) || ((aPointerEvent.iType == TPointerEvent::EDrag) ) )
				&& iGoBackRect.Contains( aPointerEvent.iPosition ) )
			iGoBackPressed = consumed = ETrue;
		else
			iGoBackPressed = EFalse;

		if( (aPointerEvent.iType == TPointerEvent::EButton1Up) &&
				iGoBackRect.Contains( aPointerEvent.iPosition ) )
			consumed = exit = ETrue;
		}

	if( iPlayer && !consumed )
		consumed = iPlayer->HandleTouchEventL( aPointerEvent );

	if( consumed && iPause )
		DrawNow();

	if( consumed && aPointerEvent.iType == TPointerEvent::EButton1Down )
		{
		MTouchFeedback* feedback = MTouchFeedback::Instance();
		if ( feedback )
			{
			feedback->InstantFeedback( this, ETouchFeedbackBasic );
			}
		}
	// Call base class HandlePointerEventL()
	if( !consumed )
		CCoeControl::HandlePointerEventL(aPointerEvent);

	if( exit )
		iView.HandleCommandL(EAknSoftkeyBack);
	}

#endif

void CEmTubePlayViewContainer::StartSeekL()
	{
	if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
		{
		HideVolumeIndicator();
		iPlayer->ShowPositionIndicator();

		iSeekSpeed = CDemuxer::ESeekSpeedNormal;
		iRestartAfter = !iPause;

		if( !iPause )
			{
			if( !iFullscreen )
				SetCurrentCbaL( R_EMTV_PLAY_VIEW_CBA );
			else
				iCurrentCba = R_EMTV_PLAY_VIEW_CBA;
			iPause = ETrue;
			iPlayer->Pause();
			DrawNow();
			}

//		iProgressEnabled = EFalse;
		iTimer->After( 2 * 1000000, EModeSeek );
		}
	}

void CEmTubePlayViewContainer::StopSeekL()
	{
	if( iPlaybackReady && iPlayer->Seekable() && !iProgressEnabled )
		{
		if( iRestartAfter )
			{
			iPause = EFalse;
			iPlayer->Play();
			if( !iFullscreen )
				SetCurrentCbaL( R_EMTV_PLAY_VIEW_PLAYING_CBA );
			else
				iCurrentCba = R_EMTV_PLAY_VIEW_PLAYING_CBA;
			}
		else
			{
			if( iVideoEntry && !iFileDownloaded && iPause )
				iTimer->After( 1000000, EModeUpdateNaviPane );
			}

		iPlayer->HidePositionIndicator();
		if( iPause )
			DrawNow();
		}
	}

void CEmTubePlayViewContainer::HandleLineStatusChangeL( CTelephony::TCallStatus& aStatus )
	{
	if( aStatus == CTelephony::EStatusDialling && !iPause )
		{
		PlayPauseL( ETrue );
		}
	}

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

#ifndef EMTUBE_PLAYVIEW_CONTAINER_H
#define EMTUBE_PLAYVIEW_CONTAINER_H

#include <coecntrl.h>
#include <coecobs.h>
#include <aknlists.h> // list
#include <e32std.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <remconcoreapitargetobserver.h>
#include <RemConCoreApiTarget.h>
#include <remconinterfaceselector.h>
#include <eikprogi.h>

#ifndef __S60_50__
#include <RRSensorApi.h>
#endif

#include <aknprogressdialog.h>

#include "emTubePlayView.h"
#include "emTubePlayer.h"
#include "emTubeTimeOutTimer.h"
#include "videohandler.h"
#include "emTubePlayerDemuxer.h"

#include "emTubeProgress.h"
#include "emTubeHttpEngine.h"

#include "emTubeLineStatusMonitor.h"

class CEmTubeAppUi;

class CEmTubePlayViewContainer : public CCoeControl, public MVideoHandler, public MTimeOutObserver,
										public MProgressObserver, public MHttpEngineObserver,
										public MRemConCoreApiTargetObserver,
										public MProgressDialogCallback,
#ifndef __S60_50__
										public MRRSensorDataListener,
#endif
										public MOsdCallback,
										public MLineStatusObserver
	{
public:
	enum TTimerMode
		{
		EModeVolume = 0,
		EModeProgress,
		EModeSeek,
		EModeOpenFile,
		EModeUpdateNaviPane,
		EModeHandleTransactionComplete,
		EModeHandleTransactionCancel
		};

	enum TMode
		{
		EIdle = 0,
		EOpenNewFile,
		EClosePlayer
		};

public: // construction / destruction
	static CEmTubePlayViewContainer* NewL(CEmTubePlayView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager );
	static CEmTubePlayViewContainer* NewLC(CEmTubePlayView& aView, const TRect& aRect, CEmTubePlaylistManager* aManager );
	~CEmTubePlayViewContainer();

private: // construction
	CEmTubePlayViewContainer(CEmTubePlayView& aView, CEmTubePlaylistManager* aManager );
	void ConstructL(const TRect& aRect);
	void SizeChanged();

public: //from MHttpEngineObserver
	void RequestFinishedL( TInt aState, TDesC8& aResponseBuffer );
	void RequestCanceledL( TInt aState );
	TBool CheckDiskSpaceL( const TDesC& aFileName, TInt aSize );
	void ShowErrorNoteL( TInt aResourceId );
	void ShowErrorNoteL( const TDesC& aText );

public: //from MRemConCoreApiTargetObserver
	void MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct);

public: //from MLineStatusObserver
	void HandleLineStatusChangeL( CTelephony::TCallStatus& aStatus );
	
public: //from MProgressObserver
	void ProgressStart( TInt aCompleteSize );
	void ProgressUpdate( TInt aCurrent, TInt aDownloadSpeed );
	void ProgressComplete();

public: //from MOsdCallback
	TInt ProgressPercentage();
	TBool NextTrackExists();
	TBool PrevTrackExists();
	TBool PlaybackEnabled();
	TInt Volume();
	TInt ScaleMode();

public:
	void PlaybackFinished();
	void PlaybackReady();
	void PositionUpdate( TInt32 aCurrent, TInt32 aTotal );

	void FileOpenedL( TInt aError );
	void FileClosedL( TInt aError );

	void HandleOsdButtonPressedL( TInt aCommand );
	void HandleOsdButtonReleasedL( TInt aCommand );
	void HandleOsdButtonRepeatL( TInt aCommand );
	void HandleOsdSliderPositionChangeL( TInt aCommand, TInt aValue );

	void ShowPropertiesL();
	TBool IsPlaybackReady() { return iPlaybackReady; }

	void OpenFileL();

	void PlayPauseL( TBool aForegroundEvent );
	void ClosePlayerL();

	TBool FileDownloaded() { return iFileDownloaded; }

	void SaveFileL();

	TInt CurrentCba() { return iCurrentCba; }

	CVideoEntry* VideoEntry() { return iVideoEntry; }

	void ChangeDisplayModeL();

protected: // from CCoeControl
	TInt CountComponentControls() const;
	CCoeControl* ComponentControl( TInt aIndex ) const;
	void HandleResourceChange(TInt aType);
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
#ifdef __S60_50__
	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
#endif
	void GetHelpContext( TCoeHelpContext& aContext ) const;

public:  // from CCoeControl
	void Draw(const TRect& aRect) const;

public:  // MTimeOutObserver
	void TimerExpired( TInt aMode );

public: // MProgressDialogCallback
	void DialogDismissedL( TInt aButtonId );

#ifndef __S60_50__
public:
	void HandleDataEventL( TRRSensorInfo aSensor, TRRSensorEvent aEvent );
#endif

private:
	void UpdateVolume( TInt aVolume ); //percentage 0-100
	void HideVolumeIndicator();
	void ShowVolumeIndicator( TInt aVolume );

	void UpdateNaviPane();

	void SetCurrentCbaL( TInt aCba );

	static TInt CallbackIncrementProgressNoteL( TAny* aThis );
	TInt UpdateProgressNote();

	void StartSeekL();
	void StopSeekL();

private: // data
	CEikButtonGroupContainer* iToolBar;
	CEikStatusPane* iStatusPane;

	HBufC* iTimeFormatString;

	TInt iCurrentCba;

	CEmTubePlayView& iView;
	CEmTubeAppUi* iAppUi;

	CEmTubePlayer* iPlayer;
	CAknNavigationControlContainer* iNaviPane;
	CAknNavigationDecorator* iNaviVolumeDecorator;
	CAknNavigationDecorator* iNaviImageDecorator;
	CAknNavigationDecorator* iNaviLabelDecorator;

	CEmTubeTimeOutTimer *iTimer;

	CVideoEntry* iVideoEntry;

	TBool iFileDownloaded;

	TInt32 iCurrentTime;
	TInt32 iTotalTime;
	TBool iFullscreen;
	TRect iDefaultRect;
	TRect iFullscreenRect;
	TBool iPause;
	TBool iPlaybackReady;
	TBool iMuted;

	TMode iMode;

//progress bar
	TBool iProgressEnabled;

	TInt iFrameCount;
	RPointerArray<CFbsBitmap> iBitmaps;
	TInt iCurrentFrame;
	TInt iProgressCompleteSize;
	TInt iProgressCurrentSize;
	TUint iProgressPercentage;

	TUint iProgressPreviousPercentage;
	TInt  iProgressPreviousSize;
	TUint iDownloadSpeed;

	TBool iWaitingForFile;

	TUint32 iProgressTicks;
//

	CDemuxer::TSeekSpeed iSeekSpeed;
	TBool iRestartAfter;
//

	TInt iFileError;
	TBool iFileOpened;

	CRemConInterfaceSelector* iInterfaceSelector;
	CRemConCoreApiTarget* iCoreTarget;

#ifndef __S60_50__
	CRRSensorApi* iAccSensor;
	RLibrary iSensorLibrary;
#endif

	CIdle* iIdle;
	CAknProgressDialog* iProgressDialog;
	CEikProgressInfo* iProgressInfo;

	RFs iSession;
	RFile iInFile;
	RFile iOutFile;
	TBuf8<65535> iFileBuf;

	TUint32 iBufferingStartPosition;
	TUint32 iBufferingPercentage;
	CEmTubePlaylistManager* iManager;
	CFont *iFont;
	HBufC* iConnectingTxt;
	HBufC* iBufferingTxt;

	// back button in touch ui
#ifdef __S60_50__
	CFbsBitmap* iGoBackBitmap;
	CFbsBitmap* iGoBackMask;
	CFbsBitmap* iGoBackBitmapPressed;
	CFbsBitmap* iGoBackMaskPressed;
	TBool iGoBackPressed;
	mutable TRect iGoBackRect;
	TPoint iGoBackButtonPoint;
#endif	
	CEmTubeLineStatusMonitor* iLineStatusMonitor;

	};

#endif // EMTUBE_PLAYVIEW_CONTAINER_H

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

#ifndef EMTUBE_PLAYVIEW_H
#define EMTUBE_PLAYVIEW_H

#ifndef EMTUBE_UIQ
#include <aknview.h>
#else
#include <qikviewbase.h>
#include <MQikListBoxObserver.h>
#include <QikCommand.h>
#endif

class CEmTubePlayViewContainer;
class CEmTubeAppUi;
class CVideoEntry;
class CEmTubePlaylistManager;

#ifndef EMTUBE_UIQ
class CEmTubePlayView : public CAknView
#else
class CEmTubePlayView : public CQikViewBase
#endif
	{

public: // construction / destruction
#ifndef EMTUBE_UIQ
	static CEmTubePlayView* NewL( CEmTubePlaylistManager* aManager );
	static CEmTubePlayView* NewLC( CEmTubePlaylistManager* aManager );
#else
	static CEmTubePlayView* NewL(CQikAppUi &aAppUi, TVwsViewId aParentViewId);
	static CEmTubePlayView* NewLC(CQikAppUi &aAppUi, TVwsViewId aParentViewId);
#endif
	~CEmTubePlayView();

private: // construction
	CEmTubePlayView( CEmTubePlaylistManager* aManager );
	void ConstructL();

public:

	TUid Id() const;
	void HandleCommandL( TInt aCommand );
	void DoActivateL(const TVwsViewId& aPrevViewId,
				TUid aCustomMessageId,
				const TDesC8& aCustomMessage);

	void DoDeactivate();

protected: // from CAknView
	void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane *aMenuPane);

public:
	void PlayPauseL( TBool aForegroundEvent );

	void OpenFileL( const TDesC& aFileName );
	void SetFileNameL( const TDesC& aFileName );
	const TDesC& FileName() { return *iFileName; }

	void OpenFileL( RFile& aFile );
	void SetFileHandleL( RFile& aFile );
	RFile& FileHandle() { return iFile; }

	void OpenFileL( CVideoEntry* aEntry );
	void SetVideoEntry( CVideoEntry* aEntry );
	CVideoEntry* VideoEntry() { return iVideoEntry; }

	void SetCbaL( TInt aCbaResource );

private: // data
	CEmTubePlayViewContainer* iContainer;
	CEmTubeAppUi* iAppUi;

	CVideoEntry* iVideoEntry;
	HBufC* iFileName;

	RFile iFile;

	HBufC* iStatusPaneText;

	CEmTubePlaylistManager* iManager;

	};

#endif // EMTUBE_PLAYVIEW_H

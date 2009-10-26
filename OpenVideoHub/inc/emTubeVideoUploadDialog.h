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

#ifndef EMTUBE_VIDEO_UPLOAD_DIALOG_H
#define EMTUBE_VIDEO_UPLOAD_DIALOG_H

#include <aknform.h>
#include <akntitle.h>

class CQueueEntry;

class CEmTubeVideoUploadDialog : public CAknForm
    {
public:
    static CEmTubeVideoUploadDialog* NewLC( CQueueEntry& aEntry );
    ~CEmTubeVideoUploadDialog();

public:
    void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
    void ProcessCommandL(TInt aCommandId);

protected: // From CAknForm
    TBool OkToExitL(TInt aButtonId);
    TBool SaveFormDataL();

private: // From CEikDialog
    void PreLayoutDynInitL();
    void PostLayoutDynInitL();
    void SetInitialCurrentLine();
    void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType);

private:
    CEmTubeVideoUploadDialog( CQueueEntry& aEntry );
    void ConstructL();

private: // Data
	CQueueEntry* iQueueEntry;
	CAknTitlePane* iTitlePane;
	HBufC* iOriginalTitle;
    };

#endif // EMTUBE_VIDEO_UPLOAD_DIALOG_H

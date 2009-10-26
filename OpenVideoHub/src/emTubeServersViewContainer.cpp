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

#include <AknLists.h>
#include <Avkon.hrh>
#include <AknIconArray.h>
#include <AknIconUtils.h>
#include <AknNoteWrappers.h>
#include <AknUtils.h>
#include <barsread.h>
#include <gulicon.h>
#include <f32file.h>
#include <GDI.H>
#include <stringloader.h>
#include <apgcli.h>

#include <avkon.mbg>

#include "emTubeResource.h"
#include <PluginInterface.h>

#include "emTube.hrh"
#include "OpenVideohub.hlp.hrh"

#include "emTubeApplication.h"
#include "emTubeAppUi.h"
#include "emTubeServersView.h"
#include "emTubeServersViewContainer.h"
#include "emTubePluginManager.h"

_LIT(KAvkonBitmapFileName, "z:\\resource\\apps\\avkon2.mbm");

CEmTubeServersViewContainer* CEmTubeServersViewContainer::NewL( CEmTubeServersView& aView, const TRect& aRect )
	{
	CEmTubeServersViewContainer* self = CEmTubeServersViewContainer::NewLC( aView, aRect );
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeServersViewContainer* CEmTubeServersViewContainer::NewLC( CEmTubeServersView& aView, const TRect& aRect )
	{
	CEmTubeServersViewContainer* self = new ( ELeave ) CEmTubeServersViewContainer( aView );
	CleanupStack::PushL( self );
	self->ConstructL( aRect );
	return self;
	}

CEmTubeServersViewContainer::~CEmTubeServersViewContainer()
	{
	delete iListBox;
	}

CEmTubeServersViewContainer::CEmTubeServersViewContainer(CEmTubeServersView& aView) : iView(aView)
	{
	}

void CEmTubeServersViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidEmTubeApp;
    aContext.iContext = KContextServers;
    }

void CEmTubeServersViewContainer::SelectPluginL()
	{
	iAppUi->PluginManager().SelectPlugin( CurrentItemIndex() );
	iAppUi->PluginManager().SetDefaultPluginUid( iAppUi->PluginManager().Uid() );
	}

TInt CEmTubeServersViewContainer::CurrentItemIndex()
	{
	return iListBox->CurrentItemIndex();
	}

TBool CEmTubeServersViewContainer::IsCurrentItemMore()
	{
#ifdef ENABLE_MORE_ON_PLUGIN_LIST
	CDesCArray* list = static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
	if( CurrentItemIndex() == ( list->Count() - 1) )
		return ETrue;
#endif
	return EFalse;
	}

void CEmTubeServersViewContainer::HandleListBoxEventL( CEikListBox* /*aListBox*/, TListBoxEvent aListBoxEvent )
	{
	switch( aListBoxEvent )
		{
		case EEventEnterKeyPressed:
		case EEventItemClicked:
			{
			if( IsCurrentItemMore() )
				{
				RApaLsSession apaLsSession;
				const TUid KOSSBrowserUidValue = {0x10008d39};

				_LIT(KUrl, "http://www.blstream.com/gvh/plugins/");

				HBufC* param = HBufC::NewLC(KUrl().Length( ) + 2);
				param->Des().Copy( _L("4 ") );
				param->Des().Append( KUrl() );

				TUid id(KOSSBrowserUidValue);
				TApaTaskList taskList(CEikonEnv::Static()->WsSession());
				TApaTask task = taskList.FindApp(id);
				if(task.Exists())
					{
					task.BringToForeground();
					HBufC8* param8 = HBufC8::NewLC(param->Length());
					param8->Des().Append(*param);
					task.SendMessage(TUid::Uid(0), *param8); // UID not used
					CleanupStack::PopAndDestroy(param8);
					}
				else
					{
					if(!apaLsSession.Handle())
						{
						User::LeaveIfError(apaLsSession.Connect());
						}
					TThreadId thread;
					User::LeaveIfError(apaLsSession.StartDocument(*param, KOSSBrowserUidValue, thread));
					apaLsSession.Close();
					}
				CleanupStack::PopAndDestroy(param);
				}
			else
				{
				CAknSinglePopupMenuStyleListBox* plist = new(ELeave) CAknSinglePopupMenuStyleListBox;
				CleanupStack::PushL(plist);

			    CAknPopupList* popupList = CAknPopupList::NewL( plist, R_AVKON_SOFTKEYS_MENU_LIST, AknPopupLayouts::EMenuWindow);
			    CleanupStack::PushL(popupList);

			    plist->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
			    plist->CreateScrollBarFrameL(ETrue);
			    plist->ScrollBarFrame()->SetScrollBarVisibilityL(
			                               CEikScrollBarFrame::EOff,
			                               CEikScrollBarFrame::EAuto);

			    MDesCArray* itemList = plist->Model()->ItemTextArray();
			    CDesCArray* items = (CDesCArray*) itemList;

				HBufC* txt = StringLoader::LoadLC( R_POPUP_SELECT_SITE_TXT );
				items->AppendL( *txt );
				CleanupStack::PopAndDestroy( txt );

				CPluginInterface* plugin = iAppUi->PluginManager().Plugin( CurrentItemIndex() );
				if( plugin->Capabilities() & KCapsLogin )
					{
					HBufC* txt = StringLoader::LoadLC( R_POPUP_LOGIN_TXT );
					items->AppendL( *txt );
					CleanupStack::PopAndDestroy( txt );
					}

			    popupList->SetTitleL( KNullDesC() );
			    TInt popupOk = popupList->ExecuteLD();
			    if(popupOk)
			        {
				    switch( plist->CurrentItemIndex() )
				    	{
			    		case 0:
							iView.HandleCommandL( EMTVSetAsDefaultServerCommand );
			    		break;

			    		case 1:
							iView.HandleCommandL( EMTVLoginCommand );
			    		break;
					    }
			        }
			    CleanupStack::Pop( popupList );
			    CleanupStack::PopAndDestroy( plist );
				}
			}
		break;

		default:
		break;
		}
	}

void CEmTubeServersViewContainer::ConstructL( const TRect& aRect )
	{
	iAppUi = STATIC_CAST(CEmTubeAppUi*, CEikonEnv::Static()->EikAppUi());

	TUint32 selectedUid = iAppUi->PluginManager().Uid();
	iAppUi->PluginManager().LoadPluginsL();
	iAppUi->PluginManager().SelectPlugin( selectedUid );

	CreateWindowL();

//create list box
	iListBox = new ( ELeave ) CAknDoubleLargeStyleListBox;
	iListBox->SetContainerWindowL( *this );
	iListBox->ConstructL( this, EAknListBoxSelectionList );
	iListBox->CreateScrollBarFrameL( ETrue );
	iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
	iListBox->SetListBoxObserver( this );
	iListBox->Model()->SetOwnershipType( ELbmOwnsItemArray );
	iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );

	CFbsBitmap *bitmap, *mask;
	CArrayPtr<CGulIcon>* icons = new(ELeave) CAknIconArray(8);
	CleanupStack::PushL(icons);
	AknIconUtils::CreateIconL(bitmap, mask, KAvkonBitmapFileName, EMbmAvkonQgn_indi_marked_add, EMbmAvkonQgn_indi_marked_add_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));

	AknIconUtils::CreateIconL(bitmap, mask, KBitmapFileName, EMbmOpenvideohubEmpty, EMbmOpenvideohubEmpty_mask);
	icons->AppendL(CGulIcon::NewL(bitmap, mask));

	RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();
	for(TInt i=0;i<plugins.Count();i++)
		{
		plugins[i]->IconL( bitmap, mask );
		TSize iconSize;
		iconSize.iWidth = 32;
		iconSize.iHeight = 32;
		AknIconUtils::SetSize( bitmap, iconSize );
		icons->AppendL( CGulIcon::NewL( bitmap, mask ) );
		}

	CleanupStack::Pop(icons);
	iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );

	FillListL( selectedUid );

	SetRect(aRect);
	ActivateL();
	}

void CEmTubeServersViewContainer::SizeChanged()
	{
	if( iListBox )
		{
		iListBox->SetRect( Rect() );
		}
	}

void CEmTubeServersViewContainer::HandleResourceChange(TInt aType)
	{
	CCoeControl::HandleResourceChange(aType);
	SetRect(iView.ClientRect());
	DrawDeferred();
	}

void CEmTubeServersViewContainer::Draw( const TRect& /*aRect*/ ) const
	{
	}

TInt CEmTubeServersViewContainer::CountComponentControls() const
	{
	return 1;
	}

CCoeControl* CEmTubeServersViewContainer::ComponentControl(TInt aIndex) const
	{
	switch ( aIndex )
		{
		case 0:
			return iListBox;
		default:
			return NULL;
		}
	}

TKeyResponse CEmTubeServersViewContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
	if( iListBox )
		return iListBox->OfferKeyEventL( aKeyEvent, aType );

	return EKeyWasNotConsumed;
	}

void CEmTubeServersViewContainer::FillListL( TUint32 aUid )
	{
	TInt selected = CurrentItemIndex();

	RBuf text;
	CDesCArray* list = static_cast<CDesCArray*>( iListBox->Model()->ItemTextArray() );
	list->Reset();
	RPointerArray<CPluginInterface>& plugins = iAppUi->PluginManager().Plugins();

	for(TInt i=0;i<plugins.Count();i++)
		{
		TPtrC name = plugins[i]->Name();
		CleanupClosePushL( text );

		HBufC* txt;

		TUint32 uid = iAppUi->PluginManager().Uid( i );

		if( !(plugins[i]->Capabilities() & KCapsLogin) )
			{
			txt = StringLoader::LoadLC( R_PLUGIN_STATUS_LOGIN_NOT_SUPPORTED_TXT );
			}
		else
			{
			if( iAppUi->PluginManager().LoggedIn( uid ) )
				{
				txt = StringLoader::LoadLC( R_PLUGIN_STATUS_LOGGED_IN_TXT );
				}
			else
				{
				txt = StringLoader::LoadLC( R_PLUGIN_STATUS_NOT_LOGGED_IN_TXT);
				}
			}

		text.CreateL( name.Length() + 10 + txt->Length() );
		if( iAppUi->PluginManager().Uid( i ) == aUid )
			{
			text.Format( _L("%d\t%S\t%S\t0"), i + 2, &name, txt );
			}
		else
			{
			text.Format( _L("%d\t%S\t%S"), i + 2, &name, txt );
			}
		CleanupStack::PopAndDestroy( txt );
		list->AppendL( text );
		CleanupStack::PopAndDestroy( &text );
		}

#ifdef ENABLE_MORE_ON_PLUGIN_LIST
	CleanupClosePushL( text );
	HBufC* more = StringLoader::LoadLC( R_PLUGIN_LIST_MORE_TXT );
	text.CreateL( more->Length() + 10 );
	text.Format( _L("%d\t%S\t"), 1, more );
	CleanupStack::PopAndDestroy( more );
	list->AppendL( text );
	CleanupStack::PopAndDestroy( &text );
#endif

	iListBox->SetFocus( ETrue , EDrawNow );
	iListBox->HandleItemAdditionL();
	if( selected < 0 )
		selected = 0;
    iListBox->SetCurrentItemIndexAndDraw( selected );
	iListBox->DrawNow();
	}

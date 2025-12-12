#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "EGTextFormat.h"
#include "ExPortraits.h"
#include "ExCharacterPortraitWidget.h"
#include "EGRandom.h"
#include "ExUiSounds.h"
#include "ExKeyboardMenu.h"
#include "ExSaveMgr.h"
#include "ExToggleWidget.h"
#include "ExTextEditWidget.h"
#include "EGMenuStack.h"
#include "ExNewGameFlowMenu.h"

static const eg_loc_char SAVE_NAME_ALLOWED_CHARS[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789_";

class ExGameCreateMenu : public ExMenu , public IExDialogListener
{
	EG_CLASS_BODY( ExGameCreateMenu , ExMenu )

public:

	virtual void OnInit() override final 
	{
		Super::OnInit();

		SetIgnoreAltNav( true );

		ExTextEditWidget* NameText = EGCast<ExTextEditWidget>( GetWidget(eg_crc("NameText")) );
		if( NameText )
		{
			NameText->InitWidget( SAVE_NAME_ALLOWED_CHARS , EX_MAX_SAVE_NAME );
			eg_uint Slot = ExSaveMgr::Get().FindFreeSaveSlot( GetOwnerClient() );
			NameText->SetText( EGFormat( L"EXPLOR {0}" , Slot ).GetString() );
		}

		SetFocusedWidget( eg_crc("NameText") , 0 , false );

		EGUiButtonWidget* CreateButton = GetWidget<EGUiButtonWidget>( eg_crc("CreateButton") );
		if( CreateButton )
		{
			CreateButton->OnPressedDelegate.Bind( this , &ThisClass::OnCreateGameClicked );
		}

		DoReveal( 1 ); // Say there is one choice so the background appears the correct size.
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			ExNewGameFlowMenu* NewGameFlowMenu = GetOwnerMenuStack() ? GetOwnerMenuStack()->FindMenuByClass<ExNewGameFlowMenu>() : nullptr;
			if( NewGameFlowMenu )
			{
				NewGameFlowMenu->HandleCancelFromNewGame();
			}
			MenuStack_Pop();
			return true;
		} 
		return false;
	}

	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override final
	{
		switch_crc( ListenerParm )
		{
			case_crc("DoCreation"):
			{
				if( Choice == eg_crc("Yes") )
				{
					DoCreation();
				}
			} break;
		}
	}

	void DoCreation()
	{
		exPackedCreateGameInfo PackCreateInfo;
		zero( &PackCreateInfo );
		static_assert( sizeof(PackCreateInfo) == sizeof(egRemoteEvent::Parms) , "exPackedCreateGameInfo is not the right size" );
		PackCreateInfo.Slot = ExSaveMgr::Get().FindFreeSaveSlot( GetOwnerClient() );
		ex_save_name SaveName = { '\0' };
		ExTextEditWidget* NameText = EGCast<ExTextEditWidget>( GetWidget( eg_crc( "NameText" ) ) );
		if( NameText )
		{
			NameText->GetText( SaveName , countof(SaveName) );
		}
		ExSaveMgr::Get().CreateSave( GetOwnerClient() , PackCreateInfo, SaveName ); // This will pop the menus and continue new game flow.
	}

	void OnCreateGameClicked( const egUIWidgetEventInfo& Info )
	{
		unused( Info );
		
		eg_bool bCanCreate = true;

		ex_save_name SaveName = { '\0' };
		ExTextEditWidget* NameText = EGCast<ExTextEditWidget>( GetWidget( eg_crc( "NameText" ) ) );
		if( NameText )
		{
			NameText->GetText( SaveName, countof( SaveName ) );
		}
		if( bCanCreate && SaveName[0] == '\0' )
		{
			ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc("CreateMsg_NoSaveName","You must name your save game to continue.") ) );
			bCanCreate = false;
		}

		if( bCanCreate )
		{
			DoCreation();
		}
	}

	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) override
	{
		Super::OnFocusedWidgetChanged( NewFocusedWidget , OldFocusedWidget );
		UpdateHelpText();
	}

	void UpdateHelpText()
	{
		EGUiWidget* HelpText = GetWidget( eg_crc("HelpText") );
		EGUiWidget* FocusedWidget = GetFocusedWidget();
		eg_bool bShowKeyboardHelp = false;
		if( FocusedWidget && HelpText )
		{
			if( FocusedWidget->GetId() == eg_crc("NameText") )
			{
				HelpText->SetText( CT_Clear , eg_loc_text( eg_loc("CreateHelp_SaveName","Name your game to refer to it later.") ) ); 
				bShowKeyboardHelp = true;
			}
			else if( FocusedWidget->GetId() == eg_crc("CreateButton") )
			{
				HelpText->SetText( CT_Clear , eg_loc_text( eg_loc("CreateHelp_CreateButton","Begin your EXPLORation of a New World.") ) ); 
			}
		}

		ClearHints();
		if( bShowKeyboardHelp )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("GameCreateMenuEditText","Edit Text")) );
		}
		AddHint( CMDA_MENU_BACK , eg_loc_text( eg_loc("GameCreateMenuCancel","Cancel") ) );
	}
};

EG_CLASS_DECL( ExGameCreateMenu )

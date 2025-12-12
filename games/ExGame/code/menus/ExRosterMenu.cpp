// (c) 2020 Beem Media. All Rights Reserved.

#include "ExRosterMenu.h"
#include "ExCharacterPortraitWidget.h"
#include "ExRoster.h"
#include "EGTextFormat.h"
#include "ExChoiceWidgetMenu.h"
#include "ExCharacterMenu.h"
#include "ExCharacterCreateMenu.h"
#include "ExHintBarWidget.h"
#include "EGUiGridWidget.h"
#include "EGMenuStack.h"

EG_CLASS_DECL( ExRosterMenu )

void ExRosterMenu::OpenRosterMenu( EGClient* Client , eg_bool bExchangeOnly )
{
	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	if( MenuStack )
	{
		MenuStack->Push( bExchangeOnly ? eg_crc("RosterMenu_ExchangeOnly") : eg_crc("RosterMenu") );
	}
}

void ExRosterMenu::OnRosterChanged()
{
	Refresh();
}

void ExRosterMenu::SetMenuState( ex_menu_s NewState , eg_bool bAllowAudio /*= true */ )
{
	ClearHints();

	ex_menu_s OldState = m_MenuState;
	m_MenuState = NewState;

	eg_bool bRosterActive = false;
	eg_bool bPartyActive = false;
	EGUiWidget* WantedFocusedWidget = nullptr;
	eg_uint WantedFocusedIndex = 0;

	switch( m_MenuState )
	{
	case ex_menu_s::BROWSING:
	{
		EGUiWidget* WantedWidget = m_CharacterRoster;
		eg_uint WantedSlot = m_RosterMemberInQuestion ? GetRosterIndexOfCharacter( m_RosterMemberInQuestion ) : 0;

		bRosterActive = true;
		bPartyActive = false;
		WantedFocusedWidget = WantedWidget;
		WantedFocusedIndex = WantedSlot;

		AddHint( CMDA_MENU_PRIMARY , eg_loc_text( eg_loc( "RosterMenuCrewMemberOptions" , "Crew Member Options" ) ) );
		AddHint( CMDA_MENU_BACK , eg_loc_text( eg_crc( "BackOutText" ) ) );
	} break;
	case ex_menu_s::ADD_TO_PARTY:
	{
		if( m_RosterMemberInQuestion )
		{
			AddHint( CMDA_MENU_PRIMARY , EGFormat( eg_loc( "AddToPartyHelp" , "Add {0:NAME}" ) , *m_RosterMemberInQuestion ) );
			AddHint( CMDA_MENU_BACK , eg_loc_text( eg_crc( "Cancel" ) ) );
		}
		// We're adding to party, so choose the first empty slot.
		eg_uint Slot = 0;
		for( eg_uint i = 0; i < ExRoster::PARTY_SIZE; i++ )
		{
			if( GetGame()->GetPartyMemberByIndex( i ) == nullptr )
			{
				Slot = i;
				break;
			}
		}

		bRosterActive = false;
		bPartyActive = true;
		WantedFocusedWidget = m_CharacterPortraits;
		WantedFocusedIndex = Slot;
	} break;
	case ex_menu_s::EXCHANGE_ORDER:
	{
		if( m_RosterMemberInQuestion )
		{
			AddHint( CMDA_MENU_PRIMARY , EGFormat( eg_loc( "ExchangePartyHelp" , "Exchange {0:NAME}" ) , *m_RosterMemberInQuestion ) );
			AddHint( CMDA_MENU_BACK , eg_loc_text( eg_crc( "Cancel" ) ) );
		}
		// We're exchanging party, it feels best to leave the slot the same.
		eg_uint Slot = GetPartyIndexOfCharacter( m_RosterMemberInQuestion );

		bRosterActive = false;
		bPartyActive = true;
		WantedFocusedWidget = m_CharacterPortraits;
		WantedFocusedIndex = Slot;
	} break;
	}

	if( m_CharacterRoster )
	{
		m_CharacterRoster->SetEnabled( bRosterActive );
	}

	if( m_CharacterPortraits )
	{
		m_CharacterPortraits->SetEnabled( bPartyActive );
	}

	SetFocusedWidget( WantedFocusedWidget , WantedFocusedIndex , bAllowAudio );
}

void ExRosterMenu::Refresh()
{
	if( m_CharacterRoster )
	{
		m_CharacterRoster->RefreshGridWidget( ExRoster::ROSTER_SIZE );
	}
	if( m_CharacterPortraits )
	{
		m_CharacterPortraits->RefreshGridWidget( countof( m_Portraits ) );
	}
}

void ExRosterMenu::OnInit()
{
	Super::OnInit();

	m_bIsExchangeOnly = GetLayout() && GetLayout()->m_Id.EqualsI( "RosterMenu_ExchangeOnly" );

	GetGame()->OnClientRosterChangedDelegate.AddUnique( this , &ThisClass::OnRosterChanged );

	m_CharacterRoster = GetWidget<EGUiGridWidget>( eg_crc( "CharacterRoster" ) );
	m_CharacterPortraits = GetWidget<EGUiGridWidget>( eg_crc( "CharacterPortraits" ) );

	if( m_CharacterRoster )
	{
		m_CharacterRoster->OnItemChangedDelegate.Bind( this , &ThisClass::OnRosterMemberCellChanged );
		m_CharacterRoster->OnItemPressedDelegate.Bind( this , &ThisClass::OnRosterMemberClicked );
	}

	if( m_CharacterPortraits )
	{
		m_CharacterPortraits->OnItemChangedDelegate.Bind( this , &ThisClass::OnPartyMemberCellChanged );
		m_CharacterPortraits->OnItemPressedDelegate.Bind( this , &ThisClass::OnPartyMemberClicked );
	}

	Refresh();
	if( m_bIsExchangeOnly )
	{
		m_RosterMemberInQuestion = GetGame() ? GetGame()->GetSelectedPartyMember() : nullptr;
		assert( nullptr != m_RosterMemberInQuestion );
		PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED ); 
		SetMenuState( ex_menu_s::EXCHANGE_ORDER , false );
	}
	else
	{
		DoReveal();
		SetMenuState( ex_menu_s::BROWSING , false );
		HideHUD();
	}
}

void ExRosterMenu::OnDeinit()
{
	GetGame()->OnClientRosterChangedDelegate.RemoveAll( this );
	Super::OnDeinit();
	if( !m_bIsExchangeOnly )
	{
		ShowHUD( true );
	}
}

eg_bool ExRosterMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		if( m_MenuState == ex_menu_s::BROWSING || m_bIsExchangeOnly )
		{
			if( m_bIsExchangeOnly )
			{
				PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
			}
			LeaveMenu();
		}
		else if( m_MenuState == ex_menu_s::ADD_TO_PARTY || m_MenuState == ex_menu_s::EXCHANGE_ORDER )
		{
			SetMenuState( ex_menu_s::BROWSING );
		}
		return true;
	}

	return false;
}

void ExRosterMenu::LeaveMenu()
{
	const ExGame* Game = GetGame();
	eg_bool bHasAtLeastOnePartyMember = false;
	for( eg_uint i = 0; i < ExRoster::PARTY_SIZE; i++ )
	{
		if( nullptr != Game->GetPartyMemberByIndex( i ) && Game->GetPartyMemberByIndex( i )->CanFight() )
		{
			bHasAtLeastOnePartyMember = true;
		}
	}

	if( bHasAtLeastOnePartyMember )
	{
		MenuStack_Pop();
	}
	else
	{
		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( eg_loc( "RosterMenu_NoParty" , "You must have at least one living party member to go adventuring." ) ) );
	}
}

eg_bool ExRosterMenu::IsCharacterInParty( const ExGame* Game , const ExFighter* Character ) const
{
	for( eg_uint i = 0; i < ExRoster::PARTY_SIZE; i++ )
	{
		if( Character == Game->GetPartyMemberByIndex( i ) )
		{
			return true;
		}
	}
	return false;
}

void ExRosterMenu::PushContextMenu( const ExFighter* RosterMember , eg_real x , eg_real y , eg_int PortraitIndex )
{
	ExChoiceWidgetMenu::exConfig ChoiceConfig;
	ChoiceConfig.bCanPop = true;
	ChoiceConfig.InitialSelection = 0;
	ChoiceConfig.Pose = eg_transform::BuildTranslation( eg_vec3( x , y , 0.f ) );
	m_RosterMemberInQuestion = RosterMember;

	for( eg_uint i = 0; i < m_CurrentChoices.LenAs<eg_uint>(); i++ )
	{
		ChoiceConfig.Choices.Append( GetChoiceText( i ) );
	}

	if( PortraitIndex >= 0 )
	{
		ChoiceConfig.SetPoseForPortrait( PortraitIndex , false );
	}

	ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( this , ChoiceConfig );
	if( ChoiceMenu )
	{
		ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnChoiceMade );
	}
}

void ExRosterMenu::OnRosterMemberCellChanged( const egUIWidgetEventInfo& ItemInfo )
{
	ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

	const ExGame* Game = GetGame();
	if( EG_IsBetween<eg_size_t>( ItemInfo.GridIndex , 0 , ExRoster::ROSTER_SIZE - 1 ) && Game )
	{
		const ExFighter* RosterMember = Game->GetRosterCharacterByIndex( ItemInfo.GridIndex );
		if( RosterMember )
		{
			eg_loc_text NameText;
			eg_string_crc CheckedEvent = CT_Clear;
			if( RosterMember->IsCreated() )
			{
				NameText = EGFormat( eg_loc("RosterNotInPartyString","{0} - LVL {1} {2}") , RosterMember->LocName , RosterMember->GetAttrValue( ex_attr_t::LVL ) , ExCore_GetClassName( RosterMember->GetClass() ) );
				CheckedEvent = IsCharacterInParty( Game , RosterMember ) ? eg_crc("SetChecked") : eg_crc("SetUnchecked");
			}
			else
			{
				NameText = eg_loc_text( eg_loc( "EmptyRoster" , "-Available-" ) );
				CheckedEvent = eg_crc("SetEmpty");
			}
			ItemInfo.Widget->SetText( eg_crc( "Text" ) , NameText );
			if( CheckedEvent.IsNotNull() )
			{
				ItemInfo.Widget->RunEvent( CheckedEvent );
			}
		}
		else
		{
			assert( false );
		}
	}
}

void ExRosterMenu::OnPartyMemberCellChanged( const egUIWidgetEventInfo& ItemInfo )
{
	ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

	if( EG_IsBetween<eg_size_t>( ItemInfo.GridIndex , 0 , ExRoster::PARTY_SIZE - 1 ) )
	{
		m_Portraits[ItemInfo.GridIndex] = EGCast<ExCharacterPortraitWidget2>( ItemInfo.Widget );
		if( m_Portraits[ItemInfo.GridIndex] )
		{
			m_Portraits[ItemInfo.GridIndex]->InitPortrait( GetGame()->GetPartyMemberByIndex( ItemInfo.GridIndex ) , ItemInfo.GridIndex , nullptr );
		}
		if( ItemInfo.bNewSelection && ItemInfo.bSelected )
		{
			ItemInfo.Widget->RunEvent( eg_crc( "PlayHighlightSound" ) );
		}
	}
}

void ExRosterMenu::OnRosterMemberClicked( const egUIWidgetEventInfo& Info )
{
	assert( !m_bIsExchangeOnly )
	if( m_bIsExchangeOnly )
	{
		return;
	}

	m_CurrentChoices.Clear();

	const ExGame* Game = GetGame();
	const ExFighter* RosterMember = Game->GetRosterCharacterByIndex( Info.GridIndex );
	if( RosterMember )
	{
		m_ChoiceTarget = Info.GridIndex;

		// The following are dependent on the content (should probably query the content but oh well...)
		static const eg_vec2 RosterItemWidgetDims( 100.f , 10.f );
		static const eg_vec2 BaseOffset( -18.f , 50.f );

		eg_vec2 Position( BaseOffset.x + (static_cast<eg_real>(Info.GridIndex % 2) - .5f) * RosterItemWidgetDims.x , BaseOffset.y - ((Info.GridIndex / 2) * RosterItemWidgetDims.y) );

		eg_bool bInParty = IsCharacterInParty( Game , RosterMember );
		if( RosterMember->IsCreated() )
		{
			if( !bInParty )
			{
				m_CurrentChoices.Append( ex_choice_t::ADD_TO_PARTY );
			}
			m_CurrentChoices.Append( ex_choice_t::VIEW_CHARACTER );
			if( bInParty )
			{
				m_CurrentChoices.Append( ex_choice_t::REMOVE_FROM_PARTY );
				m_CurrentChoices.Append( ex_choice_t::EXCHANGE_ORDER );
			}
			m_CurrentChoices.Append( ex_choice_t::DELETE_CHARACTER );
			m_CurrentChoices.Append( ex_choice_t::CANCEL );

			PushContextMenu( RosterMember , Position.x , Position.y , -1 );
		}
		else
		{
			m_CurrentChoices.Append( ex_choice_t::CREATE_CHARACTER );
			m_CurrentChoices.Append( ex_choice_t::CANCEL );
			PushContextMenu( nullptr , Position.x , Position.y , -1 );
		}
	}
}

void ExRosterMenu::OnPartyMemberClicked( const egUIWidgetEventInfo& Info )
{
	if( m_MenuState == ex_menu_s::BROWSING )
	{
		assert( false ); // Party members should not be clickable while browsing.
	}
	else if( m_MenuState == ex_menu_s::EXCHANGE_ORDER )
	{
		if( m_bIsExchangeOnly )
		{
			MenuStack_Pop();
		}
		else
		{
			SetMenuState( ex_menu_s::BROWSING );
		}
		if( GetGame() && Info.GridIndex != GetGame()->GetSelectedPartyMemberIndex() )
		{
			ExchangePartyMemberWithSlot( m_RosterMemberInQuestion , Info.GridIndex );
			RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , Info.GridIndex ) );
		}
	}
	else if( m_MenuState == ex_menu_s::ADD_TO_PARTY )
	{
		SetMenuState( ex_menu_s::BROWSING );
		AddPartyMemberToSlot( m_RosterMemberInQuestion , Info.GridIndex );
		RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , Info.GridIndex ) );
	}
}

eg_uint ExRosterMenu::GetPartyIndexOfCharacter( const ExFighter* Character )
{
	eg_uint Out = ExRoster::PARTY_SIZE;

	const ExGame* Game = GetGame();

	for( eg_uint i = 0; i < ExRoster::PARTY_SIZE; i++ )
	{
		if( Character == Game->GetPartyMemberByIndex( i ) )
		{
			Out = i;
			break;
		}
	}

	return Out;
}

eg_uint ExRosterMenu::GetRosterIndexOfCharacter( const ExFighter* Character )
{
	eg_uint Out = ExRoster::ROSTER_SIZE;

	const ExGame* Game = GetGame();

	for( eg_uint i = 0; i < ExRoster::ROSTER_SIZE; i++ )
	{
		if( Character == Game->GetRosterCharacterByIndex( i ) )
		{
			Out = i;
			break;
		}
	}

	return Out;
}

void ExRosterMenu::ExchangePartyMemberWithSlot( const ExFighter* Character , eg_uint Slot )
{
	eg_uint CharacterSlot = GetPartyIndexOfCharacter( Character );
	RunServerEvent( egRemoteEvent( eg_crc( "RosterMenuExchangePartyMembers" ) , exDoubleIndex( CharacterSlot , Slot ).ToEventParm() ) );
	ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ExchangePartyMember );
}

void ExRosterMenu::AddPartyMemberToSlot( const ExFighter* Character , eg_uint Slot )
{
	eg_uint CharacterSlot = GetRosterIndexOfCharacter( Character );
	RunServerEvent( egRemoteEvent( eg_crc( "RosterMenuAddPartyMemberToSlot" ) , exDoubleIndex( CharacterSlot , Slot ).ToEventParm() ) );
	ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ExchangePartyMember );
}

void ExRosterMenu::RemovePartyMember( const ExFighter* Character )
{
	if( nullptr == Character )
	{
		assert( false ); // No one to remove.
		return;
	}

	eg_uint RemoveIndex = GetPartyIndexOfCharacter( Character );

	if( RemoveIndex < ExRoster::PARTY_SIZE )
	{
		RunServerEvent( egRemoteEvent( eg_crc( "RosterMenuRemovePartyMember" ) , RemoveIndex ) );
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::ExchangePartyMember );
	}
	else
	{
		assert( false ); // This character was not in the party.
	}
}

void ExRosterMenu::OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	ex_choice_t Type = m_CurrentChoices.IsValidIndex( ChoiceIndex ) ? m_CurrentChoices[ChoiceIndex] : ex_choice_t::CANCEL;

	MenuStack_Pop();

	switch( Type )
	{
	case ex_choice_t::CANCEL:
	{
	} break;
	case ex_choice_t::CREATE_CHARACTER:
	{
		ExCharacterCreateMenu_OpenCreateMenu( GetOwnerClient() , m_ChoiceTarget );
	} break;
	case ex_choice_t::VIEW_CHARACTER:
	{
		assert( m_RosterMemberInQuestion );
		if( m_RosterMemberInQuestion )
		{
			ExCharacterMenu_ViewFighter( GetOwnerClient() , *m_RosterMemberInQuestion , true , GetGame()->GetRoster().GetPartyIndex( m_RosterMemberInQuestion ) );
		}
	} break;
	case ex_choice_t::REMOVE_FROM_PARTY:
	{
		RemovePartyMember( m_RosterMemberInQuestion );
	} break;
	case ex_choice_t::EXCHANGE_ORDER:
	{
		SetMenuState( ex_menu_s::EXCHANGE_ORDER );
	} break;
	case ex_choice_t::ADD_TO_PARTY:
	{
		SetMenuState( ex_menu_s::ADD_TO_PARTY );
	} break;
	case ex_choice_t::DELETE_CHARACTER:
	{
		if( m_RosterMemberInQuestion )
		{
			exYesNoDialogParms Parms( EGFormat( eg_loc( "DeleteCharacterConfirm" , "Are you sure you want to delete {0}? This cannot be undone." ) , m_RosterMemberInQuestion->LocName ) , true );
			Parms.DlgListener = this;
			Parms.DlgListenerParm = eg_crc( "DeleteCharacterConfirm" );
			ExDialogMenu_PushDialogMenu( GetOwnerClient() , Parms );
		}
	} break;
	}
}

eg_loc_text ExRosterMenu::GetChoiceText( eg_uint Index ) const
{
	eg_loc_text Out = CT_Clear;

	ex_choice_t Type = m_CurrentChoices.IsValidIndex( Index ) ? m_CurrentChoices[Index] : ex_choice_t::CANCEL;

	switch( Type )
	{
	case ex_choice_t::CANCEL: Out = eg_loc_text( eg_loc( "RosterMenuCancel" , "Cancel" ) ); break;
	case ex_choice_t::CREATE_CHARACTER: Out = eg_loc_text( eg_loc( "RosterMenuCreateCharacter" , "Create Character" ) ); break;
	case ex_choice_t::REMOVE_FROM_PARTY: Out = eg_loc_text( eg_loc( "RosterMenuRemoveFromParty" , "Remove From Party" ) ); break;
	case ex_choice_t::VIEW_CHARACTER: Out = eg_loc_text( eg_loc( "RosterMenuViewCharacter" , "View Character" ) ); break;
	case ex_choice_t::ADD_TO_PARTY: Out = eg_loc_text( eg_loc( "RosterMenuAddToParty" , "Add To Party" ) ); break;
	case ex_choice_t::EXCHANGE_ORDER: Out = eg_loc_text( eg_loc( "RosterMenuExchange" , "Exchange" ) ); break;
	case ex_choice_t::DELETE_CHARACTER: Out = eg_loc_text( eg_loc( "RosterMenuDeleteCharacter" , "Delete Character" ) ); break;
	}

	return Out;
}

void ExRosterMenu::OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice )
{
	if( ListenerParm == eg_crc( "DeleteCharacterConfirm" ) )
	{
		if( Choice == eg_crc( "Yes" ) )
		{
			GetOwnerClient()->SDK_RunServerEvent( egRemoteEvent( eg_crc( "RosterMenuDeleteRosterMember" ) , GetRosterIndexOfCharacter( m_RosterMemberInQuestion ) ) );
		}
	}
}

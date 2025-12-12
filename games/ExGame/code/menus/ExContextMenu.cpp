// (c) 2016 Beem Media

#include "ExMenu.h"
#include "ExChoiceWidgetMenu.h"
#include "ExDialogMenu.h"
#include "ExCharacterMenu.h"
#include "ExGame.h"
#include "EGMenuStack.h"
#include "ExDoor.h"
#include "ExCastSpellInWorldMenu.h"
#include "ExRosterMenu.h"
#include "ExProfileData.h"

enum class ex_context_action
{
	NONE,
	QUEST_LOG,
	VIEW_CHARACTER,
	INVENTORY,
	VIEW_MAP,
	CAST_SPELL,
	ATTUNE_SKILLS,
	SUBMENU_REST,
	REST_UNTILHEALED,
	REST_UNTILMORNING,
	UNLOCK_DOOR,
	EXCHANGE_PARTY,
};

struct exContextChoice
{
	eg_string_crc     Name;
	ex_context_action Action;
};

static exContextChoice ExContextMenu_Choices_All[] =
{
	{ eg_loc("ContextMenuUnlockDoorText","Unlock Door")       , ex_context_action::UNLOCK_DOOR },
	{ eg_loc("ContextMenuRestText","Rest")                    , ex_context_action::SUBMENU_REST },
	{ eg_loc("ContextMenuInventoryText","Inventory")          , ex_context_action::INVENTORY },
	{ eg_loc("ContextMenuViewMapText","View Map")             , ex_context_action::VIEW_MAP },
	{ eg_loc("ContextMenuCastSpellText","Cast Spell")         , ex_context_action::CAST_SPELL },
	{ eg_loc("ContextMenuViewCharacterText","View Character") , ex_context_action::VIEW_CHARACTER },
	// { eg_loc("ContextMenuAttuneSkillsText","Attune Skills")   , ex_context_action::ATTUNE_SKILLS },
	{ eg_loc("ContextMenuGameStatusText","View Quests/Status") , ex_context_action::QUEST_LOG },
	{ eg_loc("ContextMenuExchangeText","Exchange")            , ex_context_action::EXCHANGE_PARTY },
	{ eg_loc("ContextMenuBackText","Back" )                   , ex_context_action::NONE },
};

static exContextChoice ExContextMenu_Chocies_Rest[] =
{
	{ eg_loc("ContextMenuRestUntilHealed","Rest Until Healed (8 Hours)") , ex_context_action::REST_UNTILHEALED },
	{ eg_loc("ContextMenuRestUntilMorning","Rest Until Morning (6AM)") , ex_context_action::REST_UNTILMORNING },
};

class ExContextMenu : public ExMenu
{
	EG_CLASS_BODY( ExContextMenu , ExMenu )

private:

	struct exChoiceMenu
	{
		EGWeakPtr<ExChoiceWidgetMenu> Submenu;
		EGArray<exContextChoice>      Choices;
	};

private:

	EGArray<exChoiceMenu> m_Submenus;
	eg_bool               m_bUseRememberedSelection = true;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		eg_bool bUnlockDoorAllowed = false;

		ExEnt* TargetedEnt = GetGame()->GetEnt( GetGame()->GetPlayerTargetedEnt() );
		ExDoor* TargetedEntAsDoor = EGCast<ExDoor>(TargetedEnt);

		if( TargetedEntAsDoor )
		{
			m_bUseRememberedSelection = false;
		}

		EGArray<exContextChoice> Choices;
		for( const exContextChoice& Choice : ExContextMenu_Choices_All )
		{
			if( Choice.Action == ex_context_action::UNLOCK_DOOR )
			{
				if( nullptr == TargetedEntAsDoor || !TargetedEntAsDoor->IsLocked() )
				{
					continue;
				}
			}
			if( Choice.Action == ex_context_action::CAST_SPELL )
			{
				if( !ExCastSpellInWorldMenu::CanCastAnyInWorldSpells( GetGame() ) )
				{
					continue;
				}
			}
			if( Choice.Action == ex_context_action::EXCHANGE_PARTY )
			{
				eg_bool bPartySelected = GetGame() && GetGame()->GetSelectedPartyMember();
				if( !bPartySelected )
				{
					continue;
				}
			}
			Choices.Append( Choice );
		}
		PushSubmenu( Choices.GetArray() , Choices.Len() );
	}

	virtual void OnActivate() override final
	{
		Super::OnActivate();

		MenuStack_Pop();
	}

	void ClearSubmenus()
	{
		GetOwnerMenuStack()->PopTo( this );
		m_Submenus.Clear();
	}

	ex_context_action GetChoiceMade( const ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex ) const
	{
		for( const exChoiceMenu& Submenu : m_Submenus )
		{
			if( Submenu.Submenu.GetObject() == SourceMenu )
			{
				assert( Submenu.Choices.IsValidIndex(ChoiceIndex) );
				return Submenu.Choices.IsValidIndex( ChoiceIndex ) ? Submenu.Choices[ChoiceIndex].Action : ex_context_action::NONE;
			}
		}

		return ex_context_action::NONE;
	}

	void ConfirmRestWhenStarving( eg_string_crc ListenerParm,eg_string_crc Choice )
	{
		if( ListenerParm == eg_crc("RestWhileStarving") && Choice == eg_crc("Yes") )
		{
			ClearSubmenus();
			RunServerEvent( egRemoteEvent( eg_crc("Rest") , static_cast<eg_uint>(ex_rest_t::UntilHealed) ) );
		}
	}

	void OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
	{
		ex_context_action ChoiceMade = GetChoiceMade( SourceMenu , ChoiceIndex );
		ExGame* Game = GetGame();

		switch( ChoiceMade )
		{
			case ex_context_action::NONE:
				ClearSubmenus();
				break;
			case ex_context_action::QUEST_LOG:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("QuestLogMenu") );
				break;
			case ex_context_action::CAST_SPELL:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("CastSpellInWorldMenu") );
				break;
			case ex_context_action::SUBMENU_REST:
			{
				PushSubmenu( ExContextMenu_Chocies_Rest , countof( ExContextMenu_Chocies_Rest) );
			} break;
			case ex_context_action::REST_UNTILMORNING:
			case ex_context_action::REST_UNTILHEALED:
			{
				ex_rest_t RestType = ChoiceMade == ex_context_action::REST_UNTILHEALED ? ex_rest_t::UntilHealed  : ex_rest_t::UntilMorning;
				const eg_bool bPartyIsStarving = GetGame() && GetGame()->GetGameVar( eg_crc("PartyFood") ).as_int() <= 0;
				if( ChoiceMade == ex_context_action::REST_UNTILHEALED && bPartyIsStarving )
				{
					exYesNoDialogParms DlgParms( eg_loc_text(eg_loc("PartyStarvingDialogText","|SC(EX_WARN)|Notice|RC()|: The party will not recover |SC(EX_ATTR)|HP|RC()| or |SC(EX_ATTR)|MP|RC()| because they have no food. Purchase food from the Inn in order to recover. Do you wish to rest anyway?")) , true );
					DlgParms.DlgListenerParm = eg_crc("RestWhileStarving");
					DlgParms.DelegateCb.Bind( this , &ThisClass::ConfirmRestWhenStarving );
					ExDialogMenu_PushDialogMenu( GetClientOwner() , DlgParms );
				}
				else
				{
					ClearSubmenus();
					RunServerEvent( egRemoteEvent( eg_crc("Rest") , static_cast<eg_uint>(RestType) ) );
				}
			} break;
			case ex_context_action::VIEW_CHARACTER:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("CharacterMenu") );
				break;
			case ex_context_action::INVENTORY:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("InventoryMenu") );
				break;
			case ex_context_action::VIEW_MAP:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("MapMenu") );
				break;
			case ex_context_action::ATTUNE_SKILLS:
				ClearSubmenus();
				MenuStack_PushTo( eg_crc("AttuneSkillsMenu") );
				break;
			case ex_context_action::UNLOCK_DOOR:
				ClearSubmenus();
				GetGame()->UnlockTargetedDoor();
				break;
			case ex_context_action::EXCHANGE_PARTY:
			{
				ClearSubmenus();
				ExRosterMenu::OpenRosterMenu( GetClientOwner() , true );
			} break;
		}
	}

	void OnSelectionChanged( ExChoiceWidgetMenu* SourceMenu , eg_uint NewSelection )
	{
		if( m_bUseRememberedSelection && m_Submenus.IsValidIndex(0) && SourceMenu == m_Submenus[0].Submenu.GetObject() )
		{
			ExProfileData_SetLastContextMenuChoice( NewSelection );
		}
	}

	void OnChoiceAborted( ExChoiceWidgetMenu* SourceMenu )
	{
		if( m_Submenus.HasItems() && m_Submenus.Top().Submenu.GetObject() == SourceMenu )
		{
			m_Submenus.Pop();
		}
		else
		{
			assert( false );
		}
	}

	eg_bool IsChoiceAllowedInGameMode( ex_context_action ContextAction ) const
	{
		unused( ContextAction );

		return true;
	};

	void PushSubmenu( const exContextChoice* Choices , eg_size_t NumChoices )
	{
		eg_bool bFirstContextMenu = m_Submenus.IsEmpty();

		exChoiceMenu NewChoiceMenu;
		for( eg_size_t i=0; i<NumChoices; i++ )
		{
			if( IsChoiceAllowedInGameMode( Choices[i].Action ) )
			{
				NewChoiceMenu.Choices.Append( Choices[i] );
			}
		}

		ExChoiceWidgetMenu::exConfig Config;

		for( exContextChoice& Choice : NewChoiceMenu.Choices )
		{
			Config.Choices.Append( eg_loc_text(Choice.Name) );
		}

		if( bFirstContextMenu )
		{
			// Decide where the menu should appear
			{
				eg_uint PartyIndex = GetGame()->GetSelectedPartyMemberIndex();
				Config.SetPoseForPortrait( PartyIndex , false );
			}

			if( m_bUseRememberedSelection )
			{
				Config.InitialSelection = ExProfileData_GetLastContextMenuChoice();
				if( !Config.Choices.IsValidIndex(Config.InitialSelection) )
				{
					Config.InitialSelection = 0;
				}
			}
		}
		else
		{
			Config.bPushCascaded = true;
		}

		Config.bCanPop = true;


		ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( this , Config );
		if( ChoiceMenu )
		{
			NewChoiceMenu.Submenu = ChoiceMenu;
			m_Submenus.Push( NewChoiceMenu );
			ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnChoiceMade );
			ChoiceMenu->ChoiceSelectedDelegate.Bind( this , &ThisClass::OnSelectionChanged );
			ChoiceMenu->ChoiceAbortedDelegate.Bind( this , &ThisClass::OnChoiceAborted );
		}
	}
};

EG_CLASS_DECL( ExContextMenu )

// (c) 2016 Beem Media

#include "ExCombatMenu.h"
#include "ExDialogMenu.h"
#include "ExChoiceWidgetMenu.h"
#include "ExCombat.h"
#include "ExCombatMenu_SubMenus.h"
#include "EGTextFormat.h"
#include "ExCharacterPortraitWidget.h"
#include "EGUiImageWidget.h"
#include "ExBeastiary.h"
#include "ExCombatMenuOpponentsWidget.h"
#include "ExCombatPartyWidget.h"
#include "ExTextRevealWidget.h"
#include "ExConsts.h"
#include "EGSoundScape.h"
#include "EGAudio.h"
#include "EGUiGridWidget.h"
#include "ExGameSettings.h"
#include "ExItemCardWidget.h"
#include "ExWorldRenderPipe.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExCombatMenu )

static EGSettingsBool ExCombat_DebugShowCombatOrder( "ExCombat.DebugShowCombatOrder" , eg_loc("SettingsDebugShowCombatOrder","DEBUG: Show Combat Order") , false , EGS_F_USER_SAVED|EGS_F_DEBUG_EDITABLE );

static const eg_real EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS = 1.f;
static const eg_real EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS_FIRST_ATTACK = 2.f;

ExCombatMenu::ExCombatMenu() : ExMenu()
{
	m_BaseChoiceCb = EGNewObject<ExCombatMenu_CombatantAction>();
	if( m_BaseChoiceCb )
	{
		m_BaseChoiceCb->Init( this );
	}
}

ExCombatMenu::~ExCombatMenu()
{
	EGDeleteObject( m_BaseChoiceCb );
}

void ExCombatMenu::Refresh()
{
	if( m_Combat )
	{
		ExCombatantList RawTeamAList;
		m_Combat->GetCombatantList( 0, ExCombat::F_ACTIVE | ExCombat::F_DOWN | ExCombat::F_OUTOFCOMBAT, RawTeamAList );
		m_Combat->GetCombatantList( 1, ExCombat::F_ACTIVE , m_TeamBList );
		ExCombatantList TeamBOutOfCombatList;
		m_Combat->GetCombatantList( 1 , ExCombat::F_OUTOFCOMBAT , TeamBOutOfCombatList );
		m_Combat->GetCombatOrderList( m_CombatOrderList );

		m_TeamAList.Resize( ExRoster::PARTY_SIZE );
		for( eg_size_t i = 0; i < m_TeamAList.Len(); i++ )
		{
			m_TeamAList[i] = nullptr;
		}

		for( ExCombatant* Itr : RawTeamAList )
		{
			if( Itr && m_TeamAList.IsValidIndex( Itr->GetServerPartyIndex() ) )
			{
				m_TeamAList[Itr->GetServerPartyIndex()] = Itr;
			}
			else
			{
				assert( false ); // A party member was out of range!
			}
		}

		if( m_OpponentsWidget )
		{
			m_OpponentsWidget->SetCombatantList( m_Combat->GetTeamByIndex( 1 ) , m_TeamBList );
		}

		if( m_TeamA )
		{
			m_TeamA->SetCombatantList( m_Combat->GetTeamByIndex( 0 ) , m_TeamAList );
		}

		if( m_AdditionalMonstersText )
		{
			m_AdditionalMonstersText->SetVisible( TeamBOutOfCombatList.Len() > 0 );
			if( TeamBOutOfCombatList.Len() > 0 )
			{
				eg_loc_text AdditionalText = EGFormat( eg_loc("WaitingMonstersText","Additional Monsters: {0}") , TeamBOutOfCombatList.LenAs<eg_int>() );
				m_AdditionalMonstersText->SetText( CT_Clear , AdditionalText );
			}
		}

		if( m_AdditionalMonstersBg )
		{
			m_AdditionalMonstersBg->SetVisible( TeamBOutOfCombatList.Len() > 0 );
		}

		RefreshNextCombatant();
	}

	RefreshGrids();
}

void ExCombatMenu::ChangeMenuState( ex_menu_s NewState )
{
	m_MenuState = NewState;

	eg_bool bTeamAListEnabled = false;
	eg_bool bTeamBListEnabled = false;
	EGUiWidget* FocusWidget = nullptr;
	eg_uint FocusWidgetIndex = 0;

	switch( m_MenuState )
	{
	case ex_menu_s::UNK:
		assert( false );
		break;
	case ex_menu_s::BEGIN:
		break;
	case ex_menu_s::PLAYER_TURN:
		m_TurnInput = ExCombat::exTurnInput( CT_Default );
		break;
	case ex_menu_s::PLAYER_TURN_MAKING_CHOICE:
		break;
	case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA:
		bTeamAListEnabled = true;
		FocusWidget = m_TeamA;
		FocusWidgetIndex = 0;
		break;
	case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB:
		bTeamBListEnabled = true;
		FocusWidget = m_OpponentsWidget;
		FocusWidgetIndex = 0;
		break;
	case ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED:
		break;
	case ex_menu_s::AI_TURN:
		m_TimeTilAutoTurn = EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS;
		break;
	case ex_menu_s::COMPLETE:
		ChangeMusicForCombatComplete();
		m_TimeTilAutoTurn = EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS;
		if( m_TeamA )
		{
			m_TeamA->SetScaledUp( false );
		}
		break;
	}

	if( m_TeamA )
	{
		m_TeamA->SetEnabled( bTeamAListEnabled );
	}
	if( m_OpponentsWidget )
	{
		m_OpponentsWidget->SetEnabled( bTeamBListEnabled );
	}

	SetFocusedWidget( FocusWidget, FocusWidgetIndex, false );

	RefreshNextCombatant();
	PopulateHints();
}

void ExCombatMenu::ApplyAmbush()
{
	if( m_Combat )
	{
		m_Combat->ApplyAmbush();
	}
}

void ExCombatMenu::OnInit()
{
	Super::OnInit();

	ExClient* ClientOwner = EGCast<ExClient>( GetOwnerClient() );
	if( ClientOwner )
	{
		ClientOwner->PushBgMusic( "/music/combat" );
		ExWorldRenderPipe* RenderPipe = EGCast<ExWorldRenderPipe>(ClientOwner->GetWorldRenderPipe());
		if( RenderPipe )
		{
			RenderPipe->SetCombatPostProcActive( true );
		}
	}

	HideHUD();

	DoReveal();

	m_AdditionalMonstersText = GetWidget<EGUiTextWidget>( eg_crc("AdditionalMonstersText") );
	m_AdditionalMonstersBg = GetWidget<EGUiWidget>( eg_crc("AdditionalMonstersBg") );
	m_NextMonsterBg = GetWidget<EGUiWidget>( eg_crc("NextMonsterBg") );
	m_NextMonsterText = GetWidget<EGUiTextWidget>( eg_crc("NextMonsterText") );
	m_OpponentsWidget = GetWidget<ExCombatMenuOpponentsWidget>( eg_crc( "OpponentsWidget" ) );
	m_SpellInfoWidget = GetWidget<ExItemCardWidget>( eg_crc("SpellInfo") );
	m_CombatChoiceInfoWidget = GetWidget<ExItemCardWidget>( eg_crc("CombatChoiceInfoWidget") );

	m_OwnerClient = GetOwnerClient();
	if( nullptr == m_OwnerClient )
	{
		m_OwnerClient = GetPrimaryClient();
	}

	m_Combat = EGNewObject<ExCombat>();
	if( nullptr == m_Combat )
	{
		assert( false ); // Out of memory?
		MenuStack_Pop();
		return;
	}

	ExGame* Game = GetGame();
	if( Game )
	{
		Game->SetCurrentCombat( m_Combat );
	}
	m_Combat->BeginCombat( Game );

	if( m_OpponentsWidget )
	{
		m_OpponentsWidget->TargetChosenDelegate.Bind( this , &ThisClass::OnEnemyTargetChosen );
		m_OpponentsWidget->InitCombat( m_Combat );
	}

	if( m_SpellInfoWidget )
	{
		m_SpellInfoWidget->RunEvent( eg_crc("HideNow") );
	}

	if( m_CombatChoiceInfoWidget )
	{
		m_CombatChoiceInfoWidget->RunEvent( eg_crc("HideNow") );
	}

	m_TimeTilAutoTurn = EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS;

	m_CombatOrder = GetWidget( eg_crc( "CombatOrder" ) );
	m_TeamA = GetWidget<ExCombatPartyWidget>( eg_crc( "TeamA" ) );
	if( m_TeamA )
	{
		m_TeamA->CellClickedDelegate.Bind( this , &ThisClass::OnPlayerPortraitClicked );
		m_TeamA->SetScaledUp( true );
		m_TeamA->InitCombat( m_Combat );
	}
	m_HelpBg = GetWidget<EGUiWidget>( eg_crc("HelpBg") );
	m_HelpText = GetWidget<ExHintBarWidget>( eg_crc( "HelpText" ) );
	m_QuickCombatHelpText = GetWidget<ExHintBarWidget>( eg_crc("QuickCombatHint") );

	if( m_CombatOrder && m_TeamA && m_OpponentsWidget )
	{
		m_CombatOrder->SetEnabled( false );
		m_TeamA->SetEnabled( false );
		m_OpponentsWidget->SetEnabled( false );
		m_TeamA->SetVisible( true ); // This was hidden by the reveal, but since this is in the same location as the hud we just want it to stay there.
	}

	ChangeMenuState( ex_menu_s::BEGIN );
}

void ExCombatMenu::OnDeinit()
{
	Super::OnDeinit();

	ExGame* Game = GetGame();
	if( Game )
	{
		Game->SetCurrentCombat( nullptr );
	}

	if( ExClient* ClientOwner = EGCast<ExClient>( GetOwnerClient() ) )
	{
		ExWorldRenderPipe* RenderPipe = EGCast<ExWorldRenderPipe>(ClientOwner->GetWorldRenderPipe());
		if( RenderPipe )
		{
			RenderPipe->SetCombatPostProcActive( false );
		}
	}

	ChangeMusicForMenuDismissed();

	ShowHUD( true );
	EGDeleteObject( m_Combat );
	m_Combat = nullptr;
}

void ExCombatMenu::OnUpdate( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime, AspectRatio );

	if( m_TeamA )
	{
		if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA )
		{
			m_TeamA->SetHighlightedPortrait( m_TeamA->GetSelectedIndex() );
		}
		else
		{
			m_TeamA->SetHighlightedFighter( IsCombatLogFullyRevealed() ? m_Combat->GetNextCombatant() : nullptr );
		}
	}

	if( IsActive() )
	{
		if( m_MenuState == ex_menu_s::PLAYER_TURN )
		{
			if( IsCombatLogFullyRevealed() )
			{
				ChangeMenuState( ex_menu_s::PLAYER_TURN_MAKING_CHOICE );
				assert( m_Combat->GetNextCombatant() && m_Combat->GetNextCombatant()->GetTeamIndex() == 0 );
				if( m_BaseChoiceCb ) { m_BaseChoiceCb->PushChoiceMenu( m_Combat->GetNextCombatant() ); }
			}
		}
		else if( m_MenuState == ex_menu_s::AI_TURN )
		{
			if( IsCombatLogFullyRevealed() )
			{
				m_TimeTilAutoTurn -= DeltaTime;
				if( m_TimeTilAutoTurn <= 0.f )
				{
					SendTurnInputToCombat();
				}
			}
		}
		else if( m_MenuState == ex_menu_s::COMPLETE )
		{
			if( IsCombatLogFullyRevealed() )
			{
				m_TimeTilAutoTurn -= DeltaTime;
				if( m_TimeTilAutoTurn <= 0.f )
				{
					exCombatTeamResults Results = m_Combat->GetCombatResultsForTeam( 0 );

					eg_string_crc ResultHeader = eg_loc( "CombatCompleteHeader", "Combat!" );
					eg_string_crc ResultMessage = eg_loc( "CombatComplete", "The combat was completed." );

					switch( Results.Result )
					{
					case ex_combat_result::Defeat:
						ResultHeader = eg_loc( "CombatCompleteHeaderFailure", "|SC(EX_HURT)|Death Strikes!" );
						ResultMessage = eg_loc( "CombatCompleteFailure", "Defeat! All party members were vanquished. All progress since the last save has been lost." );
						ExAchievements::Get().UnlockAchievement( eg_crc("ACH_DEATHSTRIKES") );
						break;
					case ex_combat_result::Victory:
						ResultHeader = eg_loc( "CombatCompleteHeaderSuccess", "|SC(EX_HEAL)|Victory!" );
						ResultMessage = eg_loc( "CombatCompleteSuccesS", "Combat was successful! Each surviving party member receives |SC(EX_ATTR)|{0} XP|RC()|." );
						ExAchievements::Get().UnlockAchievement( eg_string_crc("ACH_COMBAT_1") );
						if( Results.bKilledBossCreature )
						{
							// Only unlock killing a boss if the combat was successful.
							ExAchievements::Get().UnlockAchievement( eg_crc("ACH_BOSS") );
						}
						if( m_Combat->IsAmbush() )
						{
							ExAchievements::Get().UnlockAchievement( eg_crc("ACH_AMBUSH") );
						}
						break;
					case ex_combat_result::Fled:
						ResultHeader = eg_loc( "CombatCompleteHeaderFled", "Escaped!" );
						ResultMessage = eg_loc( "CombatCompleteFled", "The party fled and will be returned to a safe location. No |SC(EX_ATTR)|XP|RC()| was awarded. All party members that did not escape have died." );
						break;
					}

					exMessageDialogParms DlgParams( EGFormat( ResultMessage, Results.XPPerSurvivor ) );
					DlgParams.HeaderText = eg_loc_text( ResultHeader );
					ExDialogMenu_PushDialogMenu( m_OwnerClient, DlgParams );
				}
			}
		}
	}
}

void ExCombatMenu::OnActivate()
{
	Super::OnActivate();

	if( m_MenuState == ex_menu_s::BEGIN )
	{
		OnTurnComplete(); // This will get combat rolling.

		// If the enemy team goes first give a slightly longer delay than normal.
		if( m_Combat->GetNextCombatant()->GetTeamIndex() == 1 )
		{
			m_TimeTilAutoTurn = EX_COMBAT_MENU_TIME_TIL_ENEMY_FIGHTS_FIRST_ATTACK;
		}
	}

	if( m_MenuState == ex_menu_s::COMPLETE )
	{
		m_Combat->SendCombatResultsToServer( GetGame() );
		MenuStack_Pop();
	}

	if( m_MenuState == ex_menu_s::PLAYER_TURN_MAKING_CHOICE )
	{
		eg_bool bClearHelpText = false;
		if( m_TurnInput.Action == ex_combat_actions::ABANDON )
		{
			bClearHelpText = true;
			ExDialogMenu_PushDialogMenu( m_OwnerClient, exYesNoDialogParms( eg_loc( "AbandonCombatQuestion", "Are you sure you want to abandon combat? Doing so will result in the death of all party members that have not escaped." ), this, eg_crc( "AbandonDlg" ), true ) );
		}

		// This means we backed out of the choice menu, so clear the
		// input since the player canceled.
		ChangeMenuState( ex_menu_s::PLAYER_TURN );

		if( bClearHelpText )
		{
			ClearHints();
		}
	}
	else if( m_MenuState == ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED )
	{
		if( m_TurnInput.Action == ex_combat_actions::CAST )
		{
			ExCombatant* Attacker = m_Combat->GetNextCombatant();
			if( Attacker && Attacker->GetSpells().IsValidIndex( m_TurnInput.ActionIndex ) )
			{
				const exSpellInfo* Spell = Attacker->GetSpells()[m_TurnInput.ActionIndex];
				if( Spell->Target == ex_spell_target::Self || Spell->Target == ex_spell_target::AllActive || Spell->TargetCount > 1 || Spell->Target == ex_spell_target::None )
				{
					SendTurnInputToCombat();
				}
				else if( Spell->CanTargetEnemy() )
				{
					ChangeMenuState( ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB );
				}
				else if( Spell->CanTargetFriendly() )
				{
					ChangeMenuState( ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA );
				}
				else
				{
					assert( false ); // bad state
				}
			}
			else
			{
				assert( false ); // bad state
				ChangeMenuState( ex_menu_s::PLAYER_TURN );
			}
		}
		else
		{
			SendTurnInputToCombat();
		}
	}
}

void ExCombatMenu::OnDeactivate()
{
	PopulateHints();

	Super::OnDeactivate();
}

eg_bool ExCombatMenu::OnInput( eg_menuinput_t InputType )
{
	eg_bool bHandled = true;

	switch( InputType )
	{
	case eg_menuinput_t::BUTTON_PRIMARY:
	{
		switch( m_MenuState )
		{
		case ex_menu_s::UNK:
			break;
		case ex_menu_s::BEGIN:
			break;
		case ex_menu_s::PLAYER_TURN:
			Continue();
			break;
		case ex_menu_s::PLAYER_TURN_MAKING_CHOICE:
			break;
		case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA:
			bHandled = false; // Want input to go to list.
			break;
		case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB:
			bHandled = false; // Want input to go to list.
			break;
		case ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED:
			break;
		case ex_menu_s::AI_TURN:
		{
			// This is something I've debated. If the player is watching the combat
			// log it makes sense to only reveal the log but the player is probably
			// watching the monsters or the party so pressing a button they just
			// want the next action to happen.
			eg_bool bDontStartTurnWhenSkippingLog = false; 

			if( !IsCombatLogFullyRevealed() && bDontStartTurnWhenSkippingLog )
			{
				ForceFullCombatLog();
			}
			else
			{
				Continue();
			}
		} break;
		case ex_menu_s::COMPLETE:
			Continue();
			break;
		}
	} break;
	case eg_menuinput_t::BUTTON_BACK:
	{
		if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB || m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA )
		{
			ChangeMenuState( ex_menu_s::PLAYER_TURN );
			if( m_OpponentsWidget )
			{
				m_OpponentsWidget->HandleBackedOut();
			}
		}
	} break;
	default: bHandled = false; break;
	}

	return bHandled;
}

void ExCombatMenu::OnDialogClosed( eg_string_crc ListenerParm, eg_string_crc Choice )
{
	if( ListenerParm == eg_crc( "AbandonDlg" ) )
	{
		assert( m_MenuState == ex_menu_s::PLAYER_TURN ); // Bad flow...
		if( Choice == eg_crc( "Yes" ) )
		{
			m_TurnInput = ExCombat::exTurnInput( CT_Clear );
			m_TurnInput.TurnOpt = ex_combat_turn_opt::CUSTOM;
			m_TurnInput.Action = ex_combat_actions::ABANDON;
			SendTurnInputToCombat();
		}
		else
		{
			ChangeMenuState( m_MenuState ); // This will reset the help text.
		}
	}
}

void ExCombatMenu::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( Cmds.WasPressed( CMDA_MENU_BTN2 ) || Cmds.WasRepeated( CMDA_MENU_BTN2 ) )
	{
		ForceFullCombatLog();

		if( m_MenuState == ex_menu_s::PLAYER_TURN_MAKING_CHOICE )
		{
			MenuStack_Pop();
			assert( IsActive() );
		}

		if( m_MenuState == ex_menu_s::COMPLETE )
		{
			m_TimeTilAutoTurn = 0.f;
		}

		if( m_MenuState == ex_menu_s::PLAYER_TURN || m_MenuState == ex_menu_s::AI_TURN )
		{
			if( m_MenuState == ex_menu_s::PLAYER_TURN )
			{
				m_TurnInput = ExCombat::exTurnInput( CT_Clear );
				m_TurnInput.TurnOpt = ex_combat_turn_opt::CUSTOM;
				m_TurnInput.Action = ex_combat_actions::FIGHT;
			}
			SendTurnInputToCombat(); // This will do auto for players as well.
		}
	}

	if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA && m_TeamA )
	{
		auto ChangePartyMember = [this]( eg_bool bInc ) -> void
		{
			m_TeamA->HandleInput( bInc ? eg_menuinput_t::BUTTON_RIGHT : eg_menuinput_t::BUTTON_LEFT , CT_Clear , false );
		};

		if( Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) )
		{
			ChangePartyMember( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) );
		}
	}

	if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB && m_OpponentsWidget )
	{
		auto ChangeTarget = [this]( eg_bool bInc ) -> void
		{
			m_OpponentsWidget->HandleInput( bInc ? eg_menuinput_t::BUTTON_RIGHT : eg_menuinput_t::BUTTON_LEFT , CT_Clear , false );
		};

		if( Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) )
		{
			ChangeTarget( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) );
		}
	}
}

void ExCombatMenu::OnTurnComplete()
{
	m_TurnInput = ExCombat::exTurnInput( CT_Default );
	RevealCombatLog();

	Refresh();

	if( m_Combat->IsCombatComplete() )
	{
		ChangeMenuState( ex_menu_s::COMPLETE );
	}
	else
	{
		if( m_Combat->GetNextCombatant()->GetTeamIndex() == 0 )
		{
			ChangeMenuState( ex_menu_s::PLAYER_TURN );
		}
		else
		{
			ChangeMenuState( ex_menu_s::AI_TURN );
		}
	}
}

void ExCombatMenu::SendTurnInputToCombat()
{
	SetViewedCombatChoice( nullptr , ex_combat_actions::UNKNOWN );
	m_Combat->ProcessTurn( m_TurnInput );
	OnTurnComplete();
}

void ExCombatMenu::SetCombatAction( ex_combat_actions Action )
{
	m_TurnInput.Action = Action;
	m_TurnInput.TurnOpt = Action != ex_combat_actions::AUTO ? ex_combat_turn_opt::CUSTOM : ex_combat_turn_opt::AUTO;
}

void ExCombatMenu::SetCombatActionIndex( eg_uint Index )
{
	m_TurnInput.ActionIndex = Index;
}

void ExCombatMenu::CommitCombatantChoice()
{
	ChangeMenuState( ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED );
}

void ExCombatMenu::Continue()
{
	if( m_MenuState == ex_menu_s::PLAYER_TURN )
	{
		ForceFullCombatLog();

		ChangeMenuState( ex_menu_s::PLAYER_TURN_MAKING_CHOICE );
		assert( m_Combat->GetNextCombatant()->GetTeamIndex() == 0 );
		if( m_BaseChoiceCb ) { m_BaseChoiceCb->PushChoiceMenu( m_Combat->GetNextCombatant() ); }
	}
	else if( m_MenuState == ex_menu_s::AI_TURN )
	{
		ForceFullCombatLog();
		m_TimeTilAutoTurn = 0.f;
	}
	else if( m_MenuState == ex_menu_s::COMPLETE )
	{
		ForceFullCombatLog();
		m_TimeTilAutoTurn = 0.f;
	}
}

void ExCombatMenu::OnPlayerPortraitClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
{	
	if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA && m_TeamAList.IsValidIndex( CellInfo.GridIndex ) )
	{
		if( m_TeamAList[CellInfo.GridIndex] && m_TeamAList[CellInfo.GridIndex]->GetFighter().IsCreated() )
		{
			AttemptTargetSelection( m_TeamAList[CellInfo.GridIndex] );
		}
		else
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
		}
	}
	else
	{
		assert( false );
	}
}

void ExCombatMenu::OnEnemyTargetChosen( const ExCombatant* Target )
{
	if( m_MenuState == ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB )
	{
		for( ExCombatant* ListObj : m_TeamBList )
		{
			if( Target == ListObj )
			{
				AttemptTargetSelection( ListObj );
				return;
			}
		}
	}
	
	assert( false ); // Invalid target chosen?
}

void ExCombatMenu::AttemptTargetSelection( ExCombatant* Target )
{
	ExCombatant* Attacker = m_Combat->GetNextCombatant();

	if( m_TurnInput.Action == ex_combat_actions::MELEE )
	{
		if( Target && Target->GetCombatState() == ex_combat_s::FIRST_ROW )
		{
			m_TurnInput.Target = Target;
			ChangeMenuState( ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED );
			SendTurnInputToCombat();
		}
		else
		{
			ExDialogMenu_PushDialogMenu( m_OwnerClient, exMessageDialogParms( eg_loc( "CombatInvalidMeleeTarget", "That target is out of range for melee combat." ) ) );
		}
	}
	else if( m_TurnInput.Action == ex_combat_actions::SHOOT )
	{
		if( Target && (Target->GetCombatState() == ex_combat_s::FIRST_ROW || Target->GetCombatState() == ex_combat_s::BACK_ROW) )
		{
			m_TurnInput.Target = Target;
			ChangeMenuState( ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED );
			SendTurnInputToCombat();
		}
		else
		{
			ExDialogMenu_PushDialogMenu( m_OwnerClient, exMessageDialogParms( eg_loc( "CombatInvalidShootingTarget", "That target is out of range for shooting." ) ) );
		}
	}
	else if( m_TurnInput.Action == ex_combat_actions::CAST )
	{
		if( Attacker && Attacker->GetSpells().IsValidIndex( m_TurnInput.ActionIndex ) )
		{
			const exSpellInfo* Spell = Attacker->GetSpells()[m_TurnInput.ActionIndex];
			assert( Spell->TargetCount == 1 );
			if( Target && Target->SpellCanReach( Spell ) )
			{
				m_TurnInput.Target = Target;
				ChangeMenuState( ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED );
				SendTurnInputToCombat();
			}
			else
			{
				ExDialogMenu_PushDialogMenu( m_OwnerClient, exMessageDialogParms( eg_loc( "CombatInvalidSpellTarget", "This spell cannot reach that target." ) ) );
			}
		}
		else
		{
			assert( false ); // Bad state
		}
	}
}

void ExCombatMenu::RevealCombatLog()
{
	eg_loc_char CombatLog[1024];
	m_Combat->GetCombatLog( CombatLog, countof( CombatLog ) );
	ExTextRevealWidget* CombatLogText = GetWidget<ExTextRevealWidget>( eg_crc( "CombatLogText" ) );
	if( CombatLogText )
	{
		CombatLogText->RevealText( eg_loc_text( CombatLog ), EG_To<ex_reveal_speed>(ExGameSettings_CombatTextSpeed.GetValue()) );
	}
}

void ExCombatMenu::ForceFullCombatLog()
{
	ExTextRevealWidget* CombatLogText = GetWidget<ExTextRevealWidget>( eg_crc( "CombatLogText" ) );
	if( CombatLogText )
	{
		CombatLogText->ForceFullReveal();
	}
}

eg_bool ExCombatMenu::IsCombatLogFullyRevealed()
{
	ExTextRevealWidget* CombatLogText = GetWidget<ExTextRevealWidget>( eg_crc( "CombatLogText" ) );
	return CombatLogText ? CombatLogText->IsFullyRevealed() : true;
}

void ExCombatMenu::OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
{
	ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

	switch_crc( ItemInfo.WidgetId )
	{
		case_crc( "CombatOrder" ) :
		{
			const eg_loc_char* FormatStringNameNext = L"|SetColor(RED)|{0:NAME} ({0:ROW},{0:SPEED},{0:ROUND_TIME})|RestoreColor()|";
			const eg_loc_char* FormatStringName = L"{0:NAME} ({0:ROW},{0:SPEED},{0:ROUND_TIME})";

			const ExCombatant* NextFighter = m_Combat->GetNextCombatant();
			const ExCombatant* ThisFighter = m_CombatOrderList[ItemInfo.GridIndex];

			eg_uint TeamListIndex = 0;
			const ExCombatantList& TeamList = ThisFighter->GetTeamIndex() == 0 ? m_TeamAList : m_TeamBList;

			for( eg_size_t i = 0; i < TeamList.Len(); i++ )
			{
				if( TeamList[i] == ThisFighter )
				{
					TeamListIndex = static_cast<eg_uint>( i );
				}
			}

			ItemInfo.Widget->SetText( eg_crc( "NameText" ), EGFormat( NextFighter == ThisFighter ? FormatStringNameNext : FormatStringName, ThisFighter ) );
			eg_int Health = static_cast<eg_int>( ThisFighter->GetHP() );
			eg_int MaxHealth = static_cast<eg_int>( ThisFighter->GetAttrValue( ex_attr_t::HP ) );
			ItemInfo.Widget->SetText( eg_crc( "HpText" ), EGFormat( L"{0}/{1}", Health, MaxHealth ) );
		} break;
	}
}

void ExCombatMenu::RefreshGrids()
{
	auto RefreshGridWidget = [this]( eg_string_crc Name , eg_uint Size , eg_bool bIncludeItemChanged ) -> void
	{
		EGUiGridWidget* GridWidget = GetWidget<EGUiGridWidget>( Name );
		if( GridWidget )
		{
			if( bIncludeItemChanged )
			{
				GridWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
			}
			GridWidget->RefreshGridWidget( Size );
		}
	};

	RefreshGridWidget( eg_crc( "TeamA" ), m_TeamAList.LenAs<eg_uint>() , false );
	RefreshGridWidget( eg_crc( "CombatOrder" ), m_CombatOrderList.LenAs<eg_uint>() , true );

	if( EGUiWidget* CombatOrderGrid = GetWidget<EGUiWidget>( eg_crc("CombatOrder") ) )
	{
		CombatOrderGrid->SetVisible( EX_CHEATS_ENABLED && ExCombat_DebugShowCombatOrder.GetValue() );
	}
	if( EGUiWidget* CombatOrderHeaderText = GetWidget<EGUiWidget>( eg_crc("CombatOrderText") ) )
	{
		CombatOrderHeaderText->SetVisible( EX_CHEATS_ENABLED && ExCombat_DebugShowCombatOrder.GetValue() );
	}
}

void ExCombatMenu::ChangeMusicForCombatComplete()
{
	exCombatTeamResults Results = m_Combat->GetCombatResultsForTeam( 0 );

	EGClient* OwnerClient = GetOwnerClient();
	EGSoundScape* SoundScape = OwnerClient ? OwnerClient->SDK_GetSoundScape() : nullptr;
	if( SoundScape )
	{
		switch( Results.Result )
		{
		case ex_combat_result::Defeat:
		{
			SoundScape->Stop();

			eg_bool bUse2Parts = false;
			if( bUse2Parts )
			{
				SoundScape->PushBgMusic( "/music/combat/defeat_part1" );
				SoundScape->SetNextBgMusicTrack( "/music/combat/defeat_part2" );
			}
			else
			{
				SoundScape->PushBgMusic( "/music/combat/defeat_part2" );
			}
		} break;
		case ex_combat_result::Victory:
			SoundScape->SwitchBgMusic( "/music/combat/victory" );
			break;
		case ex_combat_result::Fled:
			// Return to the map music right away.
			SoundScape->PopBgMusic();
			break;
		}
	}
}

void ExCombatMenu::ChangeMusicForMenuDismissed()
{
	exCombatTeamResults Results = m_Combat->GetCombatResultsForTeam( 0 );

	EGClient* ClientOwner = GetOwnerClient();
	EGSoundScape* SoundScape = ClientOwner ? ClientOwner->SDK_GetSoundScape() : nullptr;
	if( ClientOwner && MainAudioList ) // We check for MainAudioList because if the reason the menu is being dismissed is cuz the game is closing that won't be valid and will crash later.
	{
		switch( Results.Result )
		{
		case ex_combat_result::Defeat:
			break;
		case ex_combat_result::Victory:
			SoundScape->PopBgMusic( .5f , .5f );
			break;
		case ex_combat_result::Fled:
			break;
		default:
			break;
		}
	}
}

void ExCombatMenu::SetViewedSpell( ExCombatant* Combatant , eg_uint SpellChoice )
{
	if( m_SpellInfoWidget )
	{
		const exSpellInfo* SpellInfo = Combatant && Combatant->GetSpells().IsValidIndex( SpellChoice ) ? Combatant->GetSpells()[SpellChoice] : nullptr;

		if( Combatant && SpellInfo )
		{
			m_SpellInfoWidget->SetSpell( SpellInfo , &Combatant->GetFighter() , true );
			m_SpellInfoWidget->RunEvent( eg_crc("Show") );
		}
		else
		{
			m_SpellInfoWidget->RunEvent( eg_crc("HideNow") );
		}
	}
}

void ExCombatMenu::SetSpellCardOffset( const eg_transform& Transform )
{
	if( m_SpellInfoWidget )
	{
		m_SpellInfoWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , Transform );
	}
}

void ExCombatMenu::SetViewedCombatActionOffset( const eg_transform& Transform )
{
	if( m_CombatChoiceInfoWidget )
	{
		m_CombatChoiceInfoWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , Transform );
	}
}

void ExCombatMenu::SetViewedCombatChoice( ExCombatant* Combatant , ex_combat_actions Action , const exSpellInfo* Spell /*= nullptr*/ )
{
	if( m_CombatChoiceInfoWidget )
	{
		if( Combatant && Action != ex_combat_actions::UNKNOWN )
		{
			if( Spell )
			{
				m_CombatChoiceInfoWidget->SetSpellTargeting( &Combatant->GetFighter() , true , Action , Spell ); // SetAttack controls visibility
			}
			else
			{
				m_CombatChoiceInfoWidget->SetAttack( &Combatant->GetFighter() , true , Action , Combatant->IsInFrontLines() , Combatant->HasRangedAttack() ); // SetAttack controls visibility
			}
		}
		else
		{
			m_CombatChoiceInfoWidget->RunEvent( eg_crc("HideNow") );
		}
	}
}

void ExCombatMenu::PopulateHints()
{
	static const eg_string_crc ContinueText = eg_loc( "CombatHelp_Continue", "Continue" );
	static const eg_string_crc QuickCombatText = eg_loc( "CombatHelp_QuickCombatText", "Quick Combat" );
	static const eg_string_crc BackText = eg_loc( "CombatHelp_Back", "Back" );
	static const eg_string_crc ChooseTargetText = eg_loc("CombatHelp_ChooseTarget","Choose Target");

	eg_bool bHasQuickCombatHint = true;

	if( m_HelpText )
	{
		m_HelpText->ClearHints();

		switch( m_MenuState )
		{
			case ex_menu_s::UNK:
				break;
			case ex_menu_s::BEGIN:
				m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ContinueText) );
				break;
			case ex_menu_s::PLAYER_TURN:
				m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ContinueText) );
				break;
			case ex_menu_s::PLAYER_TURN_MAKING_CHOICE:
				break;
			case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA:
				m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ChooseTargetText) );
				m_HelpText->AddHint( CMDA_MENU_BACK , eg_loc_text(BackText) );
				bHasQuickCombatHint = false;
				break;
			case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB:
				m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ChooseTargetText) );
				m_HelpText->AddHint( CMDA_MENU_BACK , eg_loc_text(BackText) );
				bHasQuickCombatHint = false;
				break;
			case ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED:
				break;
			case ex_menu_s::AI_TURN:
				m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ContinueText) );
				break;
			case ex_menu_s::COMPLETE:
				bHasQuickCombatHint = false;
				break;
		}
	}

	if( m_QuickCombatHelpText )
	{
		m_QuickCombatHelpText->ClearHints();
		if( bHasQuickCombatHint )
		{
			m_QuickCombatHelpText->AddHint( CMDA_MENU_BTN2 , eg_loc_text(QuickCombatText) );
		}
	}

	if( m_HelpBg )
	{
		m_HelpBg->RunEvent( m_HelpText && m_HelpText->HasHints() ? eg_crc("ShowNow") : eg_crc("HideNow") );
	}
}

void ExCombatMenu::ClearHints()
{
	if( m_HelpText )
	{
		m_HelpText->ClearHints();
	}

	if( m_HelpBg )
	{
		m_HelpBg->RunEvent( eg_crc("HideNow") );
	}
}

eg_bool ExCombatMenu::ShouldShowNextCombatant() const
{
	switch( m_MenuState )
	{
	case ex_menu_s::UNK:
		break;
	case ex_menu_s::BEGIN:
		break;
	case ex_menu_s::PLAYER_TURN:
		break;
	case ex_menu_s::PLAYER_TURN_MAKING_CHOICE:
	case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMA:
	case ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB:
		return true;
	case ex_menu_s::PLAYER_TURN_CHOICE_COMMITTED:
		break;
	case ex_menu_s::AI_TURN:
		break;
	case ex_menu_s::COMPLETE:
		break;
	}

	return false;
}

void ExCombatMenu::RefreshNextCombatant()
{
	if( m_NextMonsterBg && m_NextMonsterText )
	{
		eg_loc_text NextMonsterText( CT_Clear );
		if( ShouldShowNextCombatant() && m_Combat && m_Combat->GetFutureCombatant() )
		{
			NextMonsterText = EGFormat( eg_loc("CombatMenuNextMonsterText","Next: {0:NAME}") , m_Combat->GetFutureCombatant() );
		}

		m_NextMonsterText->SetText( CT_Clear , NextMonsterText );
		m_NextMonsterText->SetVisible( NextMonsterText.GetLen() > 0 );
		m_NextMonsterBg->SetVisible( NextMonsterText.GetLen() > 0 );
	}
}

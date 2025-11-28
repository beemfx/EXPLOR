// (c) 2016 Beem Media

#include "ExCombatMenu_SubMenus.h"
#include "ExCharacterMenu.h"
#include "ExChoiceWidgetMenu.h"
#include "ExCombatant.h"
#include "ExCombatMenu.h"

EG_CLASS_DECL( ExCombatMenu_SpellsList )
EG_CLASS_DECL( ExCombatMenu_CombatantAction )

//
// Spells List
//

void ExCombatMenu_SpellsList::PushChoiceMenu( ExCombatant* Combatant )
{
	m_Combatant = Combatant;
	ExChoiceWidgetMenu::exConfig ChoiceConfig;
	ChoiceConfig.bPushCascaded = true;
	ChoiceConfig.InitialSelection = 0;
	ChoiceConfig.bCanPop = true;

	for( eg_uint i=0; i<m_Combatant->GetSpells().LenAs<eg_uint>(); i++ )
	{
		ChoiceConfig.Choices.Append( GetChoiceText( i ) );
	}

	ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( m_Owner , ChoiceConfig );
	if( ChoiceMenu )
	{
		ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnChoiceMade );
		ChoiceMenu->ChoiceSelectedDelegate.Bind( this , &ThisClass::OnChoiceHighlighted );
		ChoiceMenu->ChoiceAbortedDelegate.Bind( this , &ThisClass::OnChoiceAborted );
	}
}

void ExCombatMenu_SpellsList::OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	if( m_Combatant->GetSpells().IsValidIndex( ChoiceIndex ) )
	{
		const exSpellInfo* ChosenSpell = m_Combatant->GetSpells()[ChoiceIndex];
		const eg_bool bCanCast =  m_Combatant->CanCast( ChosenSpell );
		if( bCanCast )
		{
			m_Owner->SetCombatActionIndex( ChoiceIndex );
			m_Owner->CommitCombatantChoice();
			// Pop this and the combat choice menu
			m_Owner->SetViewedSpell( nullptr , 0 );
			m_Owner->SetViewedCombatChoice( m_Combatant , ex_combat_actions::CAST , ChosenSpell );
			// Change where DMG display appears.
			if( m_Owner )
			{
				eg_transform CombatActionPose = CT_Default;
				static eg_real OffsetAmt = -20.f;
				CombatActionPose.TranslateThis( 0.f , OffsetAmt , 0.f );
				m_Owner->SetViewedCombatActionOffset( CombatActionPose );
			}
			m_Owner->MenuStack_Pop();
			m_Owner->MenuStack_Pop();
		}
		else
		{
			ExDialogMenu_PushDialogMenu( m_Owner->GetOwnerClient() , exMessageDialogParms( eg_loc("CombatNotEnoughMana","Not enough mana to cast this spell.") ) );
		}
	}
	else
	{
		assert( false );
	}
}

void ExCombatMenu_SpellsList::OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );
	
	if( m_Owner && m_Combatant  )
	{
		m_Owner->SetViewedSpell( m_Combatant , ChoiceIndex );
	}
}

void ExCombatMenu_SpellsList::OnChoiceAborted( ExChoiceWidgetMenu* SourceMenu )
{
	unused( SourceMenu );
	
	if( m_Owner )
	{
		m_Owner->SetViewedSpell( nullptr , 0 );
	}
}

eg_loc_text ExCombatMenu_SpellsList::GetChoiceText( eg_uint Index ) const
{
	eg_loc_text Out = CT_Clear;

	if( m_Combatant->GetSpells().IsValidIndex( Index ) )
	{
		Out = EGFormat( eg_loc("CombatSpellName","{0} ({1} MP)") , m_Combatant->GetSpells()[Index]->Name , m_Combatant->GetSpells()[Index]->GetManaCost( m_Combatant->GetMeleeAttrValue( ex_attr_t::MAG ) , m_Combatant->GetAttrValue( ex_attr_t::LVL ) ) );
	}

	return Out;
}

//
// Combat Choices:
//

void ExCombatMenu_CombatantAction::PushChoiceMenu( ExCombatant* Combatant )
{
	m_Actions.Clear();
	m_Combatant = Combatant;
	Combatant->GetAvailableActions( m_Actions );
	ExChoiceWidgetMenu::exConfig ChoiceConfig;
	ChoiceConfig.bPushCascaded = false;
	ChoiceConfig.InitialSelection = 1;
	ChoiceConfig.bCanPop = false;
	ChoiceConfig.bForwardInputCmdsToParent = true;

	for( eg_uint i=0; i<m_Actions.LenAs<eg_uint>(); i++ )
	{
		ChoiceConfig.Choices.Append( GetChoiceText( i ) );
	}

	ChoiceConfig.SetPoseForPortrait( Combatant->GetServerPartyIndex() , true );
	if( Combatant->GetCombatState() == ex_combat_s::FIRST_ROW )
	{
		ChoiceConfig.Pose.TranslateThis( 0.f , EX_COMBAT_PORTRAIT_FIRST_ROW_OFFSET*EX_PORTRAIT_SCALE_BIG , 0.f );
	}

	if( m_Owner )
	{
		eg_transform SpellCardPose = ChoiceConfig.Pose;
		SpellCardPose.TranslateThis( 10.f , 43.5f + 6.5f , 0.f ); // Cascade offset.
		m_Owner->SetSpellCardOffset( SpellCardPose );

		eg_transform CombatActionPose = ChoiceConfig.Pose;
		CombatActionPose.TranslateThis( 0.f , 15.5f , 0.f );
		m_Owner->SetViewedCombatActionOffset( CombatActionPose );
	}

	ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( m_Owner , ChoiceConfig );
	if( ChoiceMenu )
	{
		ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnChoiceMade );
		ChoiceMenu->ChoiceSelectedDelegate.Bind( this , &ThisClass::OnChoiceHighlighted );
	}
}

eg_string_crc ExCombatMenu_CombatantAction::GetTextForAction( ex_combat_actions Action )
{
	eg_string_crc Out( CT_Clear );
	switch( Action )
	{
	case ex_combat_actions::UNKNOWN: assert( false ); break;
	case ex_combat_actions::AUTO: Out = eg_loc("CombatActionAuto","Auto"); break;
	case ex_combat_actions::FIGHT: Out = eg_loc("CombatActionFight","Auto"); break;
	case ex_combat_actions::BLOCK: Out = eg_loc("CombatActionBlock","Block"); break;
	case ex_combat_actions::MELEE: Out = eg_loc("CombatActionAttack","Attack"); break;
	case ex_combat_actions::SHOOT: Out = eg_loc("CombatActionShoot","Shoot"); break;
	case ex_combat_actions::CAST: Out = eg_loc("CombatActionCast","Cast"); break;
	case ex_combat_actions::ADVANCE: Out = eg_loc("CombatActionAdvance","Advance"); break;
	case ex_combat_actions::BACKAWAY: Out = eg_loc("CombatActionBackAway","Back Away"); break;
	case ex_combat_actions::RUN: Out = eg_loc("CombatActionRun","Run"); break;
	case ex_combat_actions::VIEW: Out = eg_loc("CombatActionViewCharacter","View Character"); break;
	case ex_combat_actions::ABANDON: Out = eg_loc("CombatActionAbandon","Abandon Combat"); break;
	case ex_combat_actions::DEBUG_WIN: Out = eg_loc("CombatActionDebugWin","DEBUG: Win Fight"); break;
	}
	return Out;
}

void ExCombatMenu_CombatantAction::OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	m_Owner->SetCombatAction( m_Actions[ChoiceIndex] );

	eg_bool bHasSubChoices = false;

	switch( m_Actions[ChoiceIndex] )
	{
	case ex_combat_actions::UNKNOWN:
		assert( false );
		break;
	case ex_combat_actions::ABANDON:
		m_Owner->MenuStack_Pop();
		bHasSubChoices = true;
		break;
	case ex_combat_actions::AUTO:
	case ex_combat_actions::FIGHT:
	case ex_combat_actions::BLOCK:
	case ex_combat_actions::RUN :
	case ex_combat_actions::ADVANCE:
	case ex_combat_actions::BACKAWAY:
	case ex_combat_actions::DEBUG_WIN:
		bHasSubChoices = false;
		break;
	case ex_combat_actions::MELEE:
	case ex_combat_actions::SHOOT:
		m_Owner->ChangeMenuState( ExCombatMenu::ex_menu_s::PLAYER_CHOOSING_TARGET_TEAMB );
		m_Owner->MenuStack_Pop();
		bHasSubChoices = true;

		// Change where DMG display appears.
		if( m_Owner )
		{
			eg_transform CombatActionPose = CT_Default;
			static eg_real OffsetAmt = -20.f;
			CombatActionPose.TranslateThis( 0.f , OffsetAmt , 0.f );
			m_Owner->SetViewedCombatActionOffset( CombatActionPose );
		}
		break;
	case ex_combat_actions::CAST :
		if( m_SpellsList )
		{
			m_SpellsList->PushChoiceMenu( m_Combatant );
			bHasSubChoices = true; 
		}
		break;
	case ex_combat_actions::VIEW:
		bHasSubChoices = true;
		m_Owner->MenuStack_Pop();
		EGClient* Owner = m_Owner->GetOwnerClient();
		if( nullptr == Owner )
		{
			Owner = m_Owner->GetPrimaryClient();
		}
		ExCharacterMenu_ViewFighter( Owner , m_Combatant->GetFighter() , false , m_Combatant->GetServerPartyIndex() );
		break;
	}

	if( !bHasSubChoices )
	{
		m_Owner->CommitCombatantChoice();
		m_Owner->MenuStack_Pop();
	}
}

void ExCombatMenu_CombatantAction::OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	if( m_Owner && m_Combatant )
	{
		m_Owner->SetViewedCombatChoice( m_Combatant , m_Actions.IsValidIndex(ChoiceIndex) ? m_Actions[ChoiceIndex] : ex_combat_actions::UNKNOWN );
	}
}

eg_loc_text ExCombatMenu_CombatantAction::GetChoiceText( eg_uint Index ) const
{
	eg_loc_text Out = CT_Clear;
	if( 0 <= Index && Index < m_Actions.Len() )
	{
		Out = eg_loc_text(GetTextForAction( m_Actions[Index] ));
	}
	return Out;
}

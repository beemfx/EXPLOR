// (c) 2020 Beem Media. All Rights Reserved.

#include "ExCastSpellInWorldMenu.h"
#include "ExChoiceWidgetMenu.h"
#include "ExSpellInfo.h"
#include "ExSkills.h"
#include "EGUiGridWidget.h"
#include "ExCharacterPortraitWidget.h"
#include "ExCombatPartyWidget.h"
#include "ExDialogMenu.h"
#include "ExItemCardWidget.h"
#include "ExMapInfos.h"
#include "ExAchievements.h"

EG_CLASS_DECL( ExCastSpellInWorldMenu )

static const eg_real ExCastSpellInWorldMenu_CountdownForCast = 1.f;
static const eg_real ExCastSpellInWorldMenu_CountdownForCancel = .5f;

eg_bool ExCastSpellInWorldMenu::CanCastAnyInWorldSpells( const ExGame* Game )
{
	if( Game )
	{
		const ExFighter* SelectedParty = Game->GetSelectedPartyMember();
		const ExSkills* Skills = Game->GetSkillsModule();
		if( SelectedParty && SelectedParty->CanFight() && Skills )
		{
			ExSpellList SpellList;
			Skills->GetSkills( Game , SelectedParty , SpellList , ex_skills_get_type::AvailableOutOfCombat );
			return SpellList.Len() > 0;
		}
	}

	return false;
}

void ExCastSpellInWorldMenu::OnInit()
{
	Super::OnInit();

	// HideHUD();

	const ExGame* Game = GetGame();

	m_Party.Resize( ExRoster::PARTY_SIZE );
	if( Game )
	{
		for( eg_int i=0; i<m_Party.LenAs<eg_int>(); i++ )
		{
			const ExFighter* PartyMember = Game->GetPartyMemberByIndex( i );
			if( PartyMember )
			{
				m_Party[i].Combatant.InitCombatant( nullptr , *PartyMember , i , CT_Clear , i );
			}
		}
	}

	m_FadeWidget = GetWidget<EGUiWidget>( eg_crc("FadeWidget") );
	m_CharacterPortraitsGrid = GetWidget<ExCombatPartyWidget>( eg_crc("Portraits") );
	m_HelpBg = GetWidget<EGUiWidget>( eg_crc("HelpBg") );
	m_HelpText = GetWidget<ExHintBarWidget>( eg_crc("HelpText") );
	m_SpellInfoWidget = GetWidget<ExItemCardWidget>( eg_crc("SpellInfo") );

	if( m_CharacterPortraitsGrid )
	{
		m_CharacterPortraitsGrid->SetIsCastSpellInWorld( true );
		m_CharacterPortraitsGrid->SetEnabled( false );
		m_CharacterPortraitsGrid->SetUsingBigPortraits( false );
		m_CharacterPortraitsGrid->CellChangedDelegate.Bind( this , &ThisClass::OnPortraitCellChanged );
		m_CharacterPortraitsGrid->CellClickedDelegate.Bind( this , &ThisClass::OnPortraitClicked );
		m_CharacterPortraitsGrid->RefreshGridWidget( ExRoster::PARTY_SIZE );
		SetFocusedWidget( m_CharacterPortraitsGrid , Game ? Game->GetSelectedPartyMemberIndex() : -1 , false );
	}

	if( m_SpellInfoWidget )
	{
		m_SpellInfoWidget->RunEvent( eg_crc("HideNow") );
	}

	if( Game )
	{
		m_FighterIndex = Game->GetSelectedPartyMemberIndex();
		m_Fighter = Game->GetSelectedPartyMember();
		const ExSkills* Skills = Game->GetSkillsModule();

		ExChoiceWidgetMenu::exConfig ChoiceConfig;
		ChoiceConfig.bPushCascaded = true;
		ChoiceConfig.InitialSelection = 0;
		ChoiceConfig.bCanPop = true;

		Skills->GetSkills( Game , m_Fighter , m_SpellList , ex_skills_get_type::AvailableOutOfCombat );

		for( eg_uint i=0; i<m_SpellList.LenAs<eg_uint>(); i++ )
		{
			ChoiceConfig.Choices.Append( GetChoiceText( i ) );
		}

		ChoiceConfig.SetPoseForPortrait( m_FighterIndex , false );

		eg_transform SpellCardPose = ChoiceConfig.Pose;
		SpellCardPose.TranslateThis( 0.f , 44.f , 0.f ); // Cascade offset.
		SetSpellCardOffset( SpellCardPose );

		ExChoiceWidgetMenu* ChoiceMenu = ExChoiceWidgetMenu::PushMenu( this , ChoiceConfig );
		if( ChoiceMenu )
		{
			ChoiceMenu->ChoicePressedDelegate.Bind( this , &ThisClass::OnChoiceMade );
			ChoiceMenu->ChoiceSelectedDelegate.Bind( this , &ThisClass::OnChoiceHighlighted );
			ChoiceMenu->ChoiceAbortedDelegate.Bind( this , &ThisClass::OnChoiceAborted );
		}
	}

	PopulateHints();
}

void ExCastSpellInWorldMenu::OnDeinit()
{
	// ShowHUD();
	
	Super::OnDeinit();
}

void ExCastSpellInWorldMenu::OnActivate()
{
	Super::OnActivate();

	if( m_bDismissOnActivate )
	{
		m_bDismissOnActivate = false;
		DismissSpellMenu( false , true );
	}
	else if( m_SpellList.IsValidIndex( m_ChosenSpell ) )
	{
		if( DoesSpellNeedTarget() )
		{
			if( m_CharacterPortraitsGrid )
			{
				SetFocusedWidget( m_CharacterPortraitsGrid , 0 , false );
				m_CharacterPortraitsGrid->SetEnabled( true );
				PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED ); // Always play sound when choosing target even if it's the caster
			}
		}
		else
		{
			ProcessSpell();
		}
	}
	else
	{
		DismissSpellMenu( false , true );
	}

	PopulateHints();
}

void ExCastSpellInWorldMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( m_bCountingDownToClose )
	{
		m_CountdownTimer -= DeltaTime;

		if( m_CountdownTimer <= 0.f && IsActive() )
		{
			MenuStack_Pop();
		}
	}
}

eg_bool ExCastSpellInWorldMenu::OnInput( eg_menuinput_t InputType )
{
	if( m_bCountingDownToClose )
	{
		return true;
	}

	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		DismissSpellMenu( false , true );
		PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED ); // Play sound when canceling even if we are targeting caster
		return true;
	}

	return Super::OnInput( InputType );
}

void ExCastSpellInWorldMenu::OnInputCmds( const struct egLockstepCmds& Cmds )
{
	if( m_CharacterPortraitsGrid && m_CharacterPortraitsGrid->IsEnabled() )
	{
		auto ChangePartyMember = [this]( eg_bool bInc ) -> void
		{
			m_CharacterPortraitsGrid->HandleInput( bInc ? eg_menuinput_t::BUTTON_RIGHT : eg_menuinput_t::BUTTON_LEFT , CT_Clear , false );
		};

		if( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) )
		{
			ChangePartyMember( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) );
		}
	}
}

eg_loc_text ExCastSpellInWorldMenu::GetChoiceText( eg_uint Index ) const
{
	eg_loc_text Out = CT_Clear;

	if( m_SpellList.IsValidIndex( Index ) )
	{
		Out = EGFormat( eg_loc( "CombatSpellName" , "{0} ({1} MP)" ) , m_SpellList[Index]->Name , m_SpellList[Index]->GetManaCost( m_Fighter->GetMeleeAttrValue( ex_attr_t::MAG ) , m_Fighter->GetAttrValue( ex_attr_t::LVL ) ) );
	}

	return Out;
}

void ExCastSpellInWorldMenu::OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	eg_bool bCanCast = false;
	if( m_Fighter && m_SpellList.IsValidIndex(ChoiceIndex) && m_SpellList[ChoiceIndex] )
	{
		ex_attr_value AttackerMag = m_Fighter->GetMeleeAttrValue( ex_attr_t::MAG );
		ex_attr_value AttackerLvl = m_Fighter->GetAttrValue( ex_attr_t::LVL );
		ex_attr_value ManaCost = m_SpellList[ChoiceIndex]->GetManaCost( AttackerMag , AttackerLvl );
		if( ManaCost <= m_Fighter->GetMP() )
		{
			bCanCast = true;
			m_ChosenSpell = ChoiceIndex;
		}
		else
		{
			ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( eg_loc("InWorldCastNotEnoughMana","Not enough mana to cast this spell.") ) );
		}
	}

	if( bCanCast )
	{
		SetViewedSpell( -1 );
		MenuStack_Pop();
	}
}

void ExCastSpellInWorldMenu::OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex )
{
	unused( SourceMenu );

	SetViewedSpell( ChoiceIndex );
}

void ExCastSpellInWorldMenu::OnChoiceAborted( ExChoiceWidgetMenu* SourceMenu )
{
	unused( SourceMenu );

	SetViewedSpell( -1 );
}

void ExCastSpellInWorldMenu::OnPortraitCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
{
	if( ExCombatPartyWidget* CombatPartyWidget = EGCast<ExCombatPartyWidget>( CellInfo.Owner) )
	{
		CombatPartyWidget->OnGridWidgetItemChangedFromCastSpellInWorld( CellInfo , GetGame() );
	}
}

void ExCastSpellInWorldMenu::OnPortraitClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
{
	if( m_Party.IsValidIndex(CellInfo.GridIndex) && m_Party[CellInfo.GridIndex].Combatant.GetFighter().IsCreated() )
	{
		m_ChosenTarget = CellInfo.GridIndex;

		if( m_CharacterPortraitsGrid )
		{
			m_CharacterPortraitsGrid->SetEnabled( false );
		}

		ProcessSpell();
	}
	else
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
	}

	PopulateHints();
}

void ExCastSpellInWorldMenu::DismissSpellMenu( eg_bool bImmediateAndLoad , eg_bool bWasCancel )
{
	// Make sure focus is back on caster
	ExGame* Game = GetGame();
	if( m_CharacterPortraitsGrid )
	{
		m_CharacterPortraitsGrid->SetEnabled( false );
	}
	SetFocusedWidget( m_CharacterPortraitsGrid , Game ? Game->GetSelectedPartyMemberIndex() : -1 , false );
	PopulateHints();

	if( bImmediateAndLoad )
	{
		MenuStack_Clear();
		// MenuStack_PushTo( eg_crc("FullScreenLoadingMenu") );
	}
	else
	{
		m_bCountingDownToClose = true;
		m_CountdownTimer = bWasCancel ? ExCastSpellInWorldMenu_CountdownForCancel : ExCastSpellInWorldMenu_CountdownForCast;

		if( m_FadeWidget )
		{
			m_FadeWidget->RunEvent( eg_crc("FadeDarkenToClear") );
		}
	}
}

eg_bool ExCastSpellInWorldMenu::DoesSpellNeedTarget() const
{
	const ExGame* Game = GetGame();
	
	if( m_SpellList.IsValidIndex( m_ChosenSpell ) )
	{
		const exSpellInfo* SpellInfo = m_SpellList[m_ChosenSpell];
		if( SpellInfo && SpellInfo->CanTargetInWorld() && !SpellInfo->TargetsEveryoneInWorld() )
		{
			return SpellInfo->TargetCount == 1;
		}
	}

	return false;
}

void ExCastSpellInWorldMenu::ProcessSpell()
{
	ExGame* Game = GetGame();

	const exSpellInfo* Spell = m_SpellList.IsValidIndex(m_ChosenSpell) ? m_SpellList[m_ChosenSpell] : nullptr;

	ExCombatant* Caster = m_Party.IsValidIndex( m_FighterIndex ) ? &m_Party[m_FighterIndex].Combatant : nullptr;

	eg_bool bDismissMenuImmediatelyAndLoad = false;
	
	if( Caster && Spell )
	{
		Caster->SetAttackType( ex_attack_t::MELEE );
		ex_attr_value AttackerMag = Caster->GetAttrValue( ex_attr_t::MAG );
		ex_attr_value AttackerLvl = Caster->GetAttrValue( ex_attr_t::LVL );
		Caster->SetAttackType( ex_attack_t::NONE );
		ex_attr_value ManaCost = Spell->GetManaCost( AttackerMag , AttackerLvl);
		assert( ManaCost <= Caster->GetMP() ); // Menu should not have allowed this attack to happen.
		Caster->UseMP( ManaCost );

		exDamage Damage( CT_Clear );
		if( Spell->Type == ex_spell_t::CombatDamage || Spell->Type == ex_spell_t::Resurrect )
		{
			Damage = Spell->GetDamage( AttackerMag , AttackerLvl );
		}
		eg_bool bIsHeal = Spell->Type == ex_spell_t::CombatDamage && Damage.Physical < 0;


		EGArray<ExCombatant*> Targets;
		if( Spell->CanTargetInWorld() )
		{
			if( Spell->TargetsEveryoneInWorld() )
			{
				for( exFighterWrapper& Member : m_Party )
				{
					Targets.Append( &Member.Combatant );
				}
			}
			else
			{
				if( Spell->TargetCount == 1 )
				{
					if( m_Party.IsValidIndex( m_ChosenTarget ) )
					{
						Targets.Append( &m_Party[m_ChosenTarget].Combatant );
					}
				}
				else
				{
					for( eg_int i=0; i<Spell->TargetCount; i++ )
					{
						if( m_Party.IsValidIndex( i ) )
						{
							Targets.Append( &m_Party[i].Combatant );
						}
					}
				}
			}
		}

		switch( Spell->Type )
		{

		case ex_spell_t::Unknown:
		{
			assert( false ); // What is this?
		} break;

		case ex_spell_t::AttributeBoost:
		{
			assert( false ); // Attribute boost spells are not implemented outside of combat.
		} break;

		case ex_spell_t::CombatDamage:
		{
			ex_attr_value TotalHit = 0;
		
			for( ExCombatant* Target : Targets )
			{
				ex_attr_value Hit = bIsHeal ? Damage.Physical : (Damage - Target->GetRolledDefense( m_Rng ));
				if( !bIsHeal || !Target->IsDead() )
				{
					TotalHit += Hit;
				}
				if( !bIsHeal )
				{
					// Caster->AdjustAggression( Hit );
					if( !bIsHeal && (Target->GetHP() <= 0) ) // When a fighter with 0 hit points is hit, they are killed.
					{
						Target->SetDead( true );
						// DownedTargets.Append( Target );
					}
					else
					{
						Target->ApplyRawHit( Hit );
						if( Target->GetHP() <= 0 )
						{
							Target->SetUnconcious( true );
							// DownedTargets.Append( Target );
						}
					}

					exCombatAnimInfo AnimInfo;
					AnimInfo.Type = ex_combat_anim::SpellAttack;
					AnimInfo.Damage = Hit;
					AnimInfo.Attacker = Caster;
					AnimInfo.Defender = Target;
					HandleAnim( AnimInfo );
				}
				else
				{
					if( !Target->IsDead() )
					{
						Target->ApplyRawHeal( -Hit );
						if( Target->IsUnconcious() )
						{
							ExAchievements::Get().UnlockAchievement( eg_crc("ACH_AWAKE") );
						}
						Target->SetUnconcious( false );

						exCombatAnimInfo AnimInfo;
						AnimInfo.Type = ex_combat_anim::SpellBonus;
						AnimInfo.Damage = -Hit;
						AnimInfo.Attacker = Caster;
						AnimInfo.Defender = Target;
						HandleAnim( AnimInfo );
					}
					else
					{
						if( Targets.Len() == 1 )
						{
							exCombatAnimInfo AnimInfo;
							AnimInfo.Type = ex_combat_anim::SpellFail;
							AnimInfo.Damage = 0;
							AnimInfo.Attacker = Caster;
							AnimInfo.Defender = Target;
							HandleAnim( AnimInfo );
						}
					}
				}
			}

			if( bIsHeal )
			{
				if( Targets.Len() == 1 )
				{
					// AppendCombatLog( EGFormat( eg_loc("CombatHealSingle","restored |SC(EX_HEAL)|{1} HP|RC()| to {0:NAME}.") , Targets[0] , -TotalHit ) );
				}
				else
				{
					// AppendCombatLog( EGFormat( eg_loc("CombatHealMultiple","restored |SC(EX_HEAL)|{1} HP|RC()| to {0} fighters.") , Targets.Len() , -TotalHit ) );
				}
			}
			else
			{
				if( Targets.Len() == 1 )
				{
					// AppendCombatLog( EGFormat( eg_loc("CombatSpellDamageSingle","hit {0:NAME} damaging |SC(RED)|{1} HP|RC()|.") , Targets[0] , TotalHit ) );
				}
				else
				{
					// AppendCombatLog( EGFormat( eg_loc("CombatSpellDamageMultiple","hit {0} fighters damaging |SC(RED)|{1} HP|RC()|.") , Targets.Len() , TotalHit ) );
				}
			}

			/*
			if( DownedTargets.Len() == 1 )
			{
				// AppendCombatLog( EGFormat( eg_loc("GoesDownString","{0:NAME} goes down.") , DownedTargets[0] ) );
			}
			else if( DownedTargets.Len() > 1 )
			{
				// AppendCombatLog( EGFormat( eg_loc("CombatMultipleDownString","{0} fighters go down.") , DownedTargets.Len() ) );
			}
			*/
		} break;

		case ex_spell_t::Resurrect:
		{
			EGArray<ExCombatant*> RaisedFighters;

			for( ExCombatant* Target : Targets )
			{
				if( Target && Target->IsDead() )
				{
					Target->SetDead( false );
					Target->SetUnconcious( false );
					Target->ApplyRawHeal( EG_Abs( Damage.Physical ) );
					RaisedFighters.Append( Target );

					exCombatAnimInfo AnimInfo;
					AnimInfo.Type = ex_combat_anim::SpellBonus;
					AnimInfo.Damage = EG_Abs( Damage.Physical );
					AnimInfo.Attacker = Caster;
					AnimInfo.Defender = Target;
					HandleAnim( AnimInfo );
					ExAchievements::Get().UnlockAchievement( eg_crc("ACH_RESURRECT") );
				}
				else
				{
					if( Targets.Len() == 1 )
					{
						exCombatAnimInfo AnimInfo;
						AnimInfo.Type = ex_combat_anim::SpellFail;
						AnimInfo.Damage = 0;
						AnimInfo.Attacker = Caster;
						AnimInfo.Defender = Target;
						HandleAnim( AnimInfo );

						m_bDismissOnActivate = true;
						ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( eg_loc("InWorldCastResurrectFail","That character is not dead, and the |SC(EX_ATTR)|MP|RC()| was wasted.") ) );
					}
				}
			}

			if( RaisedFighters.Len() > 1 )
			{
				// AppendCombatLog( EGFormat( eg_loc("CombatSpellResurrectMultiple","resurrected {0} fighters.") , RaisedFighters.LenAs<eg_int>() ) );
			}
			else if( RaisedFighters.Len() == 1 )
			{
				// AppendCombatLog( EGFormat( eg_loc("CombatSpellResurrectSingleTarget","resurrected {0:NAME}.") , RaisedFighters[0] ) );
			}
			else
			{
				// AppendCombatLog( eg_loc_text(eg_loc("CombatSpellResurrectNoOne","there was no one to resurrect.")) );
			}

		} break;

		case ex_spell_t::TownPortalMarkAndGo:
		{
			if( GetGame()->GetCurrentMapInfo().bCanUseTownPortal )
			{
				PlaySoundEvent( ExUiSounds::ex_sound_e::TeleportParty );
				GetGame()->SDK_RunServerEvent( eg_crc("ServerDoTownPortal") , false );
				bDismissMenuImmediatelyAndLoad = true;
				ExAchievements::Get().UnlockAchievement( eg_crc("ACH_BEACON") );
			}
			else
			{
				exCombatAnimInfo AnimInfo;
				AnimInfo.Type = ex_combat_anim::SpellFail;
				AnimInfo.Damage = 0;
				AnimInfo.Attacker = Caster;
				AnimInfo.Defender = nullptr;
				HandleAnim( AnimInfo );

				m_bDismissOnActivate = true;
				ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( eg_loc("InWorldCastTownPortalNotAllowed","A mystical force prevents Lloyd's Beacon from opening.") ) );
			}
		} break;

		case ex_spell_t::TownPortalRecall:
		{
			PlaySoundEvent( ExUiSounds::ex_sound_e::TeleportParty );
			GetGame()->SDK_RunServerEvent( eg_crc("ServerDoTownPortal") , true );
			bDismissMenuImmediatelyAndLoad = true;
		} break;

		case ex_spell_t::FortifyRanks:
		case ex_spell_t::ExpandRanks:
		{
			assert( false ); // Not available out of combat.
		} break;
		}
	}

	if( Game )
	{
		for( eg_int i=0; i<m_Party.LenAs<eg_int>(); i++ )
		{
			m_Party[i].Combatant.UpdateFighterOnServer( Game );
		}
	}
	
	if( !m_bDismissOnActivate )
	{
		DismissSpellMenu( bDismissMenuImmediatelyAndLoad , false );
	}
}

void ExCastSpellInWorldMenu::HandleAnim( const exCombatAnimInfo& AnimInfo )
{
	if( m_CharacterPortraitsGrid )
	{
		m_CharacterPortraitsGrid->PlayAnimation( AnimInfo );
	}
}

void ExCastSpellInWorldMenu::PopulateHints()
{
	static const eg_string_crc ChooseTargetText = eg_loc("CastSpellInWorldChooseTargetText","Choose Target");

	if( m_HelpText )
	{
		m_HelpText->ClearHints();

		if( m_CharacterPortraitsGrid && m_CharacterPortraitsGrid->IsEnabled() )
		{
			m_HelpText->AddHint( CMDA_MENU_PRIMARY , eg_loc_text(ChooseTargetText) );
		}
	}

	if( m_HelpBg )
	{
		m_HelpBg->RunEvent( m_HelpText && m_HelpText->HasHints() ? eg_crc("ShowNow") : eg_crc("HideNow") );
	}
}

void ExCastSpellInWorldMenu::SetViewedSpell( eg_uint SpellChoice )
{
	if( m_SpellInfoWidget )
	{
		const exSpellInfo* SpellInfo = m_SpellList.IsValidIndex( SpellChoice ) ? m_SpellList[SpellChoice] : nullptr;

		if( m_Fighter && SpellInfo )
		{
			m_SpellInfoWidget->SetSpell( SpellInfo , m_Fighter , true );
			m_SpellInfoWidget->RunEvent( eg_crc("Show") );
		}
		else
		{
			m_SpellInfoWidget->RunEvent( eg_crc("HideNow") );
		}
	}
}

void ExCastSpellInWorldMenu::SetSpellCardOffset( const eg_transform& Transform )
{
	if( m_SpellInfoWidget )
	{
		m_SpellInfoWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , Transform );
	}
}

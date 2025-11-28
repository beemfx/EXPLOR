// (c) 2017 Beem Media

#include "ExMenu.h"
#include "ExMenuPortraitsGridWidget.h"
#include "ExDialogMenu.h"
#include "ExSkills.h"
#include "ExAchievements.h"

class ExTrainingMenu;

struct exTrainingMenuStats
{
	EGUiGridWidget* GridWidget = nullptr;
	EGArray<ex_attr_t> Attributes;

	void Initialize( ExTrainingMenu* OwnerMenuIn , eg_string_crc GridWidgetName , const EGArray<ex_attr_t>& AttributesIn );
};

struct exTrainingMenuFighterStats
{
	ExFighter PreTraining = CT_Clear;
	ExFighter CurrentState = CT_Clear;
};

class ExTrainingMenu : public ExMenu
{
	EG_CLASS_BODY( ExTrainingMenu , ExMenu )

private:

	ExMenuPortraitsGridWidget*   m_Portraits;
	EGUiTextWidget*              m_ReqText;
	EGUiTextWidget*              m_NameText;
	EGUiTextWidget*              m_ClassText;
	EGUiTextWidget*              m_XpText;
	EGUiTextWidget*              m_SpellsMorePowerfulText;
	EGUiTextWidget*              m_ProgressionHeaderText;
	exTrainingMenuFighterStats   m_PreTrainingStats[ExRoster::PARTY_SIZE];
	EGArray<exTrainingMenuStats> m_Stats;
	EGUiTextWidget*              m_NewSkillsText;
	ExFighter                    m_PreTrainedFighter = CT_Clear;
	ExFighter                    m_CurrentLevelFighter = CT_Clear;
	ExSpellList                  m_SkillsUnlocked;
	eg_bool                      m_bNeedsRefresh:1;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		{
			EGArray<ex_attr_t> AttrList;
			AttrList.Append( ex_attr_t::HP );
			AttrList.Append( ex_attr_t::MP );
			AttrList.Append( ex_attr_t::DMG_ );
			AttrList.Append( ex_attr_t::DEF_ );
			// AttrList.Append( ex_attr_t::SPD );
			// AttrList.Append( ex_attr_t::STR );
			// AttrList.Append( ex_attr_t::END );
			// AttrList.Append( ex_attr_t::MAG );
			m_Stats.Resize( m_Stats.Len() + 1 );
			exTrainingMenuStats& NewStats = m_Stats[m_Stats.Len()-1];
			NewStats.Initialize( this , eg_crc("BaseAttributes") , AttrList );
		}

		m_Portraits = GetWidget<ExMenuPortraitsGridWidget>( eg_crc("Portraits") );
		m_ReqText = GetWidget<EGUiTextWidget>( eg_crc("ReqText") );
		m_NameText = GetWidget<EGUiTextWidget>( eg_crc("NameText") );
		m_ClassText = GetWidget<EGUiTextWidget>( eg_crc("ClassText") );
		m_XpText = GetWidget<EGUiTextWidget>( eg_crc("XpText") );
		m_SpellsMorePowerfulText = GetWidget<EGUiTextWidget>( eg_crc("SpellsMorePowerfulText") );
		m_NewSkillsText = GetWidget<EGUiTextWidget>( eg_crc("NewSkillsText") );
		m_ProgressionHeaderText = GetWidget<EGUiTextWidget>( eg_crc("BoostsForNextLevelText") );

		if( m_Portraits )
		{
			m_Portraits->SetEnabled( true );
			SetFocusedWidget( m_Portraits->GetId() , 0 , false );
			m_Portraits->SetSelection( GetGame()->GetSelectedPartyMemberIndex() );
			m_Portraits->SetFocusOnHover( true );
			m_Portraits->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnSelectedPortraitChanged );
		}

		// Store all fighter states:
		for( eg_int i=0; i<countof(m_PreTrainingStats); i++ )
		{
			if( GetGame()->GetPartyMemberByIndex( i ) )
			{
				m_PreTrainingStats[i].PreTraining = *GetGame()->GetPartyMemberByIndex( i );
				m_PreTrainingStats[i].CurrentState = m_PreTrainingStats[i].PreTraining;
			}
		}

		Refresh();
		DoReveal();
	}

	virtual void OnRevealComplete() override
	{
		Super::OnRevealComplete();

		Refresh();
	}

	virtual bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		if( m_bNeedsRefresh )
		{
			return true;
		}

		return Super::OnInput( InputType );
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds )
	{
		if( Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) )
		{
			ChangePartyMember( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) );
		}
	}

	void ChangePartyMember( eg_bool bInc )
	{
		if( m_Portraits )
		{
			m_Portraits->ChangeSelection( bInc );
		} 
	}

	void OnAttributeCellChanged( egUiGridWidgetCellChangedInfo& CellInfo )
	{
		for( exTrainingMenuStats& Stats : m_Stats )
		{
			if( Stats.GridWidget && Stats.GridWidget == CellInfo.Owner )
			{
				if( Stats.Attributes.IsValidIndex( CellInfo.GridIndex ) )
				{
					PopulateAttributeCell( CellInfo.GridItem , Stats.Attributes[CellInfo.GridIndex] );
				}
			}
		}
	}

	void PopulateAttributeCell( EGUiGridWidgetItem* Cell , ex_attr_t AttrType )
	{
		if( Cell )
		{
			ex_attr_value PreValue = m_PreTrainedFighter.GetAttrValue( AttrType );
			ex_attr_value CurValue = m_CurrentLevelFighter.GetAttrValue( AttrType );
			Cell->SetText( eg_crc("NameText") , EGFormat(L"|SC(EX_ATTR)|{0}|RC()|" , ExCore_GetAttributeName( AttrType )) );
			if( PreValue == CurValue )
			{
				Cell->SetText( eg_crc("ValueText") , EGFormat( L"{0}" , CurValue ) );
			}
			else
			{
				Cell->SetText( eg_crc("ValueText") , EGFormat( eg_loc("TrainingMenuAttrValueUp","{0} |SC(EX_HEAL)|(+ {1})|RC()|") , CurValue , CurValue - PreValue ) );
			}
		}
	}

	void OnSelectedPortraitChanged( EGUiGridWidget* GridOwner , eg_uint CellIndex )
	{
		unused( GridOwner );

		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		
		OnSelectedCharacterChanged( CellIndex );
	}

	void OnObjPressed( const egUIWidgetEventInfo& Info )
	{
		if( m_Portraits && Info.WidgetId == m_Portraits->GetId() )
		{
			AttemptToTrain( Info.GridIndex );
		}
	}

	virtual void Refresh() override final
	{
		m_bNeedsRefresh = false;

		if( m_Portraits )
		{
			m_Portraits->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
			m_Portraits->RefreshGridWidget( ExRoster::PARTY_SIZE );
			OnSelectedCharacterChanged( m_Portraits->GetSelectedIndex() );
		}
	}

	void RefreshStatsGrids()
	{
		for( exTrainingMenuStats& Stats : m_Stats )
		{
			if( Stats.GridWidget )
			{
				Stats.GridWidget->RefreshGridWidget( Stats.Attributes.LenAs<eg_uint>() );
			}
		}
	}

	void AttemptToTrain( eg_uint PartyIndex )
	{
		if( GetGame() )
		{
			ExFighter* PartyMember = GetGame()->GetPartyMemberByIndex( PartyIndex );
			if( PartyMember  )
			{
				eg_loc_text TrainingText;

				ex_attr_value Level = PartyMember->GetAttrValue( ex_attr_t::LVL );
				const eg_bool bMaxLevel = Level >= EX_MAX_LEVEL;
				eg_int64 CurXp = PartyMember->GetXP();
				eg_int64 XpToNext = ExCore_GetXpNeededForLevel( static_cast<ex_attr_value>( Level + 1 ) );
				eg_int64 XpNeeded = XpToNext - CurXp;

				if( bMaxLevel )
				{
					ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat(eg_loc("TrainingMenuMaxLevel","{0:NAME} has reached the maximum |SC(EX_ATTR)|LVL|RC()| and cannot train further.") , *PartyMember ) ) );
				}
				else if( !PartyMember->IsInConditionToTrainToNextLevel() )
				{
					ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat(eg_loc("TrainingMenuNeedsHealing","{0:NAME} is not in a condition to train to |SC(EX_ATTR)|LVL|RC()| {1}. Good health is required to train.") , *PartyMember , Level + 1 ) ) );
				}
				else if( !PartyMember->CanTrainToNextLevel() )
				{
					ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat(eg_loc("TrainingMenuNotEnoughXP","{0:NAME} does not have enough experience to train to |SC(EX_ATTR)|LVL|RC()| {1}.") , *PartyMember , Level + 1 ) ) );
				}
				else
				{
					ex_attr_value GoldNeeded = ExCore_GetCostToTrainToLevel( Level + 1 );
					ex_attr_value PartyGold = GetGame()->SmGetVar( eg_crc("PartyGold") ).as_int();
					if( PartyGold < GoldNeeded )
					{
						ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( EGFormat(eg_loc("TrainingMenuNotEnoughGold","|SC(EX_GOLD)|{2} gold|RC()| is needed to train {0:NAME} to |SC(EX_ATTR)|LVL|RC()| {1}.") , *PartyMember , Level + 1 , GoldNeeded ) ) );
					}
					else
					{
						m_bNeedsRefresh = true;
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::LEVEL_UP );
						ExAchievements::Get().UnlockAchievement( eg_crc("ACH_TRAINER") );
						GetGame()->SDK_RunServerEvent( eg_crc("TrainPartyMember") , PartyIndex );
					}
				}
			}
		}
	}

	void OnSelectedCharacterChanged( eg_uint PartyIndex )
	{
		if( GetGame() )
		{
			const ExFighter* PreTrainingPartyMember = &m_PreTrainingStats[PartyIndex].PreTraining;
			const ExFighter* PartyMember = GetGame()->GetPartyMemberByIndex( PartyIndex );
			if( PartyMember && PreTrainingPartyMember && m_ReqText && m_NameText && m_ClassText )
			{
				const eg_bool bMaxLevel = PartyMember->GetAttrValue( ex_attr_t::LVL ) >= EX_MAX_LEVEL;

				m_PreTrainedFighter = *PreTrainingPartyMember;
				m_PreTrainedFighter.RestoreAllHP();
				m_PreTrainedFighter.RestoreAllMP();
				m_PreTrainedFighter.ResolveReplicatedData();
				m_PreTrainedFighter.SetAttackType( ex_attack_t::MELEE );
				m_PreTrainedFighter.ResolveEquipmentBoosts();

				m_CurrentLevelFighter = *PartyMember;
				m_CurrentLevelFighter.ResolveReplicatedData();
				m_CurrentLevelFighter.RestoreAllHP();
				m_CurrentLevelFighter.RestoreAllMP();	
				m_CurrentLevelFighter.SetAttackType( ex_attack_t::MELEE );
				m_CurrentLevelFighter.ResolveEquipmentBoosts();

				eg_loc_text TrainingText;

				const ex_attr_value Level = PartyMember->GetAttrValue( ex_attr_t::LVL );
				eg_int64 CurXp = PartyMember->GetXP();
				eg_int64 XpToNext = ExCore_GetXpNeededForLevel( static_cast<ex_attr_value>( Level + 1 ) );
				eg_int64 XpNeeded = XpToNext - CurXp;
				ex_attr_value GoldNeeded = ExCore_GetCostToTrainToLevel( Level + 1 );

				if( bMaxLevel )
				{
					TrainingText = EGFormat( eg_loc("TrainingMenuMaxLevelText","{0:NAME} has reached the maximum |SC(EX_ATTR)|LVL|RC()|.") , *PartyMember );
				}
				else if( !PartyMember->CanTrainToNextLevel() )
				{
					TrainingText = EGFormat( eg_loc("TrainingMenuXPToNextLevelText","{1} |SC(EX_ATTR)|XP|RC()| needed to train to |SC(EX_ATTR)|LVL|RC()| {2}.") , PartyMember->LocName , XpNeeded , Level + 1 );
				}
				else
				{
					TrainingText = EGFormat( eg_loc("TrainingMenuEligibleToTrainText","The cost is |SC(EX_GOLD)|{0} gold|RC()| to train to |SC(EX_ATTR)|LVL|RC()| {2}.") , GoldNeeded , PartyMember->LocName , Level + 1 );
				}

				m_ReqText->SetText( CT_Clear , TrainingText );

				m_NameText->SetText( CT_Clear , EGFormat( eg_loc("TrainingMenuNameText","{0:NAME}") , *PartyMember ) );
				m_ClassText->SetText( CT_Clear , EGFormat( eg_loc("TraningMenuClassText","LVL {0:ATTR:LVL} {0:CLASS}") , *PartyMember ) ); 
			
				if( m_XpText )
				{
					m_XpText->SetText( CT_Clear , EGFormat(eg_loc("TrainingMenuXpText","{0:XP} |SC(EX_ATTR)|XP|RC()|") , *PartyMember ) );
				}

				const ex_attr_value PreLevel = m_PreTrainedFighter.GetAttrValue( ex_attr_t::LVL );
				const ex_attr_value CurrentLevel = m_CurrentLevelFighter.GetAttrValue( ex_attr_t::LVL );

				m_SkillsUnlocked.Clear();
				if( PreLevel != CurrentLevel )
				{
					ExSkills* SkillsModule = GetGame()->GetSkillsModule();
					if( SkillsModule )
					{
						for( ex_attr_value LvlIdx = PreLevel+1; LvlIdx <= CurrentLevel; LvlIdx++ )
						{
							SkillsModule->GetSkills( GetGame() , &m_CurrentLevelFighter , m_SkillsUnlocked , ex_skills_get_type::UnlockingAtLevel , LvlIdx );
						}
					}
				}

				if( m_ProgressionHeaderText )
				{
					eg_loc_text ProgressionHeader;

					if( PreLevel != CurrentLevel )
					{
						ProgressionHeader = EGFormat( eg_loc("TrainingMenuProgressionHeaderLeveledUp","Progression from LVL {0} to LVL {1} (Current Equipment)") , PreLevel , CurrentLevel );
					}
					else if( false )// bMaxLevel )
					{
						ProgressionHeader = EGFormat( eg_loc("TrainingMenuProgressionHeaderMaxed","Maximum LVL Attained") , Level );
					}
					else
					{
						ProgressionHeader = EGFormat( eg_loc("TrainingMenuProgressionHeaderCurrentLevel","Progression at LVL {0} (Current Equipment)") , Level );
					}

					m_ProgressionHeaderText->SetText( CT_Clear , ProgressionHeader );
				}

				{
					auto SetVisibleBasedOnMaxLevel = [this,&bMaxLevel]( eg_string_crc WidgetId ) -> void
					{
						EGUiWidget* Widget = GetWidget<EGUiWidget>( WidgetId );
						if( Widget )
						{
							Widget->SetVisible( !bMaxLevel );
						}
					};

					SetVisibleBasedOnMaxLevel( eg_crc("NewSkillsHeader") );
					EGUiTextWidget* AttributesHeaderTextWidget = GetWidget<EGUiTextWidget>( eg_crc("BaseAttributesHeader") );
					if( AttributesHeaderTextWidget )
					{
						AttributesHeaderTextWidget->SetText( CT_Clear , eg_loc_text( bMaxLevel ? eg_loc("TrainingMenuAttributesHeaderMaxLevel","Attributes") : eg_crc("TrainingMenuAttributeChangesHeader") ) );
					}
				}

				if( m_NewSkillsText )
				{
					eg_d_string16 SkillsString;
					for( const exSpellInfo* SkillInfo : m_SkillsUnlocked )
					{
						if( SkillInfo )
						{
							eg_loc_text SkillName = EGFormat( L"|SC(EX_ITEM)|{0:NAME}|RC()|" , SkillInfo );
							if( SkillsString.Len() > 0 )
							{
								SkillsString.Append( L'\n' );
							}
							SkillsString.Append( SkillName.GetString() );
						}
					}

					if( SkillsString.Len() == 0 )
					{
						SkillsString.Append( eg_loc_text(eg_loc("NoNewSkillsText","None")).GetString() );
					}

					m_NewSkillsText->SetText( CT_Clear , eg_loc_text( *SkillsString ) );
				}

				if( m_SpellsMorePowerfulText )
				{
					const eg_bool bVisible = PreLevel != CurrentLevel && (m_CurrentLevelFighter.GetClass() == ex_class_t::Cleric || m_CurrentLevelFighter.GetClass() == ex_class_t::Mage );
					m_SpellsMorePowerfulText->SetVisible( bVisible );
				}
			}
		}

		RefreshStatsGrids();

		RefreshHints();

		RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , PartyIndex ) );
	}

	eg_bool CanTrain( ExFighter* PartyMember )
	{
		eg_bool bCanTrain = PartyMember && PartyMember->CanTrainToNextLevel();

		if( bCanTrain )
		{
			ex_attr_value Level = PartyMember->GetAttrValue( ex_attr_t::LVL );
			ex_attr_value GoldNeeded = ExCore_GetCostToTrainToLevel( Level + 1 );
			ex_attr_value PartyGold = GetGame()->SmGetVar( eg_crc("PartyGold") ).as_int();
			bCanTrain = GoldNeeded <= PartyGold;
		}

		return bCanTrain;
	}

	void RefreshHints()
	{
		if( GetGame() )
		{
			eg_uint PartyIndex = m_Portraits ? m_Portraits->GetSelectedIndex() : 0;
			ExFighter* PartyMember = GetGame()->GetPartyMemberByIndex( PartyIndex );

			ClearHints();
			if( CanTrain( PartyMember ) )
			{
				AddHint( CMDA_MENU_PRIMARY , eg_loc_text( eg_loc("TrainingMenuTrainHint","Train") ) );
			}
			AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
			AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text(eg_loc("TrainingMenuNextParty","Switch Character")) );
			AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("TrainingMenuBackOutText","Back")) );
		}
	}
};

void exTrainingMenuStats::Initialize( ExTrainingMenu* OwnerMenuIn , eg_string_crc GridWidgetName , const EGArray<ex_attr_t>& AttributesIn )
{
	Attributes = AttributesIn;
	GridWidget = OwnerMenuIn ? OwnerMenuIn->GetWidget<EGUiGridWidget>( GridWidgetName ) : nullptr;
	if( GridWidget )
	{
		GridWidget->SetEnabled( false );
		GridWidget->CellChangedDelegate.Bind( OwnerMenuIn , &ExTrainingMenu::OnAttributeCellChanged );
	}
}

EG_CLASS_DECL( ExTrainingMenu )

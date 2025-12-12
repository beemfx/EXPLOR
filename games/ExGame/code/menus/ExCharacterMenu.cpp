// (c) 2016 Beem Media

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "EGTextFormat.h"
#include "ExPortraits.h"
#include "ExCharacterPortraitWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiImageWidget.h"
#include "EGInputTypes.h"

class ExCharacterMenu: public ExMenu
{
	EG_CLASS_BODY( ExCharacterMenu , ExMenu )

private:

	ExFighter m_ViewCharacter;

	EGUiGridWidget*             m_BaseAGrid;
	EGUiGridWidget*             m_DmgAGrid;
	EGUiGridWidget*             m_DefAGrid;
	EGUiGridWidget*             m_MiscAGrid;
	EGUiTextWidget*             m_AttrDescText;
	ExCharacterPortraitWidget2* m_PortraitWidget2;
	EGUiImageWidget*            m_LevelUpIcon;

	EGArray<ex_attr_t> m_BaseAList;
	EGArray<ex_attr_t> m_DmgAList;
	EGArray<ex_attr_t> m_DefAList;
	EGArray<ex_attr_t> m_MiscAList;

	eg_int  m_ViewedPartyMemberIndex = 0;
	eg_bool m_bViewedFromRoster = false;

public:

	ExCharacterMenu(): ExMenu(), m_ViewCharacter( CT_Clear ){ }

	virtual void OnInit() override final
	{
		Super::OnInit();

		HideHUD();

		m_BaseAList.Clear();
		m_DmgAList.Clear();
		m_DefAList.Clear();
		m_MiscAList.Clear();

		m_BaseAList.Append( ex_attr_t::LVL );
		m_BaseAList.Append( ex_attr_t::HP );
		m_BaseAList.Append( ex_attr_t::MP );
		m_BaseAList.Append( ex_attr_t::STR );
		m_BaseAList.Append( ex_attr_t::MAG );
		m_BaseAList.Append( ex_attr_t::END );
		m_BaseAList.Append( ex_attr_t::SPD );
		//m_BaseAList.Append( ex_attr_t::ACC );
		m_BaseAList.Append( ex_attr_t::AGR );

		{
			m_DmgAList.Append( ex_attr_t::DMG_ );
			m_DmgAList.Append( ex_attr_t::PDMG );
			m_DmgAList.Append( ex_attr_t::FDMG );
			m_DmgAList.Append( ex_attr_t::WDMG );
			m_DmgAList.Append( ex_attr_t::EDMG );
			m_DmgAList.Append( ex_attr_t::ADMG );

			m_DefAList.Append( ex_attr_t::DEF_ );
			m_DefAList.Append( ex_attr_t::PDEF );
			m_DefAList.Append( ex_attr_t::FDEF );
			m_DefAList.Append( ex_attr_t::WDEF );
			m_DefAList.Append( ex_attr_t::EDEF );
			m_DefAList.Append( ex_attr_t::ADEF );

			m_MiscAList.Append( ex_attr_t::AGR );
			// m_MiscAList.Append( ex_attr_t::HIT );
			// m_MiscAList.Append( ex_attr_t::DG );
			// m_MiscAList.Append( ex_attr_t::ATK );
		}

		m_BaseAGrid = GetWidget<EGUiGridWidget>( eg_crc("BaseAttributes") );
		m_DmgAGrid = GetWidget<EGUiGridWidget>( eg_crc("DmgAttributes") );
		m_DefAGrid = GetWidget<EGUiGridWidget>( eg_crc("DefAttributes") );
		m_MiscAGrid = GetWidget<EGUiGridWidget>( eg_crc("MiscAttributes") );
		m_AttrDescText = GetWidget<EGUiTextWidget>( eg_crc("AttrDescText") );
		m_LevelUpIcon = GetWidget<EGUiImageWidget>( eg_crc("LevelUpIcon") );

		RefreshGrids();

		if( GetGame() )
		{
			m_ViewedPartyMemberIndex = GetGame()->GetSelectedPartyMemberIndex();
			ExFighter* Fighter = GetGame()->GetPartyMemberByIndex( m_ViewedPartyMemberIndex );
			if( Fighter )
			{
				InitFighter( *Fighter , false , m_ViewedPartyMemberIndex );
			}
		}

		if( m_BaseAGrid )
		{
			SetFocusedWidget( m_BaseAGrid , 0 , false );
		}

		DoReveal();
	}

	virtual void OnDeinit() override final
	{
		Super::OnDeinit();
		ShowHUD( false );
	}

	virtual void OnRevealComplete() override
	{
		Super::OnRevealComplete();

		if( m_LevelUpIcon )
		{
			m_LevelUpIcon->SetVisible( m_ViewCharacter.CanTrainToNextLevel() );
		}
	}

	void InitFighter( const ExFighter& Fighter , eg_bool bViewFromRoster , eg_int PartyIndex )
	{
		m_bViewedFromRoster = bViewFromRoster;
		m_ViewedPartyMemberIndex = PartyIndex;
		
		m_ViewCharacter = Fighter;
		m_ViewCharacter.SetAttackType( ex_attack_t::MELEE );

		const ex_attr_value Level = m_ViewCharacter.GetAttrValue( ex_attr_t::LVL );
		const eg_int64 CurXp = m_ViewCharacter.GetXP();
		const eg_int64 XpToNext = ExCore_GetXpNeededForLevel( static_cast<ex_attr_value>( Level + 1 ) );
		const eg_int64 XpNeeded = XpToNext - CurXp;
		const eg_bool bMaxLevel = Level >= EX_MAX_LEVEL;
		const eg_bool bIsInCombat = GetGame() && GetGame()->IsInCombat();
		const eg_bool bCanTrain = m_ViewCharacter.CanTrainToNextLevel();

		EGUiWidget* TitleText = GetWidget( eg_crc("TitleText") );
		if( TitleText )
		{
			TitleText->SetText( eg_crc("") , EGFormat( eg_loc("CharacterMenuTitle","{0:NAME}") , m_ViewCharacter ) );
		}

		EGUiWidget* ClassText = GetWidget( eg_crc("ClassText") );
		if( ClassText )
		{
			ClassText->SetText( eg_crc( "" ), EGFormat( eg_loc("CharacterMenuLevelText","LVL {0:ATTR:LVL} {0:CLASS}"), m_ViewCharacter ) );
		}

		m_PortraitWidget2 = EGCast<ExCharacterPortraitWidget2>( GetWidget( eg_crc( "CharacterPortrait" ) ) );
		if( m_PortraitWidget2 )
		{
			m_PortraitWidget2->InitPortrait( &m_ViewCharacter, 0, nullptr );
		}

		if( m_LevelUpIcon )
		{
			m_LevelUpIcon->SetVisible( bCanTrain );
		}

		if( EGUiWidget* XpText = GetWidget( eg_crc("XpText") ) )
		{
			eg_loc_text TrainingText;
			if( bMaxLevel )
			{
				TrainingText = EGFormat( eg_loc("CharacterMenuMaxLevelText","Maximum |SC(EX_ATTR)|LVL|RC()|.") );
			}
			else if( bCanTrain )
			{
				TrainingText = EGFormat( eg_loc("CharacterMenuEligibleToTrainText","Eligible to train to |SC(EX_ATTR)|LVL|RC()| {0}.") , Level + 1 );
			}
			else
			{
				TrainingText = EGFormat( eg_loc("CharacterMenuXPToNextLevelText","{0} |SC(EX_ATTR)|XP|RC()| needed to train to |SC(EX_ATTR)|LVL|RC()| {1}.") , XpNeeded , Level + 1 );
			}
			XpText->SetText( CT_Clear , TrainingText );
		}
		if( EGUiTextWidget* XpTotalText = GetWidget<EGUiTextWidget>( eg_crc("XpTotalText") ) )
		{
			XpTotalText->SetText( CT_Clear , EGFormat( eg_loc("CharacterMenuTotalXP","{0:XP} |SC(EX_ATTR)|XP|RC()|") , m_ViewCharacter ) );
		}
		if( EGUiTextWidget* PartyPositionText = GetWidget<EGUiTextWidget>( eg_crc("PartyPositionText") ) )
		{
			if( EG_IsBetween<eg_int>( m_ViewedPartyMemberIndex , 0 , ExRoster::PARTY_SIZE-1 ) )
			{
				PartyPositionText->SetText( CT_Clear , EGFormat( bIsInCombat ? eg_loc("CharacterMenuCombatPosition","Combat Slot {0}") : eg_loc("CharacterMenuPartyPosition","Party Slot {0}") , m_ViewedPartyMemberIndex+1 ) );
			}
			else
			{
				PartyPositionText->SetText( CT_Clear , eg_loc_text(eg_loc("CharacterMenuInInnText","Visiting Inn")) );
			}
		}

		RefreshHints();
		RefreshGrids();
	}

	void RefreshHints()
	{
		ClearHints();
		if( !m_bViewedFromRoster )
		{
			AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
			AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text(eg_loc("CharacterMenuNextParty","Switch Character")) );
		}
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("BackOutText","Back")) );
	}

	void ChangePartyMember( eg_bool bInc )
	{
		if( !m_bViewedFromRoster )
		{
			const ExGame* Game = GetGame();
			if( Game )
			{
				const eg_bool bInCombat = Game->IsInCombat();
				const eg_int GridSize = ExRoster::PARTY_SIZE;
				const eg_int OldSelection = m_ViewedPartyMemberIndex;
				eg_int NewSelection = OldSelection;
				for( eg_int i=1; i<GridSize; i++ )
				{
					const eg_int CompareSelection = (OldSelection + i*(bInc?1:-1) + GridSize)%GridSize;
					if( bInCombat )
					{
						if( Game->GetPartyMemberInCombatByIndex( CompareSelection ) != nullptr )
						{
							NewSelection = CompareSelection;
							break;
						}
					}
					else
					{
						if( Game->GetPartyMemberByIndex( CompareSelection ) != nullptr )
						{
							NewSelection = CompareSelection;
							break;
						}
					}
				}

				if( NewSelection != OldSelection )
				{
					const ExFighter* Fighter = nullptr;
					if( bInCombat )
					{
						Fighter = Game->GetPartyMemberInCombatByIndex( NewSelection );
					}
					else
					{
						Fighter = Game->GetPartyMemberByIndex( NewSelection );
					}
					if( Fighter )
					{
						m_ViewedPartyMemberIndex = NewSelection;
						InitFighter( *Fighter , false , m_ViewedPartyMemberIndex );
						ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
					}
				}
			}
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		return false;
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds )
	{
		if( HandlePossibleMenuToggle( ExMenu::ex_toggle_menu_t::Character , Cmds ) )
		{
			return;
		}
				
		if( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) )
		{
			ChangePartyMember( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) );
		}
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		const EGArray<ex_attr_t>* List = nullptr;

		switch_crc(ItemInfo.WidgetId)
		{
		case_crc("BaseAttributes"):
			List = &m_BaseAList;
			break;
		case_crc("DmgAttributes"):
			List = &m_DmgAList;
			break;
		case_crc("DefAttributes"):
			List = &m_DefAList;
			break;
		case_crc("MiscAttributes"):
			List = &m_MiscAList;
			break;
		}

		if( List )
		{
			// (eg_crc("BaseAttributes") == ItemInfo.WidgetId) ? m_BaseAList : m_CompAList;
			const ex_attr_t Type = (*List)[ItemInfo.GridIndex];

			if( Type == ex_attr_t::XP_ )
			{
				ItemInfo.Widget->SetText( eg_crc("NameText") , CT_Clear );
				ItemInfo.Widget->SetText( eg_crc("ValueText") , CT_Clear );
			}
			else
			{
				const eg_d_string16 AttributeNameFormatStr = L"|SC(EX_ATTR)|{0}|RC()|"; // EGSFormat16( L"|SC({0})|\\{0\\}|RC()|" , ExCore_GetAttributeNameFormatColor( Type ) );
				ItemInfo.Widget->SetText( eg_crc("NameText") , EGFormat( *AttributeNameFormatStr , ExCore_GetAttributeName( Type ) ) );
				if( Type == ex_attr_t::HP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_ViewCharacter.GetHP()) , static_cast<eg_int>(m_ViewCharacter.GetAttrValue( Type )) ) );
				}
				else if( Type == ex_attr_t::MP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_ViewCharacter.GetMP()) , static_cast<eg_int>(m_ViewCharacter.GetAttrValue( Type )) ) );
				}
				else
				{
					const ex_attr_value FinalValue = m_ViewCharacter.GetAttrValue( Type , ex_fighter_get_attr_t::FinalValue );
					const ex_attr_value BoostValue = m_ViewCharacter.GetAttrValue( Type , ex_fighter_get_attr_t::CombatBoosts );

					eg_d_string16 AttributeValueFormatStr = EGSFormat16( L"|SC({0})|\\{0\\}|RC()|" , ExCore_GetAttributeValueFormatColor( Type ) );
					if( BoostValue != 0 )
					{
						if( Type == ex_attr_t::AGR )
						{
							AttributeValueFormatStr = BoostValue > 0 ? L"|SC(EX_HURT)|{0} (+{1})|RC()|" : L"|SC(EX_HEAL)|{0} ({1})|RC()|";
						}
						else
						{
							AttributeValueFormatStr = BoostValue > 0 ? L"|SC(EX_HEAL)|{0} (+{1})|RC()|" : L"|SC(EX_HURT)|{0} ({1})|RC()|";
						}
					}
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( *AttributeValueFormatStr , static_cast<eg_int>(FinalValue) , static_cast<eg_int>(BoostValue) ) );
				}

				ItemInfo.Widget->RunEvent( ItemInfo.IsNewlySelected() ? eg_crc("SelectBox") : eg_crc("Deselect") );

				if( ItemInfo.IsNewlySelected() )
				{
					if( m_AttrDescText )
					{
						eg_d_string16 AttributeNameFormatStr = EGSFormat16( L"|SC({0})|\\{0\\}|RC()|" , ExCore_GetAttributeNameFormatColor( Type ) );
						eg_loc_text AttrNameText = EGFormat( *AttributeNameFormatStr , ExCore_GetAttributeLongName( Type ) );
						eg_loc_text DescText = EGFormat( L"|SC(EX_ATTR)|{1}|RC()|: {2}" , ExCore_GetAttributeName( Type ) , AttrNameText , ExCore_GetAttributeDesc( Type ) );
						m_AttrDescText->SetText( eg_crc("") , DescText );
					}
				}
			}
		}
	}

	void RefreshGrids()
	{
		auto RefreshGridWidget = [this]( eg_string_crc GridId , eg_uint Size ) -> void
		{
			EGUiGridWidget* GridWidget = GetWidget<EGUiGridWidget>( GridId );
			if( GridWidget )
			{
				GridWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
				GridWidget->RefreshGridWidget( Size );
			}
		};

		RefreshGridWidget( eg_crc("BaseAttributes") ,m_BaseAList.LenAs<eg_uint>() );
		RefreshGridWidget( eg_crc("DmgAttributes") , m_DmgAList.LenAs<eg_uint>() );
		RefreshGridWidget( eg_crc("DefAttributes") , m_DefAList.LenAs<eg_uint>() );
		RefreshGridWidget( eg_crc("MiscAttributes") , m_MiscAList.LenAs<eg_uint>() );

		if( m_BaseAGrid )
		{
			eg_uint OldSelection = m_BaseAGrid->GetSelectedIndex();
			SetFocusedWidget( m_BaseAGrid , -1 , false );
			SetFocusedWidget( m_BaseAGrid , OldSelection , false );
		}

	}
};

EG_CLASS_DECL( ExCharacterMenu )

void ExCharacterMenu_ViewFighter( class EGClient* Client, const class ExFighter& Fighter , eg_bool bRosterCharacter , eg_int PartyIndex )
{
	if( Client )
	{
		ExCharacterMenu* CharacterMenu = EGCast<ExCharacterMenu>( Client->SDK_PushMenu( eg_crc("CharacterMenu") ) );
		if( CharacterMenu )
		{
			CharacterMenu->InitFighter( Fighter , bRosterCharacter , PartyIndex );
		}
	}
}
// (c) 2020 Beem Media. All Rights Reserved.


#include "ExMenu.h"
#include "EGUiGridWidget.h"
#include "ExBeastiary.h"
#include "EGEngine.h"
#include "ExDebugCombatLauncherMenu.h"

static EGSettingsInt ExDebugMonsterViewerMenu_LastViewedMonster( "DebugMonsterViewerMenu.LastViewedMonster" , CT_Clear , 0 , EGS_F_USER_SAVED );

class ExDebugMonsterViewerMenu : public ExMenu
{
	EG_CLASS_BODY( ExDebugMonsterViewerMenu , ExMenu )

private:

	ExFighter m_ViewMonster;
	eg_int    m_ViewedMonsterIndex = 0;
	eg_int    m_ViewedMonsterLevel = 1;

	EGUiGridWidget* m_BaseAGrid;
	EGUiGridWidget* m_DmgAGrid;
	EGUiGridWidget* m_DefAGrid;
	EGUiGridWidget* m_MiscAGrid;
	EGUiTextWidget* m_AttrDescText;
	EGUiMeshWidget* m_MonsterPortrait;
	EGUiTextWidget* m_ClassText;
	EGUiTextWidget* m_MoreInfoText;
	EGUiTextWidget* m_TestAgainstText;

	EGArray<ex_attr_t> m_BaseAList;
	EGArray<ex_attr_t> m_DmgAList;
	EGArray<ex_attr_t> m_DefAList;
	EGArray<ex_attr_t> m_MiscAList;
	EGArray<eg_string_crc> m_MonsterIdList;

	ex_class_t m_TestAgainstClass = ex_class_t::Warrior;

public:

	ExDebugMonsterViewerMenu()
	: ExMenu()
	, m_ViewMonster( CT_Clear )
	{
	
	}

	virtual void OnInit() override final
	{
		Super::OnInit();

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
		m_BaseAList.Append( ex_attr_t::ACC );

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
			m_MiscAList.Append( ex_attr_t::BHP );
			m_MiscAList.Append( ex_attr_t::BMP );
			m_MiscAList.Append( ex_attr_t::HIT );
			m_MiscAList.Append( ex_attr_t::DG );
			m_MiscAList.Append( ex_attr_t::ATK );
		}

		m_BaseAGrid = GetWidget<EGUiGridWidget>( eg_crc("BaseAttributes") );
		m_DmgAGrid = GetWidget<EGUiGridWidget>( eg_crc("DmgAttributes") );
		m_DefAGrid = GetWidget<EGUiGridWidget>( eg_crc("DefAttributes") );
		m_MiscAGrid = GetWidget<EGUiGridWidget>( eg_crc("MiscAttributes") );
		m_AttrDescText = GetWidget<EGUiTextWidget>( eg_crc("AttrDescText") );

		m_MonsterPortrait = GetWidget<EGUiMeshWidget>( eg_crc("MonsterPortrait") );
		m_ClassText = GetWidget<EGUiTextWidget>( eg_crc("ClassText") );
		m_MoreInfoText = GetWidget<EGUiTextWidget>( eg_crc("MoreInfoText") );
		m_TestAgainstText = GetWidget<EGUiTextWidget>( eg_crc("TestAgainstText") );

		m_MonsterIdList = ExBeastiary::Get().GetDebugMenuSortOrder();

		m_ViewedMonsterIndex = EG_Clamp<eg_int>( ExDebugMonsterViewerMenu_LastViewedMonster.GetValue() , 0 , m_MonsterIdList.LenAs<eg_int>()-1 );
		m_ViewedMonsterLevel = 1;

		RefreshGrids();

		InitMonster();

		RefreshHints();

		RefreshTestAgainstText();

		DoReveal();
	}

	virtual void OnDeinit() override final
	{
		Engine_QueMsg( "engine.SaveConfig()" );
		
		Super::OnDeinit();
	}

	void ToggleMonster( eg_bool bNext )
	{
		const eg_int TotalMonsters = m_MonsterIdList.LenAs<eg_int>();

		if( TotalMonsters > 0 )
		{
			m_ViewedMonsterIndex = ( m_ViewedMonsterIndex + TotalMonsters + (bNext ? 1 : -1) ) % TotalMonsters;
		}
		else
		{
			m_ViewedMonsterIndex = 0;
		}

		ExDebugMonsterViewerMenu_LastViewedMonster.SetValue( m_ViewedMonsterIndex );

		PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		InitMonster();
	}

	void ToggleLevel( eg_bool bNext )
	{
		if( !((m_ViewedMonsterLevel == 1 && !bNext) || m_ViewedMonsterLevel == EX_MAX_LEVEL && bNext) )
		{
			PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}

		m_ViewedMonsterLevel = EG_Clamp<eg_int>( m_ViewedMonsterLevel + (bNext ? 1 : -1) , 1 , EX_MAX_LEVEL );
	
		InitMonster();
	}

	const exBeastInfo& GetViewedMonsterInfo() const
	{
		const exBeastInfo& MonsterInfo = ExBeastiary::Get().FindInfo( m_MonsterIdList.IsValidIndex( m_ViewedMonsterIndex ) ? m_MonsterIdList[m_ViewedMonsterIndex] : CT_Clear );
		return MonsterInfo;
	}

	void InitMonster()
	{
		const exBeastInfo& MonsterInfo = GetViewedMonsterInfo();
		
		m_ViewMonster = ExFighter( CT_Default );
		m_ViewMonster.InitMonster( MonsterInfo , m_ViewedMonsterLevel );
		m_ViewMonster.SetAttackType( ex_attack_t::MELEE );
		m_ViewMonster.ResolveReplicatedData();

		if( m_ClassText )
		{
			m_ClassText->SetText( CT_Clear , EGFormat( eg_loc("DebugMonsterMenuMenuLevelText","{0:NAME} LVL {0:ATTR:LVL} ({0:CLASS})"), m_ViewMonster ) );
		}

		if( m_MonsterPortrait )
		{
			m_MonsterPortrait->SetTexture( eg_crc("Sprite") , eg_crc("Mesh") , *MonsterInfo.ImagePath.FullPath );
		}

		if( m_MoreInfoText )
		{
			const ex_xp_value XPReward = MonsterInfo.GetXPReward( m_ViewMonster.GetAttrValue( ex_attr_t::LVL ) );
			const eg_ivec2 GoldDropRange = MonsterInfo.GetGoldDropRange( m_ViewMonster.GetAttrValue( ex_attr_t::LVL ) );

			const eg_string_crc GoldSameRewardText = eg_loc("MonsterRewardGoldSameText","XP Reward: {0}, Gold Reward: {1}");
			const eg_string_crc GoldDiffRewardText = eg_loc("MonsterRewardGoldDiffText","XP Reward: {0}, Gold Reward: {1}-{2}");
			eg_loc_text MoreInfoString = EGFormat( GoldDropRange.x == GoldDropRange.y ? GoldSameRewardText : GoldDiffRewardText , XPReward , GoldDropRange.x , GoldDropRange.y );

			m_MoreInfoText->SetText( CT_Clear , MoreInfoString );
		}

		RefreshGrids();
		RefreshTestAgainstText();
	}

	void RefreshTestAgainstText()
	{
		if( m_TestAgainstText )
		{
			eg_loc_text TestAgainstStr = EGFormat( eg_loc("TestAgainstFormatText","Test Against: {0} |SC(EX_ATTR)|LVL|RC()| {1}") , ExCore_GetClassName( m_TestAgainstClass ) , m_ViewedMonsterLevel );
			m_TestAgainstText->SetText( CT_Clear , TestAgainstStr );
		}
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}
		else if( InputType == eg_menuinput_t::NEXT_PAGE )
		{
			ToggleMonster( true );
			return true;
		}
		else if( InputType == eg_menuinput_t::PREV_PAGE )
		{
			ToggleMonster( false );
			return true;
		}
		else if( InputType == eg_menuinput_t::NEXT_SUBPAGE )
		{
			ToggleLevel( true );
			return true;
		}
		else if( InputType == eg_menuinput_t::PREV_SUBPAGE )
		{
			ToggleLevel( false );
			return true;
		}
		else if( InputType == eg_menuinput_t::BUTTON_PRIMARY )
		{
			ExDebugCombatLauncherMenu::OpenSampleCombat( GetClientOwner() , GetViewedMonsterInfo().Id , m_ViewedMonsterLevel , m_TestAgainstClass );
			return true;
		}

		return false;
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override
	{
		if( Cmds.WasPressed( CMDA_MENU_BTN2 ) )
		{
			switch( m_TestAgainstClass )
			{
			case ex_class_t::Warrior:
				m_TestAgainstClass = ex_class_t::Thief;
				break;
			case ex_class_t::Thief:
				m_TestAgainstClass = ex_class_t::Mage;
				break;
			case ex_class_t::Mage:
				m_TestAgainstClass = ex_class_t::Cleric;
				break;
			default:
			case ex_class_t::Cleric:
				m_TestAgainstClass = ex_class_t::Warrior;
				break;
			}

			PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			RefreshTestAgainstText();
		}
	}

	void RefreshHints()
	{
		ClearHints();
		AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
		AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text( eg_loc("ToggleMonsterText","Toggle Monster") ) );
		AddHint( CMDA_MENU_PREV_SUBPAGE , CT_Clear );
		AddHint( CMDA_MENU_NEXT_SUBPAGE , eg_loc_text( eg_loc("ChangeLevelText","Change Level") ) );
		AddHint( CMDA_MENU_BTN2 , eg_loc_text( eg_loc("ChangeTestAgainstClassText","Change Test Class") ) );
		AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("TestFightText","Test Fight") ) );
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("BackOutText","Back")) );
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
				eg_d_string16 AttributeNameFormatStr = L"|SC(EX_ATTR)|{0}|RC()|"; // EGSFormat16( L"|SC({0})|\\{0\\}|RC()|" , ExCore_GetAttributeNameFormatColor( Type ) );
				ItemInfo.Widget->SetText( eg_crc("NameText") , EGFormat( *AttributeNameFormatStr , ExCore_GetAttributeName( Type ) ) );
				if( Type == ex_attr_t::HP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_ViewMonster.GetHP()) , static_cast<eg_int>(m_ViewMonster.GetAttrValue( Type )) ) );
				}
				else if( Type == ex_attr_t::MP )
				{
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( L"{0}/{1}" , static_cast<eg_int>(m_ViewMonster.GetMP()) , static_cast<eg_int>(m_ViewMonster.GetAttrValue( Type )) ) );
				}
				else
				{
					eg_d_string16 AttributeValueFormatStr = EGSFormat16( L"|SC({0})|\\{0\\}|RC()|" , ExCore_GetAttributeValueFormatColor( Type ) );
					ItemInfo.Widget->SetText( eg_crc("ValueText") , EGFormat( *AttributeValueFormatStr , static_cast<eg_int>(m_ViewMonster.GetAttrValue( Type )) ) );
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
			SetFocusedWidget( m_BaseAGrid , -1 , false );
			SetFocusedWidget( m_BaseAGrid , 0 , false );
		}

	}
};

EG_CLASS_DECL( ExDebugMonsterViewerMenu )

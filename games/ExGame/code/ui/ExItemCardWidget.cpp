// (c) 2018 Beem Media

#include "ExItemCardWidget.h"
#include "ExInventoryBag.h"
#include "ExSpellInfo.h"
#include "ExCore.h"
#include "ExCombatMenu_SubMenus.h"

EG_CLASS_DECL( ExItemCardWidget )

void ExItemCardWidget::SetMode( ex_mode InMode )
{
	m_Mode = InMode;
}

void ExItemCardWidget::SetItem( const exInventoryBagItem* BagItem, const ExFighter* TargetFighter , eg_bool bIncludeEquipment /*= false*/ )
{
	eg_loc_text TitleText;
	eg_loc_text BodyText;

	if( TargetFighter )
	{
		m_TargetFighter = *TargetFighter;
	}
	else
	{
		m_TargetFighter = CT_Default;
	}

	if( !bIncludeEquipment )
	{
		m_TargetFighter.DeleteEquipment();
	}
	m_TargetFighter.ResolveReplicatedData();

	exInventoryItem::SetReferenceFighter( &m_TargetFighter );

	if( BagItem )
	{
		m_ItemBeingViewed = *BagItem;
		TitleText = EGFormat( L"{0:NAME}" , BagItem );

		switch( m_Mode )
		{
		case ex_mode::Default:
			BodyText = EGFormat( L"{0:DESC:ITEM_CARD}" , BagItem );
			break;
		case ex_mode::VendorSell:
			BodyText = EGFormat( L"{0:COST:SELL}\n{0:DESC:ITEM_CARD}" , BagItem );
			break;
		case ex_mode::VendorBuy:
			BodyText = EGFormat( L"{0:COST:BUY}\n{0:DESC:ITEM_CARD}" , BagItem );
			break;
		case ex_mode::VendorBuyback:
			BodyText = EGFormat( L"{0:COST:BUYBACK}\n{0:DESC:ITEM_CARD}" , BagItem );
			break;
		}
	}
	else
	{
		m_ItemBeingViewed = CT_Clear;
	}

	exInventoryItem::SetReferenceFighter( nullptr );
	
	SetText( eg_crc("ItemName") , TitleText );
	SetText( eg_crc("ItemDesc") , BodyText );
	SetVisible( BagItem != nullptr );
}

void ExItemCardWidget::SetSpell( const exSpellInfo* SpellInfo , const ExFighter* TargetFighter , eg_bool bIncludeEquipment )
{
	eg_loc_text TitleText;
	eg_loc_text BodyText;

	if( TargetFighter )
	{
		m_TargetFighter = *TargetFighter;
	}
	else
	{
		m_TargetFighter = CT_Default;
	}

	if( !bIncludeEquipment )
	{
		m_TargetFighter.DeleteEquipment();
	}
	m_TargetFighter.ResolveReplicatedData();

	exSpellInfo::SetReferenceFighter( &m_TargetFighter );

	m_ItemBeingViewed = CT_Clear;

	if( SpellInfo )
	{
		m_TargetFighter.SetAttackType( ex_attack_t::MELEE );
		TitleText = EGFormat( L"{0:NAME}" , SpellInfo );
		BodyText = EGFormat( L"{0:DESC:ITEM_CARD}" , SpellInfo );
		m_TargetFighter.SetAttackType( ex_attack_t::NONE );
	}

	exSpellInfo::SetReferenceFighter( nullptr );

	SetText( eg_crc("ItemName") , TitleText );
	SetText( eg_crc("ItemDesc") , BodyText );
	SetVisible( SpellInfo != nullptr );
}

void ExItemCardWidget::SetAttack( const ExFighter* TargetFighter , eg_bool bIncludeEquipment , ex_combat_actions ActionType , eg_bool bCanMelee , eg_bool bCanShoot )
{
	// static const eg_cpstr16 DmgFormat = L"|SC(EX_ATTR)|{0}|RC()| {1:ATTR:DMG_} ({1:ATTR:DMG_:DETAIL})";
	static const eg_cpstr16 DmgFormat = L"|SC(EX_ATTR)|{0}|RC()| {1:ATTR:DMG_}";

	eg_loc_text BodyText;

	if( TargetFighter )
	{
		m_TargetFighter = *TargetFighter;
	}
	else
	{
		m_TargetFighter = CT_Default;
	}

	if( !bIncludeEquipment )
	{
		m_TargetFighter.DeleteEquipment();
	}
	m_TargetFighter.ResolveReplicatedData();

	eg_bool bShow = false;

	switch( ActionType )
	{
	case ex_combat_actions::UNKNOWN:
		break;
	case ex_combat_actions::AUTO:
	case ex_combat_actions::FIGHT:
	{
		bShow = true;
		if( bCanMelee )
		{
			BodyText = eg_loc_text(ExCombatMenu_CombatantAction::GetTextForAction( ex_combat_actions::MELEE ));
		}
		else if( bCanShoot )
		{
			BodyText = eg_loc_text(ExCombatMenu_CombatantAction::GetTextForAction( ex_combat_actions::SHOOT ));
		}
		else
		{
			BodyText = eg_loc_text(ExCombatMenu_CombatantAction::GetTextForAction( ex_combat_actions::BLOCK ));
		}
	} break;
	case ex_combat_actions::BLOCK:
	{
		bShow = true;
		const ex_attr_value BaseDef = m_TargetFighter.GetAttrValue( ex_attr_t::PDEF );
		const ex_attr_value BlockDef = static_cast<ex_attr_value>(BaseDef*EX_BLOCKING_MULTIPLIER);
		const ex_attr_value BlockDiff = BlockDef - BaseDef;
		BodyText = EGFormat( L"|SC(EX_ATTR)|{0}|RC()| +{1}" , ExCore_GetAttributeName( ex_attr_t::DEF_ ) , BlockDiff );
	} break;
	case ex_combat_actions::MELEE:
		bShow = true;
		m_TargetFighter.SetAttackType( ex_attack_t::MELEE );
		BodyText = EGFormat( DmgFormat , ExCore_GetAttributeName( ex_attr_t::DMG_ ) , m_TargetFighter );
		m_TargetFighter.SetAttackType( ex_attack_t::NONE );
		break;
	case ex_combat_actions::SHOOT:
		bShow = true;
		m_TargetFighter.SetAttackType( ex_attack_t::RANGED );
		BodyText = EGFormat( DmgFormat , ExCore_GetAttributeName( ex_attr_t::DMG_ ) , m_TargetFighter );
		m_TargetFighter.SetAttackType( ex_attack_t::NONE );
		break;
	case ex_combat_actions::CAST:
		bShow = true;
		m_TargetFighter.SetAttackType( ex_attack_t::MELEE );
		BodyText = EGFormat( L"|SC(EX_ATTR)|{0}|RC()| {1:ATTR:MP:DETAIL}" , ExCore_GetAttributeName( ex_attr_t::MP ) , m_TargetFighter );
		m_TargetFighter.SetAttackType( ex_attack_t::NONE );
		break;
	case ex_combat_actions::ADVANCE:
		break;
	case ex_combat_actions::BACKAWAY:
		break;
	case ex_combat_actions::RUN:
		break;
	case ex_combat_actions::VIEW:
		break;
	case ex_combat_actions::ABANDON:
		break;
	case ex_combat_actions::DEBUG_WIN:
		break;
	}

	SetText( eg_crc("ItemDesc") , BodyText );

	if( bShow )
	{
		RunEvent( eg_crc("Show") );
	}
	else
	{
		RunEvent( eg_crc("HideNow") );
	}
}

void ExItemCardWidget::SetSpellTargeting( const ExFighter* TargetFighter , eg_bool bIncludeEquipment , ex_combat_actions ActionType , const exSpellInfo* Spell )
{
	// static const eg_cpstr16 DmgFormat = L"|SC(EX_ATTR)|{0}|RC()| {1:ATTR:DMG_} ({1:ATTR:DMG_:DETAIL})";
	static const eg_cpstr16 DmgFormat = L"|SC(EX_ATTR)|{0}|RC()| {1:ATTR:DMG_}";

	eg_loc_text BodyText;

	if( TargetFighter )
	{
		m_TargetFighter = *TargetFighter;
	}
	else
	{
		m_TargetFighter = CT_Default;
	}

	if( !bIncludeEquipment )
	{
		m_TargetFighter.DeleteEquipment();
	}
	m_TargetFighter.ResolveReplicatedData();

	eg_bool bShow = false;

	switch( ActionType )
	{
	case ex_combat_actions::UNKNOWN:
	case ex_combat_actions::AUTO:
	case ex_combat_actions::FIGHT:
	case ex_combat_actions::BLOCK:
	case ex_combat_actions::MELEE:
	case ex_combat_actions::SHOOT:
		break;
	case ex_combat_actions::CAST:
	{
		if( Spell && Spell->CanTargetEnemy() && Spell->TargetCount == 1 )
		{
			bShow = true;
			m_TargetFighter.SetAttackType( ex_attack_t::MELEE );
			const ex_attr_value AttackerMag = m_TargetFighter.GetAttrValue( ex_attr_t::MAG );
			const ex_attr_value AttackerLvl = m_TargetFighter.GetAttrValue( ex_attr_t::LVL );
			const exDamage Dmg = Spell->GetDamage( AttackerMag , AttackerLvl );
			BodyText = EGFormat( L"|SC(EX_ATTR)|{0}|RC()| {1}" , ExCore_GetAttributeName( ex_attr_t::DMG_ ) , Dmg.GetRawTotal() );
			m_TargetFighter.SetAttackType( ex_attack_t::NONE );
		}
	} break;
	case ex_combat_actions::ADVANCE:
	case ex_combat_actions::BACKAWAY:
	case ex_combat_actions::RUN:
	case ex_combat_actions::VIEW:
	case ex_combat_actions::ABANDON:
	case ex_combat_actions::DEBUG_WIN:
		break;
	}

	SetText( eg_crc("ItemDesc") , BodyText );

	if( bShow )
	{
		RunEvent( eg_crc("Show") );
	}
	else
	{
		RunEvent( eg_crc("HideNow") );
	}
}

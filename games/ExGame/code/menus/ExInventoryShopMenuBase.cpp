// (c) 2020 Beem Media. All Rights Reserved.

#include "ExInventoryShopMenuBase.h"
#include "ExMenuPortraitsGridWidget.h"
#include "ExInventoryBag.h"
#include "ExItemCardWidget.h"
#include "ExArmoryItem.h"

EG_CLASS_DECL( ExInventoryShopMenuBase )

void ExInventoryShopMenuBase::OnInit()
{
	Super::OnInit();

	m_DispAttrs.Append( ex_attr_t::DMG_ );
	m_DispAttrs.Append( ex_attr_t::DEF_ );
	m_DispAttrsMid.Append( ex_attr_t::HP );
	m_DispAttrsMid.Append( ex_attr_t::MP );
	m_DispAttrsMid.Append( ex_attr_t::STR );
	m_DispAttrsMid.Append( ex_attr_t::MAG );
	m_DispAttrsMid.Append( ex_attr_t::END );
	m_DispAttrsMid.Append( ex_attr_t::SPD );

	m_Portraits = GetWidget<ExMenuPortraitsGridWidget>( eg_crc("Portraits") );
	m_InventoryListWidget = GetWidget<ExInventoryBag>( eg_crc("InventoryList") );
	m_ItemCard = GetWidget<ExItemCardWidget>( eg_crc("ItemCard") );
	m_AttributesEffectsHintWidget = GetWidget<EGUiTextWidget>( eg_crc("AttributesEffectsHint") );
	m_AttributesHeaderWidget = GetWidget<EGUiTextWidget>( eg_crc("AttributesHeader") );
	m_AttributesSubHeaderWidget = GetWidget<EGUiTextWidget>( eg_crc("AttributesSubHeader") );
	m_AttributesWidget = GetWidget<EGUiGridWidget>( eg_crc("Attributes") );
	m_AttributesMidWidget = GetWidget<EGUiGridWidget>( eg_crc("AttributesMid") );

	if( m_Portraits )
	{
		m_Portraits->SetSelection( GetGame()->GetSelectedPartyMemberIndex() );
	}

	if( m_Portraits )
	{
		m_Portraits->SetVisible( true );
		m_Portraits->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnPortraitSelected );
	}

	if( m_AttributesWidget )
	{
		m_AttributesWidget->SetEnabled( false );
		m_AttributesWidget->CellChangedDelegate.Bind( this , &ThisClass::OnAttributeCellChanged );
	}

	if( m_AttributesMidWidget )
	{
		m_AttributesMidWidget->SetEnabled( false );
		m_AttributesMidWidget->CellChangedDelegate.Bind( this , &ThisClass::OnAttributeCellChanged );
	}

	if( m_AttributesEffectsHintWidget )
	{
		m_AttributesEffectsHintWidget->SetText( CT_Clear , CT_Clear );
	}
}

void ExInventoryShopMenuBase::OnSelectedCharacterChanged( eg_uint PartyIndex , eg_bool bAllowAudio )
{
	unused( PartyIndex , bAllowAudio );
}

void ExInventoryShopMenuBase::OnPortraitSelected( EGUiGridWidget* GridOwner , eg_uint CellIndex )
{
	unused( GridOwner );

	OnSelectedCharacterChanged( CellIndex , true );
}

ex_item_slot ExInventoryShopMenuBase::GetBeginEquipTarget( const exInventoryItem& ItemIn ) const
{
	if( ItemIn.ArmoryItem )
	{
		EGArray<ex_item_slot> EquipSlots;
		if( ItemIn.ArmoryItem->EquipMode == ex_equip_m::SINGLE_SLOT )
		{
			ItemIn.ArmoryItem->EquipSlots.GetAllowedSlots( EquipSlots );
		}
		else
		{
			EquipSlots.Append( ItemIn.ArmoryItem->GetDefaultEquipToSlot() );
		}
		ex_item_slot DefaultEquipSlot = ItemIn.ArmoryItem->GetDefaultEquipToSlot();
		ex_item_slot EquipSlot = EquipSlots.IsValidIndex( m_FallbackEquipIndex ) ? EquipSlots[m_FallbackEquipIndex] : DefaultEquipSlot;
		return EquipSlot;
	}
	return ex_item_slot::NONE;
}

void ExInventoryShopMenuBase::SetupDisplayedAttributes( const exInventoryItem& ItemIn , ex_item_slot TargetEquipSlot , eg_bool bIsRemoving )
{
	m_DispAttrItem = ItemIn;
	m_DispAttrItem.ResolvePointers();
	
	const ex_attack_t AttackTypeToShow = m_DispAttrItem.ArmoryItem && m_DispAttrItem.ArmoryItem->AttackType != ex_attack_t::NONE ? m_DispAttrItem.ArmoryItem->AttackType : ex_attack_t::MELEE;

	const ExFighter* SelectedPartyMember = GetSelectedPartyMember();
	if( SelectedPartyMember )
	{
		m_DispAttrBaseFighter = *SelectedPartyMember;
	}
	else
	{
		m_DispAttrBaseFighter = CT_Clear;
		m_DispAttrBaseFighter.SetAttrValue( ex_attr_t::LVL , 1 );
	}
	m_DispAttrBaseFighter.ResolveReplicatedData();
	m_DispAttrBaseFighter.SetAttackType( AttackTypeToShow );

	m_DispAttrEquippedFighter = m_DispAttrBaseFighter;

	eg_bool bShowRemovingEffect = false;
	eg_bool bHasItem = m_DispAttrItem.RawItem.ItemId.IsNotNull();
	ex_item_slot EquipSlot = GetBeginEquipTarget( m_DispAttrItem );
	/*
	// eg_bool bIsRemoving = bHasItem && m_State == ex_state::Browsing && TargetEquipSlot != ex_item_slot::NONE;
	if( bIsEquipping || bIsRemoving )//if( m_State == ex_state::Equipping || bIsRemoving )
	{
		EquipSlot = TargetEquipSlot;
	}
	if( m_State == ex_state::DraggingInventoryItem )
	{
		if( m_DragAndDropSlotType != ex_item_slot::NONE && m_DispAttrItem.ArmoryItem->CanEquipTo( m_DragAndDropSlotType ) )
		{
			EquipSlot = m_DragAndDropSlotType;
		}
	}
	*/
	if( TargetEquipSlot != ex_item_slot::NONE )
	{
		EquipSlot = TargetEquipSlot;
	}

	eg_bool bCanEquip = bHasItem && !bIsRemoving && m_DispAttrEquippedFighter.MeetsRequirementsToEquip( m_DispAttrItem );

	if( (bCanEquip || bIsRemoving) && m_DispAttrItem.ArmoryItem && ex_item_slot::NONE != EquipSlot )
	{
		EGArray<exInventoryItem> ItemsRemoved;
		if( bIsRemoving )
		{
			if( bShowRemovingEffect )
			{
				m_DispAttrEquippedFighter.EquipInventoryItem( EquipSlot , CT_Clear , ItemsRemoved );
			}
		}
		else
		{
			m_DispAttrEquippedFighter.EquipInventoryItem( EquipSlot , m_DispAttrItem , ItemsRemoved );
		}
		// EGLogf( eg_log_t::General , "Equipping item removes %d items." , ItemsRemoved.LenAs<eg_uint>() );
	}

	// Hint if we can change what equip slot we are viewing.
	const eg_bool bShowEquipTargetHint = !bIsRemoving && bCanEquip && EquipSlot != ex_item_slot::NONE && ItemIn.ArmoryItem && ItemIn.ArmoryItem->CanEquipToMultipleSlots();
	if( m_AttributesEffectsHintWidget )
	{
		m_AttributesEffectsHintWidget->SetText( CT_Clear , bShowEquipTargetHint ? eg_loc_text(L"|Glyph(EX_MENU_COMPARE)|") : CT_Clear );
	}

	m_DispAttrEquippedFighter.ResolveReplicatedData();
	m_DispAttrEquippedFighter.SetAttackType( AttackTypeToShow );

	const eg_string_crc EffectQualifierText = AttackTypeToShow == ex_attack_t::RANGED ? eg_loc("InventoryMenuAttributesRanged"," (Ranged Attack)") : CT_Clear ;

	if( m_AttributesHeaderWidget && m_AttributesSubHeaderWidget )
	{
		m_AttributesHeaderWidget->SetText( CT_Clear , EGFormat( eg_loc( "InventoryMenuAttributesHeaderCharacter" , "{0:NAME} LVL {0:ATTR:LVL} {0:CLASS}" ) , m_DispAttrEquippedFighter ) );
		eg_loc_text HeaderText;
		if( bIsRemoving )
		{
			if( bShowRemovingEffect )
			{
				HeaderText = EGFormat( eg_loc( "InventoryMenuAttributesHeaderRemoveEffect" , "Effect removing from {0}" ) , ExCore_GetItemSlotName( EquipSlot ) );
			}
			else
			{
				HeaderText = CT_Clear;
			}
		}
		else if( bHasItem && bCanEquip )
		{
			HeaderText = EGFormat( eg_loc( "InventoryMenuAttributesHeaderEffect" , "Effect on {0}{1}" ) , ExCore_GetItemSlotName( EquipSlot ) , EffectQualifierText );
		}
		else if( bHasItem && !bCanEquip )
		{
			HeaderText = eg_loc_text( eg_loc( "InventoryMenuAttributesHeaderCannotEquip" , "|SC(RED)|Cannot Equip|RC(RED)|" ) );
		}
		else
		{
			HeaderText = CT_Clear;
		}

		if( HeaderText.GetLen() == 0 )
		{
			eg_d_string16 CurrentAttrHeader = eg_loc_text( eg_loc( "InventoryMenuAttributesCurrent" , "Current Attributes" ) ).GetString();
			CurrentAttrHeader.Append( eg_loc_text( EffectQualifierText ).GetString() );
			HeaderText = eg_loc_text(*CurrentAttrHeader);
		}

		m_AttributesSubHeaderWidget->SetText( CT_Clear , HeaderText );
	}

	if( m_AttributesWidget )
	{
		m_AttributesWidget->RefreshGridWidget( m_DispAttrs.LenAs<eg_uint>() );
	}

	if( m_AttributesMidWidget )
	{
		m_AttributesMidWidget->RefreshGridWidget( m_DispAttrsMid.LenAs<eg_uint>() );
	}
}

void ExInventoryShopMenuBase::OnAttributeCellChanged( egUiGridWidgetCellChangedInfo& CellInfo )
{
	if( CellInfo.GridItem )
	{
		ex_attr_t AttrType = ex_attr_t::XP_;
		if( CellInfo.Owner == m_AttributesWidget && m_DispAttrs.IsValidIndex( CellInfo.GridIndex ) )
		{
			AttrType = m_DispAttrs[CellInfo.GridIndex];
		}
		else if( CellInfo.Owner == m_AttributesMidWidget && m_DispAttrsMid.IsValidIndex( CellInfo.GridIndex ) )
		{
			AttrType = m_DispAttrsMid[CellInfo.GridIndex];
		}

		if( AttrType == ex_attr_t::XP_ )
		{
			// XP_ in this case means leave a blank space
			CellInfo.GridItem->SetText( eg_crc( "NameText" ) , CT_Clear );
			CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , CT_Clear );
		}
		else
		{
			CellInfo.GridItem->SetText( eg_crc( "NameText" ) , EGFormat( L"|SC(EX_ATTR)|{0}|RC()|" , ExCore_GetAttributeName( AttrType ) ) );
			eg_loc_text AttrValueText;
			FormatAttributeValue( AttrType , AttrValueText );
			CellInfo.GridItem->SetText( eg_crc( "ValueText" ) , AttrValueText );
		}
	}
}

ExInventoryShopMenuBase::exEquipChange ExInventoryShopMenuBase::GetAttrValue( ex_attr_t AttrType ) const
{
	exEquipChange Out;
	Out.OldValue = m_DispAttrBaseFighter.GetAttrValue( AttrType );
	Out.NewValue = m_DispAttrEquippedFighter.GetAttrValue( AttrType );
	return Out;
}

void ExInventoryShopMenuBase::FormatAttributeValue( ex_attr_t AttrType , eg_loc_text& Out ) const
{
	if( GetSelectedPartyMember() )
	{
		exEquipChange EquipChange = GetAttrValue( AttrType );
		if( EquipChange.IsBetter() )
		{
			Out = EGFormat( L"|SC(EX_INC)|{0} (+{1})|RC()|" , static_cast<eg_int>(EquipChange.NewValue) , EquipChange.GetDelta() );
		}
		else if( EquipChange.IsWorse() )
		{
			Out = EGFormat( L"|SC(EX_DEC)|{0} (-{1})|RC()|" , static_cast<eg_int>(EquipChange.NewValue) , EquipChange.GetDelta() );
		}
		else
		{
			Out = EGFormat( L"{0}" , static_cast<eg_int>(EquipChange.NewValue) );
		}

		eg_cpstr16 DamageFormatStr = L"(|SC(EX_PDMG)|{0}|RC()|/|SC(EX_FDMG)|{1}|RC()|/|SC(EX_WDMG)|{2}|RC()|/|SC(EX_EDMG)|{3}|RC()|/|SC(EX_ADMG)|{4}|RC()|)";

		if( AttrType == ex_attr_t::DMG_ )
		{
			eg_loc_text FullDammageText = EGFormat( DamageFormatStr , GetAttrValue( ex_attr_t::PDMG ).NewValue , GetAttrValue( ex_attr_t::FDMG ).NewValue , GetAttrValue( ex_attr_t::WDMG ).NewValue , GetAttrValue( ex_attr_t::EDMG ).NewValue , GetAttrValue( ex_attr_t::ADMG ).NewValue );
			Out = EGFormat( L"{0} {1}" , Out , FullDammageText );
		}
		else if( AttrType == ex_attr_t::DEF_ )
		{
			eg_loc_text FullDefText = EGFormat( DamageFormatStr , GetAttrValue( ex_attr_t::PDEF ).NewValue , GetAttrValue( ex_attr_t::FDEF ).NewValue , GetAttrValue( ex_attr_t::WDEF ).NewValue , GetAttrValue( ex_attr_t::EDEF ).NewValue , GetAttrValue( ex_attr_t::ADEF ).NewValue );
			Out = EGFormat( L"{0} {1}" , Out , FullDefText );
		}
	}
	else
	{
		Out = eg_loc_text( L"-" );
	}
}

const ExFighter* ExInventoryShopMenuBase::GetSelectedPartyMember() const
{
	const ExGame* Game = GetGame();
	if( Game && m_Portraits )
	{
		return Game->GetPartyMemberByIndex( m_Portraits->GetSelectedIndex() );
	}
	return nullptr;
}

eg_uint ExInventoryShopMenuBase::GetSelectedPartyMemberIndex() const
{
	const ExGame* Game = GetGame();
	if( Game && m_Portraits )
	{
		return m_Portraits->GetSelectedIndex();
	}
	return ExRoster::PARTY_SIZE;
}

// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "ExGameTypes.h"
#include "ExFighter.h"
#include "ExInventory.h"

class ExInventoryBag;
class EGUiGridWidget;
class EGUiTextWidget;
class ExMenuPortraitsGridWidget;
class ExItemCardWidget;
struct egUiGridWidgetCellChangedInfo;

class ExInventoryShopMenuBase : public ExMenu
{
	EG_CLASS_BODY( ExInventoryShopMenuBase , ExMenu )

protected:

	struct exEquipChange
	{
		ex_attr_value OldValue = 0;
		ex_attr_value NewValue = 0;

		eg_bool IsBetter() const { return NewValue > OldValue; }
		eg_bool IsWorse() const { return OldValue > NewValue; }
		eg_bool IsSame() const { return OldValue == NewValue; }
		eg_int GetDelta() const { return EG_Abs( NewValue - OldValue ); }
	};

protected:

	ExInventoryBag* m_InventoryListWidget;
	ExMenuPortraitsGridWidget* m_Portraits;
	ExItemCardWidget* m_ItemCard;
	eg_int m_FallbackEquipIndex = 0;

private:

	EGUiTextWidget* m_AttributesHeaderWidget;
	EGUiTextWidget* m_AttributesEffectsHintWidget;
	EGUiTextWidget* m_AttributesSubHeaderWidget;
	EGUiGridWidget* m_AttributesWidget;
	EGUiGridWidget* m_AttributesMidWidget;

private:

	EGArray<ex_attr_t> m_DispAttrs;
	EGArray<ex_attr_t> m_DispAttrsMid;
	ExFighter          m_DispAttrBaseFighter = CT_Clear;
	ExFighter          m_DispAttrEquippedFighter = CT_Clear;
	exInventoryItem    m_DispAttrItem = CT_Clear;

protected:

	virtual void OnInit() override;

	virtual void OnSelectedCharacterChanged( eg_uint PartyIndex , eg_bool bAllowAudio );

	void OnPortraitSelected( EGUiGridWidget* GridOwner , eg_uint CellIndex );
	ex_item_slot GetBeginEquipTarget( const exInventoryItem& Item ) const;
	void SetupDisplayedAttributes( const exInventoryItem& ItemIn , ex_item_slot TargetEquipSlot , eg_bool bIsRemoving );
	void OnAttributeCellChanged( egUiGridWidgetCellChangedInfo& CellInfo );
	exEquipChange GetAttrValue( ex_attr_t AttrType ) const;
	void FormatAttributeValue( ex_attr_t AttrType , eg_loc_text& Out ) const;

	const ExFighter* GetSelectedPartyMember() const;
	eg_uint GetSelectedPartyMemberIndex() const;
};

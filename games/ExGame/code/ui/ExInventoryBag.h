// (c) 2017 Beem Media

#pragma once

#include "EGTextFormat.h"
#include "ExInventory.h"
#include "EGUiGridWidget.h"
#include "EGUiButtonWidget.h"

enum class ex_inventory_bag_item_t
{
	None,
	EmptyInventoryItem,
	InventoryItem,
};

struct exInventoryBagItem : public IEGCustomFormatHandler
{
	ex_inventory_bag_item_t Type;
	exInventoryItem         InventoryItem;
	eg_bool                 bEquipingFrom = false;

	exInventoryBagItem()
	: Type( ex_inventory_bag_item_t::None )
	, InventoryItem( CT_Clear )
	{
	}

	exInventoryBagItem( eg_ctor_t Ct )
	: Type( ex_inventory_bag_item_t::None )
	, InventoryItem( Ct )
	{
	}

	exInventoryBagItem( const exInventoryItem& rhs )
	: Type( ex_inventory_bag_item_t::InventoryItem )
	, InventoryItem( rhs )
	{
	}

	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override final;

	static eg_bool IsTypeNonEmpty( ex_inventory_bag_item_t Type )
	{
		return Type == ex_inventory_bag_item_t::InventoryItem;
	}
};

class ExInventoryBag : public EGUiGridWidget
{
	EG_CLASS_BODY( ExInventoryBag , EGUiGridWidget )

public:

	enum class ex_populate_t
	{
		All,
		InventoryOnly,
	};

protected:

	EGArray<exInventoryBagItem>  m_Items;
	eg_bool                      m_bCanDrag = true;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	void SetCanDrag( eg_bool bCan ) { m_bCanDrag = bCan; }
	void PopulateBag( const ExGame* Game , ex_populate_t PopulateType );
	const EGArray<exInventoryBagItem>& GetItems() const { return m_Items; }
	void SetEquippingFromItem( eg_int ItemIndex );

protected:

	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ) override;
	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo );
};

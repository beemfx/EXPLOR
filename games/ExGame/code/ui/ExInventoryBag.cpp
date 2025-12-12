// (c) 2017 Beem Media

#include "ExInventoryBag.h"
#include "EGMenu.h"
#include "ExArmoryItem.h"
#include "ExGame.h"

EG_CLASS_DECL( ExInventoryBag )

void ExInventoryBag::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
}

void ExInventoryBag::PopulateBag( const ExGame* Game, ex_populate_t PopulateType )
{
	const eg_uint NumColumns = GetNumVisibleColumns();
	assert( NumColumns >= 1 );

	m_Items.Clear();

	auto InsertEmptyItems = [this]( EGArray<exInventoryBagItem>& List , eg_uint Count ) -> void
	{
		eg_size_t StartIndex = List.Len();
		List.Resize( List.Len() + Count );
		for( eg_size_t i=StartIndex; i<List.Len(); i++ )
		{
			List[i].Type = ex_inventory_bag_item_t::EmptyInventoryItem;
		}
	};

	if( Game )
	{
		EGArray<exInventoryItem> InventoryItems;
		Game->GetInventoryItems( InventoryItems );

		if( PopulateType == ex_populate_t::InventoryOnly || PopulateType == ex_populate_t::All )
		{
			const eg_size_t FirstInventoryPos = m_Items.Len();
			InsertEmptyItems( m_Items , Game->GetRoster().GetInventory().GetBagSize() );

			for( const exInventoryItem& Item : InventoryItems )
			{
				eg_size_t ListIndex = FirstInventoryPos + Item.InventoryListIndex;
				if( m_Items.IsValidIndex( ListIndex ) )
				{
					m_Items[ListIndex] = Item;
				}
				else
				{
					assert( false ); // Bag not resized properly?
				}
			}
		}
	}

	RefreshGridWidget( m_Items.LenAsUInt() );
}

void ExInventoryBag::SetEquippingFromItem( eg_int ItemIndex )
{
	for( eg_int i=0; i<m_Items.LenAs<eg_int>(); i++ )
	{
		if( m_Items[i].bEquipingFrom && ItemIndex != i )
		{
			m_Items[i].bEquipingFrom = false;
			EGUiGridWidgetItem* Widget = GetWidgetByIndex( i );
			if( Widget )
			{
				Widget->SetMuteAudio( true );
				Widget->RunEvent( GetSelectedIndex() == i ? eg_crc("Select") : eg_crc("Deselect") );
				Widget->SetMuteAudio( false );
			}
		}
		m_Items[i].bEquipingFrom = ItemIndex == i;
		if( m_Items[i].bEquipingFrom )
		{
			EGUiGridWidgetItem* Widget = GetWidgetByIndex( i );
			if( Widget )
			{
				Widget->RunEvent( eg_crc("SelectEquip") );
			}
		}
	}
}

eg_bool ExInventoryBag::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	if( m_Owner && m_bCanDrag )
	{
		EGUiDragAndDropWidget* DragWidget = m_Owner->BeginDragAndDrop( this );
		if( DragWidget )
		{
			return true;
		}
	}

	return Super::OnMousePressed( WidgetHitPoint );
}

void ExInventoryBag::OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
{
	if( ItemInfo.Widget && m_Items.IsValidIndex( ItemInfo.GridIndex ) )
	{
		const exInventoryBagItem& Item = m_Items[ItemInfo.GridIndex];
		eg_string_crc FormatText = CT_Clear;
		eg_string_small IconTexture( "/egdata/textures/default_tblack" );
		eg_string_crc StateEvent = CT_Clear;
		switch( Item.Type )
		{
		case ex_inventory_bag_item_t::None:
			FormatText = eg_loc("InventoryMenuMissing","Fix Me");
			StateEvent = eg_crc("SetStateEmpty");
			break;
		case ex_inventory_bag_item_t::EmptyInventoryItem:
			StateEvent = eg_crc("SetStateEmpty");
			break;
		case ex_inventory_bag_item_t::InventoryItem:
			FormatText = eg_loc("InventoryMenuInventoryItemListName","{0:NAME}");
			IconTexture = Item.InventoryItem.ArmoryItem ? *Item.InventoryItem.GetTierData( nullptr ).Icon.FullPath : "";
			StateEvent = eg_crc("SetStateIcon");
			break;
		}

		ItemInfo.Widget->SetTexture( eg_crc("Icon") , eg_crc("Texture") , IconTexture );
		ItemInfo.Widget->RunEvent( StateEvent );

		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( Item.bEquipingFrom )
		{
			ItemInfo.Widget->RunEvent( ItemInfo.bSelected ? eg_crc("SelectSilent") : eg_crc("SelectEquip") );
		}
	}
}

void exInventoryBagItem::FormatText( eg_cpstr Flags, EGTextParmFormatter* Formatter ) const
{
	switch( Type )
	{
	case ex_inventory_bag_item_t::None:
	case ex_inventory_bag_item_t::EmptyInventoryItem:
		Formatter->SetText( eg_loc_text( eg_loc( "InventoryMenuUnk", "-Unknown Type-" ) ).GetString() );
		break;
	case ex_inventory_bag_item_t::InventoryItem:
		InventoryItem.FormatText( Flags, Formatter );
		break;
	}
}

// (c) 2017 Beem Media

#include "ExInventoryShopMenuBase.h"
#include "ExInventoryBag.h"
#include "EGUiDragAndDropWidget.h"
#include "ExShopData.h"
#include "ExArmoryItem.h"
#include "ExDialogMenu.h"
#include "ExItemCardWidget.h"
#include "ExMenuPortraitsGridWidget.h"

class ExShopMenu : public ExInventoryShopMenuBase
{
	EG_CLASS_BODY( ExShopMenu , ExInventoryShopMenuBase )

private:
	
	enum class ex_buy_mode
	{
		ShopInventory,
		BuyBack,
	};

private:
	
	EGUiGridWidget*             m_ShopItemsList;
	EGUiTextWidget*             m_SoldOutText;
	eg_string_crc               m_ShopInvnetoryId;
	EGArray<exInventoryBagItem> m_ShopInventory;
	ex_buy_mode                 m_BuyMode = ex_buy_mode::ShopInventory;

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		m_ShopItemsList = GetWidget<EGUiGridWidget>( eg_crc("ShopItemsList") );
		m_ItemCard = GetWidget<ExItemCardWidget>( eg_crc("ItemCard") );
		m_SoldOutText = GetWidget<EGUiTextWidget>( eg_crc("SoldOutText") );

		if( m_InventoryListWidget )
		{
			m_InventoryListWidget->SetCanDrag( false );
			m_InventoryListWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnBagItemPressed );
			m_InventoryListWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnBackpackCellSelectionChanged );
		}

		if( m_ShopItemsList )
		{
			m_ShopItemsList->OnItemPressedDelegate.Bind( this , &ThisClass::OnShopItemPressed );
			m_ShopItemsList->OnItemChangedDelegate.Bind( this , &ThisClass::OnShopItemChanged );
		}

		m_ShopInvnetoryId = GetGame()->GetGameVar( eg_crc("NextShopInventory") ).as_crc();

		GetGame()->OnClientRosterChangedDelegate.AddUnique( this , &ThisClass::OnRosterChanged );

		RefreshBags();

		DoReveal();
	}

	virtual void OnDeinit() override final
	{
		GetGame()->OnClientRosterChangedDelegate.RemoveAll( this );

		Super::OnDeinit();
	}

	virtual void OnRevealComplete() override final
	{
		Super::OnRevealComplete();

		m_FallbackEquipIndex = 0;

		SetDragAndDropWidget( GetWidget<EGUiDragAndDropWidget>( eg_crc("DragAndDrop") ) );

		if( m_ShopItemsList && m_InventoryListWidget )
		{
			if( m_ShopInventory.HasItems() )
			{
				SetFocusedWidget( m_ShopItemsList , 0 , false );
			}
			else
			{
				SetFocusedWidget( m_InventoryListWidget , 0 , false );
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

		return Super::OnInput( InputType );
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds )
	{
		if( Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE )  )
		{
			if( m_Portraits )
			{
				const eg_bool bNextPressed = Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE );
				m_Portraits->ChangeSelection( bNextPressed );
				m_FallbackEquipIndex = 0;
				RefreshInfo();
			}

			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_BTN3 ) )
		{
			SortBackpack();
			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_BTN2 ) )
		{
			if( m_BuyMode == ex_buy_mode::BuyBack )
			{
				m_BuyMode = ex_buy_mode::ShopInventory;
			}
			else if( m_BuyMode == ex_buy_mode::ShopInventory )
			{
				m_BuyMode = ex_buy_mode::BuyBack;
			}
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			m_FallbackEquipIndex = 0;
			RefreshBags();
			if( m_ShopInventory.HasItems() )
			{
				SetFocusedWidget( m_ShopItemsList , 0 , false );
			}
			else
			{
				SetFocusedWidget( m_InventoryListWidget , 0 , false );
			}
			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_COMPARE ) )
		{
			exInventoryItem ViewedItem = CT_Clear;
			if( m_ItemCard && m_ItemCard->GetItemBeingViewed().Type == ex_inventory_bag_item_t::InventoryItem )
			{
				ViewedItem = m_ItemCard->GetItemBeingViewed().InventoryItem;
			}
			else
			{
				ViewedItem = CT_Clear;
			}

			ViewedItem.ResolvePointers();
			if( ViewedItem.ArmoryItem && ViewedItem.ArmoryItem->CanEquipToMultipleSlots() )
			{
				const eg_int NumSlots = ViewedItem.ArmoryItem->EquipSlots.GetNumSlots();
				if( NumSlots > 1 )
				{
					m_FallbackEquipIndex = (m_FallbackEquipIndex+1+NumSlots)%NumSlots;
					RefreshDisplayedAttributes();
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
				}
			}

			return;
		}
	}

	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override final
	{
		eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );
		switch_crc( BaseFlag )
		{
			case_crc("NUM_ITEMS"): Formatter->SetNumber( GetGame()->GetRoster().GetInventory().GetNumItems() ); break;
			case_crc("BAG_SIZE"): Formatter->SetNumber( GetGame()->GetRoster().GetInventory().GetBagSize() ); break;
		}
	}

	virtual void OnSelectedCharacterChanged( eg_uint PartyIndex , eg_bool bAllowAudio ) override
	{
		Super::OnSelectedCharacterChanged( PartyIndex , bAllowAudio );

		if( bAllowAudio && PartyIndex != EGUiGrid2::INDEX_NONE)
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}

		if( PartyIndex != EGUiGrid2::INDEX_NONE )
		{
			RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , PartyIndex ) );
		}

		RefreshDisplayedAttributes();
		RefreshHints();
	}

	void OnBackpackCellSelectionChanged( EGUiGridWidget* GridOwner , eg_uint CellIndex )
	{
		unused( GridOwner , CellIndex );
		m_FallbackEquipIndex = 0;
		RefreshInfo();
	}

	void OnBagItemPressed( const egUIWidgetEventInfo& PressInfo )
	{
		ExInventoryBag* Bag = EGCast<ExInventoryBag>( PressInfo.GridWidgetOwner );
		if( Bag && Bag->GetItems().IsValidIndex( PressInfo.GridIndex ) )
		{
			const exInventoryBagItem& BagItem = Bag->GetItems()[PressInfo.GridIndex];
			if( BagItem.Type == ex_inventory_bag_item_t::InventoryItem )
			{
				// Inventory Items can always be sold.
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::PURCHASE );
				SetInputEnabled( false );
				RunServerEvent( egRemoteEvent( eg_crc("SellInventoryItem") , BagItem.InventoryItem.InventoryListIndex ) );
			}
			else
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
			}
		}
	}

	void OnShopItemPressed( const egUIWidgetEventInfo& PressInfo )
	{
		if( m_ShopInventory.IsValidIndex( PressInfo.GridIndex ) )
		{
			const exInventoryBagItem& BagItem = m_ShopInventory[PressInfo.GridIndex];

			ex_can_buy_t CanBuy = GetGame()->CanBuyItem( BagItem.InventoryItem , m_BuyMode == ex_buy_mode::BuyBack );
			if( CanBuy == ex_can_buy_t::CanBuy )
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::PURCHASE );
				SetInputEnabled( false );
				RunServerEvent( egRemoteEvent( eg_crc("MakePurchase") , exShopPurchaseParms( BagItem.InventoryItem , m_BuyMode == ex_buy_mode::BuyBack ) ) );
			}
			else
			{
				eg_loc_text ErrorMessage;

				switch( CanBuy )
				{
				case ex_can_buy_t::CanBuy:
					assert( false );
					break;
				case ex_can_buy_t::TooExpensive:
					ErrorMessage = EGFormat(eg_loc("CantBuyTooExpensive","You need more |SC(EX_GOLD)|gold|RC()| to purchase {0:NAME}.") , &BagItem );
					break;
				case ex_can_buy_t::BagsFull:
					ErrorMessage = EGFormat(eg_loc("CantButNoBagSpace","You need free space in your inventory to purchase {0:NAME}.") , &BagItem );
					break;
				}

				ExDialogMenu_PushDialogMenu( GetClientOwner() , exMessageDialogParms( ErrorMessage ) );
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
			}
		}
	}

	void OnShopItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );

		if( m_ShopInventory.IsValidIndex( ItemInfo.GridIndex ) )
		{
			const exInventoryBagItem& InvItem = m_ShopInventory[ItemInfo.GridIndex];

			ItemInfo.Widget->SetText( eg_crc("ItemName")  , EGFormat( L"{0:NAME}" , &InvItem ) );
			if( m_BuyMode == ex_buy_mode::ShopInventory )
			{
				ItemInfo.Widget->SetText( eg_crc("DescText") , EGFormat( L"{0:COST:BUY}" , &InvItem ) );
			}
			else if( m_BuyMode == ex_buy_mode::BuyBack )
			{
				ItemInfo.Widget->SetText( eg_crc("DescText") , EGFormat( L"{0:COST:BUYBACK}" , &InvItem ) );
			}
			ItemInfo.Widget->SetTexture( eg_crc("Icon") , eg_crc("Texture") , InvItem.InventoryItem.ArmoryItem ? *InvItem.InventoryItem.GetTierData( nullptr ).Icon.FullPath : "" );
		}

		if( ItemInfo.IsNewlySelected() )
		{
			m_FallbackEquipIndex = 0;
			RefreshInfo();
		}
	}

	void OnRosterChanged()
	{
		SetInputEnabled( true );

		RefreshBags();
	}

	void PopulateShopInventory()
	{
		m_ShopInventory.Clear();

		eg_loc_text HeaderText;

		if( m_BuyMode == ex_buy_mode::ShopInventory )
		{
			const ExShopInventory& ShopInventoryList = GetGame()->GetShopInventory( m_ShopInvnetoryId );
			for( const exRawItem& RawItem : ShopInventoryList )
			{
				exInventoryItem InvItem( RawItem );
				InvItem.ResolvePointers();
				exInventoryBagItem NewShopItem( InvItem );
				m_ShopInventory.Append( NewShopItem );
			}
			HeaderText = eg_loc_text(eg_loc("ShopHeaderInventory","Shop Inventory"));
		}
		else if( m_BuyMode == ex_buy_mode::BuyBack )
		{
			const ExBuybackList& BuybackList = GetGame()->GetBuybackList();
			for( const exRawItem& RawItem : BuybackList )
			{
				exInventoryItem InvItem( RawItem );
				InvItem.ResolvePointers();
				exInventoryBagItem NewBuybackItem( InvItem );
				m_ShopInventory.InsertAt( 0 , NewBuybackItem );
			}
			HeaderText = eg_loc_text(eg_loc("ShopHeaderBuyback","Buy-back Inventory"));
		}

		EGUiTextWidget* ShopItemsHeader = GetWidget<EGUiTextWidget>( eg_crc("ShopItemsHeader") );
		if( ShopItemsHeader )
		{
			ShopItemsHeader->SetText( CT_Clear , HeaderText );
		}
	}

	void RefreshBags()
	{
		PopulateShopInventory();

		if( m_InventoryListWidget )
		{
			m_InventoryListWidget->PopulateBag( GetGame() , ExInventoryBag::ex_populate_t::InventoryOnly );
		}

		if( m_ShopItemsList )
		{
			eg_uint ShopListSize = m_ShopInventory.LenAs<eg_uint>();
			m_ShopItemsList->RefreshGridWidget( m_ShopInventory.LenAs<eg_uint>() );

			if( ShopListSize == 0 && m_ShopItemsList == GetFocusedWidget() )
			{
				SetFocusedWidget( m_InventoryListWidget , 0 , false );
			}

			m_ShopItemsList->SetEnabled( ShopListSize > 0 );
		}

		RefreshInfo();
	}

	void RefreshInfo()
	{
		RefreshHints();
		RefreshItemDescription();
		RefreshDisplayedAttributes();
		RefreshSoldOutText();
	}

	void RefreshItemDescription()
	{
		auto UpdateSelectionFor = [this]( const EGArray<exInventoryBagItem>& List , EGUiGridWidget* Widget , eg_bool bIsBuy , eg_bool bIsBuyBack ) -> void
		{
			if( Widget && m_ItemCard )
			{
				eg_size_t SelectedIndex = Widget->GetSelectedIndex();
				if( List.IsValidIndex( SelectedIndex ) && exInventoryBagItem::IsTypeNonEmpty( List[SelectedIndex].Type ) )
				{
					ExItemCardWidget::ex_mode ItemCardMode = ExItemCardWidget::ex_mode::Default;
					
					if( bIsBuy && !bIsBuyBack )
					{
						ItemCardMode = ExItemCardWidget::ex_mode::VendorBuy;
					}
					else if( bIsBuyBack )
					{
						ItemCardMode = ExItemCardWidget::ex_mode::VendorBuyback;
					}
					else
					{
						ItemCardMode = ExItemCardWidget::ex_mode::VendorSell;
					}

					m_ItemCard->SetMode( ItemCardMode );
					m_ItemCard->SetItem( &List[SelectedIndex], GetSelectedPartyMember() );
				}
				else
				{
					m_ItemCard->SetItem( nullptr , nullptr );
				}
			}
		};

		if( m_InventoryListWidget && m_InventoryListWidget == GetFocusedWidget() )
		{
			UpdateSelectionFor( m_InventoryListWidget->GetItems() , m_InventoryListWidget , false , false );
		}
		else if( m_ShopItemsList && m_ShopItemsList == GetFocusedWidget() )
		{
			UpdateSelectionFor( m_ShopInventory , m_ShopItemsList , true , m_BuyMode == ex_buy_mode::BuyBack );
		}
	}

	void RefreshHints()
	{
		ClearHints();

		const eg_bool bIsShopFocused = m_ShopItemsList && m_ShopItemsList == GetFocusedWidget();
		const eg_bool bIsInventoryFocused = m_InventoryListWidget && m_InventoryListWidget == GetFocusedWidget();

		eg_bool bCanBuy = false;
		if( bIsShopFocused && m_ShopInventory.IsValidIndex( m_ShopItemsList->GetSelectedIndex() ) && GetGame() )
		{
			const exInventoryBagItem& BagItem = m_ShopInventory[m_ShopItemsList->GetSelectedIndex()];
			bCanBuy = ex_can_buy_t::CanBuy == GetGame()->CanBuyItem( BagItem.InventoryItem , m_BuyMode == ex_buy_mode::BuyBack );
		}

		eg_bool bCanSell = false;
		if( bIsInventoryFocused && m_InventoryListWidget->GetItems().IsValidIndex( m_InventoryListWidget->GetSelectedIndex() ) )
		{
			bCanSell = m_InventoryListWidget->GetItems()[m_InventoryListWidget->GetSelectedIndex()].Type == ex_inventory_bag_item_t::InventoryItem;
		}

		if( bCanBuy )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("ShopMenuPurchaseHint","Purchase")) );
		}
		if( bCanSell )
		{
			AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("ShopMenuSellHint","Sell")) );
		}
		
		if( m_BuyMode == ex_buy_mode::ShopInventory )
		{
			AddHint( CMDA_MENU_BTN2 , eg_loc_text(eg_loc("ShopMenuChangeShopCategory","Switch to Buy-back")) );
		}
		else if( m_BuyMode == ex_buy_mode::BuyBack )
		{
			AddHint( CMDA_MENU_BTN2 , eg_loc_text(eg_loc("ShopMenuChangeBuyBackCategory","Switch to Shop")) );
		}
		AddHint( CMDA_MENU_BTN3 , eg_loc_text(eg_loc("ShopMenuSort","Sort Backpack")) );
		AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
		AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text(eg_loc("ShopMenuNextParty","Switch Character")) );
		AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("ShopMenuBackOutText","Back")) );
	}

	void RefreshDisplayedAttributes()
	{
		exInventoryItem ItemToDisplay = CT_Clear;

		if( m_ItemCard && m_ItemCard->GetItemBeingViewed().Type == ex_inventory_bag_item_t::InventoryItem )
		{
			ItemToDisplay = m_ItemCard->GetItemBeingViewed().InventoryItem;
		}
		else
		{
			ItemToDisplay = CT_Clear;
		}

		eg_bool bHasItem = ItemToDisplay.RawItem.ItemId.IsNotNull();
		eg_bool bIsRemoving = false;
		ex_item_slot TargetEquipSlot = ex_item_slot::NONE;

		SetupDisplayedAttributes( ItemToDisplay , TargetEquipSlot , bIsRemoving );
	}

	void RefreshSoldOutText()
	{
		if( m_SoldOutText )
		{
			eg_string_crc SoldOutText = CT_Clear;
			switch( m_BuyMode )
			{
				case ex_buy_mode::ShopInventory:
					SoldOutText = eg_loc("ShopMenuSoldOutShopInventory","SOLD OUT\nCheck back tomorrow for new stock.");
					break;
				case ex_buy_mode::BuyBack:
					SoldOutText = eg_loc("ShopMenuSoldOutBuyBack","NO ITEMS");
					break;
			}
			m_SoldOutText->SetText( CT_Clear , eg_loc_text( SoldOutText ) );
			m_SoldOutText->SetVisible( !m_ShopInventory.HasItems() );
		}
	}

	void SortBackpack()
	{
		RunServerEvent( egRemoteEvent( eg_crc("ServerNotifyEvent") , eg_crc("SortInventory") ) );
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
	}
};

EG_CLASS_DECL( ExShopMenu )

// (c) 2016 Beem Media

#include "ExInventoryShopMenuBase.h"
#include "ExDialogMenu.h"
#include "ExQuestItems.h"
#include "EGTextFormat.h"
#include "EGUiGridWidget.h"
#include "ExMenuPortraitsGridWidget.h"
#include "ExInventory.h"
#include "EGTextFormat.h"
#include "EGUiImageWidget.h"
#include "ExArmoryItem.h"
#include "EGUiButtonWidget.h"
#include "EGUiDragAndDropWidget.h"
#include "ExInventoryBag.h"
#include "ExItemCardWidget.h"

class ExInventoryMenu : public ExInventoryShopMenuBase , public IExDialogListener
{
	EG_CLASS_BODY( ExInventoryMenu , ExInventoryShopMenuBase )

private:

	enum class ex_state
	{
		Unknown,
		Browsing,
		Equipping,
		DraggingInventoryItem,
	};

	enum class ex_equip_error
	{
		Success,
		NotEquippable,
		InvalidData,
		RequirementsNotMet,
		NotEnoughBagSpace,
		NothingToUnequip,
	};

	EGUiDragAndDropWidget* m_DragAndDropWidget;
	EGUiImageWidget*       m_MaleSillhouette;
	EGUiImageWidget*       m_FemaleSillhouette;
	exInventoryItem        m_ItemBeingEquipped;
	ex_state               m_State;
	eg_bool                m_bEnableCharacterSwitchAudio;
	eg_bool                m_bDisableRefreshItemCards = false;
	eg_int                 m_DeleteItemIndex = -1;
	ex_item_slot           m_DragAndDropSlotType = ex_item_slot::NONE;

	#define ITEM_SLOT( _var_ , _name_ , _desc_ ) EGUiButtonWidget* m_Slot_##_var_;        
	#include "ExItemSlots.items"

public:

	virtual void OnInit() override final
	{
		Super::OnInit();

		GetGame()->OnClientRosterChangedDelegate.AddUnique( this , &ThisClass::OnRosterChanged );

		m_DragAndDropWidget = GetWidget<EGUiDragAndDropWidget>( eg_crc("DragAndDrop") );
		m_MaleSillhouette = GetWidget<EGUiImageWidget>( eg_crc("MaleSillhouette") );
		m_FemaleSillhouette = GetWidget<EGUiImageWidget>( eg_crc("FemaleSillhouette") );

		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) m_Slot_##_var_ = GetWidget<EGUiButtonWidget>( eg_crc("Slot_"#_var_) ); if( m_Slot_##_var_ ){ m_Slot_##_var_->OnPressedDelegate.Bind( this , &ThisClass::OnCharacterSlotClicked ); }     
		#include "ExItemSlots.items"

		RefreshInventoryList();

		if( m_InventoryListWidget )
		{
			m_InventoryListWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnBackpackItemClicked );
			m_InventoryListWidget->OnFocusGainedDelegate.Bind( this , &ThisClass::OnBackpackItemFocused );
			m_InventoryListWidget->OnFocusLostDelegate.Bind( this , &ThisClass::OnBackpackItemFocused );
			m_InventoryListWidget->CellSelectionChangedDelegate.Bind( this , &ThisClass::OnBackpackSelectionChanged );
			SetFocusedWidget( m_InventoryListWidget->GetId() , 0 , false );
		}

		HideHUD();

		SetState( ex_state::Browsing );

		DoReveal();
		if( m_Portraits )
		{
			m_Portraits->SetVisible( true );
		}
	}

	virtual void OnRevealComplete() override final
	{
		Super::OnRevealComplete();

		OnSelectedCharacterChanged( GetGame()->GetSelectedPartyMemberIndex() , false );
		m_bEnableCharacterSwitchAudio = true;

		if( m_DragAndDropWidget )
		{
			SetDragAndDropWidget( m_DragAndDropWidget );
		}

		m_FallbackEquipIndex = 0;
		RefreshDisplayedAttributes();
		RefreshItemDescription();
		RefreshCanDrag();
	}

	virtual void OnDeinit() override final
	{
		GetGame()->OnClientRosterChangedDelegate.RemoveAll( this );
		
		ShowHUD( true );

		Super::OnDeinit();
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		if( m_State == ex_state::DraggingInventoryItem )
		{
			return true;
		}
		
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			if( m_State == ex_state::Equipping )
			{
				RestoreBrowsingToBackpack( true );
			}
 			else if( m_State == ex_state::Browsing )
			{
				MenuStack_Pop();
			}
			return true;
		}

		return false;
	}

	virtual void OnInputCmds( const struct egLockstepCmds& Cmds )
	{
		if( m_State == ex_state::DraggingInventoryItem )
		{
			return;
		}

		if( HandlePossibleMenuToggle( ExMenu::ex_toggle_menu_t::Inventory , Cmds ) )
		{
			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE ) || Cmds.WasMenuPressed( CMDA_MENU_PREV_PAGE )  )
		{
			if( m_Portraits )
			{
				const eg_bool bNextPressed = Cmds.WasMenuPressed( CMDA_MENU_NEXT_PAGE );
				m_Portraits->ChangeSelection( bNextPressed );
				RefreshItemDescription();
				RefreshCanDrag();
			}

			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_DELETE ) )
		{
			DiscardItem();
			return;
		}

		if( Cmds.WasMenuPressed( CMDA_MENU_BTN3 ) )
		{
			m_FallbackEquipIndex = 0;
			SortBackpack();
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

	void RefreshHints()
	{
		ClearHints();

		if( m_State == ex_state::DraggingInventoryItem )
		{
			return;
		}

		if( m_State == ex_state::Browsing )
		{
			if( m_InventoryListWidget && GetFocusedWidget() == m_InventoryListWidget )
			{
				const eg_int SelectedBagItem = m_InventoryListWidget->GetSelectedIndex();
				if( m_InventoryListWidget->GetItems().IsValidIndex( SelectedBagItem ) )
				{
					const exInventoryBagItem& BagItem = m_InventoryListWidget->GetItems()[SelectedBagItem];
					const ExFighter* PartyMember = GetSelectedPartyMember();
					if( PartyMember )
					{
						if( BagItem.Type == ex_inventory_bag_item_t::InventoryItem )
						{
							if( PartyMember->MeetsRequirementsToEquip( BagItem.InventoryItem ) )
							{
								AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("InventoryMenuEquipHint","Equip")) );
							}
							else
							{
								AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("InventoryMenuSwapHint","Swap")) );
							}
						}
					}
					if( BagItem.Type == ex_inventory_bag_item_t::InventoryItem )
					{
						AddHint( CMDA_MENU_DELETE , eg_loc_text(eg_loc("InventoryMenuDropItem","Discard")) );
					}
				}
			}
			else
			{
				ex_item_slot FocusedSlotType = GetFocusedSlotType();
				if( FocusedSlotType != ex_item_slot::NONE )
				{
					const ExFighter* PartyMember = GetSelectedPartyMember();
					if( PartyMember )
					{
						exInventoryItem ItemInSlot = PartyMember->GetItemInSlot( FocusedSlotType , false );
						if( ItemInSlot.RawItem.ItemId.IsNotNull() )
						{
							exInventoryItem UnequipItem = CT_Clear;
							ex_equip_error EquipError = SimulateEquip( FocusedSlotType , UnequipItem , PartyMember );
							if( EquipError == ex_equip_error::Success )
							{
								AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("InventoryMenuUneqiupHint","Unequip")) );
							}
						}
					}
				}
			}
		}
		else if( m_State == ex_state::Equipping )
		{
			if( GetFocusedWidget() == m_InventoryListWidget )
			{
				AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("InventoryMenuSwapHint","Swap")) );
			}
			else
			{
				AddHint( CMDA_MENU_PRIMARY , eg_loc_text(eg_loc("InventoryMenuEquipToSlotHint","Equip to Slot")) );
			}
		}
		if( m_State == ex_state::Browsing )
		{
			AddHint( CMDA_MENU_BTN3 , eg_loc_text(eg_loc("InventoryMenuSortBag","Sort Backpack")) );
		}
		AddHint( CMDA_MENU_PREV_PAGE , CT_Clear );
		AddHint( CMDA_MENU_NEXT_PAGE , eg_loc_text(eg_loc("InventoryMenuNextParty","Switch Character")) );
		if( m_State == ex_state::Equipping )
		{
			AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("InventoryMenuCancelHint","Cancel")) );
		}
		else if( m_State == ex_state::Browsing )
		{
			AddHint( CMDA_MENU_BACK , eg_loc_text(eg_loc("InventoryMenuCloseHint","Back")) );
		}
	}

	void OnBackpackItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		unused( ItemInfo );
	}

	virtual void OnFocusedWidgetChanged( EGUiWidget* NewFocusedWidget , EGUiWidget* OldFocusedWidget ) override
	{
		Super::OnFocusedWidgetChanged( NewFocusedWidget , OldFocusedWidget );

		RefreshItemDescription();
		RefreshHints();
		RefreshCanDrag();
	}

	void ShowEquipError( ex_equip_error EquipError , const exInventoryBagItem& Item , const ExFighter* PartyMember , eg_bool bUnequipping ) const
	{
		eg_string_crc FormatCrc = eg_loc("InventoryMenuEquipErrorUnk","Unknown error.");

		switch( EquipError )
		{
		case ex_equip_error::Success: 
			return;
		case ex_equip_error::NotEquippable: 
			FormatCrc = eg_loc("InventoryMenuNotEquippable","{0:NAME} cannot be equipped.");
			break;
		case ex_equip_error::InvalidData:
			FormatCrc = eg_loc("InventoryMenuInvalidData","This item had invalid data. Menu logic is incorrect.");
			break;
		case ex_equip_error::RequirementsNotMet:
			FormatCrc = eg_loc("InventoryMenuRequirementsNotMet","{1:NAME} does not meet the requirements to equip {0:NAME}.");
			break;
		case ex_equip_error::NotEnoughBagSpace:
			if( bUnequipping )
			{
				FormatCrc = eg_loc("InventoryMenuNotEnoughBagSpaceToUnequip","There is not enough bag space to remove {0:NAME}.");
			}
			else
			{
				FormatCrc = eg_loc("InventoryMenuNotEnoughBagSpace","There is not enough bag space to equip {0:NAME}. Too many items would be unequipped.");
			}
			break;
		case ex_equip_error::NothingToUnequip:
		{
			return;
		} break;
		}

		eg_loc_text DlgText = EGFormat( FormatCrc , &Item , *PartyMember );

		ExDialogMenu_PushDialogMenu( GetOwnerClient() , exMessageDialogParms( DlgText ) );
	}

	ex_equip_error SimulateEquip( ex_item_slot Slot , const exInventoryItem& InvItem , const ExFighter* Fighter ) const
	{
		if( Fighter && GetGame() )
		{
			const eg_uint NumItems = GetGame()->GetRoster().GetInventory().GetNumItems();
			const eg_uint BagSize = GetGame()->GetRoster().GetInventory().GetBagSize();

			// Make sure slot for the item is correct.
			if( InvItem.RawItem.ItemId.IsNotNull() )
			{
				if( InvItem.ArmoryItem == nullptr )
				{
					assert( false ); // If this isn't to test an unequip the pointers must be resolved.
					return ex_equip_error::InvalidData;
				}

				const eg_bool bSlotValid = InvItem.ArmoryItem->EquipSlots.IsSlotSet( Slot );

				if( !bSlotValid )
				{
					// Technically the menu shouldn't allow this state
					return ex_equip_error::InvalidData;
				}

				if( !Fighter->MeetsRequirementsToEquip( InvItem ) )
				{
					return ex_equip_error::RequirementsNotMet;
				}
			}

			ExFighter SimFighter = *Fighter;
			EGArray<exInventoryItem> ItemsRemoved;
			SimFighter.EquipInventoryItem( Slot , InvItem , ItemsRemoved );
			eg_uint NumItemsRemovedFromBag = InvItem.RawItem.ItemId.IsNotNull() ? 1 : 0;

			if( ( ItemsRemoved.Len() + NumItems - NumItemsRemovedFromBag) > BagSize )
			{
				return ex_equip_error::NotEnoughBagSpace;
			}

			if( InvItem.RawItem.ItemId.IsNull() && ItemsRemoved.Len() == 0 )
			{
				return ex_equip_error::NothingToUnequip;
			}

			return ex_equip_error::Success;
		}
		else
		{
			assert( false ); // Pointers must be resolved.
		}

		return ex_equip_error::InvalidData;
	}

	ex_equip_error DoEquip( const exInventoryItem& InvItem , ex_item_slot Slot , const ExFighter* PartyMember , eg_uint PartyMemberIndex )
	{
		ex_equip_error EquipError = ex_equip_error::NotEquippable;
		if( InvItem.ArmoryItem )
		{
			EquipError = SimulateEquip( Slot , InvItem , PartyMember );
			if( EquipError == ex_equip_error::Success )
			{
				RunServerEvent( egRemoteEvent( eg_crc("EquipInventoryItem") , exEquipEventData( InvItem.InventoryListIndex , PartyMemberIndex , InvItem.ArmoryItem->GetDefaultEquipToSlot() ) ) );
			}
		}
		return EquipError;
	}

	void DoSwap( const exInventoryItem& InvItem , eg_uint ToBagSlot )
	{
		m_bDisableRefreshItemCards = true;

		if( InvItem.InventoryListIndex != ToBagSlot )
		{
			RunServerEvent( egRemoteEvent( eg_crc("SwapInventoryItem") , exSwapEventData( InvItem.InventoryListIndex , ToBagSlot ) ) );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::InventoryEquip );
		}
		else
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}
		RestoreBrowsingToBackpack( false );
		if( m_InventoryListWidget )
		{
			m_InventoryListWidget->SetMuteAudio( true );
			m_InventoryListWidget->SetSelection( ToBagSlot );
			m_InventoryListWidget->SetMuteAudio( false );
		}

		m_bDisableRefreshItemCards = false;
	}

	void BeginEquip( eg_int BagIndex , const exInventoryBagItem& Item )
	{
		ex_equip_error EquipError = ex_equip_error::NotEquippable;
		eg_bool bTooManyItemsUnequipped = false;

		const ExFighter* PartyMember = GetSelectedPartyMember();
		eg_uint PartyMemberIndex = GetSelectedPartyMemberIndex();

		if( Item.Type == ex_inventory_bag_item_t::InventoryItem )
		{
			m_ItemBeingEquipped = Item.InventoryItem;
			SetState( ex_state::Equipping );

			if( m_InventoryListWidget )
			{
				m_InventoryListWidget->SetEquippingFromItem( BagIndex );
			}

			EquipError = ex_equip_error::Success;
		}

		if( EquipError != ex_equip_error::Success )
		{
			ShowEquipError( EquipError , Item , PartyMember , false );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
		}
	}

	void HandleSlotPressed( ex_item_slot Slot , EGUiButtonWidget* Widget )
	{
		unused( Widget );

		const ExFighter* PartyMember = GetSelectedPartyMember();
		eg_uint PartyMemberIndex = GetSelectedPartyMemberIndex();

		if( PartyMember )
		{
			if( m_State == ex_state::Browsing )
			{
				exInventoryItem UnequipItem = CT_Clear;
				ex_equip_error EquipError = SimulateEquip( Slot , UnequipItem , PartyMember );

				if( EquipError == ex_equip_error::Success )
				{
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::InventoryEquip );
					RunServerEvent( egRemoteEvent( eg_crc("EquipInventoryItem") , exEquipEventData( exEquipEventData::UNEQUIP_INDEX , PartyMemberIndex , Slot ) ) );
				}
				else
				{
					ShowEquipError( EquipError , PartyMember->GetItemInSlot( Slot , false ) , PartyMember  , true );
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
				}
			}
			else if( m_State == ex_state::Equipping || m_State == ex_state::DraggingInventoryItem )
			{
				ex_equip_error EquipError = SimulateEquip( Slot , m_ItemBeingEquipped , PartyMember );

				if( EquipError == ex_equip_error::Success )
				{
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::InventoryEquip );
					RunServerEvent( egRemoteEvent( eg_crc("EquipInventoryItem") , exEquipEventData( m_ItemBeingEquipped.InventoryListIndex , PartyMemberIndex , Slot ) ) );
					RestoreBrowsingToBackpack( false );
				}
				else
				{
					ShowEquipError( EquipError , m_ItemBeingEquipped , PartyMember  , false );
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
					RestoreBrowsingToBackpack( false );
				}
			}
		}
	}

	void OnBackpackItemFocused( const egUIWidgetEventInfo& Info )
	{
		unused( Info );
		RefreshItemDescription();
		RefreshCanDrag();
	}

	void OnBackpackSelectionChanged( EGUiGridWidget* GridOwner , eg_uint CellIndex )
	{
		unused( GridOwner , CellIndex );

		m_FallbackEquipIndex = 0;
		RefreshItemDescription();
		RefreshHints();
		RefreshCanDrag();
	}

	void RefreshCanDrag()
	{
		if( m_InventoryListWidget && m_ItemCard )
		{
			m_InventoryListWidget->SetCanDrag( m_State == ex_state::Browsing && m_ItemCard->GetItemBeingViewed().Type == ex_inventory_bag_item_t::InventoryItem );
		}
	}

	void OnBackpackItemClicked( const egUIWidgetEventInfo& Info )
	{
		auto HandleBagItemPressed = [this,&Info]( const EGArray<exInventoryBagItem>& List ) -> void
		{
			if( List.IsValidIndex( Info.GridIndex ) )
			{
				const exInventoryBagItem& Item = List[Info.GridIndex];
				if( Item.Type == ex_inventory_bag_item_t::InventoryItem )
				{
					BeginEquip( Info.GridIndex , Item );
				}
				else
				{
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
				}
			}
		};

		if( m_InventoryListWidget )
		{
			if( m_State == ex_state::Equipping )
			{
				DoSwap( m_ItemBeingEquipped , Info.GridIndex );
			}
			else
			{
				HandleBagItemPressed( m_InventoryListWidget->GetItems() );
			}
		}
	}

	void OnCharacterSlotClicked( const egUIWidgetEventInfo& Info )
	{		
		if( false )
		{
		}
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) else if( m_Slot_##_var_ && Info.WidgetId == m_Slot_##_var_->GetId() ){ HandleSlotPressed( ex_item_slot::##_var_ , m_Slot_##_var_ ); }       
		#include "ExItemSlots.items"
	}

	void RefreshInventoryList()
	{
		if( m_InventoryListWidget )
		{
			m_InventoryListWidget->PopulateBag( GetGame() , ExInventoryBag::ex_populate_t::InventoryOnly );
		}

		SetState( ex_state::Browsing );
		RefreshItemDescription();
		RefreshCanDrag();
	}

	void RefreshItemDescription()
	{
		if( m_bDisableRefreshItemCards )
		{
			return;
		}

		if( nullptr == m_ItemCard )
		{
			return;
		}
		
		auto UpdateSelectionFor = [this]( const EGArray<exInventoryBagItem>& List , EGUiGridWidget* Widget ) -> void
		{
			if( Widget )
			{
				eg_size_t SelectedIndex = Widget->GetSelectedIndex();
				if( List.IsValidIndex( SelectedIndex ) && exInventoryBagItem::IsTypeNonEmpty( List[SelectedIndex].Type ) )
				{
					m_ItemCard->SetItem( &List[SelectedIndex] , GetSelectedPartyMember() );
				}
				else
				{
					m_ItemCard->SetItem( nullptr , GetSelectedPartyMember() );
				}
			}
		};

		auto UpdateSelectionForSlot = [this]( ex_item_slot Slot ) -> void
		{
			const ExFighter* PartyMember = GetSelectedPartyMember();
			if( PartyMember )
			{
				exInventoryItem ItemInSlot = PartyMember->GetItemInSlot( Slot , false );
				if( ItemInSlot.ArmoryItem )
				{
					m_ItemCard->SetItem( &exInventoryBagItem( ItemInSlot ) , PartyMember );
				}
				else
				{
					m_ItemCard->SetItem( nullptr , PartyMember );
				}
			}
			else
			{
				m_ItemCard->SetItem( nullptr , nullptr );
			}
		};

		if( m_State == ex_state::Equipping || m_State == ex_state::DraggingInventoryItem )
		{
			m_ItemCard->SetItem( &exInventoryBagItem( m_ItemBeingEquipped ) , GetSelectedPartyMember() );
		}
		else if( m_InventoryListWidget && m_InventoryListWidget == GetFocusedWidget() )
		{
			UpdateSelectionFor( m_InventoryListWidget->GetItems() , m_InventoryListWidget );
		}
		else
		{
			#define ITEM_SLOT( _var_ , _name_ , _desc_ ) if( m_Slot_##_var_ && m_Slot_##_var_ == GetFocusedWidget() ){ UpdateSelectionForSlot( ex_item_slot::_var_ );    }    
			#include "ExItemSlots.items"
		}

		RefreshDisplayedAttributes();
	}

	void OnRosterChanged()
	{
		RefreshInventoryList();
		if( m_Portraits )
		{
			OnSelectedCharacterChanged( m_Portraits->GetSelectedIndex() , false );
		}

		RestoreBrowsingToBackpack( false );

		RefreshDisplayedAttributes();
		RefreshHints();
	}

	void UpdateSlot( ex_item_slot Slot , EGUiButtonWidget* SlotWidget , const ExFighter* PartyMember )
	{
		if( SlotWidget && PartyMember )
		{
			exInventoryItem ItemInSlot = PartyMember->GetItemInSlot( Slot , true );
			if( ItemInSlot.RawItem.ItemId.IsNotNull() && ItemInSlot.ArmoryItem )
			{
				SlotWidget->RunEvent( eg_crc("HasItem") );
				SlotWidget->SetTexture( eg_crc("Icon") , eg_crc("Texture") , *ItemInSlot.GetTierData( nullptr ).Icon.FullPath );
			}
			else
			{
				ItemInSlot = PartyMember->GetItemInSlot( Slot , false );
				if( ItemInSlot.RawItem.ItemId.IsNotNull() && ItemInSlot.ArmoryItem )
				{
					SlotWidget->RunEvent( eg_crc("Occupied") );
					SlotWidget->SetTexture( eg_crc("Icon") , eg_crc("Texture") , *ItemInSlot.GetTierData( nullptr ).Icon.FullPath );
				}
				else
				{
					SlotWidget->RunEvent( eg_crc("Empty") );
				}
			}
		}
	}

	virtual void OnSelectedCharacterChanged( eg_uint PartyIndex , eg_bool bAllowAudio ) override
	{
		Super::OnSelectedCharacterChanged( PartyIndex , bAllowAudio );
		
		if( m_bEnableCharacterSwitchAudio && bAllowAudio && PartyIndex != EGUiGrid2::INDEX_NONE)
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}
		
		ExGame* Game = GetGame();
		if( Game )
		{
			const ExFighter* PartyMember = Game->GetPartyMemberByIndex( PartyIndex );
			if( PartyMember )
			{
				ex_gender_t Gender = PartyMember->GetGender();
				if( m_MaleSillhouette )
				{
					m_MaleSillhouette->SetVisible( Gender == ex_gender_t::Male );
				}
				if( m_FemaleSillhouette )
				{
					m_FemaleSillhouette->SetVisible( Gender == ex_gender_t::Female );
				}

				#define ITEM_SLOT( _var_ , _name_ , _desc_ ) UpdateSlot( ex_item_slot::_var_ , m_Slot_##_var_ , PartyMember );       
				#include "ExItemSlots.items"
			}
		}

		if( m_State == ex_state::Equipping )
		{
			SetState( ex_state::Browsing );
			if( m_InventoryListWidget )
			{
				SetFocusedWidget( m_InventoryListWidget->GetId() , m_ItemBeingEquipped.InventoryListIndex , false );
			}
		}

		if( PartyIndex != EGUiGrid2::INDEX_NONE )
		{
			RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , PartyIndex ) );
		}

		m_FallbackEquipIndex = 0;
		RefreshDisplayedAttributes();
		RefreshHints();
	}

	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const override final
	{
		if( const ExGame* Game = GetGame() )
		{
			eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );
			switch_crc( BaseFlag )
			{
				case_crc("NUM_ITEMS"): Formatter->SetNumber( Game->GetRoster().GetInventory().GetNumItems() ); break;
				case_crc("BAG_SIZE"): Formatter->SetNumber( Game->GetRoster().GetInventory().GetBagSize() ); break;
			}
		}
	}

	void SetState( ex_state NewState )
	{
		if( m_State == NewState )
		{
			return;
		}

		m_State = NewState;

		EGUiButtonWidget* FirstSlotCanEquipTo = nullptr;

		auto UpdateEnabled = [this,&FirstSlotCanEquipTo]( ex_item_slot Slot , EGUiButtonWidget* Widget , eg_bool bEnabled ) -> void
		{
			unused( Slot );

			if( Widget )
			{
				Widget->SetEnabled( bEnabled );
				Widget->RunEvent( bEnabled ? eg_crc("SetAllowed") : eg_crc("SetNotAllowed") );

				if( bEnabled && FirstSlotCanEquipTo == nullptr )
				{
					FirstSlotCanEquipTo = Widget;
				}
			}
		};

		switch( m_State )
		{
			case ex_state::Unknown:
			{
			} break;
			case ex_state::Browsing:
			{
				#define ITEM_SLOT( _var_ , _name_ , _desc_ ) UpdateEnabled( ex_item_slot::_var_ , m_Slot_##_var_ , true );       
				#include "ExItemSlots.items"

				if( m_InventoryListWidget )
				{
					m_InventoryListWidget->SetEnabled( true );
					m_InventoryListWidget->SetEquippingFromItem( -1 );
				}

			} break;
			case ex_state::DraggingInventoryItem:
			case ex_state::Equipping:
			{		
				if( m_ItemBeingEquipped.ArmoryItem )
				{
					const ex_item_slot SlotToFocus = GetBeginEquipTarget( m_ItemBeingEquipped );
					EGUiButtonWidget* TargetFocusSlot = nullptr;
					const eg_bool bMeetsRequirements = GetSelectedPartyMember() && GetSelectedPartyMember()->MeetsRequirementsToEquip( m_ItemBeingEquipped );
					#define ITEM_SLOT( _var_ , _name_ , _desc_ ) UpdateEnabled( ex_item_slot::_var_ , m_Slot_##_var_ , bMeetsRequirements && m_ItemBeingEquipped.ArmoryItem->CanEquipTo( ex_item_slot::_var_ ) );\
						if( ex_item_slot::_var_ == SlotToFocus ) { TargetFocusSlot = m_Slot_##_var_; }
					#include "ExItemSlots.items"

					if( !bMeetsRequirements && m_State == ex_state::Equipping )
					{
						// Swapping but not equipable, we'll equip first slot that is not selected slot
						const eg_uint SwapSlot = m_ItemBeingEquipped.InventoryListIndex == 0 ? 1 : 0;
						SetFocusedWidget( m_InventoryListWidget , SwapSlot , true );
					}
					else if( TargetFocusSlot && m_State == ex_state::Equipping )
					{
						SetFocusedWidget( TargetFocusSlot->GetId() , 0 , true );
					}
					else if( FirstSlotCanEquipTo && m_State == ex_state::Equipping )
					{
						SetFocusedWidget( FirstSlotCanEquipTo->GetId() , 0 , true );
					}
				}

				if( m_InventoryListWidget )
				{
					m_InventoryListWidget->SetEnabled( true );
				}
			} break;
		}

		RefreshHints();
	}

	virtual void OnDragAndDropStarted( EGUiWidget* SourceWidget , EGUiDragAndDropWidget* DragAndDropWidget ) override final
	{
		if( DragAndDropWidget && m_State == ex_state::Browsing )
		{
			eg_bool bSetIcon = false;
			if( m_InventoryListWidget && SourceWidget == m_InventoryListWidget )
			{
				eg_uint SelectedIndex = m_InventoryListWidget->GetSelectedIndex();
				if( m_InventoryListWidget->GetItems().IsValidIndex( SelectedIndex ) )
				{
					const exInventoryBagItem& DragItem = m_InventoryListWidget->GetItems()[SelectedIndex];
					if( DragItem.Type == ex_inventory_bag_item_t::InventoryItem && DragItem.InventoryItem.ArmoryItem )
					{
						m_ItemBeingEquipped = DragItem.InventoryItem;
						SetState( ex_state::DraggingInventoryItem );
						DragAndDropWidget->SetTexture( eg_crc("Icon") , eg_crc("Texture") , *DragItem.InventoryItem.GetTierData( nullptr ).Icon.FullPath );
						bSetIcon = true;
						m_InventoryListWidget->SetEquippingFromItem( SelectedIndex );
					}
				}
			}

			if( !bSetIcon )
			{
				DragAndDropWidget->RunEvent( eg_crc("SetNoDrag") );
			}
		}
	}

	virtual void OnDragAndDropEnded( EGUiWidget* DroppedOntoWidget , const eg_vec2& WidgetHitPoint ) override final
	{
		unused( WidgetHitPoint );

		if( m_State == ex_state::DraggingInventoryItem )
		{
			ClearDragOnSelection();

			eg_bool bFocusWidget = false;
			
			if( DroppedOntoWidget )
			{
				if( m_InventoryListWidget == DroppedOntoWidget )
				{
					DoSwap( m_ItemBeingEquipped , m_InventoryListWidget->GetIndexByHitPoint( WidgetHitPoint , nullptr ) );
				}
				#define ITEM_SLOT( _var_ , _name_ , _desc_ ) else if( m_Slot_##_var_ == DroppedOntoWidget ){ bFocusWidget = true; HandleSlotPressed( ex_item_slot::##_var_ , m_Slot_##_var_ ); }       
				#include "ExItemSlots.items"
			}

			m_DragAndDropSlotType = ex_item_slot::NONE;

			SetState( ex_state::Browsing );
			if( bFocusWidget )
			{
				SetFocusedWidget( DroppedOntoWidget , 0 , false );
			}
		}
	}

	virtual void OnDragAndDropHovered( EGUiWidget* DroppedOntoWidget ) override final
	{
		if( m_State == ex_state::DraggingInventoryItem )
		{
			const ex_item_slot SlotType = GetSlotTypeOfWidget( DroppedOntoWidget );
			if( m_DragAndDropSlotType != SlotType )
			{
				m_DragAndDropSlotType = SlotType;

				ClearDragOnSelection();

				if( m_ItemBeingEquipped.ArmoryItem && m_ItemBeingEquipped.ArmoryItem->CanEquipTo( SlotType ) )
				{	
					DroppedOntoWidget->RunEvent( eg_crc("SelectDragOn") );
				}

				RefreshDisplayedAttributes();
			}
		}
	}

	void ClearDragOnSelection()
	{
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) if( m_Slot_##_var_ ){ m_Slot_##_var_->RunEvent( eg_crc("Deselect") ); }       
		#include "ExItemSlots.items"
	}

	EGUiButtonWidget* GetFocusedSlot()
	{
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) if( m_Slot_##_var_ && m_Slot_##_var_ == GetFocusedWidget() ){ return m_Slot_##_var_; }       
		#include "ExItemSlots.items"
		return nullptr;
	}

	ex_item_slot GetFocusedSlotType() const
	{
		return GetSlotTypeOfWidget( const_cast<ExInventoryMenu*>(this)->GetFocusedWidget() );
	}

	ex_item_slot GetSlotTypeOfWidget( const EGUiWidget* Widget ) const
	{
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) if( m_Slot_##_var_ && m_Slot_##_var_ == Widget ){ return ex_item_slot::##_var_; }       
		#include "ExItemSlots.items"

		return ex_item_slot::NONE;
	}

	void DiscardItem()
	{
		if( m_State != ex_state::Browsing )
		{
			return;
		}

		const eg_int SelectedInventoryIndex = m_InventoryListWidget && m_InventoryListWidget == GetFocusedWidget() ? m_InventoryListWidget->GetSelectedIndex() : -1;
		if( m_InventoryListWidget->GetItems().IsValidIndex( SelectedInventoryIndex ) )
		{
			const exInventoryBagItem& DeleteItem = m_InventoryListWidget->GetItems()[SelectedInventoryIndex];
			if( DeleteItem.Type == ex_inventory_bag_item_t::InventoryItem )
			{
				m_DeleteItemIndex = DeleteItem.InventoryItem.InventoryListIndex;
				const eg_loc_text ConfirmDeleteText = EGFormat( eg_loc("InventoryMenuConfirmDelete","Are you sure you want to remove {0:NAME} from the inventory? This cannot be undone, you will receive no |SC(EX_GOLD)|gold|RC()|, and the item cannot be retrieved.") , &DeleteItem );
				exYesNoDialogParms DialogParms( ConfirmDeleteText , this , eg_crc("DeleteItem") , true );
				ExDialogMenu_PushDialogMenu( GetClientOwner() , DialogParms );
			}
		}
	}

	void SortBackpack()
	{
		if( m_State == ex_state::Browsing )
		{
			RunServerEvent( egRemoteEvent( eg_crc("ServerNotifyEvent") , eg_crc("SortInventory") ) );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		}
	}

	void RestoreBrowsingToBackpack( eg_bool bAllowAudio )
	{
		if( m_State == ex_state::Equipping || m_State == ex_state::DraggingInventoryItem )
		{
			SetState( ex_state::Browsing );
			if( m_InventoryListWidget )
			{
				SetFocusedWidget( m_InventoryListWidget->GetId() , m_ItemBeingEquipped.InventoryListIndex , bAllowAudio );
			}
		}
	}

	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override
	{
		if( ListenerParm == eg_crc("DeleteItem") )
		{
			if( Choice == eg_crc("Yes") )
			{
				RunServerEvent( egRemoteEvent( eg_crc("DropInventoryItem") , m_DeleteItemIndex ) );
			}
		}
	}

	void RefreshDisplayedAttributes()
	{
		if( m_bDisableRefreshItemCards )
		{
			return;
		}

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
		eg_bool bIsRemoving = bHasItem && m_State == ex_state::Browsing && GetFocusedSlotType() != ex_item_slot::NONE;
		ex_item_slot TargetEquipSlot = ex_item_slot::NONE;

		if( m_State == ex_state::Equipping || bIsRemoving )
		{
			TargetEquipSlot = GetFocusedSlotType();
		}
		if( m_State == ex_state::DraggingInventoryItem )
		{
			if( m_DragAndDropSlotType != ex_item_slot::NONE && ItemToDisplay.ArmoryItem->CanEquipTo( m_DragAndDropSlotType ) )
			{
				TargetEquipSlot = m_DragAndDropSlotType;
			}
		}

		SetupDisplayedAttributes( ItemToDisplay , TargetEquipSlot , bIsRemoving );
	}
};

EG_CLASS_DECL( ExInventoryMenu )

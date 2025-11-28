// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "EGUiGridWidget.h"
#include "ExShopData.h"
#include "ExInventoryBag.h"
#include "ExGlobalData.h"

class ExDebugMenu : public ExMenu
{
	EG_CLASS_BODY( ExDebugMenu , ExMenu )

protected:

	enum class ex_view
	{
		None,
		LevelUpData,
		ShoppingData,

		Last,
	};

protected:

	ex_view m_CurrentView = ex_view::None;
	EGArray<eg_d_string> m_DefaultListDisplayData;

	EGUiGridWidget* m_DefaultListWidget;
	EGUiTextWidget* m_HeaderTextWidget;

	eg_int m_ViewedShopLevel = 1;

protected:

	virtual void OnInit() override
	{
		Super::OnInit();

		m_DefaultListWidget = GetWidget<EGUiGridWidget>( eg_crc("DefaultList") );
		if( m_DefaultListWidget )
		{
			m_DefaultListWidget->CellChangedDelegate.Bind( this , &ThisClass::OnDefaultListCellChanged );
		}
		m_HeaderTextWidget = GetWidget<EGUiTextWidget>( eg_crc("SubmenuText") );

		DoReveal();
	}

	virtual void OnRevealComplete() override
	{
		Super::OnRevealComplete();

		SetView( ex_view::LevelUpData );
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		if( InputType == eg_menuinput_t::NEXT_PAGE || InputType == eg_menuinput_t::PREV_PAGE )
		{
			EGLogf( eg_log_t::General , "Page changed." );

			const eg_bool bNext = InputType == eg_menuinput_t::NEXT_PAGE;
			eg_int NewView = EG_To<eg_int>(m_CurrentView) + (bNext ? 1 : -1);
			if( NewView >= EG_To<eg_int>(ex_view::Last) )
			{
				NewView = EG_To<eg_int>(ex_view::None)+1;
			}
			if( NewView <= EG_To<eg_int>(ex_view::None) )
			{
				NewView = EG_To<eg_int>(ex_view::Last)-1;
			}
			SetView( EG_To<ex_view>(NewView) );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );

			return true;
		}

		if( InputType == eg_menuinput_t::NEXT_SUBPAGE || InputType == eg_menuinput_t::PREV_SUBPAGE )
		{
			const eg_bool bNext = InputType == eg_menuinput_t::NEXT_SUBPAGE;

			if( m_CurrentView == ex_view::ShoppingData )
			{
				if( bNext || m_ViewedShopLevel > 1 )
				{
					m_ViewedShopLevel += bNext ? 1 : -1;
					ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
					SetView( m_CurrentView );
				}
			}

			return true;
		}

		return Super::OnInput( InputType );
	}

	void SetHeaderText( eg_string_crc NewValue )
	{
		if( m_HeaderTextWidget )
		{
			m_HeaderTextWidget->SetText( CT_Clear , eg_loc_text(NewValue) );
		}
	}

	void ResetView()
	{
		SetFocusedWidget( nullptr , 0 , false );
		m_DefaultListDisplayData.Clear( false );
		PopulateDefaultList();
		SetHeaderText( CT_Clear );
	}

	void PopulateDefaultList()
	{
		if( m_DefaultListWidget )
		{
			m_DefaultListWidget->RefreshGridWidget( m_DefaultListDisplayData.LenAs<eg_uint>() );
		}
	}

	void OnDefaultListCellChanged( egUiGridWidgetCellChangedInfo& CellInfo )
	{
		if( m_DefaultListDisplayData.IsValidIndex( CellInfo.GridIndex ) )
		{
			const eg_d_string& DisplayLine = m_DefaultListDisplayData[CellInfo.GridIndex];
			if( CellInfo.GridItem )
			{
				CellInfo.GridItem->SetText( eg_crc("LabelText") , eg_loc_text( *eg_d_string16(*DisplayLine) ) );
			}
		}
	}

	void SetView( ex_view NewView )
	{
		ResetView();

		m_CurrentView = NewView;

		switch( NewView )
		{
		case ex_view::None:
		case ex_view::Last:
			break;
		case ex_view::LevelUpData:
			SetLevelUpDataView();
			break;
		case ex_view::ShoppingData:
			SetShoppingDataView();
			break;
		}
	}

	void SetLevelUpDataView()
	{
		SetHeaderText( eg_loc("LevelUpDataViewHeaderText","Level Up Data") );

		for( eg_int i=0; i<=EX_MAX_LEVEL; i++ )
		{
			const ex_xp_value XpForLevel = ExCore_GetXpNeededForLevel( i );
			const ex_xp_value XpForPrevLevel = ExCore_GetXpNeededForLevel( i-1 );
			const ex_attr_value CostForLevel = ExCore_GetCostToTrainToLevel( i );

			eg_d_string Line = EGSFormat8( "LVL {0}: " , i );
			Line += EGSFormat8( "XP Needed {0} (Delta: {1}), Cost: |SC(EX_GOLD)|{2:PRETTY}|RC()|" , XpForLevel , XpForLevel - XpForPrevLevel , CostForLevel );

			m_DefaultListDisplayData.Append( Line );
		}

		PopulateDefaultList();
		SetFocusedWidget( m_DefaultListWidget , 0 , false );
	}

	void SetShoppingDataView()
	{
		SetHeaderText( eg_loc("ShoppingDataViewHeaderText","Shopping Data") );

		m_DefaultListDisplayData.Append( *EGSFormat8( "Vendor Data Level {0}" , m_ViewedShopLevel ) );

		EGArray<exRawItem> ShopInventory;
		ExShopData::Get().GetInventory( ExGlobalData::Get().GetDefaultShopKeeperInventory() , ShopInventory , m_ViewedShopLevel , 0 );
		for( const exRawItem& ShopItem : ShopInventory )
		{
			exInventoryItem InvItem = ShopItem;
			InvItem.ResolvePointers();
			if( InvItem.ArmoryItem )
			{
				eg_d_string Line = EGSFormat8( "Item {0}: " , m_DefaultListDisplayData.LenAs<eg_int>() );
				Line += EGFormat( L"{0:NAME} {0:COST} {0:BOOSTS:SINGLELINE}" , &InvItem ).GetString();

				m_DefaultListDisplayData.Append( Line );
			}
		}

		PopulateDefaultList();
		SetFocusedWidget( m_DefaultListWidget , 0 , false );
	}
};

EG_CLASS_DECL( ExDebugMenu )

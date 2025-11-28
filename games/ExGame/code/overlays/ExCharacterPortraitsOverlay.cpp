// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "ExPortraits.h"
#include "ExCharacterPortraitWidget.h"
#include "ExClient.h"
#include "EGUiGridWidget2.h"

class ExCharacterPortraitsOverlay: public ExMenu
{
	EG_CLASS_BODY( ExCharacterPortraitsOverlay , ExMenu )

private:

	EGUiGridWidget2*            m_CharacterPortraitsWidget;
	ExCharacterPortraitWidget2* m_PortraitWidgets[ExRoster::PARTY_SIZE];

public:
	
	virtual void Refresh() override final
	{
		if( m_CharacterPortraitsWidget )
		{
			m_CharacterPortraitsWidget->SetEnabled( true );
			m_CharacterPortraitsWidget->SetGridWidgetFocusOnHover( false );
			m_CharacterPortraitsWidget->SetGridWidgetChangeSelectionOnHover( false );
			m_CharacterPortraitsWidget->CellChangedDelegate.Bind( this , &ThisClass::OnPortraitCellChanged );
			m_CharacterPortraitsWidget->CellClickedDelegate.Bind( this , &ThisClass::OnPortraitClicked );
			m_CharacterPortraitsWidget->RefreshGridWidget( ExRoster::PARTY_SIZE );
		}
	}

	virtual void OnHudVisibleChanged(  eg_bool bVisible , eg_bool bPlayTransition )
	{
		auto HandleMeshWidget = [bVisible,bPlayTransition]( EGUiWidget* Widget ) -> void
		{
			if( Widget )
			{
				if( bVisible )
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Show") : eg_crc("ShowNow") );
				}
				else
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Hide") : eg_crc("HideNow") );
				}
			}
		};
		
		for( ExCharacterPortraitWidget2* Widget : m_PortraitWidgets )
		{
			HandleMeshWidget( Widget );
		}
	}

	virtual void OnInit() override final
	{ 
		Super::OnInit();
		m_CharacterPortraitsWidget = GetWidget<EGUiGridWidget2>( eg_crc("CharacterPortraits") );
		if( GetGame() )
		{
			GetGame()->OnClientRosterChangedDelegate.AddUnique( this , &ThisClass::OnRosterChanged );
		}
		Refresh();
	}

	virtual void OnDeinit() override final
	{
		if( GetGame() )
		{
			GetGame()->OnClientRosterChangedDelegate.RemoveAll( this );
		}
		Super::OnDeinit();
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		if( m_CharacterPortraitsWidget )
		{
			m_CharacterPortraitsWidget->SetSelection( GetGame() ? GetGame()->GetSelectedPartyMemberIndex() : 0 );
		}
	}

	void OnPortraitCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
	{
		if( CellInfo.GridItem )
		{
			if( CellInfo.IsNewlySelected() )
			{
				CellInfo.GridItem->RunEvent( eg_crc("Select") );
			}
			if( CellInfo.IsNewlyDeselected() )
			{
				CellInfo.GridItem->RunEvent( eg_crc("Deselect") );
			}
		}

		if( EG_IsBetween<eg_size_t>( CellInfo.GridIndex , 0 , ExRoster::PARTY_SIZE-1 ) )
		{
			m_PortraitWidgets[CellInfo.GridIndex] = EGCast<ExCharacterPortraitWidget2>(CellInfo.GridItem);
			if( m_PortraitWidgets[CellInfo.GridIndex] )
			{
				m_PortraitWidgets[CellInfo.GridIndex]->InitPortrait( nullptr , CellInfo.GridIndex , GetGame() );
			}
		}
	}

	void OnPortraitClicked( egUiGridWidgetCellClickedInfo2& CellInfo )
	{	
		const eg_uint SelectedPartyMember = GetGame() ? GetGame()->GetSelectedPartyMemberIndex() : -1;
		if( CellInfo.GridIndex == SelectedPartyMember )
		{
			RunServerEvent( egRemoteEvent( eg_crc("ServerOpenContextMenu") ) );
		}
		else
		{
			ExFighter* ClickedFighter = GetGame() ? GetGame()->GetPartyMemberByIndex( CellInfo.GridIndex  ) : nullptr;
			if( ClickedFighter )
			{
				RunServerEvent( egRemoteEvent( eg_crc("SetSelectedPartyMember") , CellInfo.GridIndex  ) );
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			}
		}
	}

	void OnRosterChanged()
	{
		Refresh();
	}
};

EG_CLASS_DECL( ExCharacterPortraitsOverlay )

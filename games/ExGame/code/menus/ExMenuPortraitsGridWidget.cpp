// (c) 2017 Beem Media

#include "ExMenuPortraitsGridWidget.h"
#include "ExRoster.h"
#include "ExGame.h"
#include "ExMenu.h"
#include "ExCharacterPortraitWidget.h"

EG_CLASS_DECL( ExMenuPortraitsGridWidget )

void ExMenuPortraitsGridWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	m_Game = InOwner && InOwner->GetClientOwner() ? EGCast<ExGame>(InOwner->GetClientOwner()->SDK_GetGame()) : nullptr;

	OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
	// SetEnabled( false );
	RefreshGridWidget( ExRoster::PARTY_SIZE );
}

void ExMenuPortraitsGridWidget::OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
{
	if( EG_IsBetween<eg_size_t>( ItemInfo.GridIndex , 0 , ExRoster::PARTY_SIZE ) )
	{
		ExCharacterPortraitWidget2* Portrait = EGCast<ExCharacterPortraitWidget2>(ItemInfo.Widget);
		if( Portrait )
		{
			Portrait->InitPortrait( nullptr , ItemInfo.GridIndex , m_Game );
		}
	}

	if( ItemInfo.IsNewlySelected() )
	{
		if( m_FocusedEvent )
		{
			ItemInfo.Widget->RunEvent( m_FocusedEvent );
		}
	}
	else if( ItemInfo.IsNewlyDeselected() )
	{
		if( m_NotFocusedEvent )
		{
			ItemInfo.Widget->RunEvent( m_NotFocusedEvent );
		}
	}
}

eg_bool ExMenuPortraitsGridWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	return Super::OnMousePressed( WidgetHitPoint );
}

eg_bool ExMenuPortraitsGridWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* ReleasedOnWidget )
{
	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		if( this == ReleasedOnWidget  )
		{
			eg_uint PrevSelectedIndex = GetSelectedIndex();
			const eg_uint NewIndex = m_Grid.GetIndexByHitPoint( WidgetHitPoint , nullptr );
			if( m_Game && m_Game->GetPartyMemberByIndex( NewIndex ) != nullptr && NewIndex == m_CaptureGridIndex )
			{
				ChangeSelectionByHitPoint( WidgetHitPoint );
			}
			eg_uint NewSelectedIndex = GetSelectedIndex();
		}
	}
	return Super::OnMouseReleased( WidgetHitPoint , ReleasedOnWidget );;
}

eg_bool ExMenuPortraitsGridWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	if( m_bFocusOnHover && m_Game && !bIsMouseCaptured )
	{
		const eg_uint NewIndex = m_Grid.GetIndexByHitPoint( WidgetHitPoint , nullptr );
		if( EGUiGrid2::INDEX_NONE != NewIndex && m_Game->GetPartyMemberByIndex( NewIndex ) != nullptr )
		{
			m_Grid.SetSelectedIndex( NewIndex );
		}
	}
	return false;
}

eg_bool ExMenuPortraitsGridWidget::HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse )
{
	if( Event == eg_menuinput_t::BUTTON_LEFT || Event == eg_menuinput_t::BUTTON_RIGHT )
	{
		ChangeSelection( Event == eg_menuinput_t::BUTTON_RIGHT );
		return true;
	}
	return Super::HandleInput( Event , WidgetHitPoint , bFromMouse );
}

void ExMenuPortraitsGridWidget::ChangeSelection( eg_bool bInc )
{
	const eg_int GridSize = m_Grid.GetNumItems();
	const eg_int OldSelection = GetSelectedIndex();
	eg_int NewSelection = OldSelection;
	for( eg_int i=1; i<GridSize; i++ )
	{
		const eg_int CompareSelection = (OldSelection + i*(bInc?1:-1) + GridSize)%GridSize;
		if( m_Game && m_Game->GetPartyMemberByIndex( CompareSelection ) != nullptr )
		{
			NewSelection = CompareSelection;
			break;
		}
	}

	if( NewSelection != OldSelection )
	{
		SetSelection( NewSelection );
	}
}

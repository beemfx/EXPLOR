#include "ExToggleWidget.h"
#include "ExUiSounds.h"
#include "EGMenu.h"

EG_CLASS_DECL( ExToggleWidget )

void ExToggleWidget::AddItem( eg_int Value, const eg_loc_text& Text )
{
	egItem NewItem;
	NewItem.Value = Value;
	NewItem.Text = Text;
	m_Items.Append( NewItem );
}

void ExToggleWidget::SetItemByIndex( eg_uint Index )
{
	if( m_Items.IsValidIndex( Index ) )
	{
		m_Selection = Index;
	}
}

eg_int ExToggleWidget::GetSelectedValue() const
{
	return m_Items.IsValidIndex( m_Selection ) ? m_Items[m_Selection].Value : 0;
}

void ExToggleWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime, AspectRatio );

	if( m_Items.IsValidIndex( m_Selection ) )
	{
		SetText( eg_crc( "Text" ), m_Items[m_Selection].Text );
	}
}

void ExToggleWidget::OnFocusGained( eg_bool FromMouse, const eg_vec2& WidgetHitPoint )
{
	Super::OnFocusGained( FromMouse, WidgetHitPoint );

	RunEvent( eg_crc( "SelectArrow" ) );
}

void ExToggleWidget::OnFocusLost()
{
	Super::OnFocusLost();

	RunEvent( eg_crc( "Deselect" ) );
}

eg_bool ExToggleWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	Super::OnMousePressed( WidgetHitPoint );
	m_bBeganCaptureAsInc = WidgetHitPoint.x >= .5f;
	return true;
}

eg_bool ExToggleWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* ReleasedOnWidget )
{
	if( this == ReleasedOnWidget && m_Owner && m_Owner->GetMouseCapture() == this )
	{
		eg_bool bInc = WidgetHitPoint.x >= .5f;
		if( m_bBeganCaptureAsInc == bInc )
		{
			HandleInput( bInc ? eg_menuinput_t::BUTTON_RIGHT : eg_menuinput_t::BUTTON_LEFT, WidgetHitPoint, true );
		}
	}
	return Super::OnMouseReleased( WidgetHitPoint, ReleasedOnWidget );;
}

eg_bool ExToggleWidget::HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint, eg_bool bFromMouse )
{
	switch( Event )
	{
	case eg_menuinput_t::BUTTON_RIGHT:
		if( m_Items.Len() > 0 )
		{
			m_Selection = ( m_Selection + static_cast<eg_uint>(m_Items.Len()) + 1 ) % m_Items.Len();
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}
		else
		{
			m_Selection = 0;
		}
		break;
	case eg_menuinput_t::BUTTON_LEFT:
		if( m_Items.Len() > 0 )
		{
			m_Selection = ( m_Selection + static_cast<eg_uint>(m_Items.Len()) - 1 ) % m_Items.Len();
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}
		else
		{
			m_Selection = 0;
		}
		break;
	default:
		return Super::HandleInput( Event, WidgetHitPoint, bFromMouse );
	}

	return true;
}

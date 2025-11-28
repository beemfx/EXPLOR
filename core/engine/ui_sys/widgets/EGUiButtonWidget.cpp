#include "EGUiButtonWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"

EG_CLASS_DECL( EGUiButtonWidget )

void EGUiButtonWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );
}

EGUiButtonWidget::~EGUiButtonWidget()
{

}

void EGUiButtonWidget::SetFocusEvents( eg_string_crc FocusedEvent, eg_string_crc NotFocusedEvent )
{
	m_FocusedEvent = FocusedEvent;
	m_NotFocusedEvent = NotFocusedEvent;
}

eg_bool EGUiButtonWidget::HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint , eg_bool bFromMouse )
{
	if( eg_menuinput_t::BUTTON_PRIMARY == Event )
	{
		egUIWidgetEventInfo ObjPressInfo( eg_widget_event_t::ItemClicked );
		ObjPressInfo.WidgetId = m_Info->Id;
		ObjPressInfo.HitPoint = WidgetHitPoint;
		ObjPressInfo.bFromMouse = bFromMouse;
		ObjPressInfo.Widget = this;
		OnPressedDelegate.ExecuteIfBound( ObjPressInfo );
		return true;
	}

	return false;
}

eg_bool EGUiButtonWidget::OnMouseMovedOn( const eg_vec2& WidgetHitPoint )
{
	Super::OnMouseMovedOn( WidgetHitPoint );

	if( m_Owner && IsFocusable() && FocusOnHover() )
	{
		m_Owner->SetFocusedWidget( this , 0 , true );
		return true;
	}

	return false;
}

void EGUiButtonWidget::OnMouseMovedOff()
{
	Super::OnMouseMovedOff();
}

eg_bool EGUiButtonWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	unused( WidgetHitPoint );

	if( m_Owner )
	{
		m_Owner->BeginMouseCapture( this );
	}
	return true;
}

eg_bool EGUiButtonWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* WidgetReleasedOn )
{
	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		m_Owner->EndMouseCapture();
		if( this == WidgetReleasedOn )
		{
			HandleInput(eg_menuinput_t::BUTTON_PRIMARY , WidgetHitPoint , true );
		}
	}
	return true;
}

void EGUiButtonWidget::OnFocusGained( eg_bool FromMouse, const eg_vec2& WidgetHitPoint )
{
	unused( FromMouse , WidgetHitPoint );
	egUIWidgetEventInfo ItemInfo( eg_widget_event_t::FocusGained );
	ItemInfo.WidgetId = m_Info->Id;
	ItemInfo.Widget = this;
	ItemInfo.bIsButton = true;
	ItemInfo.bSelected = true;
	ItemInfo.bNewSelection = true;
	if( m_FocusedEvent )
	{
		RunEvent( m_FocusedEvent );
	}
	OnFocusGainedDelegate.ExecuteIfBound( ItemInfo );
}

void EGUiButtonWidget::OnFocusLost()
{
	egUIWidgetEventInfo ItemInfo( eg_widget_event_t::FocusLost );
	ItemInfo.WidgetId = m_Info->Id;
	ItemInfo.Widget = this;
	ItemInfo.bIsButton = true;
	ItemInfo.bSelected = false;
	ItemInfo.bNewSelection = true;
	if( m_NotFocusedEvent )
	{
		RunEvent( m_NotFocusedEvent );
	}
	OnFocusLostDelegate.ExecuteIfBound( ItemInfo );
}

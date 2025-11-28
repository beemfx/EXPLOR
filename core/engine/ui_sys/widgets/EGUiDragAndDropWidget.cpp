// (c) 2017 Beem Media

#include "EGUiDragAndDropWidget.h"
#include "EGMenu.h"

EG_CLASS_DECL( EGUiDragAndDropWidget )

void EGUiDragAndDropWidget::OnInitDragAndDrop()
{
	m_bGetHitByMouse = false;
	SetVisible( false );
	RunEvent( eg_crc("InitDragAndDrop") );
}

void EGUiDragAndDropWidget::OnBeginDrag()
{
	m_bGetHitByMouse = true;
	SetVisible( true );
	RunEvent( eg_crc("BeginDrag") );
	MoveToMousePos();
}

void EGUiDragAndDropWidget::OnDrop()
{
	m_bGetHitByMouse = false;
	SetVisible( false );
	RunEvent( eg_crc("EndDrag") );
}

eg_bool EGUiDragAndDropWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* WidgetReleasedOn )
{
	unused( WidgetHitPoint , WidgetReleasedOn );

	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		m_bGetHitByMouse = false;
		eg_vec2 MousePos = m_Owner->GetMousePos();
		eg_vec2 DroppedOnHitPoint( 0.f , 0.f );
		EGUiWidget* DroppedOnWidget = m_Owner->FindObjectAt( MousePos.x , MousePos.y , &DroppedOnHitPoint , nullptr , true );
		m_Owner->EndDragAndDrop( DroppedOnWidget , DroppedOnHitPoint );
	}
	return true;
}

eg_bool EGUiDragAndDropWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint, eg_bool bIsMouseCaptured )
{
	unused( WidgetHitPoint );

	if( bIsMouseCaptured )
	{
		MoveToMousePos();

		// Handle hovering...
		m_bGetHitByMouse = false;
		eg_vec2 MousePos = m_Owner->GetMousePos();
		eg_vec2 DroppedOnHitPoint( 0.f , 0.f );
		EGUiWidget* DroppedOnWidget = m_Owner->FindObjectAt( MousePos.x , MousePos.y , &DroppedOnHitPoint , nullptr , true );
		if( DroppedOnWidget != m_LastHoveredWidget )
		{
			m_LastHoveredWidget = DroppedOnWidget;
			m_Owner->OnDragAndDropHovered( DroppedOnWidget );
		}
		m_bGetHitByMouse = true;
	}
	return true;
}

void EGUiDragAndDropWidget::MoveToMousePos()
{
	if( m_Owner )
	{
		eg_vec2 MousePos = m_Owner->GetMousePos();
		eg_transform Pose( CT_Default );
		Pose = eg_transform::BuildTranslation( eg_vec3(MousePos.x*MENU_ORTHO_SIZE , MousePos.y*MENU_ORTHO_SIZE , 0.f) );
		SetOffset( EGUiWidget::eg_offset_t::PRE , Pose );
	}
}

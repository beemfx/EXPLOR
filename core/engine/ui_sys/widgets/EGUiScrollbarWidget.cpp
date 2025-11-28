#include "EGUiScrollbarWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiGridWidget2.h"
#include "EGMenu.h"

EG_CLASS_DECL( EGUiScrollbarWidget )

eg_bool EGUiScrollbarWidget::HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint, eg_bool bFromMouse )
{
	unused( WidgetHitPoint );

	if( ( eg_menuinput_t::SCROLL_DOWN == Event || eg_menuinput_t::SCROLL_UP == Event ) )
	{
		EGUiGridWidget* BarsGridWidget = GetGridOfThisScrollbar();
		if( BarsGridWidget )
		{
			BarsGridWidget->HandleInput( Event, eg_vec2( 0.f, 0.f ), bFromMouse );
		}

		EGUiGridWidget2* BarsGridWidget2 = GetGrid2OfThisScrollbar();
		if( BarsGridWidget2 )
		{
			BarsGridWidget2->HandleInput( Event, eg_vec2( 0.f, 0.f ), bFromMouse );
		}

		return true;
	}

	return false;
}

eg_bool EGUiScrollbarWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	if( bIsMouseCaptured )
	{
		HandleScrollbarMouseInteraction( WidgetHitPoint , false );
	}
	return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
}

eg_bool EGUiScrollbarWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	if( m_Owner )
	{
		m_Owner->BeginMouseCapture( this );
		HandleScrollbarMouseInteraction( WidgetHitPoint, true );
	}
	return true;
}

eg_bool EGUiScrollbarWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* WidgetReleasedOn )
{
	unused( WidgetHitPoint , WidgetReleasedOn );

	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		m_Owner->EndMouseCapture();
	}
	return true;
}

eg_aabb EGUiScrollbarWidget::GetBoundsInMouseSpace( eg_real AspectRatio, const eg_mat& MatView, const eg_mat& MatProj ) const
{
	eg_aabb Out = Super::GetBoundsInMouseSpace( AspectRatio , MatView , MatProj );
	// If we're a scrollbar the height may have been changed by a list.
	Out.Min.y -= m_Info->TrackExtend*m_ScaleVector.y / MENU_ORTHO_SIZE;
	return Out;
}

void EGUiScrollbarWidget::HandleScrollbarMouseInteraction( const eg_vec2& ScrollbarHitPoint, eg_bool bCapture )
{
	{
		EGUiGridWidget* GridWidget = EGCast<EGUiGridWidget>( m_Owner->GetWidget( GetGridIdOfThisScrollbar() ) );
		if( GridWidget )
		{
			if( bCapture )
			{
				m_ScrollPosAtCapture = GridWidget->ByScrollbar_GetHitPointPos();
				m_ScrollCapturePoint = ScrollbarHitPoint;
				m_bGrabbedScrollbarHandle = GridWidget->ByScrollbar_IsHitOnHandle( ScrollbarHitPoint );
			}
			eg_vec2 MoveAmount = ScrollbarHitPoint - m_ScrollCapturePoint;
			eg_vec2 ScrollOffset = m_ScrollPosAtCapture + ScrollbarHitPoint - m_ScrollCapturePoint;

			if( m_bGrabbedScrollbarHandle )
			{
				GridWidget->ByScrollbar_ScrollToHitPoint( ScrollOffset );
			}
		}
	}

	{
		EGUiGridWidget2* GridWidget2 = EGCast<EGUiGridWidget2>( m_Owner->GetWidget( GetGridIdOfThisScrollbar() ) );
		if( GridWidget2 )
		{
			if( bCapture )
			{
				m_ScrollPosAtCapture = GridWidget2->ByScrollbar_GetHitPointPos();
				m_ScrollCapturePoint = ScrollbarHitPoint;
				m_bGrabbedScrollbarHandle = GridWidget2->ByScrollbar_IsHitOnHandle( ScrollbarHitPoint );
			}
			eg_vec2 MoveAmount = ScrollbarHitPoint - m_ScrollCapturePoint;
			eg_vec2 ScrollOffset = m_ScrollPosAtCapture + ScrollbarHitPoint - m_ScrollCapturePoint;

			if( m_bGrabbedScrollbarHandle )
			{
				GridWidget2->ByScrollbar_ScrollToHitPoint( ScrollOffset );
			}
		}
	}
}

eg_string_crc EGUiScrollbarWidget::GetGridIdOfThisScrollbar() const
{
	assert( GetType() == EGUiWidget::eg_t::SCROLLBAR );
	return m_Info->GridIdOfThisScrollbar;
}

class EGUiGridWidget* EGUiScrollbarWidget::GetGridOfThisScrollbar() const
{
	return EGCast<EGUiGridWidget>( m_Owner->GetWidget( GetGridIdOfThisScrollbar() ) );
}

class EGUiGridWidget2* EGUiScrollbarWidget::GetGrid2OfThisScrollbar() const
{
	return EGCast<EGUiGridWidget2>( m_Owner->GetWidget( GetGridIdOfThisScrollbar() ) );
}

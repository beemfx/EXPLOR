#pragma once

#include "EGUiMeshWidget.h"


class EGUiScrollbarWidget : public EGUiMeshWidget
{
	EG_CLASS_BODY( EGUiScrollbarWidget, EGUiMeshWidget )

private:

	eg_vec2 m_ScrollPosAtCapture;
	eg_vec2 m_ScrollCapturePoint;
	eg_bool m_bGrabbedScrollbarHandle : 1;

public:

	virtual eg_bool IsWidget() const override { return true; }

	virtual eg_bool HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint, eg_bool bFromMouse ) override;
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override;
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint  ) override;
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn ) override;
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio, const eg_mat& MatView, const eg_mat& MatProj )const override;

	eg_string_crc GetGridIdOfThisScrollbar() const;
	class EGUiGridWidget* GetGridOfThisScrollbar() const;
	class EGUiGridWidget2* GetGrid2OfThisScrollbar() const;

private:

	void HandleScrollbarMouseInteraction( const eg_vec2& ScrollbarHitPoint, eg_bool bCapture );
};

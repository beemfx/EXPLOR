// (c) 2017 Beem Media

#pragma once

#include "EGUiMeshWidget.h"

class EGUiDragAndDropWidget : public EGUiMeshWidget
{
	EG_CLASS_BODY( EGUiDragAndDropWidget , EGUiMeshWidget )

protected:

	EGUiWidget* m_LastHoveredWidget = nullptr;
	eg_bool m_bGetHitByMouse:1;
	
public:

	virtual void OnInitDragAndDrop();
	virtual void OnBeginDrag();
	virtual void OnDrop();

protected:

	// BEGIN EGUiMeshWidget
	virtual eg_bool IsFocusable() const override { return true; }
	virtual eg_bool IsWidget() const override { return m_bGetHitByMouse; }
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn ) override;
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured  ) override;
	// END EGUiMeshWidget

	void MoveToMousePos();
};

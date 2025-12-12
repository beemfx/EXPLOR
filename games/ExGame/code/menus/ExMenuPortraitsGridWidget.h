// (c) 2017 Beem Media

#pragma once

#include "EGUiGridWidget.h"

class ExGame;

class ExMenuPortraitsGridWidget : public EGUiGridWidget
{
	EG_CLASS_BODY( ExMenuPortraitsGridWidget , EGUiGridWidget )

private:

	ExGame* m_Game;
	eg_bool m_bFocusOnHover = false;

public:

	// BEGIN EGUiGridWidget
	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override final;
	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo );
	virtual eg_bool FocusOnHover() const override final { return m_bFocusOnHover; }
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ) override final;
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* ReleasedOnWidget ) override final;
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured ) override final;
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ) override;
	// END EGUiGridWidget

	void SetFocusOnHover( eg_bool bNewValaue ) { m_bFocusOnHover = bNewValaue; }
	void ChangeSelection( eg_bool bInc );
};

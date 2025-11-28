// UI Button Widget Object
// (c) 2016 Beem Media
#pragma once
#include "EGUiMeshWidget.h"

class EGUiButtonWidget: public EGUiMeshWidget
{
	EG_CLASS_BODY( EGUiButtonWidget , EGUiMeshWidget )

private:

	eg_string_crc m_FocusedEvent = eg_crc("Select");
	eg_string_crc m_NotFocusedEvent = eg_crc("Deselect");

public:

	EGUIWidgetEventDelegate OnFocusGainedDelegate;
	EGUIWidgetEventDelegate OnFocusLostDelegate;
	EGUIWidgetEventDelegate OnPressedDelegate;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	~EGUiButtonWidget();

	void SetFocusEvents( eg_string_crc FocusedEvent , eg_string_crc NotFocusedEvent );

	//
	// EGUiOBject Interface:
	//
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ) override;
	virtual eg_bool OnMouseMovedOn( const eg_vec2& WidgetHitPoint ) override;
	virtual void OnMouseMovedOff() override;
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ) override;
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn ) override;
	virtual void OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint ) override;
	virtual void OnFocusLost() override;
	virtual eg_bool IsFocusable() const override { return true; }
	virtual eg_bool FocusOnHover() const override { return true; }
	virtual eg_bool IsWidget() const { return true; }
};
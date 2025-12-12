// (c) 2016 Beem Media
#pragma once

#include "EGUiButtonWidget.h"

class ExToggleWidget : public EGUiButtonWidget
{
	EG_CLASS_BODY( ExToggleWidget , EGUiButtonWidget )

private:
	
	struct egItem
	{
		eg_int            Value;
		eg_loc_text Text;
	};

	EGArray<egItem> m_Items;
	eg_uint         m_Selection;
	eg_bool         m_bBeganCaptureAsInc:1;

public:

	void AddItem( eg_int Value , const eg_loc_text& Text );
	void SetItemByIndex( eg_uint Index );
	eg_int GetSelectedValue() const;

	// EGUiWidget Interface:
	virtual void Update(  eg_real DeltaTime, eg_real AspectRatio ) override;
	virtual void OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint ) override;
	virtual void OnFocusLost() override;
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint ) override;
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* ReleasedOnWidget ) override;
	virtual eg_bool HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint, eg_bool bFromMouse ) override;
};

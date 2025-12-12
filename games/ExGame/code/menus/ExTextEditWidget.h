// (c) 2016 Beem Media
#pragma once

#include "EGUiButtonWidget.h"
#include "EGKbCharHandler.h"
#include "ExKeyboardMenu.h"

class ExTextEditWidget : public EGUiButtonWidget , public IExKeyboardMenuCb , public EGKbCharHandler::IListener
{
	EG_CLASS_BODY( ExTextEditWidget , EGUiButtonWidget )

private:

	EGArray<eg_loc_char> m_AllowedChars;
	EGArray<eg_loc_char> m_Text;
	eg_real              m_BlinkTime;
	eg_uint              m_MaxLength;
	eg_bool              m_bBlinkerOn:1;
	eg_bool              m_bIsFocused:1;

public:
	ExTextEditWidget();

	virtual void Update(  eg_real DeltaTime, eg_real AspectRatio ) override final;
	virtual void OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint ) override final;
	virtual void OnFocusLost() override final;
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ) override final;
	
	virtual eg_bool HandleTypedChar( eg_char16 Char ) override final;

	void InitWidget( const eg_loc_char* AllowedChars , eg_uint MaxLength );
	void SetText( const eg_loc_char* NewText );
	void GetText( eg_loc_char* Out , eg_size_t OutSize );
	eg_bool HasText() const;

	virtual void OnTextFromKeyboardMenu( const eg_loc_char* String ) override final;

private:

	void RefreshView();
};

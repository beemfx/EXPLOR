// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"

class ExFullScreenLoadingMenu: public ExMenu
{
	EG_CLASS_BODY( ExFullScreenLoadingMenu , ExMenu )

private:

	EGUiWidget* m_Bg;
	EGUiWidget* m_Needle;
	EGUiTextWidget* m_LoadingText;
	EGUiTextWidget* m_LoadingHintText;
	eg_real m_Countdown;
	eg_real m_TotalTime = 0.f;
	eg_bool m_bHasSwitchedMusic = false;
	EGRandom m_Rng = CT_Default;
	eg_bool m_bDontSwitchMusic = false;
	eg_bool m_bHasEverBeenReady = false;
	eg_bool m_bOnlyFadeOut = false;
	eg_bool m_bHasTriggeredLoadComplete = false;

public:

	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final;
	void SwitchToNewMapMusic();

	void SetDontSwitchMusic( eg_bool bNewValue ) { m_bDontSwitchMusic = bNewValue; }
	void SetOnlyFadeOut( eg_bool bNewValue );

	static eg_bool IsGameLoadedAndReady( EGClient* Client );
};
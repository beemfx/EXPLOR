// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGUiTextWidget.h"

class ExHudTextWidget : public EGUiTextWidget
{
	EG_CLASS_BODY( ExHudTextWidget , EGUiTextWidget )

protected:

	struct exAlphaTween
	{
		eg_real StartValue = 0.f;
		eg_real EndValue = 0.f;
		eg_real Duration = 0.f;
		eg_real TimeElapsed = 0.f;
		eg_bool bActive = false;
	};

	exAlphaTween m_AlphaTween;

public:

	void HudPlayShow();
	void HudPlayHide();
	void HudShowNow();
	void HudHideNow();

protected:

	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override;
};

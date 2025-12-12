// (c) 2017 Beem Media

#pragma once

#include "ExMenu.h"

class ExScriptMenu : public ExMenu
{
	EG_CLASS_BODY( ExScriptMenu , ExMenu )

protected:

	eg_bool m_bWaitingForServer = false;
	eg_bool m_bWasDismissed = false;
	eg_real m_DismissTime = 0.f;

public:
	
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;

	virtual void Refresh() override;
	virtual void DismissMenu();
	virtual void Continue(); // Send when the server has processed the choice.

protected:

	void SendChoiceToServer( eg_uint ChoiceIndex );
};


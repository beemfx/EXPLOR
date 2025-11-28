// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"

class ExEndGameFlowMenu : public ExMenu
{
	EG_CLASS_BODY( ExEndGameFlowMenu , ExMenu )

private:

	enum class ex_flow_s
	{
		None,
		WaitForAudioToEnd,
		EndingMovie,
		Credits,
		Congratulations,
		ReturnToTown,
		Done,
	};

private:

	ex_flow_s m_State = ex_flow_s::None;
	eg_real m_TimeElapsed = 0.f;

private:

	virtual void OnInit() override;
	virtual void OnActivate() override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;
	void AdvanceToNextState();
};



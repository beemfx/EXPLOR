#pragma once

#include "EGEngineInst.h"

class ExEngineInst : public EGEngineInst
{
	EG_CLASS_BODY( ExEngineInst , EGEngineInst )

private:

	static eg_s_string_sml8 s_BuildVersion;

public:

	virtual void OnGameEvent( eg_game_event Event ) override final;
	virtual void OnRegisterInput( class ISdkInputRegistrar* Registrar ) override final;
	virtual void SetupViewports( eg_real ScreenAspect, egScreenViewport* aVpOut, eg_uint VpCount )const override;

	static eg_s_string_sml8 GetBuildVersion() { return s_BuildVersion; }
};
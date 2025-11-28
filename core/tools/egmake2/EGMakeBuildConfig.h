// (c) 2018 Beem Media

#pragma once

#include "EGBuildConfig.h"
#include "EGXMLBase.h"

class EGMakeBuildConfig : public EGBuildConfig
{
private:

	using EGBuildConfig::LoadConfig;

	static EGMakeBuildConfig s_Inst;

public:

	static EGMakeBuildConfig& Get() { return s_Inst; }

	void LoadGameConfig( eg_cpstr GameName );
};
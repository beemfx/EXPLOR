// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

bool EGPlatform_IsWinSteam();

#if IS_PLATFORM_Steam && (defined( __WIN64__ ) || defined( __WIN32__ ))

#include "EGPlatformWindows.h"

class EGPlatformWinSteam : public EGPlatformWindows
{
	EG_DECL_SUPER( EGPlatformWindows )

public:

	virtual eg_platform_init_result Init( eg_cpstr GameName ) override;
	virtual void Deinit() override;

	virtual void Update( eg_real DeltaTime ) override;
	virtual void UnlockAchievement( eg_cpstr8 AchId ) override;
	virtual void ResetAchievement( eg_cpstr8 AchId ) override;
	virtual void SetStatsVersion( eg_int StatsVersionValue ) override;
};

#endif

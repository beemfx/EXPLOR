// (c) 2020 Beem Media. All Rights Reserved.

#include "EGPlatformWinSteam.h"

bool EGPlatform_IsWinSteam()
{
#if IS_PLATFORM_Steam && (defined( __WIN64__ ) || defined( __WIN32__ ))
	return true;
#else
	return false;
#endif
}

#if IS_PLATFORM_Steam && (defined( __WIN64__ ) || defined( __WIN32__ ))

#include "EGSteamworks.h"

eg_platform_init_result EGPlatformWinSteam::Init( eg_cpstr GameName )
{
	const eg_platform_init_result WinInitResult = Super::Init( GameName );
	if( eg_platform_init_result::Success != WinInitResult )
	{
		return WinInitResult;
	}

	eg_bool bThirdPartConnected = true;
	eg_bool bPlatformNeedsReset = false;
	bPlatformNeedsReset = EGSteamworks_PreInit_NeedsRestart();

	if( !bPlatformNeedsReset )
	{
		bThirdPartConnected = EGSteamworks_Init( this );
	}

	if( !bThirdPartConnected || bPlatformNeedsReset )
	{
		return bPlatformNeedsReset ? eg_platform_init_result::Restarting3rdParty : eg_platform_init_result::Failed3rdParty;
	}

	return eg_platform_init_result::Success;
}

void EGPlatformWinSteam::Deinit()
{
	EGSteamworks_Deinit();
}

void EGPlatformWinSteam::Update( eg_real DeltaTime )
{
	Super::Update( DeltaTime );

	EGSteamworks_Update( DeltaTime );
}

void EGPlatformWinSteam::UnlockAchievement( eg_cpstr8 AchId )
{
	Super::UnlockAchievement( AchId );

	EGSteamworks_UnlockAchievement( AchId );
}

void EGPlatformWinSteam::ResetAchievement( eg_cpstr8 AchId )
{
	Super::ResetAchievement( AchId );

	EGSteamworks_ResetAchievement( AchId );
}

void EGPlatformWinSteam::SetStatsVersion( eg_int StatsVersionValue )
{
	Super::SetStatsVersion( StatsVersionValue );

	EGSteamworks_SetStatsVersion( StatsVersionValue );
}

#endif

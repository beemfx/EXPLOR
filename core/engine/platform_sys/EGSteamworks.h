// (c) 2019 Beem Media. All Rights Reserved.

#pragma once

#if IS_PLATFORM_Steam

class EGPlatformWinSteam;

eg_bool EGSteamworks_PreInit_NeedsRestart();
eg_bool EGSteamworks_Init( EGPlatformWinSteam* InOwner );
void EGSteamworks_Deinit();
void EGSteamworks_Update( eg_real DeltaTime );
void EGSteamworks_UnlockAchievement( eg_cpstr8 AchId );
void EGSteamworks_ResetAchievement( eg_cpstr8 AchId );
void EGSteamworks_ResetStats( bool bAchievementsToo );
void EGSteamworks_SetStatsVersion( eg_int StatsVersionValue );

#endif

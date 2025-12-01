// (c) 2019 Beem Media. All Rights Reserved.

#include "EGSteamworks.h"

#if IS_PLATFORM_Steam

#include "EGGlobalConfig.h"
#include "EGOsFile.h"
#include "EGFileData.h"
#include "EGGlobalConfig.h"
#include "EGLibFile.h"
#include "EGPlatformWinSteam.h"

#pragma warning(push)
#pragma warning(disable:4996)
#include "steam/steam_api.h"
#pragma warning(pop)

#pragma comment(lib,"steam_api64.lib")

class EGSteamworks
{
private:

	STEAM_CALLBACK( EGSteamworks , OnUserStatsReceived , UserStatsReceived_t );
	STEAM_CALLBACK( EGSteamworks , OnUserStatsStored , UserStatsStored_t );
	STEAM_CALLBACK( EGSteamworks , OnGameOverlayActivated , GameOverlayActivated_t );

	EGPlatformWinSteam*const m_Owner;
	EGArray<eg_s_string_sml8> m_AchievementBuffer;
	EGArray<eg_s_string_sml8> m_ClearAchievementBuffer;

	eg_bool m_bGotInitialStats = false;
	eg_int m_StatsVersionValue = 0;

public:

	EGSteamworks( EGPlatformWinSteam* InOwner )
	: m_Owner( InOwner )
	{
		InitUser();
	}

	~EGSteamworks()
	{
	}

	void Update( eg_real DeltaTime )
	{
		unused( DeltaTime );
		
		SteamAPI_RunCallbacks();
	}

	void UnlockAchievement( eg_cpstr8 AchId )
	{
		m_AchievementBuffer.AppendUnique( AchId );
		ProcessAchievmentBuffer();
	}

	void ClearAchievement( eg_cpstr8 AchId )
	{
		m_ClearAchievementBuffer.AppendUnique( AchId );
		ProcessAchievmentBuffer();
	}

	void ResetStats( bool bAchievementsToo )
	{
		ISteamUser* User = SteamUser();
		if( User )
		{
			if( User->BLoggedOn() )
			{
				ISteamUserStats* UserStats = SteamUserStats();
				if( UserStats )
				{
					UserStats->ResetAllStats( bAchievementsToo );
				}
			}
		}
	}

	void SetStatsVersion( eg_int StatsVersionValue )
	{
		m_StatsVersionValue = StatsVersionValue;
		ProcessStatsVersion();
	}

private:

	void InitUser()
	{
		ISteamUser* User = SteamUser();
		if( User )
		{
			if( User->BLoggedOn() )
			{
				ISteamUserStats* UserStats = SteamUserStats();
				if( UserStats )
				{
					eg_bool bMadeRequest = UserStats->RequestCurrentStats();
					if( bMadeRequest )
					{
						EGLogf( eg_log_t::General , "Made stats request." );
					}
				}
			}
		}
	}

	void ProcessAchievmentBuffer()
	{
		ISteamUserStats* UserStats = SteamUserStats();
		if( m_bGotInitialStats && UserStats )
		{
			for( const eg_s_string_sml8& Str : m_AchievementBuffer )
			{
				UserStats->SetAchievement( *Str );
				if( UserStats->StoreStats() )
				{
					EGLogf( eg_log_t::General , "Stored achievement %s" , *Str );
				}
			}

			m_AchievementBuffer.Clear();

			for( const eg_s_string_sml8& Str : m_ClearAchievementBuffer )
			{
				UserStats->ClearAchievement( *Str );
				if( UserStats->StoreStats() )
				{
					EGLogf( eg_log_t::General , "Cleared achievement %s" , *Str );
				}
			}

			m_ClearAchievementBuffer.Clear();
		}
	}

	void ProcessStatsVersion()
	{
		if( m_bGotInitialStats && m_StatsVersionValue > 0 )
		{
			if( ISteamUser* User = SteamUser() )
			{
				if( User->BLoggedOn() )
				{
					if( ISteamUserStats* UserStats = SteamUserStats() )
					{
						int32 SteamStatsVersion = 0;
						if( UserStats->GetStat( "StatsVersion" , &SteamStatsVersion ) )
						{
							if( m_StatsVersionValue != SteamStatsVersion )
							{
								SteamStatsVersion = m_StatsVersionValue;
								UserStats->ResetAllStats( true );
								if( UserStats->SetStat( "StatsVersion" , SteamStatsVersion ) )
								{
									if( UserStats->StoreStats() )
									{
										EGLogf( eg_log_t::General , "Steamworks: Stats Version Changed" );
									}
								}
							}
						}
					}
				}
			}
		}
	}
};

void EGSteamworks::OnUserStatsReceived( UserStatsReceived_t* Callback )
{
	if( Callback && Callback->m_nGameID == EGString_ToUInt( *GlobalConfig_PlatformAppId.GetValue() ) )
	{
		if( k_EResultOK == Callback->m_eResult )
		{
			m_bGotInitialStats = true;
			ProcessAchievmentBuffer();
			ProcessStatsVersion();
		}
	}
}

void EGSteamworks::OnUserStatsStored( UserStatsStored_t* Callback )
{
	if( Callback && Callback->m_nGameID == EGString_ToUInt( *GlobalConfig_PlatformAppId.GetValue() ) )
	{
		if( k_EResultOK == Callback->m_eResult )
		{

		}
	}
}

void EGSteamworks::OnGameOverlayActivated( GameOverlayActivated_t* Callback )
{
	if( Callback )
	{
		if( m_Owner )
		{
			m_Owner->PlatformOverlayActivatedDelegate.Broadcast( Callback->m_bActive != 0 );
		}
	}
}

static eg_byte SteamworksMem[sizeof(EGSteamworks)];
static EGSteamworks* Steamworks = nullptr;

eg_bool EGSteamworks_PreInit_NeedsRestart()
{
#if defined( __EGEDITOR__ )
	{
		const eg_bool bInCorrectDir = EGOsFile_DoesFileExist( L"egame.ini" );
		if( bInCorrectDir )
		{
			// Write the AppId
			EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
			FileData.WriteStr8( *GlobalConfig_PlatformAppId.GetValue() );
			eg_cpstr16 AppIdFilename = L"steam_appid.txt";
			const eg_bool bHasSteamAppId = false;// EGOsFile_DoesFileExist( AppIdFilename );
			if( !bHasSteamAppId )
			{
				EGLibFile_SaveFile( AppIdFilename , eg_lib_file_t::OS , FileData );
			}
		}
	}
#endif

	const eg_bool bShouldRestart = SteamAPI_RestartAppIfNecessary( EGString_ToUInt( *GlobalConfig_PlatformAppId.GetValue() ) );
	
	return bShouldRestart;

}

eg_bool EGSteamworks_Init( EGPlatformWinSteam* InOwner )
{
	const eg_bool bSteamInitialized = SteamAPI_Init();
	if( bSteamInitialized )
	{
		EGLogf( eg_log_t::General , "Steam was initialized." );

		Steamworks = new (SteamworksMem) EGSteamworks( InOwner );
	}
	else
	{
		EGLogf( eg_log_t::Warning , "Steam was not initialized." );
	}
#if defined( __EGEDITOR__ )
	return true;
#else
	return bSteamInitialized;
#endif
}

void EGSteamworks_Deinit()
{
	if( Steamworks )
	{
		Steamworks->~EGSteamworks();
		Steamworks = nullptr;
	}
	SteamAPI_Shutdown();
}

void EGSteamworks_Update( eg_real DeltaTime )
{
	if( Steamworks )
	{
		Steamworks->Update( DeltaTime );
	}
}

void EGSteamworks_UnlockAchievement( eg_cpstr8 AchId )
{
	if( Steamworks )
	{
		Steamworks->UnlockAchievement( AchId );
	}
}

void EGSteamworks_ResetAchievement( eg_cpstr8 AchId )
{
	if( Steamworks )
	{
		Steamworks->ClearAchievement( AchId );
	}
}

void EGSteamworks_ResetStats( bool bAchievementsToo )
{
	if( Steamworks )
	{
		Steamworks->ResetStats( bAchievementsToo );
	}
}

void EGSteamworks_SetStatsVersion( eg_int StatsVersionValue )
{
	if( Steamworks )
	{
		Steamworks->SetStatsVersion( StatsVersionValue );
	}
}

#endif

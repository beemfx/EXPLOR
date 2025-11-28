/******************************************************************************
	Platform - A module to put in utility functions that have different
	implementations on different platforms.

	(c) 2015 Beem Software
******************************************************************************/

#pragma once

#include "EGDelegate.h"

class EGPlatform;

enum class eg_platform_init_result
{
	Success,
	Failed3rdParty,
	Restarting3rdParty,
	BadDirectoryOrData,
	NoPlatform,
	UnknownFailure,
};

struct egEnginePlatformParms
{
	eg_d_string16 ExeDir;
	eg_d_string16 UserDir;
	eg_d_string16 SysCfgDir;
	eg_string_small StartMap;
	eg_string_small RemoteHost;
	eg_bool         IsDedicatedServer:1;

	egEnginePlatformParms( eg_ctor_t Ct )
		: StartMap( Ct )
		, RemoteHost( Ct )
		, IsDedicatedServer( false )
		, ExeDir( Ct )
		, UserDir( Ct )
		, SysCfgDir( Ct )
	{
		assert( Ct == CT_Clear || Ct == CT_Default );
	}
};

eg_bool EGPlatform_IsMainThread();
void EGPlatform_GetRunParms( struct egEnginePlatformParms* RunParmsOut );

eg_platform_init_result EGPlatform_Init( eg_cpstr GameName );
void EGPlatform_Deinit();
EGPlatform& EGPlatform_GetPlatform();

class EGPlatform
{
public:

	EGMCDelegate<eg_bool /* bActive */> PlatformOverlayActivatedDelegate;

public:

	virtual ~EGPlatform() { }

	virtual eg_platform_init_result Init( eg_cpstr GameName ){ unused( GameName ); return eg_platform_init_result::NoPlatform; }
	virtual void Deinit() { }

	virtual void Update( eg_real DeltaTime ) { unused( DeltaTime ); }
	virtual void UnlockAchievement( eg_cpstr8 AchId ) { unused( AchId ); }
	virtual void ResetAchievement( eg_cpstr8 AchId ) { unused( AchId ); }
	virtual void SetStatsVersion( eg_int StatsVersionValue ) { unused( StatsVersionValue ); }
};


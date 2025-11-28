// (c) 2014 Beem Media. All Rights Reserved.

#include "EGPlatform.h"
#include "EGSettings2Types.h"
#include "EGTargetBuildPlatform.generated.hpp"

static EGPlatform* EGPlatform_PlatformInstance = nullptr;

static EGSettingsInt EGPlatform_StatsVersion( "EGPlatform.StatsVersion" , CT_Clear , 0 , 0 );

#if defined( __WIN32__ ) || defined( __WIN64__ )

#if IS_PLATFORM_Steam

#include "EGPlatformWinSteam.h"
#include "EGWindowsAPI.h"

static eg_byte EGPlatform_PlatformInstanceMem[sizeof(EGPlatformWinSteam)];

eg_platform_init_result EGPlatform_Init( eg_cpstr GameName )
{
	eg_platform_init_result Result = eg_platform_init_result::UnknownFailure;

	EGPlatform_PlatformInstance = new ( &EGPlatform_PlatformInstanceMem ) EGPlatformWinSteam;
	if( EGPlatform_PlatformInstance )
	{
		Result = EGPlatform_PlatformInstance->Init( GameName );
	}

	return Result;
}

void EGPlatform_Deinit()
{
	if( EGPlatform_PlatformInstance )
	{
		EGPlatform_PlatformInstance->Deinit();
		EGPlatform_PlatformInstance->~EGPlatform();
	}
	EGPlatform_PlatformInstance = nullptr;
}

#else

#include "EGPlatformWindows.h"
#include "EGWindowsAPI.h"

static eg_byte EGPlatform_PlatformInstanceMem[sizeof(EGPlatformWindows)];

eg_platform_init_result EGPlatform_Init( eg_cpstr GameName )
{
	eg_platform_init_result Result = eg_platform_init_result::UnknownFailure;

	EGPlatform_PlatformInstance = new ( &EGPlatform_PlatformInstanceMem ) EGPlatformWindows;
	if( EGPlatform_PlatformInstance )
	{
		Result = EGPlatform_PlatformInstance->Init( GameName );
	}

	return Result;
}

void EGPlatform_Deinit()
{
	if( EGPlatform_PlatformInstance )
	{
		EGPlatform_PlatformInstance->Deinit();
		EGPlatform_PlatformInstance->~EGPlatform();
	}
	EGPlatform_PlatformInstance = nullptr;
}

#endif

#else

#pragma __WARNING__( "No platform set for "__FILE__"." )

#endif



EGPlatform& EGPlatform_GetPlatform()
{
	return *EGPlatform_PlatformInstance;
}
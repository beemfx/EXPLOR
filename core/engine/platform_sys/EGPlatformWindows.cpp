// (c) 2020 Beem Media. All Rights Reserved.

#include "EGPlatformWindows.h"

bool EGPlatform_IsWindows()
{
#if defined( __WIN64__ ) || defined( __WIN32__ )
	return true;
#else
	return false;
#endif
}

#if defined( __WIN64__ ) || defined( __WIN32__ )

#include "EGWindowsAPI.h"

eg_platform_init_result EGPlatformWindows::Init( eg_cpstr GameName )
{
	unused( GameName );

	return eg_platform_init_result::Success;
}

#endif

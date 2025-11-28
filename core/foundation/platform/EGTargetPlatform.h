// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

// Platform information is to be used for tools only in generating the
// final build of the game, a game itself should generally not be
// concerned with other platform types.

enum class eg_target_platform_t
{
#define EGDECL_PLATFORM_TYPE( _name_ ) _name_ ,
	#include "EGTargetPlatform.items.h"
#undef EGDECL_PLATFORM_TYPE
};

static const eg_cpstr EGTargetPlatform_Names[] = 
{
	#define EGDECL_PLATFORM_TYPE( _name_ ) #_name_ ,
	#include "EGTargetPlatform.items.h"
	#undef EGDECL_PLATFORM_TYPE
};

//
// These functions are for tools only
//

static inline eg_size_t EGTargetPlatform_GetNumPlatforms()
{
	return countof(EGTargetPlatform_Names);
}

static inline eg_cpstr EGTargetPlatform_GetPlatformName( eg_target_platform_t PlatformType )
{
	eg_size_t Index = static_cast<eg_size_t>(PlatformType);
	if( EG_IsBetween<eg_size_t>( Index , 0 , EGTargetPlatform_GetNumPlatforms()-1 ) )
	{
		return EGTargetPlatform_Names[Index];
	}

	return "";
}

static inline eg_cpstr EGTargetPlatform_GetPlatformNameByIndex( eg_size_t PlatformIndex )
{
	return EGTargetPlatform_GetPlatformName( static_cast<eg_target_platform_t>(PlatformIndex) );
}

static inline eg_target_platform_t EGTargetPlatform_GetPlatformFromName( eg_cpstr PlatformName )
{
	for( eg_size_t i=0; i<EGTargetPlatform_GetNumPlatforms(); i++ )
	{
		if( EGString_EqualsI( PlatformName , EGTargetPlatform_Names[i] ) )
		{
			return static_cast<eg_target_platform_t>(i);
		}
	}

	return eg_target_platform_t::Default;
}
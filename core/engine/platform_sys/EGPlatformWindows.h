// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

bool EGPlatform_IsWindows();

#if defined( __WIN64__ ) || defined( __WIN32__ )

#include "EGPlatform.h"

class EGPlatformWindows : public EGPlatform
{
	EG_DECL_SUPER( EGPlatform )

public:

	virtual eg_platform_init_result Init( eg_cpstr GameName ) override;
};

#endif

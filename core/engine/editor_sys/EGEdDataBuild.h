// (c) 2018 Beem Media

#pragma once

#if !defined( __EGEDITOR__ )
#pragma __ERROR__("Cannot include EGEdDataBuild.h in non-editor builds.")
static_assert( false , "Cannot include EGEdDataBuild.h in non-editor builds.h" );
#endif

int EGEdDataBuild_Run( eg_cpstr16 Filename , const struct egSdkEngineInitParms& EngineParms );

// (c) 2017 Beem Media

#pragma once

#if !defined( __EGEDITOR__ )
#pragma __ERROR__("Cannot include EGDataAssetEditor.h in non-editor builds.")
static_assert( false , "Cannot include EGDataAssetEditor.h in non-editor builds.h" );
#endif

int EGDataAssetEditor_Run( eg_cpstr16 Filename , const struct egSdkEngineInitParms& EngineParms );
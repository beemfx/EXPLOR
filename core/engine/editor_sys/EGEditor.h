// (c) 2017 Beem Media

#pragma once

#if !defined( __EGEDITOR__ )
#pragma __ERROR__("Cannot include EGEditor.h in non-editor builds.")
static_assert( false , "Cannot include EGEditor.h in non-editor builds.h" );
#endif

int EGEditor_Run( const class EGWndAppParms& AppParms , const struct egSdkEngineInitParms& EngineParms );
eg_bool EGEditor_IsEditorCmdLine( const class EGWndAppParms& AppParms );
eg_bool EGEditor_IsDataBuild( const class EGWndAppParms& AppParms );
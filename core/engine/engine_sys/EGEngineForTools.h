#pragma once

void EngineForTools_Init( const struct egSdkEngineInitParms& InitParms , eg_bool bStandalone );
void EngineForTools_InitRenderer( eg_uintptr_t WndOwner );
void EngineForTools_Deinit( eg_bool bStandalone );
void EngineForTools_OnToolWindowResized( eg_uint Width , eg_uint Height );
void EngineForTools_Draw();
void EngineForTools_Update( eg_real DeltaTime );
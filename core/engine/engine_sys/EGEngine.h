/******************************************************************************
Engine - The main engine class, manages initialization, shutdown, and the main
game loop.

(c) 2011 Beem Software
******************************************************************************/
#pragma once

eg_bool Engine_Run( const struct egSdkEngineInitParms& InitParms );
void Engine_QueMsg( eg_cpstr Msg );
void Engine_QueExit();
class EGClient* Engine_GetClientByIndex( eg_uint Index );

eg_string Engine_GetName();
eg_d_string16 Engine_GetGameTitle();
eg_string Engine_GetGameId();
eg_string Engine_GetGameName();
eg_bool   Engine_IsTool();
eg_bool   Engine_IsEditor();
eg_bool   Engine_WantsMouseCursor();
const struct egSdkEngineInitParms& Engine_GetInitParms();
EGClass* Engine_GetRenderPipeClass();
void EGEngine_GetClients( EGArray<class EGClient*>& ClientsOut );
void Engine_BindInput( eg_cpstr ClientId , const class EGInput& Input , eg_cpstr StrAction , eg_cpstr StrGp , eg_cpstr StrKb );
void Engine_WriteBindings( class EGFileData& MemFile );

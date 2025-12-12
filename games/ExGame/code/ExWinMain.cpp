// (c) 2016 Beem Media

#include "ExGameTypes.h"
#include "EGEngineApp.h"
#include "EGWindowsAPI.h"
#include "EGEngine.h"
#include "EGGlobalConfig.h"

int WINAPI WinMain( HINSTANCE hInstance , HINSTANCE hPrevInstance , LPSTR lpCmdLine , int nCmdShow)
{
	unused( hInstance , hPrevInstance , lpCmdLine , nCmdShow );

	extern EGClass& ExWorldRenderPipe_GetClass();
	
	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.ServerClassName = "ExServer";
	InitParms.ClientClassName = "ExClient";
	InitParms.EngineInstClassName = "ExEngineInst";
	InitParms.GameClassName = "ExGame";
	InitParms.GameName = "ExGame";
	InitParms.SaveFolder = L"EXPLOR";
	InitParms.GameTitle = "E.X.P.L.O.R.: A New World";
	InitParms.WorldRenderPipeClass = &ExWorldRenderPipe_GetClass();
	InitParms.bAllowConsole = EX_CHEATS_ENABLED;
	InitParms.bAllowDedicatedServer = false;
	InitParms.bIsDedicatedServer = false;
	
	#if IS_PLATFORM_Steam
	GlobalConfig_PlatformAppId.SetValue( "1387120" );
	#endif

	int Res = EGEngineApp_Run( InitParms );
	return Res;
}
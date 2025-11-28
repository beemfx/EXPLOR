// (c) 2017 Beem Media

#pragma once

struct egSdkEngineInitParms
{
	eg_string_small ServerClassName;
	eg_string_small ClientClassName;
	EGClass*        WorldRenderPipeClass = nullptr;
	eg_string_small EngineInstClassName;
	eg_string_small GameClassName;
	eg_string_small GameName;
	eg_s_string_sml16 SaveFolder;
	eg_s_string_sml8 GameTitle;
	eg_bool bAllowConsole:1;
	eg_bool bAllowDedicatedServer:1; // Does the command line -dedicated do anything?
	eg_bool bIsDedicatedServer:1;
	eg_bool bAllowCommandLineMap:1;

	egSdkEngineInitParms( eg_ctor_t Ct )
	: GameName( Ct )
	, bAllowConsole( true )
	, bAllowDedicatedServer( true )
	, bIsDedicatedServer( false )
	, bAllowCommandLineMap( false )
	, ServerClassName( "EGServer" )
	, ClientClassName( "EGClient" )
	, EngineInstClassName( "EGEngineInst" )
	, GameClassName( "EGGame" )
	, SaveFolder( L"EG Save Data" )
	, GameTitle( L"EG Game" )
	{
		assert( Ct == CT_Clear || Ct == CT_Default );
	}
};

int EGEngineApp_Run( const struct egSdkEngineInitParms& InitParms );
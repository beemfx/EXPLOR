/******************************************************************************
File: Engine.cpp
Class: EGEngine
Purpose: The main game class, manages initialization, shutdown, and the main
game loop.

(c) 2011 Beem Software
******************************************************************************/

#include "EGEngine.h"
#include "EGEngineApp.h"
#include "EGTimer.h"
#include "EGLoader.h"
#include "EGClient.h"
#include "EGConsole.h"
#include "EGInput.h"
#include "EGMesh.h"
#include "EGSkel.h"
#include "EGRenderer.h"
#include "EGFileData.h"
#include "EGConDraw.h"
#include "EGEntDict.h"
#include "EGNetRequest.h"
#include "EGRenderThread.h"
#include "EGEngine.h"
#include "EGClient.h"
#include "EGServerInterface.h"
#include "EGSaveMgr.h"
#include "EGPlatform.h"
#include "EGCrcDb.h"
#include "EGFont.h"
#include "EGUiLayout.h"
#include "EGKbCharHandler.h"
#include "EGEngineInst.h"
#include "EGServer.h"
#include "EGGame.h"
#include "EGDebugText.h"
#include "EGTextNode.h"
#include "EGAudio.h"
#include "EGEngineMem.h"
#include "EGEngineCore.h"
#include "EGWorkerThreads.h"
#include "EGDebugShapes.h"
#include "EGGlobalConfig.h"
#include "EGLocalize.h"

static EGSettingsInt EGEngine_MaxGameFPS( "EGEngine.MaxGameFPS" , eg_loc("MaxGameFPSName","Max Frame Rate: Game Thread") , 120 , EGS_F_SYS_SAVED|EGS_F_DEBUG_EDITABLE ); // It's important that the game rate is above the video rate or motion will feel jittery.

class EGInput;
class EGClient;
class EGNetComm;

class EGEngine
{
public:

	EGEngine( eg_bool IsTool );
	~EGEngine();

	eg_bool Run( const struct egSdkEngineInitParms& InitParms );
	void QueMsg( eg_cpstr Msg ){ EGFunctionLock Lock( &m_MsgLock ); m_MsgStack.EnQueue( Msg ); }
	void QueExit() { m_bShouldQuit = true; }

	eg_string GetGameId()const
	{
		eg_string Out = EGString_Format( "%s %s" , m_InitParms.GameName.String() , ENGINE_PLATFORM );
		return Out;
	}

	eg_string GetGameName() const
	{
		return m_InitParms.GameName;
	}

	eg_d_string16 GetGameTitle() const
	{
		return *m_InitParms.GameTitle;
	}

	const egSdkEngineInitParms& GetInitParms() const { return m_InitParms; }

	class EGClient* GetClientByIndex( eg_uint Index )
	{
		EGClient* Out = nullptr;
		if( m_Clients.IsValidIndex( Index ) )
		{
			Out = m_Clients[Index];
		}
		return Out;
	}

	eg_bool IsTool()const{ return m_bIsTool; }
	eg_bool WantsMouseCursor() const { return m_Clients[0] && m_Clients[0]->WantsMouseCursor(); }
	
	void GetClients( EGArray<EGClient*>& ClientsOut )
	{
		assert( EGWorkerThreads_IsThisMainThread() );

		for( EGClient* Client : m_Clients )
		{
			if( Client && Client->IsConnected() )
			{
				ClientsOut.Append( Client );
			}
		}
	}

	void BindInput( eg_cpstr ClientId , const class EGInput& Input, eg_cpstr StrAction, eg_cpstr StrGp, eg_cpstr StrKb )
	{
		for( EGClient* Client : m_Clients )
		{
			if( EGString_Equals( ClientId , Client->GetId().ToString() ) )
			{
				Client->BindInput( Input , StrAction , StrGp , StrKb );
			}
		}
	}

	void WriteBindings( EGFileData& Out )
	{
		for( EGClient* Client : m_Clients )
		{
			EGInput::Get().WriteBindings( Client->GetInputBindings() , Out, EGString_Format( "bind( %s , \"%%s\" , \"%%s\" , \"%%s\" );\r\n" , Client->GetId().ToString().String() ) );
		}
	}

	void WaitTillStarted()
	{
		assert( !EGWorkerThreads_IsThisMainThread() );

		while( true )
		{
			EGFunctionLock FnLock( &m_BootupLock );
			if( m_bBootComplete )
			{
				return;
			}
		}
	}

	void Tools_Init(  const struct egSdkEngineInitParms& InitParms , eg_uintptr_t WndOwner )
	{
		m_Tools_WndOwner = WndOwner;
		Init( InitParms );
	}

	void Tools_InitRenderer( eg_uintptr_t WndOwner )
	{
		m_Tools_RenderThread.ToolsInit( WndOwner );
		EGWorkerThreads_SetToolCreatedRenderer( &m_Tools_RenderThread );
	}

	void Tools_Deinit()
	{
		Deinit();
	}

	void Tools_OnWindowResized( eg_uint Width , eg_uint Height )
	{
		m_Tools_RenderThread.ToolsOnWindowResized( Width , Height );
	}

	void Tools_Update( eg_real DeltaTime )
	{
		unused( DeltaTime );
		Update();
	}

	void Tools_Draw()
	{
		m_Tools_RenderThread.ToolsUpdate( 1.f/60.f );
	}

private:

	eg_bool Init( const struct egSdkEngineInitParms& InitParms );
	void    Deinit();
	void    Update();
	eg_bool ShouldQuit()const{return m_bShouldQuit;}

private:

	static const eg_uint MAX_MESSAGES = 128;
	typedef EGQueue<eg_string , MAX_MESSAGES> EGMsgStack;
	typedef EGFixedArray<EGClient*,MAX_LOCAL_CLIENTS> EGClientList;

private:

	EGEngineInst*    m_EngineInst = nullptr;
	EGEngineCore     m_EngineCore;
	EGTimer          m_Timer;
	EGClientList     m_Clients;
	EGMutex          m_MsgLock;
	EGMutex          m_BootupLock;
	EGMsgStack       m_MsgStack;
	eg_uint          m_nTicks;
	EGInputBindings  m_EngineInputBindings;
	EGInputCmdState  m_EngineInputCmdState;
	eg_bool          m_bShouldQuit;
	egEnginePlatformParms m_PlatformParms;
	egSdkEngineInitParms  m_InitParms;
	eg_bool          m_bHasBegun:1;
	eg_bool          m_bBootComplete:1;
	const eg_bool    m_bIsTool:1;

	eg_uintptr_t     m_Tools_WndOwner;
	EGRenderThread   m_Tools_RenderThread; // Only used for drawing for tools

private:

	void HandleMsg(eg_cpstr sMsg);
	void Begin();
	void DisplayDebugInfo( egLockstepCmds* Cmds );
	void Update_ServerOnly( eg_real DeltaTime );
	void Update_ToolsOnly( eg_real DeltaTime );
	void Update_ProcessGameInput( egLockstepCmds* Cmds );
	void Update_ProcessMsgs();
	void Update_Clients( eg_real DeltaTime , eg_real GameAspectRatio );

	static void ConsoleCmdCb( eg_cpstr StrCmd , void* Data )
	{
		EGEngine* _this = reinterpret_cast<EGEngine*>(Data);
		//Commands are forwarded to the command object
		_this->QueMsg( StrCmd );
		_this->QueMsg( "engine.SaveConfig()" ); // This makes it so that the console history gets saved.
	}

	eg_bool IsServerOnly()const{ return m_InitParms.bIsDedicatedServer; }
};

eg_bool EGEngine::Run( const struct egSdkEngineInitParms& InitParms )
{
	eg_bool Inited = Init( InitParms );
	if( !Inited )
	{
		return false;
	}

	//The game loop:
	while(!ShouldQuit())
	{
		Update();
	}
	
	Deinit();
	return true;
}

void EGEngine::Begin()
{
	if( IsTool() ) // Tools don't need to initialize the game.
	{
		return;
	}

	const eg_bool SERVER_ON_MAIN_THREAD = !GlobalConfig_ServerHasThread.GetValueThreadSafe();
	Server_Start( SERVER_ON_MAIN_THREAD ? SERVER_START_THIS_THREAD : SERVER_START_THREAD );
	eg_bool OpenMenu = true;

	eg_bool bAllowCmdLineMap = m_InitParms.bAllowCommandLineMap;

	if( IsServerOnly() || IsTool() )
	{
		OpenMenu = false;
	}
	else
	{
		if( bAllowCmdLineMap && m_PlatformParms.RemoteHost.Len() > 0 )
		{
			m_Clients[0]->Connect( m_PlatformParms.RemoteHost );
			OpenMenu = false;
		}
		else
		{
			m_Clients[0]->Connect(nullptr); //Local connection
			//m_Clients[1].Connect(EGString_Format( "127.0.0.1:%u" , EGSettings_Get()->GetServer_Port() ) );
			//m_Clients[2].Connect(nullptr);
			//m_Clients[3].Connect(nullptr);
		}
	}
	
	if( bAllowCmdLineMap && m_PlatformParms.StartMap.Len() > 0 )
	{
		this->QueMsg( EGString_Format( "server.Load( \"%s\" )" , m_PlatformParms.StartMap.String() ) );
		OpenMenu = false;
	}

	if( OpenMenu )
	{
		EGAudio_BeginFrame(); // Highly likely that a menu will be push in begin game, so we want to play audio associated with it.
		m_Clients[0]->BeginGame();
		EGAudio_EndFrame();
	}
	//Test whatever you want here, the game is initialized and ready to go.
}

EGEngine::EGEngine(eg_bool InIsTool) 
: m_bShouldQuit(false)
, m_bHasBegun(false)
, m_bBootComplete(false)
, m_PlatformParms( CT_Clear )
, m_InitParms( CT_Clear )
, m_bIsTool( InIsTool )
, m_Clients( CT_Clear )
{

}

EGEngine::~EGEngine()
{

}

void EGEngine::HandleMsg(eg_cpstr sMsg)
{
	//This method is called from the console function
	eg_cpstr strCmd = sMsg;

	egParseFuncInfo ParseInfoStr;
	EGPARSE_RESULT r = EGParse_ParseFunction(strCmd, &ParseInfoStr);
	if(EGPARSE_OKAY != r)
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": Invalid function call \"%s\". Parse error %u: %s." , strCmd , r , EGParse_GetParseResultString(r) );
		return;
	}

	egParseFuncInfoAsEgStrings Info( ParseInfoStr );

	//Commands come in three varieties, game, server, and client
	if( Info.SystemName.Equals("game") )
	{
		Server_PostMsg( sMsg );
	}
	else if(Info.SystemName.Equals(("engine")))
	{
		//Process game commands.
		if(Info.FunctionName.EqualsI("SetLogEnabled"))
		{
			eg_s_string_sml8 LogChannelStr = Info.NumParms >= 1 ? Info.Parms[0] : "";
			eg_bool bEnabled = Info.NumParms >= 2 ? EGString_ToBool( Info.Parms[1] ) : true;
			EGString_ToUpper( const_cast<eg_pstr>(LogChannelStr.AsCStr()) , LogChannelStr.Len()+1 );
			eg_string_crc LogChannelCrc = eg_string_crc(*LogChannelStr);
			EGLog_SetChannelSuppressed( eg_log_channel(LogChannelCrc) , !bEnabled );
		}
		else if(!Info.FunctionName.Compare(("quit")))
		{
			m_bShouldQuit = true;
		}
		else if(!Info.FunctionName.CompareI(("VidRestart")))
		{
			EGRenderer::Get().ResetDisplay();
		}
		else if(!Info.FunctionName.CompareI(("VidResolutions")))
		{
			EGArray<eg_ivec2> Resolutions;
			EGRenderer::Get().GetResolutions( Resolutions );
			for( const eg_ivec2& Res : Resolutions )
			{
				EGLogf( eg_log_t::General , ("%ux%u"), Res.x , Res.y );
			}
		}
		else if(!Info.FunctionName.Compare(("SaveConfig")))
		{
			m_EngineCore.ProcessSettings( false );
		}
		else if(!Info.FunctionName.Compare(("LoadConfig")))
		{
			m_EngineCore.ProcessSettings( true );
		}
		else if( Info.FunctionName.Equals( ("dir")))
		{
			m_EngineCore.PrintDir();
		}
		else if( Info.FunctionName.EqualsI( "Set" ) )
		{
			if( Info.NumParms >=2 )
			{
				EGSettingsVar* Setting = EGSettingsVar::GetSettingByName( Info.Parms[0] );
				if( Setting )
				{
					if( Setting->IsDebugEditable() || Setting->IsEditable() )
					{
						Setting->SetFromString( Info.Parms[1] );
					}
					else
					{
						EGLogf( eg_log_t::General , "Cannot change %s through console." , Info.Parms[0].String() );
					}
				}
				else
				{
					EGLogf( eg_log_t::General , "No such setting: %s" , Info.Parms[0].String() );
				}
			}
			else
			{
				EGLogf( eg_log_t::General , "Usage: engine.Set( \"VarName\" , VarValue )" );
			}
		}
		else if( Info.FunctionName.EqualsI( "LocDump" ) )
		{
			EGLocalize_DumpDb();
		}
		else
		{
			EGLogf( eg_log_t::Warning , "\"%s.%s\" is not recognized as a valid command." , Info.SystemName.String() , Info.FunctionName.String() );
		}
	}
	else if(Info.SystemName.Equals(("server")))
	{
		if( "Start" == Info.FunctionName )
		{
			Server_Start( GlobalConfig_ServerHasThread.GetValueThreadSafe() ? SERVER_START_THREAD : SERVER_START_THIS_THREAD );
		}
		else if( "Stop" == Info.FunctionName )
		{
			Server_Stop();
		}
		else
		{
			Server_PostMsg( sMsg );
		}
	}
	else if(EGString_EqualsCount( Info.SystemName , "client" , 6 ) )
	{
		EGClient* Client = nullptr;
		
		if( Info.SystemName.Len() == 7 )
		{
			const eg_char CliIdStr[] = { Info.SystemName.CharAt(6) , '\0' };
			eg_uint Id = eg_string( CliIdStr ).ToUInt();
			if( 1 <= Id && Id<=m_Clients.Len() )
			{
				Client = m_Clients[Id-1];
			}
			if( 0 == CliIdStr[0] )
			{
				Client = m_Clients[0];
			}
		}
		else if( Info.SystemName.Len() == 6 )
		{
			Client = m_Clients[0];
		}

		if( nullptr != Client && !IsServerOnly() )
		{
			//Process client commands.
			if(Info.FunctionName.EqualsI(("connect")))
			{
				Client->Connect( (Info.NumParms == 1) ? Info.Parms[0].String() : nullptr );
			}
			else if(Info.FunctionName.EqualsI(("disconnect")))
			{
				Client->Disconnect();
			}
			else
			{
				Client->SendCmd( sMsg );
			}
		}
		else
		{
			EGLogf( eg_log_t::Warning , "%s is an invalid client, command not processed." , Info.SystemName.String() );
		}
	}
	else if(Info.SystemName.Equals(("con")))
	{
		if(!Info.FunctionName.Compare(("Clear")))
		{
			MainConsole.Clear();
			extern void WinCon_RefreshCon();
			WinCon_RefreshCon(); //Safe to call even if there is no console.
		}
		else if( Info.FunctionName.Equals( "echo" ) )
		{
			if( Info.NumParms > 0 )
			{
				EGLogf( eg_log_t::General , "%s" , Info.Parms[0].String() );
			}
		}
		else if(1 == Info.NumParms && !Info.FunctionName.Compare(("Dump")))
		{
			EGFileData ConFile( eg_file_data_init_t::HasOwnMemory );
			MainConsole.Dump(ConFile);
			MainLoader->SaveData( Info.Parms[0] , ConFile.GetDataAs<eg_byte>() , ConFile.GetSize() );
		}
	}
	else
	{
		EGLogf (eg_log_t::Warning , "No system specified or invalid system (%s), command not processed: %s" , Info.SystemName.String() , sMsg );
	}
}

void EGEngine::DisplayDebugInfo( egLockstepCmds* Cmds )
{
	if( DebugConfig_DrawMenuButtons.GetValue() )
	{
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( ( "Mouse: %.2f %.2f" ) , Cmds->m_MousePos.x , Cmds->m_MousePos.y ) );
	}
}

void EGEngine::Update_ProcessMsgs()
{
	EGFunctionLock Lock( &m_MsgLock );

	for(eg_uint l = 0; m_MsgStack.HasItems() && l < MAX_MESSAGES; l++)
	{
		HandleMsg( m_MsgStack.Front() );
		m_MsgStack.DeQueue();
	}
}

void EGEngine::Update_ProcessGameInput( egLockstepCmds* Cmds )
{
	if( m_InitParms.bAllowConsole || IS_DEBUG )
	{
		if(Cmds->WasMenuPressed(::CMDA_SYS_CONTOGGLE) )
		{
			//Clear all input so we don't confuse the newly opened window.
			Cmds->Clear();
			ConDraw_ToggleActive();
		}
	}
}

void EGEngine::Update_ServerOnly( eg_real DeltaTime )
{
	Server_UpdatePart1( DeltaTime );
	Server_UpdatePart2( DeltaTime );
	Update_ProcessMsgs();
	NetRequest_Update( DeltaTime );
	m_nTicks++;
}
	
void EGEngine::Update_ToolsOnly( eg_real DeltaTime )
{
	unused( DeltaTime );

	EntDict_Update();
	Update_ProcessMsgs();
	m_nTicks++;
}

void EGEngine::Update()
{
	const eg_uint GAME_MAX_FPS           = EGEngine_MaxGameFPS.GetValue();
	const eg_real GAME_MAX_FRAME_TIME    = 1.0f/GAME_MAX_FPS;
	const eg_uint GAME_MAX_FRAME_TIME_MS = 1000/GAME_MAX_FPS;
	
	// We don't actually start the game until all initial loading is complete
	if( !m_bHasBegun && !(MainLoader->IsLoading() || MainLoader->IsSaving()) && !IsTool() )
	{
		m_bHasBegun = true;
		Begin();
		EGRenderer::Get().SetDrawSplash( false );
	}

	const eg_real FrameTime = m_Timer.GetRawElapsedSec();
	if( FrameTime < GAME_MAX_FRAME_TIME )
	{
		const eg_real GAME_FRAME_THRESHOLD = EGTHREAD_DEFAULT_SLEEP_THRESHOLD;//1.f/20.f;
		const eg_real TimeLeft = GAME_MAX_FRAME_TIME - FrameTime;
		if( TimeLeft >= GAME_FRAME_THRESHOLD )
		{
			EGThread_Sleep( TimeLeft - GAME_FRAME_THRESHOLD );
		}
		else
		{
			EGThread_Sleep( 0 );
		}
		return;
	}

	m_Timer.Update();
	const eg_real DeltaTime = FrameTime;

	m_EngineCore.Update( DeltaTime );
	
	if( EGMem2_HasUsedEmergencyMemory() )
	{
		EGLogToScreen::Get().Log( this , __LINE__ , 1.f , "Out of memory!" );
	}

	if( IsTool() )
	{
		Update_ToolsOnly( FrameTime );
		return;
	}

	if( IsServerOnly() )
	{
		Update_ServerOnly( FrameTime );
		return;
	}

	const eg_real GameAspectRatio = EGRenderer::Get().GetAspectRatio(); // We get the aspect ratio every frame because it's possible it changed.

	Server_UpdatePart1( DeltaTime );

	EGAudio_BeginFrame();
	Update_ProcessMsgs();

	//Process characters
	{
		for( eg_char c=EGInput::Get().GetChar(); '\0' != c; c=EGInput::Get().GetChar() )
		{
			if( ConDraw_IsActive() ) //If the ConDraw is open it takes the chars
			{
				ConDraw_OnChar( c );
			}
			else //Otherwise the character is consumed into the void.
			{
				eg_bool bCharHandled = EGKbCharHandler::Get().ProcessChar( c );
			}
		}
	}

	eg_bool ConIsActive = ConDraw_IsActive(); //We save this off here, because ConDraw_Update may hide the console, but we don't want to process commands if it was visible beforehand.
	
	egLockstepCmds InputCmds = EGInput::Get().UpdateCmds( eg_input_device_id::KBM , m_EngineInputBindings , DeltaTime , m_EngineInputCmdState, GameAspectRatio );
	ConDraw_Update( DeltaTime , &InputCmds );
	if( ConIsActive )
	{
		InputCmds.ClearButtonState();
	}
	EntDict_Update();
	EGDebugShapes::Get().Update( DeltaTime );
	NetRequest_Update( DeltaTime );
	
	Update_Clients( DeltaTime , GameAspectRatio );

	Update_ProcessGameInput( &InputCmds );
	DisplayDebugInfo( &InputCmds );
	EGAudio_EndFrame();

	EGRenderer::Get().BeginFrame();
	{
		//Drawing functions go here...

		// Draw clients:
		{
			eg_uint LocalClients[MAX_LOCAL_CLIENTS];
			eg_uint NumLocalClients = 0;
			for( eg_uint i=0; i<m_Clients.Len(); i++ )
			{
				if( m_Clients[i]->IsConnected() )
				{
					LocalClients[NumLocalClients] = i;
					NumLocalClients++;
				}
			}
			for( eg_uint i=0; i<NumLocalClients; i++ )
			{
				m_Clients[LocalClients[i]]->Draw( NumLocalClients > 1 );
			}
		}

		MainDisplayList->ResetScreenViewport();
		ConDraw_Draw( GameAspectRatio );
		EGLogToScreen::Get().Draw( GameAspectRatio );
	}
	EGRenderer::Get().EndFrame();

	Server_UpdatePart2( DeltaTime );

	m_nTicks++;
}

void EGEngine::Update_Clients( eg_real DeltaTime , eg_real GameAspectRatio )
{
	eg_uint LocalClients[MAX_LOCAL_CLIENTS];
	eg_uint NumLocalClients = 0;
	for( eg_uint i=0; i<m_Clients.Len(); i++ )
	{
		if( m_Clients[i]->IsConnected() )
		{
			LocalClients[NumLocalClients] = i;
			NumLocalClients++;
		}
	}
	egScreenViewport ClientVp[MAX_LOCAL_CLIENTS];

	eg_uint ClientIndex = 0;
	for( eg_uint i=0; i<m_Clients.Len(); i++ )
	{
		const eg_real A = GameAspectRatio;
		egScreenViewport ClientView = { -1.0f*A , 1.0f*A , -1.f , 1.f , 0.0f , 1.0f };
	
		if( 2 == NumLocalClients )
		{
			switch( ClientIndex )
			{
				case 0: ClientView.Bottom = 0; break;
				case 1: ClientView.Top = 0; break;
				default: break;
			}
		}
		else if( 3 == NumLocalClients || 4 == NumLocalClients )
		{
			switch( ClientIndex )
			{
				case 0: 
					ClientView.Right = 0; 
					ClientView.Bottom = 0; 
					break;
				case 1: 
					ClientView.Left = 0;
					ClientView.Bottom = 0;
					break;
				case 2:
					ClientView.Right = 0;
					ClientView.Top = 0;
					break;
				case 3:
					ClientView.Left = 0;
					ClientView.Top = 0;
					break;
				default: break;
			}
		}

		ClientVp[ClientIndex] = ClientView;

		if( m_Clients[i]->IsConnected() )
		{
			ClientIndex++;
		}
	}

	m_EngineInst->SetupViewports( GameAspectRatio , ClientVp , NumLocalClients );

	// Decide what input devices for which client.
	if( NumLocalClients == 1 )
	{
		// If there is only one client, use the input device last used.
		for( eg_input_device_id InputDevice = eg_input_device_id::NONE; InputDevice<eg_input_device_id::COUNT;  )
		{
			if( EGInput::Get().WasThereInputThisFrameFor( InputDevice ) )
			{
				m_Clients[LocalClients[0]]->SetInputDevice( InputDevice );
				break;
			}

			InputDevice = static_cast<eg_input_device_id>( static_cast<eg_uint>(InputDevice)+1);
		}
	}

	if( NumLocalClients > 1 )
	{	
		for( eg_uint i=0; i<NumLocalClients; i++ )
		{
			eg_input_device_id GamepadForThisClient = static_cast<eg_input_device_id>( static_cast<eg_uint>(eg_input_device_id::GP1) + LocalClients[i] );
			m_Clients[LocalClients[i]]->SetViewport( ClientVp[i] , GameAspectRatio );
			m_Clients[LocalClients[i]]->Update( DeltaTime );

			if( i == 0 )
			{
				if( EGInput::Get().WasThereInputThisFrameFor( eg_input_device_id::KBM ) )
				{
					m_Clients[LocalClients[i]]->SetInputDevice( eg_input_device_id::KBM );
				}
				else if( EGInput::Get().WasThereInputThisFrameFor( GamepadForThisClient ) )
				{
					m_Clients[LocalClients[i]]->SetInputDevice( GamepadForThisClient );
				}
			}
			else
			{
				m_Clients[LocalClients[i]]->SetInputDevice( GamepadForThisClient );
			}
		}
	}
	else
	{
		for( eg_uint i=0; i<NumLocalClients; i++ )
		{
			m_Clients[LocalClients[i]]->SetViewport( ClientVp[i] , GameAspectRatio );
			m_Clients[LocalClients[i]]->Update( DeltaTime );
		}
	}

	if( DebugConfig_DrawGameState.GetValue() )
	{
		eg_string StrConn = "Local Clients: ";
		for( eg_uint i=0; i<NumLocalClients; i++ )
		{
			StrConn += EGString_Format( "%u " , LocalClients[i]+1 );
		}
		EGLogToScreen::Get().Log( this , 0 , 1.f , StrConn );
	}
}

eg_bool EGEngine::Init( const struct egSdkEngineInitParms& InitParms )
{
	EGFunctionLock FnLock( &m_BootupLock );

	EGLogf( eg_log_t::General , "%s (%s)" , ENGINE_NAME , ENGINE_VERSION );

	m_InitParms = InitParms;
	EGPlatform_GetRunParms( &m_PlatformParms );

	m_EngineInst = EGNewObject<EGEngineInst>( EGClass::FindClassSafe( m_InitParms.EngineInstClassName , EGEngineInst::GetStaticClass() ) , eg_mem_pool::System );
	m_EngineInst->SetIsTool( IsTool() );
	m_EngineInst->SetIsServerOnly( IsServerOnly() );
	m_EngineInst->SetGameName( m_InitParms.GameName );
	m_EngineInst->SetPlatformExeDir( *m_PlatformParms.ExeDir );
	m_EngineInst->SetPlatformUserDir( *m_PlatformParms.UserDir );
	m_EngineInst->SetPlatformSysCfgDir( *m_PlatformParms.SysCfgDir );

	EGLog_SetDefaultSuppressedChannels();

	EGTextNode::RegisterDefaultColors();

	m_Timer.Init();
	m_Timer.SoftReset();

	MainLoader->SetIsLoadOnOtherThreadOkay( true );

	m_EngineCore.Init( m_EngineInst );

	EGClass* ServerClass = EGClass::FindClassSafe( m_InitParms.ServerClassName, EGServer::GetStaticClass() );
	EGClass* GameClass = EGClass::FindClassSafe( m_InitParms.GameClassName , EGGame::GetStaticClass() );
	Server_Init( ServerClass , GameClass );

	if( IsTool() )
	{
		Tools_InitRenderer( m_Tools_WndOwner );
	}

	eg_string RendererDevice = *GlobalConfig_ScreenDriverClass.GetValueThreadSafe();
	if( IsServerOnly() || IsTool() )
	{
		RendererDevice = "";
	}
	eg_bool CreateRenderer = EGRenderer::Get().Init( RendererDevice );
	if(!CreateRenderer)
	{ 
		assert(false); 
		HandleMsg( EGString_Format( "con.Dump( \"%s/condump.txt\" )" , GAME_SYSCFG_PATH ) );
		MainLoader->WaitForAllLoads();

		EGMem2_SetSysFreeAllowed( true );
		EGWorkerThreads_Deinit();
		m_Timer.Shutdown();
		m_EngineCore.Deinit();
		EGMem2_SetSysFreeAllowed( false );
		return false; 
	}

	EGRenderer::Get().SetDrawSplash( true );

	//At this point the console and file system are initialized, so we may run
	//scripts without a problem.
	MainConsole.SetCommandCallback( ConsoleCmdCb , this );

	EntDict_Init();
	EGDebugShapes::Get().Init();
	ConDraw_Init();
	EGFontMgr::Get()->Init();
	EGUiLayoutMgr::Get()->Init();

	m_EngineInst->OnGameEvent( EGEngineInst::eg_game_event::INIT );

	EGAudio_BeginFrame(); // Clients may want to play audio on init (especially if a menu was pushed)

	EGClass* ClientClass = EGClass::FindClassSafe( m_InitParms.ClientClassName , EGClient::GetStaticClass() );
	for( eg_uint i=0; i<MAX_LOCAL_CLIENTS; i++ )
	{
		EGClient* NewClient = EGNewObject<EGClient>( ClientClass , eg_mem_pool::System );
		NewClient->Init( egClientId(i+1), GameClass );
		m_Clients.Append( NewClient );
	}

	m_EngineCore.LoadKeyBindings();

	EGAudio_EndFrame();


	MainLoader->SetIsLoadOnOtherThreadOkay( false );
	m_nTicks=0;
		
	//To make sure the timer is at zero when the game starts running
	m_Timer.Update();
	EGLogf( eg_log_t::General , "Emergence Game Engine initialized in %f seconds." , m_Timer.GetElapsedSec() );
	#if defined( __DEBUG__ )
	eg_real SECONDS_TO_SHOW_SPLASH = 1.0f;
	#else
	eg_real SECONDS_TO_SHOW_SPLASH = 3.0f;
	#endif

	if( IsTool() )
	{
		SECONDS_TO_SHOW_SPLASH = 0.f;
		EGRenderer::Get().SetDrawSplash( false );
	}

	EGThread_Sleep( EG_Max( 0.0f , SECONDS_TO_SHOW_SPLASH-m_Timer.GetElapsedSec()) ); //Make sure the splash screen shows for at least X seconds.
	//To make sure the timer is at zero when the game starts running.
	m_Timer.SetTimeMs(0);
	m_Timer.SoftReset();

	m_bBootComplete = true;

	// Bind some system keys
	m_EngineInputBindings.Bind( CMDA_SYS_CONTOGGLE , GP_BTN_NONE , KBM_BTN_F11 );
	m_EngineInputBindings.Bind( CMDA_SYS_CONPAGEDOWN , GP_BTN_NONE , KBM_BTN_NEXT );
	m_EngineInputBindings.Bind( CMDA_SYS_CONPAGEUP , GP_BTN_NONE , KBM_BTN_PRIOR );
	m_EngineInputBindings.Bind( CMDA_SYS_CONHISTORY , GP_BTN_NONE , KBM_BTN_UP );
	m_EngineInputCmdState.Init();

	return true;
}

void EGEngine::Deinit()
{	
	EGMem2_SetSysFreeAllowed( true );
	MainConsole.SetCommandCallback( nullptr , nullptr );

	for( EGClient* Client : m_Clients )
	{
		Client->Disconnect();
		Client->Deinit();
		EGDeleteObject( Client );
	}
	m_Clients.Clear();

	m_EngineInst->OnGameEvent( EGEngineInst::eg_game_event::DEINIT );

	Server_Deinit();
	EGUiLayoutMgr::Get()->Deinit();
	EGFontMgr::Get()->Deinit();
	ConDraw_Deinit();
	EGDebugShapes::Get().Deinit();
	EntDict_Deinit();
	if( IsTool() )
	{
		m_Tools_RenderThread.ToolsDeinit();
	}
	EGRenderer::Get().Deinit();
	{
		//Dump the console before it is destroyed (must be dumped before game threads are deinited or the load will not complete):
		HandleMsg( EGString_Format( "con.Dump( \"%s/condump.txt\" )" , GAME_SYSCFG_PATH ) );
		MainLoader->WaitForAllLoads();
	}
	EGDeleteObject( m_EngineInst );
	m_EngineInst = nullptr;

	m_EngineCore.Deinit();

	m_Timer.Shutdown();

	EGMem2_SetSysFreeAllowed( false );
}

static EGEngine* GlobalEngine = nullptr;
static eg_byte Engine_Mem[sizeof(EGEngine)];

eg_bool Engine_Run( const struct egSdkEngineInitParms& InitParms )
{
	GlobalEngine = new( Engine_Mem ) EGEngine( false );
	eg_bool Result = false;
	if( nullptr != GlobalEngine )
	{
		Result = GlobalEngine->Run( InitParms );
		EGMem2_SetSysFreeAllowed( true );
		GlobalEngine->~EGEngine();
		//EG_SafeDelete( GlobalEngine );
		GlobalEngine = nullptr;
		EGMem2_SetSysFreeAllowed( false );
	}
	return Result;
}

void Engine_QueMsg( eg_cpstr Msg )
{
	if( GlobalEngine )
	{
		GlobalEngine->QueMsg( Msg );
	}
}

void Engine_QueExit()
{
	if( GlobalEngine )
	{
		GlobalEngine->QueExit();
	}
}

eg_string Engine_GetName()
{
	return EGString_Format( "%s %s %s %s" , ENGINE_NAME , ENGINE_VERSION , ENGINE_PLATFORM , ENGINE_BUILD );
}

eg_d_string16 Engine_GetGameTitle()
{
	if( GlobalEngine )
	{
		if( IS_DEBUG )
		{
			return EGSFormat16( L"{0} {1} {2} {3}" , *GlobalEngine->GetGameTitle() , ENGINE_VERSION , ENGINE_PLATFORM , ENGINE_BUILD );
		}
		return GlobalEngine->GetGameTitle();
	}
	
	return Engine_GetName().String();
}

eg_string Engine_GetGameId()
{
	return GlobalEngine->GetGameId();
}

eg_string Engine_GetGameName()
{
	return GlobalEngine->GetGameName();
}

eg_bool Engine_IsTool()
{
	return GlobalEngine->IsTool();
}

eg_bool Engine_IsEditor()
{
#if defined( __EGEDITOR__ )
	return true;
#else
	return false;
#endif
}

eg_bool Engine_WantsMouseCursor()
{
	return GlobalEngine->WantsMouseCursor();
}

const struct egSdkEngineInitParms& Engine_GetInitParms()
{
	return GlobalEngine->GetInitParms();
}

EGClass* Engine_GetRenderPipeClass()
{
	EGClass* Out = GlobalEngine->GetInitParms().WorldRenderPipeClass;
	if( Out == nullptr )
	{
		extern EGClass& EGWorldRenderPipe_GetClass();
		Out = &EGWorldRenderPipe_GetClass();
	}
	return Out;
}

void EGEngine_GetClients( EGArray<class EGClient*>& ClientsOut )
{
	GlobalEngine->GetClients( ClientsOut );
}

void Engine_BindInput( eg_cpstr ClientId , const class EGInput& Input, eg_cpstr StrAction, eg_cpstr StrGp, eg_cpstr StrKb )
{
	GlobalEngine->BindInput( ClientId , Input , StrAction , StrGp , StrKb );
}

void Engine_WriteBindings( EGFileData& MemFile )
{
	GlobalEngine->WriteBindings( MemFile );
}

void Engine_WaitTillStarted()
{
	return GlobalEngine->WaitTillStarted();
}

class EGClient* Engine_GetClientByIndex( eg_uint Index )
{
	return GlobalEngine->GetClientByIndex( Index );
}

static egSdkEngineInitParms EngineForTools_InitParms( CT_Clear );

void EngineForTools_Init(const struct egSdkEngineInitParms& InitParms , eg_bool bStandalone )
{
	EngineForTools_InitParms = InitParms;

	if( bStandalone )
	{
		EGEngineMem_Init();
		EGPlatform_Init(InitParms.GameName);
		EGCrcDb::Init();
	}
}

void EngineForTools_InitRenderer( eg_uintptr_t WndOwner )
{
	GlobalEngine = new( Engine_Mem ) EGEngine( true );
	if( GlobalEngine )
	{
		GlobalEngine->Tools_Init( EngineForTools_InitParms , WndOwner );
	}
}

void EngineForTools_OnToolWindowResized( eg_uint Width , eg_uint Height )
{
	if( GlobalEngine )
	{
		GlobalEngine->Tools_OnWindowResized( EG_Max<eg_int>(Width,320) , EG_Max<eg_int>(Height,200) );
	}
}

void EngineForTools_Deinit( eg_bool bStandalone )
{
	if( GlobalEngine )
	{
		GlobalEngine->Tools_Deinit();
		EGMem2_SetSysFreeAllowed( true );
		GlobalEngine->~EGEngine();
		GlobalEngine = nullptr;
		EGMem2_SetSysFreeAllowed( false );
	}

	if( bStandalone )
	{
		EGCrcDb::Deinit();
		EGPlatform_Deinit();
		EGEngineMem_Deinit();
	}
}

void EngineForTools_Draw()
{
	if( GlobalEngine )
	{
		GlobalEngine->Tools_Draw();
	}
}

void EngineForTools_Update( eg_real DeltaTime )
{
	if( GlobalEngine )
	{
		GlobalEngine->Tools_Update( DeltaTime );
	}
}

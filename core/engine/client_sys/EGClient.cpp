// (c) 2011 Beem Software

#include "EGClient.h"
#include "EGGame.h"
#include "EGEnt.h"
#include "EGInput.h"
#include "EGBoolList.h"
#include "EGEnt.h"
#include "EGInput.h"
#include "EGMesh.h"
#include "EGSkel.h"
#include "EGLoader.h"
#include "EGConsole.h"
#include "EGConDraw.h"
#include "EGOverlayMgr.h"
#include "EGMenuStack.h"
#include "EGParse.h"
#include "EGEngine.h"
#include "EGAudioList.h"
#include "EGTimer.h"
#include "EGSoundScape.h"
#include "EGDebugText.h"
#include "EGInputGlyphs.h"
#include "EGAudio.h"
#include "EGMeshMgr2.h"
#include "EGDebugShapes.h"
#include "EGEntWorld.h"
#include "EGEntWorldRegion.h"
#include "EGWorldSceneGraph.h"
#include "EGWorldRenderPipe.h"
#include "EGEngine.h"
#include "EGGlobalConfig.h"
#include "EGFileData.h"
#include "EGAmbientSoundComponent.h"
#include <fs_sys2/fs_sys2.h> // Don't like this, but we want to know if the profile exists before we attempt to load it

EG_CLASS_DECL( EGClient )

EGClient::EGClient()
: m_MenuStack(nullptr)
, m_ClientTime(0)
, m_DeltaTime(0)
, m_fTimeSinceLastComm(0)
, m_MouseInputMode(MOUSE_INPUT_NONE)
, m_OverlayMgr(nullptr)
, m_TopLevelOverlayMgr(nullptr)
, m_DbNetId( NET_ID_NOT_CONNECTED )
, m_TimeSinceLastLockstep( 0 )
, m_NetRate( 0 )
, m_InputRate( 0 )
, m_InputFrame( 0 )
, m_InputTime( 0 )
, m_Ping1SendTime( 0 )
, m_WantPing( false )
, m_Ping1RequestId( 0 )
, m_DelayedEvents( 1 )
, m_ClientId( 0 )
, m_LastMousePos(0,0)
, m_LastAspectRatio( 0.f )
, m_InputDeviceId( eg_input_device_id::NONE )
{
	m_RemoteInputBuffer.Clear();
	m_LocalInputBuffer.Clear();
	m_InputCmdState.Init();

	m_BgScape = EGNewObject<EGSoundScape>( eg_mem_pool::Audio );
	m_AmbientSoundManager = EGAmbientSoundManager::Create();
}

EGClient::~EGClient()
{
	EGDeleteObject( m_AmbientSoundManager );
	m_AmbientSoundManager = nullptr;

	EGDeleteObject( m_BgScape );
	m_BgScape = nullptr;

	// Purge delayed events.
	while( m_DelayedEvents.HasItems() )
	{
		egDelayedRemoteEvent* DelayedEvent = m_DelayedEvents.GetFirst();
		m_DelayedEvents.Remove( DelayedEvent );
		delete DelayedEvent;
	}
}

void EGClient::SendCmd( eg_cpstr Cmd )
{
	eg_string CmdStr( Cmd );
	egParseFuncInfo ParseInfo;
	EGPARSE_RESULT Res = EGParse_ParseFunction( CmdStr , &ParseInfo );
	if( EGPARSE_OKAY != Res )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Invalid command: %s." , EGParse_GetParseResultString( Res ) );
		assert( false );
		return;
	}

	eg_string_crc CrcCmd = eg_string_crc(ParseInfo.FunctionName);

	if( eg_crc("Ping") == CrcCmd )
	{
		m_Ping1RequestId++;
		m_Ping1SendTime = Timer_GetRawTime();
		m_WantPing = true;
	}
	else
	{
		assert( false ); //Command not recognized.
	}
}

void EGClient::BindInput( const EGInput& Input, eg_cpstr StrAction, eg_cpstr StrGp, eg_cpstr StrKb )
{
	Input.Bind( m_InputBindings , StrAction , StrGp , StrKb );
}

void EGClient::Init( const egClientId& Id, EGClass* GameClass )
{
	assert( GameClass && GameClass->IsA( &EGGame::GetStaticClass() ) );
	assert( m_ClientId == egClientId(0) ); // Client id already set?
	m_ClientId = Id;
	m_GameClass = GameClass;
	m_InputBindings = EGInput::Get().GetDefaultBindings();
	m_OverlayMgr = new ( eg_mem_pool::System ) EGOverlayMgr( this );
	m_MenuStack = new ( eg_mem_pool::System ) EGMenuStack( this );
	m_TopLevelOverlayMgr = new ( eg_mem_pool::System ) EGOverlayMgr( this );
	Init_DebugItems(true);

	m_BgScape->Init();
	m_NetComm.Init();

	assert( nullptr == m_EntWorld );
	m_EntWorld = EGNewObject<EGEntWorld>( eg_mem_pool::System );
	if( m_EntWorld )
	{
		m_EntWorld->InitWorld( eg_ent_world_role::Client , &m_NetMsgBuffer , this , m_GameClass );
		m_EntWorld->PrimaryMapChangedDelegate.AddUnique( this , &ThisClass::OnPrimaryMapChanged );
		m_EntWorld->MapLoadedDelegate.AddUnique( this , &ThisClass::OnMapLoaded );
		m_EntWorld->TerrainLoadedDelegate.AddUnique( this , &ThisClass::OnTerrainLoaded );
	}
	EGClass* WorldRenderPipeClass = Engine_GetRenderPipeClass();
	m_WorldRenderPipe = EGNewObject<EGWorldRenderPipe>( WorldRenderPipeClass , eg_mem_pool::System );
}

void EGClient::Deinit()
{	
	m_NetComm.Deinit();
	m_BgScape->Deinit();

	Init_DebugItems(false);
	EG_SafeDelete( m_TopLevelOverlayMgr );
	EG_SafeDelete( m_MenuStack );
	EG_SafeDelete( m_OverlayMgr );

	if( m_WorldRenderPipe )
	{
		EGDeleteObject( m_WorldRenderPipe );
		m_WorldRenderPipe = nullptr;
	}

	if( m_EntWorld )
	{
		EGDeleteObject( m_EntWorld );
		m_EntWorld = nullptr;
	}
}

void EGClient::FinalizeLoad()
{
	m_ClientTime = 0;
	m_RemoteInputBuffer.Clear();
	m_LocalInputBuffer.Clear();

	OnMapLoadComplete();
}

void EGClient::Init_DebugItems(eg_uint bInit)
{
	if(bInit)
	{
		m_DbItems.Init();
	}
	else
	{
		m_DbItems.Deinit();
	}
}

void EGClient::SetupCamera( class EGClientView& View ) const
{
	View.SetPose( CT_Default , 90.f , .1f , 1000.f );
}

void EGClient::OnRemoteEvent( const egRemoteEvent& Event )
{
	EGGame* Game = SDK_GetGame();
	if( Game )
	{
		Game->HandleRemoteEvent( INVALID_ID , Event );
	}
}

void EGClient::Connect(eg_cpstr strServer)
{
	EGLogf( eg_log_t::Client , "Connection to %s requested." , strServer ? strServer : "Local Server" );
	Disconnect();

	if( strServer )
	{
		eg_string ServerWithPort = strServer;
		if( !ServerWithPort.Contains( ":" ) ) //If the port wasn't specified use the port in the settings.
		{
			ServerWithPort.Append( EGString_Format( ":%u" , GlobalConfig_ServerPort.GetValue() ) );
		}
		m_NetComm.Connect( ServerWithPort );
	}
	else
	{
		m_NetComm.Connect( nullptr );
	}
	m_RemoteInputBuffer.Clear();
	m_LocalInputBuffer.Clear();
	OnConnected();
}

void EGClient::Disconnect()
{
	OnDisconnected();
	//Since we are doing a lot of writing, we'll lock the console.
	EGLogf( eg_log_t::Client , "Client: Disconnecting." );
	m_NetComm.Disconnect();
	m_LockstepId = INVALID_ID;
	
	if( m_EntWorld )
	{
		m_EntWorld->ClearWorld();
	}
}

void EGClient::SetInputDevice( eg_input_device_id NewInputDevice )
{
	m_InputDeviceId = NewInputDevice;
}

void EGClient::Update_Camera( eg_real DeltaTime )
{
	unused( DeltaTime );

	SetupCamera( m_ClientView );
	m_ClientView.Update();
	if( m_EntWorld )
	{
		m_EntWorld->SetActivityPos( m_ClientView.GetCamera().GetPose().GetPosition() );
	}
}

void EGClient::OnPrimaryMapChanged( EGGameMap* GameMap )
{
	unused( GameMap );
}

void EGClient::OnMapLoaded( EGGameMap* GameMap )
{
	unused( GameMap );

	FinalizeLoad();
}

void EGClient::OnTerrainLoaded( EGTerrain2* GameTerrain )
{
	unused( GameTerrain );
}

void EGClient::Update( eg_real DeltaTime )
{
	egLockstepCmds InputCmds = EGInput::Get().UpdateCmds( m_InputDeviceId , m_InputBindings , DeltaTime , m_InputCmdState , m_ClientView.GetGameAspectRatio() );
	if( ConDraw_IsActive() )
	{
		InputCmds.Clear();
	}

	if( !IsConnected() )
	{
		m_RemoteInputBuffer.Clear();
		m_LocalInputBuffer.Clear();
		m_Ping1SendTime = 0;
		return;
	}

	if( DebugConfig_DrawFPS.GetValue() )
	{
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "C. %u FPS: %u , Net Rate: %ums" , m_DbNetId.Id , eg_uint(1.0f/DeltaTime) , eg_uint(m_NetRate*1000) ) );
	}

	if( (1.f/30.f) < DeltaTime )
	{
		if( DebugConfig_ReportSlowFrames.GetValue() )
		{
			EGLogf( eg_log_t::Warning , "Had a slow frame (%f seconds)." , DeltaTime );
		}
	}

	m_DeltaTime = DeltaTime;
	m_ClientTime += DeltaTime;
	m_fTimeSinceLastComm += DeltaTime;
	m_TimeSinceLastLockstep+=DeltaTime;
	m_OneMinuteCycle += DeltaTime;
	if( m_OneMinuteCycle >= 60.f )
	{
		m_OneMinuteCycle = 0.f;
	}

	Net_BeginFrame();

	if( m_EntWorld )
	{
		m_EntWorld->Update( m_DeltaTime );
	}

	m_DbItems.Update( m_DeltaTime );
	m_LastMousePos = eg_vec2( InputCmds.GetMouseX() , InputCmds.GetMouseY() );
	m_LastAspectRatio = m_ClientView.GetViewport().GetAspectRatio();
	m_OverlayMgr->Update( DeltaTime , m_MenuStack->Len() == 0 && WantsMouseCursor() ? &InputCmds : nullptr , m_LastAspectRatio );
	m_MenuStack->Update( DeltaTime , &InputCmds , m_LastAspectRatio );
	m_TopLevelOverlayMgr->Update( DeltaTime , m_MenuStack->Len() == 0 && WantsMouseCursor() ? &InputCmds : nullptr , m_LastAspectRatio );
	// If there is something on the menu stack (or was something on the stack, it consumes all input)
	if( m_MenuStack->HasStackChanged() || !m_MenuStack->IsEmpty() )
	{
		InputCmds.Clear();
	}
	m_BgScape->Update( DeltaTime );

	m_RemoteInputBuffer.CatenateNextFrame(InputCmds); //Input is catenated until the next lockstep frame request.
	m_LocalInputBuffer = InputCmds;

	Update_Camera( DeltaTime );

	//Set the listener position as well:
	//We actually just want to stick with the up direction, so that looking up and down does
	//not disorient:
	{
		const eg_transform& CamPose = m_ClientView.GetCamera().GetPose();

		egAudioListenerInfo LI;
		const eg_vec4 CamPos = CamPose.GetPosition();
		const eg_vec4 CamAt  = eg_vec4(0.f,0.f,1.f,0.f)*CamPose;
		const eg_vec4 CamUp  = eg_vec4(0.f,1.f,0.f,0.f)*CamPose;
		LI.vPos = CamPos;
		//The at is a direction for the audio whereas it is a position for the camera:
		LI.vAt = CamAt;
		LI.vAt.NormalizeThisAsVec3();
		LI.vVel = eg_vec4(0,0,0,0);
		LI.vUp = CamUp;
		LI.vUp.NormalizeThisAsVec3();

		MainAudioList->SetListener(&LI);

		if( m_AmbientSoundManager )
		{
			m_AmbientSoundManager->Update( DeltaTime , MainAudioList , CamPose );
		}
	}

	Update_BroadCastInput( DeltaTime );

	Net_EndFrame();

	if( m_WorldRenderPipe )
	{
		m_WorldRenderPipe->Update( DeltaTime );
	}
}

void EGClient::Draw_UI()
{
	EGInputGlyphs_SetClientForInputGlyphs( this );
	m_OverlayMgr->Draw();
	m_MenuStack->Draw();
	m_TopLevelOverlayMgr->Draw();
	EGInputGlyphs_SetClientForInputGlyphs( nullptr );
	//The mouse comes in in the range of (-1, 1) x (-1, 1), but the window
	//"desktop" size is in the range of (-1*aspect, 1*aspect) x (-1, 1) so
	//it is necessary to adjust the position of the mouse so that it is
	//displayed in the correct position.
	if( (m_MenuStack->HasInput() || WantsMouseCursor()) && (m_InputDeviceId == eg_input_device_id::KBM) )
	{
		m_MenuStack->DrawCursor( m_LastAspectRatio , m_LastMousePos.x , m_LastMousePos.y );
	}
}

EGEnt* EGClient::GetEnt( eg_ent_id Id )
{
	return m_EntWorld ? m_EntWorld->GetEnt( Id ) : nullptr;
}

const EGEnt* EGClient::GetEnt( eg_ent_id Id ) const
{
	return m_EntWorld ? m_EntWorld->GetEnt( Id ) : nullptr;
}
	
void EGClient::Draw( eg_bool bForceSimple )
{
	MainDisplayList->SetFloat( F_ONEMINUTECYCLE , EG_Clamp( m_OneMinuteCycle/60.f , 0.f , 1.f ) );
	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "One Minute Cycle: {0}" , m_OneMinuteCycle/60.f ) );
	MainDisplayList->SetScreenViewport( m_ClientView.GetViewport() );

	//The first thing to do is calculate what is visible in the actual scene.
	//This visibility information is stored in the 0 register.
	m_ClientView.PreSceneSetup();
	if( m_EntWorld )
	{
		m_EntWorld->UpdateScenGraph( m_ClientView.GetCamera() );
	}

	if( m_EntWorld && m_WorldRenderPipe )
	{
		egWorldRenderPipeDrawSpecs WorldDrawSpecs;
		WorldDrawSpecs.bIsSplitscreen = bForceSimple;
		m_WorldRenderPipe->DrawSceneGraph( m_EntWorld->GetScenGraph() , m_ClientView.GetCamera() , WorldDrawSpecs );
	}

	// At this point the depth buffer should still contain the real scene's
	// depth values, so when drawing debug stuff they should be sorted correctly
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF );
	Draw_DebugLights();
	Draw_DebugItems();
	Draw_DebugZOverlay();
	MainDisplayList->PopDepthStencilState();

	Draw_UI();
}

eg_bool EGClient::WantsMouseCursor() const
{
	if( m_MenuStack->Len() > 0 )
	{
		return m_MenuStack->HasInput();
	}

	return m_MouseInputMode == MOUSE_INPUT_CURSOR;
}

void EGClient::Draw_DebugLights()
{
	if( m_EntWorld && DebugConfig_DrawCloseLights.GetValue() )
	{
		EGLogToScreen::Get().Log( this , 88 , 1.f , "Cannot draw lights wihtout knowing what ent to draw them for." );
		// m_EntWorld->DrawLights( m_PCEnt , m_DbItems.DbLight );
	}
}

void EGClient::Draw_DebugZOverlay()
{
	if( DebugConfig_DrawZOverlay.GetValue() )
	{
		eg_mat m1 = eg_mat::I;
		m1 = eg_mat::BuildScaling(eg_vec3(0.5f, 0.5f, 1.0f));
		m1.tx = 0.5f;
		m1.ty = 0.5f;
		MainDisplayList->SetViewTF(eg_mat::I);
		MainDisplayList->SetProjTF(eg_mat::I);
		MainDisplayList->SetWorldTF(m1);
		MainDisplayList->DrawDebugZOverlay( m_ClientView.GetCamera().GetNear() , m_ClientView.GetCamera().GetFar() , .92f );
	}
}

void EGClient::Draw_DebugItems()
{
	m_ClientView.PreSceneSetup();
	MainDisplayList->SetWorldTF(eg_mat::I);
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	MainDisplayList->SetMaterial(EGV_MATERIAL_NULL);

	if( m_EntWorld && DebugConfig_DrawMapBBs.GetValue() )
	{
		m_EntWorld->DrawMapBBs();
	}

	if( m_EntWorld && DebugConfig_DrawMapGraphs.GetValue() )
	{
		m_EntWorld->DrawMapGraphs( m_DbItems.DbGraphVertex );
	}

	if( m_EntWorld && DebugConfig_DrawPortals.GetValue() )
	{
		m_EntWorld->DrawMapPortals( m_DbItems.DbPortal );
	}

	if( DebugConfig_DrawEntBBs.GetValue() )
	{
		if( m_EntWorld )
		{
			m_EntWorld->DrawEntBBs( m_DbItems.DbBox );
		}
	}

	if( m_WorldRenderPipe )
	{
		m_WorldRenderPipe->DrawDebugItems( MainDisplayList );
	}

	//Done.
	MainDisplayList->PopDefaultShader();
}

EGEnt* EGClient::SDK_GetEnt( eg_ent_id Id )
{
	EGEnt* Ent = GetEnt( Id );
	return Ent;
}

const EGEnt* EGClient::SDK_GetEnt( eg_ent_id Id ) const
{
	return GetEnt( Id );
}

void EGClient::SDK_ClearAllOverlays()
{
	if( m_OverlayMgr )
	{
		m_OverlayMgr->Clear();
	}

	if( m_TopLevelOverlayMgr )
	{
		m_TopLevelOverlayMgr->Clear();
	}
}

EGMenu* EGClient::SDK_AddOverlay( eg_string_crc OverlayId , eg_real DrawPriority , eg_bool bTopLevel /*= false*/ )
{
	EGMenu* OverlayOut = nullptr;

	if( bTopLevel )
	{
		if( m_TopLevelOverlayMgr )
		{
			OverlayOut = m_TopLevelOverlayMgr->AddOverlay( OverlayId , DrawPriority );
		}
	}
	else
	{
		if( m_OverlayMgr )
		{
			OverlayOut = m_OverlayMgr->AddOverlay( OverlayId , DrawPriority );
		}
	}

	return OverlayOut;
}

void EGClient::SDK_RemoveOverlay( EGMenu* Overlay )
{
	if( m_OverlayMgr && m_OverlayMgr->OwnsOverlay( Overlay ) )
	{
		m_OverlayMgr->RemoveOverlay( Overlay );
	}
	if( m_TopLevelOverlayMgr && m_TopLevelOverlayMgr->OwnsOverlay( Overlay ) )
	{
		m_TopLevelOverlayMgr->RemoveOverlay( Overlay );
	}
}

void EGClient::SDK_RunServerEvent( const egRemoteEvent& Event )
{
	egDelayedRemoteEvent* NewRemoteEvent = new egDelayedRemoteEvent();
	NewRemoteEvent->Event = Event;
	m_DelayedEvents.InsertLast( NewRemoteEvent );
}

void EGClient::SDK_RunClientEvent( const egRemoteEvent& Event )
{
	OnRemoteEvent( Event );
}

EGGame* EGClient::SDK_GetGame()
{
	return m_EntWorld ? m_EntWorld->GetGame() : nullptr;
}

const EGGame* EGClient::SDK_GetGame() const
{
	return m_EntWorld ? m_EntWorld->GetGame() : nullptr;
}

void EGClient::SDK_ClearMenuStack()
{
	m_MenuStack->Clear();
}

EGMenu* EGClient::SDK_PushMenu( eg_string_crc MenuId )
{
	return m_MenuStack->Push( MenuId );
}

void EGClient::SDK_PlayBgMusic( eg_cpstr Filename )
{
	m_BgScape->FadeToBgMusic( Filename , 0.5f , 2.f );
}

void EGClient::SDK_SetShowMouse( eg_bool bShowMouse )
{
	m_MouseInputMode = bShowMouse ? MOUSE_INPUT_CURSOR : MOUSE_INPUT_NONE;
}

void EGClient::SDK_GetLockstepCmds( egLockstepCmds* Cmds ) const
{
	*Cmds = m_LocalInputBuffer;
}

const EGGameMap* EGClient::SDK_GetMap() const
{
	return m_EntWorld ? m_EntWorld->GetPrimaryMap() : nullptr;
}

eg_bool EGClient::SDK_AskStateQuestion( eg_client_state_q Question )
{
	eg_bool Res = false;

	switch( Question )
	{
		case eg_client_state_q::UNKNOWN:
			Res = false; 
			break;
		case eg_client_state_q::IS_CONNECTED:
			Res = IsConnected();
			break;
		case eg_client_state_q::HAS_MAP:
			Res = SDK_GetMap() != nullptr;
			break;
		case eg_client_state_q::IS_SAVING:
			Res = MainLoader->IsSaving();
			break;
		case eg_client_state_q::IS_LOADING_THREAD_ACTIVE:
			Res = MainLoader->IsSaving() || MainLoader->IsLoading();
			break;
	}

	return Res;
}

void EGClient::egDebugItems::Init()
{
	DbLight = EGNewObject<EGDebugSphere>( eg_mem_pool::System );
	DbPortal = EGNewObject<EGDebugSphere>( eg_mem_pool::System );
	DbGraphVertex = EGNewObject<EGDebugSphere>( eg_mem_pool::System );
	DbBox = EGNewObject<EGDebugBox>( eg_mem_pool::System );
}

void EGClient::egDebugItems::Deinit()
{
	EG_SafeRelease( DbLight );
	EG_SafeRelease( DbPortal );
	EG_SafeRelease( DbGraphVertex );
	EG_SafeRelease( DbBox );
}

void EGClient::egDebugItems::Update( eg_real DeltaTime )
{
	DbLight->Update( DeltaTime );
	DbPortal->Update( DeltaTime );
	DbGraphVertex->Update( DeltaTime );
	DbBox->Update( DeltaTime );
}

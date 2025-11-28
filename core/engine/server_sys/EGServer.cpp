// (c) 2011 Beem Software

#include "EGServer.h"
#include "EGGame.h"
#include "EGGameLoader.h"
#include "EGEnt.h"
#include "EGEntDict.h"
#include "EGLoader.h"
#include "EGParse.h"
#include "EGPhysSim2.h"
#include "EGConsole.h"
#include "EGEngine.h"
#include "EGThread.h"
#include "EGSaveGame.h"
#include "EGDebugText.h"
#include "EGFileData.h"
#include "EGPhysBodyComponent.h"
#include "EGEntWorld.h"
#include "EGGameMap.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGServer )

static const eg_uint SRV_LOCAL_FPS  = 60;
static const eg_real SRV_LOCAL_FRAME_TIME    = 1.0f/SRV_LOCAL_FPS;
static const eg_uint SRV_LOCAL_FRAME_TIME_MS = 1000/SRV_LOCAL_FPS;

static const eg_uint SRV_ONLINE_FPS = 30;
static const eg_real SRV_ONLINE_FRAME_TIME    = 1.0f/SRV_ONLINE_FPS;
static const eg_uint SRV_ONLINE_FRAME_TIME_MS = 1000/SRV_ONLINE_FPS;

static const eg_real SRV_ONLINE_UPDATEENT_RATE = 1.f/15.f;

EGServer::EGServer()
: m_nGameTime(0)
, m_Cmds( CT_Clear , m_SrvCmdDefault )
, m_Timer()
, m_GameLoader(nullptr)
, m_LockstepFrame(0)
, m_DeltaTime(0)
, m_TimeSinceLastEntSend(0)
, m_UpdateEntRate(0)
, m_bHasMapToLoad(false)
{
	for( egClientInfo& Client : m_Clients )
	{
		Client.Clear();
	}

	SrvCmd_Init();
}

EGServer::~EGServer()
{
	
}

void EGServer::QueMsg( eg_cpstr Msg )
{
	EGFunctionLock Lock( &m_MsgLock );
	m_MsgStack.EnQueue( Msg );
}

void EGServer::OnRemoteEvent( const eg_lockstep_id& SenderId , const egRemoteEvent& Event )
{
	EGGame* Game = SDK_GetGame();
	if( Game )
	{
		Game->HandleRemoteEvent( SenderId , Event );
	}
}

void EGServer::Update_ProcessMsgs()
{
	EGFunctionLock Lock( &m_MsgLock );

	//for(eg_uint l = 0; m_MsgStack.HasItems() && l < MAX_MESSAGES; l++)

	//Only one message is handled per frame, this is so that if a load message
	//is called, the messages after it won't be processed until after the load
	//completes.
	if( m_MsgStack.HasItems() )
	{
		HandleMsg( m_MsgStack.Front() );
		m_MsgStack.DeQueue();
	}
}

void EGServer::SpawnMapEnts()
{
	EGLogf( eg_log_t::Server , "Spawning map entities..." );

	if( m_EntWorld )
	{
		m_EntWorld->SpawnMapEnts( m_EntWorld->GetPrimaryMap() );
	}
}

void EGServer::HandleMsg(eg_cpstr sMsg)
{
	egParseFuncInfo Info;
	EGPARSE_RESULT r = EGParse_ParseFunction(sMsg, &Info);
	if(EGPARSE_OKAY != r)
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ "Invalid function call \"%s\". Parse error %u: %s.", sMsg, r, ::EGParse_GetParseResultString(r));
		return;
	}

	if( EGString_Equals( Info.SystemName , "game" ) )
	{
		OnConsoleCmd( sMsg );
	}
	else if( m_Cmds.Contains( eg_string_crc(Info.FunctionName) ) )
	{
		const egSrvCmdInfo& Cmd = m_Cmds.Get( eg_string_crc(Info.FunctionName) );
		if( -1 == Cmd.NumParms || Info.NumParms == Cmd.NumParms )
		{
			Cmd.FnCall( this , &Info );
		}
		else
		{
			EGLogf( eg_log_t::Error , __FUNCTION__ "Invalid function call \"%s\". Expected %u arg(s).", sMsg, Cmd.NumParms);
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ "Invalid function call \"%s\". Function not registered.", sMsg );
	}
}

void EGServer::Init()
{
	assert( m_GameClass && m_GameClass->IsA( &EGGame::GetStaticClass() ) );

	m_NetComm.BeginHosting();
	
	for(eg_uint i=0; i<countof(m_Clients); i++)
	{
		m_Clients[i].NetCommId = NET_ID_NOT_CONNECTED;
		m_Clients[i].bPingedWaiting = false;
		m_Clients[i].LockstepId = eg_lockstep_id::IndexToId(i);
	}

	m_nGameTime = 0;

	m_Timer.Init();

	m_EntWorld = EGNewObject<EGEntWorld>( eg_mem_pool::System );
	if( m_EntWorld )
	{
		Net_BeginFrame();
		m_EntWorld->InitWorld( eg_ent_world_role::Server , &m_NetMsgBuffer , this , m_GameClass );
		Net_EndFrame();
	}

	if( m_EntWorld )
	{
		m_EntWorld->BeginPhysSim( "EGBpPhysSim" );
	}

}

void EGServer::Deinit()
{
	//should broadcast a disconnect to all clients:
	Net_BeginFrame();
	for( eg_uint i=0; i<countof(m_Clients); i++ )
	{
		egClientInfo* Info = &m_Clients[i];
		if( Info->IsConnected() )
		{
			m_NetComm.DropConnectedClient( Info->NetCommId );
			Info->NetCommId = NET_ID_NOT_CONNECTED;
			m_EntWorld->OnClientLeaveWorld( Info->LockstepId );
			if( m_EntWorld && m_EntWorld->GetGame() )
			{
				m_EntWorld->GetGame()->OnClientDisconnected( Info->LockstepId );
			}
		}
	}

	ResetLoad();

	Net_EndFrame();
	for( egClientInfo& Client : m_Clients )
	{
		Client.Clear();
	}

	if( m_EntWorld )
	{
		EGDeleteObject( m_EntWorld );
		m_EntWorld = nullptr;
	}
	m_NetMsgBuffer.FlushMsgs();

	m_NetComm.EndHosting();
}

void EGServer::Update( eg_real DeltaTimeDoNotUse )
{
	unused( DeltaTimeDoNotUse );
	
	eg_bool IsOnline = m_NetComm.HasOnlineClients();
	const eg_real SrvFrameTime = IsOnline ? SRV_ONLINE_FRAME_TIME : SRV_LOCAL_FRAME_TIME;
	const eg_uint SrvFrameTimeMs = IsOnline ? SRV_ONLINE_FRAME_TIME_MS : SRV_LOCAL_FRAME_TIME_MS;
	eg_real Fps = m_Timer.GetRawElapsedSec();
	//0. We won't process the frame if not enough time has passed.
	if( Fps < SrvFrameTime )
	{
		EGThread_Sleep( 0 );
		return;
	}
	m_DeltaTime = SrvFrameTime;
	m_UpdateEntRate = IsOnline ? SRV_ONLINE_UPDATEENT_RATE : m_DeltaTime;

	if( DebugConfig_DrawGameState.GetValueThreadSafe() )
	{
		eg_string State = "Server";

		State += IsOnline ? " (Online)" : " (Offline)";

		if( m_EntWorld )
		{
			EGLogToScreen::Get().Log( this , 0 , 1.f , State );
			EGLogToScreen::Get().Log( this , 1 , 1.f , EGString_Format( "Ent Updates: %u, Rate: %ums" , m_EntWorld->Net_GetDirtyEntSendSize() , static_cast<eg_uint>(m_UpdateEntRate*1000) ) );
			EGLogToScreen::Get().Log( this , 2 , 1.f , EGString_Format( "Pending Input Size: %u" , m_Clients[0].InCmds.Len() ) );
		}
	}

	if( DebugConfig_DrawFPS.GetValueThreadSafe() )
	{
		EGLogToScreen::Get().Log( this , 3 , 1.f , EGString_Format( "S. FPS: %u" , eg_uint(1.0f/Fps) ) );
	}

	m_Timer.Update();

	Net_BeginFrame();

	Update_PingClients();

	MainLoader->ProcessCompletedLoads( EGLoader::LOAD_THREAD_SERVER );

	//If we are in the process of a load we halt the server here, that way
	//future messages sent to the server that are meant to happen after the
	//load won't be ignored.
	if( m_GameLoader )
	{
		if( m_GameLoader->IsLoaded() )
		{
			BeginLoad_Finalize();
		}
		Net_EndFrame();
		return;
	}

	if( m_bHasMapToLoad )
	{
		m_bHasMapToLoad = false;

		// Since we are going to load and it'll take a while till the next
		// net frame we'll flush all the net messages now.
		Net_EndFrame();

		Net_BeginFrame();
		BeginLoad( m_NextMapToLoad );
		Net_EndFrame();
		return;
	}

	Update_ProcessMsgs();

	m_nGameTime += SrvFrameTimeMs; //Game time only advances if the game is running.

	// Update the global server and all active entities
	{
		if( m_EntWorld )
		{
			m_EntWorld->SetGameTimeMs( m_nGameTime );
			m_EntWorld->Update( m_DeltaTime );
		}
	}

	Update_ClientInfo();

	Net_EndFrame();
}

void EGServer::Update_PingClients()
{
	for(eg_uint nClient = 0; nClient < countof(m_Clients); nClient++)
	{
		egClientInfo* pInfo = &m_Clients[nClient];
		if( !pInfo->IsConnected() )continue;

		//We ping every 5 secons no matter what:
		if( pInfo->PingTimer >= 5.0f )
		{
			m_NetMsgBuffer.AddStrMsg( pInfo->NetCommId , "Ping(0)" );
			pInfo->PingTimer = 0;
		}
		pInfo->PingTimer += m_DeltaTime;
	}
}

void EGServer::Update_ClientInfo()
{
	for(eg_uint nClient = 0; nClient < countof(m_Clients); nClient++)
	{
		egClientInfo* pInfo = &m_Clients[nClient];
		if( !pInfo->IsConnected() )continue;

		#if 1
		//First of all, if it has been too long since we've heard from the
		//client, we need to ask it if it is still there.
		static const eg_uint MID_CLIENT_DELAY = 30000;
		static const eg_uint MAX_CLIENT_DELAY = 60000;
		if( (m_nGameTime > pInfo->nLastComm + MID_CLIENT_DELAY) && !pInfo->bPingedWaiting	)
		{
			::EGLogf(
				eg_log_t::Server ,
				("Haven't heard from client %i in %i seconds. Pinging..."),
				pInfo->NetCommId,
				MID_CLIENT_DELAY/1000);
			m_NetMsgBuffer.AddStrMsg( pInfo->NetCommId , "Ping(0)" );
			pInfo->bPingedWaiting = true;
		}

		if(m_nGameTime > pInfo->nLastComm + MAX_CLIENT_DELAY)
		{
			::EGLogf( eg_log_t::Server , ("Haven't heard from client %i in %i seconds. Dropping..."), pInfo->NetCommId.Id, MAX_CLIENT_DELAY/1000);

			//We post a thread message, becasue it isn't a good idea to change
			//the size of m_Clients, while we are iterating through them.
			m_NetComm.DropConnectedClient( pInfo->NetCommId );
		}
		#endif

		if( pInfo->ExcessInputs > 0 )
		{
			EGLogf( eg_log_t::Verbose , "Receiving input too fast, catenated %u frames." , pInfo->ExcessInputs );
			pInfo->ExcessInputs = 0;
		}

		if( pInfo->InCmds.Len() > 1 ) //If there is pending input, use it.
		{
			pInfo->InCmds.RemoveFirst();
			if( pInfo->FramesWithoutInput > 0 )
			{
				//EGLogf( "Server went %u frames without input from %u." , pInfo->FramesWithoutInput , egNetCommId::IdToIndex(pInfo->NetCommId) );
				pInfo->FramesWithoutInput = 0;
			}
		}
		else if( pInfo->InCmds.Len() == 1 ) //If there has been no new input, clear presses and releases, but leave the rest of the state the same.
		{
			pInfo->FramesWithoutInput++;
			pInfo->InCmds[0].ClearPressesAndReleases();
		}
		else if( pInfo->InCmds.Len() == 0 ) //If the client has no input, insert an empty input.
		{
			egLockstepCmds NewCmds;
			NewCmds.Clear();
			pInfo->InCmds.InsertLast( NewCmds );
		}		
	}
}

void EGServer::DoSave(eg_cpstr strFile)
{
	::EGLogf(eg_log_t::Server, ("Saving \"%s\"."), strFile);

	EGSaveGame* pSaver = new EGSaveGame;

	if( pSaver )
	{
		// Header
		EGSaveGame::egHeader Header( CT_Default );
		Header.MapTimeMs = m_nGameTime;
		Header.bSpawnMapsEnts = false;
		pSaver->AddHeader( Header );
		
		if( m_EntWorld )
		{
			m_EntWorld->SaveToSaveGame( *pSaver );
		}

		EGFileData fout( eg_file_data_init_t::HasOwnMemory );

		pSaver->SaveBinary( fout );

		EG_SafeDelete( pSaver );

		MainLoader->SaveData(strFile, fout.GetDataAs<eg_byte>(), fout.GetSize());

	}
}

void EGServer::BeginLoad(eg_cpstr strFileIn)
{
	//Reset load clears everything that was loaded before.
	ResetLoad();

	assert( m_GameLoader == nullptr );
	m_LoadSaveFilename = strFileIn;
	
	EGLogf( eg_log_t::Server , "Beginning save game load \"%s\"...", m_LoadSaveFilename.String() );

	// If no filename was specified, we are good to just clear.
	if( m_LoadSaveFilename.Len() > 0 )
	{
		eg_string strFileEx = EGString_Format( "%s.esave" , m_LoadSaveFilename.String() );

		m_GameLoader = EGNewObject<EGGameLoader>();
		m_GameLoader->InitGameLoader( strFileEx , m_EntWorld );
	}
	else
	{
		//Now create an entity for each client.
		for( eg_uint nClient = 0; nClient < countof( m_Clients ); nClient++ )
		{
			egClientInfo* pInfo = &m_Clients[nClient];
			if( !pInfo->IsConnected() )
			{
				continue;
			}

			if( m_EntWorld )
			{
				m_EntWorld->OnClientEnterWorld( pInfo->LockstepId );
			}
		}
	}
}

void EGServer::BeginLoad_Finalize()
{
	assert_pointer( m_GameLoader );
	assert( m_GameLoader->IsLoaded() );

	assert_pointer( m_GameLoader );

	EGGameLoader& Loader = *m_GameLoader;

	//Broadcast the Server Load message:
	const EGSaveGame::egHeader& Header = Loader.m_SaveGame.m_Headers.IsValidIndex(0) ? Loader.m_SaveGame.m_Headers[0] : EGSaveGame::egHeader();
	eg_bool bSpawnMapEnts = Header.bSpawnMapsEnts;
	m_nGameTime = Header.MapTimeMs;

	//After everything in the load is complete we set the timer.
	m_nGameTime = 0;

	EGDeleteObject( m_GameLoader );
	m_GameLoader = nullptr;

	if( bSpawnMapEnts )
	{
		SpawnMapEnts();
	}

	//Now create an entity for each client.
	for(eg_uint nClient = 0; nClient < countof(m_Clients); nClient++)
	{
		egClientInfo* pInfo = &m_Clients[nClient];
		if( !pInfo->IsConnected() )
		{
			continue;
		}

		if( m_EntWorld )
		{
			m_EntWorld->OnClientEnterWorld( pInfo->LockstepId );
		}
	}
}

void EGServer::ResetLoad()
{
	for( eg_uint nClient = 0; nClient < countof( m_Clients ); nClient++ )
	{
		egClientInfo* pInfo = &m_Clients[nClient];
		if( !pInfo->IsConnected() )continue;
		if( m_EntWorld )
		{
			m_EntWorld->OnClientLeaveWorld( pInfo->LockstepId );
		}
		pInfo->InCmds.Clear();
	}

	if( m_EntWorld )
	{
		m_EntWorld->ClearWorld();
	}
}


EGServer::egClientInfo* EGServer::GetClientInfo( const egNetCommId& nID )
{
	for(eg_uint i=0; i<countof(m_Clients); i++)
	{
		egClientInfo* pInfo = &m_Clients[i];
		if( !pInfo->IsConnected() )continue;

		if(pInfo->NetCommId == nID)
		{
			assert( pInfo->LockstepId == eg_lockstep_id::IndexToId(i) );
			return pInfo;
		}
	}
	//::EGLogf(("Info was requested for nonexistent client %i"), nID);
	return nullptr;
}

EGEnt* EGServer::SpawnEnt( const eg_string_crc& EntDef, const eg_transform& Pose /*= CT_Default */, eg_cpstr InitString /*= "" */ )
{
	return m_EntWorld ? m_EntWorld->SpawnEnt( EntDef , Pose , InitString ) : nullptr;
}

void EGServer::DestroyEnt( eg_ent_id EntId )
{
	if( m_EntWorld )
	{
		m_EntWorld->DestroyEnt( EntId );
	}
}

void EGServer::SDK_PostCmd(eg_cpstr sCmd)
{
	QueMsg( sCmd );
}

void EGServer::SDK_GetClientMouseTarget( eg_lockstep_id LockStepId , eg_vec3& OrgOut , eg_vec3& DirOut ) const
{
	for( const egClientInfo& Info : m_Clients )
	{
		if( Info.IsConnected() && Info.LockstepId == LockStepId )
		{
			OrgOut = Info.vRayOrg;
			DirOut = Info.vRayDir;
			break;
		}
	}
}

void EGServer::SDK_GetCommandsForPlayer( eg_lockstep_id LockstepId , egLockstepCmds* Out ) const
{
	eg_uint Index = eg_lockstep_id::IdToIndex( LockstepId );
	assert( 0 <= Index && Index < countof(m_Clients) );
	Index = EG_Clamp<eg_uint>( Index , 0 , countof(m_Clients)-1 );
	if( m_Clients[Index].InCmds.Len() > 0 )
	{
		*Out = m_Clients[Index].InCmds[0];
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Attempted to get input commands when they didn't exist." );
		Out->Clear();
	}
}

EGEnt* EGServer::SDK_GetEnt(eg_ent_id nUEID)
{
	return GetEnt( nUEID );
}

EGEnt* EGServer::SDK_SpawnEnt( const eg_string_crc& EntDef, const eg_transform& Pose /*= CT_Default */, eg_cpstr InitString /*= "" */ )
{
	return SpawnEnt( EntDef , Pose , InitString );
}

void EGServer::SDK_DestroyEnt( eg_ent_id EntId )
{
	if( m_EntWorld )
	{
		m_EntWorld->DestroyEnt( EntId );
	}
}

void EGServer::SDK_RayCast( const eg_vec4& Position , const eg_vec4& Direction , egRaycastInfo* OutInfo )
{
	if( m_EntWorld )
	{
		const eg_vec4& vOrg = Position;
		const eg_vec4& vDir = Direction;
		assert( vDir.IsNormalizedAsVec3() );
		egEntWorldRaycastResult RaycastRes = m_EntWorld->RaycastWorld( vOrg , vDir );
		eg_uint nHitEnt = 0;
		OutInfo->nGroup = static_cast<eg_uint>(RaycastRes.Group);
		OutInfo->vHit = RaycastRes.HitPoint;
		OutInfo->nEntID = RaycastRes.HitEnt;
		OutInfo->vFrom = vOrg;
	}
}

void EGServer::SDK_RayCastFromEnt( const eg_ent_id EntId , const eg_vec4& Direction , egRaycastInfo* OutInfo ) const
{
	if( m_EntWorld )
	{
		//We get the information of the entity requesting the ray trace.
		EGEnt* pEnt = GetEnt( EntId );
		//Setup the raycast.
		eg_vec4 vOrg(pEnt->GetPose().GetPosition());
		//Calculate the look direction:
		eg_vec4 vDir = Direction;
		vDir.NormalizeThisAsVec3();
		assert( vDir.IsNormalizedAsVec3() );

		//When we do the raycast, we want to ignore the entity requesting it
		//so we temporarily change the group.
		eg_bool bRayCastState = pEnt->IsRaycastHitEnabled();
		pEnt->SetRaycastHitEnabled( false );

		egEntWorldRaycastResult RaycastRes = m_EntWorld->RaycastWorld( vOrg , vDir );
		OutInfo->nGroup = static_cast<eg_uint>(RaycastRes.Group);
		OutInfo->vHit = RaycastRes.HitPoint;
		OutInfo->nEntID = RaycastRes.HitEnt;
		OutInfo->vFrom = vOrg;

		pEnt->SetRaycastHitEnabled(bRayCastState);
	}
}

void EGServer::SDK_RunClientEvent( const egRemoteEvent& Event )
{
	egNetMsgData bcs;
	bcs.Header.nSize = sizeof(bcs.RemoteEvent);
	bcs.Header.nMsg = eg_net_msg::NET_REMOTE_EVENT;
	bcs.Header.NetId = NET_ID_ALL;
	bcs.RemoteEvent = Event;
	m_NetMsgBuffer.AddMsg( &bcs );
}

void EGServer::SDK_RunServerEvent( const eg_lockstep_id& SrcLockstepId , const egRemoteEvent& Event )
{
	OnRemoteEvent( SrcLockstepId , Event );
}

void EGServer::SDK_SaveGameState( eg_cpstr Filename )
{
	eg_string SaveFile = EGString_Format( "%s/%s.esave" , GAME_USER_PATH , Filename );
	DoSave(SaveFile);
}

void EGServer::SDK_ClearGlobalData()
{
	if( m_EntWorld )
	{
		m_EntWorld->ResetGame();
	}
}

void EGServer::SDK_ResetWorld()
{
	ResetLoad();

	//Now create an entity for each client.
	for(eg_uint nClient = 0; nClient < countof(m_Clients); nClient++)
	{
		egClientInfo* pInfo = &m_Clients[nClient];
		if( !pInfo->IsConnected() )
		{
			continue;
		}

		if( m_EntWorld )
		{
			m_EntWorld->OnClientEnterWorld( pInfo->LockstepId );
		}
	}
}

void EGServer::SDK_LoadLevel( eg_cpstr Filename, bool PrependUserPath /*= false */ )
{
	eg_string Request = PrependUserPath ? EGString_Format( "%s/%s", GAME_USER_PATH, Filename ) : Filename;

	m_bHasMapToLoad = true;
	m_NextMapToLoad = Request;
}

EGGame* EGServer::SDK_GetGame()
{
	return m_EntWorld ? m_EntWorld->GetGame() : nullptr;
}

void EGServer::SDK_LoadWorld( eg_cpstr Filename )
{
	SDK_ClearGlobalData();
	SDK_ResetWorld();
	if( m_EntWorld )
	{
		m_EntWorld->LoadWorld( Filename );
	}
}

EGEnt* EGServer::GetEnt( eg_ent_id Id ) const
{
	return m_EntWorld ? m_EntWorld->GetEnt( Id ) : nullptr;
}

void EGServer::SrvCmd_Init()
{
	static const egSrvCmdInfo List[] =
	{
		{ SrvCmd_ShowCmds , ("ShowCmds" ) , 0 , ( "" ) , ( "" ) },
		{ SrvCmd_Drop , ("Drop") , 1 , ( "ClientId" ) , ( "" ) },
		{ SrvCmd_Load , ("Load") , 1 , ( "Filename" ) , ( "" ) },
		{ SrvCmd_LoadWorld , ("LoadWorld") , 1 , ( "Filename" ) , ( "" ) },
		{ SrvCmd_Spawn , ("Spawn") , 4 , ( "CrcId , x , y , z" ) , ( "" ) },
		{ SrvCmd_Kill  , ("Kill") , 1 , ( "EntId" ) , ("") },
		{ SrvCmd_MapOp , ("MapOp") , 1 , ( "Op" ) , ( "Op = [ \"spawnents\" ]" ) },
		{ SrvCmd_ShowInfo , ("ShowInfo") , 1 , ( "InfoId" ) , ( "Op = [ \"edict\" , \"map\" , \"ai\" ]" ) },
		{ SrvCmd_Listen , "Listen" , 1 , "bOn" , "bOn = [true|false]" },
	};

	for( eg_uint i=0; i<countof(List); i++ )
	{
		assert( !m_Cmds.Contains( eg_string_crc(List[i].FnName) ) );
		m_Cmds.Insert( eg_string_crc(List[i].FnName) , List[i] );
	}
}

void EGServer::SrvCmd_ShowCmds( EGServer* _this , const struct egParseFuncInfo* Info )
{
	unused( Info );

	EGLogf( eg_log_t::Server , "Command List:" );
	EGLogf( eg_log_t::Server , "=============" );

	for( eg_uint i=0; i<_this->m_Cmds.Len(); i++ )
	{
		const egSrvCmdInfo& Info = _this->m_Cmds.GetByIndex( i );
		EGLogf( eg_log_t::Server , "server.%s( %s )", Info.FnName , Info.ParmList );
		eg_string StrParmHelp = Info.ParmHelp;
		if( Info.NumParms > 0 && StrParmHelp.Len() > 0 )
		{
			EGLogf( eg_log_t::Server , "\t%s" , StrParmHelp.String() );
		}
	}
}

void EGServer::SrvCmd_Drop( EGServer* _this , const struct egParseFuncInfo* Info )
{
	egNetCommId NetId = egNetCommId::RawToId( eg_string(Info->Parms[0]).ToUInt() );
	_this->m_NetComm.DropConnectedClient( NetId );
}

void EGServer::SrvCmd_Load( EGServer* _this , const struct egParseFuncInfo* Info )
{
	_this->BeginLoad( Info->Parms[0] );
}

void EGServer::SrvCmd_LoadWorld( EGServer* _this, const struct egParseFuncInfo* Info )
{
	_this->SDK_LoadWorld( Info->Parms[0] );
}

void EGServer::SrvCmd_Spawn( EGServer* _this , const struct egParseFuncInfo* Info )
{	
	eg_transform mPos = eg_transform::BuildTranslation( eg_vec3(eg_string(Info->Parms[1]).ToFloat() , eg_string(Info->Parms[2]).ToFloat(), eg_string(Info->Parms[3]).ToFloat()) );

	EGEnt* NewEnt = _this->SpawnEnt( eg_string_crc(Info->Parms[0]) , mPos );
	if( NewEnt )
	{
		EGLogf( eg_log_t::Server , "Spawned %s as %u" , Info->Parms[0] , NewEnt->GetID().Id );
	}
}

void EGServer::SrvCmd_Kill( EGServer* _this , const struct egParseFuncInfo* Info )
{
	//We have to go by ID.
	eg_ent_id Id = eg_ent_id::RawToId( eg_string(Info->Parms[0]).ToUInt() );
	_this->DestroyEnt( Id );
}

void EGServer::SrvCmd_MapOp( EGServer* _this , const struct egParseFuncInfo* Info )
{
	if(!eg_string(Info->Parms[0]).Compare(("spawnents")))
	{
		_this->SpawnMapEnts();
	}
}

void EGServer::SrvCmd_ShowInfo( EGServer* _this , const struct egParseFuncInfo* Info )
{
	if(!eg_string(Info->Parms[0]).Compare(("edict")))
	{
		EntDict_ShowInfo();
	}
	else if(!eg_string(Info->Parms[0]).Compare(("map")))
	{
		if( _this->m_EntWorld->GetPrimaryMap() )
		{
			_this->m_EntWorld->GetPrimaryMap()->ShowInfo();
		}
	}
	else if(!eg_string(Info->Parms[0]).Compare(("ai")))
	{
		if( _this->m_EntWorld )
		{
			_this->m_EntWorld->PrintDebugInfo();
		}
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ "%s was not recognized as a valid ShowInfo item." , Info->Parms[0] );
	}
}

void EGServer::SrvCmd_Listen( EGServer* _this , const struct egParseFuncInfo* Info )
{
	if( eg_string(Info->Parms[0]).EqualsI( "true" ) || eg_string(Info->Parms[0]).EqualsI( "1" ) )
	{
		EGLogf( eg_log_t::Server , "Server: Listening for clients..." );
		_this->m_NetComm.Listen( true );
	}
	else if( eg_string(Info->Parms[0]).EqualsI( "false" ) || eg_string(Info->Parms[0]).EqualsI( "0" ) )
	{
		EGLogf( eg_log_t::Server , "Server: Stopped listening for clients." );
		_this->m_NetComm.Listen( false );
	}
}

// (c) 2011 Beem Software

#include "EGClient.h"
#include "EGEnt.h"
#include "EGInput.h"
#include "EGParse.h"
#include "EGEngine.h"
#include "EGAudioList.h"
#include "EGEngine.h"
#include "EGTimer.h"
#include "EGGame.h"
#include "EGEntWorld.h"

void EGClient::Net_BeginFrame()
{
	assert( m_NetMsgBuffer.IsEmpty() );
	m_NetComm.BeginFrame();
	m_NetComm.ProcessReceivedMsgs( Net_OnReceivedMsg , this );

	if( m_fTimeSinceLastComm > 6.0f )
	{
		//If it has been a while since a NetComm, ping the server. If the connection
		//has been lost this will force a disconnect.
		EGLogf( eg_log_t::Client , "Haven't heard from server in %g seconds, pinging..." , m_fTimeSinceLastComm );
		m_fTimeSinceLastComm = 0;
		m_NetMsgBuffer.AddStrMsg( NET_ID_ALL , "Ping(0)" );
	}
}

void EGClient::Net_EndFrame()
{
	if( m_WantPing )
	{
		m_NetMsgBuffer.AddStrMsg( NET_ID_ALL , EGString_Format("Ping(%u)",m_Ping1RequestId) );
		m_WantPing = false;
	}

	// Process all events.
	while( m_DelayedEvents.HasItems() )
	{
		egDelayedRemoteEvent* DelayedEvent = m_DelayedEvents.GetFirst();
		m_DelayedEvents.Remove( DelayedEvent );
		egNetMsgData bcs;
		bcs.Header.nSize = sizeof(bcs.RemoteEvent);
		bcs.Header.nMsg = eg_net_msg::NET_REMOTE_EVENT;
		bcs.Header.NetId = NET_ID_ALL;
		bcs.RemoteEvent = DelayedEvent->Event;
		m_NetMsgBuffer.AddMsg( &bcs );
		delete DelayedEvent;
	}

	m_NetMsgBuffer.PumpMsgs( Net_PumpMsgs , this );
	m_NetComm.EndFrame();
}

void EGClient::Net_PumpMsgs( const egNetMsgData* Data , void* UserPointer )
{
	EGClient* _this = static_cast<EGClient*>(UserPointer);
	_this->m_NetComm.PostMsg( Data );
}

void EGClient::Net_OnReceivedMsg( const egNetMsgData* Data , void* UserPointer )
{
	EGClient* _this = static_cast<EGClient*>(UserPointer);
	_this->OnNetMessage( static_cast<eg_net_msg>(Data->Header.nMsg) , *Data );
}

void EGClient::OnNetMessage(eg_net_msg msg, const egNetMsgData& bc)
{		
	m_fTimeSinceLastComm = 0;

	if( eg_net_msg::NET_STRCMD == msg )
	{
		SM_StrCmd( bc );
		return;
	}

	if( eg_net_msg::EW_MARK_FirstMsg < bc.Header.nMsg && bc.Header.nMsg < eg_net_msg::EW_MARK_LastMsg )
	{
		if( m_EntWorld )
		{
			m_EntWorld->HandleNetMsg( bc );
		}

		return;
	}

	switch(msg)
	{
	case eg_net_msg::STOC_LOCKSTEP: SM_Lockstep(bc); break;
	case eg_net_msg::NET_REMOTE_EVENT:
	{
		egRemoteEvent Event = bc.RemoteEvent.ToEvent();
		OnRemoteEvent( Event );
	} break;
	default: assert(false); break; //Need to handle this.
	}
}

void EGClient::SM_Lockstep( const egNetMsgData& bc )
{
	m_NetRate = m_TimeSinceLastLockstep;
	if( m_EntWorld )
	{
		m_EntWorld->SetEntUpdateRate( bc.SrvLockstep.EntUpdateRate );
	}
	m_InputRate = bc.SrvLockstep.InputRate;
	m_InputFrame = bc.SrvLockstep.Frame;
}

void EGClient::Update_BroadCastInput( eg_real DeltaTime )
{
	unused( DeltaTime );

	//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Client input info: %f" , m_InputRate ) );
	if( m_TimeSinceLastLockstep > m_InputRate )
	{
		m_TimeSinceLastLockstep = 0;
		egNetMsgData bcs;
		bcs.Header.nMsg=eg_net_msg::CTOS_LOCKSTEP;
		bcs.Header.NetId=NET_ID_ALL;
		bcs.Header.nSize=sizeof(SC_Lockstep);
		bcs.CliLockstep.Cmds = m_RemoteInputBuffer;
		//If we posted input, let's also find out what the mouse is pointing at:
		eg_vec4 Org, Dir;
		eg_vec2 MousePos = m_MouseInputMode == MOUSE_INPUT_CURSOR ? m_RemoteInputBuffer.m_MousePos : eg_vec2(0.f,0.f);
		m_ClientView.GetCameraTraceVecs( MousePos.x , MousePos.y , &Org , &Dir );
		bcs.CliLockstep.Frame = m_InputFrame;
		bcs.CliLockstep.RayTrace.Org[0] = Org.x;
		bcs.CliLockstep.RayTrace.Org[1] = Org.y;
		bcs.CliLockstep.RayTrace.Org[2] = Org.z;

		bcs.CliLockstep.RayTrace.Dir[0] = Dir.x;
		bcs.CliLockstep.RayTrace.Dir[1] = Dir.y;
		bcs.CliLockstep.RayTrace.Dir[2] = Dir.z;

		m_NetMsgBuffer.AddMsg( &bcs);

		m_RemoteInputBuffer.Clear();
	}
}

void EGClient::SM_StrCmd( const egNetMsgData& Info )
{
	eg_string CmdStr( Info.StrCmd.Cmd );
	egParseFuncInfo ParseInfo;
	EGPARSE_RESULT Res = EGParse_ParseFunction( CmdStr , &ParseInfo );
	if( EGPARSE_OKAY != Res )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Invalid command: %s." , EGParse_GetParseResultString( Res ) );
		assert( false );
		return;
	}

	eg_string_crc CrcSystem = eg_string_crc(ParseInfo.SystemName);
	eg_string_crc CrcCmd = eg_string_crc(ParseInfo.FunctionName);

	if( CrcSystem == eg_crc("") )
	{
		switch_crc( CrcCmd )
		{
		case_crc("Welcome"):
		{
			egNetCommId NetId = egNetCommId::RawToId( eg_string(ParseInfo.Parms[0]).ToUInt() );
			eg_string GameId( ParseInfo.Parms[1] );
			eg_lockstep_id LockstepId = eg_lockstep_id::RawToId( eg_string(ParseInfo.Parms[2]).ToUInt() );
			SM_Welcome( NetId , GameId , LockstepId );
		} break;
		case_crc("Ping"):
		{
			m_NetMsgBuffer.AddStrMsg( NET_ID_ALL , EGString_Format("Pong(%u)",eg_string(ParseInfo.Parms[0]).ToUInt()) );
		} break;
		case_crc("Pong" ):
		{
			m_fTimeSinceLastComm = 0;
			eg_uint PingId = eg_string(ParseInfo.Parms[0]).ToUInt();
			if( PingId == m_Ping1RequestId )
			{
				eg_uint64 RecTime = Timer_GetRawTime();
				eg_real RecElapsed = Timer_GetRawTimeElapsedSec( m_Ping1SendTime , RecTime );
				EGLogf( eg_log_t::Client , "Ping %u, %ums" , PingId , eg_uint(RecElapsed*1000));
			}
		} break;
		default:
		{
			assert( false ); //Message not recognized.
		}
		break;
		}
	}
}

void EGClient::SM_Welcome( const egNetCommId& Id , eg_cpstr GameId , const eg_lockstep_id& LockstepId )
{		
	if( !EGString_EqualsI( GameId , Engine_GetGameId() ) )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ " Warning: Tried to connect to a server that was running a different game." );
		EGLogf( eg_log_t::Warning , "Client is \"%s\", server is \"%s\"" , Engine_GetGameId().String() , GameId );
		EGLogf( eg_log_t::Warning , "Disconnecting" );
		Disconnect();
	}
	else if(NET_ID_NOT_CONNECTED != Id )
	{
		EGLogf( eg_log_t::Client , "Welcomed at %u." , Id.Id );
		
		//Put all entities in the unused list.
		if( m_EntWorld )
		{
			m_EntWorld->ClearWorld();
		}

		m_fTimeSinceLastComm = 0;
		m_DbNetId = Id;
		m_LockstepId = LockstepId;
		m_RemoteInputBuffer.Clear();
		m_LocalInputBuffer.Clear();
	}
	else
	{
		EGLogf( eg_log_t::Client , "Connection was rejected." );
		Disconnect();
	}
}

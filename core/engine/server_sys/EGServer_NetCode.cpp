/******************************************************************************
File: Server_NetCode.cpp
Class: EGServer
Purpose: Code specifically for dealing with networking.

(c) 2011 Beem Software
******************************************************************************/

#include "EGServer.h"
#include "EGEngine.h"
#include "EGParse.h"
#include "EGDebugText.h"
#include "EGGame.h"
#include "EGEntWorld.h"

void EGServer::Net_BeginFrame()
{
	assert( m_NetMsgBuffer.IsEmpty() );

	m_NetComm.BeginFrame();
	m_NetComm.ProcessReceivedMsgs( Net_OnReceivedMsg , this );

	Net_EnumerateClients();
}

void EGServer::Net_EnumerateClients()
{
	//Find out what clients we have.
	egNetCommId CurrentClients[MAX_CLIENTS];
	eg_uint NumClients = m_NetComm.GetConnectedClients( CurrentClients , countof(CurrentClients) );

	//We'll mark all clients as not considered.
	for( eg_uint i=0; i<countof(m_Clients); i++ )
	{
		egClientInfo* Client = &m_Clients[i];
		Client->bSwept = false;
	}

	//For each connected client, if it is present, do nothing.
	//If it is a new client, add it.
	for( eg_uint i=0; i<NumClients; i++ )
	{
		egClientInfo* Client = GetClientInfo( CurrentClients[i] );
		if( nullptr == Client )
		{
			ConnectClient( CurrentClients[i] );
		}
		else
		{
			Client->bSwept = true;
		}
	}

	//Finally if a client is not connected, but has a netcomm id then it
	//disconnected.
	for( eg_uint i=0; i<countof(m_Clients); i++ )
	{
		egClientInfo* Client = &m_Clients[i];

		if( NET_ID_NOT_CONNECTED != Client->NetCommId && !Client->bSwept )
		{
			EGLogf( eg_log_t::Server , "Dropped %u." , Client->NetCommId.Id );
			//m_NetComm.DropConnectedClient( Client->NetCommId ); //Don't need to drop because it was dropped on it's own.
			Client->NetCommId = NET_ID_NOT_CONNECTED;
			if( m_EntWorld && m_EntWorld->GetGame() )
			{
				m_EntWorld->OnClientLeaveWorld( Client->LockstepId );
				m_EntWorld->GetGame()->OnClientDisconnected( Client->LockstepId );
			}
		}
	}
}

void EGServer::Net_EndFrame()
{
	Net_BroadcastDirtyEnts();
	m_NetMsgBuffer.PumpMsgs( Net_PumpMsgs , this );
	m_NetComm.EndFrame();
}

void EGServer::Net_BroadcastDirtyEnts()
{
	m_TimeSinceLastEntSend += m_DeltaTime;

	if( m_TimeSinceLastEntSend < m_UpdateEntRate )return;

	//If our input buffer is running low, ask for more inputs:
	//if( m_Clients[0].InCmds.Count() < (PENDING_INPUT_SIZE/2) )
	{
		egNetMsgData ReqLockstep;
		ReqLockstep.Header.NetId = NET_ID_ALL;
		ReqLockstep.Header.nMsg  = eg_net_msg::STOC_LOCKSTEP;
		ReqLockstep.Header.nSize = sizeof(ReqLockstep.SrvLockstep);
		ReqLockstep.SrvLockstep.Frame = m_LockstepFrame;
		ReqLockstep.SrvLockstep.EntUpdateRate = m_UpdateEntRate;
		ReqLockstep.SrvLockstep.InputRate = m_DeltaTime;
		m_NetComm.PostMsg( &ReqLockstep );
		m_LockstepFrame++;
	}

	m_TimeSinceLastEntSend = 0.f;

	if( m_EntWorld )
	{
		m_EntWorld->Net_BroadcastDirtyEnts( m_nGameTime );
	}
}

void EGServer::Net_PumpMsgs( const egNetMsgData* Data , void* UserPointer )
{
	EGServer* _this = static_cast<EGServer*>(UserPointer);
	_this->m_NetComm.PostMsg( Data );
}

void EGServer::Net_OnReceivedMsg( const egNetMsgData* Data , void* UserPointer )
{
	EGServer* _this = static_cast<EGServer*>(UserPointer);
	_this->OnNetMessage( static_cast<eg_net_msg>(Data->Header.nMsg) , *Data );
}

void EGServer::OnNetMessage(eg_net_msg msg, const egNetMsgData& bc)
{
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
	case eg_net_msg::CTOS_LOCKSTEP:CM_Lockstep( bc );break;
	case eg_net_msg::NET_STRCMD: CM_StrCmd( bc ); break;
	case eg_net_msg::NET_REMOTE_EVENT: CM_RemoteEvent( bc ); break;
	default:
		assert(false); //Need to handle this message.
		break;
	}
}

void EGServer::CM_RemoteEvent( const egNetMsgData& bc )
{
	egNetCommId SenderId = bc.Header.NetId;
	egClientInfo* pInfo = GetClientInfo( SenderId );
	if( pInfo )
	{
		egRemoteEvent Event = bc.RemoteEvent.ToEvent();
		OnRemoteEvent( pInfo->LockstepId , Event );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Event sent from invalid client %u." , egNetCommId::IdToIndex(SenderId) );
	}
}

void EGServer::CM_StrCmd( const egNetMsgData& Info )
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

	// Who sent the message?
	egNetCommId SenderId = Info.Header.NetId;
	egClientInfo* Sender = GetClientInfo( SenderId );

	if( nullptr == Sender )
	{
		//Got a message from a client that is no longer connected, ignore it.
		return;
	}

	eg_string_crc CrcCmd = eg_string_crc(ParseInfo.FunctionName);
	switch_crc( CrcCmd )
	{
	case_crc("Ping"):
	{
		m_NetMsgBuffer.AddStrMsg( Info.Header.NetId , EGString_Format( "Pong(%u)" , eg_string(ParseInfo.Parms[0]).ToUInt()) );
	} break;
	case_crc("Pong"):
	{
		Sender->nLastComm = m_nGameTime;
		Sender->bPingedWaiting = false;
	} break;
	default:
	{
		assert( false ); //Unrecognized message.
	} break;
	}
}

void EGServer::CM_Lockstep( const egNetMsgData& bcs )
{		
	egNetCommId SenderId = bcs.Header.NetId;
	egClientInfo* pInfo = GetClientInfo( SenderId );

	if(!pInfo)
	{
		return;
	}

	pInfo->nLastComm = m_nGameTime;
	
	//Because the server may be running behind the client
	//the input is catenated, so all input posted to the server
	//will be accounted for (that way the mouse movement isn't too
	//small, and pressed commands aren't skipped).
	if( pInfo->InCmds.IsFull() )
	{
		pInfo->ExcessInputs++;
		pInfo->InCmds[PENDING_INPUT_SIZE-1].CatenateNextFrame( bcs.CliLockstep.Cmds );
	}
	else
	{
		pInfo->InCmds.InsertLast( bcs.CliLockstep.Cmds );
	}

	pInfo->vRayOrg.x = bcs.CliLockstep.RayTrace.Org[0];
	pInfo->vRayOrg.y = bcs.CliLockstep.RayTrace.Org[1];
	pInfo->vRayOrg.z = bcs.CliLockstep.RayTrace.Org[2];

	pInfo->vRayDir.x = bcs.CliLockstep.RayTrace.Dir[0];
	pInfo->vRayDir.y = bcs.CliLockstep.RayTrace.Dir[1];
	pInfo->vRayDir.z = bcs.CliLockstep.RayTrace.Dir[2];

	pInfo->nLastComm = m_nGameTime;
}

void EGServer::ConnectClient( const egNetCommId& NetId )
{
	//Find the first client item that is not used.
	eg_bool bFoundSpot = false;
	eg_uint nSpot = countof(m_Clients);
	for(eg_uint i=0; i<countof(m_Clients) && !bFoundSpot; i++)
	{
		bFoundSpot = !m_Clients[i].IsConnected();
		if(bFoundSpot)
		{
			nSpot=i;
		}		
	}

	if(bFoundSpot)
	{
		egClientInfo* pInfo = &m_Clients[nSpot];
		pInfo->Clear();
		pInfo->NetCommId = NetId;
		pInfo->LockstepId = eg_lockstep_id::IndexToId(nSpot);
		pInfo->InCmds.Clear();
		//Create a player character for the client:
		pInfo->bPingedWaiting = false;
		pInfo->bSwept = true; //So that it doesn't immediately get dropped.
		pInfo->nLastComm = m_nGameTime;
		pInfo->FramesWithoutInput = 0;
		pInfo->ExcessInputs = 0;

		::EGLogf( eg_log_t::Server , ("Client %i connected, sending welcome."), NetId.Id);
		eg_string WelcomeMsg = EGString_Format( "Welcome(%u,\"%s\",%u)" , NetId.Id , Engine_GetGameId().String() , eg_lockstep_id::IdToRaw( pInfo->LockstepId ) );
		m_NetMsgBuffer.AddStrMsg( NetId , WelcomeMsg );

		if( m_EntWorld )
		{
			m_EntWorld->FullyReplicateToClient( NetId );

			if( m_EntWorld->GetGame() )
			{
				m_EntWorld->GetGame()->OnClientConnected( pInfo->LockstepId );
			}
			m_EntWorld->OnClientEnterWorld( pInfo->LockstepId );
		}
	}
	else
	{
		EGLogf( eg_log_t::Server , "Couldn't connect client %u at this time." , NetId.Id );
	}
}

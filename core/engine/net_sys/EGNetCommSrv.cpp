// EGNetCommSrv
// (c) 2015 Beem Software

#include "EGNetCommSrv.h"
#include "EGNetLocal.h"
#include "EGNetOnline.h"
#include "EGNetMsgs.h"
#include "EGEngineConfig.h"
#include "EGNetMsgBuffer.h"
#include "EGGlobalConfig.h"

struct egClientData
{
	egNetCommId    NetId;
	INetSocket*    Sock;
	EGNetMsgBuffer Rec;
	EGNetMsgBuffer Send;

	eg_bool IsConnected()const{ return NET_ID_NOT_CONNECTED != NetId; }
};

EG_ALIGN struct EGNetCommSrv::egData
{
	egNetCommId  NextNetId;
	INetSocket*  Listener;
	egClientData Clients[MAX_CLIENTS];
};

EGNetCommSrv::EGNetCommSrv()
: EGNetCommBase()
{
	static_assert( sizeof(egData) <= sizeof(m_DMem) , "Increase the size of m_DMem." );
	zero( &m_DMem );
	m_D = new (m_DMem) egData;

	for( eg_uint i=0; i<countof(m_D->Clients); i++ )
	{
		assert( !m_D->Clients[i].IsConnected() ); // Was this was zero'd out correcly?
		m_D->Clients[i].NetId = NET_ID_NOT_CONNECTED;
	}

	m_D->Listener = nullptr;

	m_D->NextNetId.Id = MAX_LOCAL_CLIENTS + 1; //Id's 1 to 4 are specifically for local clients.
}

EGNetCommSrv::~EGNetCommSrv()
{
	m_D->~egData();
	m_D = nullptr;
}

void EGNetCommSrv::Listen( eg_bool On )
{
	if( On && nullptr == m_D->Listener )
	{
		m_D->Listener = NetOnline_CreateListener( GlobalConfig_ServerPort.GetValueThreadSafe() );
	}
	else if( !On && nullptr != m_D->Listener )
	{
		NetOnline_DestroyListener( m_D->Listener );
		m_D->Listener = nullptr;
	}
}

void EGNetCommSrv::BeginHosting()
{
	Init();
	NetOnline_Server_Init( GlobalConfig_ServerPort.GetValueThreadSafe() );
	Listen( true );
	NetLocal_Server_BeginHosting();
}

void EGNetCommSrv::EndHosting()
{
	NetLocal_Server_EndHosting();
	Listen( false );

	for( eg_uint i=0; i<countof(m_D->Clients); i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( !Client->IsConnected() )continue;
		
		DropConnectedClient( Client->NetId );
	}
	NetOnline_Server_Deinit();
	Deinit();
}

void EGNetCommSrv::BeginFrame()
{
	EGNetCommBase::BeginFrame();

	NetOnline_Server_Update();
	//Check for new clients:
	INetSocket* NewClientSock = m_D->Listener ? m_D->Listener->ListenerScanForClient() : nullptr;
	if( nullptr != NewClientSock )
	{
		//Found a new client!
		eg_string Addr = NewClientSock->GetRemoteAddr();
		EGLogf( eg_log_t::NetCommS , "Got connection from: %s." , Addr.String() );


		egClientData* NewClient = nullptr;
		for( eg_uint i=0; i<countof(m_D->Clients) && nullptr == NewClient; i++ )
		{
			if( !m_D->Clients[i].IsConnected() )
			{
				NewClient = &m_D->Clients[i];
			}
		}

		if( nullptr != NewClient )
		{
			NewClient->Sock = NewClientSock;
			NewClient->NetId = m_D->NextNetId;

			assert( NewClient->Rec.IsEmpty() );
			assert( NewClient->Send.IsEmpty() );

			NetOnline_AddNetObj( NewClient->Sock , NewClient->NetId , NewClient->Rec , NewClient->Send );

			m_D->NextNetId.Id++; //Technically we should check to see tha the next net id isn't being used and that it isn't a local client, but that would take 4 billion connections, which ain't going to happen in the life of this program.
		}
		else
		{
			EGLogf( eg_log_t::NetCommS , "Rejected %s, no slot for it." , Addr.String() );
			NetOnline_Disconnect( NewClientSock ); //Eventually the client will realize it isn't reading from this socket and disconnect.
		}
	}

	//Drop unresponsive clients:
	for( eg_uint i=0; i<countof(m_D->Clients) ; i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( !Client->IsConnected() )continue;

		if( Client->Sock->HadError() )
		{
			eg_string StrAddr = Client->Sock->GetRemoteAddr();
			EGLogf( eg_log_t::NetCommS , "Lost connection to %s, dropping it." , StrAddr.String() );
			DropConnectedClient( Client->NetId );
		}
	}
}

void EGNetCommSrv::EndFrame()
{
	//Handle locally received messages:
	NetLocal_Server_HandleReceivedMsgs( HandleRecMsg , this );
	//Handle messages received from the network:
	for( eg_uint i=0; i<countof(m_D->Clients); i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( !Client->IsConnected() )continue;
		
		Client->Rec.PumpMsgs( HandleRecMsg , this );
	}
	EGNetCommBase::EndFrame();
}

eg_uint EGNetCommSrv::GetConnectedClients( egNetCommId* Out , eg_uint SizeOut )const
{
	eg_uint Count = 0;
	for( eg_uint i=0; i<MAX_LOCAL_CLIENTS; i++ )
	{
		if( NetLocal_IsClientConnected( egNetLocalId::IndexToId(i) ) && Count < SizeOut )
		{
			Out[Count] = egNetCommId::IndexToId(i);
			Count++;
		}
	}

	for( eg_uint i=0; i<countof(m_D->Clients); i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( Client->IsConnected() && Count < SizeOut )
		{
			Out[Count] = Client->NetId;
			Count++;
		}
	}

	return Count;
}

eg_bool EGNetCommSrv::HasOnlineClients()const
{
	eg_bool HasOnline = false;

	for( eg_uint i=0; i<countof(m_D->Clients) && !HasOnline ; i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( Client->IsConnected() )
		{
			HasOnline = true;
		}
	}

	return HasOnline;
}

void EGNetCommSrv::DropConnectedClient( const egNetCommId& Id )
{
	if( 0 <= egNetCommId::IdToIndex(Id) && egNetCommId::IdToIndex(Id) < MAX_LOCAL_CLIENTS )
	{
		//Can't drop yet...
		unused( Id );
		EGLogf( eg_log_t::NetCommS , "Attempted to drop local client %u, feature not implemented." , Id.Id );
	}
	else
	{
		for( eg_uint i=0; i<countof(m_D->Clients); i++ )
		{
			egClientData* Client = &m_D->Clients[i];
			if( Client->IsConnected() && Client->NetId == Id )
			{
				NetOnline_RemoveNetObj( Client->Sock );
				NetOnline_Disconnect( Client->Sock ); //Eventually the client will realize it was dropped, but maybe we should broadcast one last message.
				Client->Rec.FlushMsgs();
				Client->Send.FlushMsgs();
				Client->NetId = NET_ID_NOT_CONNECTED;
			}
		}
	}
}

void EGNetCommSrv::ProcessOutgoingMsg( const struct egNetMsgData* Data )
{
	//Broadcast to local clients:
	NetLocal_Server_PostMsg( Data );
	//Broadcast to network clients:
	for( eg_uint i=0; i<countof(m_D->Clients); i++ )
	{
		egClientData* Client = &m_D->Clients[i];
		if( !Client->IsConnected() )continue;

		if( NET_ID_ALL == Data->Header.NetId || Client->NetId == Data->Header.NetId )
		{
			Client->Send.AddMsg( Data );
		}
	}
}

void EGNetCommSrv::HandleRecMsg( const struct egNetMsgData* Data , void* UserPointer )
{
	EGNetCommSrv* _this = static_cast<EGNetCommSrv*>(UserPointer);
	_this->PostRecMsg( Data );
}
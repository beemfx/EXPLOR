// (c) 2015 Beem Media. All Rights Reserved.

#include "EGNetOnline.h"
#if WITH_NET_COMS
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "EGEngineConfig.h"
#include "EGNetMsgs.h"
#include "EGNetMsgBuffer.h"
#include "EGStringMap.h"

//
// Net Socket Interface
//

enum NET_SOCKET_T
{
	NET_SOCKET_UNK,
	NET_SOCKET_CLIENT,
	NET_SOCKET_SERVER,
	NET_SOCKET_LISTENER,
	NET_SOCKET_SERVER_CLIENT,
};

class EGNetSocketUDP: public INetSocket
{
friend class EGNetOnlineServer;
private:
	SOCKET        m_Socket;
	sockaddr_in   m_AddrMe;
	sockaddr_in   m_AddrTo;
	eg_string_crc m_AddrMeHash;
	NET_SOCKET_T  m_Type:8;
	eg_bool       m_NotAcceptedYet:1;
	mutable eg_bool       m_HadError:1;
public:
	EGNetSocketUDP()
	: m_AddrMeHash(CT_Clear)
	, m_Type(NET_SOCKET_UNK)
	, m_NotAcceptedYet(false)
	, m_HadError(false)
	{
		zero( &m_Socket );
		zero( &m_AddrMe );
		zero( &m_AddrTo);
	}

	void Connect( eg_cpstr StrAddr );
	void Disconnect();

	// INetSocket Interface
	virtual eg_bool     HadError()const override;
	virtual void        SetError() override;
	virtual eg_string   GetRemoteAddr()const override;
	virtual eg_string   GetMyAddr()const override;
	virtual eg_bool     IsReadyToRead() const override;
	virtual eg_size_t   Read( egNetMsgData* DataIn ) override;
	virtual void        Write( const egNetMsgData* DataOut ) override;
	virtual INetSocket* ListenerScanForClient()const override;

public:
	static EGNetSocketUDP* CreateSocketBoundTo( eg_uint16 Port );
};

static class EGNetOnlineServer
{
private:
	struct egClientData
	{
		EGNetMsgBuffer RecvBuffer;
		EGNetSocketUDP Socket;

		egClientData()
		{

		}
	};

	typedef EGStringCrcMapFixedSize<egClientData*,MAX_CLIENTS> EGClientMap;
private:
	EGNetMsgBuffer  m_SendBuffer;
	EGNetMsgBuffer  m_RecvBuffer;
	egClientData    m_ClientData[MAX_CLIENTS];
	EGClientMap     m_ClientMap;
	EGNetSocketUDP* m_Socket;
	EGNetSocketUDP  m_ListenerDummy;
	eg_bool         m_IsListening:1;
public:
	EGNetOnlineServer()
	: m_Socket( nullptr )
	, m_IsListening( false )
	, m_ClientMap( CT_Clear )
	{
		m_ListenerDummy.m_Type = NET_SOCKET_LISTENER;
	}

	void Init( eg_uint Port )
	{
		assert( nullptr == m_Socket );
		m_Socket = EGNetSocketUDP::CreateSocketBoundTo( static_cast<eg_uint16>(Port) );
		if( nullptr != m_Socket )
		{
			m_Socket->m_Type = NET_SOCKET_SERVER;
			NetOnline_AddNetObj( m_Socket , egNetCommId::IndexToId(0) , m_RecvBuffer , m_SendBuffer );
		}
	}

	void Deinit()
	{
		assert( nullptr != m_Socket );
		if( m_Socket )
		{
			NetOnline_RemoveNetObj( m_Socket );
			NetOnline_Disconnect( m_Socket );
			m_Socket = nullptr;
			m_SendBuffer.FlushMsgs();
			m_RecvBuffer.FlushMsgs();
		}
	}

	void Update()
	{
		//All received messages should be dispatched.
		assert( m_SendBuffer.IsEmpty() ); //The server shouldn't be sending messages, the individual sockets should do that.
		m_RecvBuffer.PumpMsgs( ProcessIncomingMsgs , this );
	}

	static void ProcessIncomingMsgs( const egNetMsgData* Msg , void* UserData )
	{
		EGNetOnlineServer* _this = static_cast<EGNetOnlineServer*>(UserData);
		static_assert( sizeof(Msg->Footer.FromAddr) >= sizeof(_this->m_Socket->m_AddrMe) , "Can't fit address in NetId" );

		eg_string_crc AddrHash = eg_string_crc::HashData( &Msg->Footer.FromAddr , sizeof(Msg->Footer.FromAddr) );
		
		if( Msg->Header.nMsg == eg_net_msg::CTOS_UDP_CONNECT ) //A request connection is handled here.
		{
			if( !_this->m_IsListening )
			{
				EGLogf( eg_log_t::NetOnlineUDP , "NetOnline: A client attempted to connect but the server is not accepting connections." );
			}
			else
			{
				assert( Msg->Header.nSize == sizeof(Msg->UdpConnect) );
				//Find a spot for this client:
				egClientData* NewSocket = nullptr;
				for( eg_uint i=0; i<countof(_this->m_ClientData) && nullptr == NewSocket; i++ )
				{
					egClientData* Client = &_this->m_ClientData[i];
					if( Client->Socket.m_Type == NET_SOCKET_UNK )
					{
						NewSocket = Client;
					}
				}

				if( nullptr == NewSocket )
				{
					EGLogf( eg_log_t::NetOnlineUDP , "NetOnline: A client attempted to connect but there were no available slots." );
				}
				else
				{
					_this->m_ClientMap.Insert( AddrHash , NewSocket );
					NewSocket->Socket = *_this->m_Socket;
					NewSocket->Socket.m_Type = NET_SOCKET_SERVER_CLIENT;
					NewSocket->Socket.m_AddrMeHash = AddrHash;
					NewSocket->Socket.m_NotAcceptedYet = true; //This flag is set until the netcomm server receives the client.
					EGMem_Copy( &NewSocket->Socket.m_AddrTo , &Msg->Footer.FromAddr , sizeof(NewSocket->Socket.m_AddrTo) );
					EGLogf( eg_log_t::NetOnlineUDP , "NetOline: Client requested connection from %s." , NewSocket->Socket.GetRemoteAddr().String() );
				}
			}
		}
		else //Any messages besides UDP connect should be directed to the correct message buffer.
		{
			egClientData* ClientData = _this->m_ClientMap.Get( AddrHash );
			if( ClientData )
			{
				ClientData->RecvBuffer.AddMsg( Msg );
			}
			else
			{
				assert( false );
				EGLogf( eg_log_t::NetOnlineUDP , "NetOnline: A messages was sent to a non-connected client that was not a connection request." );
			}
		}
	}

	INetSocket* CreateListener()
	{
		m_IsListening = true;
		return &m_ListenerDummy;
	}

	void DestroyListener( INetSocket* Listener )
	{
		unused( Listener );
		assert( Listener == &m_ListenerDummy ); //WTF?
		m_IsListening = false;
	}

	INetSocket* ListenerScanForClient( const EGNetSocketUDP* Listener )
	{
		unused( Listener );
		assert( Listener == &m_ListenerDummy );

		//Just go through the list and see if there are any connected clients that are still waiting to be accepted.
		egClientData* NewSocket = nullptr;
		for( eg_uint i=0; i<countof(m_ClientData) && nullptr == NewSocket; i++ )
		{
			egClientData* Client = &m_ClientData[i];
			if( Client->Socket.m_Type == NET_SOCKET_SERVER_CLIENT && Client->Socket.m_NotAcceptedYet )
			{
				NewSocket = Client;
			}
		}

		INetSocket* Out = nullptr;

		if( nullptr != NewSocket )
		{
			Out = &NewSocket->Socket;
			assert( NewSocket->RecvBuffer.IsEmpty() );
			NewSocket->Socket.m_NotAcceptedYet = false;
			EGLogf( eg_log_t::NetOnlineUDP , "NetOnline: Server obtained data for client at %s." , NewSocket->Socket.GetRemoteAddr().String() );
		}

		return Out;
	}

	void CloseClientSocket( EGNetSocketUDP* Socket )
	{
		eg_int Index = -1;
		for( eg_uint i=0; i<countof(m_ClientData) && -1 == Index; i++ )
		{
			if( Socket == &m_ClientData[i].Socket )
			{
				Index = i;
			}
		}
		assert( -1 != Index ); //This wasn't actually a connection to a client?
		if( -1 != Index )
		{
			m_ClientData[Index].RecvBuffer.FlushMsgs();
			m_ClientMap.Delete( m_ClientData[Index].Socket.m_AddrMeHash );
		}
		Socket->m_Type = NET_SOCKET_UNK;
		Socket->m_NotAcceptedYet = false; //Should already be false, but just in case.
	}

	eg_bool IsClientReadyToRead( const EGNetSocketUDP* Socket )
	{
		egClientData* ClientData = m_ClientMap.Get( Socket->m_AddrMeHash );
		if( nullptr == ClientData )return false;

		return !ClientData->RecvBuffer.IsEmpty();
	}

	eg_size_t Read( EGNetSocketUDP* Socket , egNetMsgData* DataIn )
	{
		egClientData* ClientData = m_ClientMap.Get( Socket->m_AddrMeHash );
		if( nullptr == ClientData )return 0;

		eg_size_t SizeRead = 0;
		if( !ClientData->RecvBuffer.IsEmpty() )
		{
			ClientData->RecvBuffer.GetAndRemoveFirstItem( DataIn );
			SizeRead = sizeof(DataIn->Header) + DataIn->Header.nSize;
		}

		return SizeRead;
	}

} NetOnline_Server;

EGNetSocketUDP* EGNetSocketUDP::CreateSocketBoundTo( eg_uint16 Port )
{
	EGNetSocketUDP* Out = new ( eg_mem_pool::Default ) EGNetSocketUDP;
	if( nullptr == Out )return nullptr;

	Out->m_Socket = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP );

	if( INVALID_SOCKET == Out->m_Socket )
	{
		NetOnline_PrintWinSockError( __FUNCTION__"(socket)" , WSAGetLastError() );
		delete Out;
		return nullptr;
	}

	int nRes = 0;
	
	//Make the socket non-blocking.
	if( SOCKET_ERROR != nRes )
	{
		u_long nonBlocking = 1;
		nRes = ioctlsocket( Out->m_Socket , FIONBIO , &nonBlocking );
	}

	//Bind the socket to the specified port, note that if 0 is passed it in
	//will be assigned a port.
	if( SOCKET_ERROR != nRes )
	{
		Out->m_AddrMe.sin_family = AF_INET;
		Out->m_AddrMe.sin_port   = htons(Port);
		Out->m_AddrMe.sin_addr.s_addr = htonl(INADDR_ANY);
		nRes = bind( Out->m_Socket , reinterpret_cast<const sockaddr*>(&Out->m_AddrMe) , sizeof(Out->m_AddrMe) );
	}

	//Get the sockename, this is because 0 will bind to a port so we want to
	//find out which port the socket was actually bound to.
	if( SOCKET_ERROR != nRes )
	{
		int Size = sizeof(Out->m_AddrMe);
		nRes = getsockname( Out->m_Socket , reinterpret_cast<sockaddr*>(&Out->m_AddrMe) , &Size );
	}

	if( SOCKET_ERROR == nRes )
	{
		NetOnline_PrintWinSockError( __FUNCTION__"(ioctlsocket/bind/getsockname)" , WSAGetLastError() );
		shutdown( Out->m_Socket , SD_BOTH );
		closesocket( Out->m_Socket );
		delete Out;
		return nullptr;
	}

	return Out;
}

static void NetOnline_StrAddrToN( eg_cpstr Addr , sockaddr_in* Out )
{
	//Connect comes in the form "x.x.x.x:port"
	eg_string sAddr;
	eg_string sPort;

	eg_cpstr sPtrSrvAddr = Addr;
	eg_bool OnAddress = true;
	while(*sPtrSrvAddr != 0)
	{
		if(':' == *sPtrSrvAddr)
		{
			OnAddress = false;
		}
		else
		{
			if(OnAddress)
			{
				sAddr += *sPtrSrvAddr;
			}
			else
			{
				sPort += *sPtrSrvAddr;
			}
		}
		sPtrSrvAddr++;
	}

	eg_uint16 Port = static_cast<eg_uint16>(sPort.ToUInt());

	eg_char8 sAddr8[64];
	sAddr.CopyTo(sAddr8, countof(sAddr8));
	Out->sin_family = AF_INET;
	InetPtonA( AF_INET , sAddr8 , &Out->sin_addr.s_addr );
	Out->sin_port = htons( Port );
}

void NetOnline_Server_InitUDP( eg_uint Port )
{
	NetOnline_Server.Init( Port );	
}

void NetOnline_Server_DeinitUDP()
{
	NetOnline_Server.Deinit();
}

void NetOnline_Server_UpdateUDP()
{
	NetOnline_Server.Update();
}

void EGNetSocketUDP::Connect( eg_cpstr StrAddr )
{
	m_Type = NET_SOCKET_CLIENT;
	EGLogf( eg_log_t::NetOnlineUDP , "NetOnline created socket at %s." , GetMyAddr().String() );
	//We want to target the host machine:
	NetOnline_StrAddrToN( StrAddr , &m_AddrTo );		

	//Send a message to the listener.
	egNetMsgData Msg;
	Msg.Header.NetId = NET_ID_NOT_CONNECTED;
	Msg.Header.nMsg  = eg_net_msg::CTOS_UDP_CONNECT;
	Msg.Header.nSize = sizeof(Msg.UdpConnect);
	Msg.UdpConnect.Port = ntohs(m_AddrMe.sin_port);
	int nRes = sendto( m_Socket , reinterpret_cast<const char*>(&Msg) , static_cast<int>(sizeof(Msg.Header)+Msg.Header.nSize) , 0 , reinterpret_cast<const sockaddr*>(&m_AddrTo) , sizeof(m_AddrTo));
	if( SOCKET_ERROR == nRes )
	{
		m_HadError = true;
	}
}

void EGNetSocketUDP::Disconnect()
{
switch( m_Type )
	{
		case NET_SOCKET_SERVER_CLIENT:
			NetOnline_Server.CloseClientSocket( this );
			break;
		case NET_SOCKET_SERVER:
		case NET_SOCKET_CLIENT:
		{
			int Res = 0;
			Res = shutdown( m_Socket , SD_BOTH );
			if( SOCKET_ERROR == Res )
			{
				NetOnline_PrintWinSockError( "shutdown" , WSAGetLastError() );
			}
	
			Res = closesocket( m_Socket );
			if( SOCKET_ERROR == Res )
			{
				NetOnline_PrintWinSockError( "closesocket" , WSAGetLastError() );
			}

			delete this;
		} break;
		default:
			assert( false ); //Disconnect should not be called on this type of socket.
			break;
	}
}

INetSocket* EGNetSocketUDP::ListenerScanForClient()const
{
	return NetOnline_Server.ListenerScanForClient( static_cast<const EGNetSocketUDP*>(this) );
}

eg_bool EGNetSocketUDP::HadError()const
{
	return nullptr == this || m_HadError;
}

eg_string EGNetSocketUDP::GetRemoteAddr()const
{
	eg_string StrOut;
	if( nullptr == this ){ StrOut = ""; return StrOut; }
	eg_char Temp[1024];
	InetNtopA( m_AddrTo.sin_family , const_cast<ULONG*>(&m_AddrTo.sin_addr.s_addr) , Temp , countof(Temp) );
	StrOut = Temp;
	StrOut.Append( EGString_Format( ":%u" , static_cast<eg_uint>(ntohs(m_AddrTo.sin_port)))  );
	return StrOut;
}

eg_string EGNetSocketUDP::GetMyAddr()const
{
	eg_string StrOut;
	if( nullptr == this ){ StrOut = ""; return StrOut; }
	eg_char Temp[1024];
	InetNtopA( m_AddrMe.sin_family , const_cast<ULONG*>(&m_AddrMe.sin_addr.s_addr) , Temp , countof(Temp) );
	StrOut = Temp;
	StrOut.Append( EGString_Format( ":%u" , static_cast<eg_uint>(ntohs(m_AddrMe.sin_port)))  );
	return StrOut;
}

eg_bool EGNetSocketUDP::IsReadyToRead() const
{
	if( m_HadError )return false;

	eg_bool Ready = false;

	switch( m_Type )
	{
		case NET_SOCKET_CLIENT:
		case NET_SOCKET_SERVER:
		{
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(m_Socket, &readSet);
			timeval timeout;
			timeout.tv_sec = 0;  // Zero timeout (poll)
			timeout.tv_usec = 0;
			int nRes = select( 0 , &readSet , nullptr , nullptr , &timeout );

			if( SOCKET_ERROR == nRes )
			{
				NetOnline_PrintWinSockError( "select" , WSAGetLastError() );
				m_HadError = true;
			}
			Ready = nRes > 0;
		}
		break;
		case NET_SOCKET_SERVER_CLIENT:
		{
			Ready = NetOnline_Server.IsClientReadyToRead( this );
		}
		break;
		default: assert( false ); break;
	}

	return Ready;
}

eg_size_t EGNetSocketUDP::Read( egNetMsgData* DataIn )
{
	if( nullptr == this )return 0;
	if( m_HadError )return 0;

	eg_size_t OutSizeRead = 0;

	switch( m_Type )
	{
		case NET_SOCKET_CLIENT:
		case NET_SOCKET_SERVER:
		{
			sockaddr_in FromAddr;
			int FromAddrSize = sizeof(FromAddr);

			int Res = recvfrom( m_Socket , reinterpret_cast<char*>(DataIn) , static_cast<int>(sizeof(*DataIn)) , 0 , reinterpret_cast<sockaddr*>(&FromAddr) , &FromAddrSize );

			if( SOCKET_ERROR == Res && WSAEWOULDBLOCK != WSAGetLastError() )
			{
				NetOnline_PrintWinSockError( "recv" , WSAGetLastError() );
				m_HadError = true;
				Res = 0;
			}
			else
			{
				static_assert( sizeof(DataIn->Footer.FromAddr) == sizeof(FromAddr) , "egNet_FromAddr not big enough." );
				assert( FromAddrSize == sizeof(DataIn->Footer.FromAddr) );
				EGMem_Copy( &DataIn->Footer.FromAddr , &FromAddr , sizeof(DataIn->Footer.FromAddr) );
				if( SOCKET_ERROR == Res )
				{
					Res = 0;
				}
			}
			OutSizeRead = static_cast<eg_size_t>(Res);
		} break;
		case NET_SOCKET_SERVER_CLIENT:
		{
			OutSizeRead = NetOnline_Server.Read( this , DataIn );
		} break;
		default: assert( false ); break;
	}

	return OutSizeRead;
}

void EGNetSocketUDP::Write( const egNetMsgData* DataOut )
{
	if( nullptr == this )return;
	if( m_HadError )return;

	eg_size_t DataSize = sizeof(DataOut->Header) + DataOut->Header.nSize;

	int Res = sendto( m_Socket , reinterpret_cast<const char*>(DataOut) , static_cast<int>(DataSize) , 0 , reinterpret_cast<const sockaddr*>(&m_AddrTo) , sizeof(m_AddrTo));
	if( SOCKET_ERROR == Res )
	{
		NetOnline_PrintWinSockError( "send" , WSAGetLastError() );
		m_HadError = true;
	}
}

void EGNetSocketUDP::SetError()
{
	m_HadError = true;
}

INetSocket* NetOnline_ConnectUDP( eg_cpstr Addr )
{
	EGNetSocketUDP* SocketOut = EGNetSocketUDP::CreateSocketBoundTo( 0 );

	if( SocketOut )
	{
		SocketOut->Connect( Addr );
	}

	return SocketOut;
}

void NetOnline_DisconnectUDP( INetSocket* Socket )
{
	if( nullptr == Socket )return;
	EGNetSocketUDP* SocketUDP = static_cast<EGNetSocketUDP*>(Socket);
	SocketUDP->Disconnect();
}

INetSocket* NetOnline_CreateListenerUDP( eg_uint Port )
{
	unused( Port );
	return NetOnline_Server.CreateListener();
}

void NetOnline_DestroyListenerUDP( INetSocket* Socket )
{
	NetOnline_Server.DestroyListener( Socket );
}
#endif

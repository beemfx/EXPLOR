// NetOnline - An interface that handles internet traffic.
// (c) 2015 Beem Software
#include "EGNetOnline.h"
#if WITH_NET_COMS
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "EGEngineConfig.h"
#include "EGNetThread.h"

void NetOnline_Server_InitUDP( eg_uint Port );
void NetOnline_Server_DeinitUDP();
void NetOnline_Server_UpdateUDP();
INetSocket* NetOnline_ConnectUDP( eg_cpstr Addr );
void NetOnline_DisconnectUDP( INetSocket* Socket );
INetSocket* NetOnline_CreateListenerUDP( eg_uint Port );
void NetOnline_DestroyListenerUDP( INetSocket* Socket );

void NetOnline_Server_InitTCP( eg_uint Port );
void NetOnline_Server_DeinitTCP();
void NetOnline_Server_UpdateTCP();
INetSocket* NetOnline_ConnectTCP( eg_cpstr Addr );
void NetOnline_DisconnectTCP( INetSocket* Socket );
INetSocket* NetOnline_CreateListenerTCP( eg_uint Port );
void NetOnline_DestroyListenerTCP( INetSocket* Socket );

void NetOnline_PrintWinSockError( eg_cpstr s , eg_int nCode )
{
	//assert(false);
	eg_char Buffer[1024];
	eg_uint nSize = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, nCode, 0, Buffer, countof(Buffer), nullptr);
	if(nSize > 2)
	{
		if(Buffer[nSize-2] == '\r')Buffer[nSize-2]=0;
	}
	EGLogf( eg_log_t::Error , __FUNCTION__ "WinSock Error (%s) %u: %s" , s , nCode , Buffer );
}

static class EGNetOnline
{
private:
	enum COMM_T
	{
		COMM_UDP,
		COMM_TCP,
	};
private:
	EGNetThread m_NetThread;
	WSAData m_WsData;
	COMM_T  m_CommType:7;
	eg_bool m_IsInited:1;
public:
	EGNetOnline()
	: m_IsInited(false)
	, m_NetThread( "NetThread" )
	, m_CommType( COMM_UDP )
	{
		zero( &m_WsData );
	}

	void Init()
	{
		assert( !m_IsInited );
		EGLogf( eg_log_t::NetOnline , "Initializing WinSock..." );

		eg_int nRes = 0;
		const eg_uint16 WS_VER = MAKEWORD(2,2);

		zero(&m_WsData);
		nRes = WSAStartup(WS_VER, &m_WsData);
		if(0 == nRes && WS_VER == m_WsData.wVersion)
		{
			eg_uint VersionHigh = (m_WsData.wVersion&0xFF);
			eg_uint VersionLow  = (m_WsData.wVersion&0xFF00)>>8;
			{
				eg_string StrDesc, StrStatus;
				StrDesc = m_WsData.szDescription;
				StrStatus = m_WsData.szSystemStatus;
				EGLogf(eg_log_t::NetOnline , "   Desc: %s, version %u.%u", StrDesc.String(), VersionHigh, VersionLow);
				EGLogf(eg_log_t::NetOnline , "   Status: %s", StrStatus.String());
			}

			m_IsInited = true;
		}
		else
		{
			NetOnline_PrintWinSockError( "WSAStartup", nRes );
		}

		m_NetThread.Start();
	}

	void Deinit()
	{
		if( !m_IsInited )return;

		m_NetThread.Stop();

		WSACleanup();
	}

	void AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer )
	{
		m_NetThread.AddNetObj( Sock , Id , RecBuffer , SendBuffer );
	}

	void RemoveNetObj( INetSocket* Sock  )
	{
		m_NetThread.RemoveNetObj( Sock );
	}

	void Server_Init( eg_uint Port )
	{
		switch( m_CommType )
		{
			case COMM_UDP:
				NetOnline_Server_InitUDP( Port );
				break;
			case COMM_TCP:
				NetOnline_Server_InitTCP( Port );
				break;
			default:
				assert( false );
				break;
		}
	}

	void Server_Deinit()
	{
		switch( m_CommType )
		{
			case COMM_UDP:
				NetOnline_Server_DeinitUDP();
				break;
			case COMM_TCP:
				NetOnline_Server_DeinitTCP();
				break;
			default:
				assert( false );
				break;
		}
	}

	void Server_Update()
	{
		switch( m_CommType )
		{
			case COMM_UDP:
				NetOnline_Server_UpdateUDP();
				break;
			case COMM_TCP:
				NetOnline_Server_UpdateTCP();
				break;
			default:
				assert( false );
				break;
		}
	}

	INetSocket* Connect( eg_cpstr Addr )
	{
		switch( m_CommType )
		{
			case COMM_UDP: return NetOnline_ConnectUDP( Addr );
			case COMM_TCP: return NetOnline_ConnectTCP( Addr );
		}
		assert( false );
		return nullptr;
	}

	void Disconnect( INetSocket* Socket )
	{
		switch( m_CommType )
		{
			case COMM_UDP:
				NetOnline_DisconnectUDP( Socket );
				break;
			case COMM_TCP:
				NetOnline_DisconnectTCP( Socket );
				break;
		}
	}


	INetSocket* CreateListener( eg_uint Port )
	{
		switch( m_CommType )
		{
			case COMM_UDP: return NetOnline_CreateListenerUDP( Port );
			case COMM_TCP: return NetOnline_CreateListenerTCP( Port );
		}
		assert( false );
		return nullptr;
	}

	void DestroyListener( INetSocket* Socket )
	{
		switch( m_CommType )
		{
			case COMM_UDP:
				NetOnline_DestroyListenerUDP( Socket );
				break;
			case COMM_TCP:
				NetOnline_DestroyListenerTCP( Socket );
				break;
		}
	}

} NetOnline;

void NetOnline_Init()
{
	NetOnline.Init();
}

void NetOnline_Deinit()
{
	NetOnline.Deinit();
}

void NetOnline_AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer )
{
	NetOnline.AddNetObj( Sock , Id , RecBuffer , SendBuffer );
}

void NetOnline_RemoveNetObj( INetSocket* Sock )
{
	NetOnline.RemoveNetObj( Sock );
}

void NetOnline_Server_Init( eg_uint Port )
{
	NetOnline.Server_Init( Port );
}

void NetOnline_Server_Deinit()
{
	NetOnline.Server_Deinit();
}

void NetOnline_Server_Update()
{
	NetOnline.Server_Update();
}

INetSocket* NetOnline_Connect( eg_cpstr Addr )
{
	return NetOnline.Connect( Addr );
}

void NetOnline_Disconnect( INetSocket* Socket )
{
	return NetOnline.Disconnect( Socket );
}


INetSocket* NetOnline_CreateListener( eg_uint Port )
{
	return NetOnline.CreateListener( Port );
}

void NetOnline_DestroyListener( INetSocket* Socket )
{
	return NetOnline.DestroyListener( Socket );
}

#endif

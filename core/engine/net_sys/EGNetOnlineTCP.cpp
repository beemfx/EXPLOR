// (c) 2015 Beem Media. All Rights Reserved.

#include "EGNetOnline.h"
#if WITH_NET_COMS
#include "EGEngineConfig.h"
#include "EGNetMsgs.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>

class EGNetSocketTCP : public INetSocket
{
friend INetSocket* NetOnline_ConnectTCP( eg_cpstr Addr );
friend void NetOnline_DisconnectTCP( INetSocket* Socket );
friend INetSocket* NetOnline_CreateListenerTCP( eg_uint Port );
private:
	SOCKET   m_Socket;
	eg_char8 m_Addr[16];
	egNetMsgData m_ReadData;
	int      m_ReadDataSize;
	eg_bool  m_IsListener:1;
	mutable eg_bool  m_HadError:1;
public:
	EGNetSocketTCP()
	: m_ReadDataSize(0)
	, m_IsListener(false)
	, m_HadError(false)
	{
		zero( &m_Socket );
		zero( &m_Addr );
		zero( &m_ReadData );
	}

	virtual eg_bool     HadError()const override;
	virtual void        SetError() override;
	virtual eg_string   GetRemoteAddr()const override;
	virtual eg_string   GetMyAddr()const override;
	virtual eg_bool     IsReadyToRead() const override;
	virtual eg_size_t   Read( egNetMsgData* DataIn ) override;
	virtual void        Write( const egNetMsgData* DataOut ) override;
	virtual INetSocket* ListenerScanForClient()const override;
};

//
// Net Socket Interface
//

void NetOnline_Server_InitTCP( eg_uint Port )
{
	unused( Port );
}

void NetOnline_Server_DeinitTCP()
{

}

void NetOnline_Server_UpdateTCP()
{

}

INetSocket* NetOnline_ConnectTCP( eg_cpstr Addr )
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

	EGNetSocketTCP* SocketOut = nullptr;

	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == Socket)
	{
		NetOnline_PrintWinSockError(("socket"), WSAGetLastError());
	}
	else
	{
		const char NoDelayOpt = 1;
		int nRes = setsockopt( Socket , IPPROTO_TCP , TCP_NODELAY , &NoDelayOpt , sizeof(NoDelayOpt) );
		if( SOCKET_ERROR == nRes )
		{
			NetOnline_PrintWinSockError( "setsockopt" , WSAGetLastError() );
			closesocket( Socket );
		}
		else
		{
			eg_char8 sAddr8[64];
			sAddr.CopyTo(sAddr8, countof(sAddr8));

			eg_uint16 Port = static_cast<eg_uint16>(sPort.ToUInt());

			sockaddr_in AddrIn;
			zero( &AddrIn );
			AddrIn.sin_family = AF_INET;
			AddrIn.sin_port   = htons(Port);
			InetPtonA( AF_INET , sAddr8 , &AddrIn.sin_addr.s_addr );
			nRes = connect(Socket, reinterpret_cast<sockaddr*>(&AddrIn), sizeof(AddrIn));
	
			if(SOCKET_ERROR == nRes)
			{
				NetOnline_PrintWinSockError(("connect"), WSAGetLastError());
				closesocket(Socket);
			}
			else
			{
				SocketOut = new ( eg_mem_pool::Default ) EGNetSocketTCP;
				if( nullptr != SocketOut )
				{
					EGLogf(eg_log_t::NetOnlineTCP , "NetOnline: Connected to %s.", Addr );
					SocketOut->m_Socket = Socket;
					InetNtopA( AddrIn.sin_family , &AddrIn.sin_addr.s_addr , SocketOut->m_Addr , countof(SocketOut->m_Addr) );
					SocketOut->m_IsListener  = false;
					SocketOut->m_HadError = false;
					SocketOut->m_ReadDataSize = 0;
				}
				else
				{
					EGLogf(eg_log_t::NetOnlineTCP , "NetOnline: Out of memory, couldn't connect to %s." , Addr );
					closesocket(Socket);
				}
			}
		}
	}
	return SocketOut;
}

void NetOnline_DisconnectTCP( INetSocket* Socket )
{
	if( nullptr == Socket )return;
	EGNetSocketTCP* SocketTCP = static_cast<EGNetSocketTCP*>(Socket);

	int Res = 0;
	if( !SocketTCP->m_IsListener )
	{
		Res = shutdown( SocketTCP->m_Socket , SD_BOTH );
		if( SOCKET_ERROR == Res )
		{
			NetOnline_PrintWinSockError( "shutdown" , WSAGetLastError() );
		}
	}
	Res = closesocket( SocketTCP->m_Socket );
	if( SOCKET_ERROR == Res )
	{
		NetOnline_PrintWinSockError( "closesocket" , WSAGetLastError() );
	}

	delete SocketTCP;
}

INetSocket* NetOnline_CreateListenerTCP( eg_uint Port )
{
	//const egNetSocketInternal* ISocket = reinterpret_cast<const egNetSocketInternal*>(&Socket);
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	EGNetSocketTCP* SocketOut = nullptr;
	if(INVALID_SOCKET == Socket)
	{
		NetOnline_PrintWinSockError(("socket"), WSAGetLastError());
	}
	else
	{
		sockaddr_in AddrIn;
		AddrIn.sin_family      = AF_INET;
		AddrIn.sin_port        = htons(static_cast<eg_uint16>(Port));
		AddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

		int nRes = bind(Socket, reinterpret_cast<const sockaddr*>(&AddrIn), sizeof(AddrIn));
	
		if(SOCKET_ERROR == nRes)
		{
			NetOnline_PrintWinSockError("bind", WSAGetLastError());
			closesocket(Socket);
		}
		else
		{
			nRes = listen(Socket, MAX_CLIENTS*2);
			if(SOCKET_ERROR == nRes)
			{
				NetOnline_PrintWinSockError("listen", WSAGetLastError());
				closesocket(Socket);
			}
			else
			{
				SocketOut = new ( eg_mem_pool::Default ) EGNetSocketTCP;
				if( nullptr != SocketOut )
				{
					EGLogf( eg_log_t::NetOnlineTCP , ("NetOnline: Listening on port %u."), Port );
					SocketOut->m_Socket = Socket;
					InetNtopA( AddrIn.sin_family , &AddrIn.sin_addr.s_addr , SocketOut->m_Addr , countof(SocketOut->m_Addr) );
					SocketOut->m_IsListener = true;
					SocketOut->m_HadError = false;
					SocketOut->m_ReadDataSize = 0;
				}
				else
				{
					EGLogf( eg_log_t::NetOnlineTCP , "NetOnline: Out of memory, couldn't create listener at %u." , Port );
					closesocket(Socket);
				}
			}
		}
	}
	return SocketOut;
}

void NetOnline_DestroyListenerTCP( INetSocket* Socket )
{
	NetOnline_DisconnectTCP( Socket );
}

INetSocket* EGNetSocketTCP::ListenerScanForClient()const
{
	EGNetSocketTCP* SocketOut = nullptr;

	if( m_IsListener )
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
		}
		else if( nRes > 0 )
		{
			//We'll only get one client at this time.
			sockaddr_in AddrFrom;
			int AddrLen = sizeof(AddrFrom);
			SOCKET Socket = accept( m_Socket , reinterpret_cast<sockaddr*>(&AddrFrom), &AddrLen);
			if(INVALID_SOCKET == Socket)
			{
				NetOnline_PrintWinSockError( "accept" , WSAGetLastError() );
			}
			else
			{
				const char NoDelayOpt = 1;
				int nRes = setsockopt( Socket , IPPROTO_TCP , TCP_NODELAY , &NoDelayOpt , sizeof(NoDelayOpt) );
				if( SOCKET_ERROR == nRes )
				{
					NetOnline_PrintWinSockError( "setsockopt" , WSAGetLastError() );
					closesocket( Socket );
				}
				else
				{
					SocketOut = new ( eg_mem_pool::Default ) EGNetSocketTCP;
					if( nullptr != SocketOut )
					{
						SocketOut->m_Socket = Socket;
						InetNtopA( AddrFrom.sin_family , &AddrFrom.sin_addr.s_addr , SocketOut->m_Addr , countof(SocketOut->m_Addr) );
						EGLogf( eg_log_t::NetOnlineTCP , "NetOnline: Got connection from %s." , SocketOut->m_Addr );
						SocketOut->m_IsListener = false;
						SocketOut->m_HadError = false;
						SocketOut->m_ReadDataSize = 0;
					}
					else
					{
						EGLogf( eg_log_t::NetOnlineTCP , "NetOnline: Out of memory, couldn't obtain client." );
						closesocket(Socket);
					}
				}
			}
		}
	}
	else
	{
		assert( false ); //Should only call this on a socket that was created as a listener.
	}

	return SocketOut;
}

eg_bool EGNetSocketTCP::HadError()const
{
	return nullptr == this || m_HadError;
}

eg_string EGNetSocketTCP::GetRemoteAddr()const
{
	eg_string StrOut = m_Addr;
	return StrOut;
}

eg_string EGNetSocketTCP::GetMyAddr()const
{
	eg_string StrOut = "UNK";
	return StrOut;
}

eg_bool EGNetSocketTCP::IsReadyToRead() const
{
	if( m_HadError )return false;
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

	return nRes > 0;
}

eg_size_t EGNetSocketTCP::Read( egNetMsgData* DataIn )
{
	if( nullptr == this )return 0;
	if( m_HadError )return 0;

	eg_int SizeWanted = 0;
	if( m_ReadDataSize < sizeof(m_ReadData.Header) )
	{
		//If the SizeRead is less than the size of the header, read up to 
		//the end of the header.
		SizeWanted = sizeof(m_ReadData.Header);
	}
	else
	{
		//Otherwise we want the whole message:
		SizeWanted = sizeof(m_ReadData.Header)+m_ReadData.Header.nSize;
	}

	eg_int Start = m_ReadDataSize;
	eg_int SizeToRead = SizeWanted - Start;
	assert( 0 < SizeToRead && SizeToRead <= (static_cast<eg_int>(sizeof(egNetMsgData))-Start) );
	eg_byte* ByteBuffer = reinterpret_cast<eg_byte*>(&m_ReadData);
	int Res = recv( m_Socket , reinterpret_cast<char*>(&ByteBuffer[Start]) , static_cast<int>(SizeToRead) , 0 );
	eg_int BytesRead = Res != SOCKET_ERROR ? Res : 0;

	m_ReadDataSize += BytesRead;

	eg_size_t SizeOut = 0;
	if( m_ReadDataSize >= sizeof(m_ReadData.Header) )
	{
		eg_int TotalSizeToRead = sizeof(m_ReadData.Header)+m_ReadData.Header.nSize;
		assert( TotalSizeToRead <= sizeof(egNetMsgData) );

		if( m_ReadDataSize == TotalSizeToRead )
		{
			if( EG_To<eg_net_msg>(0) <= m_ReadData.Header.nMsg && m_ReadData.Header.nMsg < eg_net_msg::COUNT )
			{
				*DataIn = m_ReadData;
				SizeOut = TotalSizeToRead;
			}
			else //If we got a bogus message we lost packets or something and there is no way to recover.
			{
				SetError();
			}
			m_ReadDataSize = 0;
		}
	}
	
	if( SOCKET_ERROR == Res )
	{
		NetOnline_PrintWinSockError( "recv" , WSAGetLastError() );
		m_HadError = true;
	}

	return SizeOut;
}

void EGNetSocketTCP::Write( const egNetMsgData* DataOut )
{
	if( nullptr == this )return;
	if( m_HadError )return;

	int Res = send( m_Socket , reinterpret_cast<const char*>(DataOut) , static_cast<int>(sizeof(DataOut->Header)+DataOut->Header.nSize) , 0 );
	if( SOCKET_ERROR == Res )
	{
		NetOnline_PrintWinSockError( "send" , WSAGetLastError() );
		m_HadError = true;
	}
}

void EGNetSocketTCP::SetError()
{
	m_HadError = true;
}

#endif

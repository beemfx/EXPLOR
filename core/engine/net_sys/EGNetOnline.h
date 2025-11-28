// NetOnline - An interface that handles internet traffic.
// (c) 2015 Beem Software
#pragma once

#if WITH_NET_COMS

class EGNetMsgBuffer;
struct egNetCommId;
struct egNetMsgData;

class INetSocket;

void NetOnline_Init();
void NetOnline_Deinit();

void NetOnline_Server_Init( eg_uint Port );
void NetOnline_Server_Update();
void NetOnline_Server_Deinit();

INetSocket* NetOnline_Connect( eg_cpstr Addr );
void        NetOnline_Disconnect( INetSocket* Socket );

INetSocket* NetOnline_CreateListener( eg_uint Port );
void        NetOnline_DestroyListener( INetSocket* Socket );


class INetSocket
{
public:
	virtual eg_bool     HadError()const=0;
	virtual void        SetError()=0;
	virtual eg_string   GetRemoteAddr()const=0;
	virtual eg_string   GetMyAddr()const=0;
	virtual eg_bool     IsReadyToRead()=0;
	virtual eg_size_t   Read( egNetMsgData* DataIn )=0;
	virtual void        Write( const egNetMsgData* DataOut )=0;
	virtual INetSocket* ListenerScanForClient()const=0;
};

void NetOnline_AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer );
void NetOnline_RemoveNetObj( INetSocket* Sock );
void NetOnline_PrintWinSockError( eg_cpstr s , eg_int nCode );

#else

class EGNetMsgBuffer;
struct egNetCommId;
struct egNetMsgData;

class INetSocket;

static inline void NetOnline_Init() { }
static inline void NetOnline_Deinit() { }

static inline void NetOnline_Server_Init( eg_uint Port ) { unused( Port ); }
static inline void NetOnline_Server_Update() { }
static inline void NetOnline_Server_Deinit() { }

static inline INetSocket* NetOnline_Connect( eg_cpstr Addr ) { unused( Addr ); return nullptr; }
static inline void        NetOnline_Disconnect( INetSocket* Socket ) { unused( Socket ); }

static inline INetSocket* NetOnline_CreateListener( eg_uint Port ) { unused( Port ); return nullptr; }
static inline void        NetOnline_DestroyListener( INetSocket* Socket ) { unused( Socket ); }


class INetSocket
{
public:
	virtual eg_bool     HadError() const { return true; }
	virtual void        SetError() { }
	virtual eg_string   GetRemoteAddr() const { return ""; }
	virtual eg_string   GetMyAddr() const { return ""; }
	virtual eg_bool     IsReadyToRead() const { return true; }
	virtual eg_size_t   Read( egNetMsgData* DataIn ) { unused( DataIn ); return 0; }
	virtual void        Write( const egNetMsgData* DataOut ) { unused( DataOut ); }
	virtual INetSocket* ListenerScanForClient() const { return nullptr; }
};

static inline void NetOnline_AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer ) { unused( Sock , Id , RecBuffer , SendBuffer ); }
static inline void NetOnline_RemoveNetObj( INetSocket* Sock ) { unused( Sock ); }
static inline void NetOnline_PrintWinSockError( eg_cpstr s , eg_int nCode ) { unused( s , nCode ); }

#endif

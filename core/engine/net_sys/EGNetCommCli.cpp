// EGNetCommCli
// (c) 2015 Beem Software

#include "EGNetCommCli.h"
#include "EGNetLocal.h"
#include "EGNetOnline.h"
#include "EGNetMsgs.h"
#include "EGNetMsgBuffer.h"
#include "EGWorkerThreads.h"
#include "EGThreadRequest.h"

static eg_uint EGNetCommCli_NextClientId = 0;

class EGReqConnCb: public IRequestCallback
{
public:
	enum STATE
	{
		STATE_NONE,
		STATE_CONNECTING,
		STATE_COMPLETE,
	};
public:
	EGReqConnCb(): m_Socket( nullptr ), m_State(STATE_NONE){ }
	
	STATE GetState()const
	{
		EGFunctionLock Lock( &m_Mutex );
		STATE Out = m_State;
		return Out;
	}

	void RequestConnect( eg_cpstr Addr )
	{
		assert( nullptr == m_Socket );
		assert( m_State == STATE_NONE ); //Not a good idea to put the same request in twice.
		m_Address = Addr;
		m_State = STATE_CONNECTING;
		WorkerThread->MakeRequest( this );
	}

	void Cancel()
	{
		if( GetState() == STATE_CONNECTING )
		{
			WorkerThread->CancelRequest( this ); //It's possible we can cause a stall during this cancel, but that case should be pretty rare.
			m_State = STATE_COMPLETE;
		}

		assert( m_State != STATE_CONNECTING );
		if( m_Socket )
		{
			NetOnline_Disconnect( m_Socket );
		}

		Reset();
	}

	INetSocket* GetSocket()const
	{
		assert( m_State == STATE_COMPLETE ); //Don't call this until the request is complete.
		return m_Socket;
	}

	void Reset()
	{
		assert( m_State == STATE_COMPLETE || m_State == STATE_NONE ); //Don't call this until the request is complete.
		m_Socket = nullptr;
		m_State = STATE_NONE;
		m_Address.Clear();
	}

private:
	mutable EGMutex m_Mutex;
	eg_string       m_Address;
	INetSocket*    m_Socket;
	STATE           m_State;

private:
	virtual void Action() override
	{
		assert( nullptr == m_Socket );
		assert( m_State == STATE_CONNECTING );
		m_Socket = NetOnline_Connect( m_Address );
	}

	virtual void Callback()
	{
		EGFunctionLock Lock( &m_Mutex ); //We lock this, because it's possible this callback will happen on a different thread, we just want to make sure that the data is correct.
		m_State = STATE_COMPLETE;
	}

};

EG_ALIGN struct EGNetCommCli::egData
{
	egNetLocalId   LocalId;
	INetSocket*   Sock;
	EGNetMsgBuffer Rec;
	EGNetMsgBuffer Send;
	EGReqConnCb    ConnReq;
	eg_uint        ClientIndex;
	eg_bool        PendingConnect:1;
};

EGNetCommCli::EGNetCommCli()
: EGNetCommBase()
{
	static_assert( sizeof(egData) <= sizeof(m_DMem) , "Increase the size of m_DMem." );
	zero( &m_DMem );
	m_D = new (m_DMem) egData;
	m_D->LocalId = NET_LOCAL_ID_NOT_CONNECTED;
	m_D->Sock = nullptr;
	m_D->ClientIndex = EGNetCommCli_NextClientId;
	EGNetCommCli_NextClientId++;
}

EGNetCommCli::~EGNetCommCli()
{
	m_D->~egData();
	m_D = nullptr;
}

eg_bool EGNetCommCli::IsLocal()const
{
	return NetLocal_IsClientConnected( m_D->LocalId );
}

void EGNetCommCli::Connect( eg_cpstr Address )
{
	Disconnect();
	if( Address )
	{
		assert( !m_D->PendingConnect );
		m_D->PendingConnect = true;
		m_D->ConnReq.RequestConnect( Address );
	}
	else
	{
		m_D->LocalId = NetLocal_Client_Connect();
	}
}

void EGNetCommCli::Disconnect()
{
	if( NetLocal_IsClientConnected( m_D->LocalId ) )
	{
		NetLocal_Client_Disconnect( m_D->LocalId );
	}
	else if( nullptr != m_D->Sock )
	{
		NetOnline_RemoveNetObj( m_D->Sock );
		NetOnline_Disconnect( m_D->Sock ); //Should probably tell the server we are disconnecting, but it will eventually figure out that we aren't there anymore.
	}

	m_D->Rec.FlushMsgs();
	m_D->Send.FlushMsgs();

	m_D->ConnReq.Cancel();
	m_D->PendingConnect = false;

	m_D->LocalId = NET_LOCAL_ID_NOT_CONNECTED;
	m_D->Sock = nullptr;
}

EGNetCommCli::CONN_S EGNetCommCli::GetConnectionStatus()const
{
	if( m_D->PendingConnect )
	{
		return CONN_CONNECTING;
	}
	return NetLocal_IsClientConnected( m_D->LocalId ) || ( nullptr != m_D->Sock ) ? CONN_CONNECTED : CONN_NOT_CONNECTED;
}

void EGNetCommCli::BeginFrame()
{
	EGNetCommBase::BeginFrame();

	if( m_D->PendingConnect )
	{
		assert( EGReqConnCb::STATE_NONE != m_D->ConnReq.GetState() );
		if( EGReqConnCb::STATE_COMPLETE == m_D->ConnReq.GetState() )
		{
			m_D->Sock = m_D->ConnReq.GetSocket();
			m_D->ConnReq.Reset();
			if( nullptr != m_D->Sock )
			{
				NetOnline_AddNetObj( m_D->Sock , egNetCommId::IndexToId(m_D->ClientIndex) , m_D->Rec , m_D->Send );
			}
			m_D->PendingConnect = false;
		}
	}

	if( nullptr != m_D->Sock && m_D->Sock->HadError() )
	{
		EGLogf( eg_log_t::NetCommC , "Lost connection to host." );
		NetOnline_RemoveNetObj( m_D->Sock );
		NetOnline_Disconnect( m_D->Sock );
		m_D->Sock = nullptr;
		m_D->Rec.FlushMsgs();
		m_D->Send.FlushMsgs();
	}
}

void EGNetCommCli::EndFrame()
{
	//Handle locally received messages:
	if( NetLocal_IsClientConnected( m_D->LocalId ) )
	{
		NetLocal_Client_HandleReceivedMsgs( m_D->LocalId , HandleRecMsg , this );
	}
	else if( nullptr != m_D->Sock )
	{
		m_D->Rec.PumpMsgs( HandleRecMsg , this );
	}
	EGNetCommBase::EndFrame();
}

void EGNetCommCli::PostMsg( const struct egNetMsgData* Data )
{
	//Final Data Here...
	assert( NET_ID_ALL == Data->Header.NetId ); //Client should always braodcast to all (since there is only a server).
	egNetMsgData FinalData = *Data;
	FinalData.Header.NetId.Id = m_D->LocalId.Id;
	EGNetCommBase::PostMsg( &FinalData );
}

void EGNetCommCli::ProcessOutgoingMsg( const struct egNetMsgData* Data )
{
	if( NetLocal_IsClientConnected( m_D->LocalId ) )
	{
		NetLocal_Client_PostMsg( m_D->LocalId , Data );
	}
	else if( nullptr != m_D->Sock )
	{
		m_D->Send.AddMsg( Data );
	}
	else
	{
		//assert( false ); //Message dropped.
		EGLogf( eg_log_t::NetCommC , __FUNCTION__ " Warning: No connection,  message dropped." );
	}
}

void EGNetCommCli::HandleRecMsg( const struct egNetMsgData* Data , void* UserPointer )
{
	EGNetCommCli* _this = static_cast<EGNetCommCli*>(UserPointer);
	_this->PostRecMsg( Data );
}

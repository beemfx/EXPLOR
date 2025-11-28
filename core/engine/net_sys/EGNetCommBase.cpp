// EGNetComm2
// (c) 2015 Beem Software

#include "EGNetCommBase.h"
#include "EGNetMsgBuffer.h"
#include "EGNetLocal.h"

struct EGNetCommBase::egNetComm2Data
{
	EGNetMsgBuffer RecBuffer;
	EGNetMsgBuffer OutBuffer;
	eg_bool        Inited:1;
	eg_bool        InFrame:1;
};

EGNetCommBase::EGNetCommBase()
{
	static_assert( sizeof(egNetComm2Data) <= sizeof(m_DN2Mem) , "Increase the size of m_DMem." );
	zero( &m_DN2Mem );
	m_DN2 = new (m_DN2Mem) egNetComm2Data;
}

EGNetCommBase::~EGNetCommBase()
{
	assert( m_DN2->OutBuffer.IsEmpty() );
	assert( m_DN2->RecBuffer.IsEmpty() );

	m_DN2->~egNetComm2Data();
	m_DN2 = nullptr;
}

void EGNetCommBase::Init()
{
	m_DN2->Inited = true;
}

void EGNetCommBase::Deinit()
{
	m_DN2->Inited = false;
	m_DN2->OutBuffer.FlushMsgs();
	m_DN2->RecBuffer.FlushMsgs();
}

void EGNetCommBase::BeginFrame()
{
	assert( m_DN2->OutBuffer.IsEmpty() );
	assert( !m_DN2->InFrame );
	assert( m_DN2->Inited );
	m_DN2->InFrame = true;
}

void EGNetCommBase::EndFrame()
{
	assert( m_DN2->InFrame );
	assert( m_DN2->Inited );
	m_DN2->InFrame = false;
	m_DN2->OutBuffer.PumpMsgs( ProcessOutgoingMsgs , this );
}

void EGNetCommBase::PostMsg( const struct egNetMsgData* Data )
{
	assert( m_DN2->InFrame ); //Should only post a message while in-frame.
	assert( m_DN2->Inited );
	m_DN2->OutBuffer.AddMsg( Data );
}

void EGNetCommBase::PostRecMsg( const struct egNetMsgData* Data )
{
	assert( m_DN2->Inited );
	m_DN2->RecBuffer.AddMsg( Data );
}

void EGNetCommBase::ProcessReceivedMsgs( void ( * Callback )( const struct egNetMsgData* Data , void* UserPointer ) , void* UserPointer )
{
	assert( m_DN2->InFrame ); //Many incoming messages send outgoing messages, so we should do this in-frame.
	assert( m_DN2->Inited );
	m_DN2->RecBuffer.PumpMsgs( Callback , UserPointer );
}

void EGNetCommBase::ProcessOutgoingMsgs( const struct egNetMsgData* Data , void* UserPointer )
{
	EGNetCommBase* _this = static_cast<EGNetCommBase*>( UserPointer );
	assert( _this->m_DN2->Inited );
	_this->ProcessOutgoingMsg( Data );
}

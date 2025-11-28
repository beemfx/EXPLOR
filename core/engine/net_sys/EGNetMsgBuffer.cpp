// EGNetMsgBuffer
// (c) 2015 Beem Software
#include "EGNetMsgBuffer.h"

EGNetMsgBuffer::EGNetMsgBuffer()
:m_List(1)
{
}

EGNetMsgBuffer::~EGNetMsgBuffer()
{
	assert( 0 == m_List.Len() );
}

void EGNetMsgBuffer::AddMsg( const egNetMsgData* Data )
{
	EGFunctionLock Lock( &m_Mutex );

	egMsg* NewMsg = new ( eg_mem_pool::DefaultHi ) egMsg;
	if( nullptr == NewMsg )
	{
		assert( false ); //Out of memory?
		return;
	}

	NewMsg->Data = *Data;
	m_List.InsertLast( NewMsg );
}

void EGNetMsgBuffer::AddStrMsg( const struct egNetCommId& DestNetId , eg_cpstr String )
{
	egNetMsgData Data;
	Data.Header.NetId = DestNetId;
	Data.Header.nMsg = eg_net_msg::NET_STRCMD;
	eg_string S( String );
	assert( S.Len() < countof(Data.StrCmd.Cmd) );
	S.CopyTo( Data.StrCmd.Cmd , countof(Data.StrCmd.Cmd) );
	Data.StrCmd.Length = static_cast<eg_uint8>(S.Len());
	Data.Header.nSize = sizeof(Data.StrCmd.Length) + sizeof(Data.StrCmd.Cmd[0])*(Data.StrCmd.Length+1); 
	Data.StrCmd.Cmd[Data.StrCmd.Length] = 0;
	AddMsg( &Data );
}

void EGNetMsgBuffer::AddNoParmMsg( const struct egNetCommId& DestNetId, eg_net_msg Msg )
{
	egNetMsgData MsgData;
	MsgData.Header.NetId = DestNetId;
	MsgData.Header.nMsg = Msg;
	MsgData.Header.nSize = 0;
	AddMsg( &MsgData );
}

void EGNetMsgBuffer::PumpMsgs( void ( * Callback )( const egNetMsgData* Data , void* UserPointer ) , void* UserPointer )
{
	EGFunctionLock Lock( &m_Mutex );

	while( !IsEmpty() )
	{
		auto Item = m_List.GetFirst();
		m_List.Remove( Item );

		Callback( &Item->Data , UserPointer );

		delete Item;
	}
}

void EGNetMsgBuffer::FlushMsgs()
{
	EGFunctionLock Lock( &m_Mutex );

	while( !IsEmpty() )
	{
		auto Item = m_List.GetFirst();
		m_List.Remove( Item );
		delete Item;
	}
}

void EGNetMsgBuffer::GetAndRemoveFirstItem( egNetMsgData* DataOut )
{
	EGFunctionLock Lock( &m_Mutex );

	assert( m_List.HasItems() );
	if( m_List.HasItems() )
	{
		auto Item = m_List.GetFirst();
		m_List.Remove( Item );
		*DataOut = Item->Data;
		delete Item;
	}
}
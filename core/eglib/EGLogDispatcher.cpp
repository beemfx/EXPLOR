// (c) 2017 Beem Media

#include "EGLogDispatcher.h"

void EGLogDispatcher::QueueLog( eg_log_channel Channel, eg_cpstr LogText )
{
	EGFunctionLock Lock( &m_Lock );
	m_Queue.InsertLastAndOverwriteFirstIfFull( egLogEntry( Channel, LogText ) );
}

void EGLogDispatcher::DispatchLogs()
{
	EGFunctionLock Lock( &m_Lock );
	for( eg_uint i = 0; i < m_Queue.Len(); i++ )
	{
		Delegate.Broadcast( eg_log_channel( m_Queue[i].Channel ), m_Queue[i].LogText );
	}
	m_Queue.Clear();
}

void EGLogDispatcher::CopyTo( EGLogDispatcher& Dest )
{
	EGFunctionLock Lock( &m_Lock );
	EGFunctionLock DestLock( &Dest.m_Lock );

	for( eg_uint i=0; i< m_Queue.Len(); i++ )
	{
		Dest.m_Queue.InsertLastAndOverwriteFirstIfFull( m_Queue[i] );
	}
}

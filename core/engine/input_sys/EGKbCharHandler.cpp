#include "EGKbCharHandler.h"

EGKbCharHandler EGKbCharHandler::s_Instance;

void EGKbCharHandler::AddListener( IListener* Listener , eg_bool bBlockOtherListeners )
{
	if( nullptr == Listener )
	{
		assert( false );
		return;
	}

	if( !m_Listeners.IsFull() )
	{
		egListenerInfo ListenInfo;
		ListenInfo.Listener = Listener;
		ListenInfo.bBlockOtherListeners = bBlockOtherListeners;
		m_Listeners.Append( ListenInfo );
	}
	else
	{
		EGLogf( eg_log_t::Error , "Tool many character listeners added (is one not being removed?)" );
		assert( false );
	}
}

void EGKbCharHandler::RemoveListener( IListener* Listener )
{
	if( nullptr == Listener )
	{
		assert( false );
		return;
	}

	eg_bool bRemoved = false;
	for( eg_size_t i=0; i<m_Listeners.Len(); i++ )
	{
		if( Listener == m_Listeners[i].Listener )
		{
			m_Listeners.DeleteByIndex( i );
			bRemoved = true;
			break;
		}
	}

	assert( bRemoved );
}

eg_bool EGKbCharHandler::ProcessChar( eg_char16 Char )
{
	eg_bool bHandled = false;

	// Process listeners in reverse order

	for( eg_size_t i=0; i<m_Listeners.Len(); i++ )
	{
		eg_size_t Index = m_Listeners.Len() - 1 - i;
		egListenerInfo& Info = m_Listeners[Index];
		if( Info.Listener->HandleTypedChar( Char ) )
		{
			bHandled = true;
		}

		if( Info.bBlockOtherListeners )
		{
			break;
		}
	}

	return bHandled;
}

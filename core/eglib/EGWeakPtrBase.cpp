// (c) 2017 Beem Media

#include "EGWeakPtrBase.h"
#include "EGObject.h"

void EGWeakPtrBase::Register()
{
	if( m_AsObject )
	{
		m_AsObject->AddWeakPtr( this );
	}
}

void EGWeakPtrBase::Unregister()
{
	if( m_AsObject )
	{
		m_AsObject->RemoveWeakPtr( this );
	}

	m_AsObject = nullptr;
}

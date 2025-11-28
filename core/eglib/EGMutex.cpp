#include "EGMutex.h"

#include "EGWindowsAPI.h"

struct EGMutex::egInternalData
{
	CRITICAL_SECTION m_CS;
};


EGMutex::EGMutex()
{
	static_assert( sizeof(m_InternalDataMem) >= sizeof(egInternalData) , "Size wrong" );
	this->m_pD = new ( m_InternalDataMem ) egInternalData;
	InitializeCriticalSection(&m_pD->m_CS);
}

EGMutex::~EGMutex()
{
	DeleteCriticalSection(&m_pD->m_CS);
}

void EGMutex::Lock()
{
	EnterCriticalSection(&m_pD->m_CS);
}

void EGMutex::Unlock()
{
	LeaveCriticalSection(&m_pD->m_CS);
}

eg_bool EGMutex::TryLock()
{
	return TRUE == TryEnterCriticalSection(&m_pD->m_CS);
}

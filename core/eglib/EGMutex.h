// (c) 2017 Beem Media

#pragma once

class EGMutex
{
public:
	EGMutex();
	~EGMutex();

	void Lock();
	void Unlock();
	eg_bool TryLock(); //Will not halt the thread, returns true if lock was had.

private:
	struct  egInternalData;
	struct  egInternalData* m_pD;
	eg_byte m_InternalDataMem[40];
};

class EGFunctionLock
{
public:
	EGFunctionLock( EGMutex* Mutex )
	: m_Mutex( Mutex )
	{
		m_Mutex->Lock();
	}

	~EGFunctionLock()
	{
		m_Mutex->Unlock();
	}

private:
	EGMutex* m_Mutex;
};
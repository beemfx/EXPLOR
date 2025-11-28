// (c) 2011 Beem Media. All Rights Reserved.

#include "EGThread.h"

#if defined( __WIN64__ ) || defined( __WIN32__ )

#include <process.h>
#include "EGWindowsAPI.h"
#include "EGTimer.h"

struct EGThread::egPlatformData
{
	HANDLE       hThread;
	unsigned int ThreadID;
};

void EGThread_Sleep( eg_real Seconds )
{
	Sleep( eg_uint(Seconds*1000) );
}

EGThread::EGThread( const egInitParms& InitParms )
: m_Lock()
, m_InitParms( InitParms )
, m_Timer()
, m_State( STATE_NOT_RUNNING )
, m_Procs( eg_string_crc(*InitParms.ThreadName).ToUint32() )
{
	static_assert( sizeof(m_PlDMem) >= sizeof(egPlatformData) , "Need more mem for this." );
	m_PlD = new( m_PlDMem ) egPlatformData;
	m_PlD->hThread = nullptr;
	m_PlD->ThreadID = 0;
}

EGThread::~EGThread()
{
	if(IsRunning())
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ " Warning: Thread %s went out of scope while the thread was still running.", *m_InitParms.ThreadName );
		Stop();
	}
}

eg_bool EGThread::IsCurrentThread()const
{
	return m_PlD->ThreadID == GetCurrentThreadId();
}

unsigned __stdcall EGThread::Thread(void* pParm)
{
	EGThread* _this = static_cast<EGThread*>(pParm);

	_this->m_Timer.Init();
	for( IThreadProc* Proc : _this->m_Procs )
	{
		Proc->OnStart();
	}
	assert( STATE_INITIALIZING == _this->GetState() );
	_this->SetState( STATE_RUNNING );

	_this->m_Timer.Update(); //Update timer during main thread so the actual time elapsed is since it was initialized.
	
	while( STATE_RUNNING == _this->GetState() )
	{
		//Slow the Update to the desired framerate.
		eg_real FrameTime = _this->m_Timer.GetRawElapsedSec();

		if( FrameTime < _this->m_InitParms.DesiredFrameTime )
		{
			const eg_real TimeLeft = _this->m_InitParms.DesiredFrameTime - FrameTime;
			if( TimeLeft > _this->m_InitParms.SleepThreshold )
			{
				EGThread_Sleep( TimeLeft - _this->m_InitParms.SleepThreshold );
			}
			else
			{
				EGThread_Sleep( 0 );
			}
			continue;
		}

		//If there is a new proc to be run, start it.
		_this->m_Timer.Update();
		//The workload:
		_this->ProcessThreadMsgs();
		eg_real Elapsed = _this->m_Timer.GetElapsedSec();
		for( IThreadProc* Proc : _this->m_Procs )
		{
			Proc->Update( Elapsed );
		}
	}
	//Clean up the thread.
	assert( STATE_QUIT_REQUEST == _this->GetState() );
	_this->SetState( STATE_DEINITIALIZING );
	for( IThreadProc* Proc : _this->m_Procs )
	{
		Proc->OnStop();
	}
	_this->m_Timer.Shutdown();

	assert( STATE_DEINITIALIZING == _this->GetState() );
	_this->SetState( STATE_NOT_RUNNING );

	_endthreadex(0);

	return 0;
}

void EGThread::Start()
{
	if(IsRunning())
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Warning: Thread %s was already started." , *m_InitParms.ThreadName );
		return;
	}

	EGLogf( eg_log_t::Thread , "Starting %s thread..." , *m_InitParms.ThreadName );
	assert( STATE_NOT_RUNNING == GetState() );
	SetState( STATE_INITIALIZING );

	m_PlD->hThread = (HANDLE)::_beginthreadex(nullptr, 0, Thread, reinterpret_cast<void*>(this), 0, &m_PlD->ThreadID);
	
	if(INVALID_HANDLE_VALUE == m_PlD->hThread)
	{
		SetState( STATE_NOT_RUNNING );
		assert( false );
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Thread %s failed to be created." , *m_InitParms.ThreadName );
		return;
	}
	#if 1//defined(DEBUG)
	{
		const DWORD MS_VC_EXCEPTION=0x406D1388;

		#pragma pack(push,8)
		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType; // Must be 0x1000.
			LPCSTR szName; // Pointer to name (in user addr space).
			DWORD dwThreadID; // Thread ID (-1=caller thread).
			DWORD dwFlags; // Reserved for future use, must be zero.
		} THREADNAME_INFO;
		#pragma pack(pop)

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = *m_InitParms.ThreadName;
		info.dwThreadID = m_PlD->ThreadID;
		info.dwFlags = 0;

		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	#endif //Debug
	EGLogf( eg_log_t::Thread , "Thread %s created at 0x%08X with id of 0x%08X." , *m_InitParms.ThreadName , reinterpret_cast<eg_uintptr_t>(m_PlD->hThread) , m_PlD->ThreadID );
}

eg_bool EGThread::WaitForInitialization( void )
{
	STATE State = GetState();
	if( !(STATE_RUNNING == State || STATE_INITIALIZING == State) )
	{
		assert( false ); //Don't call this on a thread unless between calls to Start and Stop.
		return false;
	}

	while( STATE_INITIALIZING == State )
	{
		State = GetState();
		EGThread_Sleep(0);
	}

	assert( STATE_RUNNING == State );
	return STATE_RUNNING == State;
}

void EGThread::Stop()
{
	if(!IsRunning())
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Tried to stop a dead thread (%s)." , *m_InitParms.ThreadName );
		return;
	}

	EGLogf( eg_log_t::Thread , "Notifying thread 0x%08X (%s) to stop..." , reinterpret_cast<eg_uintptr_t>(m_PlD->hThread) , *m_InitParms.ThreadName );
	SetState( STATE_QUIT_REQUEST );

	STATE State = GetState();
	while( STATE_NOT_RUNNING != State )
	{
		State = GetState();
		EGThread_Sleep(0);
	}

	::WaitForSingleObject(m_PlD->hThread, 10000);
	eg_uint nExitCode = STILL_ACTIVE;
	GetExitCodeThread(m_PlD->hThread, &nExitCode);

	if(STILL_ACTIVE != nExitCode)
	{
		EGLogf( eg_log_t::Thread , "Thread 0x%08X (%s) ended with exit code %u." , reinterpret_cast<eg_uintptr_t>(m_PlD->hThread), *m_InitParms.ThreadName , nExitCode );
	}
	else
	{
		assert(false); //Thread didn't end.
	}

	m_PlD->hThread   = 0;
	m_PlD->ThreadID = 0;
}

void EGThread::PostThreadMsg(eg_cpstr strParm)
{
	EGFunctionLock Lock( &m_Lock );
	m_Msgs.EnQueue(strParm);
}

eg_bool EGThread::IsRunning()const
{
	EGFunctionLock Lock( &m_Lock );
	eg_bool bRunning = nullptr != m_PlD->hThread;
	return bRunning;
}

void EGThread::ProcessThreadMsgs()
{
	EGFunctionLock Lock( &m_Lock );

	for(eg_uint l = 0; m_Msgs.HasItems() && l < MAX_MESSAGES; l++)
	{
		eg_string_small m = m_Msgs.Front();
		m_Msgs.DeQueue();
		//We're not really sure which process should get the message, so just send
		//it to all of them (messages are usually sent to the main threads that only 
		//have one process anyway).
		for( IThreadProc* Proc : m_Procs )
		{
			Proc->OnThreadMsg( m );
		}
	}
}

void EGThread::RegisterProc( IThreadProc* Proc )
{
	assert( 0 == Proc->GetListId() ); //Procs should only be registered to one thread (possible that constructor wasn't called too).
	assert( STATE_NOT_RUNNING == GetState() ); //Procs can only be registered while the thread is not running.
	if( STATE_NOT_RUNNING == GetState() )
	{
		m_Procs.Insert( Proc );
	}
}

void EGThread::UnregisterProc( IThreadProc* Proc )
{
	assert( STATE_NOT_RUNNING == GetState() ); //Procs can only be unregistered while the thread is not running.
	if( STATE_NOT_RUNNING == GetState() )
	{
		m_Procs.Remove( Proc );
	}
}

#else

#pragma __NOTICE__( "No implementation for "__FILE__" on this platform." )

#endif

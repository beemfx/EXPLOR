// (c) 2011 Beem Media. All Rights Reserved.
/******************************************************************************
File: Thread.h
Class: EGThread
Purpose: An EG Thread is meant to be the base class for any object that is to
be, or to use, it's own thread. This base class has all the functionality
necessary for starting and stopping threads, as well as passing messages
between threads.

Note that even if an object is the "Main Thread" (i.e. the game thread). It
should still inherit from this class, but should return null from 
GetThreadFunc.

The object may override the following methods:
Init
Update
UnInit
OnThreadMsg

Here is a brief overview of how a thread works.

EGThread::Start is called. At this point a new thread is created. Start will
return very quickly, and at that point the other public members may be called
on the thread class. Note that the thread class should be persistent throughout
the life of the thread.

When the thread first begins there is a call to Init. Technically you could do
everything in the thread here, but usually threads are designed to last for
some time. Init is a good opportunity to prepare any information and data that
the thread will need.

After that repeated calls to Update are made. If the thread doesn't need to do
anything for a while it is a good idea to call EGThread_Sleep to pause the thread for
a bit so that it won't take more processing power than is requisite.

Thread messages may be posted to the thread at any time these are posted by
calling PostThreadMsg. Thread messages are not processed right away, they
are queued up and processed as a batch as part of the thread loop. Thread
messages are processed right before Update is called. Note that they are
queued in order, so they are processed in the order that they are received.

Generally it's not a good idea to pass a lot of thread messages, usually only
when major events occur. Most of the communication between threads in the game
is between the client and server, and that is accomplished through NetComm.

To stop the thread, Stop should be called. Note that the thread itself will 
stop after the next call to Update. The stop method itself doesn't return
until the thread is actually stopped. And if a deadlock occured somewhere the
calling thread may never stop as well. Care should be taken to catch such
situations.

A server may stop itself by claling StopFromThread.

An instance of a thread object may only run one thread at a time, but more than
one instance of a thread class may be created without problems (unless the
thread class uses static variables, in which case only one instance should be
created).

The public interface should only be called outside of the thread object. And
the thread object should not be shared accross multiple threads. Actually, as
a general rule all thread objects should be contained within the game thread,
but in some cases the server may also want to create it's own worker threads.
Additionally the thread functions should never call the public interface.
Think about this. If you called Stop inside of Update, Update would post a
message for the thread, but update would be waiting for Stop to exit so that
message would never get processed and the thread would deadlock. Then if the
thread that contained the thread object called stop, you'd have two frozen
threads.

Each thread has a timer. The timer exists in:
Init
Update
UnInit
OnThreadMsg
It is update once each "Thread Frame". Threads should call
Timer_Time(), Timer_Elapsed(), Timer_Felapsed() to user timer fuctions.

It is best to avoid timer functions. When a thread is created you can specify
how fast it should be run, this is a parameter of the constructor. Then when
the thread is started it will attempt to take that many seconds (or more likely
fractions of a second to run, and will otherwise sleep). This will get you a
pretty good chance that the frame time will be fixed every update.

As a rule any methods that are called directly to the server contain such
as posting messages and so forth should be public, all methods that are run
on the actual thread should be private (especially the Update method, imagine
what would happen if Update was called on both the thread and the container,
probably nothing good).

Locking the Thread
==================
Inevitably data will need to be shared between the thread and the thread
container. The way to do this is by locking the thread whenever the data is
modified or read. Locks should be as quick as possible, basically the idea is
Lock the thread read the data then Unlock the thread, or Lock write Unlock, if
a lot of processing is done in side of a lock it will cause stalls. You may use
an EGMutex to lock a thread.

MAX_LOCKS locks are allowed on a thread.  Different locks can be used with
different data, but the same lock should always be used with the same data.

(c) 2011 Beem Software
******************************************************************************/

#pragma once

#include "EGThreadProc.h"
#include "EGQueue.h"
#include "EGMutex.h"
#include "EGTimer.h"

// TODO: Make these values that can be set before the game starts EGLibrary_Init...
static const eg_real EGTHREAD_DEFAULT_SLEEP_THRESHOLD = 1.f/30.f;

static const eg_real EGTHREAD_DEFAULT_LOW_PRIORITY_FRAME_RATE = 1.f/10.f;
static const eg_real EGTHREAD_DEFAULT_GAME_PRIORITY_FRAME_RATE = 1.f/120.f;
static const eg_real EGTHREAD_DEFAULT_FAST_AS_POSSIBLE_FRAME_RATE = 0.f;

class EGThread final
{
public:

	enum STATE
	{
		STATE_NOT_RUNNING,
		STATE_INITIALIZING,
		STATE_RUNNING,
		STATE_QUIT_REQUEST,
		STATE_DEINITIALIZING,
	};

	enum class eg_init_t
	{
		Default,
		LowPriority,
		GamePriority,
		FastAsPossible,
	};

	struct egInitParms
	{
		eg_s_string_sml8 ThreadName = CT_Clear;
		eg_real DesiredFrameTime = EGTHREAD_DEFAULT_LOW_PRIORITY_FRAME_RATE;
		eg_real SleepThreshold = EGTHREAD_DEFAULT_SLEEP_THRESHOLD;

		egInitParms() = delete;

		egInitParms( eg_cpstr ThreadNameIn , eg_init_t InitType )
		: ThreadName( ThreadNameIn )
		{
			switch( InitType )
			{
				case eg_init_t::Default:
				case eg_init_t::LowPriority:
					DesiredFrameTime = EGTHREAD_DEFAULT_LOW_PRIORITY_FRAME_RATE;
					SleepThreshold = 0.f;
					break;
				case eg_init_t::GamePriority:
					DesiredFrameTime = EGTHREAD_DEFAULT_GAME_PRIORITY_FRAME_RATE;
					SleepThreshold = EGTHREAD_DEFAULT_SLEEP_THRESHOLD;
					break;
				case eg_init_t::FastAsPossible:
					DesiredFrameTime = EGTHREAD_DEFAULT_FAST_AS_POSSIBLE_FRAME_RATE;
					SleepThreshold = EGTHREAD_DEFAULT_SLEEP_THRESHOLD;
					break;
			}
		}
	};

public: 

	EGThread( const egInitParms& InitParms );
	~EGThread();

	void    Start();
	void    Stop();
	eg_bool WaitForInitialization( void );
	void    PostThreadMsg(eg_cpstr sMsg);
	eg_bool IsCurrentThread()const;
	STATE   GetState()const{ EGFunctionLock Lock( &m_Lock ); STATE OutState = m_State; return OutState; }

	void RegisterProc( IThreadProc* Proc );
	void UnregisterProc( IThreadProc* Proc );

private:

	void ProcessThreadMsgs();

private:

	static const eg_uint MAX_MESSAGES = 128;
	typedef EGQueue<eg_string_small , MAX_MESSAGES> EGMsgStack;
	typedef EGList<IThreadProc> EGProcList;

	EGTimer         m_Timer;
	EGProcList      m_Procs;
	const egInitParms m_InitParms;
	STATE           m_State;
	mutable EGMutex m_Lock;
	EGMsgStack      m_Msgs;

	struct egPlatformData;
	egPlatformData* m_PlD;
	eg_byte         m_PlDMem[16];
	
	
	void    SetState( STATE NewState ){ EGFunctionLock Lock( &m_Lock ); m_State = NewState; }
	eg_bool IsRunning()const;

	static unsigned __stdcall Thread(void* pParm);
};

void EGThread_Sleep( eg_real Seconds );
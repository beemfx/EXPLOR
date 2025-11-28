// EGNetThread - A thread for handling the sending and receving of net data.
// (c) 2015 Beem Software
#pragma once

#include "EGThread.h"
#include "EGNetMsgBuffer.h"
#include "EGEngineConfig.h"

struct egNetCommId;
class INetSocket;
class EGNetMsgBuffer;

class EGNetThread: public IThreadProc
{
public:
	EGNetThread( eg_cpstr ThreadId );
	~EGNetThread();

	void AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer );
	void RemoveNetObj( INetSocket* Sock );

	void Start(){ m_Thread.RegisterProc( this ); m_Thread.Start(); }
	void Stop(){ m_Thread.Stop(); m_Thread.UnregisterProc( this ); }

private:
	virtual void OnStart() override;
	virtual void Update( eg_real DeltaTime )  override;
	virtual void OnStop() override;
	virtual void OnThreadMsg(eg_cpstr strParm) override{ unused( strParm ); assert( false ); }

private:
	struct egNetObj;

	static const eg_uint MAX_HANDLED = MAX_CLIENTS;
	static const eg_uint READ_BUFFER_SIZE = 1024;

	egNetObj* m_Objs[MAX_HANDLED];
	eg_byte   m_ObjsMem[(64+32+READ_BUFFER_SIZE)*MAX_HANDLED];
	EGMutex   m_AddRemoveLock;
	EGNetMsgBuffer m_TempMsgBuffer;
	EGThread       m_Thread;
private:
	 static void PumpSendMsgs( const struct egNetMsgData* Data , void* UserPointer );
	 static void PumpCopyMsgs( const struct egNetMsgData* Data , void* UserPointer );
};

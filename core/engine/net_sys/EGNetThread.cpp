// EGNetThread
// (c) 2015 Beem Software

#include "EGNetThread.h"
#include "EGNetMsgs.h"
#include "EGNetOnline.h"
#include "EGNetMsgBuffer.h"

EG_ALIGN struct EGNetThread::egNetObj
{
	egNetCommId        NetId;
	INetSocket*       Sock;
	EGNetMsgBuffer*    Rec;
	EGNetMsgBuffer*    Send;
};

EGNetThread::EGNetThread( eg_cpstr ThreadId )
: m_Thread( EGThread::egInitParms( ThreadId , EGThread::eg_init_t::GamePriority ) )
{
	for( eg_uint i=0; i<countof(m_Objs); i++ )
	{
		m_Objs[i] = reinterpret_cast<egNetObj*>(&m_ObjsMem[i*sizeof(egNetObj)]);
		static_assert( sizeof(m_ObjsMem) >= sizeof(egNetObj)*countof(m_Objs) , "m_ObjsMem must be larger." );

		m_Objs[i]->NetId = NET_ID_NOT_CONNECTED;
		m_Objs[i]->Sock = nullptr;
		m_Objs[i]->Rec = nullptr;
		m_Objs[i]->Send = nullptr;
	}
}

EGNetThread::~EGNetThread()
{
	for( eg_uint i=0; i<countof(m_Objs); i++ )
	{
		assert( m_Objs[i]->Sock == nullptr ); //A connected net object was not removed.
	}
}

void EGNetThread::OnStart()
{

}

void EGNetThread::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	EGThread_Sleep( 0 );

	EGFunctionLock Lock( &m_AddRemoveLock ); //Can't add or remove while reading or writing.

	for( eg_uint i=0; i<countof(m_Objs); i++ )
	{
		egNetObj* Obj = m_Objs[i];
		if( nullptr == Obj->Sock )continue;

		//If we are currently doing a read, continue it:
		//Do the reads first.
		
		if( Obj->Sock->IsReadyToRead() )
		{		
			egNetMsgData Data;
			eg_size_t SizeRead = Obj->Sock->Read( &Data );
			if( 0 != SizeRead )
			{
				assert( SizeRead == (sizeof(Data.Header)+Data.Header.nSize));
				if( EG_To<eg_net_msg>(0) <= Data.Header.nMsg && Data.Header.nMsg < eg_net_msg::COUNT )
				{
					Data.Header.NetId = Obj->NetId;
					Obj->Rec->AddMsg( &Data );
				}
				else //If we got a bogus message we lost packets or something and there is no way to recover.
				{
					Obj->Sock->SetError();
				}
			}
		}

		//Do some writes, we copy the send buffer to a temp buffer so that the applicaiton doesn't stall.
		assert( m_TempMsgBuffer.IsEmpty() );
		Obj->Send->PumpMsgs( PumpCopyMsgs , &m_TempMsgBuffer );
		m_TempMsgBuffer.PumpMsgs( PumpSendMsgs , Obj );
	}
}

void EGNetThread::PumpSendMsgs( const struct egNetMsgData* Data , void* UserPointer )
{
	egNetObj* Obj = static_cast<egNetObj*>(UserPointer);
	Obj->Sock->Write( Data );
}

void EGNetThread::PumpCopyMsgs( const struct egNetMsgData* Data , void* UserPointer )
{
	EGNetMsgBuffer* InBuffer = static_cast<EGNetMsgBuffer*>(UserPointer);
	InBuffer->AddMsg( Data );
}

void EGNetThread::OnStop()
{
	
}

void EGNetThread::AddNetObj( INetSocket* Sock , const egNetCommId& Id , EGNetMsgBuffer& RecBuffer , EGNetMsgBuffer& SendBuffer )
{
	EGFunctionLock Lock( &m_AddRemoveLock );

	egNetObj* NewObj = nullptr;
	for( eg_uint i=0; i<countof(m_Objs) && nullptr == NewObj; i++ )
	{
		if( nullptr == m_Objs[i]->Sock )
		{
			NewObj = m_Objs[i];
		}
	}

	if( NewObj )
	{
		NewObj->NetId = Id;
		NewObj->Sock  = Sock;
		NewObj->Rec   = &RecBuffer;
		NewObj->Send  = &SendBuffer;
	}
	else
	{
		assert( false ); //Should be a max of the number of clients.
	}
}

void EGNetThread::RemoveNetObj( INetSocket* Sock )
{
	EGFunctionLock Lock( &m_AddRemoveLock );

	egNetObj* RemObj = nullptr;
	for( eg_uint i=0; i<countof(m_Objs) && nullptr == RemObj; i++ )
	{
		if( Sock == m_Objs[i]->Sock )
		{
			RemObj = m_Objs[i];
		}
	}

	if( RemObj )
	{
		RemObj->NetId = NET_ID_NOT_CONNECTED;
		RemObj->Sock  = nullptr;
		RemObj->Rec   = nullptr;
		RemObj->Send  = nullptr;
	}
	else
	{
		assert( false ); // Net obj was never added.
	}
}
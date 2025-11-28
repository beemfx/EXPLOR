// EGNetMsgBuffer - A buffer that stores information to be transmitted.
// (c) 2015 Beem Software
#pragma once
#include "EGNetMsgs.h"
#include "EGList.h"
#include "EGMutex.h"

typedef void ( * FnNetMsgCallback )( const struct egNetMsgData* Data , void* UserPointer );

class EGNetMsgBuffer
{
public:
	EGNetMsgBuffer();
	~EGNetMsgBuffer();
	void AddMsg( const egNetMsgData* Data );
	void AddStrMsg( const struct egNetCommId& DestNetId , eg_cpstr String );
	void AddNoParmMsg( const struct egNetCommId& DestNetId , eg_net_msg Msg );

	void PumpMsgs( FnNetMsgCallback Callback , void* UserPointer );
	void FlushMsgs();

	eg_bool IsEmpty()const{ EGFunctionLock Lock( &m_Mutex ); return 0 == m_List.Len(); }
	void GetAndRemoveFirstItem( egNetMsgData* DataOut );

private:
	struct egMsg: public IListable
	{
		egNetMsgData Data;
	};

private:
	EGList<egMsg>   m_List;
	mutable EGMutex m_Mutex;
};
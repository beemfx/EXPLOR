// EGThreadRequest
// (c) 2015 Beem Software
#include "EGThreadRequest.h"
#include "EGThread.h"

EGThreadRequest::EGThreadRequest(eg_cpstr strId)
: IThreadProc()
, m_LoadItem(nullptr)
, m_LoadQue(1)
, m_LoadCompleteQue(2)
, m_SleepTimeWhenNoProc(0.f)
{
	unused( strId );
}

EGThreadRequest::~EGThreadRequest()
{

}

eg_bool EGThreadRequest::RemoveFromQueue( IRequestCallback* L , EGLoadQue* Que )
{
	egRequestData* FoundItem = nullptr;
	//Search the queues for this Loadable.
	for( egRequestData* Item : *Que )
	{
		if( L == Item->Request )
		{
			FoundItem = Item;
			break;
		}
	}

	if( nullptr != FoundItem )
	{
		Que->Remove( FoundItem );
		EG_SafeDelete( FoundItem );
		return true;
	}

	//assert( false ); //Item was not in this que.
	return false;
}

void EGThreadRequest::CancelRequest(IRequestCallback* L)
{
	//We want the CancelRequest to happen as quickly as possible. The worst case
	//is if we are currenlty loading the item we want to cancel.
	eg_bool bRemoved = false;
	eg_bool bCurrentlyLoading = false;

	//First check to see if the item is either waiting to be loaded,
	//loaded already and waiting for OnLoadComplete to be called.
	//Or in the process of being loaded.
	m_LockQue.Lock();
	{
		if(!bRemoved)
		{
			bRemoved = bRemoved || RemoveFromQueue( L, &m_LoadQue );
		}
		if(!bRemoved)
		{	
			bRemoved = bRemoved || RemoveFromQueue( L, &m_LoadCompleteQue );
		}
		if(!bRemoved)
		{
			assert( m_LoadItem && L == m_LoadItem->Request );
			assert( false ); // We don't know how to cancel, this the callee needs to handle this.
		}
		if(!bRemoved)
		{
			assert( L == m_LoadItem->Request ); //Where is this item?
		}
	}
	m_LockQue.Unlock();

	//It's possible that right here we finished the load and started loading
	//something else. In which case we actually have to wait for the next
	//loaded item to load before we actually get here.

	if(!bRemoved)
	{
		//We hit the worst case, where this object was currently being
		//loaded, or it wasn't in the que. So we just wait for the loading
		//thread to finish.
		m_LockUpdate.Lock();
		{
			bRemoved = bRemoved || RemoveFromQueue( L, &m_LoadCompleteQue );
		}
		m_LockUpdate.Unlock();
	}

	assert(bRemoved); //If we are hitting this we need to rethink the EGThread_Sleep's.
}

void EGThreadRequest::MakeRequest( IRequestCallback* L )
{
	EGFunctionLock QueLock( &m_LockQue );

	egRequestData* LD = EGMem2_New<egRequestData>( eg_mem_pool::DefaultHi );
	if( LD )
	{
		zero( LD );
		LD->Request = L;
		m_LoadQue.InsertLast(LD);
	}
	else
	{
		assert( false ); //Out of memory?
	}
}

void EGThreadRequest::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	//Do not return from this function as LOCK_UPDATEINPGRESS must end.
	eg_bool bQuedItemFound = false;
	eg_bool bHadItems = false;

	{
		EGFunctionLock LockUpdate( &m_LockUpdate );
		//First check to see if the Load que is empty, if it is, just sleep, if not
		//get the next item:
		{
			EGFunctionLock LockQu( &m_LockQue );

			bHadItems = m_LoadQue.HasItems();
			bQuedItemFound = bHadItems;
			if(bQuedItemFound)
			{
				m_LoadItem = m_LoadQue.GetFirst();
				m_LoadQue.Remove(m_LoadItem);
			}
			else if(bHadItems)
			{
				//If we didn't find an item, but it can't be loaded now,
				//just move the front item to the back so we don't stall forever
				//on an item that can't be loaded.
				m_LoadItem = m_LoadQue.GetFirst();
				m_LoadQue.Remove( m_LoadItem );
				m_LoadQue.InsertLast(m_LoadItem);
				m_LoadItem = nullptr;
			}
		}

		if(bQuedItemFound)
		{
			{
				EGFunctionLock Lock( &m_LockAction );
				m_LoadItem->Request->Action();
			}

			//Now push this to the OnLoadComplete Que
			{
				EGFunctionLock LockQu( &m_LockQue );
				m_LoadCompleteQue.InsertLast(m_LoadItem);
				m_LoadItem = nullptr;
			}
		}
	}

	if(!bQuedItemFound)
	{
		EGThread_Sleep(bHadItems ? 0.0f : m_SleepTimeWhenNoProc);
	}

	assert( nullptr == m_LoadItem );
}

void EGThreadRequest::ProcessCompletedRequests()
{
	//We only process one completed load per frame:
	egRequestData* LD = nullptr;
	eg_bool bFoundItem = false;
	m_LockQue.Lock();
	{
		bFoundItem = m_LoadCompleteQue.HasItems();
		if(bFoundItem)
		{
			LD = m_LoadCompleteQue.GetFirst();
			m_LoadCompleteQue.Remove( LD );
		}
	}
	m_LockQue.Unlock();

	if(bFoundItem)
	{
		LD->Request->Callback();
		EG_SafeDelete( LD );
	}
}

void EGThreadRequest::PurgeRequests()
{
	assert( nullptr == m_LoadItem );

	//Move everything to the complete que.
	{
		EGFunctionLock Lock( &m_LockQue );
		while( m_LoadQue.HasItems() )
		{
			auto LD = m_LoadQue.GetFirst();
			m_LoadQue.Remove( LD );
			m_LoadCompleteQue.InsertLast( LD );
		}
	}

	ProcessCompletedRequests();
}

// (c) 2016 Beem Media

#include "EGLoader.h"
#include "EGLoader_Loadable.h"
#include "EGFileData.h"
#include "EGThread.h"
#include "EGEngine.h"
#include "EGWorkerThreads.h"
#include <fs_sys2/fs_sys2.h>

EGLoader::EGLoader(eg_cpstr strId,class EGThread* OwnerThread)
: IThreadProc()
, m_LoadItem( nullptr )
, m_LoadQue( 1 )
, m_IsLoadOnOtherThreadOkay( false )
, m_Id( strId )
, m_OwnerThread( OwnerThread )
, m_LoadCount(0)
, m_SaveCount(0)
{
	for( eg_uint i=0; i<countof(m_LoadCompleteQue); i++ )
	{
		m_LoadCompleteQue[i].Init( 2 + i ); //Start at 2 cuz 1 is the load que.
	}

	OwnerThread->RegisterProc( this );
}

EGLoader::~EGLoader()
{
	m_OwnerThread->UnregisterProc( this );
	assert( 0 == m_LoadCount );
	assert( 0 == m_SaveCount );
}

eg_bool EGLoader::IsCurrentThread()const
{
	return m_OwnerThread->IsCurrentThread();
}

eg_bool EGLoader::IsLoading()const
{
	//EGFunctionLock Lock( &m_LockLoadState ); // May not need to lock since we will only read this and if it's wrong it will only be wrong for a frame or so.
	eg_bool IsLoading = m_LoadCount > 0;
	return IsLoading;
}
eg_bool EGLoader::IsSaving()const
{
	//EGFunctionLock Lock( &m_LockLoadState ); // May not need to lock since we will only read this and if it's wrong it will only be wrong for a frame or so.
	eg_bool IsSaving = m_SaveCount > 0;
	return IsSaving;
}

eg_bool EGLoader::CancelLoad_RemoveFromQueue( ILoadable* L , EGLoadQue* Que )
{
	egLoadData* FoundItem = nullptr;
	//Search the queues for this Loadable.
	for( egLoadData* Item : *Que )
	{
		if( L == Item->pLoadable )
		{
			FoundItem = Item;
			break;
		}
	}

	if( nullptr != FoundItem )
	{
		Que->Remove( FoundItem );
		assert( !FoundItem->IsSave );
		EG_SafeDelete( FoundItem );
		return true;
	}

	//assert( false ); //Item was not in this que.
	return false;
}

void EGLoader::CancelLoad(ILoadable* L)
{
	//We want the CancelLoad to happen as quickly as possible. The worst case
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
			bRemoved = bRemoved || CancelLoad_RemoveFromQueue( L, &m_LoadQue );
		}
		for( eg_uint LoadThreadId = 0; LoadThreadId < LOAD_THREAD_COUNT; LoadThreadId++ )
		{
			if(!bRemoved)
			{	
				bRemoved = bRemoved || CancelLoad_RemoveFromQueue( L, &m_LoadCompleteQue[LoadThreadId] );
			}
		}
		if(!bRemoved)
		{
			assert( m_LoadItem && L == m_LoadItem->pLoadable ); //Where is this item?
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
			for( eg_uint LoadThreadId = 0; LoadThreadId < LOAD_THREAD_COUNT; LoadThreadId++ )
			{
				bRemoved = bRemoved || CancelLoad_RemoveFromQueue( L, &m_LoadCompleteQue[LoadThreadId] );
			}
		}
		m_LockUpdate.Unlock();
	}

	if( bRemoved )
	{
		DecLoadCount(); // Can't cancel a save.
	}

	assert(bRemoved); //If we are hitting this we need to rethink the EGThread_Sleep's.
}

void EGLoader::BeginLoad( eg_cpstr strFile , ILoadable* L , LOAD_THREAD_ID LoadThreadId )
{
	EGFunctionLock QueLock( &m_LockQue );
	egLoadData* LD = new egLoadData( L , strFile , LoadThreadId );
	IncLoadCount();
	m_LoadQue.InsertLast( LD );
}

void EGLoader::LoadNow(eg_cpstr strFile, ILoadable* L)
{
	IncLoadCount();
	eg_string sFile(strFile);
	
	{
		EGFunctionLock Lock( &m_LockDiskAccess );
		EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
		LoadNowTo( strFile , MemFile );
		L->DoLoad( strFile , MemFile.GetDataAs<eg_byte>() , MemFile.GetSize() );
	}

	L->OnLoadComplete(sFile);
	DecLoadCount();
}

void EGLoader::LoadNowTo( eg_cpstr Filename, EGFileData& File )
{
	IncLoadCount();
	EGFunctionLock Lock( &m_LockDiskAccess );
	eg_string sFile( Filename );
	assert( m_IsLoadOnOtherThreadOkay || IsCurrentThread() ); //We are at a point in the game where all loads should happen on the loading thread.
	LF_FILE3 fin = LF_Open( EGString_ToWide( sFile ), LF_ACCESS_READ | LF_ACCESS_MEMORY, LF_OPEN_EXISTING );

	if( fin )
	{
		File.Write( ::LF_GetMemPointer( fin ), LF_GetSize( fin ) );
		LF_Close( fin );
	}
	else
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": Failed to load \"%s\".", sFile.String() );
	}
	DecLoadCount();
}

void EGLoader::SaveData(eg_cpstr strFile, const eg_byte* pMem, const eg_size_t Size)
{
	EGFunctionLock QueLoack( &m_LockQue );
	egLoadData* LD = new egLoadData( static_cast<const void*>(pMem) , Size , strFile );
	IncSaveCount();
	if( LD )
	{
		m_LoadQue.InsertLast( LD );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to save \"%s\"." , strFile );
		assert( false );
	}
}

void EGLoader::WaitForAllLoads()
{
	assert( EGWorkerThreads_IsThisMainThread() );
	eg_bool HasLoads = true;

	while( HasLoads )
	{
		{
			EGFunctionLock QueLock( &m_LockQue );
			HasLoads = false;
			HasLoads = HasLoads || m_LoadQue.Len() > 0;
			HasLoads = HasLoads || nullptr != m_LoadItem;
			for( eg_uint i=0; i<countof(m_LoadCompleteQue); i++ )
			{
				HasLoads = HasLoads || m_LoadCompleteQue[i].Len() > 0;
			}
		}
		ProcessCompletedLoads( LOAD_THREAD_MAIN );
		EGThread_Sleep( 0 );
	}
}

void EGLoader::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	//Do not return from this function as LOCK_UPDATEINPGRESS must end.

	eg_int NumRequestsProcessed = 0;
	eg_bool bContinueProcessing = true;
	static const eg_int MAX_REQUESTS_PER_TICK = 1000;
	for( eg_int i=0; i<MAX_REQUESTS_PER_TICK && bContinueProcessing; i++ )
	{
		m_LockUpdate.Lock();
		bContinueProcessing = HandleNextRequest();
		m_LockUpdate.Unlock();

		if( bContinueProcessing )
		{
			NumRequestsProcessed++;
		}
	}

	if( NumRequestsProcessed )
	{
		// EGLogf( eg_log_t::Verbose , "Processed %d loads." , NumRequestsProcessed );
	}
}

eg_bool EGLoader::HandleNextRequest()
{
	eg_bool bQuedItemFound = false;
	eg_bool bHadItems = false;

	//First check to see if the Load queue is empty, if it is, just sleep, if not
	//get the next item:
	m_LockQue.Lock();
	{
		bHadItems = m_LoadQue.Len() > 0;
		bQuedItemFound = bHadItems && ( m_LoadQue.GetFirst()->IsSave || m_LoadQue.GetFirst()->pLoadable->CanLoadNow() );
		if(bQuedItemFound)
		{
			m_LoadItem = m_LoadQue.GetFirst();
			m_LoadQue.Remove( m_LoadItem );
		}
		else if(bHadItems)
		{
			//If we didn't find an item, but it can't be loaded now,
			//just move the front item to the back so we don't stall forever
			//on an item that can't be loaded.
			m_LoadItem = m_LoadQue.GetFirst();
			m_LoadQue.Remove( m_LoadItem );
			m_LoadItem = nullptr;
		}
	}
	m_LockQue.Unlock();

	if(bQuedItemFound)
	{
		assert_pointer( m_LoadItem );
		{
			EGFunctionLock Lock( &m_LockDiskAccess );
			//EGLogf( LOG_GENERNAL , ("Begin loading \"%s\"."), m_LoadItem.strFile.String());

			if( m_LoadItem->IsSave )
			{
				LF_FILE3 fin = LF_Open( EGString_ToWide(m_LoadItem->strFile) , LF_ACCESS_WRITE , LF_CREATE_ALWAYS );
				assert_pointer(fin);
				if(fin)
				{
					eg_uint nWritten = LF_Write(fin, m_LoadItem->SaveData, static_cast<fs_dword>(m_LoadItem->SaveDataSize));
					assert(nWritten == m_LoadItem->SaveDataSize);
					LF_Close(fin);
				}
			}
			else
			{
				LF_FILE3 fin = LF_Open( EGString_ToWide(m_LoadItem->strFile) , LF_ACCESS_READ|LF_ACCESS_MEMORY , LF_OPEN_EXISTING );
				if(fin)
				{
					m_LoadItem->pLoadable->DoLoad(m_LoadItem->strFile, static_cast<const eg_byte*>(::LF_GetMemPointer(fin)), LF_GetSize(fin));
					LF_Close(fin);
				}
				else
				{
					EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to load \"%s\"." , m_LoadItem->strFile.String() );
					m_LoadItem->pLoadable->DoLoad(m_LoadItem->strFile, nullptr, 0);
				}
			}
		}

		//Now push this to the OnLoadComplete Que
		m_LockQue.Lock();
		{
			assert( m_LoadItem->LoadThreadId < LOAD_THREAD_COUNT );
			m_LoadCompleteQue[m_LoadItem->LoadThreadId].InsertLast(m_LoadItem);
			m_LoadItem = nullptr;
		}
		m_LockQue.Unlock();
	}

	return bHadItems;
}

void EGLoader::ProcessCompletedLoads( LOAD_THREAD_ID ThreadId )
{
	if( ThreadId > LOAD_THREAD_COUNT )
	{
		assert( false );
		return;
	}
	//We only process one completed load per frame:
	egLoadData* LD = nullptr;
	eg_bool bFoundItem = false;
	m_LockQue.Lock();
	{
		bFoundItem = m_LoadCompleteQue[ThreadId].Len() > 0;
		if(bFoundItem)
		{
			LD = m_LoadCompleteQue[ThreadId].GetFirst();
			m_LoadCompleteQue[ThreadId].Remove( LD );
		}
	}
	m_LockQue.Unlock();

	//While it does look like that becuase the Loadable is no longer in a que
	//CancelLoad could potentially be called while in this phase, CancelLoad
	//should properly be called only from the same thread that this function
	//is called from anyway. The only place where that might not potentially
	//happen is in the render thread, but the render thread is locked within
	//asset locks anyway (though, I admit being that they are separate modules
	//the render thread sould prolly be better designed to address this.)

	if(bFoundItem)
	{
		if( LD->IsSave )
		{
			//Nothing to do on a save completed.
			DecSaveCount();
		}
		else
		{
			LD->pLoadable->OnLoadComplete(LD->strFile);
			DecLoadCount();
		}
		EG_SafeDelete( LD );
	}
}

// (c) 2016 Beem Media

/******************************************************************************
EGLoader - Loads assets on the loading thread.

Specifications:
An asset may be loaded on the calling thread by calling LoadNow instead of
BeginLoad. This will wait for the next available opportunity to Load an asset
load it and then when the function returns the asset will be completely loaded.

The typical behavior is to call BeginLoad. This will queue the asset to be
loaded, when it is the assets turn to be loaded DoLoad is called on the loading
thread, and then when the load is complete OnLoadComplete is called on the 
calling thread. At which point the object may assume to be loaded.
******************************************************************************/

#pragma once

#include "EGThreadProc.h"
#include "EGList.h"

class EGFileData;
class ILoadable;

class EGLoader: public IThreadProc
{
public:
	enum LOAD_THREAD_ID
	{
		LOAD_THREAD_MAIN,
		LOAD_THREAD_RENDER,
		LOAD_THREAD_SOUND,
		LOAD_THREAD_SERVER,
		LOAD_THREAD_COUNT,
	};

public:
	EGLoader(eg_cpstr strId,class EGThread* OwnerThread);
	~EGLoader();

public:
	void BeginLoad( eg_cpstr strFile , ILoadable* L , LOAD_THREAD_ID LoadThreadId );
	//LoadNow will have loaded the asset when the call completes, does not use
	//the loading thread at all, but it will wait for the currenlty loading
	//asset to load before completing, so this may cause a stall. Should be
	//used rarely on the main thread.
	void LoadNow(eg_cpstr strFile, ILoadable* L);
	void LoadNowTo( eg_cpstr Filename , EGFileData& File );

	//CancelLoad will remove the loadable from whatever que it is in, if it
	//is currently being loaded, the DoLoad will still be called so the
	//ILoadable should respond appropriately to that. When this function returns
	//the load will have been canceled. It occurs on the calling thread.
	void CancelLoad(ILoadable* L);

	void SaveData(eg_cpstr strFile, const eg_byte* pMem, const eg_size_t Size);

	//Called only within the thread that owns the loader:
	void ProcessCompletedLoads( LOAD_THREAD_ID ThreadId );

	void WaitForAllLoads();
	void SetIsLoadOnOtherThreadOkay( eg_bool Okay )
	{
		m_IsLoadOnOtherThreadOkay = Okay; 
	}

	eg_bool IsCurrentThread()const;
	eg_bool IsLoading()const; // True if anything is loading or queued to be loaded.
	eg_bool IsSaving()const; // True if anything is saving or queued to be saved.
private:
	virtual void OnStart() override{};
	virtual void Update( eg_real DeltaTime );
	virtual void OnStop() override{};
	virtual void OnThreadMsg(eg_cpstr /*strParm*/) override{assert(false);};

	void IncLoadCount()
	{
		EGFunctionLock Lock( &m_LockLoadState );
		m_LoadCount++;
	}

	void DecLoadCount()
	{
		EGFunctionLock Lock( &m_LockLoadState );
		m_LoadCount--;
		assert( m_LoadCount >= 0 );
	}

	void IncSaveCount()
	{
		EGFunctionLock Lock( &m_LockLoadState );
		m_SaveCount++;
	}

	void DecSaveCount()
	{
		EGFunctionLock Lock( &m_LockLoadState );
		m_SaveCount--;
		assert( m_SaveCount >= 0 );
	}

	eg_bool HandleNextRequest();

private:

	struct egLoadData: public IListable
	{
		ILoadable*const      pLoadable;
		void*                SaveData;
		eg_size_t            SaveDataSize;
		const eg_string      strFile;
		const LOAD_THREAD_ID LoadThreadId;
		const eg_bool        IsSave:1;

		egLoadData()
		: IListable()
		, pLoadable( nullptr )
		, SaveData( nullptr )
		, SaveDataSize( 0 )
		, strFile( "" )
		, LoadThreadId( LOAD_THREAD_MAIN )
		, IsSave( false )
		{
			assert( false );
		}

		egLoadData( ILoadable* _Loadable , eg_cpstr _Filename , LOAD_THREAD_ID _LoadThreadId )
		: IListable()
		, pLoadable( _Loadable )
		, SaveData( nullptr )
		, SaveDataSize( 0 )
		, strFile( _Filename )
		, LoadThreadId( _LoadThreadId )
		, IsSave( false )
		{

		}

		egLoadData( const void* _SaveData , eg_size_t _SaveDataSize , eg_cpstr _Filename )
		: IListable()
		, pLoadable( nullptr )
		, SaveData( nullptr )
		, SaveDataSize( 0 )
		, strFile( _Filename )
		, LoadThreadId( LOAD_THREAD_MAIN )
		, IsSave( true )
		{
			SaveData = EGMem2_Alloc( _SaveDataSize , eg_mem_pool::DefaultHi );
			if( nullptr != SaveData )
			{
				SaveDataSize = _SaveDataSize;
				EGMem_Copy( SaveData , _SaveData , _SaveDataSize );
			}
		}

		~egLoadData()
		{
			if( SaveData )
			{
				EGMem2_Free( SaveData );
				SaveData = nullptr;
			}
		}
	};

	typedef EGList<egLoadData> EGLoadQue;

	mutable EGMutex m_LockDiskAccess;
	mutable EGMutex m_LockQue;
	mutable EGMutex m_LockUpdate;
	mutable EGMutex m_LockLoadState;

	//Every item is always in one of these queues at all times.
	EGLoadQue   m_LoadQue;
	egLoadData* m_LoadItem;
	EGLoadQue   m_LoadCompleteQue[LOAD_THREAD_COUNT];

	eg_int      m_LoadCount;
	eg_int      m_SaveCount;

	eg_string   m_Id;
	EGThread*   m_OwnerThread;
	eg_bool     m_IsLoadOnOtherThreadOkay:1;

private:
	static eg_bool CancelLoad_RemoveFromQueue( ILoadable* L , EGLoadQue* Que );
};

extern class EGLoader* MainLoader;

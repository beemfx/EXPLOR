// (c) 2011 Beem Media

#include "EGSkel.h"
#include "EGWorkerThreads.h"
#include "EGLoader.h"

EGSkel::EGSkel( eg_cpstr Filename )
: EGSkelBase()
, m_LoadState( LOAD_NOT_LOADED )
{
	LoadFromBinaryOnThread(Filename);
}

EGSkel::~EGSkel()
{
	Unload();
}

void EGSkel::LoadFromBinaryOnThread(eg_cpstr strFile)
{
	assert( EGWorkerThreads_IsThisMainThread() );
	Unload();
	m_LoadState = LOAD_LOADING;
	MainLoader->BeginLoad(strFile, this, EGLoader::LOAD_THREAD_MAIN);
}

void EGSkel::DoLoad(eg_cpstr /*strFile*/, const eg_byte*const  pMem, const eg_size_t Size)
{
	assert( nullptr == m_pMem );
	assert( LOAD_LOADING == m_LoadState );

	if( nullptr == pMem || 0 == Size )
	{
		zero( &m_H );
		return;
	}

	m_nMemSize = Size;
	m_pMem = EGMem2_NewArray<eg_byte>(Size, eg_mem_pool::Default);
	if( nullptr != m_pMem )
	{
		EGMem_Copy(m_pMem, pMem, m_nMemSize);

		m_pH = (egHeader*)&m_pMem[0];
		if(!(SKEL_ID == m_pH->nID && SKEL_VERSION == m_pH->nVersion))
		{
			assert( false ); //Not a valid skel?
			EGMem2_Free(m_pMem);
			m_pMem = nullptr;
			return;
		}
	}
	else
	{
		assert( false ); //Out of memory.
		zero( &m_H );
	}
}

void EGSkel::OnLoadComplete(eg_cpstr strFile)
{
	unused( strFile );
	assert( 0 == m_H.nID && 0 == m_H.nVersion ); //m_H should only be set in this function and not on the loading thread, since IsLoaded is based on these values.

	m_LoadState = LOAD_LOADED;

	if( nullptr != m_pMem )
	{
		SetPointers(*m_pH);
	}
	else
	{
		zero( &m_H );
	}

	OnLoaded.Broadcast();
}

void EGSkel::Unload()
{
	if( LOAD_LOADING == m_LoadState )
	{
		MainLoader->CancelLoad(this);
	}

	DeallocateMemory();
	m_LoadState = LOAD_NOT_LOADED;
	zero(&m_H);
}

void EGSkel::DeallocateMemory()
{
	//Only have to delete the lump of data,
	//then set everything else to null.
	if(m_pMem)
	{
		EGMem2_Free(m_pMem);
		m_pMem = nullptr;
	}
	m_nMemSize=0;

	#define SKEL_DATA_SECTION( _type_ , _var_ ) m_p##_var_ = nullptr;
	#include "EGSkelDataSections.inc"
	#undef SKEL_DATA_SECTION
}

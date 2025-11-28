// (c) 2014 Beem Media

#include "EGMap_PhysLoader.h"
#include "EGBpPhysLib.h"
#include "EGLoader.h"

void EGMap_PhysLoader::BeginLoad( eg_cpstr strFile )
{
	eg_string PhysFile = strFile;
	#if defined( __WIN32__ )
	PhysFile.Append( ".emapphysx32" );
	#elif defined( __WIN64__ )
	PhysFile.Append( ".emapphysx64" );
	#else
	#Error Platform?
	#endif

	assert( STATUS_NOT_LOADED == m_State );
	m_State = STATUS_LOADING;
	MainLoader->BeginLoad( PhysFile , this , EGLoader::LOAD_THREAD_MAIN);
}

void EGMap_PhysLoader::LoadNow( eg_cpstr strFile )
{
	eg_string PhysFile = strFile;
	#if defined( __WIN32__ )
	PhysFile.Append( ".emapphysx32" );
	#elif defined( __WIN64__ )
	PhysFile.Append( ".emapphysx64" );
	#else
	#Error Platform?
	#endif

	assert( STATUS_NOT_LOADED == m_State );
	m_State = STATUS_LOADING;
	MainLoader->LoadNow( PhysFile , this );
	assert( STATUS_LOADED == m_State );
}

void EGMap_PhysLoader::Clear()
{
	if( STATUS_LOADING == m_State )
	{
		MainLoader->CancelLoad( this );
	}

	if( m_Chunk )
	{
		EGMem2_Free( m_Chunk );
		m_Chunk = nullptr;
		m_AlignedChunk = nullptr;
		m_Size = 0;
	}
	m_State = STATUS_NOT_LOADED;
}

void EGMap_PhysLoader::DoLoad(eg_cpstr /*strFile*/, const eg_byte*const  pMem, const eg_size_t Size)
{
	//We want this memory 128-bit aligned. So we allocate just a little extra.
	assert( 0 == m_Size && nullptr == m_Chunk );
	m_Size = Size;
	m_Chunk = EGMem2_NewArray<eg_byte>( Size , eg_mem_pool::Default );
	eg_uintptr_t Address = reinterpret_cast<eg_uintptr_t>(m_Chunk);
	eg_uintptr_t AlignedAddress = EG_AlignUp( Address , 16 );
	eg_uintptr_t Offset = AlignedAddress - Address;
	assert( Offset <= 16 );

	m_AlignedChunk = &m_Chunk[Offset];

	EGMem_Copy( m_AlignedChunk , pMem , Size );
}

void EGMap_PhysLoader::OnLoadComplete(eg_cpstr /*strFile*/)
{
	m_State = STATUS_LOADED;
}

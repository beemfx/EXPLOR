// (c) 2017 Beem Media

#include "EGPhysMemMgr.h"

EGPhysMemMgr EGPhysMemMgr::s_Inst;

void EGPhysMemMgr::Init()
{

}

void EGPhysMemMgr::Deinit()
{
	for( egChunk& Chunk : m_Chunks )
	{
		assert( !Chunk.bUsed && Chunk.Data && Chunk.DataSize == EG_PHYSICS_SIM_MEM_SIZE );
		EGMem2_Free( Chunk.Data );
	}

	m_Chunks.Clear();
}

EGPhysMemMgr::egChunk EGPhysMemMgr::GetChunk()
{
	// Search to see if a chunk is available.
	for( egChunk& Chunk : m_Chunks )
	{
		if( !Chunk.bUsed )
		{
			assert( Chunk.Data && Chunk.DataSize == EG_PHYSICS_SIM_MEM_SIZE );
			Chunk.bUsed = true;
			return Chunk;
		}
	}

	// If we made it here no chunk was avaiable so attempt to create a new one.
	egChunk NewChunk;

	if( m_Chunks.IsFull() )
	{
		EGLogf( eg_log_t::Error , "Too many physics chunks were allocated." );
		return NewChunk;
	}

	NewChunk.bUsed = true;
	NewChunk.DataSize = EG_PHYSICS_SIM_MEM_SIZE;
	NewChunk.Data = EGMem2_Alloc( NewChunk.DataSize , eg_mem_pool::System );
	if( NewChunk.Data )
	{
		m_Chunks.Append( NewChunk );
	}
	else
	{
		EGLogf( eg_log_t::Error , "Not enough memory to allocate a physics chunk." );
		assert( false ); // Not enough memory for physics chunk.
		NewChunk.DataSize = 0;
		NewChunk.bUsed = false;
	}

	return NewChunk;
}

void EGPhysMemMgr::ReleaseChunk( void* Data )
{
	for( egChunk& Chunk : m_Chunks )
	{
		if( Chunk.Data == Data )
		{
			assert( Chunk.bUsed );
			Chunk.bUsed = false;
		}
	}
}

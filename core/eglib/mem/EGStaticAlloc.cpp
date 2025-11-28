/******************************************************************************
File: StaticAlloc.cpp
Class: EGStaticAlloc

Allocations are not gauranteed to be aligned, but the allocator will assert if
it creates an allocation that is not aligned.

Allocation time is O(1)

StaticAlloc's are the only allocators that actually get memory from the system.
All other memory is either actual static memory that is part of the executable
or allocated from one of the allocators.

Static allocators are meant to be the device that get's memory for other
allocators or for objects that exist throughout the life of the application
(such as the Game, the Server, and Client). All dynamic memory originates
from a static allocator.

Memory obtained through Static allocators is not actually freed until the
game terminates.

(c) 2012 Beem Software
******************************************************************************/
#include "EGStaticAlloc.h"

void EGStaticAlloc::Init( void* Mem , eg_size_t Size , eg_uint Alignment )
{
	m_pChunk = static_cast<eg_byte*>(Mem);
	CHUNK_SIZE = Size;
	m_Alignment = Alignment;
}

void EGStaticAlloc::Deinit( void )
{
	m_pChunk = nullptr;
	CHUNK_SIZE = 0;
}

eg_size_t EGStaticAlloc::GetInfo(INFO_T Type)const
{
	switch(Type)
	{
		case INFO_ALLOC_BLOCKS: return this->m_nNumBlocks;
		case INFO_MAX_BLOCKS  : return 0;
		case INFO_ALLOC_MEM   : return this->m_nNextAllocOffset;
		case INFO_MAX_MEM     : return this->CHUNK_SIZE;
		case INFO_FRAGMENTS   : return 0;
		case INFO_LARGEST_ALLOC: return m_nLargestAlloc;
		default: assert(false); return -1; //Not implemented, please implement.
	}
}

void* EGStaticAlloc::Alloc(eg_size_t nSize, const eg_char8*const /*strType*/, const eg_char8*const /*strFile*/, const eg_uint /*nLine*/)
{
	assert((m_nNextAllocOffset+nSize) <= CHUNK_SIZE);
	if((m_nNextAllocOffset + nSize) > CHUNK_SIZE)return nullptr;

	if( 0 == m_Alignment )
	{
		assert((nSize%EG_ALIGNMENT) == 0); //It is highly recommended that all static alignments are aligned to the game's bit alignment, otherwise future alignments will not be aligned.
	}

	m_nLargestAlloc = EG_Max(m_nLargestAlloc, nSize);

	eg_size_t nOffset = m_nNextAllocOffset;
	m_nNextAllocOffset += 0 == m_Alignment ? nSize : EG_AlignUp( nSize , m_Alignment );
	m_nNumBlocks++;
	assert((reinterpret_cast<eg_uintptr_t>(&m_pChunk[nOffset])%(m_Alignment!=0?m_Alignment:EG_ALIGNMENT)) == 0); //This data was not aligned, probably due to the previous allocation getting the above assert.
	return &m_pChunk[nOffset];
}

void* EGStaticAlloc::AllocAligned(eg_size_t Size, eg_size_t Alignment , const eg_char8* strType, const eg_char8* strFile, eg_uint nLine)
{
	void* Out = Alloc( EG_AlignUp(Size+Alignment) , strType , strFile , nLine );
	eg_uintptr_t Addr = reinterpret_cast<eg_uintptr_t>(Out);
	//We can only do this because the free for this (below) just decrements a number:
	Addr = EG_AlignUp( Addr , static_cast<eg_uint>(Alignment) );
	Out = reinterpret_cast<void*>(Addr);
	return Out;
}

void  EGStaticAlloc::Free(void* /*pMem*/)
{
	//The static allocator doesn't actually free anything.
	assert(m_nNumBlocks > 0);
	m_nNumBlocks--;
	if( 0 == m_nNumBlocks )
	{
		m_nNextAllocOffset = 0;
	}
}
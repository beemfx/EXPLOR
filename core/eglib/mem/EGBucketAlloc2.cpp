/******************************************************************************
File: BucketAlloc.cpp
Class: EGBucketAlloc

Allocations are aligned to EG_ALIGNMENT (16).

Alloc time is O(1)
Free  time is O(1)

The actual chunk allocated is the same size for every allocation made, that size
is BucketSize.

Bucket allocations never get fragmented. They are ideal for allocating memory
for objects that either are, or about the same size, or at least have a known
maximum size and a known number of those objects will be allocated at any given
time.

For example the Menu system uses these allocations to allocate menus, which are
all about the same size (assets, such as textures, used in the drawing the menu
are loaded in their respecive memory managers, but the data for the menu is
allocated here) then menus can quickly be swaped out, and only a limited number
of them exist at any given time.

Another possible use is world geometry being streamed into potential heaps, as
long as the world geometry chunks are all about the same size, but with a
terrain renderer, this may be likely.

BucketAlloc get's it's memory from the StaticHeap.

(c) 2012 Beem Software
******************************************************************************/
#include "EGBucketAlloc2.h"

///////////////////////////////////////////////////////////////////////////////
//
// EGBucketAlloc2
//
///////////////////////////////////////////////////////////////////////////////


void EGBucketAlloc2::Init( void* Mem , eg_size_t MemSize , eg_size_t BucketSize )
{
	//It is highly recommended (in fact enforced by these asserts), that you choose values that will create aligned buckets.
	assert_aligned(Mem);
	assert_aligned(BucketSize);

	m_MemAddr = reinterpret_cast<eg_uintptr_t>(Mem);
	eg_uintptr_t MemEnd = m_MemAddr + MemSize;
	m_MemSize = MemSize;
	m_BucketSize = BucketSize;
	m_pChunk = Mem;

	//We need to compute the maximum number of buckets allowed.
	m_NumBuckets= static_cast<eg_uint>(MemSize/(BucketSize+sizeof(m_apBuckets[0])));
	eg_uintptr_t StackStart = m_MemAddr + m_BucketSize*m_NumBuckets;
	eg_uintptr_t StackEnd = StackStart + sizeof(m_apBuckets[0])*m_NumBuckets;
	assert( StackEnd <= MemEnd );
	//We put the stack at the end of the memory chunk, so that the buckets
	//can be aligned at the beginning of the chunk.
	m_apBuckets = reinterpret_cast<void**>(StackStart);
	eg_byte* BucketStart = reinterpret_cast<eg_byte*>(m_pChunk);

	eg_size_t WastedMem = MemEnd - StackEnd;
	WastedMem = WastedMem;

	//Now set the buckets list to the default values:
	for(eg_uint i=0; i<m_NumBuckets; i++)
	{
		m_apBuckets[i] = &BucketStart[m_BucketSize*i];
		assert_aligned( m_apBuckets[i] );
		assert( reinterpret_cast<eg_uintptr_t>(m_apBuckets[i])+m_BucketSize <= StackStart );
	}
	m_StackPos = m_NumBuckets;
}

void EGBucketAlloc2::Deinit()
{
	assert( m_StackPos == m_NumBuckets ); //Memory wasn't freed somewhere.

	m_BucketSize=0;
	m_MemSize=0;
	m_NumBuckets=0;
	m_apBuckets=nullptr;
	m_StackPos=0;
	m_pChunk=nullptr;
	m_LargestAlloc=0;
	m_MemAddr=0;
}

eg_size_t EGBucketAlloc2::GetInfo(INFO_T Type)const
{
	switch(Type)
	{
		case INFO_ALLOC_BLOCKS: return m_NumBuckets - m_StackPos;
		case INFO_MAX_BLOCKS  : return m_NumBuckets;
		case INFO_ALLOC_MEM   : return (m_NumBuckets - m_StackPos)*m_BucketSize;
		case INFO_MAX_MEM     : return m_NumBuckets*m_BucketSize;
		case INFO_FRAGMENTS   : return 0;
		case INFO_LARGEST_ALLOC: return m_LargestAlloc;
		case INFO_FREE_BLOCKS : return m_StackPos;
		case INFO_HI_ADDR     : return m_MemAddr + m_MemSize;;
		case INFO_LO_ADDR     : return m_MemAddr;
		case INFO_MAX_MEM_USED: return 0;
		case INFO_FREE_MEM    : return m_StackPos*m_BucketSize;
	}
	return 0;
}

void* EGBucketAlloc2::Alloc(eg_size_t nSize, const eg_char8*const /*strType*/, const eg_char8*const /*strFile*/, const eg_uint /*nLine*/)
{    
	m_LargestAlloc = EG_Max(m_LargestAlloc, nSize);

	if(nSize > m_BucketSize)
	{
		assert( false ); //This allocator may need bigger buckets if this allocation wasn't a mistake.
		return nullptr;
	}

	if(0 == m_StackPos)
	{
		assert( false ); //Out of memory.
		return nullptr;
	}

	//Just grab the chunk on the top of the stack:
	m_StackPos--;
	void* pOut = static_cast<void*>(m_apBuckets[m_StackPos]);
	assert_aligned(pOut);
	eg_uintptr_t Location = reinterpret_cast<eg_uintptr_t>(pOut);
	assert( m_MemAddr <= Location && Location <= (m_MemAddr+m_MemSize) );
	return pOut;
}

void EGBucketAlloc2::Free(void* Mem)
{
	eg_uintptr_t Location = reinterpret_cast<eg_uintptr_t>(Mem);
	if( !(m_MemAddr <= Location && Location <= (m_MemAddr+m_MemSize) ) )
	{
		assert( false ); //This memory block is not in this chunk.
		return;
	}

	//Just push this bucket on the top of the stack:
	assert( m_StackPos < m_NumBuckets); //The heck? we double deallocated something.
	m_apBuckets[m_StackPos] = static_cast<eg_byte*>(Mem);
	m_StackPos++;
}

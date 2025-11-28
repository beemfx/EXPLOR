/******************************************************************************
File: HeapAlloc.cpp
Class: EGHeapAlloc

Allocations are aligned to EG_ALIGNMENT (16).

Let n be the maximum number of allocations that can be made
Allocation time is worse case O(n), but if memory is freed in the reverse order
that it was allocated it will be O(1), and this is how a Heap Allocator should
ideally be used.

Free time is always O(1)

A heap allocator is a very basic memory allocator for use in the Emergence
Engine. Memory is allocated in order, with new allocations being allocated
following older ones. The newest allocation is always placed directly after
the allocation with the highest memory address.

Memory may become fragmented because of this. Fragmented memory will only
be recovered if memory after the fragment is freed.

HeapAllocs are most suitable for data that is loaded together and destroyed
together. Like before and after a level load, for example.

HeapAlloc gets it's memory from the StaticHeap.

(c) 2012 Beem Media
******************************************************************************/
#include "EGHeap2.h"

//#define EGHEAP2_IN_ORDER_FREE_LIST

///////////////////////////////////////////////////////////////////////////////
//
// EGHeap2
//
///////////////////////////////////////////////////////////////////////////////

void EGHeap2::Init( void* Mem , eg_size_t Size , eg_size_t MinAlloc , eg_uint Alignment )
{
	assert( nullptr == m_Mem ); //This heap was already initialized, is it the intent to reinit?
	
	m_Mem = nullptr;
	m_MemSize = 0;

	m_Alignment = EG_Max<eg_uint>(1,Alignment);
	m_MinAlloc = MinAlloc;
	m_Mem = Mem;
	m_MemSize = Size;
	m_MemAddr = reinterpret_cast<eg_uintptr_t>(m_Mem);
	m_MemUsedAtOnce = 0;
	m_MemAlloced = 0;

	assert_aligned_to( reinterpret_cast<eg_uintptr_t>(m_Mem) , m_Alignment ); //It is highly recommended that the memory be aligned.

	FreeAll();
}

void EGHeap2::Deinit()
{
	assert( m_AllocList.Len() == 1 ); //Memory wasn't freed somewhere.
	assert( m_FreeList.Len() == 1 );
	m_AllocList.Clear();
	m_FreeList.Clear();
	m_Mem = nullptr;
	m_MemSize = 0;
}

void EGHeap2::FreeAll()
{
	m_AllocList.Clear();
	m_FreeList.Clear();

	#if defined( __DEBUG__ )
	EGMem_Set( m_Mem , 0xcd , m_MemSize );
	#endif

	//Insert the very fist block.
	egBlock* Block = new ( m_Mem ) egBlock;

	Block->bAllocated = false;
	Block->ChunkBegin = m_MemAddr;
	Block->ChunkSize  = m_MemSize;
#if EGHEAP2_STORE_DEBUG_DATA
	Block->Type = nullptr;
	Block->File = nullptr;
	Block->Line = 0;
#endif // EGHEAP2_STORE_DEBUG_DATA
	m_MemAlloced = 0;
	m_AllocList.Insert( static_cast<egMainList*>(Block) );
	m_FreeList.Insert( static_cast<egFreeList*>(Block) );
}

eg_uintptr_t EGHeap2::GetRelativeAddress( void* Alloc )const
{
	eg_uintptr_t Addr = reinterpret_cast<eg_uintptr_t>(Alloc);
	eg_uintptr_t MemAddr = reinterpret_cast<eg_uintptr_t>(m_Mem);
	assert( MemAddr <= Addr && Addr <= (MemAddr+m_MemSize) ); //Trying to get relative address of memory that isn't in this chunk!
	return Addr-MemAddr;
}

void* EGHeap2::GetAbsoluteAddress( eg_uintptr_t Addr )const
{
	eg_uintptr_t MemAddr = reinterpret_cast<eg_uintptr_t>(m_Mem);
	eg_uintptr_t AbsAddr = MemAddr+Addr;
	assert( MemAddr <= AbsAddr && AbsAddr <= (MemAddr+m_MemSize) ); //Trying to get absolute address of memory that isn't in this chunk!
	return reinterpret_cast<void*>(AbsAddr);
}

void* EGHeap2::Alloc( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line )
{
	return AllocInternal( Size , Type , File , Line , false );
}

void* EGHeap2::AllocHigh( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line )
{
	return AllocInternal( Size , Type , File , Line , true );
}

void* EGHeap2::AllocInternal( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line , eg_bool bHigh )
{
	unused( Line , File , Type );
	void* Out = nullptr;

	if( m_AllocList.Len() == 0 )
	{
		 assert(false);//Was Init never called?
		 return nullptr;
	}

	m_BiggestAlloc = EG_Max( m_BiggestAlloc , Size );

	//We'll compute the actual size we need for this chunk. It's always a multiple of the
	//min alloc.
	eg_size_t SizeNeeded = EG_AlignUp( Size , m_Alignment ) + GetBlockSize();
	if( 0 != m_MinAlloc )
	{
		SizeNeeded /= m_MinAlloc;
		SizeNeeded ++;
		SizeNeeded *= m_MinAlloc;
		assert( 0 == (SizeNeeded%m_MinAlloc) );
	}
	assert_aligned_to( SizeNeeded , m_Alignment );

	//Linear search to first free with enough memory:

	egBlock* FoundFreeBlock = FindFreeBlock( SizeNeeded , bHigh );
	egBlock* NewBlock = nullptr;

	if( nullptr != FoundFreeBlock )
	{
		assert( reinterpret_cast<eg_size_t>(FoundFreeBlock) == FoundFreeBlock->ChunkBegin );
		
		if( bHigh )
		{
			// For a high allocation we aren't moving the free block,
			// we just make it shorter, the new block is at the end of the free
			// chunk so in the alloc list we insert it after the free block.
			eg_size_t NewBlockStart = FoundFreeBlock->ChunkBegin+FoundFreeBlock->ChunkSize-SizeNeeded;
			FoundFreeBlock->ChunkSize -= SizeNeeded;
			assert( FoundFreeBlock->ChunkSize >= sizeof(egBlock) );
			NewBlock = new ( reinterpret_cast<void*>(NewBlockStart) ) egBlock;
			NewBlock->bAllocated = true;
			NewBlock->ChunkSize = SizeNeeded;
#if EGHEAP2_STORE_DEBUG_DATA
			NewBlock->Type = Type;
			NewBlock->File = File;
			NewBlock->Line = Line;
#endif // EGHEAP2_STORE_DEBUG_DATA
			NewBlock->ChunkBegin = NewBlockStart;

			m_AllocList.InsertAfter( static_cast<egMainList*>(FoundFreeBlock) , static_cast<egMainList*>(NewBlock) );
		}
		else
		{
			// For a low alloc the free block we found will be the block
			// allocated. So we remove it from the free list. (We'll insert
			// a new empty block from the end of that chunk.)
			m_FreeList.Remove( static_cast<egFreeList*>( FoundFreeBlock ) );

			eg_uintptr_t NextStart = FoundFreeBlock->ChunkBegin + SizeNeeded;
			eg_size_t NextSize = FoundFreeBlock->ChunkSize - SizeNeeded;
			NewBlock = FoundFreeBlock;
			NewBlock->bAllocated = true;
			NewBlock->ChunkSize = SizeNeeded;
#if EGHEAP2_STORE_DEBUG_DATA
			NewBlock->Type = Type;
			NewBlock->File = File;
			NewBlock->Line = Line;
#endif // EGHEAP2_STORE_DEBUG_DATA

			egBlock* NewEmptyBlock = new ( reinterpret_cast<void*>(NextStart) ) egBlock;
			NewEmptyBlock->ChunkBegin = NextStart;
			NewEmptyBlock->ChunkSize = NextSize;
			m_AllocList.InsertAfter( static_cast<egMainList*>(FoundFreeBlock) , static_cast<egMainList*>(NewEmptyBlock) );
			//Add the new empty block to the free list.
			InsertIntoFreeList(NewEmptyBlock);
		}

		#if defined( __DEBUG__ )
		EGMem_Set( reinterpret_cast<void*>(NewBlock->ChunkBegin+GetBlockSize()) , 0xcd , NewBlock->ChunkSize - GetBlockSize() );
		#endif
		
		Out = reinterpret_cast<void*>(NewBlock->ChunkBegin + GetBlockSize());
		eg_uintptr_t Addr = reinterpret_cast<eg_uintptr_t>(Out);
		assert_aligned_to(Out,m_Alignment);

		m_MemAlloced += NewBlock->ChunkSize;
		m_MemUsedAtOnce = EG_Max( m_MemAlloced , m_MemUsedAtOnce );
		m_NumAllocs++;
	}
	else
	{
		// assert( false ); //Out of memory, or memory too fragmented for a block of this size.
	}

	return Out;
}

EGHeap2::egBlock* EGHeap2::FindFreeBlock( eg_size_t SizeNeeded , eg_bool bHigh )
{
	egBlock* FreeBlock = nullptr;
	const eg_size_t AdjSizeNeeded = SizeNeeded+GetBlockSize();

	if( bHigh ) // High allocations search from the end back.
	{
		for( egFreeList* IntBlock = m_FreeList.GetLast(); nullptr != IntBlock; IntBlock = m_FreeList.GetPrev( IntBlock ) )
		{
			egBlock* Block = static_cast<egBlock*>(IntBlock);
			if( Block->bAllocated )continue;
			if( Block->ChunkSize < (AdjSizeNeeded) ) continue;

			FreeBlock = Block;
			break;
		}
	}
	else
	{
		
		for( egFreeList* IntBlock = m_FreeList.GetFirst(); nullptr != IntBlock; IntBlock = m_FreeList.GetNext( IntBlock ) )
		{
			egBlock* Block = static_cast<egBlock*>(IntBlock);
			if( Block->bAllocated )continue;
			if( Block->ChunkSize < (AdjSizeNeeded) ) continue;

			FreeBlock = Block;
			break;
		}
	}

	return FreeBlock;
}

void EGHeap2::InsertIntoFreeList( egBlock* Block )
{
	#if !defined(EGHEAP2_IN_ORDER_FREE_LIST)
	m_FreeList.Insert( static_cast<egFreeList*>(Block) );
	#else
	eg_bool Inserted = false;

	//If there is no items in the free list it's the only item, so insert it.
	if( !Inserted && 0 == m_FreeList.Len() )
	{
		m_FreeList.Insert( static_cast<egFreeList*>(Block) );
		Inserted = true;
	}

	//If this block comes before the first block, insert it at the start
	egFreeList* PrevBlock = m_FreeList.GetFirst();
	egBlock* Prev = static_cast<egBlock*>(PrevBlock);
	if( !Inserted && Block->ChunkBegin < Prev->ChunkBegin )
	{
		m_FreeList.InsertFirst( static_cast<egFreeList*>(Block) );
		Inserted = true;
	}

	//If there was items, find the first item to appear after it, and insert the item before that item.
	for( egFreeList* CurBlock = m_FreeList.GetNext(PrevBlock); !Inserted && nullptr != CurBlock;  )
	{
		Prev = static_cast<egBlock*>(PrevBlock);
		egBlock* Cur  = static_cast<egBlock*>(CurBlock);

		assert( Prev->ChunkBegin != Cur->ChunkBegin );
		if( Prev->ChunkBegin < Block->ChunkBegin && Block->ChunkBegin < Cur->ChunkBegin )
		{
			m_FreeList.InsertAfter( PrevBlock , static_cast<egFreeList*>(Block) );
			Inserted = true;
		}

		PrevBlock = CurBlock;
		CurBlock = m_FreeList.GetNext(CurBlock);
	}

	if( !Inserted )
	{
		m_FreeList.InsertLast( static_cast<egFreeList*>(Block) );
		Inserted = true;
	}

	assert( Inserted );
	#endif
}

void EGHeap2::AssertIntegrity()
{
	#if defined( __DEBUG__ ) && defined(EGHEAP2_IN_ORDER_FREE_LIST)
	//Do a check to make sure all chunks are in order.
	{
		for( egFreeList* CurBlock : m_FreeList )
		{
			egBlock* Cur = static_cast<egBlock*>(CurBlock);
			egBlock* Next = static_cast<egBlock*>( m_FreeList.GetNext( CurBlock ) );
			if(nullptr != Next )
			{
				assert( Cur->ChunkBegin < Next->ChunkBegin );
			}
		}
	}
	#endif
}

void  EGHeap2::Free( void* Mem )
{
	AssertIntegrity();
	eg_uintptr_t Location = reinterpret_cast<eg_uintptr_t>(Mem) - GetBlockSize();
	if( !(m_MemAddr <= Location && Location <= (m_MemAddr+m_MemSize) ) )
	{
		assert( false ); //This memory block is not in this chunk.
		return;
	}
	egBlock* Block = reinterpret_cast<egBlock*>(Location);
	assert( Block->bAllocated );

	m_MemAlloced -= Block->ChunkSize;

	//There are three possibilities:
	//1) Both the prev and next blocks are free.
	//2) The previous block is free.
	//3) The next block is free.
	//4) Both the next and prev blocks are allocated.
	//5) There is only one block.
	egBlock* Prev = static_cast<egBlock*>(m_AllocList.GetPrev( static_cast<egMainList*>(Block) ));
	egBlock* Next = static_cast<egBlock*>(m_AllocList.GetNext( static_cast<egMainList*>(Block) ));

	if( nullptr != Prev && !Prev->bAllocated && nullptr != Next && !Next->bAllocated )
	{
		Prev->ChunkSize += Block->ChunkSize + Next->ChunkSize;
		Prev->bAllocated = false;
#if EGHEAP2_STORE_DEBUG_DATA
		Prev->Type = nullptr;
		Prev->File = nullptr;
		Prev->Line = 0;
#endif // EGHEAP2_STORE_DEBUG_DATA
		m_AllocList.Remove( static_cast<egMainList*>(Block) );
		m_AllocList.Remove( static_cast<egMainList*>(Next) );
		m_FreeList.Remove( static_cast<egFreeList*>(Next) );
		AssertIntegrity();
	}
	else if( nullptr != Prev && !Prev->bAllocated )
	{
		//Case 2) Combine the previous block with this one.
		Prev->ChunkSize += Block->ChunkSize;
		Prev->bAllocated = false;
#if EGHEAP2_STORE_DEBUG_DATA
		Prev->Type = nullptr;
		Prev->File = nullptr;
		Prev->Line = 0;
#endif // EGHEAP2_STORE_DEBUG_DATA
		m_AllocList.Remove( static_cast<egMainList*>(Block) );
		AssertIntegrity();
	}
	else if( nullptr != Next && !Next->bAllocated )
	{
		//Case 3) Combine this block with the next one.
		Block->ChunkSize += Next->ChunkSize;
		Block->bAllocated = false;
#if EGHEAP2_STORE_DEBUG_DATA
		Block->Type = nullptr;
		Block->File = nullptr;
		Block->Line = 0;
#endif // EGHEAP2_STORE_DEBUG_DATA
		m_AllocList.Remove( static_cast<egMainList*>(Next) );
		m_FreeList.Remove( static_cast<egFreeList*>(Next) );
		InsertIntoFreeList( Block );
		AssertIntegrity();
	}
	else if( (nullptr != Next && Next->bAllocated ) || (nullptr != Prev && Prev->bAllocated ) )
	{
		//Case 4) Convert this block to an empty block.
		Block->bAllocated = false;
#if EGHEAP2_STORE_DEBUG_DATA
		Block->Type = nullptr;
		Block->File = nullptr;
		Block->Line = 0;
#endif // EGHEAP2_STORE_DEBUG_DATA
		InsertIntoFreeList( Block );
		AssertIntegrity();
	}
	/*
	else if( nullptr == Next && nullptr == Prev )
	{
		//Case 5) Convert this block to an empty block.
		//This case doesn't occur because there is always an empty block, even if it is too small to make an allocation.
		Block->Flags = 0;
		Block->Type = nullptr;
		Block->File = nullptr;
		Block->Line = 0;
	}
	*/
	else
	{
		assert( false );//How did we have two empty blocks in a row?
	}

	m_NumAllocs--;
}

void EGHeap2::GetInfo_Iterate_CountFragments( egMainList* Item , eg_uintptr_t Data )
{
	eg_size_t* Out = reinterpret_cast<eg_size_t*>(Data);
	egBlock* Block = static_cast<egBlock*>(Item);

	if( !Block->bAllocated )
	{
		(*Out)++;
	}
}

void EGHeap2::GetInfo_Iterate_CountAllocMem( egMainList* Item , eg_uintptr_t Data )
{
	eg_size_t* Out = reinterpret_cast<eg_size_t*>(Data);
	egBlock* Block = static_cast<egBlock*>(Item);

	if( Block->bAllocated )
	{
		(*Out) += Block->ChunkSize;
	}
}

void EGHeap2::GetInfo_Iterate_CountFreeMem( egMainList* Item , eg_uintptr_t Data )
{
	eg_size_t* Out = reinterpret_cast<eg_size_t*>(Data);
	egBlock* Block = static_cast<egBlock*>(Item);

	if( !Block->bAllocated )
	{
		(*Out) += Block->ChunkSize;
	}

}

eg_size_t EGHeap2::GetInfo(INFO_T Type)const
{
	eg_size_t Out = 0;

	switch( Type )
	{
		case INFO_HI_ADDR: Out = m_MemAddr + m_MemSize; break;
		case INFO_LO_ADDR: Out = m_MemAddr; break;
		case INFO_FRAGMENTS: 
			const_cast<EGList<egMainList>*>(&m_AllocList)->IterateAll( GetInfo_Iterate_CountFragments , reinterpret_cast<eg_uintptr_t>(&Out) );
			break;
		case INFO_MAX_MEM: Out = m_MemSize; break;
		case INFO_ALLOC_MEM:
			const_cast<EGList<egMainList>*>(&m_AllocList)->IterateAll( GetInfo_Iterate_CountAllocMem , reinterpret_cast<eg_uintptr_t>(&Out) );
			assert( Out == m_MemAlloced );
			break;
		case INFO_FREE_BLOCKS:
		case INFO_MAX_BLOCKS: Out = 0; break;
		case INFO_ALLOC_BLOCKS: Out = m_NumAllocs; break;
		case INFO_LARGEST_ALLOC: Out = m_BiggestAlloc; break;
		case INFO_FREE_MEM: 
			const_cast<EGList<egMainList>*>(&m_AllocList)->IterateAll( GetInfo_Iterate_CountFreeMem , reinterpret_cast<eg_uintptr_t>(&Out) );
			break;
		case INFO_MAX_MEM_USED: Out = m_MemUsedAtOnce; break;
	}

	#if 0
	eg_uint Count = 0;
	eg_size_t TotalMem = 0;
	for( const egMainList* Block : m_AllocList )
	{
		assert( TotalMem == (Block->ChunkBegin-reinterpret_cast<eg_uintptr_t>(m_Mem)) );
		TotalMem += Block->ChunkSize;
		printf( "% 8u: " , Count );
		printf( "%016X " , Block->ChunkBegin );
		printf( "% 8u bytes " , Block->ChunkSize );
		if( Block->bAllocated )
		{
			printf( "ALLOC " );
			printf( "%s %s(%u) " , Block->Type , Block->File , Block->Line );
		}
		else
		{
			printf( "FREE " );
		}
		printf( "\n" );
		Count++;
	}

	printf( "%u bytes found.\n" , TotalMem );
	#endif
	return Out;
}

eg_size_t EGHeap2::GetBlockSize()const
{
	eg_size_t BlockSize = EG_AlignUp(sizeof(egBlock),static_cast<eg_uint>(m_Alignment));
	return BlockSize;
}

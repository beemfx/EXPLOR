// (c) 2017 Beem Media

#include "EGEngineMem.h"
#include "EGMem2.h"
#include "EGHeap2.h"
#include "EGStaticAlloc.h"
#include <malloc.h>

#define EG_ENGINE_MEM_USE_MALLOC 1

#if EG_ENGINE_MEM_USE_MALLOC

static void* EGEngineMem_AllocFn( eg_size_t Size , eg_mem_pool MemPool )
{
	unused( MemPool );

	void* Out = malloc( Size );
	/*
	if( Out )
	{
		EGMem_Set( Out , 0 , Size ); // Zero memory so binary files created in EGMake are consistent between builds.
	}
	*/
	return Out;
}

static void EGEngineMem_FreeFn( void* Chunk , eg_mem_pool MemPool )
{
	unused( MemPool );

	return free( Chunk );
}

void EGEngineMem_Init()
{
	EGMem2_Init( EGEngineMem_AllocFn , EGEngineMem_FreeFn );
}

void EGEngineMem_Deinit()
{
	EGMem2_Deinit();
}


#else

static const eg_size_t  CHUNK_MAIN_MEGS = 1024; //1 GIG

static void* EGEngineMem_AllocFn( eg_size_t Size , eg_mem_pool MemPool );
static void EGEngineMem_FreeFn( void* Chunk , eg_mem_pool MemPool );

enum class eg_emem_heap_t
{
	DEFAULT,
	BIG_ALLOCS,
	VID_MEM,

	COUNT,
};

static struct egEngineMemHeapData
{
	const eg_emem_heap_t Type;
	eg_cpstr            Name;
	const eg_size_t     BudgetSize;
	const eg_size_t     MinAllocSize;
	void*               Mem;
	eg_size_t           LoAddr;
	eg_size_t           HiAddr;
	EGHeap2*            Heap;
}
EGEngineMem_HeapData[] =
{
	{ eg_emem_heap_t::DEFAULT    , "Heap:Default"   , 255*1024*1024 , 0 /*128*/               , nullptr , 0 , 0 , nullptr },
	{ eg_emem_heap_t::BIG_ALLOCS , "Heap:BigAllocs" , 256*1024*1024 , EG_BIG_ALLOC_SIZE       , nullptr , 0 , 0 , nullptr },
	{ eg_emem_heap_t::VID_MEM    , "Heap:VidMem"    , 512*1024*1024 , 0                       , nullptr , 0 , 0 , nullptr },
};
static_assert( countof(EGEngineMem_HeapData) == static_cast<eg_size_t>(eg_emem_heap_t::COUNT) , "Missing type?" );

static struct egEngineMemHeapData& EGEngineMem_GetHeapInfo( eg_emem_heap_t Type )
{
	for( egEngineMemHeapData& Data : EGEngineMem_HeapData )
	{
		if( Data.Type == Type )
		{
			return Data;
		}
	}
	assert( false );
	return EGEngineMem_HeapData[0];
}

static struct egEngineMemPoolData
{
	const eg_mem_pool&   PoolType;
	const eg_emem_heap_t HeapType;
	eg_cpstr8            PoolName;
	const eg_bool        bHighAlloc:1;
	mutable eg_uint      NumAllocs;
}
EGEngineMem_PoolData[] =
{
	{ eg_mem_pool::Default        , eg_emem_heap_t::DEFAULT    , "Default"        , false , 0 },
	{ eg_mem_pool::DefaultHi      , eg_emem_heap_t::DEFAULT    , "Default Hi-Mem" , true  , 0 },
	{ eg_mem_pool::Entity         , eg_emem_heap_t::DEFAULT    , "Entity"         , false , 0 },
	{ eg_mem_pool::System         , eg_emem_heap_t::DEFAULT    , "System"         , false , 0 },
	{ eg_mem_pool::Audio          , eg_emem_heap_t::DEFAULT    , "Audio"          , false , 0 },
	{ eg_mem_pool::FsSystem       , eg_emem_heap_t::DEFAULT    , "File Sys"       , false , 0 },
	{ eg_mem_pool::FsTemp         , eg_emem_heap_t::DEFAULT    , "File Sys Temp"  , true  , 0 },
	{ eg_mem_pool::DefaultArray   , eg_emem_heap_t::DEFAULT    , "Default Array"  , false , 0 },
	{ eg_mem_pool::DefaultObject  , eg_emem_heap_t::DEFAULT    , "Default Object" , false , 0 },
	{ eg_mem_pool::String         , eg_emem_heap_t::DEFAULT    , "String"         , true  , 0 },
	{ eg_mem_pool::RenderResource , eg_emem_heap_t::VID_MEM    , "RenderResource" , false , 0 },
	{ eg_mem_pool::BigAllocs      , eg_emem_heap_t::BIG_ALLOCS , "Big Alloc"      , false , 0 },
	{ eg_mem_pool::BigAllocsHi    , eg_emem_heap_t::BIG_ALLOCS , "Big Alloc Hi-M" , true  , 0 },
};

static struct egEngineMemPoolData& EGEngineMem_GetPoolInfo( eg_mem_pool Pool )
{
	for( egEngineMemPoolData& Data : EGEngineMem_PoolData )
	{
		if( Data.PoolType == Pool )
		{
			return Data;
		}
	}

	return EGEngineMem_GetPoolInfo( eg_mem_pool::DefaultHi ); // If the pool asked for didn't exist, it's going to be scratch.
}

struct egEngineMemAllocItem : public IListable
{
	void*     Mem;
	eg_size_t Size;
	eg_cpstr  Reason;
};

static eg_byte*                     EGEngineMem_MainMemChunk = nullptr;
static eg_size_t                    EGEngineMem_MainMemChunkSize = 0;
static EGStaticAlloc                EGEngineMem_MainMem;
static EGHeap2                      EGEngineMem_Heaps[countof(EGEngineMem_HeapData)];
static EGMutex                      EGEngineMem_Lock;
static EGList<egEngineMemAllocItem> EGEngineMem_Allocs( EGList<egEngineMemAllocItem>::DEFAULT_ID );

static void* EGEngineMem_AllocateHeap( eg_size_t Size , eg_cpstr Reason , eg_size_t Alignment )
{
	eg_size_t SizeWanted = Size + sizeof(egEngineMemAllocItem);
	void* pOut = EGEngineMem_MainMem.AllocAligned(SizeWanted, Alignment>0?Alignment:EG_ALIGNMENT , Reason , __FILE__ , __LINE__ );
	//Tack the ILIstable on the end.
	egEngineMemAllocItem* NewListable = reinterpret_cast<egEngineMemAllocItem*>(static_cast<eg_byte*>(pOut) + Size);
	zero( NewListable );
	NewListable->Mem = pOut;
	NewListable->Size = Size;
	NewListable->Reason = Reason;
	EGEngineMem_Allocs.Insert( NewListable );
	return pOut;
}

static void EGEngineMem_DeallocateHeap( void* Mem )
{
	EGEngineMem_MainMem.Free( Mem );
}

void EGEngineMem_Init()
{
	EGFunctionLock Lock( &EGEngineMem_Lock );

	{
		const eg_size_t TargetSize = CHUNK_MAIN_MEGS*1024*1024;
		EGEngineMem_MainMemChunk = EG_To<eg_byte*>(_aligned_malloc( TargetSize , EG_ALIGNMENT ));
		if( EGEngineMem_MainMemChunk )
		{
			EGEngineMem_MainMemChunkSize = TargetSize;
		}
	}

	EGEngineMem_MainMem.Init( EGEngineMem_MainMemChunk , EGEngineMem_MainMemChunkSize , EG_ALIGNMENT );

	for( eg_uint i=0; i<countof(EGEngineMem_HeapData); i++ )
	{
		egEngineMemHeapData* Data = &EGEngineMem_HeapData[i];
		Data->Heap = &EGEngineMem_Heaps[i];
		Data->Mem = EGEngineMem_AllocateHeap( EG_AlignUp(Data->BudgetSize) , Data->Name , EG_ALIGNMENT );
		Data->Heap->Init( Data->Mem , Data->BudgetSize , Data->MinAllocSize , EG_ALIGNMENT );

		Data->LoAddr = Data->Heap->GetInfo( IMemAlloc::INFO_LO_ADDR );
		Data->HiAddr = Data->Heap->GetInfo( IMemAlloc::INFO_HI_ADDR );
	}

	//Do a check to make sure no memory allocators overlap each other.
#if defined( __DEBUG__ )

	for(eg_uint i=0; i<countof(EGEngineMem_HeapData); i++)
	{
		for(eg_uint j=i+1; j<countof(EGEngineMem_HeapData); j++)
		{
			assert( (EGEngineMem_HeapData[i].HiAddr <= EGEngineMem_HeapData[j].LoAddr) || (EGEngineMem_HeapData[i].LoAddr >= EGEngineMem_HeapData[j].HiAddr) );
		}
	}

#endif

	// Make sure we are utilizing the memory:
	const eg_size_t TotalAllocated = EGEngineMem_MainMem.GetInfo( IMemAlloc::INFO_ALLOC_MEM );
	const eg_size_t TotalAvailable = EGEngineMem_MainMemChunkSize;
	assert( TotalAllocated <= TotalAvailable );
	const eg_int WastedMemory = EG_To<eg_int>(TotalAvailable - TotalAllocated);
	const eg_real64 WastedPercentage = 1.0 - EG_To<eg_real64>(TotalAllocated)/TotalAvailable;

	assert( WastedPercentage < .001 ); // More memory should be assigned to the buckets.

	EGMem2_Init( EGEngineMem_AllocFn , EGEngineMem_FreeFn );
}

void EGEngineMem_Deinit()
{
	EGFunctionLock Lock( &EGEngineMem_Lock );

	EGMem2_Deinit();

	for( eg_uint i=0; i<countof(EGEngineMem_HeapData); i++ )
	{
		egEngineMemHeapData* Data = &EGEngineMem_HeapData[i];
		Data->Heap->Deinit();
		EGEngineMem_DeallocateHeap( Data->Mem );
		Data->Mem = nullptr;
	}

	//And delete the static manager last.
	assert( 0 == EGEngineMem_MainMem.GetInfo( IMemAlloc::INFO_ALLOC_MEM ) );
	EGEngineMem_MainMem.Deinit();

	if( EGEngineMem_MainMemChunk )
	{
		_aligned_free( EGEngineMem_MainMemChunk );
		EGEngineMem_MainMemChunk = nullptr;
	}
	EGEngineMem_MainMemChunkSize = 0;
}


static void* EGEngineMem_AllocFn( eg_size_t Size , eg_mem_pool MemPool )
{
	egEngineMemPoolData& PoolData = EGEngineMem_GetPoolInfo( MemPool );
	egEngineMemHeapData& HeapData = EGEngineMem_GetHeapInfo( PoolData.HeapType );

	eg_size_t SizeNeeded = Size;

	EGHeap2* Heap = HeapData.Heap;

	if( nullptr == Heap )
	{
		assert( false ); // EGMem2 not initialized.
		return nullptr;
	}

	void* NewAlloc = nullptr;
	if( PoolData.bHighAlloc )
	{
		NewAlloc = Heap->AllocHigh( SizeNeeded , __FUNCTION__ , __FILE__ , __LINE__ );
	}
	else
	{
		NewAlloc = Heap->Alloc( SizeNeeded , __FUNCTION__ , __FILE__ , __LINE__ );
	}
	if( NewAlloc )
	{
		PoolData.NumAllocs++;
	}
	return NewAlloc;
}

static void EGEngineMem_FreeFn( void* Chunk , eg_mem_pool MemPool )
{
	egEngineMemPoolData& PoolData = EGEngineMem_GetPoolInfo( MemPool );
	egEngineMemHeapData& HeapData = EGEngineMem_GetHeapInfo( PoolData.HeapType );
	PoolData.NumAllocs--;
	EGHeap2* Heap = HeapData.Heap;
	Heap->Free( Chunk );
}

#endif
// (c) 2017 Beem Media

#include "EGMem2.h"
#include "EGHeap2.h"
#include "EGStaticAlloc.h"

struct egAllocInfo : public IListable
{
	eg_mem_pool    Pool;
	eg_cpstr8     Ref;
	eg_size_t     Size;
	void*         AllocStart;
	void*         Data;

	egAllocInfo( eg_mem_pool InPoolId ): Pool( InPoolId ){ }
};

static EGMem2AllocFn       EGMem2_AllocFn = nullptr;
static EGMem2FreeFn        EGMem2_FreeFn = nullptr;
static EGMutex             EGMem2_Lock;
static eg_int              EGMem2_SysFreeAllowedCount = 0;
static EGList<egAllocInfo> EGMem2_MasterTracker( EGList<egAllocInfo>::DEFAULT_ID );
static eg_size_t           EGMem2_HeaderSize = EG_AlignUp( sizeof(egAllocInfo) );
static const eg_mem_pool   EGMem2_EmergencySystemMemPool(eg_crc("EmergencySystemMem"));
static eg_bool             EGMem2_bHasUsedEmergencyMemory = false;

static void* EGMem2_EmergencySystemMemAlloc( eg_size_t Size );
static void EGMem2_EmergencySystemMemFree( void* Mem );

void EGMem2_Init( EGMem2AllocFn AllocFn , EGMem2FreeFn FreeFn )
{
	assert( nullptr == EGMem2_AllocFn && nullptr == EGMem2_FreeFn );

	EGMem2_AllocFn = AllocFn;
	EGMem2_FreeFn = FreeFn;
}

void EGMem2_Deinit()
{
	assert( EGMem2_MasterTracker.IsEmpty() );
	EGMem2_AllocFn = nullptr;
	EGMem2_FreeFn = nullptr;
}

void EGMem2_SetSysFreeAllowed( eg_bool bAllowed )
{
	EGFunctionLock Lock( &EGMem2_Lock );

	if( bAllowed )
	{
		EGMem2_SysFreeAllowedCount++;
	}
	else
	{
		EGMem2_SysFreeAllowedCount--;
	}

	assert( EGMem2_SysFreeAllowedCount >= 0 );
}

eg_bool EGMem2_IsSysFreeAllowed()
{
	EGFunctionLock Lock( &EGMem2_Lock );

	return EGMem2_SysFreeAllowedCount == 0;
}

eg_bool EGMem2_HasUsedEmergencyMemory()
{
	return EGMem2_bHasUsedEmergencyMemory;
}

void* EGMem2_Alloc( eg_size_t Size, eg_mem_pool PoolId )
{
	return EGMem2_AllocA( Size , PoolId , 0 );
}

void* EGMem2_AllocA( eg_size_t Size , eg_mem_pool PoolId , eg_size_t Alignment )
{
	EGFunctionLock Lock( &EGMem2_Lock );

	if( Size >= EG_BIG_ALLOC_SIZE )
	{
		eg_bool bFoundTargetPool = false;
		
		const eg_mem_pool BigAllocPools[] =
		{
			eg_mem_pool::Default ,
			eg_mem_pool::System ,
			eg_mem_pool::DefaultObject ,
			eg_mem_pool::Entity ,
			eg_mem_pool::Audio ,
		};

		const eg_mem_pool BigAllocHiPools[] =
		{
			eg_mem_pool::DefaultHi ,
			eg_mem_pool::DefaultArray ,
			eg_mem_pool::String ,
			eg_mem_pool::FsTemp ,
		};

		const eg_mem_pool BigAllocIgnorePools[] =
		{
			eg_mem_pool::FsSystem ,
			eg_mem_pool::RenderResource ,
		};

		for( const eg_mem_pool& CompareMemPool : BigAllocPools )
		{
			if( PoolId == CompareMemPool )
			{
				PoolId = eg_mem_pool::BigAllocs;
				bFoundTargetPool = true;
				break;
			}
		}
		
		for( const eg_mem_pool& CompareMemPool : BigAllocHiPools )
		{
			if( PoolId == CompareMemPool )
			{
				PoolId = eg_mem_pool::BigAllocsHi;
				bFoundTargetPool = true;
				break;
			}
		}

		for( const eg_mem_pool& CompareMemPool : BigAllocIgnorePools )
		{
			if( PoolId == CompareMemPool )
			{
				// We don't move the pool for these things...
				bFoundTargetPool = true;
				break;
			}
		}

		assert( bFoundTargetPool ); // This pool's case isn't covered?...
	}

	assert( nullptr != EGMem2_AllocFn );

	eg_size_t SizeNeeded = Size + EGMem2_HeaderSize + Alignment;

	void* Out = nullptr;

	void* NewAlloc = EGMem2_AllocFn ? EGMem2_AllocFn( SizeNeeded , PoolId  ) : nullptr; 

	// If the real allocator ran out of memory. Allocate from emergency system memory.
	if( nullptr == NewAlloc )
	{
		EGMem2_bHasUsedEmergencyMemory = true;
		PoolId = EGMem2_EmergencySystemMemPool;
		NewAlloc = EGMem2_EmergencySystemMemAlloc( SizeNeeded );
	}

	if( NewAlloc )
	{
		eg_uintptr_t AllocStart = reinterpret_cast<eg_uintptr_t>(NewAlloc);
		eg_uintptr_t AllocEnd = AllocStart + SizeNeeded;
		eg_uintptr_t DataStart = Alignment > 0 ? EG_AlignUp( AllocStart + EGMem2_HeaderSize , Alignment ) : AllocStart+EGMem2_HeaderSize;
		eg_uintptr_t HeaderStart = DataStart - EGMem2_HeaderSize;
		assert( (DataStart+Size) <= (AllocEnd) );

		egAllocInfo* AllocInfo = new( reinterpret_cast<void*>(HeaderStart) ) egAllocInfo( PoolId );
		AllocInfo->Ref = __FUNCTION__;
		AllocInfo->Size = Size;
		AllocInfo->Data = reinterpret_cast<void*>(DataStart);
		AllocInfo->AllocStart = NewAlloc;
		Out = AllocInfo->Data;
		EGMem2_MasterTracker.Insert( AllocInfo );
	}

	assert( NewAlloc ); // Out of memory.

	return Out;
}

void EGMem2_Free( void* Mem )
{
	EGFunctionLock Lock( &EGMem2_Lock );

	assert( nullptr != EGMem2_FreeFn );

	if( Mem )
	{
		egAllocInfo* AllocInfo = reinterpret_cast<egAllocInfo*>(reinterpret_cast<eg_uintptr_t>(Mem)-EGMem2_HeaderSize);
		assert( AllocInfo->Data == Mem );
		EGMem2_MasterTracker.Remove( AllocInfo );
		if( AllocInfo->Pool == EGMem2_EmergencySystemMemPool )
		{
			EGMem2_EmergencySystemMemFree( AllocInfo->AllocStart );
		}
		else if( EGMem2_FreeFn )
		{
			EGMem2_FreeFn( AllocInfo->AllocStart , AllocInfo->Pool );
		}
	}
}

eg_size_t EGMem2_GetAllocSize( void* Mem )
{
	EGFunctionLock Lock( &EGMem2_Lock );

	eg_size_t Out = 0;

	if( Mem )
	{
		void* AllocStart = reinterpret_cast<void*>( reinterpret_cast<eg_uintptr_t>( Mem ) - EGMem2_HeaderSize );
		egAllocInfo* AllocInfo = reinterpret_cast<egAllocInfo*>( AllocStart );
		assert( AllocInfo->Data == Mem );
		if( AllocInfo->Data == Mem )
		{
			Out = AllocInfo->Size;
		}
	}

	return Out;
}


static void* EGMem2_EmergencySystemMemAlloc( eg_size_t Size )
{
	return malloc( Size );
}

static void EGMem2_EmergencySystemMemFree( void* Mem )
{
	free( Mem );
}
// (c) 2018 Beem Media

#include "EGToolMem.h"
#include "EGMem2.h"
#include <malloc.h>

static void* EGToolMem_AllocFn( eg_size_t Size , eg_mem_pool MemPool )
{
	unused( MemPool );

	void* Out = malloc( Size );
	if( Out )
	{
		EGMem_Set( Out , 0 , Size ); // Zero memory so binary files created in EGMake are consistent between builds.
	}
	return Out;
}

static void EGToolMem_FreeFn( void* Chunk , eg_mem_pool MemPool )
{
	unused( MemPool );

	return free( Chunk );
}

void EGToolMem_Init()
{
	EGMem2_Init( EGToolMem_AllocFn , EGToolMem_FreeFn );
}

void EGToolMem_Deinit()
{
	EGMem2_Deinit();
}

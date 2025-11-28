// (c) 2017 Beem Media

#include "EGMem2StdLibReplacement.h"
#include "EGMem2.h"

extern "C" void* EG_malloc(eg_size_t Size)
{
	void* Out = EGMem2_Alloc( Size , eg_mem_pool::DefaultHi );
	return Out;
}

extern "C" void* EG_calloc(eg_size_t Num, eg_size_t Size)
{
	void* Out = EGMem2_Alloc( Num*Size, eg_mem_pool::DefaultHi );
	if( Out )
	{
		EGMem_Set( Out , 0 , Num*Size);
	}
	return Out;
}

extern "C" void* EG_realloc( void* Mem , eg_size_t Size )
{
	void* Out = nullptr;
	if( nullptr == Mem )
	{
		Out = EG_malloc( Size );
	}
	else
	{
		eg_size_t OldSize = EGMem2_GetAllocSize( Mem );
		if( OldSize >= Size )
		{
			Out = Mem;
		}
		else
		{
			Out = EGMem2_Alloc( Size , eg_mem_pool::DefaultHi );
			assert( Size > OldSize );
			EGMem_Copy( Out , Mem , OldSize );
			EG_free( Mem );
		}
	}
	return Out;
}

extern "C" void EG_free(void* Mem)
{
	EGMem2_Free( Mem );
}

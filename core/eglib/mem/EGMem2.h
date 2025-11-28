// (c) 2017 Beem Media

#pragma once

typedef void* ( * EGMem2AllocFn )( eg_size_t ChunkSize , eg_mem_pool PoolId );
typedef void ( * EGMem2FreeFn )( void* Chunk , eg_mem_pool PoolId );

void EGMem2_Init( EGMem2AllocFn AllocFn , EGMem2FreeFn FreeFn );
void EGMem2_Deinit();
void EGMem2_SetSysFreeAllowed( eg_bool bAllowed );
eg_bool EGMem2_IsSysFreeAllowed();
eg_bool EGMem2_HasUsedEmergencyMemory();

void* EGMem2_Alloc(eg_size_t Size , eg_mem_pool PoolId );
void* EGMem2_AllocA( eg_size_t Size , eg_mem_pool PoolId , eg_size_t Alignment );
void  EGMem2_Free( void* Mem );
eg_size_t EGMem2_GetAllocSize( void* Mem );

template<typename T> static inline T* EGMem2_New( eg_mem_pool PoolId )
{
	return reinterpret_cast<T*>( EGMem2_Alloc( sizeof(T) , PoolId ) );
}

template<typename T> static inline T* EGMem2_NewArray( eg_size_t Count , eg_mem_pool PoolId )
{
	return reinterpret_cast<T*>( EGMem2_Alloc( sizeof(T)*Count , PoolId ) );
}

template<typename T> static inline T* EGMem2_NewArrayA( eg_size_t Count , eg_mem_pool PoolId , eg_size_t Alignment )
{
	return reinterpret_cast<T*>( EGMem2_AllocA( sizeof(T)*Count , PoolId , Alignment ) );
}

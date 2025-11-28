// (c) 2017 Beem Media

#include "EGMemNew.h"

///////////////////////////////////////////////////////////////////////////////
//
// new replacement:
//
///////////////////////////////////////////////////////////////////////////////

#include <new>

void* __CRTDECL operator new[] ( size_t _Size , const std::nothrow_t& );
void* operator new      ( size_t size );
void  operator delete   ( void* p );
void* operator new[]    ( size_t size );
void  operator delete[] ( void* p );

void* operator new ( size_t size )
{
	return EGMem2_Alloc( size , eg_mem_pool::DefaultHi );
}

void operator delete ( void* p )
{	
	EGMem2_Free(p);
}

void* __CRTDECL operator new [] ( eg_size_t _Size , const std::nothrow_t& ) // This exists solely for the texture loader (hopefully nothing else is using it)
{
	return EGMem2_Alloc( _Size , eg_mem_pool::DefaultHi );
}

void* operator new [] ( size_t size )
{
	return EGMem2_Alloc( size , eg_mem_pool::DefaultHi );
}

void operator delete [] ( void* p )
{	
	EGMem2_Free(p);
}

void* operator new ( eg_size_t Size , eg_mem_pool MemPool )
{
	return EGMem2_Alloc( Size , MemPool );
}

void* operator new [] ( eg_size_t Size , eg_mem_pool MemPool )
{
	return EGMem2_Alloc( Size , MemPool );
}

void operator delete ( void* p , eg_mem_pool MemPool )
{
	unused( MemPool );

	EGMem2_Free( p );
}

void operator delete [] ( void* p , eg_mem_pool MemPool )
{
	unused( MemPool );

	EGMem2_Free( p );
}

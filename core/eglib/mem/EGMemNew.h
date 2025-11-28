// (c) 2017 Beem Media

#pragma once

void* operator new ( eg_size_t Size , eg_mem_pool MemPool );
void* operator new [] ( eg_size_t Size , eg_mem_pool MemPool );
void operator delete ( void* p , eg_mem_pool MemPool );
void operator delete [] ( void* p , eg_mem_pool MemPool );

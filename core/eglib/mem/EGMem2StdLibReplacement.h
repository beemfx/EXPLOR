//(c) 2017 Beem Media

#pragma once

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

void* EG_malloc( eg_size_t Size );
void* EG_calloc(eg_size_t Num, eg_size_t Size);
void* EG_realloc(void* Mem, eg_size_t Size);
void EG_free( void* Mem );

#ifdef __cplusplus
}
#endif __cplusplus

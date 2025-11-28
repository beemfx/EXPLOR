// (c) 2012 Beem Media

#pragma once

#include "EGMemAlloc.h"

class EGStaticAlloc: public IMemAlloc
{
public:

	EGStaticAlloc()
	: CHUNK_SIZE(0)
	, m_pChunk(nullptr)
	, m_nNumBlocks(0)
	, m_nNextAllocOffset(0)
	, m_nLargestAlloc(0)
	, m_Alignment(0)
	{
		//
	}

	void Init( void* Mem , eg_size_t Size , eg_uint Alignment );
	void Deinit( void );

	virtual void* Alloc(eg_size_t nSize, const eg_char8*const strType, const eg_char8*const strFile, const eg_uint nLine);
	void* AllocAligned(eg_size_t Size, eg_size_t Alignment , const eg_char8* strType, const eg_char8* strFile, eg_uint nLine);
	virtual void  Free(void*);
	void FreeAll(){ m_nNumBlocks = 0; m_nNextAllocOffset=0; }

	virtual eg_size_t GetInfo(INFO_T Type)const;

private:

	eg_size_t CHUNK_SIZE;
	eg_byte*  m_pChunk;

	eg_uint   m_nNumBlocks;
	eg_size_t m_nNextAllocOffset;
	eg_size_t m_nLargestAlloc;
	eg_uint   m_Alignment;
};
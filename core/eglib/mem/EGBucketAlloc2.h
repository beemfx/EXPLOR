#pragma once
#include "EGMemAlloc.h"

class EGBucketAlloc2: public IMemAlloc
{
public:
	EGBucketAlloc2()
	: m_BucketSize(0)
	, m_MemSize(0)
	, m_NumBuckets(0)
	, m_apBuckets(nullptr)
	, m_StackPos(0)
	, m_pChunk(nullptr)
	, m_LargestAlloc(0)
	, m_MemAddr(0)
	{
		//
	}

	void Init( void* Mem , eg_size_t MemSize , eg_size_t BucketSize );
	void Deinit( void );

	virtual void* Alloc(eg_size_t nSize, const eg_char8*const strType, const eg_char8*const strFile, const eg_uint nLine);
	virtual void  Free(void*);

	virtual eg_size_t GetInfo(INFO_T Type)const;
private:
	//The bucket's are just a list of pointers to the chunck
	void**       m_apBuckets;
	eg_uint      m_StackPos;
	eg_uint      m_NumBuckets;
	void*        m_pChunk;
	eg_uintptr_t m_MemAddr;
	eg_size_t    m_MemSize;
	eg_size_t    m_BucketSize;
	//For metrics:
	eg_size_t    m_LargestAlloc;
};

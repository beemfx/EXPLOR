/******************************************************************************
File: MemAllocatorBase.h

(c) 2012 Beem Software
******************************************************************************/
#pragma once

class IMemAlloc
{
public:
	virtual void* Alloc(eg_size_t nSize, eg_cpstr8 const strType, eg_cpstr8 const strFile , const eg_uint nLine )=0;
	virtual void  Free(void*)=0;

	enum INFO_T
	{
		INFO_FREE_BLOCKS,
		INFO_ALLOC_BLOCKS,
		INFO_MAX_BLOCKS,

		INFO_FREE_MEM,
		INFO_ALLOC_MEM,
		INFO_MAX_MEM,

		INFO_FRAGMENTS,
		INFO_LARGEST_ALLOC,
		INFO_MAX_MEM_USED,

		INFO_LO_ADDR,
		INFO_HI_ADDR,
	};

	virtual eg_size_t GetInfo(INFO_T Type)const=0;
};
// (c) 2018 Beem Media

#pragma once

static const eg_size_t EG_BIG_ALLOC_SIZE = 1*1024*1024; // 1MB is a big alloc.

struct eg_mem_pool
{
private:

	eg_string_crc PoolId = CT_Clear;

public:

	explicit eg_mem_pool( eg_string_crc InPoolId ): PoolId( InPoolId ){ }
	eg_bool operator==( const eg_mem_pool& rhs ) const { return PoolId == rhs.PoolId; }

	eg_bool IsNull() const { return PoolId.IsNull(); }

public:

	static const eg_mem_pool Default;
	static const eg_mem_pool DefaultHi;
	static const eg_mem_pool Entity;
	static const eg_mem_pool System;
	static const eg_mem_pool Audio;
	static const eg_mem_pool FsSystem;
	static const eg_mem_pool FsTemp;
	static const eg_mem_pool DefaultArray;
	static const eg_mem_pool DefaultObject;
	static const eg_mem_pool String;
	static const eg_mem_pool RenderResource;
	static const eg_mem_pool BigAllocs;
	static const eg_mem_pool BigAllocsHi;
};
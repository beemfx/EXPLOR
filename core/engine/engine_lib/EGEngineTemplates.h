// (c) 2015 Beem Software

#pragma once

#include "EGItemMap.h"

//
// EGSysMemItemMap - An EGItemMap that gets it's memory from system mem Init 
// must be called, usually used for always loaded resource types.
// It is system mem, so the memory isn't actually freed when Deinit is called, 
// for that reason this should strictly be used for always loaded types.
//
template <class T>
class EGSysMemItemMap: public EGItemMap<EGStringCrcMapKey,T>
{
public:

	EGSysMemItemMap( const T& Default = nullptr ): EGItemMap<EGStringCrcMapKey,T>( eg_mem_pool::System , Default ){ }

	void Init( eg_size_t MaxItems )
	{
		m_List.Reserve( MaxItems );
	}

	void Deinit()
	{
		Clear();
	}
};

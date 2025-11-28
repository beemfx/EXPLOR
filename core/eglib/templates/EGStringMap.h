// (c) 2017 Beem Media

#pragma once

#include "EGItemMap.h"

template <class T, const eg_uint MAX>
class EGStringCrcMapFixedSize: public EGFixedItemMap<EGStringCrcMapKey,T,MAX>
{
public:

	EGStringCrcMapFixedSize( eg_ctor_t Ct , T Default = nullptr )
	: EGFixedItemMap( Ct , Default )
	{ 

	}
};

template <class T>
class EGStringCrcMap: public EGItemMap<EGStringCrcMapKey,T>
{
public:

	EGStringCrcMap( T Default = nullptr )
	: EGItemMap( Default )
	{ 

	}
};

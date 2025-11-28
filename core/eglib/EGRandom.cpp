// EGRandom - Class for pseudo-rng
// (c) 2016 Beem Media
#include "EGRandom.h"
#include "EGTimer.h"

static eg_int EG_GNUC_RAND_MAX = 2147483647;
static eg_int EG_GNUC_rand_r( eg_uint* seed );

EGRandom::eg_seed_v EGRandom::CreateSeed()
{
	return static_cast<eg_seed_v>(Timer_GetRawTime()%EG_GNUC_RAND_MAX);
}

EGRandom::EGRandom( eg_ctor_t Ct )
{
	if( Ct == CT_Clear )
	{
		m_CurSeed = 0;
	}
	else if( Ct == CT_Default )
	{
		m_CurSeed = CreateSeed();
	}
	else
	{
		// Do nothing, that way if this is serialized RNG won't change on load.
	}
}

EGRandom::EGRandom( eg_seed_v Seed )
: m_CurSeed( Seed )
{

}

void EGRandom::ReSeed( eg_seed_v Seed )
{
	m_CurSeed = Seed;
}

eg_int EGRandom::GetRandomRange( eg_int Min, eg_int Max )
{
	eg_real RandF = GetRandomRange( 0.f , 1.f );
	return EG_Clamp( Min + EGMath_floor(RandF*(Max - Min + 1)) , Min , Max );
}

eg_real EGRandom::GetRandomRange( eg_real Min, eg_real Max )
{
	eg_seed_v Rand = GetNextPseudoRN();
	eg_real RandF = static_cast<eg_real>(Rand) / EG_GNUC_RAND_MAX;
	return EGMath_GetMappedRangeValue( RandF , eg_vec2(0.f,1.f) , eg_vec2(Min,Max) );
}


EGRandom::eg_seed_v EGRandom::GetNextPseudoRN()
{
	eg_seed_v Out = EG_GNUC_rand_r( &m_CurSeed )%EG_GNUC_RAND_MAX;
	return Out;
}

/* Reentrant random function from POSIX.1c.
Copyright (C) 1996, 1999, 2009 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.  */


/* This algorithm is mentioned in the ISO C standard, here extended
for 32 bits.  */
static eg_int EG_GNUC_rand_r( eg_uint* seed )
{
	eg_uint next = *seed;
	eg_int result;

	next *= 1103515245;
	next += 12345;
	result = (eg_uint) (next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (eg_uint) (next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (eg_uint) (next / 65536) % 1024;

	*seed = next;

	return result;
}
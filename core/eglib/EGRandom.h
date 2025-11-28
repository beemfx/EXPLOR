// EGRandom - Class for pseudo-rng
// (c) 2016 Beem Media
#pragma once

class EGRandom
{
public:
	typedef eg_uint eg_seed_v;

public:
	static eg_seed_v CreateSeed(); // Helper function to create a seed.

public:

	EGRandom() = default;
	EGRandom( eg_ctor_t Ct );
	EGRandom( eg_seed_v Seed );

	void    ReSeed( eg_seed_v Seed );

	eg_int  GetRandomRangeI( eg_int Min , eg_int Max ){ return GetRandomRange( Min , Max ); }
	eg_int  GetRandomRange( eg_int Min , eg_int Max ); // Inclusive
	eg_real GetRandomRange( eg_real Min , eg_real Max ); // Inclusive
	eg_real GetRandomRangeF( eg_real Min , eg_real Max ) { return GetRandomRange( Min , Max ); }

private:
	eg_seed_v m_CurSeed;

private:
	eg_seed_v GetNextPseudoRN();
};
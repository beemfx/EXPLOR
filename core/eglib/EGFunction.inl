#include "EGStdLibAPI.h"

EGFUNC_INLINE void EGMem_Copy( void* Dest , const void* Src , eg_size_t Size )
{
	#if defined( __DEBUG__ )
	//Check for memory overlap.
	eg_uintptr_t DestStart = reinterpret_cast<eg_uintptr_t>(Dest);
	eg_uintptr_t DestEnd   = DestStart+Size;
	eg_uintptr_t SrcStart = reinterpret_cast<eg_uintptr_t>(Src);
	eg_uintptr_t SrcEnd   = SrcStart+Size;

	assert( !(DestStart <= SrcEnd && SrcStart <= DestEnd) );
	#endif
	memcpy( Dest , Src , Size );
}

EGFUNC_INLINE eg_bool EGMem_Equals( const void* Left , const void* Right , eg_size_t Size )
{
	if( Left == Right )
	{
		return true;
	}
	return 0 == memcmp( Left , Right , Size );
}

EGFUNC_INLINE void EGMem_Set( void* Dest , eg_int Value , eg_size_t Size )
{
	memset( Dest , Value , Size );
}

EGFUNC_INLINE eg_bool EGMem_Contains( const void* Mem , eg_size_t MemSize , const void* Question , eg_size_t QuestionSize )
{
	const eg_uintptr_t MemValue = reinterpret_cast<eg_uintptr_t>(Mem);
	const eg_uintptr_t QuestionValue = reinterpret_cast<eg_uintptr_t>(Question);

	return MemValue <= QuestionValue && (QuestionValue+QuestionSize) <= (MemValue+MemSize);
}

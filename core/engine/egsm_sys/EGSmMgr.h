// (c) 2017 Beem Media

#pragma once

#include "EGSmTypes.h"

class EGSmProc;

struct egsmVarPair
{
	eg_string_crc Id;
	egsm_var      Value;

	eg_int64 Store() const
	{
		eg_int64 Out = 0;//(static_cast<eg_int64>(Id.ToUint32()) << 32) | (static_cast<eg_uint32>(Value) << 0);
		EGMem_Copy( &Out , this , sizeof(Out) );
		return Out;
	}

	void Load( eg_int64 AsInt )
	{
		egsmVarPair In;
		EGMem_Copy( &In , &AsInt , sizeof(In) );
		*this = In;
	}
};

void EGSmMgr_Init();
void EGSmMgr_Deinit();
const EGSmProc* EGSmMgr_GetProc( eg_string_crc ProcId );
void EGSmMgr_GetVars( EGArray<egsmVarPair>& Out );
eg_string_small EGSmMgr_VarCrcToStr( eg_string_crc Crc );
egsm_var_t EGSmMgr_GetVarType( eg_string_crc Crc );

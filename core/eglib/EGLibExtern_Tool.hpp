// (c) 2017 Beem Media
// Implementation of EGLibExtern suitable for tools.

#include "EGLibExtern.h"
#include "EGToolsHelper.h"

eg_bool EGLibExtern_LoadNowTo( eg_cpstr8 Filename , EGFileData& MemFile )
{
	return EGToolsHelper_OpenFile( EGString_ToWide( Filename ) , MemFile );
}

eg_bool EGLibExtern_LoadNowTo( eg_cpstr16 Filename , EGFileData& MemFile )
{
	return EGToolsHelper_OpenFile( Filename , MemFile );
}

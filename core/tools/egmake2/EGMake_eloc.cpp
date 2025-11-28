// (c) 2017 Beem Media

#include "EGMake.h"
#include "EGLocCompiler.h"

eg_bool EGMakeRes_eloc()
{
	EGLocCompiler Compiler( ::EGMake_GetInputPath() );
	eg_bool bLoaded = 0 != Compiler.GetOutput().GetSize() > 0;
	::EGMake_WriteOutputFile( EGMake_GetOutputPath(FINFO_FULL_FILE) , Compiler.GetOutput() );
	return bLoaded;
}
// (c) 2017 Beem Media

#include "EGMake.h"
#include "EGFileData.h"
#include "EGSmFile2.h"

eg_bool EGMakeRes_egsm()
{
	EGSmFile2 File;
	File.LoadForTool( EGString_ToWide(EGMake_GetInputPath()) );

	EGFileData CompiledFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bSucceeded = File.CompileToByteCode( CompiledFile );

	if( bSucceeded )
	{
		eg_string_big FinalName = EGMake_GetOutputPath( FINFO_NOEXT_FILE );
		FinalName.Append( ".egsmbin" );
		bSucceeded = EGMake_WriteOutputFile( FinalName , CompiledFile );
	}

	return bSucceeded;
}

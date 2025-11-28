// (c) 2017 Beem Media

#include "EGMake.h"
#include "EGFileData.h"
#include "EGLibFile.h"

eg_bool EGMakeRes_Copy( eg_cpstr ExtOverride )
{
	eg_bool bSuccess = false;
	EGFileData SourceFile( eg_file_data_init_t::HasOwnMemory );
	if( EGLibFile_OpenFile( EGString_ToWide(EGMake_GetInputPath()) , eg_lib_file_t::OS , SourceFile ) )
	{
		eg_string_big OutputFile;

		if( nullptr == ExtOverride )
		{
			OutputFile = EGMake_GetOutputPath();
		}
		else
		{
			OutputFile = ::EGMake_GetOutputPath( FINFO_NOEXT_FILE );
			OutputFile += '.';
			OutputFile += ExtOverride;
		}

		bSuccess = EGLibFile_SaveFile( EGString_ToWide(OutputFile) , eg_lib_file_t::OS , SourceFile );
	}

	return bSuccess;
}

eg_bool EGMakeRes_Copy()
{
	return EGMakeRes_Copy( nullptr );
}
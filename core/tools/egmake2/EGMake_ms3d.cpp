// (c) 2018 Beem Media

#include "EGMake.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGMs3DCompiler.h"

eg_bool EGMakeRes_ms3d()
{
	eg_bool bMeshSuccess = true;
	eg_bool bSkelSuccess = true;

	eg_string_big InFilename = EGMake_GetInputPath( FINFO_FULL_FILE );
	eg_string_big OutFilename = EGMake_GetOutputPath( FINFO_NOEXT_FILE );
	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	eg_bool bOpenedFile = EGLibFile_OpenFile( EGString_ToWide(InFilename) , eg_lib_file_t::OS , FileData );

	if( bOpenedFile )
	{
		FileData.Seek( eg_file_data_seek_t::Begin , 0 );
		EGMs3DCompiler Compiler( FileData );

		if( Compiler.HasMesh() )
		{
			eg_string_big OutMeshFilename = OutFilename;
			OutMeshFilename += ".emesh";
			EGFileData OutData( eg_file_data_init_t::HasOwnMemory );
			Compiler.SaveEMesh( OutData );
			bMeshSuccess = EGLibFile_SaveFile( EGString_ToWide(OutMeshFilename) , eg_lib_file_t::OS , OutData );
		}

		if( Compiler.HasSkel() )
		{
			eg_string_big OutSkelFilename = OutFilename;
			OutSkelFilename += ".eskel";
			EGFileData OutData( eg_file_data_init_t::HasOwnMemory );
			Compiler.SaveESkel( OutData );
			bMeshSuccess = EGLibFile_SaveFile( EGString_ToWide(OutSkelFilename) , eg_lib_file_t::OS , OutData );
		}
	}
	else
	{
		bMeshSuccess = false;
		bSkelSuccess = false;
	}

	return bMeshSuccess && bSkelSuccess;
}
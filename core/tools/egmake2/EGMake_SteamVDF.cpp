// (c) 2020 Beem Media. All rights reserved.

#include "EGMake_SteamVDF.h"
#include "EGWnd.h"
#include "EGPath2.h"
#include "EGFileData.h"
#include "EGToolsHelper.h"
#include "EGOsFile.h"

int EGMake_SteamVDF_main( int argc , char* argv[] )
{
	unused( argc , argv );
	
	EGLogf( eg_log_t::General , "Steam VDF" );

	EGWndAppParms AppParms;
	EGWnd_GetAppParmsFromCommandLine( AppParms );

	const eg_d_string16 InputFile = *EGPath2_CleanPath( AppParms.GetParmValue( "-in" ).String() , L'/' );
	const eg_d_string16 OutputFile = *EGPath2_CleanPath( AppParms.GetParmValue( "-out" ).String() , L'/' );

	const eg_d_string8 Comment = AppParms.GetParmValue( "-comment" );
	const eg_bool bWantPreview = AppParms.ContainsType( "-preview" );

	if( Comment.Len() == 0 )
	{
		EGLogf( eg_log_t::General , "VDF must have a comment." );
		if( EGOsFile_DoesFileExist( *OutputFile ) )
		{
			EGOsFile_DeleteFile( *OutputFile );
		}
		return -1;
	}
	
	EGLogf( eg_log_t::General , "Using VDF Template \"%s\" -> \"%s\"..." , *eg_d_string8( *InputFile ) , *eg_d_string8( *OutputFile ) );

	EGFileData TemplateFileData( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( *InputFile , TemplateFileData );
	TemplateFileData.Seek( eg_file_data_seek_t::End , 0 );
	TemplateFileData.Write<eg_char8>( '\0' );
	eg_d_string8 FinalFileStr( TemplateFileData.GetDataAs<eg_char8>() );

	EGStringEx_ReplaceAll( FinalFileStr , "%%%DESC%%%" , *Comment );
	EGStringEx_ReplaceAll( FinalFileStr , "%%%PREVIEW%%%" , bWantPreview ? "1" : "0" );

	EGFileData OutFileData( eg_file_data_init_t::HasOwnMemory );
	OutFileData.WriteStr8( *FinalFileStr );
	const eg_bool bSaved = EGToolsHelper_SaveFile( *OutputFile , OutFileData );

	return bSaved ? 0 : -1;
}

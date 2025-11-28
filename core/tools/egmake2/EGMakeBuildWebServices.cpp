// (c) 2016 Beem Media

#include "EGMake.h"
#include "EGToolsHelper.h"
#include "EGOsFile.h"
#include "EGNetCore.h"
#include "EGPath2.h"
#include "EGBase64.h"
#include "EGFileData.h"
#include "EGLibFile.h"

int EGMakeWebService_main( int argc , char *argv[] )
{
	unused( argc , argv );

	EGNetCore_Init();

	eg_s_string_big8 SourceFolder = EGToolsHelper_GetEnvVar( "EGSRC" );
	eg_s_string_big8 OutFolder = EGToolsHelper_GetEnvVar( "EGOUT" );
	eg_s_string_big8 DestUrl = eg_s_string_big8(EGToolsHelper_GetEnvVar( "EGWEBSVCHOST" ));
	eg_s_string_big16 RequestUrl = *(DestUrl + "/cadmin.php");
	eg_s_string_sml8 GameBuild = EGMake_GetGameToBuildFromReg();

	EGLogf( eg_log_t::General , "Building \"%s\" web services using \"%s\"" , *GameBuild , *DestUrl );

	const eg_d_string16 EngineRoot = EGPath2_CleanPath( *EGSFormat16( L"{0}/core/web" , *SourceFolder ) , '/' );
	const eg_d_string8 EngineTarget = "";
	const eg_d_string16 EngineDiffFile = EGPath2_CleanPath( *EGSFormat16( L"{0}/dbdiff/web/{1}.egdiff" , *OutFolder , L"egweb" ) , '/' );
	const eg_d_string16 GameRoot = EGPath2_CleanPath( *EGSFormat16( L"{0}/games/{1}/web" , *SourceFolder , *GameBuild ) , '/' );
	const eg_d_string8 GameTarget = *GameBuild;
	const eg_d_string16 GameDiffFile = EGPath2_CleanPath( *EGSFormat16( L"{0}/dbdiff/web/{1}.egdiff" , *OutFolder , *GameBuild ) , '/' );

	const eg_bool bEngineWebHasChanged = EGMake_HasDataChanged( *eg_d_string8(*EngineRoot) , *eg_d_string8(*EngineDiffFile) );
	const eg_bool bGameWebHasChanged = EGMake_HasDataChanged( *eg_d_string8(*GameRoot) , *eg_d_string8(*GameDiffFile) );

	if( bEngineWebHasChanged || bGameWebHasChanged )
	{
		eg_d_string8 ResultString;

		EGLogf( eg_log_t::General, "Wiping current data..." );
		ResultString = EGNetCore_MakeRequest( *RequestUrl, L"WIPE" );
		EGLogf( eg_log_t::General, "Result: %s", *ResultString );

		auto BuildDir = [&DestUrl,&RequestUrl]( const eg_d_string16& RootDir , const eg_d_string8& TargetDir ) -> void
		{
			EGLogf( eg_log_t::General , "Uploading Directory: %s to %s/%s" , *eg_s_string_sml8(*RootDir) , *DestUrl , *TargetDir );

			EGOsFileWantsFileFn WantsFileFn = []( eg_cpstr Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
			{
				if( EGString_EqualsI( Filename , "cadmin.php" ) )
				{
					return false;
				}

				if( FileAttr.bHidden || FileAttr.bDirectory )
				{
					return false;
				}

				return true;
			};

			EGArray<eg_d_string> FoundFiles;
			EGOsFile_FindAllFiles( *eg_d_string8(*RootDir) , WantsFileFn , FoundFiles );
			for( const eg_d_string& File : FoundFiles )
			{
				eg_d_string FinalFilename = TargetDir.Len() > 0 ? TargetDir + "/" + File : File;
				eg_d_string FinalSourceFilename = *EGPath2_CleanPath( *EGSFormat16( L"{0}/{1}" , *RootDir , *File ) , '/' );
				EGLogf( eg_log_t::General , "Uploading File: %s to %s/%s" , *FinalSourceFilename , *DestUrl , *FinalFilename );

				EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
				EGLibFile_OpenFile( *FinalSourceFilename , eg_lib_file_t::OS , FileData );
				const eg_d_string8 TestFileBase64 = EGBase64_Encode( FileData );
				const eg_d_string8 RequestString = EGSFormat8( "UPLOAD:{0}:", *FinalFilename ) + TestFileBase64;
				const eg_d_string8 ResultString = EGNetCore_MakeRequest( *RequestUrl , RequestString );
				EGLogf( eg_log_t::General, "Request Result: \"%s\"", *ResultString );
			}
		};

		BuildDir( EngineRoot , EngineTarget );
		BuildDir( GameRoot , GameTarget );
	}
	else
	{
		EGLogf( eg_log_t::General , "No changes detected, skipping." );
	}

	EGNetCore_Deinit();

	return 0;
}
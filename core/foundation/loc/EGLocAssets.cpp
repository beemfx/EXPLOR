// (c) 2018 Beem Media

#include "EGLocAssets.h"
#include "EGThread.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGOsFile.h"
#include "EGExtProcess.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGEdUtility.h"

static void LocalizeCodeFile( eg_cpstr SourceCodeFilename , eg_cpstr LocFileName )
{
	EGLogf( eg_log_t::Verbose , "Localizing %s..." , SourceCodeFilename );
	eg_string_big LocCmd = EGString_Format( "egmake2_x64.exe CODELOC -in \"%s\" -out \"%s\"" , SourceCodeFilename , LocFileName );
	EGExtProcess_Run(  LocCmd , nullptr );
}

static void LocalizeXmlAssetFile( eg_cpstr SourceCodeFilename , eg_cpstr LocFileName )
{
	EGLogf( eg_log_t::Verbose , "Localizing %s..." , SourceCodeFilename );
	eg_string_big LocCmd = EGString_Format( "egmake2_x64.exe CODELOC -type \"XMLASSET\" -in \"%s\" -out \"%s\"" , SourceCodeFilename , LocFileName );
	EGExtProcess_Run(  LocCmd , nullptr );
}

static void LocalizeEgsmAssetFile( eg_cpstr SourceCodeFilename , eg_cpstr LocFileName )
{
	EGLogf( eg_log_t::Verbose , "Localizing %s..." , SourceCodeFilename );
	eg_string_big LocCmd = EGString_Format( "egmake2_x64.exe CODELOC -type \"EGSM\" -in \"%s\" -out \"%s\"" , SourceCodeFilename , LocFileName );
	EGExtProcess_Run(  LocCmd , nullptr );
}

static void LocalizeDir( eg_cpstr DirPath )
{
	eg_string_big TargetDir = EGToolsHelper_GetEnvVar("EGOUT");
	EGLogf( eg_log_t::General , "Searching %s..." , DirPath );
	egPathParts2 PathParts = EGPath2_BreakPath( DirPath );
	eg_string_small GameName = PathParts.Folders.HasItems() ? *PathParts.Folders.Last() : L"";

	EGArray<eg_string_small> IgnorePaths;
	IgnorePaths.Append( "core/lib3p" );
	IgnorePaths.Append( "core/eglib" );
	IgnorePaths.Append( "core/libs" );
	IgnorePaths.Append( "core/tools" );
	IgnorePaths.Append( "core/PreBuild" );
	IgnorePaths.Append( "core/audio" );
	IgnorePaths.Append( "core/web" );
	IgnorePaths.Append( "core/BuildTools" );

	EGArray<eg_string_big> TargetFiles;

	{
		EGArray<eg_string_small> WantedTypes;
		WantedTypes.Append( "cpp" );
		WantedTypes.Append( "h" );
		WantedTypes.Append( "hpp" );
		WantedTypes.Append( "items" );
		WantedTypes.Append( "inc" );

		EGArray<eg_string_big> FileList;
		EGOsFile_FindAllFiles( ".\\" , DirPath , WantedTypes , IgnorePaths , FileList );

		eg_string_big LocFileName = *EGPath2_CleanPath( EGString_Format( "%s/locbuild/codeloc-%s-enus.eloc" , TargetDir.String() , GameName.String() ) , '/' );
		TargetFiles.Append( LocFileName );
		EGOsFile_DeleteFile( LocFileName );
		for( const eg_string_big& File : FileList )
		{
			LocalizeCodeFile( File , LocFileName );
		}
	}

	{
		EGArray<eg_string_small> WantedTypes;
		WantedTypes.Append( "xml" );
		WantedTypes.Append( "elyt" );
		WantedTypes.Append( "edef" );
		WantedTypes.Append( "egasset" );

		EGArray<eg_string_big> FileList;
		EGOsFile_FindAllFiles( ".\\" , DirPath , WantedTypes , IgnorePaths , FileList );

		eg_string_big LocFileName = *EGPath2_CleanPath( EGString_Format( "%s/locbuild/assetloc-%s-enus.eloc" , TargetDir.String() , GameName.String() ) , '/' );
		TargetFiles.Append( LocFileName );
		EGOsFile_DeleteFile( LocFileName );
		for( const eg_string_big& File : FileList )
		{
			LocalizeXmlAssetFile( File , LocFileName );
		}
	}

	{
		EGArray<eg_string_small> WantedTypes;
		WantedTypes.Append( "egsm" );

		EGArray<eg_string_big> FileList;
		EGOsFile_FindAllFiles( ".\\" , DirPath , WantedTypes , IgnorePaths , FileList );

		eg_string_big LocFileName = *EGPath2_CleanPath( EGString_Format( "%s/locbuild/egsmloc-%s-enus.eloc" , TargetDir.String() , GameName.String() ) , '/' );
		TargetFiles.Append( LocFileName );
		EGOsFile_DeleteFile( LocFileName );
		for( const eg_string_big& File : FileList )
		{
			LocalizeEgsmAssetFile( File , LocFileName );
		}
	}

	EGLogf( eg_log_t::General , "Importing localization files to game data..." );
	for( const eg_string_big& Filename : TargetFiles )
	{
		eg_string_big RootPath = EGString_Format( "%s/data/loc/" , DirPath );
		if( !EGOsFile_DoesDirExist( RootPath.String() ) )
		{
			eg_string_big EgDataRootPath = EGString_Format( "%s/egdata/loc/" , DirPath );
			if( EGOsFile_DoesDirExist( EgDataRootPath.String() ) )
			{
				RootPath = EgDataRootPath;
			}
		}
		eg_string_big ReimportFilename = *EGPath2_CleanPath( EGString_Format( "%s%s" , RootPath.String() , *EGPath2_GetFilename( Filename ) ) , '/' );
		if( EGOsFile_DoesFileExist( Filename ) )
		{
			EGLogf( eg_log_t::General , "Copying \"%s\" to \"%s\"" , Filename.String() , ReimportFilename.String() );
			EGFileData LocFileData( eg_file_data_init_t::HasOwnMemory );
			EGLibFile_OpenFile( EGString_ToWide(Filename) , eg_lib_file_t::OS , LocFileData );
			eg_bool bSaved = EGEdUtility_SaveFile( EGString_ToWide(ReimportFilename) , LocFileData );
			EGEdUtility_RevertFile( EGString_ToWide(ReimportFilename) , true );
			if( bSaved )
			{
				
			}
			else
			{
				EGLogf( eg_log_t::Error , "Could not save \"%s\", may not have been able to check out of source control." , ReimportFilename.String() );
			}
		}
	}
}

static void GetLocDirs( EGArray<eg_d_string>& Out )
{
	Out.Append( "core/" );
	Out.Append( EGString_Format( "games/%s/"  , EGToolsHelper_GetEnvVar( "EGGAME" ).String() ).String() );
}

void EGLocAssets_Execute()
{
	const eg_d_string16 RestoreDir = EGOsFile_GetCWD();
	EGOsFile_SetCWD( EGToolsHelper_GetEnvVar("EGSRC") );
	EGOsFile_CreateDirectory( EGString_Format( "%s/locbuild" , EGToolsHelper_GetEnvVar("EGOUT").String() ) );
	EGLogf( eg_log_t::General , "Localizing game assets..." );
	EGArray<eg_d_string> LocDirs;
	GetLocDirs( LocDirs );
	for( const eg_d_string& Dir : LocDirs )
	{
		LocalizeDir( *Dir );
	}

	EGOsFile_SetCWD(*RestoreDir);
	EGLogf( eg_log_t::General , "Localization complete." );
}

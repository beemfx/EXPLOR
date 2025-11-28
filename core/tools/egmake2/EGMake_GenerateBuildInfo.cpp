// (c) 2020 Beem Media. All Rights Reserved.

#include "EGMake_GenerateBuildInfo.h"
#include "EGWnd.h"
#include "EGPath2.h"
#include "EGOsFile.h"
#include "EGFileData.h"
#include "EGEdUtility.h"

static eg_bool EGMake_GenerateBuildInfo_WriteFileDataForType( EGFileData& BcFile , const eg_d_string16& Ext )
{
	if( Ext.EqualsI( L"png" ) || Ext.EqualsI( L"tga" ) )
	{
		BcFile.WriteStr8( "\t<NoMipMaps>False</NoMipMaps>\r\n" );
		BcFile.WriteStr8( "\t<Width>Default</Width>\r\n" );
		BcFile.WriteStr8( "\t<Height>Default</Height>\r\n" );
		BcFile.WriteStr8( "\t<Format>R8G8B8A8_UNORM</Format>\r\n" );
		BcFile.WriteStr8( "\t<Filter></Filter>\r\n" );

		return true;
	}

	if( Ext.EqualsI( L"ms3d" ) )
	{
		return true;
	}

	if( Ext.EqualsI( L"xml" ) )
	{
		return true;
	}

	if( Ext.EqualsI( L"xlsx" ) )
	{
		return true;
	}

	return false;
}

int EGMake_GenerateBuildInfo_main( int argc , char* argv[] )
{
	unused( argc , argv );
	
	EGWndAppParms AppParms;
	EGWnd_GetAppParmsFromCommandLine( AppParms );

	eg_d_string16 TargetDirectory = AppParms.GetParmValue( "-in" );
	TargetDirectory.Append( '\\' );
	TargetDirectory = EGPath2_CleanPath( *TargetDirectory , '\\' );

	EGLogf( eg_log_t::General , "Generating Build Info for \"%s\"..." , *eg_d_string8(*TargetDirectory) );

	if( !EGOsFile_DoesDirExist( *TargetDirectory ) )
	{
		EGLogf( eg_log_t::General , "No such directory, nothing to build." );
		return 0;
	}

	egOsFileInfo DirectoryContents;
	EGOsFile_GetDirectoryContents( *TargetDirectory , DirectoryContents );

	EGLogf( eg_log_t::General , "Root: %s" , *eg_d_string8(*DirectoryContents.Name) );
	for( egOsFileInfo& Info : DirectoryContents.Children )
	{
		const egPathParts2 FileParts = EGPath2_BreakPath( *Info.Name );
		
		if( FileParts.Ext.EqualsI( L"BuildConfig" ) || FileParts.Ext.EqualsI( L"BuildTerrain" ) )
		{
			continue;
		}

		EGLogf( eg_log_t::General , "Processing \"%s\"..." , *eg_d_string8(*Info.Name) );

		const eg_d_string16 BuildConfigPath = DirectoryContents.Name + FileParts.GetFilename( false ) + L".BuildConfig";
		if( EGOsFile_DoesFileExist( *BuildConfigPath ) )
		{
			EGLogf( eg_log_t::General , "Build Config already exists for \"%s\", skipping." , *eg_d_string8(*Info.Name) );
			continue;
		}

		EGLogf( eg_log_t::General , "Generating \"%s\"..." , *eg_d_string8(*BuildConfigPath) );

		EGFileData BcFile( eg_file_data_init_t::HasOwnMemory );
		BcFile.WriteStr8( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
		BcFile.WriteStr8( "<BuildConfig>\r\n" );
		BcFile.WriteStr8( "\t<Ignore>False</Ignore>\r\n" );
		eg_bool bShouldSave = EGMake_GenerateBuildInfo_WriteFileDataForType( BcFile , FileParts.Ext );
		BcFile.WriteStr8( "</BuildConfig>\r\n" );

		if( bShouldSave )
		{
			EGEdUtility_SaveFile( *BuildConfigPath , BcFile );
		}
		else
		{
			EGLogf( eg_log_t::General , "Not a valid game asset, ignoring." );
		}
	}

	return 0;
}

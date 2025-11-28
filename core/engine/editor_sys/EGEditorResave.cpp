// (c) 2019 Beem Media

#include "EGEditorResave.h"
#include "EGOsFile.h"
#include "EGPath2.h"
#include "EGWindowsAPI.h"
#include "EGStdLibAPI.h"
#include "EGLibFile.h"
#include "EGFileData.h"
#include "EGEntDef.h"
#include "EGEdUtility.h"
#include "EGUiLayout.h"
#include "EGDataAsset.h"
#include "EGWorldFile.h"
#include "EGSmFile2.h"

static void EGEditorResave_ResaveEntDef( const eg_d_string& FullFilename )
{
	EGLogf( eg_log_t::General , "Resaving Entity Definition \"%s\"..." , *FullFilename );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( *FullFilename , eg_lib_file_t::OS , FileData );

	EGEntDef DefFile( CT_Clear );
	DefFile.LoadForEditor( FileData , *FullFilename );

	EGFileData OutFileData( eg_file_data_init_t::HasOwnMemory );
	DefFile.SerializeToXml( OutFileData );

	EGEdUtility_SaveFile( EGString_ToWide(*FullFilename) , OutFileData );
	EGEdUtility_RevertFile( EGString_ToWide(*FullFilename) , true );
}

static void EGEditorResave_ResaveUILayout( const eg_d_string& FullFilename )
{
	EGLogf( eg_log_t::General , "Resaving UI Layout \"%s\"..." , *FullFilename );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( *FullFilename , eg_lib_file_t::OS , FileData );

	EGUiLayout DefFile;
	DefFile.LoadForResave( FileData , *FullFilename );

	EGFileData OutFileData( eg_file_data_init_t::HasOwnMemory );
	DefFile.SaveTo( OutFileData );

	EGEdUtility_SaveFile( EGString_ToWide(*FullFilename) , OutFileData );
	EGEdUtility_RevertFile( EGString_ToWide(*FullFilename) , true );
}

static void EGEditorResave_ResaveAsset( const eg_d_string& FullFilename )
{
	EGLogf( eg_log_t::General , "Resaving Asset \"%s\"..." , *FullFilename );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( *FullFilename , eg_lib_file_t::OS , FileData );

	egRflEditor RflEditor = CT_Clear;
	EGDataAsset* DataAsset = EGDataAsset::LoadDataAsset( FileData , EGString_ToWide(*FullFilename) , eg_mem_pool::System , true , RflEditor );
	if( DataAsset )
	{
		EGDataAsset::SaveDataAsset( EGString_ToWide(*FullFilename) , DataAsset , RflEditor );
		EGEdUtility_RevertFile( EGString_ToWide(*FullFilename) , true );
		EGDeleteObject( DataAsset );
	}
}

static void EGEditorResave_ResaveWorld( const eg_d_string& FullFilename )
{
	EGLogf( eg_log_t::General , "Resaving World \"%s\"..." , *FullFilename );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( *FullFilename , eg_lib_file_t::OS , FileData );

	EGWorldFile* WorldFile = EGNewObject<EGWorldFile>();
	if( WorldFile )
	{
		WorldFile->Load( FileData , *FullFilename , true );

		EGFileData OutFileData( eg_file_data_init_t::HasOwnMemory );
		WorldFile->Save( OutFileData );
		EGEdUtility_SaveFile( EGString_ToWide(*FullFilename) , OutFileData );
		EGEdUtility_RevertFile( EGString_ToWide(*FullFilename) , true );

		EGDeleteObject( WorldFile );
	}
}

static void EGEditorResave_ResaveEGSM( const eg_d_string& FullFilename )
{
	EGLogf( eg_log_t::General , "Resaving EGSM Script \"%s\"..." , *FullFilename );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( *FullFilename , eg_lib_file_t::OS , FileData );

	EGSmFile2 ScriptFile;
	ScriptFile.LoadFromMemFile( FileData );
	ScriptFile.Save( EGString_ToWide(*FullFilename) );
	EGEdUtility_RevertFile( EGString_ToWide(*FullFilename) , true );
	ScriptFile.Clear();
}

static void EGEditorResave_ResaveFile( eg_cpstr InFilename )
{
	egPathParts2 PathParts = EGPath2_BreakPath( InFilename );
	eg_d_string FullFilename = *PathParts.ToString();// true , '\\' );

	if( EGString_EqualsI(*PathParts.Ext , L"edef") )
	{
		EGEditorResave_ResaveEntDef( FullFilename );
	}
	else if( EGString_EqualsI( *PathParts.Ext , L"elyt" ) )
	{
		EGEditorResave_ResaveUILayout( FullFilename );
	}
	else if( EGString_EqualsI( *PathParts.Ext , L"egasset" ) || EGString_EqualsI( *PathParts.Ext , L"egsnd" ) )
	{
		EGEditorResave_ResaveAsset( FullFilename );
	}
	else if( EGString_EqualsI( *PathParts.Ext , L"egworld" ) )
	{
		EGEditorResave_ResaveWorld( FullFilename );
	}
	else if( EGString_EqualsI( *PathParts.Ext , L"egsm" ) )
	{
		EGEditorResave_ResaveEGSM( FullFilename );
	}
	else
	{
		EGLogf( eg_log_t::Verbose , "Skipping \"%s\"." , *FullFilename );
	}
}

static void EGEditorResave_LogHandler( eg_log_channel LogChannel , const eg_string_base& LogString )
{
	unused( LogChannel );

	eg_char FinalString[2048];
	EGString_FormatToBuffer( FinalString , countof(FinalString) , "[RESAVE] %s\n" , LogString.String() );
	if( eg_log_t::Verbose != LogChannel )
	{
		printf( "%s" , FinalString );
		OutputDebugStringA(FinalString);
	}
}

int EGEditor_Resave( eg_cpstr16 InitialFilename, const struct egSdkEngineInitParms& EngineParms )
{
	unused( InitialFilename , EngineParms );

	EGLog_SetHandler( EGEditorResave_LogHandler );

	EGLogf( eg_log_t::General , "Resaving assets..." );

	EGArray<eg_d_string> AllAssets;
	EGOsFile_FindAllFiles( EGString_ToMultibyte(InitialFilename) , nullptr , AllAssets );

	for( const eg_d_string& AssetFile : AllAssets )
	{
		EGEditorResave_ResaveFile( *(eg_d_string(EGString_ToMultibyte(InitialFilename)) + "/" + AssetFile) );
	}

	EGLogf( eg_log_t::General , "Done." );
	return 0;
}

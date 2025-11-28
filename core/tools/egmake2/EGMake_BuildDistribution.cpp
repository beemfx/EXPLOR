// (c) 2019 Beem Media. All Rights Reserved.

#include "EGMake_BuildDistribution.h"
#include "EGEdUtility.h"
#include "EGOsFile.h"
#include "EGLibFile.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"

static void EGBuildDist_PackageGame( eg_cpstr16 Root , const eg_s_string_sml8& GameName , const eg_bool bIncludTools )
{
	const eg_d_string16 BinDir = EGPath2_CleanPath( *EGSFormat16( L"{0}/bin" , Root ) , '/' );
	const eg_d_string16 DistDir = EGPath2_CleanPath( *EGSFormat16( L"{0}/_dist/{1}" , Root , *GameName ) , '/' );

	EGLogf( eg_log_t::General , "Packaging %s from %s to %s.." ,  *GameName , *eg_d_string8(*BinDir) , *eg_d_string8(*DistDir) );

	if( EGOsFile_DoesDirExist( *DistDir ) )
	{
		EGOsFile_ClearDirectory( *DistDir );
	}

	const eg_bool bCreatedDistDir = EGOsFile_CreateDirectory( *DistDir );
	if( !bCreatedDistDir )
	{
		EGLogf( eg_log_t::Error , "Failed to create distribution directory." );
		return;
	}

	auto IsGoodExt = []( eg_cpstr16 Filename ) -> eg_bool
	{
		static const eg_cpstr16 GoodExts[] = { L".exe" , L".dll" , L".ini" , L".txt" };

		for( eg_int i=0; i<countof(GoodExts); i++ )
		{
			if( EGString_EndsWithI( Filename , GoodExts[i] ) )
			{
				return true;
			}
		}

		return false;
	};

	auto IsIgnoreFile = []( eg_cpstr16 Filename ) -> eg_bool
	{
		static const eg_cpstr16 IgnoreFiles[] = { L"steam_appid.txt" , L"_Debug.exe" , L"_DebugEditor.exe" };
		for( eg_int i=0; i<countof(IgnoreFiles); i++ )
		{
			if( EGString_EndsWithI( Filename , IgnoreFiles[i] ) )
			{
				return true;
			}
		}
		return false;
	};

	auto IsToolFile = [&GameName]( eg_cpstr16 Filename ) -> eg_bool
	{
		static const eg_cpstr16 ToolFilesBegin[] = { L"egmake2" /* , L"EGScc" */ , L"EGEdResLib" , L"EGEdApp" , L"EGEdConfig" };

		for( eg_int i=0; i<countof(ToolFilesBegin); i++ )
		{
			if( EGString_BeginsWithI( Filename , ToolFilesBegin[i] ) )
			{
				return true;
			}
		}

		if( EGString_BeginsWithI( Filename , *eg_s_string_sml16(*GameName) ) && EGString_EndsWithI( Filename , L"_ReleaseEditor.exe" ) )
		{
			return true;
		}

		return false;
	};

	egOsFileInfo BinTree;

	EGOsFileWantsFileFn16 WantsFile = [&IsGoodExt,&IsIgnoreFile,&IsToolFile,bIncludTools,&GameName]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
	{
		if( FileAttr.bDirectory )
		{
			return false;
		}

		if( FileAttr.bHidden )
		{
			return false;
		}

		if( !IsGoodExt( Filename ) || IsIgnoreFile( Filename ) )
		{
			return false;
		}

		if( IsToolFile( Filename ) )
		{
			return bIncludTools;
		}

		if( !EGString_EndsWithI( Filename , L".exe" ) )
		{
			return true;
		}

		return EGString_BeginsWithI( Filename , *eg_s_string_sml16(*GameName) );
	};

	EGOsFile_BuildTree( *BinDir , WantsFile , BinTree );
	// BinTree.DebugPrint( "   " );

	// Copy the base directory contents.
	BinTree.WalkTree( [&BinDir,&DistDir]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> void
	{
		unused( FileAttr );

		const eg_d_string16 SourceFilename = BinDir + L"/" + Filename;
		const eg_d_string16 DestFilename = DistDir + L"/"  + Filename;
		EGLogf( eg_log_t::General , "Copying %s -> %s..." , *eg_d_string8(*SourceFilename) , *eg_d_string8(*DestFilename) );
		EGOsFile_CopyFile( *SourceFilename , *DestFilename );
	} );

	// Copy the engine core directory and the game directory.
	const eg_d_string16 SourceCoreDir = BinDir + L"/egdata";
	const eg_d_string16 DestCoreDir = DistDir + L"/egdata";
	EGOsFile_CopyDirectory( *SourceCoreDir , *DestCoreDir );
	const eg_d_string16 SourceGameDir = BinDir + L"/" + *GameName;
	const eg_d_string16 DestGameDir = DistDir + L"/" + *GameName;
	EGOsFile_CopyDirectory( *SourceGameDir , *DestGameDir );
}

int EGMakeBuildDist_main( int argc , char* argv[] )
{
	EGEdUtility_Init();

	EGCmdLineParms CmdLineParms( argc , argv );

	const eg_s_string_sml8 GameToBuild = *CmdLineParms.GetParmValue( L"-game" );
	const eg_d_string16 RootDir = CmdLineParms.GetParmValue( L"-root" );
	const eg_bool bIncludeTools = CmdLineParms.ContainsType( L"-includetools" );

	EGBuildDist_PackageGame( *RootDir , GameToBuild , bIncludeTools );

	EGEdUtility_Deinit();
	return 0;
}

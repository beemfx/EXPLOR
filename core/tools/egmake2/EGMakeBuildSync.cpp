// (c) 2018 Beem Media

#include "EGWnd.h"
#include "EGPath2.h"
#include "EGSync.h"
#include "EGOsFile.h"

int EGMakeBuildSync_main( int argc , char *argv[] )
{
	unused( argc , argv );

	EGLogf( eg_log_t::General , "EG Sync Packager (c) 2018 Beem Media" );

	EGWndAppParms AppParms;
	EGWnd_GetAppParmsFromCommandLine( AppParms );

	eg_d_string InDir = AppParms.GetParmValue( "-in" );
	eg_d_string OutDir = AppParms.GetParmValue( "-out" );
	InDir.Append( "/" );
	OutDir.Append( "/" );
	InDir = EGPath2_CleanPath( *InDir , '/' );
	OutDir = EGPath2_CleanPath( *OutDir , '/' );
	eg_d_string OutDirTempDir = OutDir;
	OutDirTempDir.Append( "t_bigfile/" );
	eg_d_string BigFile = OutDirTempDir;
	BigFile.Append( "all.egbig" );

	const eg_bool bClearCurrent = true;
	const eg_bool bBuildBigFile = true;
	const eg_bool bSplitBigFile = true;
	const eg_bool bReasembleTest = false;

	// We want to start fresh so delete output directory
	if( bClearCurrent )
	{
		EGOsFile_DeleteDirectory( *OutDir );
	}

	// Build a monolithic file with everything
	if( bBuildBigFile )
	{
		EGSync_PackBigFile( *InDir , *BigFile );
	}

	// Split the monolithic file up and generated the manifest for it
	if( bSplitBigFile )
	{
		EGSync_SplitBigFile( *BigFile , *OutDir );
	}

	if( bReasembleTest )
	{
		// A Test to reassemble big file:
		eg_d_string RebuiltBigFile = OutDir;
		RebuiltBigFile.Append( "t_rebuild/rebuilt.egbig" );
		EGSync_AssembleBigFile( *OutDir , *RebuiltBigFile );
		eg_d_string UnpackedOutDir = OutDir;
		UnpackedOutDir.Append( "t_unpacked/" );
		EGSync_UnpackBigFile( *RebuiltBigFile , *UnpackedOutDir );
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
// egmake2 - Makes most things in an EG build
// (c) 2015 Beem Media
///////////////////////////////////////////////////////////////////////////////

#pragma comment( lib , "lib3p.lib" )
#pragma comment( lib , "eglib.lib" )
#pragma comment( lib , "egfoundation.lib" )
#pragma comment( lib , "winhttp.lib" )

#include "EGMake.h"
#include "EGCrcDb.h"
#include "EGToolsHelper.h"
#include "EGFileData.h"
#include "EGMakeCodeLoc.h"
#include "EGWindowsAPI.h"
#include "EGLibFile.h"
#include "EGToolMem.h"
#include "EGRflGen.h"
#include "EGMake_vsgen.h"
#include "EGMake_PrepPlatform.h"
#include "EGMake_BuildDistribution.h"
#include "EGMake_GenerateBuildInfo.h"
#include "EGMake_SteamVDF.h"

static void EGMake2_LogHandler( eg_log_channel LogChannel , const eg_string_base& LogString );

int EGResource_main( int argc, char* argv[] );
int EGMake_main( int argc , char *argv[] );
int EGPack_main( int argc , char *argv[] );
int EGMap_main( int argc , char *argv[] );
int EGMakeWebService_main( int argc , char *argv[] );
int EGMakeBuildSync_main( int argc , char *argv[] );

enum class eg_tool_t
{
	UNK,
	RESOURCE,
	MAKE,
	PACK,
	MAP,
	MAKEWEBSERVICE,
	CODELOC,
	RFLGEN,
	BUILDSYNC,
	VSGEN,
	PREP_PLATFORM,
	BUILD_DIST,
	GENERATE_BUILD_INFO,
	STEAMVDF,
};

static eg_cpstr EGMake_ToolTypeText = "";

struct egToolTypeItem
{
	eg_tool_t ToolType = eg_tool_t::UNK;
	int ( * ToolFn )(  int argc , char* argv[] ) = nullptr;
	eg_cpstr CmdLine = "";
	eg_cpstr TextId = "";
};

static const egToolTypeItem EGMake_ToolTypeTable[] =
{
	{ eg_tool_t::RESOURCE ,EGResource_main , "RESOURCE" , "RES" },
	{ eg_tool_t::MAKE , EGMake_main , "DATA" , "DATA" },
	{ eg_tool_t::MAKE , EGMake_main , "SHADER_HEADER3" , "SHADER_HEADER3" },
	{ eg_tool_t::MAKE , EGMake_main , "SHADER_HEADER5" , "SHADER_HEADER5" },
	{ eg_tool_t::MAKE , EGMake_main , "CREATE_GAME_INI" , "CREATE_GAME_INI" },
	{ eg_tool_t::MAKE , EGMake_main , "SET_GAME_BUILD" , "SET_GAME_BUILD" },
	{ eg_tool_t::PACK , EGPack_main , "PACK" , "PACK" },
	{ eg_tool_t::MAP , EGMap_main , "MAP" , "MAP" },
	{ eg_tool_t::MAKEWEBSERVICE , EGMakeWebService_main , "MAKEWEBSERVICE" , "MAKEWEBSERVICE" },
	{ eg_tool_t::CODELOC , EGMakeCodeLoc_main , "CODELOC" , "CODELOC" },
	{ eg_tool_t::RFLGEN , EGRflGen_main , "RFLGEN" , "RFLGEN" },
	{ eg_tool_t::BUILDSYNC , EGMakeBuildSync_main , "BUILDSYNC" , "BUILDSYNC" },
	{ eg_tool_t::VSGEN , EGMakeVSGen_main , "VSGEN" , "VSGEN" },
	{ eg_tool_t::PREP_PLATFORM , EGMakePrepPlatform_main , "PREP_PLATFORM" , "PREP_PLATFORM" },
	{ eg_tool_t::BUILD_DIST , EGMakeBuildDist_main , "BUILD_DIST" , "BUILD_DIST" },
	{ eg_tool_t::GENERATE_BUILD_INFO , EGMake_GenerateBuildInfo_main , "GENERATE_BUILD_INFO" , "GENERATE_BUILD_INFO" },
	{ eg_tool_t::STEAMVDF , EGMake_SteamVDF_main , "STEAMVDF" , "STEAMVDF" },
};

static egToolTypeItem EGMake_FindToolTypeFromCmdLine( eg_cpstr InCmdLine )
{
	for( eg_int i=0; i<countof(EGMake_ToolTypeTable); i++ )
	{
		if( EGString_EqualsI( EGMake_ToolTypeTable[i].CmdLine , InCmdLine ) )
		{
			return EGMake_ToolTypeTable[i];
		}
	}

	return { eg_tool_t::UNK , nullptr , "" , "" };
}

//
// Entry Point
//
int main( int argc , char* argv[] )
{
	EGToolMem_Init();
	EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t::OS );
	EGLog_SetHandler( EGMake2_LogHandler );
	EGLog_SetChannelSuppressed( eg_log_t::Thread , true );
	EGToolsHelper_SetPathEnv();
	const eg_bool bIsInSolutionDir = EGToolsHelper_SetDirToSolutionDir();

	if( !bIsInSolutionDir )
	{
		EGLogf( eg_log_t::Warning , "egmake could not find the MSBuild solution (EG.sln). Not all tools may work." );
	}

	egToolTypeItem ToolTypeData = { eg_tool_t::UNK , nullptr , "" , "" };

	for( eg_int i=0; i<argc && eg_tool_t::UNK == ToolTypeData.ToolType; i++ )
	{
		ToolTypeData = EGMake_FindToolTypeFromCmdLine( argv[i] );
	}

	EGMake_ToolTypeText = ToolTypeData.TextId;

	int RetValue = 0;

	EGCrcDb::Init();

	if( ToolTypeData.ToolType == eg_tool_t::UNK )
	{
		EGLogf( eg_log_t::Error , "No tool type specified." );
		RetValue = -1; 
	}
	else if( ToolTypeData.ToolFn )
	{
		RetValue = ToolTypeData.ToolFn( argc , argv );
	}
	else
	{
		EGLogf( eg_log_t::Error , "No tool handler specified." );
		RetValue = -1; 
	}

	EGCrcDb::Deinit();

	EGToolMem_Deinit();

	return RetValue;
}

eg_bool EGMake_WriteOutputFile(eg_cpstr strFile, const eg_byte* pData, eg_size_t Size)
{
	return EGMake_WriteOutputFile( strFile , EGFileData( pData , Size ) );
}

eg_bool EGMake_WriteOutputFile( eg_cpstr strFile, const EGFileData& MemFile )
{
	return EGLibFile_SaveFile( EGString_ToWide(strFile) , eg_lib_file_t::OS , MemFile );
}

eg_bool EGMake_ReadInputFile( eg_cpstr strFile, class EGFileData& MemFile )
{
	return EGLibFile_OpenFile( EGString_ToWide(strFile) , eg_lib_file_t::OS , MemFile );
}

///////////////////////////////////////////////////////////////////////////////
/// Paths
///////////////////////////////////////////////////////////////////////////////
static const eg_uint EGR_MAX_PATH = _MAX_DRIVE+_MAX_PATH+_MAX_FNAME+_MAX_EXT+20;

static struct
{
	eg_char sFull[EGR_MAX_PATH];
	eg_char sNoExt[EGR_MAX_PATH];
	eg_char sDirOnly[EGR_MAX_PATH];
	eg_char sExtOnly[EGR_MAX_PATH];
	eg_char sShort[EGR_MAX_PATH];
	eg_char sShortNoExt[EGR_MAX_PATH];
}
g_FileInfo[2];

void EGRMake_InitPaths(eg_cpstr strIn, eg_cpstr strOut)
{
	eg_char strDrive[_MAX_DRIVE];
	eg_char strPath[_MAX_PATH];
	eg_char strFile[_MAX_FNAME];
	eg_char strExt[_MAX_EXT];

	for(eg_uint i=0; i<2; i++)
	{
		eg_cpstr str = (0==i) ? strIn : strOut;
		if( str )
		{
			_splitpath_s( str, strDrive, countof(strDrive), strPath, countof(strPath), strFile, countof(strFile), strExt, countof(strExt));

			wsprintfA(g_FileInfo[i].sFull       , ("%s%s%s%s" ), strDrive, strPath, strFile, strExt);
			wsprintfA(g_FileInfo[i].sNoExt      , ("%s%s%s"   ), strDrive, strPath, strFile);
			wsprintfA(g_FileInfo[i].sDirOnly    , ("%s%s"     ), strDrive, strPath);
			wsprintfA(g_FileInfo[i].sExtOnly    , ("%s"       ), strExt);
			wsprintfA(g_FileInfo[i].sShort      , ("%s%s"     ), strFile, strExt);
			wsprintfA(g_FileInfo[i].sShortNoExt , ("%s"       ), strFile);
		}
	}

	EGMake_SetActualOutputFile((""));
}

eg_cpstr EGMake_GetInputPath(EGR_FINFO_T t)
{
	switch(t)
	{
	case FINFO_FULL_FILE   : return g_FileInfo[0].sFull;
	case FINFO_NOEXT_FILE  : return g_FileInfo[0].sNoExt;
	case FINFO_DIR         : return g_FileInfo[0].sDirOnly;
	case FINFO_EXT         : return g_FileInfo[0].sExtOnly;
	case FINFO_SHORT       : return g_FileInfo[0].sShort;
	case FINFO_SHORT_NOEXT : return g_FileInfo[0].sShortNoExt;
	}
	return g_FileInfo[0].sFull;
}

eg_cpstr EGMake_GetOutputPath(EGR_FINFO_T t)
{
	switch(t)
	{
	case FINFO_FULL_FILE   : return g_FileInfo[1].sFull;
	case FINFO_NOEXT_FILE  : return g_FileInfo[1].sNoExt;
	case FINFO_DIR         : return g_FileInfo[1].sDirOnly;
	case FINFO_EXT         : return g_FileInfo[1].sExtOnly;
	case FINFO_SHORT       : return g_FileInfo[1].sShort;
	case FINFO_SHORT_NOEXT : return g_FileInfo[1].sShortNoExt;
	}
	return g_FileInfo[1].sFull;
}


static eg_string_big g_sFinal;

void EGMake_SetActualOutputFile(eg_cpstr strFilename)
{
	g_sFinal = strFilename;
}

eg_cpstr EGMake_GetActualOutputFile()
{
	return g_sFinal.String();
}

eg_cpstr EGMake_GetEGSRC()
{
	static eg_string_big String;
	if( String.Len() == 0 )
	{
		String = EGToolsHelper_GetEnvVar( "EGSRC" );
	}
	return String;
}

eg_cpstr EGMake_GetEGOUT()
{
	static eg_string_big String;
	if( String.Len() == 0 )
	{
		String = EGToolsHelper_GetEnvVar( "EGOUT" );
	}
	return String;
}

///////////////////////////////////////////////////////////////////////////////

eg_bool EGMakeRes_Ignore()
{
	EGLogf( eg_log_t::Verbose , ("   Notice: Skipping \"%s\". \"%s\" is not a game format."), EGMake_GetInputPath(FINFO_SHORT), EGMake_GetInputPath(FINFO_EXT));
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Lib
///////////////////////////////////////////////////////////////////////////////

#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"

static void EGMake2_LogHandler( eg_log_channel LogChannel , const eg_string_base& LogString )
{
	unused( LogChannel );

	eg_char FinalString[2048];
	EGString_FormatToBuffer( FinalString , countof(FinalString) , "[%s] %s\n" , EGMake_ToolTypeText , LogString.String() );
	if( eg_log_t::Verbose != LogChannel )
	{
		printf( "%s" , FinalString );
		OutputDebugStringA(FinalString);
	}
}

#include "EGLibExtern_Tool.hpp"

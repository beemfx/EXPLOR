// (c) 2017 Beem Media

#include "EGmake.h"
#include "EGToolsHelper.h"
#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"
#include "EGMakeBuildConfig.h"
#include "EGOsFile.h"
#include "EGExtProcess.h"
#include "EGPath2.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGEdUtility.h"

static eg_bool EGMake_SetGameBuild( eg_cpstr GameBuildStr );
static bool EGMake_ShaderHeader( eg_cpstr Path , eg_uint HlslVersion );

static eg_char EGMake_EgOut[eg_string_big::STR_SIZE];
static eg_char EGMake_EgSrc[eg_string_big::STR_SIZE];

typedef EGArray<eg_string_big> EGStringList;

static void EGMake_Print( eg_cpstr Str )
{
	EGLogf( eg_log_t::Warning , "%s" , Str );
}

eg_string_big EGMake_GetGameToBuildFromReg()
{
	eg_string_big Out = EGToolsHelper_GetEnvVar( "EGGAME" );

	if( 0 == Out.Len() )
	{
		EGMake_Print( "Warning: The environment variable EGGAME is not set, assuming \"testgame\". Please use EGEdConfig_x64.exe to set a game build." );
		Out = "testgame";
	}

	return Out;
}

eg_bool EGMake_HasDataChanged( eg_cpstr Directory , eg_cpstr CheckpointFile )
{
	egPathParts2 CheckpointParts = EGPath2_BreakPath( CheckpointFile );
	EGOsFile_CreateDirectory( *CheckpointParts.GetDirectory('/') );

	eg_int NumDiffs = 0;
	bool Res = false;
	eg_string_big Cmd =  EGString_Format( "diffdir.exe -Q -D \"%s\" -O \"%s\"" , Directory , CheckpointFile );
	Res = EGExtProcess_Run( Cmd , &NumDiffs );
	if( !Res )
	{
		EGMake_Print( "ERROR: Failed to run diffdir." );
		return false; 
	}

	return ( 0 != NumDiffs );
}

static void EGMake_BuildDataDir( eg_cpstr Src , eg_cpstr Dest , eg_cpstr Name , eg_cpstr FinalName , eg_cpstr PakFile , eg_bool AlwaysBuild )
{
	EGMake_Print( EGString_Format( "%s -> %s" , Src , PakFile ) );
	EGOsFile_CreateDirectory( Dest );
	//Check for changes:
	eg_string_big DiffFile = *EGPath2_CleanPath( EGString_Format( "%s/dbdiff/pakdata/%s/%s.egdiff" , EGToolsHelper_GetEnvVar("EGOUT").String() , FinalName , Name ) , '/' );
	eg_bool bDataChanged = EGMake_HasDataChanged( Src , DiffFile );

	if( !AlwaysBuild && !bDataChanged )
	{
		EGMake_Print( "No changes detected. Skipping." );
		return;
	}
	//Create the list of files:
	eg_string_big ListFilename = EGString_Format( "%s/../%s_list.txt" , Dest , Name );

	EGOsFileWantsFileFn WantsFile = []( eg_cpstr Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
	{
		unused( Filename );
		if( FileAttr.bHidden )
		{
			return false;
		}
		return true;
	};

	EGArray<eg_d_string> FileList;
	EGOsFile_FindAllFiles( Src , WantsFile , FileList );

	eg_string_big Cmd;

	//Run egresource on all the files:
	for( eg_uint i=0; i<FileList.LenAs<eg_uint>(); i++ )
	{
		eg_string_big SourceFile = *EGPath2_CleanPath( *EGSFormat8( "{0}/{1}" , Src , *FileList[i] ) , '/' );
		eg_string_big DestFile = EGString_Format( "%s/%s" , Dest , *FileList[i] );

		EGOsFile_CreateDirectory( *EGPath2_BreakPath( DestFile ).GetDirectory() );

		// EGMake_Print( EGString_Format( "%s -> %s" , Manifest[i].String() , DestFile.String() ) );
		Cmd = EGString_Format( "egmake2_x64.exe RESOURCE -in \"%s\" -out \"%s\"" , SourceFile.String() , DestFile.String() );
		// EGMake_Print( Cmd );
		EGExtProcess_Run( Cmd , nullptr );
	}

	// Create the pak file:
	Cmd = EGString_Format( "egmake2_x64.exe PACK -q -s \"%s\" -d \"%s\" -lp \"%s\"" , Dest , PakFile , Name );
	// EGMake_Print( Cmd );
	EGExtProcess_Run( Cmd , nullptr );
}

static bool EGMake_DataDir( eg_cpstr Path , eg_cpstr FinalName , eg_bool bIsEngine )
{
	eg_string_big OutPath = EGMake_EgOut;
	OutPath.Replace( "\\" , "/" );
	if( !EGString_EndsWith( OutPath , "/" ) )
	{
		OutPath.Append( "/" );	
	}
	OutPath.Append( EGString_Format( "databuild/%s/" , FinalName ) );

	eg_string_big InPath = EGMake_EgSrc;
	InPath.Replace( "\\" , "/" );
	if( !EGString_EndsWith( InPath , "/" ) )
	{
		InPath.Append( "/" );
	}
	InPath.Append( EGString_Format( bIsEngine ? "%s/egdata/" : "%s/data/" , Path ) );

	EGMake_Print( EGString_Format( ( "Building \"%s\" -> \"%s\"" ) , InPath.String() , OutPath.String() ) );

	eg_string CheckpointFile = EGString_Format( "%s/dbdiff/fulldata/%s.egidff" , EGToolsHelper_GetEnvVar("EGOUT").String() , FinalName );
	eg_bool bPathHasDiffs = EGMake_HasDataChanged( InPath , CheckpointFile );
	if( !bPathHasDiffs )
	{
		EGMake_Print( "No changes detected. Nothing to build." );
		return true;
	}
	
	//First thing to do is get a list of the directories in question:
	EGStringList DirList;

	//Find Directories:
	{
		WIN32_FIND_DATAA FindData;
		zero( &FindData );
		eg_string_big FindPath = InPath.String();
		FindPath.Append( "*" );
		HANDLE hFind = FindFirstFileA( FindPath , &FindData );
		if( INVALID_HANDLE_VALUE == hFind )
		{
			EGMake_Print( "No directories found, nothing to build." );
			return false;
		}

		do
		{
			//We only want to find directories, if we find a file, spit a warning
			//that this file won't be built.
			eg_string_big FoundFile = FindData.cFileName;

			if( FoundFile.Equals( "." ) || FoundFile.Equals( ".." ) )
			{
				//Skip self and prev dir.
			}
			else if( (FILE_ATTRIBUTE_DIRECTORY&FindData.dwFileAttributes) != 0 )
			{
				DirList.Push( FoundFile );
			}
			else
			{
				EGMake_Print( EGString_Format( "WARNING: %s is not a directory, this file will not be built." , FindData.cFileName ) );
			}
			FindData = FindData;
		} while (FindNextFileA( hFind , &FindData ) );
	}

	eg_string_big PakDest = EGMake_EgOut;
	PakDest.Replace( "\\" , "/" );
	if( !EGString_EndsWith( PakDest , "/" ) )
	{
		PakDest.Append( "/" );
	}
	PakDest.Append( EGString_Format("bin/%s" , FinalName) );

	EGOsFile_CreateDirectory( PakDest );

	for( eg_uint i=0; i<DirList.LenAs<eg_uint>(); i++ )
	{
		eg_string_big Src = InPath;
		Src.Append( DirList[i] );
		eg_string_big Dest = OutPath;
		Dest.Append( DirList[i] );
		eg_string_big PakFile = EGString_Format( "%s/%s.lpk" , PakDest.String() , DirList[i].String() );
		EGMake_BuildDataDir( Src , Dest , DirList[i] , FinalName , PakFile , false );
	}

	return true;
}

static void EGMake_HaveToolsChanged( eg_bool bCleanIfTheyHave )
{	
	eg_string_big EgOutStr = EGMake_EgOut;
	EgOutStr.Replace( "\\" , "/" );
	if( !EGString_EndsWith( EgOutStr , "/" ) )
	{
		EgOutStr.Append( "/" );
	}
	eg_string_big EgSrcStr = EGMake_EgSrc;
	EgSrcStr.Replace( "\\" , "/" );
	if( !EGString_EndsWith( EgSrcStr , "/" ) )
	{
		EgSrcStr.Append( "/" );
	}

	eg_string_big DatabuildDir = EgOutStr;
	DatabuildDir.Append( "databuild/" );
	eg_string_big DatabuildDiffDir = EgOutStr;
	DatabuildDiffDir.Append( "dbdiff/tools/" );

	EGOsFile_CreateDirectory( DatabuildDir );
	EGOsFile_CreateDirectory( DatabuildDiffDir );

	eg_string_big SourceToolsDiff = EGString_Format( "%sSourceTools.egdiff" , DatabuildDiffDir.String() );
	eg_string_big EGMakeExeDiff = EGString_Format( "%segmake2.egdiff" , DatabuildDiffDir.String()  );

	eg_string_big SourceToolsPath = EGString_Format( "%stools/bin" , EgSrcStr.String() );
	eg_string_big EGMakeExePath = EGString_Format( "%sbin/egmake2_x64.exe" , EgOutStr.String() );

	eg_bool bSrcToolsChanged = EGMake_HasDataChanged( SourceToolsPath , SourceToolsDiff );
	eg_bool bBuildToolsChanged = EGMake_HasDataChanged( EGMakeExePath , EGMakeExeDiff );

	if( (bSrcToolsChanged || bBuildToolsChanged ) && bCleanIfTheyHave )
	{
		if( EGToolsHelper_GetEnvVar( "EGNOTOOLDELTA" ).ToBool() )
		{
			EGLogf( eg_log_t::General , "Tools changed, data rebuild ignored." );
		}
		else
		{
			EGLogf( eg_log_t::General , "Tools changed, cleaning databuild..." );
			EGEdUtility_CleanGameData( false );
			// Re-create the tools checkpoint (since it was cleaned above)
			EGMake_HaveToolsChanged( false );
		}
	}
}

static eg_bool EGMake_Data()
{
	EGMake_HaveToolsChanged( true );

	EGMake_ShaderHeader( EGString_Format( "%s/generated_code/ShaderConsts_3.hlsli" , EGMake_EgOut ) , 3 );
	EGMake_ShaderHeader( EGString_Format( "%s/generated_code/ShaderConsts_5.hlsli" , EGMake_EgOut ) , 5 );

	eg_string_big GameDir = EGMake_GetGameToBuildFromReg();

	EGMakeBuildConfig::Get().LoadGameConfig( "core" );
	EGMakeBuildConfig::Get().LoadGameConfig( GameDir );

 	eg_bool bMadeCore = EGMake_DataDir( "core" , "egdata" , true );
	eg_bool bMadeGame = false;
	if( bMadeCore )
	{
		bMadeGame = EGMake_DataDir( EGString_Format( "games/%s" , GameDir.String() ) , GameDir , false );
	}

	EGMakeBuildConfig::Get().ClearConfig();

	return bMadeCore && bMadeGame;
}

static bool EGMake_CreateGameIni()
{
	eg_string_big Path = EGMake_EgOut;
	Path.Replace( "\\" , "/" );
	if( !EGString_EndsWith( Path , "/" ) )
	{
		Path.Append( "/" );
	}
	Path.Append( "bin/" );
	EGOsFile_CreateDirectory( Path );
	Path.Append( "egame.ini" );

	eg_string_big GameBuild = EGMake_GetGameToBuildFromReg();
	EGMake_Print( EGString_Format( "Creating %s (Current Game: \"%s\")." , Path.String() , GameBuild.String()) );

	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	FileData.WriteStr8( "Emergence Game Engine " );
	FileData.WriteStr8( __DATE__ );
	FileData.WriteStr8( " " );
	FileData.WriteStr8( __TIME__ );

	return EGLibFile_SaveFile( Path , eg_lib_file_t::OS , FileData );
}

#include "EGRendererTypes.h"
static bool EGMake_ShaderHeader( eg_cpstr Path , eg_uint HlslVersion )
{
	EGMake_Print( EGString_Format( "Building shader header %u -> \"%s\"" , HlslVersion , Path ) );

	EGOsFile_CreateDirectory( *EGPath2_BreakPath( Path ).GetDirectory() );

	static const eg_char sMatrix[] = "float4x4 g_%s : register(%s, %s ): %s;";

	FILE* File = nullptr;
	errno_t Res = fopen_s( &File , Path , "wb" );
	if( 0 != Res )
	{
		EGLogf( eg_log_t::General , "egshaderbuild Error: Could not open \"%s\"" , Path );
		return false;
	}

	eg_uint Offset = 32187; //Put junk in to make sure that the include file initializes this correctly.
	eg_string_big Line( ( "JUNK_TEXT" ) );
	eg_cpstr sShaderType = 0;

	static_assert( sizeof(EGLight)%(sizeof(eg_real)*4) == 0 , "Light does not fit exactly in register." );
	static_assert( sizeof(EGMaterial)%(sizeof(eg_real)*4) == 0 , "Material does not fit exactly in register." );
	
	static const eg_uint LIGHT_REG_SIZE = sizeof(EGLight)/(sizeof(eg_real)*4);
	static const eg_uint MATERIAL_SIZE = sizeof(EGMaterial)/(sizeof(eg_real)*4);
	
	if( 3 == HlslVersion )
	{
		#define SHADER_BUILD_DECL3
		#include "EGVertexTypes.inc"
		#include "EGRenderTypes.inc"
		#undef SHADER_BUILD_DECL3
		
		sShaderType = ("vs");
		#define VS_CONSTS
		#define SHADER_BUILD_DECL3
		#include "EGShaderConsts.inc"
		#undef SHADER_BUILD_DECL3
		#undef VS_CONSTS

		sShaderType = ("ps");
		#define PS_CONSTS
		#define SHADER_BUILD_DECL3
		#include "EGShaderConsts.inc"
		#undef SHADER_BUILD_DECL3
		#undef PS_CONSTS
	}
	else if( 5 == HlslVersion )
	{
		#define SHADER_BUILD_DECL5
		#include "EGVertexTypes.inc"
		#include "EGRenderTypes.inc"
		#undef SHADER_BUILD_DECL5

		sShaderType = ("vs");
		#define VS_CONSTS
		#define SHADER_BUILD_DECL5
		#include "EGShaderConsts.inc"
		#undef SHADER_BUILD_DECL5
		#undef VS_CONSTS

		sShaderType = ("ps");
		#define PS_CONSTS
		#define SHADER_BUILD_DECL5
		#include "EGShaderConsts.inc"
		#undef SHADER_BUILD_DECL5
		#undef PS_CONSTS
	}

	fclose( File );
	return true;
}

static void EGMake_SetToolsEnv()
{
	eg_string_big StrSrcTools = EGToolsHelper_GetEnvVar( "EGSRC" );
	StrSrcTools.Append( "/core/BuildTools/bin" );

	eg_string_big StrBuildTools = EGToolsHelper_GetEnvVar( "EGOUT" );
	StrBuildTools.Append( "/bin" );

	const eg_d_string8 PathStr = "_BUILD\\bin;core\\BuildTools\\bin";// EGSFormat8( "{0}\\\\_BUILD\\\\bin;{0}\\\\tools\\\\bin" , *EGOsFile_GetCWD() );
	_putenv_s( "PATH" , *PathStr );
}

int EGMake_main( int argc , char *argv[] )
{
	const eg_bool bInCorrectDir = EGToolsHelper_SetDirToSolutionDir();

	if (!bInCorrectDir)
	{
		EGMake_Print( "Couldn't find the solution file." );
		return -1;
	}

	EGString_Copy( EGMake_EgSrc , L"." , countof(EGMake_EgSrc) );
	EGString_Copy( EGMake_EgOut , L"_BUILD" , countof(EGMake_EgOut) );

	EGMake_SetToolsEnv();

	eg_string_big InputType;
	eg_string_big OutputPath;

	enum MAKE_T
	{
		NOT_FOUND,
		DATA,
		SHADER_HEADER3,
		SHADER_HEADER5,
		CREATE_GAME_INI,
		SET_GAME_BUILD,
	} Type = NOT_FOUND;

	for( int i=0; i<argc; i++ )
	{
		eg_string_big Parm = argv[i];

		switch( Type )
		{
			case NOT_FOUND:
			{
				if( Parm.EqualsI( "DATA" ) )
				{
					Type = DATA;
				}
				else if( Parm.EqualsI( "SHADER_HEADER3" ) )
				{
					Type = SHADER_HEADER3;
				}
				else if( Parm.EqualsI( "SHADER_HEADER5" ) )
				{
					Type = SHADER_HEADER5;
				}
				else if( Parm.EqualsI( "CREATE_GAME_INI" ) )
				{
					Type = CREATE_GAME_INI;
				}
				else if( Parm.EqualsI( "SET_GAME_BUILD" ) )
				{
					Type = SET_GAME_BUILD;
				}
				else
				{
					//Junk keyword.
				}
			} break;

			case DATA:
			{
				InputType = Parm;
			} break;

			case SET_GAME_BUILD:
			case CREATE_GAME_INI:
			case SHADER_HEADER3:
			case SHADER_HEADER5:
			{
				//Treat anything as the output directory.
				OutputPath = Parm;
			} break;
		}
	}

	bool Success = true;

	switch( Type )
	{
		case NOT_FOUND: 
			EGMake_Print( ( "Usage: egmake MAKETYPE [PARMS]" ) );
			EGMake_Print( ( "MAKETYPES:" ) );
			EGMake_Print( ( "   DATADIR - Builds a directory of game data." ) );
			EGMake_Print( ( "   SHADER_HEADER - Builds the shader header file and ouptuts it to [PARMS]." ) );
			EGMake_Print( ( "   CREATE_GAME_INI - Set's the game ini file as [PARMS]." ) );
			break;
		case DATA           : Success = EGMake_Data(); break;
		case SHADER_HEADER3 : Success = EGMake_ShaderHeader( OutputPath , 3 ); break;
		case SHADER_HEADER5 : Success = EGMake_ShaderHeader( OutputPath , 5 ); break;
		case CREATE_GAME_INI: Success = EGMake_CreateGameIni(); break;
		case SET_GAME_BUILD : Success = EGMake_SetGameBuild( OutputPath ); break;
	}

	return Success ? 0 : -1;
}

static eg_bool EGMake_SetGameBuild( eg_cpstr GameBuildStr )
{
	EGMake_Print( EGString_Format( "Setting the game build (EGGAME) to \"%s\"..." , GameBuildStr ) );
	EGToolsHelper_SetEnvVar( "EGGAME" , GameBuildStr );
	return true;
}

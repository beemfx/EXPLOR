// (c) 2016 Beem Media

#include "EGToolsHelper.h"
#include "EGWindowsAPI.h"
#include "EGStdLibAPI.h"
#include "EGPath2.h"
#include "EGLibFile.h"
#include "EGOsFile.h"

struct egBuildVar
{
	eg_cpstr8 VarName = "";
	eg_char16 Value[_MAX_PATH] = { };
};

static egBuildVar EGToolsHelper_BuildVars[] = 
{
	{ "EGGAME" , L"" } ,
	{ "EGNOAUTOBUILD" , L"" } ,
	{ "EGNOTOOLDELTA" , L"" } ,
	{ "EGSRC" , L"." } ,
	{ "EGOUT" , L"_BUILD" } ,
	{ "EGSYNCHOST" , L"" } ,
	{ "EGWEBSVCHOST" , L"" } ,
	{ "BEEMOUT" , L"" } ,
	{ "P4IGNORE" , L"" } ,
	{ "EGP4SERVER" , L"" } ,
	{ "EGP4CLIENT" , L"" } ,
	{ "EGP4USER" , L"" } ,
};

eg_string_big EGToolsHelper_GetDefaultGameName()
{
#define EG_DECLARE_GAME(_GameName_) #_GameName_
	static eg_cpstr GameName =
#include "../../EGEngine.egconfig"
		;

	return GameName;
}

eg_string_big EGToolsHelper_GetEnvVar( eg_cpstr Var , eg_bool bForceFromRegistry /* = false */ )
{
	unused(bForceFromRegistry);

	return *EGToolsHelper_GetBuildVar( Var );
}

eg_s_string_big16 EGToolsHelper_GetBuildVar( eg_cpstr Var )
{
	const eg_bool bIsGameVar = EGString_EqualsI( Var , "EGGAME" );

	for( auto& Item : EGToolsHelper_BuildVars )
	{
		if( EGString_EqualsI( Item.VarName , Var ) )
		{
			if (bIsGameVar && EGString_StrLen( Item.Value ) == 0 )
			{
				return *EGToolsHelper_GetDefaultGameName();
			}

			return Item.Value;
		}
	}

	return "";
}

void EGToolsHelper_SetBuildVar( eg_cpstr Var, eg_cpstr16 NewValue )
{
	for( auto& Item : EGToolsHelper_BuildVars )
	{
		if( EGString_EqualsI( Item.VarName , Var ) )
		{
			EGString_Copy( Item.Value , NewValue, countof(Item.Value) );
		}
	}
}

void EGToolsHelper_SetEnvVar( eg_cpstr Var , eg_cpstr NewValue , eg_bool bForceFromRegistry /* = false */ )
{
	unused( bForceFromRegistry );

	EGToolsHelper_SetBuildVar( Var , *eg_d_string16(NewValue) );
}

void EGToolsHelper_DeleteEnvVar( eg_cpstr Name )
{
	unused( Name );
}

void EGToolsHelper_SetGameEditorConfigSetting( eg_cpstr Var , eg_cpstr16 NewValue )
{
	const eg_d_string16 FullKeyPath = EGSFormat16( L"Software\\\\EGEngine\\\\Editor\\\\{0}\\\\Config" , EGToolsHelper_GetEnvVar( "EGGAME" ).String() );

	HKEY RegKey;
	DWORD Disp = 0;
	const LSTATUS Res = RegCreateKeyExW( HKEY_CURRENT_USER , *FullKeyPath , 0 , nullptr , REG_OPTION_NON_VOLATILE , KEY_SET_VALUE , nullptr , &RegKey , &Disp );
	if( ERROR_SUCCESS == Res )
	{
		const LSTATUS SetRes = RegSetValueExW( RegKey , *eg_s_string_sml16(Var) , 0 , REG_SZ , reinterpret_cast<const BYTE*>(NewValue) , EG_To<DWORD>(EGString_StrLen(NewValue)*sizeof(NewValue[0])) );
		if( ERROR_SUCCESS == SetRes )
		{
			
		}

		RegCloseKey( RegKey );
	}
}

eg_d_string16 EGToolsHelper_GetGameEditorConfigSetting( eg_cpstr Var )
{
	eg_d_string16 Out = CT_Clear;

	const eg_d_string16 FullKeyPath = EGSFormat16( L"Software\\\\EGEngine\\\\Editor\\\\{0}\\\\Config" , EGToolsHelper_GetEnvVar( "EGGAME" ).String() );
	
	HKEY RegKey;
	const LSTATUS Res = RegOpenKeyExW( HKEY_CURRENT_USER , *FullKeyPath , 0 , KEY_QUERY_VALUE , &RegKey );
	if( ERROR_SUCCESS == Res )
	{
		eg_char16 KeyValue[1024];
		DWORD KeySize = sizeof(KeyValue);
		DWORD Type = REG_NONE;
		const LSTATUS QueryRes = RegQueryValueExW( RegKey , *eg_s_string_sml16(Var) , NULL , &Type , reinterpret_cast<LPBYTE>(KeyValue) , &KeySize );
		if( ERROR_SUCCESS == QueryRes && REG_SZ == Type && KeySize < countof(KeyValue) )
		{
			KeyValue[KeySize/sizeof(KeyValue[0])] = '\0';
			Out = KeyValue;
		}
		
		RegCloseKey( RegKey );
	}

	return Out;
}

eg_bool EGToolsHelper_OpenFile( eg_cpstr16 Filename, class EGFileData& Out )
{
	return EGLibFile_OpenFile( Filename , eg_lib_file_t::OS , Out );
}

eg_bool EGToolsHelper_SaveFile( eg_cpstr16 Filename, const class EGFileData& In )
{
	return EGLibFile_SaveFile( Filename , eg_lib_file_t::OS , In );
}

void EGToolsHelper_SetPathEnv()
{
	eg_string_big StrSrcTools = EGToolsHelper_GetEnvVar( "EGSRC" );
	StrSrcTools.Append( "/core/BuildTools/bin" );

	eg_string_big StrBuildTools = EGToolsHelper_GetEnvVar( "EGOUT" );
	StrBuildTools.Append( "/bin" );

	_putenv_s( "PATH" , EGString_Format( "%s;%s" , StrSrcTools.String() , StrBuildTools.String() ).String() );
}

eg_string EGToolsHelper_GameAssetPathToSource( eg_cpstr GameAssetPath , eg_cpstr GameName )
{
	eg_string PathOut;

	eg_string EgSrcPath = EGToolsHelper_GetEnvVar( "EGSRC" );

	egPathParts2 PathParts;
	EGPath2_BreakPath( *eg_d_string16(GameAssetPath) , PathParts );

	if( PathParts.Folders.IsValidIndex( 0 ) )
	{
		if( PathParts.Folders[0].EqualsI( L"game" ) )
		{
			PathParts.Folders.DeleteByIndex( 0 );
			PathParts.Ext = "egsnd";
			PathOut = *PathParts.ToString8();
			PathOut = EGString_Format( "%s%s/data/%s" , EgSrcPath.String() , GameName , PathOut.String() );
		}
		else if( PathParts.Folders[0].EqualsI( L"egdata" ) )
		{
			PathParts.Folders.DeleteByIndex( 0 );
			PathParts.Ext = "egsnd";
			PathOut = *PathParts.ToString8();
			PathOut = EGString_Format( "%s%s/data/%s" , EgSrcPath.String() , "core" , PathOut.String() );
		}

	}

	PathOut = *EGPath2_CleanPath( PathOut , '/' );

	return PathOut;
}

eg_d_string16 EGToolsHelper_GetRawAssetPathFromGameAssetPath( eg_cpstr16 GameAssetPathIn )
{
	// const egPathParts2 PathParts = EGPath2_BreakPath( FullPath );
	egPathParts2 RawAssetParts = EGPath2_BreakPath( GameAssetPathIn );
	for( eg_int i=0; i<RawAssetParts.Folders.LenAs<eg_int>(); i++ )
	{
		// Replace "data" with "data_src".
		if( RawAssetParts.Folders[i].EqualsI( L"data" ) )
		{
			RawAssetParts.Folders[i] = L"data_src";
			break;
		}
	}
	const eg_d_string16 RawAssetPath = RawAssetParts.ToString();
	return RawAssetPath;
}

eg_bool EGToolsHelper_SetDirToSolutionDir()
{
	const eg_d_string16 FileToFind = L"EG.sln";

	const eg_d_string16 DirsToTry[] =
	{
		L"." ,
		L".." ,
		L"../.." , // This should get there, the directory should be _BUILD\bin
		L"../../.." , // Running in debug this should get there, since the working directory will be the project dir by default.
		L"../../../.." , // One more for good measure.
	};


	// Try directories win the game's path.
	for( const eg_d_string16& RootDir : DirsToTry )
	{
		const eg_d_string16 Path = EGSFormat16( L"{0}/{1}" , *RootDir , *FileToFind );
		if( EGOsFile_DoesFileExist( *Path ) )
		{
			EGOsFile_SetCWD( *RootDir );
			break;
		}
	}

	const eg_bool bSuccess = EGOsFile_DoesFileExist( *FileToFind );
	return bSuccess;
}

void EGToolsHelper_InitEnvironment()
{
	const eg_bool bInGameDir = EGOsFile_DoesFileExist("egame.ini");
	const eg_bool bInSrcDir = EGOsFile_DoesFileExist("EG.sln");

	egPathParts2 CurDirPath = EGPath2_BreakPath( *EGOsFile_GetCWD() );
	if (CurDirPath.Filename.Len() > 0)
	{
		CurDirPath.Folders.Append(CurDirPath.Filename);
	}

	if (bInGameDir)
	{
		CurDirPath.Folders.Resize( CurDirPath.Folders.Len() - 2 );
		const eg_d_string16 NewSource = CurDirPath.GetDirectory();
		EGToolsHelper_SetEnvVar( "EGSRC" , *eg_d_string8(*NewSource) );
	}
	else if (bInSrcDir)
	{
		const eg_d_string16 NewSource = CurDirPath.GetDirectory();
		EGToolsHelper_SetEnvVar( "EGSRC" , *eg_d_string8( *NewSource ) );
	}
	else
	{
		// assert( false ); // Directory should have been set prior to this.
	}
}

void EGToolsHelper_GetCmdLineParms( const EGArray<eg_s_string_sml8>& ArgsArray , EGCmdLineParms& ParmsOut )
{
	for( const eg_s_string_sml8& Str : ArgsArray )
	{
		if( Str.Len() > 0 && Str[0] == '-' )
		{
			ParmsOut.AddParm( *eg_s_string_sml16(*Str) , L"" );
		}
		else
		{
			ParmsOut.AppendLastParm( *eg_s_string_sml16(*Str) );
		}
	}
}

void EGToolsHelper_GetCmdLineParms( int argc , char* argv[] , EGCmdLineParms& ParmsOut )
{
	EGArray<eg_s_string_sml8> ArgsArray;
	for( int i = 0; i < argc; i++ )
	{
		ArgsArray.Append( argv[i] );
	}

	EGToolsHelper_GetCmdLineParms( ArgsArray , ParmsOut );
}

EGCmdLineParms::EGCmdLineParms( int argc , char* argv[] )
{
	EGToolsHelper_GetCmdLineParms( argc , argv , *this );
}

EGCmdLineParms::EGCmdLineParms( const EGArray<eg_s_string_sml8>& ArgsArray )
{
	EGToolsHelper_GetCmdLineParms( ArgsArray , *this );
}

void EGCmdLineParms::AddParm( eg_cpstr16 Type , eg_cpstr16 Value )
{
	egParmInfo NewParm;
	NewParm.Type = Type;
	NewParm.Value = Value;
	m_Parms.Append( NewParm );
}

void EGCmdLineParms::AppendLastParm( eg_cpstr16 Value )
{
	if( m_Parms.Len() == 0 )
	{
		AddParm( L"" , Value );
	}
	else
	{
		if( m_Parms[m_Parms.Len() - 1].Value.Len() > 0 )
		{
			m_Parms[m_Parms.Len() - 1].Value.Append( ' ' );
		}

		m_Parms[m_Parms.Len() - 1].Value.Append( Value );
	}
}

eg_bool EGCmdLineParms::ContainsType( eg_cpstr16 Type ) const
{
	for( const egParmInfo& Info : m_Parms )
	{
		if( Info.Type.EqualsI( Type ) )
		{
			return true;
		}
	}
	return false;
}

eg_d_string16 EGCmdLineParms::GetParmValue( eg_cpstr16 Type ) const
{
	for( const egParmInfo& Info : m_Parms )
	{
		if( Info.Type.EqualsI( Type ) )
		{
			return Info.Value;
		}
	}
	return L"";
}

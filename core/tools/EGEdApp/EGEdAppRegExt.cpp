// (c) 2017 Beem Media

#include "EGEdAppRegExt.h"
#include "EGWindowsAPI.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "../EGEdResLib/resource.h"

void EGEdAppRegExt_Register()
{
	eg_char8 ThisAppFilename[MAX_PATH];
	GetModuleFileNameA( GetModuleHandleW(NULL) , ThisAppFilename , countof(ThisAppFilename) );

	egPathParts2 AppDirParts= EGPath2_BreakPath( ThisAppFilename );

	eg_string BinPath = *AppDirParts.GetDirectory( '\\' );

	// MessageBoxA( nullptr , EGString_Format( "The bin directory is \"%s\"." , BinPath.String() ) , "App" , MB_OK );

	eg_string_big EditorPath = EGString_Format( "%s%s" , BinPath.String() , *AppDirParts.GetFilename8() );
	EditorPath = *EGPath2_CleanPath( EditorPath , '\\' );

	eg_string_big EditorCmd = EGString_Format( "\"%s\" \"%%1\"" , EditorPath.String() );

	eg_bool bHadError = false;

	auto CreateExtension = [&bHadError,&BinPath,&EditorCmd]( eg_cpstr ExtStr , eg_uint IconIndex ) -> void
	{
		eg_string_big IconPath = EGString_Format( "%s\\EGEdResLib.dll,%u" , BinPath.String() , IconIndex );
		IconPath = *EGPath2_CleanPath( IconPath , '\\' );

		auto WriteValue = [&bHadError]( HKEY Owner , eg_cpstr ValueName , eg_cpstr Value ) -> void
		{
			if( Owner && Value )
			{
				eg_string ValueStr = Value;
				LONG SetRes = RegSetValueExA( Owner , ValueName , 0 , REG_SZ , reinterpret_cast<const BYTE*>(ValueStr.String()) , ValueStr.Len()*sizeof(eg_char) );
				if( ERROR_SUCCESS == SetRes )
				{

				}
				else
				{
					bHadError = true;
				}
			}
		};

		auto CreateKey = [&bHadError,&WriteValue]( HKEY Owner , eg_cpstr KeyName , eg_cpstr ValueName , eg_cpstr Value ) -> HKEY
		{
			HKEY KeyOut = nullptr;
			LONG Res = RegCreateKeyExA( Owner , KeyName , 0 , nullptr ,  REG_OPTION_NON_VOLATILE , KEY_SET_VALUE , nullptr , &KeyOut , nullptr );
			if( ERROR_SUCCESS == Res )
			{
				WriteValue( KeyOut , ValueName , Value );

				// RegCloseKey( KeyOut );
				// KeyOut = nullptr;
			}
			else
			{
				bHadError = true;
			}

			return KeyOut;
		};

		eg_string ExtHandler = EGString_Format( "EGEd%s" , ExtStr );

		HKEY ClassesRoot = CreateKey( HKEY_CURRENT_USER , "Software\\Classes" , nullptr , "" );
		HKEY ExtKey = CreateKey( ClassesRoot , ExtStr ,  nullptr , ExtHandler );
		HKEY ExtTypeKey = CreateKey( ClassesRoot , ExtHandler , nullptr , "" );
		HKEY ExtIconKey = CreateKey( ExtTypeKey , "DefaultIcon" , nullptr , IconPath );
		HKEY ExtOpenKey = CreateKey( ExtTypeKey , "shell\\open\\command" , nullptr , EditorCmd );
		// HKEY ExtEditKey = CreateKey( ExtTypeKey , "shell\\edit\\command" , nullptr , EditorCmd );

		// RegCloseKey( ExtEditKey );
		RegCloseKey( ExtOpenKey );
		RegCloseKey( ExtIconKey );
		RegCloseKey( ExtTypeKey );
		RegCloseKey( ExtKey );
		RegCloseKey( ClassesRoot );
	};

	CreateExtension( ".elyt" , -IDI_ELYT );
	CreateExtension( ".edef" , -IDI_EDEF );
	CreateExtension( ".egasset" , -IDI_EGASSET );
	CreateExtension( ".egsnd" , -IDI_EGASSET );
	CreateExtension( ".lpk" , -IDI_LPK );
	CreateExtension( ".egsm" , -IDI_EGSM );
	CreateExtension( ".egworld" , -IDI_EGWORLD );

	if( bHadError )
	{
		MessageBoxW( nullptr , L"Could not register file types." , L"App Name" , MB_OK );
	}

	SHChangeNotify( SHCNE_ASSOCCHANGED , SHCNF_IDLIST , 0 , 0 );
}

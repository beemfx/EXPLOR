/******************************************************************************
File: WinMain.cpp
Purpose: Game entry point, and runs the main game loop. All other features of
the game are managed within the EGEngine class. These functions handle all Window
management and is for the most part independent of the rest of the game.
The game itself is run in Window Loop with calls to Init, Loop, and Shutdown.
Additional input is provided to the game through the WM_CHAR message which
dumps characters into the input buffer, the rest of the game is independent of
windows with 1 exception, the video class will change the size of the window
if windowed mode was specified as an option.

All libraries are also linked in this file using #pragama comment(lib, "name")
instead of linking them within the project settings.

(c) 2011 Beem Software
******************************************************************************/

/****************
*** Libraries ***
****************/

#pragma comment(lib, "lib3p.lib")
#if WITH_NET_COMS
#pragma comment(lib, "ws2_32.lib")
#endif
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "xaudio2_9redist.lib")
#pragma comment(lib, "XInput.lib")


/**************
*** WinMain ***
**************/
#include "EGEngine.h"
#include "EGEngineConfig.h"
#include "EGInput.h"
#include "EGPlatform.h"
#include "EGEgWinCon.h"
#include "EGEngineApp.h"
#include "EGDebugText.h"
#include "EGClassImport.h"
#include "EGLibFile.h"
#include "EGEngineDataAssetLib.h"
#include "EGEngineMem.h"
#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"
#include "EGOsFile.h"

#if defined( __EGEDITOR__ )
#include "EGWnd.h"
#include "EGEditor.h"
#include "EGExtProcess.h"
#include "EGEdLib.h"
#endif

//The game
static struct egWinMain
{
public:
	HANDLE           MainThread;
	DWORD            MainThreadId;
	egEnginePlatformParms RunParms;
	eg_bool          ShowConsole:1;
	eg_bool          bNoAssetBuild:1;
public:
	egWinMain()
	: MainThread(nullptr)
	, MainThreadId(0)
	, ShowConsole(false)
	, bNoAssetBuild(false)
	, RunParms( CT_Clear )
	{
		RunParms.IsDedicatedServer = false;
	}

} WinMain_Data;

#include "EGLibExtern.h"
#include "EGLoader.h"
#include "EGFileData.h"

eg_bool EGLibExtern_LoadNowTo( eg_cpstr8 strFile , EGFileData& pFile )
{
	MainLoader->LoadNowTo(strFile, pFile);
	return pFile.GetSize() != 0;
}

eg_bool EGLibExtern_LoadNowTo( eg_cpstr16 strFile , EGFileData& pFile )
{
	return EGLibExtern_LoadNowTo( EGString_ToMultibyte(strFile) , pFile);
}

static void EGWinMain_LogHandler( eg_log_channel LogChannel , const eg_string_base& LogString )
{
	eg_string_small Prefix = EGLog_GetNameForChannel( LogChannel );
	eg_color32 Color = EGLog_GetColorForChannel( LogChannel );

	eg_bool  Ignore = false;
	eg_string ColorPrefix = EGString_Format( "&c!%08X" , Color );

	if( !Ignore )
	{
		eg_string_big Str = EGString_Format( "[%s] %s" , Prefix.String() , LogString.String() );
	
		OutputDebugStringA(Str);
		OutputDebugStringA("\r\n");

		if( ColorPrefix.Len() > 0 )
		{
			MainConsole.InsertString( ColorPrefix );
		}
		MainConsole.InsertString(Str.String());
		MainConsole.InsertString( "\n" );
		WinCon_RefreshCon(); //Safe to call even if there is no console.
	}
}

eg_bool EGPlatform_IsMainThread()
{
	return GetCurrentThreadId() == WinMain_Data.MainThreadId;
}

void EGPlatform_GetRunParms( struct egEnginePlatformParms* RunParmsOut )
{
	if( RunParmsOut )
	{
		*RunParmsOut = WinMain_Data.RunParms;
	}
}

static eg_bool WinMain_EnsureDirectory(LPWSTR szCommandLine);

/*** EnsureDirectory - Ensures that the current directory is where the
executable file is located. This function is modified for the debug version of
the game to insure that the current directory is the actual application path.
This function should only be called once from WinMain. ***/
static eg_bool WinMain_EnsureDirectory(LPWSTR szCommandLine)
{
	//First we'll just check if we're in a good directory.
	DWORD Att = GetFileAttributesW( L"./egame.ini" );
	if( INVALID_FILE_ATTRIBUTES != Att )return true;

	eg_char16 szNewDir[MAX_PATH+1];
	//MessageBox(0, _tgetcwd(szNewDir, MAX_PATH), 0, 0);
	
	//The directory only needs to be insured if we are running the release
	//version of the game. The debug version directory should be specified by
	//the project settings.
	//There is an initial quote to get rid of.
	szCommandLine++;
	
	//Now we loop through copying the new string until we find a quote, then
	//we end the string at the last \ found because that will get us the
	//directory the exe file is in.
	
	//int nLen = _tcslen(szCommandLine);
	
	for(int i=0, nLastSep=0; ;i++)
	{
		eg_char16 c = szCommandLine[i];
		
		if(c=='\\')
			nLastSep=i;
		
		if (c=='\"' || c==0)
		{
			szNewDir[nLastSep]=0;
			break;
		}
		else
		{
			szNewDir[i]=c;
		}
	}
	
	
	
	_wchdir(szNewDir);
	Att = GetFileAttributesW( L"./egame.ini" );
	if( INVALID_FILE_ATTRIBUTES != Att )
	{
		return true;
	}

	//Finally, we may be running from visual studio build, in that case use
	//the environment variable:
	eg_char EgOut[1024];
	GetEnvironmentVariableA( "EGOUT" , EgOut , countof(EgOut) );
	_chdir(EgOut);
	_chdir(".\\bin");
	Att = GetFileAttributesW( L"./egame.ini" );
	if( INVALID_FILE_ATTRIBUTES != Att )
	{
		return true;
	}
	return false;
}

#if defined( __DEBUG__ )
static _CrtMemState WinMain_MemCheckpoint;
#endif

static void WinMain_InitCrtMemDebug()
{
	#if defined( __DEBUG__ )
	//_crtBreakAlloc = 1334;
	_CrtMemCheckpoint(&WinMain_MemCheckpoint);
	#endif
}

static void WinMain_DeinitCrtMemDebug()
{
	#if defined( __DEBUG__ )
	_CrtMemState s2, s3;
	_CrtMemCheckpoint(&s2);
	_CrtMemDifference(&s3, &WinMain_MemCheckpoint, &s2);
	_CrtMemDumpStatistics(&s3);
	_CrtDumpMemoryLeaks();
	#endif
}

static void WinMain_ProcessINI()
{
	// At this point the egame.ini exists only so that EnsureDirectory works.
	HANDLE hFile = CreateFileW( (L"egame.ini"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		assert(false); //Game file not found.
		return;
	}
	CloseHandle(hFile);
}

static void WinMain_ProcessCmdLine( void )
{	
	LPWSTR sCmdLine = GetCommandLineW();

	enum
	{
		NONE,
		MAP,
		CONNECT,
	} NextType = NONE;
	
	int NumArgs = 0;
	LPWSTR* sArgs = ::CommandLineToArgvW( sCmdLine, &NumArgs);

	for(int i=0; i<NumArgs; i++)
	{
		eg_char sArg[1024];
		EGString_Copy(sArg, sArgs[i], countof(sArg));

		if( MAP == NextType )
		{
			WinMain_Data.RunParms.StartMap = sArg;
			NextType = NONE;
		}
		else if( CONNECT == NextType )
		{
			WinMain_Data.RunParms.RemoteHost = sArg;
			NextType = NONE;
		}
		else if(NONE == NextType)
		{
			if( EGString_Equals( "-dedicated" , sArg ))
			{
				WinMain_Data.RunParms.IsDedicatedServer = true;
			} 
			else if( EGString_Equals("-map" , sArg) )
			{
				NextType = MAP;
			}
			else if( EGString_Equals( "-connect" , sArg) )
			{
				NextType = CONNECT;
			}
			else if( EGString_Equals( "-wincon" , sArg) )
			{
				WinMain_Data.ShowConsole = true;
			}
			else if( EGString_Equals( "-nodatabuild" , sArg ) )
			{
				WinMain_Data.bNoAssetBuild = true;
			}
		}
	}
}

static void WinMain_CreateUserDir( eg_cpstr16 SaveFolder )
{
	auto CreateUserDir =[&SaveFolder]( eg_d_string16& Target , int csidl )
	{
		//We want to put save games and settings in a user folder.
		//We'll do the traditional my documents method.
		eg_char16 strTemp[MAX_PATH];
		::SHGetSpecialFolderPathW( NULL , strTemp , csidl , TRUE );
		Target = strTemp;
		if( !EGOsFile_DoesDirExist( *Target ) )
		{
			Target = L".\\"; // Fall back to game directory.
		}

		if( EGOsFile_DoesDirExist( *Target ) )
		{
			Target += L"\\";
			Target += SaveFolder;

			eg_bool bCreatedFolder = EGOsFile_CreateDirectory( *Target );
			if( !bCreatedFolder )
			{
				::MessageBoxW(nullptr, L"Could not create user directory.\nSaving may not be possible." , *Engine_GetGameTitle() , MB_ICONERROR);
			}
		}
		else
		{
			::MessageBoxW(nullptr, L"Could not find suitable location for save games.\nSaving may not be possible." , *Engine_GetGameTitle() , MB_ICONERROR);
		}
	};

	CreateUserDir( WinMain_Data.RunParms.UserDir , CSIDL_APPDATA );
	CreateUserDir( WinMain_Data.RunParms.SysCfgDir , CSIDL_LOCAL_APPDATA );
}

eg_platform_init_result EGWinMain_Init( eg_cpstr GameName , eg_cpstr16 SaveFolder )
{
	EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t::OS );
	EGLibFile_SetFunctions( eg_lib_file_t::GameData , EGEngineDataAssetLib_GameDataOpenFn , EGEngineDataAssetLib_GameDataSaveFn );
	EGLog_SetHandler( EGWinMain_LogHandler );
	WinMain_Data.MainThread = GetCurrentThread();
	WinMain_Data.MainThreadId = GetCurrentThreadId();
	//We always want the directory to be the directory where the game is
	//located. So that any other paths that might have been set when a shortcut
	//is run will not be the assumed game directory.
	eg_bool InCorrectDir = WinMain_EnsureDirectory(GetCommandLine());
	if( !InCorrectDir )
	{
	#if !defined(__EGEDITOR__)
		MessageBoxW( nullptr , L"The game assets could not be found." , *Engine_GetGameTitle() , MB_OK|MB_ICONERROR );
	#endif
		return eg_platform_init_result::BadDirectoryOrData;
	}
	WinMain_ProcessINI();
	WinMain_ProcessCmdLine();
	WinMain_CreateUserDir( SaveFolder );
	WinMain_Data.RunParms.ExeDir = L".\\";

	const eg_platform_init_result PlatformInitRes = EGPlatform_Init( GameName );

	return PlatformInitRes;
}

void EGWinMain_Deinit()
{
	WinMain_Data.RunParms.UserDir.Clear();
	WinMain_Data.RunParms.SysCfgDir.Clear();
	WinMain_Data.RunParms.ExeDir.Clear();
	EGPlatform_Deinit();
}


int EGEngineApp_Run( const egSdkEngineInitParms& InitParms )
{
	unused( InitParms );

	auto RunGameDataBuild = []() -> void
	{
#if defined( __EGEDITOR__ )
		{
			eg_int DataBuildResult = 0;
			eg_char BuildFilename[1024];
			if( GetModuleFileNameA( NULL , BuildFilename , countof(BuildFilename) ) )
			{
				eg_bool bRanDataBuild = EGExtProcess_Run( EGString_Format( "\"%s\" -databuild" , BuildFilename ) , &DataBuildResult );
			}
		}
#endif
	};

#if(_WIN32_WINNT >= 0x0600)
	SetProcessDPIAware();
#endif

	WinMain_InitCrtMemDebug();
	EGEngineMem_Init();

	EGLog_SetDefaultSuppressedChannels();

	EGClassImport_ImportClasses();

	eg_platform_init_result PlatformResult = EGWinMain_Init( InitParms.GameName , *InitParms.SaveFolder );

	if( PlatformResult == eg_platform_init_result::Failed3rdParty || PlatformResult == eg_platform_init_result::Restarting3rdParty )
	{
#if IS_PLATFORM_Steam
		if( PlatformResult != eg_platform_init_result::Restarting3rdParty )
		{
			MessageBoxW( NULL , L"Please launch the game through Steam." , *Engine_GetGameTitle() , MB_ICONERROR );
		}
#endif
		EGEngineMem_Deinit();
		WinMain_DeinitCrtMemDebug();
		return 0;
	}

	#if defined( __EGEDITOR__ )
	{
		eg_bool bWasEditor = false;
		int EditorResult = 0;
		{
			EGWndAppParms AppParms;
			EGWnd_GetAppParmsFromCommandLine( AppParms );

			eg_bool bShouldDoDataBuild = true;

			if( EGEditor_IsDataBuild( AppParms ) )
			{
				// Don't do the data-build if we are the data-build (since that would be recursively bad).
				bShouldDoDataBuild = false;
			}
			else if( WinMain_Data.bNoAssetBuild )
			{
				// Don't do the data-build if we were explicitly asked not to.
				bShouldDoDataBuild = false;
			}
			else if( !EGEdLib_IsAutoDataBuildEnabled() )
			{
				// Don't do the data-build if automatic data-build is disabled.
				bShouldDoDataBuild = false;
			}

			if( bShouldDoDataBuild )
			{
				RunGameDataBuild();
			}

			if( EGEditor_IsEditorCmdLine( AppParms ) )
			{
				EditorResult = EGEditor_Run( AppParms , InitParms );
				bWasEditor = true;
			}
		}

		if( bWasEditor )
		{
			EGWinMain_Deinit();
			EGEngineMem_Deinit();
			return EditorResult;
		}
	}
	#endif

	if( InitParms.bIsDedicatedServer )
	{
		WinMain_Data.RunParms.IsDedicatedServer = true;
	}

	if( !InitParms.bAllowDedicatedServer )
	{
		WinMain_Data.RunParms.IsDedicatedServer = false;
	}

	if( !InitParms.bAllowConsole )
	{
		WinMain_Data.ShowConsole = false;
	}

	if( PlatformResult != eg_platform_init_result::Success )
	{
		return 0;
	}

	eg_bool ShowConsole = WinMain_Data.RunParms.IsDedicatedServer || WinMain_Data.ShowConsole;

	if( ShowConsole )
	{
		WinCon_CreateCon( !WinMain_Data.RunParms.IsDedicatedServer );
	}

	eg_bool bRan = Engine_Run( InitParms );

	if(!bRan)
	{
		MessageBoxW( nullptr , L"An error occurred while running the game, see debug log for details." , *Engine_GetGameTitle() , MB_OK );
	}

	if( ShowConsole )
	{
		WinCon_DestroyCon();
	}

	EGWinMain_Deinit();

	EGEngineMem_Deinit();
	WinMain_DeinitCrtMemDebug();

	return 0;
}

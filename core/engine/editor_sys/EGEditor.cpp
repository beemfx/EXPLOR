// (c) 2017 Beem Media

#if defined( __EGEDITOR__ )

#include "EGEditor.h"
#include "EGWnd.h"
#include "EGCrcDb.h"
#include "EGDataAssetEditor.h"
#include "EGPath2.h"
#include "EGLytEd.h"
#include "EGDefEd.h"
#include "EGPlatform.h"
#include "EGEngineApp.h"
#include "EGResourceLib.h"
#include "EGLpkViewer.h"
#include "EGSmEd.h"
#include "EGEdUtility.h"
#include "EGEdDataBuild.h"
#include "EGEdLib.h"
#include "EGWorldEd.h"
#include "EGEditorResave.h"
#include "EGToolsHelper.h"

enum class eg_editor_t
{
	Auto,
	DataAsset,
	DefEd,
	LytEd,
	EgSmEd,
	LpkViewer,
	DataBuild,
	World,
};

struct egEditorApp
{
	eg_editor_t Type;
	eg_cpstr    Ext;
	eg_cpstr    CommandLine;
	int ( * Exec )( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms );

	egEditorApp() = default;
};

static const egEditorApp EGEditor_Launchers[] =
{
	{ eg_editor_t::Auto      , ""        , "-editor"   , nullptr },
	{ eg_editor_t::DataAsset , "egasset" , "-asseted"  , EGDataAssetEditor_Run },
	{ eg_editor_t::DataAsset , "egsnd"   , "-asseted"  , EGDataAssetEditor_Run },
	{ eg_editor_t::DefEd     , "edef"    , "-defed"    , EGDefEd_Run },
	{ eg_editor_t::LytEd     , "elyt"    , "-lyted"    , EGLytEd_Run },
	{ eg_editor_t::EgSmEd    , "egsm"    , "-egsmed"   , EGSmEd_Run },
	{ eg_editor_t::LytEd     , "lpk"     , "-lpkview"  , EGLpkViewer_Run },
	{ eg_editor_t::DataBuild , "__databuild__" , "-databuild", EGEdDataBuild_Run },
	{ eg_editor_t::World     , "egworld" , "-worlded" , EGWorldEd_Run },
	{ eg_editor_t::DataBuild , "__resave__" , "-resaveall" , EGEditor_Resave },
};

static void EGEditor_Init( const struct egSdkEngineInitParms& EngineParms )
{
	EGToolsHelper_InitEnvironment();
	EGLog_SetDefaultSuppressedChannels();
	EGPlatform_Init( EngineParms.GameName );
	EGCrcDb::Init();
	EGResourceLib_Init();
	EGEdUtility_Init();
}

static void EGEditor_Deinit()
{
	EGEdUtility_Deinit();
	EGResourceLib_Deinit();
	EGCrcDb::Deinit();   
	EGPlatform_Deinit();
}

int EGEditor_Run( const class EGWndAppParms& AppParms , const struct egSdkEngineInitParms& EngineParms )
{
	EGEdLib_Init();
	EGEditor_Init( EngineParms );

	egEditorApp EdApp;
	zero( &EdApp );
	eg_string_big InputFile = "";

	for( const egEditorApp& Launcher : EGEditor_Launchers )
	{
		if( AppParms.ContainsType( Launcher.CommandLine ) )
		{
			InputFile = AppParms.GetParmValue( Launcher.CommandLine );
			EdApp = Launcher;
			break;
		}
	}

	egPathParts2 PathParts = EGPath2_BreakPath( InputFile );

	if( EdApp.Type == eg_editor_t::Auto )
	{
		// We might not open the file with the editor requested if it has a different extension.
		for( const egEditorApp& Launcher : EGEditor_Launchers )
		{
			if( PathParts.Ext.EqualsI( *eg_d_string16(Launcher.Ext) ) )
			{
				EdApp = Launcher;
			}
		}
	}

	int AppRes = 0;

	if( EdApp.Exec )
	{
		EdApp.Exec( *PathParts.ToString( true , '\\' ) , EngineParms );
	}
	else
	{
		MessageBoxW( nullptr , L"No tool exists for this type yet." , L"EG" , MB_OK );
	}

	EGEditor_Deinit();

	return AppRes;
}

eg_bool EGEditor_IsEditorCmdLine( const class EGWndAppParms& AppParms )
{
	for( const egEditorApp& App : EGEditor_Launchers )
	{
		if( AppParms.ContainsType( App.CommandLine ) )
		{
			return true;
		}
	}

	return false;
}

eg_bool EGEditor_IsDataBuild( const class EGWndAppParms& AppParms )
{
	return AppParms.ContainsType( "-databuild" );
}

#endif

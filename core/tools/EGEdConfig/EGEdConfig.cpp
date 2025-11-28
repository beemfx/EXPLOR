// (c) 2018 Beem Media

#pragma comment( lib , "eglib.lib" )
#pragma comment( lib , "egfoundation.lib" )
#pragma comment( lib , "winhttp.lib" )
#pragma comment( lib , "lib3p.lib" )

#include "EGEdConfig.h"
#include "resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGWndPanelPropEditor.h"
#include "EGToolsHelper.h"
#include "EGToolMem.h"
#include "EGReflectionPrimitives.h"
#include "EGLibExtern_Tool.hpp"
#include "EGPath2.h"
#include "EGEdConfigSync.h"
#include "EGLibFile.h"
#include "EGTargetPlatform.h"

struct egEdConfigProps;

static const eg_char16 EGEdConfig_Title[] = L"EG Editor Config";

struct egEdConfigProp
{
	eg_string_small  m_EnvVar = "";
	eg_string_small  m_DisplayName = "";
	eg_bool          m_bHasChanged = false;
	eg_bool          m_bRequiresLogoff = false;
	eg_bool          m_bIsPath = false;
	egRflEditor      m_Editor = CT_Clear;
	egEdConfigProps* m_Owner = nullptr;

public:

	void SetEnvVar( eg_cpstr NewEnvVar )
	{
		m_EnvVar = NewEnvVar;
	}

	void SetOwner( egEdConfigProps* InOwner )
	{
		m_Owner = InOwner;
	}

	void SetHasChanged( eg_bool bHasChanged )
	{
		m_bHasChanged = bHasChanged;
	}

	void SetRequiresLogoff( eg_bool bNewValue )
	{
		m_bRequiresLogoff = bNewValue;
	}

	void SetDisplayName( eg_cpstr NewValue )
	{
		m_DisplayName = NewValue;
	}

	void SetIsPath( eg_bool NewValue )
	{
		m_bIsPath = NewValue;
	}

	eg_bool HasChanged() const { return m_bHasChanged; }
	eg_bool RequiresLogoff() const { return m_bRequiresLogoff; }

	void RefreshFromRegistery()
	{
		m_Editor.SetFromString( EGToolsHelper_GetEnvVar( m_EnvVar , true ) );
	}

	void SaveToRegistry()
	{
		// Make sure there is a slash at the end of paths
		eg_string MyValue = *m_Editor.ToString();

		if( m_bIsPath )
		{
			if( MyValue.Len() > 0 )
			{
				MyValue.Append( '\\' );
				MyValue = *EGPath2_CleanPath( MyValue , '\\' );
			}
		}

		eg_bool bIsFalseBool = m_Editor.GetValueType() == &Rfl_eg_bool && !EGString_ToBool( MyValue );

		if( MyValue.Len() > 0 && !bIsFalseBool )
		{
			EGToolsHelper_SetEnvVar( m_EnvVar , MyValue , true );
		}
		else
		{
			EGToolsHelper_DeleteEnvVar( m_EnvVar );
		}
	}

	eg_cpstr GetEnvVar() const { return m_EnvVar; }

	virtual eg_string_small GetDisplayName() const { return m_DisplayName; }
};

struct egEdConfigProps
{
	eg_combo_box_str_ed  m_GameToBuild           = "";
	eg_combo_box_str_ed  m_TargetPlatform        = "";
	eg_os_browse_dir_ed  m_PathToEmergenceSource = "";
	eg_os_browse_dir_ed  m_PathToEmergenceBuild  = "";
	eg_d_string          m_SyncHost              = "";
	eg_d_string          m_WebServicesHost       = "";
	eg_os_browse_dir_ed  m_PathToMilkShape3D     = "";
	eg_os_browse_dir_ed  m_PathTo3DWorldStudio   = "";
	eg_os_browse_dir_ed  m_BeemOut               = "";
	eg_os_browse_file_ed m_PathToP4IGNOREFile    = "";
	eg_d_string          m_P4CLIENT              = "";
	eg_d_string          m_P4USER                = "";
	eg_d_string          m_P4SERVER              = "";
	eg_os_browse_dir_ed  m_PathToRawDataAssets   = "";
	eg_button_ed         m_SaveAndExit           = "Apply";
	eg_button_ed         m_Cancel                = "Cancel";
	eg_button_ed         m_DoSyncButton          = "Sync Binaries";
	eg_button_ed         m_DoFullSyncButton      = "Sync Game Data";
	eg_bool              m_bDisableAutoBuild     = false;
	eg_bool              m_bDisableToolDelta     = false;
	eg_bool              m_bEnableShaderPDB      = false;

	egRflEditor m_Editor;

	EGArray<egEdConfigProp> m_AllProps;
	EGArray<eg_d_string>    m_GamesList;

	egEdConfigProps()
	{
		m_Editor = egRflEditor( "EnvironmentVariables" , this , &Rfl_CustomEditor );

		auto RegisterItem = [this]( void* Data , eg_cpstr EnvVar , eg_bool bRequiresLogoff , eg_cpstr DisplayName , const egRflValueType* ValueType , bool bForceNotPath = false ) -> void
		{
			egEdConfigProp NewProp;
			NewProp.SetEnvVar( EnvVar );
			NewProp.SetOwner( this );
			NewProp.SetRequiresLogoff( bRequiresLogoff );
			eg_string_big FinalDisplayName = EGString_Format( "%s: %s" , EnvVar , DisplayName );
			NewProp.SetDisplayName( FinalDisplayName );
			NewProp.SetIsPath( &Rfl_eg_os_browse_dir_ed == ValueType && !bForceNotPath );
			m_Editor.AddExplicitChild( egRflEditor( EnvVar , Data , ValueType ) );
			m_Editor.GetChildPtr( EnvVar )->SetDisplayName( FinalDisplayName );
			NewProp.m_Editor = m_Editor.GetChild( EnvVar );
			m_AllProps.Append( NewProp );
		};


		RegisterItem( &m_GameToBuild           , "EGGAME"        , false , "Which game to edit"                      , &Rfl_eg_combo_box_str_ed );
		RegisterItem( &m_TargetPlatform        , "EGTARGETPLATFORM" , false , "Target Platform" , &Rfl_eg_combo_box_str_ed );
		RegisterItem( &m_bDisableAutoBuild     , "EGNOAUTOBUILD" , false , "Disable automatic data-build when running the editor" , &Rfl_eg_bool );
		RegisterItem( &m_bDisableToolDelta     , "EGNOTOOLDELTA" , false , "Don't rebuild data when tools change" , &Rfl_eg_bool );
		RegisterItem( &m_bEnableShaderPDB      , "EGSHADERDEBUG" , false , "Build shader debug info" , &Rfl_eg_bool );
		RegisterItem( &m_PathToEmergenceSource , "EGSRC"         , true  , "Path to Emergence Source"                , &Rfl_eg_os_browse_dir_ed );
		RegisterItem( &m_PathToEmergenceBuild  , "EGOUT"         , true  , "Path to where Emergence should be built" , &Rfl_eg_os_browse_dir_ed );
		RegisterItem( &m_SyncHost              , "EGSYNCHOST"    , false , "URL from which to sync data (e.g. https://egsync.beemsoft.net)" , &Rfl_eg_d_string );
		m_Editor.AddExplicitChild( egRflEditor( "" , &m_DoSyncButton , &Rfl_eg_button_ed ) );
		m_Editor.AddExplicitChild( egRflEditor( "" , &m_DoFullSyncButton , &Rfl_eg_button_ed ) );
		RegisterItem( &m_WebServicesHost       , "EGWEBSVCHOST"  , false , "URL For web services (https://eg.beemsoft.net)" , &Rfl_eg_d_string );
		RegisterItem( &m_BeemOut               , "BEEMOUT"       , true  , "Non-Emergence Beem Software build output (optional)"      , &Rfl_eg_os_browse_dir_ed );
		RegisterItem( &m_PathToP4IGNOREFile    , "P4IGNORE"      , true  , "Path to P4IGNORE file"                   , &Rfl_eg_os_browse_file_ed );
		RegisterItem( &m_P4SERVER              , "EGP4SERVER"    , true  , "Perforce Server for source control (e.g. P4.BEEMSOFT.NET:1666)"   , &Rfl_eg_d_string );
		RegisterItem( &m_P4CLIENT              , "EGP4CLIENT"    , true  , "P4CLIENT Used for certain build things"  , &Rfl_eg_d_string);
		RegisterItem( &m_P4USER                , "EGP4USER"      , true  , "Perforce user."                          , &Rfl_eg_d_string);
		
		// RegisterItem( &m_PathToMilkShape3D     , "EGOUT_MS3D" , true  , "Path to MilkShape 3D"                    , &Rfl_eg_os_browse_dir_ed );
		// RegisterItem( &m_PathTo3DWorldStudio   , "EGOUT_3DWS" , true  , "Path to 3D World Studio"                 , &Rfl_eg_os_browse_dir_ed );
		// RegisterItem( &m_PathToRawDataAssets   , "EGRAWSRC"   , true  , "Path to raw data assets (non-P4)"        , &Rfl_eg_os_browse_dir_ed);


		m_Editor.AddExplicitChild( egRflEditor( "" , &m_SaveAndExit , &Rfl_eg_button_ed ) );
		m_Editor.AddExplicitChild( egRflEditor( "" , &m_Cancel , &Rfl_eg_button_ed ) );

		egComboBoxPopulateFn PopulateCb = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
		{
			Out.Append( "" );
			Out.Append( m_GamesList );
			bManualEditOut = true;
		};

		m_GameToBuild.PopulateCb = PopulateCb;

		egComboBoxPopulateFn PopulatePlatformCb = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
		{
			for( eg_size_t i=0; i<EGTargetPlatform_GetNumPlatforms(); i++ )
			{
				Out.Append( EGTargetPlatform_GetPlatformNameByIndex( i ) );
			}
			bManualEditOut = false;
		};

		m_TargetPlatform.PopulateCb = PopulatePlatformCb;
	}

	void LoadFromRegistry()
	{
		for( egEdConfigProp& Item : m_AllProps )
		{
			Item.RefreshFromRegistery();
		}

		PopulateGamesList();
		if( m_TargetPlatform.String.Len() == 0 )
		{
			m_TargetPlatform.String = EGTargetPlatform_GetPlatformName( eg_target_platform_t::Default );
		}
	}

	void SaveToRegistry()
	{
		for( egEdConfigProp& Item : m_AllProps )
		{
			Item.SaveToRegistry();
		}
	}

	void PopulateGamesList()
	{
		m_GamesList.Clear();

		static const eg_cpstr16 ExcludeList[] =
		{
			L".",
			L"..",
			L".vs",
			L"core",
			L"tools",
			L"debug",
			L"release",
		};

		auto IsInExcludeList = []( eg_cpstr16 Str ) -> eg_bool
		{
			for( eg_cpstr16 ExcStr : ExcludeList )
			{
				if( EGString_EqualsI( ExcStr , Str ) )
				{
					return true;
				}
			}

			return false;
		};

		eg_string_big SourcePath = *m_PathToEmergenceSource;
		SetCurrentDirectoryW( EGString_ToWide(SourcePath) );

		WIN32_FIND_DATAW FindData;
		zero( &FindData );
		HANDLE hFind = FindFirstFileW( L"games/*" , &FindData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			eg_bool bContinue = true;
			while( bContinue )
			{
				eg_flags FileAttr( FindData.dwFileAttributes );
				if( FileAttr.IsSet( FILE_ATTRIBUTE_DIRECTORY ) && !FileAttr.IsSet( FILE_ATTRIBUTE_HIDDEN ) && !IsInExcludeList( FindData.cFileName ) )
				{
					m_GamesList.Append( FindData.cFileName );
				}
				bContinue = FindNextFileW( hFind , &FindData ) == TRUE;
			}

			FindClose( hFind );
		}
	}
};

class EGEdConfigPropEditor : public EGWndPanelPropEditor
{
	EG_DECL_SUPER( EGWndPanelPropEditor )

private:

	egEdConfigProps& m_Props;
	EGWndAppWindow& m_AppWindow;

public:

	EGEdConfigPropEditor( EGWndPanel* Parent , egEdConfigProps* Props , EGWndAppWindow* AppWindow )
	: Super( Parent )
	, m_Props( *Props )
	, m_AppWindow( *AppWindow )
	{

	}

	virtual void HandlePropChanged( const egRflEditor* Property ) override
	{
		if( Property->GetData() == &m_Props.m_PathToEmergenceSource )
		{
			m_Props.PopulateGamesList();
			PropEditorRebuild();
		}

		for( egEdConfigProp& Prop : m_Props.m_AllProps )
		{
			if( Prop.m_Editor.GetData() == Property->GetData() )
			{
				Prop.SetHasChanged( true );
			}
		}
	}

	virtual void OnButtonPressed( const egRflEditor* Property ) override
	{
		if( Property->GetData() == &m_Props.m_Cancel )
		{
			PostMessageW( m_AppWindow.GetWnd() , WM_EGEDCONFIG_CANCEL , 0 , 0 ); 
		}
		else if( Property->GetData() == &m_Props.m_SaveAndExit )
		{
			PostMessageW( m_AppWindow.GetWnd() , WM_CLOSE , 0 , 0 );
		}
		else if( Property->GetData() == &m_Props.m_DoSyncButton )
		{
			PostMessageW( m_AppWindow.GetWnd() , WM_EGEDCONFIG_SYNC , 0 , 0 );
		}
		else if( Property->GetData() == &m_Props.m_DoFullSyncButton )
		{
			PostMessageW( m_AppWindow.GetWnd() , WM_EGEDCONFIG_FULL_SYNC , 0 , 0 );
		}
	}
};

class EGEdConfigMainWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

private:

	EGWndPanel            m_MainPanel;
	EGEdConfigPropEditor* m_PropEditor = nullptr;
	EGEdConfigSyncPanel*  m_SyncPanel = nullptr;
	egEdConfigProps       m_Props;
	eg_char16             m_LastSaveFilename[256];
	int                   m_nShowCmd;
	eg_bool               m_bIsSyncing:1;
	eg_bool               m_bIsDone:1;

public:

	EGEdConfigMainWnd( int nCmdShow )
	: Super( EGEdConfig_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_nShowCmd( nCmdShow )
	, m_bIsSyncing( false )
	, m_bIsDone( false )
	{
		m_Props.LoadFromRegistry();

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );

		EGWndPanel* MidPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		MidPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		// Mid Pane
		{
			m_SyncPanel = MidPane->AddChild<EGEdConfigSyncPanel>( this );
			m_SyncPanel->SetVisible( false );
			MidPane->AddChild<EGWndPanelHeader>( "Configuration" );
			m_PropEditor = MidPane->AddChild<EGEdConfigPropEditor>( &m_Props , this );
			MidPane->AddChild<EGWndPanelHeader>( "Debug Log" );
			EGWndDebugLogPanel* DebugLogPanel = MidPane->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Percent , 20 );
		}

		if( m_PropEditor )
		{
			m_PropEditor->SetEditObject( &m_Props.m_Editor );
		}

		SetAppWidth( 500 );
		const eg_int MaxAppHeight = 750;
		if( GetAppHeight() > MaxAppHeight )
		{
			SetAppHeight( MaxAppHeight );
		}

		m_MainPanel.HandleResize();
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		Super::OnAppStart( Args );

		if( !m_bIsDone )
		{
			ShowWindow( GetWnd() , m_nShowCmd );
			UpdateWindow( GetWnd() );
		}
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }
	
private:

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam ) override
	{
		switch( Msg )
		{
			case WM_EGEDCONFIG_CANCEL:
			{
				HandleQuitRequest( true );
			} return 0;

			case WM_EGEDCONFIG_SYNC:
			{
				if( !m_bIsDone && !m_bIsSyncing )
				{
					m_bIsSyncing = true;
					m_PropEditor->SetVisible( false );
					m_SyncPanel->SetVisible( true );
					m_SyncPanel->BeginSync( *m_Props.m_SyncHost , *m_Props.m_PathToEmergenceBuild , true );
				}
			} return 0;

			case WM_EGEDCONFIG_FULL_SYNC:
			{
				if( !m_bIsDone && !m_bIsSyncing )
				{
					m_bIsSyncing = true;
					m_PropEditor->SetVisible( false );
					m_SyncPanel->SetVisible( true );
					m_SyncPanel->BeginSync( *m_Props.m_SyncHost , *m_Props.m_PathToEmergenceBuild , false );
				}
			} return 0;

			case WM_EGEDCONFIG_SYNC_DONE:
			{
				if( m_bIsSyncing )
				{
					m_bIsSyncing = false;
					m_PropEditor->SetVisible( true );
					m_SyncPanel->SetVisible( false );
				}
			} return 0;
		}

		return Super::WndProc( Msg , wParam , lParam );
	}

	virtual void OnWmActivateApp( eg_bool bActivating , DWORD ThreadId ) override
	{
		Super::OnWmActivateApp( bActivating , ThreadId );
	}

	virtual void OnWmClose() override
	{
		Super::OnWmClose();

		if( m_bIsSyncing )
		{
			MessageBoxW( GetWnd() , L"Wait for sync to complete before quitting." , EGEdConfig_Title , MB_OK|MB_ICONWARNING );
		}
		else
		{
			HandleQuitRequest( false );
		}
	}

	virtual void OnWmQuit( eg_int ExitCode ) override
	{
		Super::OnWmQuit( ExitCode );
	}

	virtual void OnWmGetMinMaxInfo( MINMAXINFO* MinMaxInfo ) override
	{
		MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3MidMinWidth();
		MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize )
	{
		Super::OnWmSize( NewClientSize );

		m_MainPanel.HandleResize();
	}

	void HandleQuitRequest( eg_bool bIsCancel )
	{
		if( !bIsCancel )
		{
			eg_bool bRequiresLogoff = false;
			eg_bool bAnythingChanged = false;

			for( egEdConfigProp& Var : m_Props.m_AllProps )
			{
				if( Var.HasChanged() )
				{
					bAnythingChanged = true;
					if( Var.RequiresLogoff() )
					{
						bRequiresLogoff = true;
					}
				}
			}

			if( bAnythingChanged && bRequiresLogoff )
			{
				int LogoffRes = MessageBoxW( GetWnd() , L"You must log out of Windows for these settings to take effect.\n\nDo you want to log out now?" , EGEdConfig_Title , MB_YESNOCANCEL|MB_ICONQUESTION );

				if( IDCANCEL == LogoffRes )
				{
					return;
				}
				else if( IDYES == LogoffRes )
				{
					ExitWindowsEx( EWX_LOGOFF , 0 );
				}
			}

			m_Props.SaveToRegistry();
		}

		m_bIsDone = true;
	}
};

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	EGToolMem_Init();
	EGLog_SetDefaultSuppressedChannels();

	EGLogf( eg_log_t::General , "Starting %s (%s %s)" , EGString_ToMultibyte(EGEdConfig_Title) , __DATE__ , __TIME__ );
	unused( hPrevInstance );
	unused( lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , hInstance , nCmdShow , false , L"EGEDCONFIG_ICON" , L"EGEDCONFIG_ICON" , nullptr );

	EGCrcDb::Init();
	EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t::OS );

	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGEdConfigMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 

	EGCrcDb::Deinit();
	EGToolMem_Deinit();
	return 0;
}

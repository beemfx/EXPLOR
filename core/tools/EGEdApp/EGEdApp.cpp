// (c) 2017 Beem Media

// Basic foundation for a EGWnd application

#pragma comment( lib , "egfoundation.lib" )
#pragma comment( lib , "eglib.lib" )
#pragma comment( lib , "lib3p.lib" )
#pragma comment( lib , "winhttp.lib" )

#include "EGEdApp.h"
#include "../EGEdResLib/resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGToolMem.h"
#include "EGResourceLib.h"
#include "EGPath2.h"
#include "EGToolsHelper.h"
#include "EGEdAppRegExt.h"
#include "EGWndPanelPropEditor.h"
#include "EGThread.h"
#include "EGEdAppProc.h"
#include "EGEdUtility.h"
#include "EGLibFile.h"
#include "EGWndFileBrowser.h"
#include "EGWndDraw.h"
#include "EGOsFile.h"

static void EGEdApp_OpenFile( eg_cpstr16 FileToLaunch );

static const eg_char16 EGWndApp_Title[] = L"EG Editor";

class EGEdAppContentWnd : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	eg_bool       m_bHasHadTool = false;
	eg_bool       m_bToolRunning = false;
	eg_int        m_BuildingElipses = 0;
	eg_string_big m_CurrentToolStr = CT_Clear;
	
public:

	EGEdAppContentWnd( EGWndPanel* InParent )
	: Super( InParent , eg_panel_size_t::Percent , 10 )
	{ 
		FullRedraw();
	}

	void SetToolStarted( eg_cpstr NewToolStr )
	{
		m_bHasHadTool = true;
		m_bToolRunning = true;
		m_CurrentToolStr = NewToolStr;
	}

	void UpdateProgress( eg_bool bIsBuilding )
	{
		m_BuildingElipses++;
		m_BuildingElipses %= 4;
		m_bToolRunning = bIsBuilding;
		FullRedraw();
	}

private:

	virtual void OnDrawBg( EGWndDc& Dc ) override
	{
		Dc.DcDrawFilledRect( GetViewArea() , egw_color_t::BG_STATIC );
	}

	virtual void OnPaint( EGWndDc& Dc ) override
	{
		eg_recti rc = GetViewArea();
		Dc.DcSetFont( egw_font_t::DEFAULT );
		Dc.DcSetTextColor( egw_color_t::FG_STATIC );
		Dc.DcSetBkMode( eg_wnd_dc_bk_mode::Transparent );

		if( m_bToolRunning )
		{
			eg_s_string_sml16 BuildingText( CT_Clear );
			for( eg_int i=0; i<m_BuildingElipses; i++ )
			{
				BuildingText += L".";
			}
			BuildingText += m_CurrentToolStr;
			for( eg_int i=0; i<m_BuildingElipses; i++ )
			{
				BuildingText += L".";
			}

			Dc.DcDrawText( *BuildingText , rc , EGWND_DC_F_CENTER|EGWND_DC_F_VCENTER|EGWND_DC_F_SINGLELINE );
		}
		else
		{
			Dc.DcDrawText( L"Ready" , rc , EGWND_DC_F_CENTER|EGWND_DC_F_VCENTER|EGWND_DC_F_SINGLELINE );
		}
	}
};

class EGEdAppMainWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

private:

	static const eg_uint64 UDPATE_TIMER_ID = 1;

private:

	EGThread                m_WorkerThread;
	EGEdAppProc             m_WorkerProc;
	EGWndPanel              m_MainPanel;
	EGEdAppContentWnd*      m_ContentWnd = nullptr;
	EGArray<EGWndFileBrowser*> m_FileBrowsers;
	eg_char16               m_LastSaveFilename[256];
	int                     m_nShowCmd;
	eg_bool                 m_bIsDone:1;

public:

	EGEdAppMainWnd( int nCmdShow )
	: EGWndAppWindow( EGWndApp_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_nShowCmd( nCmdShow )
	, m_bIsDone( false )
	, m_WorkerThread( EGThread::egInitParms( "EGEdApp Worker" , EGThread::eg_init_t::LowPriority ) )
	{
		m_WorkerThread.RegisterProc( &m_WorkerProc );
		m_WorkerThread.Start();

		SetMenu( GetWnd() , LoadMenuW( EGResourceLib_GetLibrary() , L"EGEDAPP_MAIN_MENU" ) );

		SetWndTimer( UDPATE_TIMER_ID , 333 );

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );

		EGWndPanel* MidPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		MidPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		// Mid Pane
		{
			MidPane->AddChild<EGWndPanelHeader>( "Status" );
			m_ContentWnd = MidPane->AddChild<EGEdAppContentWnd>();
			EGWndTabbedPanel* BrowserTabs = MidPane->AddChild<EGWndTabbedPanel>();
			for( eg_int i=0; i<4; i++ )
			{
				EGWndFileBrowser* NewBrowser = BrowserTabs->AddTab<EGWndFileBrowser>( *EGSFormat8( "Browser {0}" , i+1 ) , BrowserTabs );
				m_FileBrowsers.Append( NewBrowser );
			}
			MidPane->AddChild<EGWndPanelHeader>( "Log" );
			EGWndDebugLogPanel* DebugLogPanel = MidPane->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Auto , 0 );
		}

		{
			eg_ivec2 DesktopRes = EGWnd_GetDesktopResolution();

			SetAppWidth( DesktopRes.x/2 );
			// SetAppHeight( DesktopRes.y/2 );
		}

		for( EGWndFileBrowser* FileBrowser : m_FileBrowsers )
		{
			if( FileBrowser )
			{
				FileBrowser->FileDoubleClickedDelegate.Bind( this , &EGEdAppMainWnd::OnFileDoubleClicked );
				FileBrowser->FileRightClickedDelegate.Bind( this , &EGEdAppMainWnd::OnFileRightClicked );
			}
		}

		m_MainPanel.HandleResize();

		HandleGameChanged();
	}

	~EGEdAppMainWnd()
	{
		SaveConfig();
		
		m_WorkerThread.Stop();
		m_WorkerThread.UnregisterProc( &m_WorkerProc );
	}

	void HandleGameChanged()
	{
		LoadConfig();
	}

	void LoadConfig()
	{
		const eg_s_string_sml16 GameToBuild = *EGToolsHelper_GetBuildVar( "EGGAME" );
		const eg_s_string_big16 SourceDir = *EGToolsHelper_GetBuildVar( "EGSRC" );
		const eg_s_string_big16 GameDataPath = *EGPath2_CleanPath(*EGSFormat16( L"{0}/games/{1}/data" , *SourceDir , *GameToBuild ),L'\\');
		
		for( eg_int i=0; i<m_FileBrowsers.LenAs<eg_int>(); i++ )
		{
			EGWndFileBrowser* FileBrowser = m_FileBrowsers[i];
			if( FileBrowser )
			{
				FileBrowser->SetRoot( *GameDataPath );
				const eg_d_string16 SavedPath = EGToolsHelper_GetGameEditorConfigSetting( *EGSFormat8("EditorFileBrowserLastPath_{0}",i) );
				if( SavedPath.Len() > 0 )
				{
					FileBrowser->SetDirectory( *SavedPath );
				}
			}
		}

		// Last reimport
		{
			eg_edapproc_reimport_t Type = EG_To<eg_edapproc_reimport_t>(EGString_ToInt( *EGToolsHelper_GetGameEditorConfigSetting( "EditorLastReimportType" ) ));
			eg_d_string16 Parm = EGToolsHelper_GetGameEditorConfigSetting( "EditorLastReimportParm" );
			m_WorkerProc.SetLastReimportInfo( Type , *eg_d_string8(*Parm) );
		}
	}

	void SaveConfig()
	{
		for( eg_int i=0; i<m_FileBrowsers.LenAs<eg_int>(); i++ )
		{
			EGWndFileBrowser* FileBrowser = m_FileBrowsers[i];
			if( FileBrowser )
			{
				eg_cpstr16 ViewDir = FileBrowser->GetViewedDirectory();
				EGToolsHelper_SetGameEditorConfigSetting( *EGSFormat8("EditorFileBrowserLastPath_{0}",i) , ViewDir );
			}
		}

		// Last reimport
		const egEdAppProcReimportInfo& LastReimportInfo = m_WorkerProc.GetLastReimportInfo();
		EGToolsHelper_SetGameEditorConfigSetting( "EditorLastReimportType" , *EGSFormat16( L"{0}" , EG_To<eg_int>(LastReimportInfo.Type) ) );
		EGToolsHelper_SetGameEditorConfigSetting( "EditorLastReimportParm" , *eg_d_string16(*LastReimportInfo.Parm) );
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

	virtual void OnWmActivateApp( eg_bool bActivating , DWORD ThreadId ) override
	{
		Super::OnWmActivateApp( bActivating , ThreadId );
	}

	virtual void OnWmClose() override
	{
		Super::OnWmClose();
		HandleQuitRequest();
	}

	virtual void OnWmGetMinMaxInfo( MINMAXINFO* MinMaxInfo ) override
	{
		MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3MidMinWidth();
		MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override
	{
		unused( bFromAccelerator );

		unused( CmdId );
		switch (CmdId)
		{
			case ID_NEW_WORLD:
			{
				RunThreadCommand( "New World" , "NewWorld()" );
			} break;
			case ID_NEW_DATAASSET:
			{
				RunThreadCommand( "New Data Asset" , "NewDataAsset()" );
			} break;
			case ID_NEW_ENTITYDEFINITION:
			{
				RunThreadCommand( "New Entity Definition" , "NewEntityDef()" );
			}  break;
			case ID_NEW_UILAYOUT:
			{
				RunThreadCommand( "New UI Layout" , "NewUILayout()" );
			} break;
			case ID_NEW_EGSMSCRIPT:
			{
				RunThreadCommand( "New UI Layout" , "NewEGSMScript()" );
			} break;
			case ID_TOOLS_RESAVEASSETS:
				RunThreadCommand( "Resave Assets" , "ResaveAssets()" );
				break;
			case ID_TOOLS_EDITORCONFIGURATION:
			{
				RunThreadCommand( "Editor Configuration" , "EditorConfig()" );
			} break;
			case ID_TOOLS_BUILDGAMEDATA:
			{
				RunThreadCommand( "Data Build" , "DataBuild()" );
			} break;
			case ID_TOOLS_LOCALIZE:
			{
				RunThreadCommand( "Localize" , "Localize()" );
			} break;
			case ID_TOOLS_CREATEVSPROJECTS:
			{
				RunThreadCommand( "Creating VS Projects" , "CreateVSProjects()" );
			} break;
			case ID_TOOLS_PACKAGEGAME:
			{
				RunThreadCommand( "Packaging Game" , "PackageGame()" );
			} break;
			case ID_TOOLS_IMPORTGAMEDATA:
			{
				RunThreadCommand( "Importing Data" , "ImportData()" );
			} break;
			case ID_IMPORTRAWTYPE_XML:
			{
				RunThreadCommand( "Importing Data" , "ImportData(\"XML\")" );
			} break;
			case ID_IMPORTRAWTYPE_TERRAIN:
			{
				RunThreadCommand( "Importing Data" , "ImportData(\"BuildTerrain\")" );
			} break;
			case ID_IMPORTRAWTYPE_MILKSHAPE3D:
			{
				RunThreadCommand( "Importing Data" , "ImportData(\"MS3D\")" );
			} break;
			case ID_IMPORTRAWTYPE_TEXTURE:
			{
				RunThreadCommand( "Importing Data" , "ImportData(\"Texture\")" );
			} break;
			case ID_GAMEDATA_CLEANGAMEDATA:
			{
				RunThreadCommand( "Cleaning Data" , "CleanDataBuild()" );
			} break;
			case ID_TOOLS_BUILDWEBSERVER:
			{
				RunThreadCommand( "Building Web Server" , "BuildWebServer()" );
			} break;
			case ID_GAME_PLAY:
			{
				RunThreadCommand( "Playing Game" , "PlayGame()" );
			} break;
			case ID_GAMEDATA_REDOLASTIMPORT:
			{
				RunThreadCommand( "Importing Data" , "RedoLastImport()" );
			} break;
			case ID_FILE_QUIT:
				HandleQuitRequest();
				break;
			default:
				assert( false ); // Not handled
				break;
		}
	}

	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId )
	{
		unused( WndControl , NotifyId , ControlId );
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize )
	{
		Super::OnWmSize( NewClientSize );

		m_MainPanel.HandleResize();
	}

	void HandleQuitRequest()
	{
		if( m_WorkerProc.IsExecuting() )
		{
			m_WorkerProc.CancelCommands();
			MessageBoxW( GetWnd() , L"Cannot quit. Commands are currently being executed." , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
		}
		else
		{
			m_bIsDone = true;
		}
	}

	virtual void OnWmTimer( eg_uint64 TimerId )
	{
		Super::OnWmTimer( TimerId );

		if( TimerId == UDPATE_TIMER_ID )
		{
			m_ContentWnd->UpdateProgress( m_WorkerProc.IsExecuting() );
		}
	}

	void RunThreadCommand( eg_cpstr LabelStr , eg_cpstr CmdStr )
	{
		if( m_WorkerProc.IsExecuting() )
		{
			MessageBoxA( GetWnd() , "Already executing a command, please wait until it finishes." , LabelStr , MB_OK|MB_ICONINFORMATION );
		}
		else
		{
			m_WorkerThread.PostThreadMsg( CmdStr );
			m_ContentWnd->SetToolStarted( LabelStr );
		}
	}

	static void OnFileDoubleClicked( void* thisv , eg_cpstr16 FullPath , const egOsFileInfo& FileInfo )
	{
		unused( FileInfo );
		
		EGEdAppMainWnd* _this = reinterpret_cast<EGEdAppMainWnd*>(thisv);
		if( _this )
		{
			// MessageBoxW( _this->GetWnd() , *EGSFormat16( L"You clicked {0}" , FullPath ) , L"File Click" , MB_OK );
			eg_d_string16 AdjFullPath = FullPath;
			if (EGString_BeginsWithI(FullPath, L".\\") || EGString_BeginsWithI( FullPath , L"./" ))
			{
				const eg_d_string16 CurDir = EGOsFile_GetCWD();
				AdjFullPath = EGSFormat16(L"{0}\\\\{1}" , *CurDir , &FullPath[2] );
				AdjFullPath = EGPath2_CleanPath(*AdjFullPath , '\\' );
			}
			EGEdApp_OpenFile( *AdjFullPath );
		}
	}

	// File browser context menu
	enum class eg_editor_file_context_cmd : eg_int
	{
		None,
		CopyToCliboard,
		File_Reimport,
		File_Resave,
		File_BroweTo,
		File_BrowseToRawAsset,
		File_CheckOut,
		File_Delete,
		Dir_GenerateBuildConfig,
		Dir_Reimport,
		Dir_BrowseTo,
		Dir_BrowseToRawAsset,
		Dir_CreateRawAssetDir,
	};

	void HandleContextCommand( eg_editor_file_context_cmd Cmd , eg_cpstr16 FullPath , const egOsFileInfo& FileInfo )
	{
		unused( FileInfo );

		auto ExplorerOpenSpecificFile = []( eg_cpstr16 Filename ) -> void
		{
			egPathParts2 FilenameParts = EGPath2_BreakPath( *(EGOsFile_GetCWD() + L"\\" + Filename) );
			EGPath2_ResolveSpecialFolders( FilenameParts );
			const eg_d_string16 FullPathAdj = FilenameParts.ToString( true , '\\' );
			LPITEMIDLIST pIDL = ILCreateFromPathW( *FullPathAdj );
			if( pIDL )
			{
				HRESULT hRet = SHOpenFolderAndSelectItems(pIDL, 0, NULL, 0);
				if( FAILED(hRet) )
				{
					EGLogf( eg_log_t::Error , "SHOpenFolderAndSelectItems Failed." );
				}

				ILFree(pIDL);
			}
			else
			{
				EGLogf( eg_log_t::Error , "ILCreateFromPathW Failed." );
			}
		};

		auto ExplorerOpenDirectory = []( eg_cpstr16 DirName ) -> void
		{
				egPathParts2 FilenameParts = EGPath2_BreakPath( *(EGOsFile_GetCWD() + L"\\" + DirName) );
				EGPath2_ResolveSpecialFolders( FilenameParts );
				const eg_d_string16 FullPathAdj = FilenameParts.ToString( true , '\\' );

			ShellExecuteW( NULL , L"open" , *FullPathAdj , NULL , NULL , SW_SHOWNORMAL );
		};

		auto CopyToCliboard = []( eg_cpstr16 PathName ) -> void
		{
			eg_d_string16 FormattedPath = EGPath2_CleanPath( PathName , L'\\' );

			const eg_size_t StrLen = FormattedPath.Len() + 1;
			const eg_size_t MemSize = sizeof(FormattedPath[0])*StrLen;
			HGLOBAL MemHandle =  GlobalAlloc( GMEM_MOVEABLE , MemSize );
			if( LPVOID MemPtr = GlobalLock( MemHandle ) )
			{
				EGMem_Copy( MemPtr , *FormattedPath , MemSize );
				GlobalUnlock( MemHandle );

				OpenClipboard( NULL );
				EmptyClipboard();
				SetClipboardData( CF_UNICODETEXT , MemHandle );
				CloseClipboard();

				EGLogf( eg_log_t::General , "Copied \"%s\" to clipboard." , *eg_d_string8(*FormattedPath) );
			}
		};
		
		switch( Cmd )
		{
			case eg_editor_file_context_cmd::None:
			{
				EGLogf( eg_log_t::General , "No command specified." );
			}break;

			case eg_editor_file_context_cmd::CopyToCliboard:
			{
				CopyToCliboard( FullPath );
			}break;

			case eg_editor_file_context_cmd::File_Reimport:
			{
				RunThreadCommand( "Reimporting File" , *EGSFormat8( "ReimportFile(\"{0}\")" , FullPath) );
			} break;

			case eg_editor_file_context_cmd::File_Resave:
			{
				MessageBoxW( GetWnd() , *EGSFormat16( L"Not implemented {0}", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
			} break;

			case eg_editor_file_context_cmd::File_BroweTo:
			{
				if( EGOsFile_DoesFileExist( FullPath ) )
				{
					ExplorerOpenSpecificFile( FullPath );
				}
			} break;

			case eg_editor_file_context_cmd::File_BrowseToRawAsset:
			{
				const egPathParts2 RawPathParts = EGPath2_BreakPath( *EGToolsHelper_GetRawAssetPathFromGameAssetPath( FullPath ) );
				const eg_d_string16 RawAssetDir = *RawPathParts.GetDirectory();
				if( EGOsFile_DoesDirExist( *RawAssetDir ) )
				{
					// See if there is a .BuildConfig file. If so select it, otherwise open the directory.
					const eg_d_string16 BuildConfigFile = RawAssetDir + RawPathParts.Filename + L".BuildConfig";
					if( EGOsFile_DoesFileExist( *BuildConfigFile ) )
					{
						ExplorerOpenSpecificFile( *BuildConfigFile );
					}
					else
					{
						ExplorerOpenDirectory( *RawAssetDir );
					}
				}
				else
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Raw asset directory does not exist for {0}.", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
			} break;

			case eg_editor_file_context_cmd::File_CheckOut:
			{
				if( EGOsFile_DoesFileExist( FullPath ) )
				{
					eg_bool bCheckedOut = EGEdUtility_CheckoutFile( FullPath );
					if( !bCheckedOut )
					{
						MessageBoxW( GetWnd() , *EGSFormat16( L"Wasn't able to check out {0}", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
					}
				}
				else
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Could not find {0}", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
			} break;

			case eg_editor_file_context_cmd::File_Delete:
			{
				if( EGOsFile_DoesFileExist( FullPath ) )
				{
					eg_bool bCheckedOut = EGEdUtility_DeleteFile( FullPath );
					if( !bCheckedOut )
					{
						MessageBoxW( GetWnd() , *EGSFormat16( L"Wasn't able to delete {0}.\nThe file must not be checked out in order to delete it.", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
					}
				}
				else
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Could not find {0}", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
			} break;

			case eg_editor_file_context_cmd::Dir_GenerateBuildConfig:
			{
				const eg_d_string16 RawAssetPath = EGToolsHelper_GetRawAssetPathFromGameAssetPath( FullPath );
				RunThreadCommand( "Generating Files" , *EGSFormat8( "GenerateBuildConfig(\"{0}\")" , *RawAssetPath ) );
			} break;

			case eg_editor_file_context_cmd::Dir_Reimport:
			{
				RunThreadCommand( "Reimporting File" , *EGSFormat8( "ReimportFile(\"{0}/\")" , FullPath) );
			} break;

			case eg_editor_file_context_cmd::Dir_BrowseTo:
			{
				if( EGOsFile_DoesDirExist( FullPath ) )
				{
					ExplorerOpenDirectory( FullPath );
				}
				else
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Not such folder: {0}", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
			} break;

			case eg_editor_file_context_cmd::Dir_BrowseToRawAsset:
			{
				const eg_d_string16 RawAssetPath = EGToolsHelper_GetRawAssetPathFromGameAssetPath( FullPath );
				if( EGOsFile_DoesDirExist( *RawAssetPath ) )
				{
					ExplorerOpenDirectory( *RawAssetPath );
				}
				else
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Raw asset directory does not exist for {0}, use \"Create Raw Directory\" to create it.", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
			} break;

			case eg_editor_file_context_cmd::Dir_CreateRawAssetDir:
			{
				const eg_d_string16 RawAssetPath = EGToolsHelper_GetRawAssetPathFromGameAssetPath( FullPath );
				if( EGOsFile_DoesDirExist( *RawAssetPath ) )
				{
					MessageBoxW( GetWnd() , *EGSFormat16( L"Raw asset directory already exists for {0}.", FullPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
				}
				else
				{
					const eg_bool bCreated = EGOsFile_CreateDirectory( *RawAssetPath );
					if( bCreated )
					{
						ExplorerOpenDirectory( *RawAssetPath );
					}
					else
					{
						MessageBoxW( GetWnd() , *EGSFormat16( L"Failed to create {0}.", *RawAssetPath ) , EGWndApp_Title , MB_OK|MB_ICONINFORMATION );
					}
				}
			} break;
		}
		// MessageBoxW( GetWnd() , *EGSFormat16( L"You chose {0}" , EG_To<eg_int>(Cmd) ) , L"File Click" , MB_OK );
	}

	static void OnFileRightClicked( void* thisv , eg_cpstr16 FullPath , const egOsFileInfo& FileInfo )
	{
		EGEdAppMainWnd* _this = reinterpret_cast<EGEdAppMainWnd*>(thisv);
		if( _this )
		{
			// MessageBoxW( _this->GetWnd() , *EGSFormat16( L"You right clicked {0}" , FullPath ) , L"File Click" , MB_OK );

			EGWndMenuItemDef ItemContextMenu;
			if( FileInfo.Attributes.bDirectory )
			{
				ItemContextMenu.AddCmd( "Copy Path to Clipboard" , EG_To<eg_uint>(eg_editor_file_context_cmd::CopyToCliboard) );
				ItemContextMenu.AddCmd( "Generate .BuildConfig" , EG_To<eg_uint>(eg_editor_file_context_cmd::Dir_GenerateBuildConfig) );
				ItemContextMenu.AddCmd( "Re-Import Directory" , EG_To<eg_uint>(eg_editor_file_context_cmd::Dir_Reimport) );
				ItemContextMenu.AddCmd( "Browse To Directory" , EG_To<eg_uint>(eg_editor_file_context_cmd::Dir_BrowseTo) );
				ItemContextMenu.AddCmd( "Browse To Raw Directory" , EG_To<eg_uint>(eg_editor_file_context_cmd::Dir_BrowseToRawAsset) );
				ItemContextMenu.AddCmd( "Create Raw Directory" , EG_To<eg_uint>(eg_editor_file_context_cmd::Dir_CreateRawAssetDir) );
			}
			else
			{
				ItemContextMenu.AddCmd( "Copy Path to Clipboard" , EG_To<eg_uint>(eg_editor_file_context_cmd::CopyToCliboard) );
				ItemContextMenu.AddCmd( "Re-Import" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_Reimport) );
				// ItemContextMenu.AddCmd( "Re-save" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_Resave) );
				ItemContextMenu.AddCmd( "Browse To" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_BroweTo) );
				ItemContextMenu.AddCmd( "Browse To Raw Asset" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_BrowseToRawAsset) );
				ItemContextMenu.AddCmd( "Check out (SCC)" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_CheckOut) );
				ItemContextMenu.AddCmd( "Delete (SCC)" , EG_To<eg_uint>(eg_editor_file_context_cmd::File_Delete) );
			}
			HMENU PopupMenu = EGWnd_CreateMenu( ItemContextMenu , true );
			POINT CursorPos;
			GetCursorPos( &CursorPos );
			BOOL RetValue = TrackPopupMenu( PopupMenu , TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_LEFTBUTTON , CursorPos.x , CursorPos.y , 0 , _this->GetWnd() , nullptr );
			if( RetValue != 0 )
			{
				const eg_int CmdId = static_cast<eg_int>(RetValue);
				_this->HandleContextCommand( EG_To<eg_editor_file_context_cmd>(CmdId) , FullPath , FileInfo );
			}
			
			DestroyMenu( PopupMenu );
		}
	}
};

static void EGEdApp_RunLauncher( HINSTANCE hInstance , int nCmdShow )
{
	static const egWndAppInfo AppInfo( hInstance , EGResourceLib_GetLibrary() , nCmdShow , false , L"EGEDAPP_ICON" , L"EGEDAPP_ICON" , L"EGEDAPP_ACCELERATOR" );
	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGEdAppMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 
}

static void EGEdApp_OpenFile( eg_cpstr16 FileToLaunch )
{
	eg_string_small GameToLaunch = EGToolsHelper_GetEnvVar( "EGGAME" );

	eg_string FileAsString = FileToLaunch;
	eg_string RelativeTo = *EGPath2_GetRelativePathTo( FileAsString , EGToolsHelper_GetEnvVar("EGSRC") );
	egPathParts2 PathParts = EGPath2_BreakPath( RelativeTo );
	if( PathParts.Root.Len() == 0 )
	{
		if( PathParts.Folders.Len() >= 3 && PathParts.Folders[2].EqualsI( L"data" ) )
		{
			if( !PathParts.Folders[1].EqualsI( *eg_d_string16(EGToolsHelper_GetEnvVar("EGGAME")) ) && !PathParts.Folders[0].EqualsI(L"core") )
			{
				eg_string_big StrMessage = EGString_Format( "This asset is part of \"%s\" EGGAME must be changed to open this. Do you want to change it now? (Application must be restarted to open this file.)" , *eg_d_string8(*PathParts.Folders[0]) );
				// if( IDYES == MessageBoxW( nullptr , EGString_ToWide(StrMessage) , EGWndApp_Title , MB_YESNO ) )
				{
					EGToolsHelper_SetEnvVar( "EGGAME" , *eg_d_string8(*PathParts.Folders[1]) );
					GameToLaunch = *eg_d_string8(*PathParts.Folders[1]);
				}

				// return;
			}
		}
	}

	eg_string_big FullExeName = *EGEdApp_GetEditorExecutable();

	// MessageBoxA( nullptr , FullExeName , "EXE Name" , MB_OK );

	eg_char16 FullCmdLine[2048];

	EGString_FormatToBuffer( FullCmdLine , countof(FullCmdLine) , L"\"%s\" -editor \"%s\"" , EGString_ToWide(FullExeName.String() ) , FileToLaunch );

	BOOL bSucc = FALSE;
	SECURITY_ATTRIBUTES SA;
	zero( &SA );
	SA.nLength = sizeof(SA);
	SA.bInheritHandle = TRUE;
	SA.lpSecurityDescriptor = NULL;

	STARTUPINFOW SI;
	zero(&SI);
	SI.cb = sizeof(SI);

	PROCESS_INFORMATION PI;
	zero(&PI);

	void* Env = nullptr;

	eg_cpstr16 WorkingDir = L"_BUILD\\Bin";
	bSucc = CreateProcessW( NULL , FullCmdLine , &SA , &SA , TRUE , 0 , Env , WorkingDir , &SI , &PI );

	if( !bSucc )
	{
		eg_string_big ErrorMsg = EGString_Format( "Was not able to launch %s. Make sure it is built." , FullExeName.String() );
		MessageBoxW( nullptr , EGString_ToWide( ErrorMsg ) , EGWndApp_Title , MB_OK );
	}
}

static void EgEdApp_SetToSolutionDir()
{
	const eg_cpstr16 DirsToTry[] =
	{
		L"." ,
		L".." ,
		L"..\\..",
		L"..\\..\\..",
	};

	const eg_d_string16 WorkingDir = EGOsFile_GetCWD();

	for (eg_cpstr16 Dir : DirsToTry)
	{
		const eg_d_string16 DirToTry = WorkingDir + "\\" + Dir;
		::SetCurrentDirectoryW(*DirToTry);

		const DWORD SlnAttr = ::GetFileAttributesW(L"EG.sln");
		if (SlnAttr == INVALID_FILE_ATTRIBUTES)
		{
			continue;
		}

		if ((SlnAttr&FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			continue;
		}

		return;
	}
}

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	EGToolMem_Init();

#if( _WIN32_WINNT >= 0x0600 )
	SetProcessDPIAware();
#endif

	EGLog_SetChannelSuppressed( eg_log_t::Thread , true );
	EGLog_SetChannelSuppressed( eg_log_t::Verbose , true );
	EGLogf( eg_log_t::General , "Starting %s" , EGString_ToMultibyte(EGWndApp_Title) );

	EgEdApp_SetToSolutionDir();
	
	unused( hPrevInstance );
	unused( lpCmdLine );

	EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t::OS );
	EGResourceLib_Init();
	EGCrcDb::Init();
	EGEdUtility_Init();

	eg_char16 FileToLaunch[256] = { '\0' };

	int nArgs = 0;
	LPWSTR* szArglist = CommandLineToArgvW( GetCommandLineW(), &nArgs );
	if( szArglist )
	{
		if( nArgs >= 2 )
		{
			EGString_Copy( FileToLaunch , szArglist[1] , countof(FileToLaunch) );
		}

		LocalFree( szArglist );
	}

	if( EGString_StrLen( FileToLaunch ) > 0 )
	{
		EGEdApp_OpenFile( FileToLaunch );
	}
	else
	{
		EGEdAppRegExt_Register();
		EGEdApp_RunLauncher( hInstance , nCmdShow );
	}

	EGEdUtility_Deinit();
	EGCrcDb::Deinit();
	EGResourceLib_Deinit();

	EGToolMem_Deinit();

	return 0;
}

bool EGLibExtern_LoadNowTo(char const *,class EGFileData &)
{
	assert( false ); // Not supported.
	return false;
}

eg_d_string EGEdApp_GetEditorExecutable()
{
	eg_string_small GameToLaunch = EGToolsHelper_GetEnvVar( "EGGAME" );
	eg_string_small ExecutableName = EGString_Format( "%s_x64_ReleaseEditor.exe" , GameToLaunch.String() );
#if 1
	// Since EGEdApp is in the same direcory as the executable just exe is enough.
	return ExecutableName.String();
#else
	eg_char8 ThisAppFilename[MAX_PATH];
	GetModuleFileNameA( GetModuleHandleW(NULL) , ThisAppFilename , countof(ThisAppFilename) );
	egPathParts AppDirParts;
	EGPath_BreakPath( ThisAppFilename , AppDirParts );
	eg_string BinPath = AppDirParts.GetDirectory( '\\' );
	eg_string_big FullExeName = EGString_Format( "%s\\%s" , BinPath.String() , ExecutableName.String() );
	return EGPath_CleanPath( FullExeName , '\\' ).String();
#endif
}

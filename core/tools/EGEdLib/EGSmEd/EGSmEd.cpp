// (c) 2016 Beem Media

#include "EGSmEd.h"
#include "../EGEdResLib/resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGParse.h"
#include "EGSmEdVarMgr.h"
#include "EGSmEdResources.h"
#include "EGSmEdNodeWnd.h"
#include "EGSmEdScriptPanel.h"
#include "EGSmEdPropPanel.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGResourceLib.h"

const eg_char16 EGSmEd_Title[] = L"EGSM Editor";

EGSmFile2 EGSmEd_SmFile;

class EGSmEdMainWnd : public EGWndAppWindow , public ISmEdApp
{
private: typedef EGWndAppWindow Super;
private:

	EGWndPanel         m_MainPanel;
	EGSmEdScriptPanel* m_ScriptPanel = nullptr;
	EGSmEdPropPanel*   m_PropPanel = nullptr;
	EGSmEdPropPanel*   m_SettingsPanel = nullptr;
	eg_char16          m_LastSaveFilename[256];
	ISmEdApp::egFocus  m_Focus;
	int                m_nShowCmd;
	eg_bool            m_bIsDone:1;
	eg_bool            m_bIsDirty:1;

public:

	EGSmEdMainWnd( int nCmdShow , eg_cpstr16 InitialFilename )
	: EGWndAppWindow( EGSmEd_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_nShowCmd( nCmdShow )
	, m_bIsDone( false )
	, m_bIsDirty( false )
	, m_Focus( CT_Clear )
	{
		SetMenu( GetWnd() , LoadMenuW( EGResourceLib_GetLibrary() , L"EGCE_MAINMENU" ) );

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );

		// EGWndPanel* LeftPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		// LeftPane->SetFillSize( eg_panel_size_t::Fixed , EGWndPanel::GetPane3SideWidth() );

		EGWndPanel* ScriptPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		ScriptPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		EGWndTabbedPanel* RightPane = MainContainer->AddChild<EGWndTabbedPanel>();
		RightPane->SetFillSize( eg_panel_size_t::Fixed , EGWndPanel::GetPane3SideWidth() );

		EGWndTabbedPanel* PropPane = RightPane;
		EGWndTabbedPanel* SettingsPane = RightPane;

		// Script Pane:
		{
			ScriptPane->AddChild<EGWndPanelHeader>( "Script" );
			m_ScriptPanel = ScriptPane->AddChild<EGSmEdScriptPanel>();
			m_ScriptPanel->Init( this );

			// if( IS_DEBUG )
			{
				ScriptPane->AddChild<EGWndPanelHeader>( "Debug Log" );
				EGWndDebugLogPanel* DebugLogPanel = ScriptPane->AddChild<EGWndDebugLogPanel>();
				DebugLogPanel->SetFillSize( eg_panel_size_t::Percent , 15 );
			}
		}

		// Right Pane:
		{
			m_PropPanel = PropPane->AddTab<EGSmEdPropPanel>( "Properties" , EGSmEdPropPanel::egsm_panel_t::STATE_PROPS , this  );
		}

		// Left Pane:
		{
			m_SettingsPanel = SettingsPane->AddTab<EGSmEdPropPanel>( "Machine Settings" , EGSmEdPropPanel::egsm_panel_t::SETTINGS , this );
		}

		m_MainPanel.HandleResize();
		m_SettingsPanel->RefreshLayout();
		m_PropPanel->RefreshLayout();

		LoadSmFile( InitialFilename );
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		Super::OnAppStart( Args );

		EGWndAppParms Parms;
		EGWnd_GetAppParms( Args , Parms );

		if( Parms.ContainsType( "-localize" ) )
		{
			eg_string_big LocTo = Parms.GetParmValue( "-localize" );
			if( LocTo.Len() > 0 )
			{
				ExportToEn( LocTo );
			}
			m_bIsDone = true;
		}
		
		if( !m_bIsDone )
		{
			ShowWindow( GetWnd() , m_nShowCmd );
			UpdateWindow( GetWnd() );
		}
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void LoadSmFile( eg_cpstr16 Filename )
	{
		eg_bool bStartingEmpty = false;

		if( Filename && EGString_StrLen( Filename ) > 0 )
		{
			EGString_Copy( m_LastSaveFilename , Filename , countof(m_LastSaveFilename) );
			EGSmEd_GetFile().LoadForTool( Filename );
			if( EGSmEd_GetFile().GetNodeCount() == 0 )
			{
				// If there was no nodes on load we probably opened a blank file.
				EGSmEd_GetFile().InitDefault();
			}
		}
		else
		{
			m_LastSaveFilename[0] = '\0';
			EGSmEd_GetFile().Clear();
			EGSmEd_GetFile().InitDefault();
			bStartingEmpty = true;
		}
		EGSmEdVarMgr_InitForFile( Filename );
		EGSmEdResources_RebuildContextMenus();
		m_bIsDirty = false;
		if( m_ScriptPanel )
		{
			m_ScriptPanel->ReinitializeStates();
		}
		UpdateHeaderText();

		if( m_PropPanel )
		{
			m_PropPanel->RefreshLayout();
		}
		if( m_SettingsPanel )
		{
			m_SettingsPanel->RefreshLayout();
		}

		if( !bStartingEmpty )
		{
			ShowSettingsIfNoIdIsSet();
		}
	}

	virtual EGSmEdPropPanel* GetPropPanel() override
	{
		return m_PropPanel;
	}

	virtual EGSmEdPropPanel* GetSettingsPanel() override
	{
		return m_SettingsPanel;
	}

	virtual EGSmEdScriptPanel* GetScriptPanel() override
	{
		return m_ScriptPanel;
	}


	virtual void SetFocusedNode( const egFocus& NewFocus ) override
	{
		if( m_Focus != NewFocus )
		{
			egFocus OldFocus = m_Focus;
			m_Focus = NewFocus;
			if( m_PropPanel )
			{
				m_PropPanel->OnFocusChanged( m_Focus );
			}
			if( m_ScriptPanel )
			{
				m_ScriptPanel->OnFocusChanged( m_Focus , OldFocus );
			}
		}
	}


	virtual egFocus GetFocusedNode() const override
	{
		return m_Focus;
	}

private:

	virtual LRESULT WndProc( UINT Msg , WPARAM wParam , LPARAM lParam )
	{
		switch( Msg )
		{
			case WM_ACTIVATEAPP:
			{
				eg_bool bActivating = wParam == TRUE;
				if( !bActivating )
				{
					GetScriptPanel()->OnAppLostFocus();
				}
			} break;
			case WM_QUIT:
			case WM_CLOSE:
			{
				HandleQuitRequest();
			} return 0;
			case WM_GETMINMAXINFO:
			{
				MINMAXINFO* MinMaxInfo = reinterpret_cast<MINMAXINFO*>( lParam );
				MinMaxInfo = MinMaxInfo;
				MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3SideWidth()*2 + EGWndPanel::GetPane3MidMinWidth();
				MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
			} return 0;
		}

		return Super::WndProc( Msg , wParam , lParam );
	}

private:

	void PrepareForEnExport()
	{
		for( eg_size_t i=0; i<EGSmEd_GetFile().GetNodeCount(); i++ )
		{
			egsmNodeScr& Node = EGSmEd_GetFile().GetNode( i );
			if( Node.Type == egsm_node_t::NATIVE_EVENT )
			{
				egsmVarDeclScr FnDecl = EGSmEdVarMgr_GetFunctionInfo( eg_string_crc(Node.Parms[0]) );
				if( FnDecl.DeclType == egsm_var_decl_t::FUNCTION )
				{
					for( eg_size_t i=0; i<countof(FnDecl.ParmInfo); i++ )
					{
						if( FnDecl.ParmInfo[i].Type == egsm_var_t::LOC_TEXT )
						{
							Node.EnParmIndex = i+1; // Plus one because the first parm in the state is the event name.
							break;
						}
					}
				}
			}
		}
	}

	void ExportToEn( eg_cpstr Filename = nullptr )
	{
		eg_char16 SaveFilename[1024] = { '\0' };
		if( Filename )
		{
			EGString_Copy( SaveFilename , Filename , countof(SaveFilename) );
		}
		eg_bool bShouldSave = Filename != nullptr ? true : EGWndHelper_GetFile( m_hwnd , SaveFilename , countof(SaveFilename) , true , L"EG Localization (*.eloc)\0*.eloc\0XML (*.xml)\0*.xml\0" );
		if( bShouldSave )
		{
			if( !EGString_Contains( SaveFilename , L"." ) )
			{
				EGString_StrCat( SaveFilename , countof(SaveFilename) , L".eloc" );
			}
			PrepareForEnExport();
			eg_bool bSaved = EGSmEd_GetFile().SaveEnLoc( SaveFilename );
			if( !bSaved )
			{
				MessageBoxW( m_hwnd , L"Failed to save!" , EGSmEd_Title , MB_ICONERROR );
			}
		}
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override
	{
		unused( bFromAccelerator );

		switch (CmdId)
		{
		case EGCE_MAINMENU_EXPORTENUS:
		{
			ExportToEn();
		} break;
		case EGCE_MAINMENU_FIXLOCIDS:
		{
			EGWnd_ClearFocus();
			SetFocusedNode( CT_Clear );
			PrepareForEnExport();
			EGSmEd_GetFile().FixLocIds();
			SetDirty();
			m_ScriptPanel->ReinitializeStates();
		} break;
		case EGCE_MAINMENU_NEW:
		{
			eg_bool bDoNew = true;
			if( m_bIsDirty )
			{
				bDoNew = IDYES == MessageBoxW( m_hwnd , L"You have unsaved changes, are you sure you want to continue?" , EGSmEd_Title , MB_YESNO );
			}
			if( bDoNew )
			{
				SetFocusedNode( CT_Clear );
				m_LastSaveFilename[0] = '\0';
				EGSmEdVarMgr_InitForFile( m_LastSaveFilename );
				EGSmEdResources_RebuildContextMenus();
				EGSmEd_GetFile().Clear();
				EGSmEd_GetFile().InitDefault();
				m_ScriptPanel->ReinitializeStates();
				m_SettingsPanel->RefreshLayout();
				UpdateHeaderText();
			}
		} break;
		case EGCE_MAINMENU_OPEN:
		{
			eg_bool bDo = true;
			if( m_bIsDirty )
			{
				bDo = IDYES == MessageBoxW( m_hwnd , L"You have unsaved changes, are you sure you want to continue?" , EGSmEd_Title , MB_YESNO );
			}
			if( bDo )
			{
				DoOpen();
			}
		} break;
		case EGCE_MAINMENU_SAVE:
		{
			DoSave( false );
		} break;
		case EGCE_MAINMENU_SAVEAS:
		{
			DoSave( true );
			EGSmEdVarMgr_InitForFile( m_LastSaveFilename );
			EGSmEdResources_RebuildContextMenus();
		} break;
		case EGCE_MAINMENU_QUIT:
			HandleQuitRequest();
			break;
		default:
			assert( false ); // Not handled
			break;
		}
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize )
	{
		Super::OnWmSize( NewClientSize );

		m_MainPanel.HandleResize();
	}

	void ShowSettingsIfNoIdIsSet()
	{
		if( EGString_StrLen( EGSmEd_GetFile().GetMachineProps().Id ) == 0 )
		{
			MessageBoxW( GetWnd() , L"No Machine ID has been set. This script will not run properly without a unique ID." , EGSmEd_Title , MB_ICONWARNING );
		}
	}

	virtual void UpdateHeaderText() override
	{
		eg_char16 WindowTitle[1024];
		EGString_FormatToBuffer( WindowTitle , countof(WindowTitle) , L"%s %s[%s]" , EGSmEd_Title , m_bIsDirty ? L"*" : L"" , m_LastSaveFilename );
		SetWindowTextW( m_hwnd , WindowTitle );
	}

	void DoSave( eg_bool bForceUseFileSelector )
	{
		ShowSettingsIfNoIdIsSet();

		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , L"EG State Machine Script (*.egsm)\0*.egsm\0" );
		}

		if( bShouldSave )
		{
			eg_bool bFileOkay = EGSmEd_GetFile().VerifyIntegrity();

			if( !bFileOkay )
			{
				// We'll show a message box, but we'll still save.
				MessageBoxW( m_hwnd , L"There was a crc collision in the state names!" , EGSmEd_Title , MB_ICONERROR );
			}

			if( !EGString_Contains( m_LastSaveFilename , L"." ) )
			{
				EGString_StrCat( m_LastSaveFilename , countof(m_LastSaveFilename) , L".egsm" );
			}
			eg_bool bSaved = EGSmEd_GetFile().Save( m_LastSaveFilename );
			EGSmEdVarMgr_InitForFile( m_LastSaveFilename );
			EGSmEdResources_RebuildContextMenus();
			if( !bSaved )
			{
				MessageBoxW( m_hwnd , L"Failed to save!" , EGSmEd_Title , MB_ICONERROR );
			}
			else
			{
				m_bIsDirty = false;
			}
		}

		UpdateHeaderText();
	}

	void DoOpen()
	{
		SetFocusedNode( CT_Clear );
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , L"EG State Machine Script (*.egsm)\0*.egsm\0" ) )
		{
			LoadSmFile( m_LastSaveFilename );
		}
	}

	void HandleQuitRequest()
	{
		if( m_bIsDirty )
		{
			int MsgRes = MessageBoxW( m_hwnd , L"You have unsaved changes, would you like to save before exiting?" , EGSmEd_Title , MB_YESNOCANCEL );
			if( MsgRes == IDYES )
			{
				DoSave( false );
				if( !m_bIsDirty )
				{
					m_bIsDone = true;
				}
			}
			else if( MsgRes == IDNO )
			{
				m_bIsDone = true;
			}
		}
		else
		{
			m_bIsDone = true;
		}
	}

	virtual void SetDirty() override
	{
		m_bIsDirty = true;
		UpdateHeaderText();
	}

	virtual void RefreshProperties()
	{
		if( m_PropPanel )
		{
			m_PropPanel->RefreshLayout();
		}
	}
};

#if defined( __EGEDITORLIB__ )

int EGSmEd_Run( eg_cpstr16 InitialFilename, const struct egSdkEngineInitParms& EngineParms )
{
	unused( EngineParms );

	HINSTANCE hInstance = GetModuleHandleW( nullptr );

	static const egWndAppInfo AppInfo( hInstance , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , false , MAKEINTRESOURCEW(IDI_EGSM) , MAKEINTRESOURCEW(IDI_EGSM) , L"EGCE_ACCELERATORS" );

	EGSmEdResources_Init( hInstance );

	EGWnd_RunApp( AppInfo , [&InitialFilename](int nCmdShow)->EGWndAppWindow*{ return new EGSmEdMainWnd( nCmdShow , InitialFilename ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 

	EGSmEdResources_Deinit( hInstance );

	EGSmEdVarMgr_Deinit();

	return 0;
}

#else

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	unused( hPrevInstance );
	unused( lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , hInstance , nCmdShow , false , L"EGCE_ICON" , L"EGCE_ICON_SMALL" , L"EGCE_ACCELERATORS" );

	EGCrcDb::Init();
	EGSmEdResources_Init( hInstance );

	// eg_size_t MemChunkSize = 512*1024*1024;
	// eg_byte* MemChunk = new eg_byte[ MemChunkSize ];
	// EGMem2_Init( MemChunk , MemChunkSize );

	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGSmEdMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 

	// EGMem2_Deinit();
	// delete [] MemChunk;

	EGSmEdResources_Deinit( hInstance );
	EGCrcDb::Deinit();
	return 0;
}

#pragma comment( lib , "lib3p.lib" )
#pragma comment( lib , "eglib.lib" )
#include "EGLibExtern_Tool.hpp"
#include "EGSmFile2.cpp"
#include "EGSmVars.cpp"
#include "EGSmTypes.cpp"

#endif
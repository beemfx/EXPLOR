// (C) 2016 Beem Media

#include "EGLytEd.h"
#include "EGEngineApp.h"
#include "EGWndAppWindow.h"
#include "../EGEdResLib/resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGRenderer.h"
#include "EGWndPanel.h"
#include "EGWndTabbedPanel.h"
#include "EGLytEdLibPanel.h"
#include "EGLytEdPreviewPanel.h"
#include "EGLytEdLayoutPanel.h"
#include "EGLytEdPropPanel.h"
#include "EGLytEdSettingsPanel.h"
#include "EGEngine.h"
#include "EGLytEdFile.h"
#include "EGEngineForTools.h"
#include "EGTimer.h"
#include "EGDebugText.h"
#include "EGEGWinCon.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGEngineMem.h"
#include "EGCrcDb.h"
#include "EGFileData.h"
#include "EGWndDebugLogPanel.h"
#include "EGLogDispatcher.h"
#include "EGResourceLib.h"
#include "EGEdUtility.h"
#include "EGEdLib.h"

void EGLytEd_DrawPreview()
{
	EGRenderer::Get().BeginFrame();

	EGLytEdFile::Get().Draw();
	EGLogToScreen::Get().Draw( EGLytEdFile::Get().GetPreviewAspectRatio() );

	EGRenderer::Get().EndFrame();


	EngineForTools_Draw();
}

class EGLytEdMainWnd : public EGWndAppWindow
{
private: typedef EGWndAppWindow Super;

public:

	static eg_cpstr16 AppName;
	static EGLytEdMainWnd* GlobalInst;

private:

	EGWndPanel            m_MainPanel;
	eg_char16             m_LastSaveFilename[256];
	eg_string_big         m_CurrentGameFilename;
	eg_bool               m_bIsDirty:1;
	eg_bool               m_bIsDone:1;

public:

	EGLytEdMainWnd( int nCmdShow , eg_cpstr16 InitialFile  )
	: EGWndAppWindow( AppName )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_bIsDirty( false )
	, m_bIsDone( false )
	{
		assert( nullptr == GlobalInst );
		GlobalInst = this;
		m_LastSaveFilename[0] = '\0';

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );

		EGWndPanel* LeftPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		LeftPane->SetFillSize( eg_panel_size_t::Percent , 18 );
		EGWndPanel* PreviewContainer = MainContainer->AddContainer( eg_panel_fill::Vertical );
		EGWndTabbedPanel* RightPane = MainContainer->AddChild<EGWndTabbedPanel>();
		RightPane->SetFillSize( eg_panel_size_t::Percent , 25 );

		// Preview Panel:
		{
			EGWndBgPanel* PreviewBg = PreviewContainer->AddChild<EGWndBgPanel>( RGB(0,0,0) );
			PreviewBg->AddChild<EGLytEdPreviewPanel>();

			PreviewContainer->AddChild<EGWndPanelHeader>( "Debug Log" );
			EGWndDebugLogPanel* DebugLogPanel = PreviewContainer->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Percent , 25 );
		}

		// Left Pane:
		{
			EGWndPanel* LayoutContainer = LeftPane->AddContainer( eg_panel_fill::Vertical );
			LayoutContainer->AddChild<EGWndPanelHeader>( "Layout" );
			LayoutContainer->AddChild<EGLytEdLayoutPanel>();
			
			EGWndPanel* LibContainer = LeftPane->AddContainer( eg_panel_fill::Vertical );
			LibContainer->AddChild<EGWndPanelHeader>( "Asset Library" );
			LibContainer->AddChild<EGLytEdLibPanel>();
		}

		// Right Pane:
		{
			RightPane->AddTab<EGLytEdPropPanel>( "Properties" );
			RightPane->AddTab<EGLytEdSettingsPanel>( "Layout Settings" );
		}

		m_MainPanel.HandleResize();

		EGLytEdFile::Get().RefreshSettingsPanel();

		SetForegroundWindow( m_hwnd );

		UpdateHeaderText();

		EGLytEdFile::Get().SetPreviewAspectRatio( 16.f/9.f );

		SetMenu( GetWnd() , LoadMenuW( EGResourceLib_GetLibrary() , L"EGLAYOUT_MAIN_MENU" ) );

		ShowWindow(m_hwnd, nCmdShow);
		UpdateWindow(m_hwnd);

		if( InitialFile )
		{
			LoadLytFile( InitialFile );
		}
	}

	~EGLytEdMainWnd()
	{
		assert( this == GlobalInst );
		GlobalInst = nullptr;
	}

	void UpdateHeaderText()
	{
		eg_char16 WindowTitle[1024];
		eg_string_big GameName = EGToolsHelper_GetEnvVar( "EGGAME" );
		GameName.ConvertToUpper();
		m_CurrentGameFilename = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte( m_LastSaveFilename ) );
		EGString_FormatToBuffer( WindowTitle , countof( WindowTitle ) , L"%s %s %s[%s]" , AppName , EGString_ToWide( GameName ) , m_bIsDirty ? L"*" : L"" , EGString_ToWide( m_CurrentGameFilename ) /* m_LastSaveFilename */ );
		SetWindowTextW( m_hwnd , WindowTitle );
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		unused( Args );

	#if ( !defined( __EGEDITORLIB__ ) )
		eg_cpstr16 LoadFile = Args.IsValidIndex(1) ? Args[1] : nullptr;
		if( LoadFile )
		{
			LoadLytFile( LoadFile );
		}
	#endif
	}

	virtual void OnAppUpdate( eg_real DeltaTime ) override final
	{
		EngineForTools_Update( DeltaTime );
		EGLytEdFile::Get().Update( DeltaTime );
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "Tool FPS: %u", eg_uint( 1.f / DeltaTime ) ) );

		EGLytEd_DrawPreview();
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void DoSave( eg_bool bForceUseFileSelector )
	{
		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , L"EG Layout (*.elyt)\0*.elyt\0" );
		}

		if( bShouldSave )
		{
			eg_bool bFileOkay = true;//m_ConvFile.VerifyIntegrity();

			if( bFileOkay )
			{
				if( !EGString_Contains( m_LastSaveFilename , L"." ) )
				{
					EGString_StrCat( m_LastSaveFilename , countof(m_LastSaveFilename) , L".elyt" );
				}
				eg_bool bSaved = EGLytEdFile::Get().Save( m_LastSaveFilename );
				if( !bSaved )
				{
					MessageBoxW( m_hwnd , L"Failed to save!" , AppName , MB_ICONERROR );
				}
				else
				{
					m_bIsDirty = false;
				}
			}
			else
			{
				MessageBoxW( m_hwnd , L"There was something wrong with the file in the state names!" , AppName , MB_ICONERROR );
			}

		}

		UpdateHeaderText();
	}

	void DoOpen()
	{
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , L"EG Layout (*.elyt)\0*.elyt\0" ) )
		{
			LoadLytFile( m_LastSaveFilename );
		}
	}

	void LoadLytFile( eg_cpstr16 Filename )
	{
		if( CanOpenInCurrentGame( EGString_ToMultibyte( Filename ) ) )
		{
			EGString_Copy( m_LastSaveFilename , Filename , countof( m_LastSaveFilename ) );

			UpdateHeaderText();

			EGLytEdFile::Get().Open( m_LastSaveFilename );

			m_bIsDirty = false;
		}
	}

	eg_bool CmdProc( int wmId )
	{
		switch (wmId)
		{
		case ID_FILE_NEW:
		{
			eg_bool bDo = true;
			if( m_bIsDirty )
			{
				bDo = IDYES == MessageBoxW( m_hwnd , L"You have unsaved changes, are you sure you want to continue?" , AppName , MB_YESNO );
			}
			if( bDo )
			{
				EGLytEdFile::Get().Deinit();
				EGLytEdFile::Get().Init();
				EGLytEdLayoutPanel::GetPanel()->RefreshPanel( -1 );
				m_LastSaveFilename[0] = '\0';
				m_bIsDirty = false;
				UpdateHeaderText();
			}
		} break;
		case ID_FILE_QUIT:
			HandleQuitRequest();
			break;
		case ID_PREVIEW_16X9:
			EGLytEdFile::Get().SetPreviewAspectRatio( 16.f/9.f );
			m_MainPanel.HandleResize();
			break;
		case ID_PREVIEW_16X10:
			EGLytEdFile::Get().SetPreviewAspectRatio( 16.f/10.f );
			m_MainPanel.HandleResize();
			break;
		case ID_PREVIEW_4X3:
			EGLytEdFile::Get().SetPreviewAspectRatio( 4.f/3.f );
			m_MainPanel.HandleResize();
			break;
		case ID_PREVIEW_SHOWTEXTOUTLINES:
		{
			EGLytEdFile::Get().ToggleShowTextOutlines();
		} break;
		case ID_PREVIEW_REBUILD:
		{
			EGLytEdFile::Get().FullRebuild();
		} break;
		case ID_FILE_OPEN:
		{
			eg_bool bDo = true;
			if( m_bIsDirty )
			{
				bDo = IDYES == MessageBoxW( m_hwnd , L"You have unsaved changes, are you sure you want to continue?" , AppName , MB_YESNO );
			}
			if( bDo )
			{
				DoOpen();
			}
		} break;
		case ID_FILE_SAVE:
		{
			DoSave( false );
		} break;
		case ID_FILE_SAVEAS:
		{
			DoSave( true );
		} break;
		case ID_TOOLS_CONVERTALLTEXTTOBWNOSHADOW:
		{ 
			EGLytEdFile::Get().SetAllTextToBWNoShadow();
		} break;
		default:
			return false;
		}
		return true;
	}

	void HandleQuitRequest()
	{
		if( m_bIsDirty )
		{
			int MsgRes = MessageBoxW( m_hwnd , L"You have unsaved changes, would you like to save before exiting?" , AppName , MB_YESNOCANCEL );
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

	virtual LRESULT WndProc( UINT message, WPARAM wParam, LPARAM lParam) override final
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		{
			EGWnd_ClearFocus();
		} return DefWindowProcW(m_hwnd, message, wParam, lParam);
		case WM_COMMAND:
		{
			if( CmdProc(LOWORD(wParam)) )
			{

			}
			else
			{
				return DefWindowProcW(m_hwnd, message, wParam, lParam);
			}
		}
		break;
		case WM_QUIT:
		case WM_CLOSE:
		{
			HandleQuitRequest();
		} break;
		case WM_SIZE:
		{
			m_MainPanel.HandleResize();
		} break;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* MinMaxInfo = reinterpret_cast<MINMAXINFO*>( lParam );
			MinMaxInfo = MinMaxInfo;
			MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3SideWidth()*2 + EGWndPanel::GetPane3MidMinWidth();
			MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
		} break;
		case WM_KEYDOWN:
		{
			if( 'L' == wParam )
			{
				if( EGLytEdLayoutPanel::GetPanel() )
				{
					EGLytEdLayoutPanel::GetPanel()->OnToggleLocked();
				}
			}
		} return DefWindowProcW( m_hwnd , message , wParam , lParam );
		default:
			return DefWindowProcW(m_hwnd, message, wParam, lParam);
		}
		return 0;
	}
	
	void SetDirty()
	{
		m_bIsDirty = true;
		UpdateHeaderText();
	}

	eg_bool CanOpenInCurrentGame( eg_cpstr Filename )
	{
		eg_string RelativeTo = *EGPath2_GetRelativePathTo( Filename , EGToolsHelper_GetEnvVar( "EGSRC" ) );
		egPathParts2 PathParts = EGPath2_BreakPath( RelativeTo );
		if( PathParts.Root.Len() == 0 )
		{
			if( PathParts.Folders.Len() >= 3 && PathParts.Folders[2].EqualsI( L"data" ) )
			{
				if( !PathParts.Folders[1].EqualsI( *eg_d_string16(EGToolsHelper_GetEnvVar( "EGGAME" )) ) )
				{
					eg_string_big StrMessage = EGString_Format( "This asset is part of \"%s\" EGGAME must be changed to open this. Do you want to change it now? (Application must be restarted to open this file.)" , *eg_d_string8(*PathParts.Folders[0]) );
					if( IDYES == MessageBoxW( GetWnd() , EGString_ToWide( StrMessage ) , AppName , MB_YESNO ) )
					{
						EGToolsHelper_SetEnvVar( "EGGAME" , *eg_d_string8(*PathParts.Folders[1]) );
					}

					return false;
				}
			}
		}

		return true;
	}

	eg_string_big GetCurrentGameFilename() const { return m_CurrentGameFilename; }
};

eg_cpstr16 EGLytEdMainWnd::AppName = L"EG Layout Editor";
EGLytEdMainWnd* EGLytEdMainWnd::GlobalInst = nullptr;


void EGLytEd_SetDirty()
{
	if( EGLytEdMainWnd::GlobalInst )
	{
		EGLytEdMainWnd::GlobalInst->SetDirty();
	}
}

eg_string_big EGLytEd_GetCurrentGameFilename()
{
	return EGLytEdMainWnd::GlobalInst ? EGLytEdMainWnd::GlobalInst->GetCurrentGameFilename() : "";
}

#if defined( __EGEDITORLIB__ )

int EGLytEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms )
{
	static const egWndAppInfo AppInfo( GetModuleHandleW( nullptr ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , true , MAKEINTRESOURCEW(IDI_ELYT), MAKEINTRESOURCEW(IDI_ELYT), L"EGLAYOUT_ACCELERATOR" );

	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.GameName = EngineParms.GameName;
	InitParms.bAllowDedicatedServer = false;
	InitParms.bAllowCommandLineMap = false;
	EngineForTools_Init( InitParms , false );

	EGLytEdFile::Get().Init();

	EGWnd_RunApp( AppInfo , [&InitialFilename](int nCmdShow)->EGWndAppWindow*{ return new EGLytEdMainWnd( nCmdShow , InitialFilename ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } );

	EGLytEdFile::Get().Deinit();

	EngineForTools_Deinit( false );

	return 0;
}

#else

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	unused( hPrevInstance , lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , EGResourceLib_GetLibrary() , nCmdShow, true, L"EGLAYOUT_ICON", L"EGLAYOUT_ICON_SMALL", L"EGLAYOUT_ACCELERATOR" );

	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.GameName = EGToolsHelper_GetEnvVar( "EGGAME" );
	InitParms.bAllowDedicatedServer = false;
	InitParms.bAllowCommandLineMap = false;
	EngineForTools_Init( InitParms , true );

	EGLytEdFile::Get().Init();

	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGLytEdMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } );

	EGLytEdFile::Get().Deinit();

	EngineForTools_Deinit( true );

	return 0;
}

#endif

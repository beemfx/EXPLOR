// (C) 2017 Beem Media

#include "EGEngineApp.h"
#include "EGWndAppWindow.h"
#include "../EGEdResLib/resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGRenderer.h"
#include "EGWndPanel.h"
#include "EGDefEdLibPanel.h"
#include "EGDefEdPreviewPanel.h"
#include "EGDefEdComponentsPanel.h"
#include "EGDefEdPropPanel.h"
#include "EGEngine.h"
#include "EGDefEdFile.h"
#include "EGEngineForTools.h"
#include "EGTimer.h"
#include "EGDebugText.h"
#include "EGEGWinCon.h"
#include "EGDefEd.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGFileData.h"
#include "EGEngineMem.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGDefEdSettingsPanel.h"
#include "EGDefEdTimelineEditor.h"
#include "EGDefEdEditorConfigPanel.h"
#include "EGDefEdPreview.h"
#include "EGResourceLib.h"
#include "EGEdUtility.h"
#include "EGEdLib.h"

static eg_bool EGDefEd_bIsNewSystem = true;


class EGDefEdMainWnd : public EGWndAppWindow
{
private: typedef EGWndAppWindow Super;

public:

	static eg_cpstr16 AppName;
	static EGDefEdMainWnd* GlobalInst;

private:

	EGWndPanel                m_MainPanel;
	EGDefEdSettingsPanel*     m_SettingsPanel = nullptr;
	EGDefEdEditorConfigPanel* m_EditorConfigPanel = nullptr;
	EGDefEdTimelineEditor*    m_TimelineEditor = nullptr;
	EGDefEdPreviewPanel*      m_PreviewPanel = nullptr;
	eg_string_big             m_CurrentGameFilename;
	eg_char16                 m_LastSaveFilename[256];
	eg_bool                   m_bIsDirty:1;
	eg_bool                   m_bIsDone:1;

public:

	EGDefEdMainWnd( int nCmdShow , eg_cpstr16 InitialFilename )
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
		EGWndTabbedPanel* RightPane = MainContainer->AddChild<EGWndTabbedPanel>();
		RightPane->SetFillSize( eg_panel_size_t::Percent , 25 );
		EGWndPanel* CompLibAndPreview = LeftPane->AddContainer( eg_panel_fill::Horizontal );
		EGWndPanel* CompLib = CompLibAndPreview->AddContainer( eg_panel_fill::Vertical );
		CompLib->SetFillSize( eg_panel_size_t::Fixed , EGWndPanel::GetPane3SideWidth() );

		EGWndPanel* PreviewContainer = CompLibAndPreview->AddContainer( eg_panel_fill::Vertical );

		// Preview Panel:
		{
			EGWndBgPanel* PreviewBg = PreviewContainer->AddChild<EGWndBgPanel>( RGB(0,0,0) );
			m_PreviewPanel = PreviewBg->AddChild<EGDefEdPreviewPanel>();

			EGWndTabbedPanel* TimelineDebugContainer = LeftPane->AddChild<EGWndTabbedPanel>();
			TimelineDebugContainer->SetFillSize( eg_panel_size_t::Percent , 28 );

			m_TimelineEditor = TimelineDebugContainer->AddTab<EGDefEdTimelineEditor>( "Timeline" );
			EGWndDebugLogPanel* DebugLog = TimelineDebugContainer->AddTab<EGWndDebugLogPanel>( "Debug Log" );
		}

		// Left Pane:
		{
			EGWndPanel* ComponentsContainer = CompLib->AddContainer( eg_panel_fill::Vertical );
			ComponentsContainer->AddChild<EGWndPanelHeader>( "Components" );
			ComponentsContainer->AddChild<EGDefEdComponentsPanel>();

			EGWndPanel* LibContainer = CompLib->AddContainer( eg_panel_fill::Vertical );
			LibContainer->SetFillSize( eg_panel_size_t::Percent , 35 );
			LibContainer->AddChild<EGWndPanelHeader>( "Component Library" );
			LibContainer->AddChild<EGDefEdLibPanel>();
		}

		// Right Pane:
		{
			RightPane->AddTab<EGDefEdPropPanel>( "Properties" );
			m_SettingsPanel = RightPane->AddTab<EGDefEdSettingsPanel>( "Class Settings" );
			m_EditorConfigPanel = RightPane->AddTab<EGDefEdEditorConfigPanel>( "Editor Config" );
		}

		m_MainPanel.HandleResize();

		SetForegroundWindow( m_hwnd );

		UpdateHeaderText();

		SetMenu( GetWnd() , LoadMenuW( EGResourceLib_GetLibrary() , L"EGDEFED_MAIN_MENU" ) );

		ShowWindow(m_hwnd, nCmdShow);
		UpdateWindow(m_hwnd);

		if( InitialFilename )
		{
			LoadDefFile( InitialFilename );
		}
	}

	~EGDefEdMainWnd()
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
		EGString_FormatToBuffer( WindowTitle , countof(WindowTitle) , L"%s %s %s[%s]" , AppName , EGString_ToWide(GameName) , m_bIsDirty ? L"*" : L"" , EGString_ToWide(m_CurrentGameFilename) /* m_LastSaveFilename */ );
		SetWindowTextW( m_hwnd , WindowTitle );
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		unused( Args );

#if !defined( __EGEDITORLIB__ )
		eg_cpstr16 LoadFile = Args.IsValidIndex(1) ? Args[1] : nullptr;
		if( LoadFile )
		{
			LoadDefFile( LoadFile );
		}
#endif
	}

	virtual void OnAppUpdate( eg_real DeltaTime ) override final
	{
		EngineForTools_Update( DeltaTime );
		EGDefEdFile::Get().Update( DeltaTime );
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "Tool FPS: %u", eg_uint( 1.f / DeltaTime ) ) );

		if( m_PreviewPanel )
		{
			m_PreviewPanel->HandleAppUpdate( DeltaTime );
		}

		EGDefEdPreview::Get().Draw();
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void DoSave( eg_bool bForceUseFileSelector )
	{
		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , L"EG Entity Definition (*.edef)\0*.edef\0" );
		}

		if( bShouldSave )
		{
			eg_bool bFileOkay = true;//m_ConvFile.VerifyIntegrity();

			if( bFileOkay )
			{
				if( !EGString_Contains( m_LastSaveFilename , L"." ) )
				{
					EGString_StrCat( m_LastSaveFilename , countof(m_LastSaveFilename) , L".edef" );
				}
				eg_bool bSaved = EGDefEdFile::Get().Save( m_LastSaveFilename );
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
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , L"EG Entity Definition (*.edef)\0*.edef\0" ) )
		{
			LoadDefFile( m_LastSaveFilename );
		}
	}

	void LoadDefFile( eg_cpstr16 Filename )
	{
		if( CanOpenInCurrentGame( EGString_ToMultibyte(Filename) ) )
		{
			EGString_Copy( m_LastSaveFilename , Filename , countof(m_LastSaveFilename) );

			EGDefEdFile::Get().Open( m_LastSaveFilename );

			m_bIsDirty = false;
			UpdateHeaderText();
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
				EGDefEdFile::Get().Deinit();
				EGDefEdFile::Get().Init();
				EGDefEdPreview::Get().ResetCamera();
				EGDefEdComponentsPanel::GetPanel()->RefreshPanel( -1 );
				m_LastSaveFilename[0] = '\0';
				m_bIsDirty = false;
				UpdateHeaderText();
			}
		} break;
		case ID_FILE_QUIT:
			HandleQuitRequest();
			break;
		case ID_PREVIEW_SHOWTEXTOUTLINES:
		{
			EGDefEdFile::Get().ToggleShowTextOutlines();
		} break;
		case ID_PREVIEW_REBUILD:
		{
			EGDefEdFile::Get().FullRebuild();
		} break;
		case ID_PREVIEW_RESETCAMERA:
		{
			EGDefEdPreview::Get().ResetCamera();
		} break;
		case ID_SETBOUNDS_TOMESHBOUNDS:
		{
			EGDefEdFile::Get().SetBoundsTo( EGDefEdFile::eg_setbound_t::ToMeshBounds );
		} break;
		case ID_SETBOUNDS_TOPHYSICSBOUNDS:
		{
			EGDefEdFile::Get().SetBoundsTo( EGDefEdFile::eg_setbound_t::ToPhysicsBounds );
		} break;
		case ID_SETBOUND_TO_M_AND_P_BOUNDS:
		{
			EGDefEdFile::Get().SetBoundsTo( EGDefEdFile::eg_setbound_t::ToMeshAndPhysicsBounds );
		} break;
		case ID_TOOLS_CREATETEXTCOMPONENTS:
		{
			EGDefEdFile::Get().CreateTextComponents();
		} break;
		case ID_TOOLS_MOVECOMPONENTTOCAMERA:
		{
			if( EGDefEdComponentsPanel::GetPanel() && EGDefEdComponentsPanel::GetPanel()->GetEditComponent() )
			{
				EGDefEdFile::Get().MoveComponentToPose( EGDefEdComponentsPanel::GetPanel()->GetEditComponent() , EGDefEdPreview::Get().GetCameraController().GetTransform() );
			}
		} break;
		case ID_TOOLS_CONVERTBWNOSHADOW:
		{
			EGDefEdFile::Get().ConvertToBWNoShadow();
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
				if( EGDefEdComponentsPanel::GetPanel() )
				{
					EGDefEdComponentsPanel::GetPanel()->OnToggleLocked();
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
		if( !m_bIsDirty )
		{
			m_bIsDirty = true;
			UpdateHeaderText();
		}
	}

	eg_bool CanOpenInCurrentGame( eg_cpstr Filename )
	{
		eg_string RelativeTo = *EGPath2_GetRelativePathTo( Filename , EGToolsHelper_GetEnvVar("EGSRC") );
		egPathParts2 PathParts= EGPath2_BreakPath( RelativeTo );
		if( PathParts.Root.Len() == 0 )
		{
			if( PathParts.Folders.Len() >= 3 && PathParts.Folders[2].EqualsI( L"data" ) )
			{
				if( !PathParts.Folders[1].EqualsI( *eg_d_string16(EGToolsHelper_GetEnvVar("EGGAME")) ) && !PathParts.Folders[0].EqualsI(L"core") )
				{
					eg_string_big StrMessage = EGString_Format( "This asset is part of \"%s\" EGGAME must be changed to open this. Do you want to change it now? (Application must be restarted to open this file.)" , *eg_d_string8(*PathParts.Folders[0]) );
					if( IDYES == MessageBoxW( GetWnd() , EGString_ToWide(StrMessage) , AppName , MB_YESNO ) )
					{
						EGToolsHelper_SetEnvVar( "EGGAME" , *eg_d_string8(*PathParts.Folders[1]) );
					}

					return false;
				}
			}
		}

		return true;
	}

	EGWndPanelPropEditor* GetSettingsPanel(){ return m_SettingsPanel; }
	EGDefEdTimelineEditor* GetTimelineEditor(){ return m_TimelineEditor; }
	EGWndPanelPropEditor* GetEditorConfigPanel(){ return m_EditorConfigPanel; }
	eg_string_big GetCurrentGameFilename() const { return m_CurrentGameFilename; }
};

eg_cpstr16 EGDefEdMainWnd::AppName = L"EG Entity Definition Editor";
EGDefEdMainWnd* EGDefEdMainWnd::GlobalInst = nullptr;

void EGDefEd_SetDirty()
{
	if( EGDefEdMainWnd::GlobalInst )
	{
		EGDefEdMainWnd::GlobalInst->SetDirty();
	}
}

class EGWndPanelPropEditor* EGDefEd_GetSettingsPanel()
{
	return EGDefEdMainWnd::GlobalInst ? EGDefEdMainWnd::GlobalInst->GetSettingsPanel() : nullptr;
}

class EGWndPanelPropEditor* EGDefEd_GetEditorConfigPanel()
{
	return EGDefEdMainWnd::GlobalInst ? EGDefEdMainWnd::GlobalInst->GetEditorConfigPanel() : nullptr;
}

class EGDefEdTimelineEditor* EGDefEd_GetTimelineEditor()
{
	return EGDefEdMainWnd::GlobalInst ? EGDefEdMainWnd::GlobalInst->GetTimelineEditor() : nullptr;
}

eg_string_big EGDefEd_GetCurrentGameFilename()
{
	return EGDefEdMainWnd::GlobalInst ? EGDefEdMainWnd::GlobalInst->GetCurrentGameFilename() : "";
}

#if defined( __EGEDITORLIB__ )

int EGDefEd_Run( eg_cpstr16 InitialFilename, const struct egSdkEngineInitParms& EngineParms )
{
	static const egWndAppInfo AppInfo( GetModuleHandleW( nullptr ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , true , MAKEINTRESOURCEW(IDI_EDEF) , MAKEINTRESOURCEW(IDI_EDEF) , L"EGDEFED_ACCELERATOR" );

	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.GameName = EngineParms.GameName;
	InitParms.bAllowDedicatedServer = false;
	InitParms.bAllowCommandLineMap = false;
	EngineForTools_Init( InitParms , false );

	EGDefEdFile::Get().Init();

	EGWnd_RunApp( AppInfo , [&InitialFilename](int nCmdShow)->EGWndAppWindow*{ return new EGDefEdMainWnd( nCmdShow , InitialFilename ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } );

	EGDefEdFile::Get().Deinit();

	EngineForTools_Deinit( false );

	return 0;
}

eg_bool EGDefEd_IsNewSystem()
{
	return EGDefEd_bIsNewSystem;
}

#else

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	unused( hPrevInstance , lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , EGResourceLib_GetLibrary() , nCmdShow, true, MAKEINTRESOURCEW(IDI_EDEF), MAKEINTRESOURCEW(IDI_EDEF), L"EGDEFED_ACCELERATOR" );

	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.GameName = EGToolsHelper_GetEnvVar( "EGGAME" );
	InitParms.bAllowDedicatedServer = false;
	InitParms.bAllowCommandLineMap = false;
	EngineForTools_Init( InitParms , true );

	EGDefEdFile::Get().Init();

	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGDefEdMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } );

	EGDefEdFile::Get().Deinit();

	EngineForTools_Deinit( true );

	return 0;
}

#endif

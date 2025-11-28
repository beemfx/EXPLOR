// (c) 2017 Beem Media

#include "EGLpkViewer.h"
#include "EGWnd.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGWndHelpers.h"
#include "EGLpkFileListPanel.h"
#include "../EGEdResLib/resource.h"
#include "fs_sys2/fs_lpk.h"
#include "EGResourceLib.h"

const eg_char16 EGLpkViewer_Title[] = L"LPK Viewer";

class EGLpkViewerMainWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

private:

	EGWndPanel          m_MainPanel;
	EGLpkFileListPanel* m_FileListPanel = nullptr;
	eg_char16           m_LastSaveFilename[256];
	eg_bool             m_bIsDone:1;

public:

	EGLpkViewerMainWnd( int nCmdShow , eg_cpstr16 InitialFilename )
	: EGWndAppWindow( EGLpkViewer_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_bIsDone( false )
	{
		SetMenu( GetWnd(), LoadMenuW( EGResourceLib_GetLibrary() , L"LPKVIEWER_MAINMENU" ) );

		EGWndPanel* Container = nullptr;

		Container = m_MainPanel.AddContainer( eg_panel_fill::Vertical );
		Container->AddChild<EGWndPanelHeader>( "Files" );
		Container->AddChild<EGLpkFileListPanelHeader>();
		m_FileListPanel = Container->AddChild<EGLpkFileListPanel>();

		ShowWindow( GetWnd(), nCmdShow );
		UpdateWindow( GetWnd() );

		FS_SetMemFuncs( Malloc , Free );

		EGString_Copy( m_LastSaveFilename , InitialFilename , countof(m_LastSaveFilename) );
		LoadLpkFile( InitialFilename );
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void LoadLpkFile( eg_cpstr16 Filename )
	{
		if( Filename )
		{
			CLArchive Archive;
			Archive.Open( Filename );
			m_FileListPanel->OnArchiveUpdated( Archive );
			UpdateHeaderText();
		}
	}

private:

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam )
	{
		switch( Msg )
		{
		case WM_COMMAND:
		{
			if( CmdProc( LOWORD( wParam ) ) )
			{
				return 0;
			}
		}
		break;
		case WM_ACTIVATEAPP:
		{
			eg_bool bActivating = wParam == TRUE;
			unused( bActivating );
		} break;
		case WM_QUIT:
		case WM_CLOSE:
		{
			HandleQuitRequest();
		} return 0;
		case WM_SIZE:
		{
			m_MainPanel.HandleResize();
		} return 0;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* MinMaxInfo = reinterpret_cast<MINMAXINFO*>( lParam );
			MinMaxInfo = MinMaxInfo;
			MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3SideWidth() * 2 + EGWndPanel::GetPane3MidMinWidth();
			MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
		} return 0;
		}

		return Super::WndProc( Msg, wParam, lParam );
	}

private:

	eg_bool CmdProc( int wmId )
	{
		switch( wmId )
		{
		case ID_FILE_OPEN:
		{
			eg_bool bDo = true;
			if( bDo )
			{
				DoOpen();
			}
		} break;
		case ID_FILE_EXIT:
			HandleQuitRequest();
			break;
		default:
			return false;
		}
		return true;
	}

	void UpdateHeaderText()
	{
		eg_char16 WindowTitle[1024];
		EGString_FormatToBuffer( WindowTitle, countof( WindowTitle ), L"%s [%s]", EGLpkViewer_Title, m_LastSaveFilename );
		SetWindowTextW( m_hwnd, WindowTitle );
	}

	void DoOpen()
	{
		if( EGWndHelper_GetFile( m_hwnd, m_LastSaveFilename, countof( m_LastSaveFilename ), false, L"LPK Package (*.lpk)\0*.lpk\0" ) )
		{
			LoadLpkFile( m_LastSaveFilename );
		}
	}

	void HandleQuitRequest()
	{
		m_bIsDone = true;
	}

	//Memory allocation functions for the file system
	static void* Malloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line)
	{
		unused( reason , type , file , line );

		return reinterpret_cast<void*>(new eg_byte[size]);
	}

	static void  Free(void* p, LF_ALLOC_REASON reason)
	{
		unused( reason );
		delete [] reinterpret_cast<eg_byte*>(p);
	}
};

#if defined( __EGEDITORLIB__ )

int EGLpkViewer_Run( eg_cpstr16 InitialFilename, const struct egSdkEngineInitParms& EngineParms )
{
	unused( EngineParms );

	static const egWndAppInfo AppInfo( GetModuleHandleW( nullptr ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , false, MAKEINTRESOURCEW(IDI_LPK), nullptr, nullptr );

	EGWnd_RunApp( AppInfo, [&InitialFilename]( int nCmdShow )->EGWndAppWindow* { return new EGLpkViewerMainWnd( nCmdShow , InitialFilename ); }, []( EGWndAppWindow* AppWnd )->void { delete AppWnd; } );

	return 0;
}

#else

int APIENTRY wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
	unused( hPrevInstance );
	unused( lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , hInstance , nCmdShow, false, L"LPKICON", nullptr, nullptr );

	EGWnd_RunApp( AppInfo, []( int nCmdShow )->EGWndAppWindow* { return new EGLpkViewerMainWnd( nCmdShow ); }, []( EGWndAppWindow* AppWnd )->void { delete AppWnd; } );

	return 0;
}

#endif

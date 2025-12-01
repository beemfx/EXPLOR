// (c) 2017 Beem Media

// Basic foundation for a EGWnd application

#pragma comment( lib , "eglib.lib" )
#pragma comment( lib , "egfoundation.lib" )
#pragma comment( lib , "lib3p.lib" )

#include "resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGWndTestSamplePanels.h"
#include "EGWndPanelPropEditor.h"
#include "EGWndTestSamplePropStruct.h"
#include "EGResourceLib.h"
#include "EGLibExtern_Tool.hpp"
#include "EGToolMem.h"
#include "EGLibFile.h"
#include "EGWndFileBrowser.h"
#include "EGItemMap.h"
#include "EGRandom.h"
#include "EGWndTestSamplePropStruct.reflection.h"

static const eg_char16 EGWndApp_Title[] = L"TODO: Name Me";
static const eg_char16 EGWndApp_SaveFileOptions[] = L"TODO: Set files types (*.*)\0*.*\0";

class EGWndAppMainWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

private:

	EGWndPanel            m_MainPanel;
	EGWndPanelPropEditor* m_PropEditor = nullptr;
	EGWndFileBrowser*     m_FileBrowser = nullptr;
	EGWndTestPropStruct   m_Props;
	eg_char16             m_LastSaveFilename[256];
	int                   m_nShowCmd;
	eg_bool               m_bIsDone:1;
	eg_bool               m_bIsDirty:1;

public:

	void StartupTest()
	{
		/*
		eg_string_crc TestValue = eg_crc("Hi");
		eg_int64 BigNumber = -120394832098402480;
		eg_transform Transform = CT_Default;
		eg_d_string8 MyFormattedText = EGSFormat8( "Hi {0}, you are a {1} the value is {2:PRETTY} big number {3:PRETTY}" , "Jack" , L"Man" , TestValue , EGTransformFormatter(Transform) );
		EGLogf( eg_log_t::General , "Formatted Text: %s" , *eg_d_string8(*MyFormattedText) );
		*/

		EGRandom Rng( CT_Default );

		// A test.
		EGItemMap<eg_int,eg_int> Test( CT_Clear );
		for( eg_int i=0; i<1000; i++ )
		{
			Test[Rng.GetRandomRangeI( 1 , 100000 )]++;
			// const eg_string_crc EncounterId = GetCurrentMapInfo().EncounterData.RandomEncouter.GetEncounter( m_Rng );
			// Test[EncounterId]++;
		}

		for( eg_int i=0; i<Test.Len(); i++ )
		{
			eg_int Key = Test.GetKeyByIndex( i );
			eg_int Count = Test.GetByIndex( i );
			EGLogf( eg_log_t::General , "Encounter[%d]=%d" , Key , Count );
		}
	}

	EGWndAppMainWnd( int nCmdShow )
	: EGWndAppWindow( EGWndApp_Title )
	, m_MainPanel( egWndPanelAppContainer( this ) )
	, m_nShowCmd( nCmdShow )
	, m_bIsDone( false )
	, m_bIsDirty( false )
	{
		SetMenu( GetWnd() , LoadMenuW( GetModuleHandleW( nullptr ) , L"EGWNDAPP_MENU_MAIN" ) );

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );
		EGWndPanel* LeftPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		LeftPane->SetFillSize(eg_panel_size_t::Fixed , EGWndPanel::GetPane3SideWidth() );

		EGWndPanel* MidPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		MidPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		EGWndTabbedPanel* RightPane = MainContainer->AddChild<EGWndTabbedPanel>();
		RightPane->SetFillSize( eg_panel_size_t::Fixed , EGWndPanel::GetPane3SideWidth() );

		// Mid Pane
		{
			MidPane->AddChild<EGWndPanelHeader>( "Main Content Header" );
			EGWndTabbedPanel* MainTabPane = MidPane->AddChild<EGWndTabbedPanel>();
			m_FileBrowser = MainTabPane->AddTab<EGWndFileBrowser>( "File Browser" , MainTabPane );
			m_PropEditor = MainTabPane->AddTab<EGWndPanelPropEditor>( "Property Test" );
			// MidPane->AddChild<EGWndTestTodoPanel>();
			MidPane->AddChild<EGWndTestHorzScroller>();
			MidPane->AddChild<EGWndPanelHeader>( "Debug Log" );
			EGWndDebugLogPanel* DebugLogPanel = MidPane->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Percent , 50 );
		}

		// Left Pane
		{
			EGWndTestSampleDragableEditor* LayoutPanel = LeftPane->AddChild<EGWndTestSampleDragableEditor>();
			LayoutPanel->RefreshPanel();

			LeftPane->AddChild<EGWndTestTodoPanel>();
		}

		// Right Pane
		{
			EGWndPanel* Tab1 = RightPane->AddTab<EGWndPanel>( "Sample Tab 1" , eg_panel_fill::Vertical );
			Tab1->AddChild<EGWndTestTextEditorPanel>();
			Tab1->AddChild<EGWndTestTodoPanel>();
			Tab1->AddChild<EGWndTestTodoPanel>();
			RightPane->AddTab<EGWndTestTodoPanel>( "Sample Tab 2" );
			RightPane->AddTab<EGWndTestTodoPanel>( "Sample Tab 3" );
		}

		if( m_PropEditor )
		{
			m_PropEditor->SetEditObject( m_Props.GetEditor() );
		}

		if( m_FileBrowser )
		{
			m_FileBrowser->SetRoot( L".\\" );
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
			UpdateHeaderText();
		}

		StartupTest();
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void LoadAppFile( eg_cpstr16 Filename )
	{
		eg_bool bStartingEmpty = false;

		if( Filename )
		{
			EGString_Copy( m_LastSaveFilename , Filename , countof(m_LastSaveFilename) );
		}
		else
		{
			m_LastSaveFilename[0] = '\0';
			bStartingEmpty = true;
		}

		if( !bStartingEmpty )
		{
			// App should load m_LastFilename;
			EGLogf( eg_log_t::General , "Opening %s..." , EGString_ToMultibyte(m_LastSaveFilename) );
		}
		else
		{
			DoNew();
		}

		m_bIsDirty = false;

		UpdateHeaderText();
	}

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

	virtual void OnWmQuit( eg_int ExitCode ) override
	{
		Super::OnWmQuit( ExitCode );
	}

	virtual void OnWmGetMinMaxInfo( MINMAXINFO* MinMaxInfo ) override
	{
		MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3SideWidth()*2 + EGWndPanel::GetPane3MidMinWidth();
		MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override
	{
		unused( bFromAccelerator );

		switch (CmdId)
		{
		case EGWNDAPP_FILE_NEW:
		{
			DoNew();
		} break;
		case EGWNDAPP_FILE_OPEN:
			DoOpen();
			break;
		case EGWNDAPP_FILE_SAVE:
			DoSave( false );
			break;
		case EGWNDAPP_FILE_SAVE_AS:
			DoSave( true );
			break;
		case EGWNDAPP_FILE_QUIT:
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

	void UpdateHeaderText()
	{
		eg_char16 WindowTitle[1024];
		EGString_FormatToBuffer( WindowTitle , countof(WindowTitle) , L"%s %s[%s]" , EGWndApp_Title , m_bIsDirty ? L"*" : L"" , m_LastSaveFilename );
		SetWindowTextW( m_hwnd , WindowTitle );
	}

	void DoSave( eg_bool bForceUseFileSelector )
	{
		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , EGWndApp_SaveFileOptions );
		}

		if( bShouldSave )
		{
			// App should save to m_LastSaveFilename here.
			EGLogf( eg_log_t::General , "Saving to %s..." , EGString_ToMultibyte(m_LastSaveFilename) );
		}

		UpdateHeaderText();
	}

	void DoOpen()
	{
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , EGWndApp_SaveFileOptions ) )
		{
			LoadAppFile( m_LastSaveFilename );
		}

		UpdateHeaderText();
	}

	void DoNew()
	{
		// App should create a new file
		EGLogf( eg_log_t::General , "Creating new..." );
	}

	void HandleQuitRequest()
	{
		if( m_bIsDirty )
		{
			int MsgRes = MessageBoxW( m_hwnd , L"You have unsaved changes, would you like to save before exiting?" , EGWndApp_Title , MB_YESNOCANCEL );
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

	void SetDirty()
	{
		m_bIsDirty = true;
		UpdateHeaderText();
	}

	void Refresh()
	{

	}
};

int APIENTRY wWinMain( _In_ HINSTANCE hInstance , _In_opt_ HINSTANCE hPrevInstance , _In_ LPWSTR lpCmdLine , _In_ int nCmdShow )
{
	EGToolMem_Init();
	EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t::OS );

	EGLogf( eg_log_t::General , "Starting %s" , EGString_ToMultibyte(EGWndApp_Title) );
	unused( hPrevInstance );
	unused( lpCmdLine );

	static const egWndAppInfo AppInfo( hInstance , hInstance , nCmdShow , false , L"EGWNDAPP_ICON" , L"EGWNDAPP_ICON" , L"EGWNDAPP_ACCEL" );

	EGCrcDb::Init();
	EGResourceLib_Init();

	// eg_size_t MemChunkSize = 512*1024*1024;
	// eg_byte* MemChunk = new eg_byte[ MemChunkSize ];
	// EGMem2_Init( MemChunk , MemChunkSize );

	EGWnd_RunApp( AppInfo , [](int nCmdShow)->EGWndAppWindow*{ return new EGWndAppMainWnd( nCmdShow ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 

	// EGMem2_Deinit();
	// delete [] MemChunk;

	EGResourceLib_Deinit();
	EGCrcDb::Deinit();

	EGToolMem_Deinit();
	return 0;
}

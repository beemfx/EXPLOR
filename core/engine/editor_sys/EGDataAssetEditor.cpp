// (c) 2017 Beem Media

#if defined( __EGEDITOR__ )

#include "EGDataAssetEditor.h"
#include "EGDataAssetBase.h"
#include "EGWndAppWindow.h"
#include "EGWndPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGWndPanelPropEditor.h"
#include "EGWndHelpers.h"
#include "EGResourceLib.h"
#include "EGEngineFS.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGEngineApp.h"
#include "EGEdLib.h"
#include "EGAssetPath.h"
#include "EGEntDict.h"
#include "EGLoader.h"
#include "../tools/EGEdResLib/resource.h"

class EGDataAssetEditorWnd;

static const eg_char16 EGDataAssetEditor_Title[] = L"EG Asset Editor";
static const eg_char16 EGDataAssetEditor_SaveFileOptions[] = L"EG Asset (*.egasset)\0*.egasset\0EG Sound Pack (*.egsnd)\0*.egsnd\0";

static const eg_uint EGDAED_FILE_OPEN    = 1001;
static const eg_uint EGDAED_FILE_SAVE    = 1002;
static const eg_uint EGDAED_FILE_SAVE_AS = 1003;
static const eg_uint EGDAED_FILE_QUIT    = 1004;

static const eg_uint EGDAED_FILE_NEW_CLASS_START = 25001;

class EGDataAssetPropEditor : public EGWndPanelPropEditor
{
	EG_DECL_SUPER( EGWndPanelPropEditor )

private:

	EGDataAssetEditorWnd*const m_App = nullptr;

public:

	EGDataAssetPropEditor( EGWndPanel* Parent , EGDataAssetEditorWnd* App ): Super( Parent ) , m_App(App) { }

	virtual void HandlePropChanged( const egRflEditor* Editor ) override;
};

class EGDataAssetEditorWnd : public EGWndAppWindow
{
private: typedef EGWndAppWindow Super;

private:

	EGDataAsset*           m_DataAsset = nullptr;
	egRflEditor            m_DataAssetEditor = CT_Clear;
	EGDataAssetPropEditor* m_PropertyEditorPanel = nullptr;
	EGWndPanel             m_MainPanel;
	HMENU                  m_MainMenu = nullptr;
	EGArray<EGClass*>      m_ClassesForNew;
	eg_string_big          m_CurrentGameFilename;
	eg_char16              m_LastSaveFilename[256];
	int                    m_nShowCmd;
	eg_bool                m_bIsDone:1;
	eg_bool                m_bIsDirty:1;

public:

	EGDataAssetEditorWnd( int nCmdShow , eg_cpstr16 InitialFile )
		: EGWndAppWindow( EGDataAssetEditor_Title )
		, m_MainPanel( egWndPanelAppContainer( this ) )
		, m_nShowCmd( nCmdShow )
		, m_bIsDone( false )
		, m_bIsDirty( false )
	{
		m_LastSaveFilename[0] = '\0';

		LoadAppFile( InitialFile );
		
		CreateMainMenu();

		EGWndPanel* MainContainer = m_MainPanel.AddContainer( eg_panel_fill::Horizontal );

		EGWndPanel* MidPane = MainContainer->AddContainer( eg_panel_fill::Vertical );
		MidPane->SetFillSize( eg_panel_size_t::Auto , 0 );

		// Mid Pane
		{
			MidPane->AddChild<EGWndPanelHeader>( "Property Editor" );
			m_PropertyEditorPanel = MidPane->AddChild<EGDataAssetPropEditor>( this );
			MidPane->AddChild<EGWndPanelHeader>( "Debug Log" );
			EGWndDebugLogPanel* DebugLogPanel = MidPane->AddChild<EGWndDebugLogPanel>();
			DebugLogPanel->SetFillSize( eg_panel_size_t::Percent , 25 );
		}

		SetAppWidth( 500 );

		m_MainPanel.HandleResize();
	}

	~EGDataAssetEditorWnd()
	{
		if( m_PropertyEditorPanel )
		{
			m_PropertyEditorPanel->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
		}

		if( m_DataAsset )
		{
			EGDeleteObject( m_DataAsset );
			m_DataAsset = nullptr;
			m_DataAssetEditor = CT_Clear;
		}

		DestroyMainMenu();
	}

	void CreateMainMenu()
	{
		EGWndMenuItemDef MainMenuDef;
		MainMenuDef.Children.ExtendSize( 1 );
		MainMenuDef.Children.Last().Label = "&File";
		MainMenuDef.Children.Last().AddCmd( "&New" , 0 );
		MainMenuDef.Children.Last().AddCmd( "&Open" , EGDAED_FILE_OPEN );
		MainMenuDef.Children.Last().AddCmd( "&Save" , EGDAED_FILE_SAVE );
		MainMenuDef.Children.Last().AddCmd( "Save &as..." , EGDAED_FILE_SAVE_AS );
		MainMenuDef.Children.Last().AddSeparator();
		MainMenuDef.Children.Last().AddCmd( "E&xit\tAlt+F4" , EGDAED_FILE_QUIT );

		EGWndMenuItemDef& NewSubmenu = MainMenuDef.Children[0].Children[0];

		EGClass::FindAllClassesOfType( &EGDataAsset::GetStaticClass() , m_ClassesForNew );

		m_ClassesForNew.DeleteAllByPredicate(
			[]( const EGClass* Class ) -> eg_bool
			{
				return Class == &EGDataAsset::GetStaticClass();
			} );

		m_ClassesForNew.Sort( []( EGClass* Left , EGClass* Right ) -> eg_bool { return EGString_CompareI( Left->GetName() , Right->GetName() ) < 0; } );

		for( eg_uint i=0; i<m_ClassesForNew.LenAs<eg_uint>(); i++ )
		{
			NewSubmenu.AddCmd( m_ClassesForNew[i]->GetName() , i + EGDAED_FILE_NEW_CLASS_START );
		}

		m_MainMenu = EGWnd_CreateMenu( MainMenuDef , false );

		SetMenu( GetWnd() , m_MainMenu );
	}

	void DestroyMainMenu()
	{
		SetMenu( GetWnd() , nullptr );
		DestroyMenu( m_MainMenu );
	}

	virtual void OnAppStart( const EGArray<eg_cpstr16>& Args ) override final
	{
		Super::OnAppStart( Args );

		if( !m_bIsDone )
		{
			ShowWindow( GetWnd() , m_nShowCmd );
			UpdateWindow( GetWnd() );
			UpdateHeaderText();
			RefreshPropertyEditor();
		}
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void LoadAppFile( eg_cpstr16 Filename )
	{
		eg_string_big GamePath = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte(Filename) );
		eg_asset_path::SetCurrentFile( GamePath );

		if( m_PropertyEditorPanel )
		{
			m_PropertyEditorPanel->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
		}

		if( m_DataAsset )
		{
			EGDeleteObject( m_DataAsset );
			m_DataAsset = nullptr;
			m_DataAssetEditor = CT_Clear;
		}

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
			EGLogf( eg_log_t::General , "Opening \"%s\" for editing..." , EGString_ToMultibyte(Filename) );
			m_DataAssetEditor = CT_Clear;
			m_DataAsset = EGDataAsset::LoadDataAsset( m_LastSaveFilename , eg_mem_pool::Default , true , m_DataAssetEditor );

			m_bIsDirty = false;
			RefreshPropertyEditor();
			UpdateHeaderText();
		}
		
		if( nullptr == m_DataAsset )
		{
			DoNew( &EGDataAsset::GetStaticClass() );
		}
	}

	void SetDirty()
	{
		m_bIsDirty = true;
		UpdateHeaderText();
	}

	void HandlePropChanged( const egRflEditor* Editor )
	{
		SetDirty();

		if( m_DataAsset )
		{
			if( Editor )
			{
				eg_bool bNeedsRebuild = false;
				const_cast<egRflEditor*>(Editor)->ExecutePostLoad( GetCurrentGameFilename() , true );
				m_DataAsset->OnPropChanged( *Editor , m_DataAssetEditor , bNeedsRebuild );
				if( bNeedsRebuild )
				{
					if( m_PropertyEditorPanel )
					{
						m_PropertyEditorPanel->PropEditorRebuild();
					}
				}
			}
		}
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
		MinMaxInfo->ptMinTrackSize.x = EGWndPanel::GetPane3MidMinWidth();
		MinMaxInfo->ptMinTrackSize.y = EGWndPanel::GetMainWndMinHeight();
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override
	{
		unused( bFromAccelerator );

		switch (CmdId)
		{
		case EGDAED_FILE_OPEN:
			DoOpen();
			break;
		case EGDAED_FILE_SAVE:
			DoSave( false );
			break;
		case EGDAED_FILE_SAVE_AS:
			DoSave( true );
			break;
		case EGDAED_FILE_QUIT:
			HandleQuitRequest();
			break;
		default:
			if( EGDAED_FILE_NEW_CLASS_START <= EG_To<eg_uint>(CmdId) && EG_To<eg_uint>(CmdId) < (EGDAED_FILE_NEW_CLASS_START + m_ClassesForNew.LenAs<eg_uint>()) )
			{
				DoNew( m_ClassesForNew[CmdId - EGDAED_FILE_NEW_CLASS_START] );
			}
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
		eg_string_small ClassName = m_DataAsset ? m_DataAsset->GetObjectClass()->GetName() : "";
		eg_char16 WindowTitle[1024];
		eg_string_big GameName = EGToolsHelper_GetEnvVar( "EGGAME" );
		GameName.ConvertToUpper();
		m_CurrentGameFilename = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte( m_LastSaveFilename ) );
		EGString_FormatToBuffer( WindowTitle , countof(WindowTitle) , L"%s %s %s[%s]" , EGDataAssetEditor_Title , EGString_ToWide(GameName) , m_bIsDirty ? L"*" : L"" , EGString_ToWide(m_CurrentGameFilename) );
		SetWindowTextW( m_hwnd , WindowTitle );
	}

	void DoSave( eg_bool bForceUseFileSelector )
	{
		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , EGDataAssetEditor_SaveFileOptions , L".egasset" );
		}

		if( bShouldSave )
		{
			EGLogf( eg_log_t::General , "Saving to %s..." , EGString_ToMultibyte(m_LastSaveFilename) );
			if( EGDataAsset::SaveDataAsset( m_LastSaveFilename , m_DataAsset , m_DataAssetEditor ) )
			{
				m_bIsDirty = false;
			}
			else
			{
				MessageBoxW( GetWnd() , L"Could not save, make sure this asset is checked out of source control." , EGDataAssetEditor_Title , MB_OK );
			}
		}

		UpdateHeaderText();
	}

	void DoOpen()
	{
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , EGDataAssetEditor_SaveFileOptions ) )
		{
			LoadAppFile( m_LastSaveFilename );
		}

		UpdateHeaderText();
	}

	void DoNew( EGClass* Class )
	{
		if( Class )
		{
			EGLogf( eg_log_t::General , "Creating new %s..." , Class->GetName() );

			m_LastSaveFilename[0] = '\0';

			if( m_PropertyEditorPanel )
			{
				m_PropertyEditorPanel->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
			}

			if( m_DataAsset )
			{
				EGDeleteObject( m_DataAsset );
				m_DataAsset = nullptr;
				m_DataAssetEditor = CT_Clear;
			}
			
			m_DataAsset = EGDataAsset::CreateDataAsset( Class , eg_mem_pool::Default , m_DataAssetEditor );	

			m_bIsDirty = false;
			RefreshPropertyEditor();
			UpdateHeaderText();
		}
	}

	void HandleQuitRequest()
	{
		if( m_bIsDirty )
		{
			int MsgRes = MessageBoxW( m_hwnd , L"You have unsaved changes, would you like to save before exiting?" , EGDataAssetEditor_Title , MB_YESNOCANCEL );
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

	void RefreshPropertyEditor()
	{
		if( m_PropertyEditorPanel )
		{
			m_PropertyEditorPanel->SetEditObject( &m_DataAssetEditor );
		}
	}

	eg_string_big GetCurrentGameFilename() const { return m_CurrentGameFilename; }
};

void EGDataAssetPropEditor::HandlePropChanged( const egRflEditor* Editor )
{
	Super::HandlePropChanged( Editor );

	m_App->HandlePropChanged( Editor );
}

int EGDataAssetEditor_Run( eg_cpstr16 Filename , const struct egSdkEngineInitParms& EngineParms )
{
	unused( EngineParms );

	eg_d_string16 RootDir = *EGPath2_CleanPath( EGString_Format( "%s/bin" , EGToolsHelper_GetEnvVar("EGOUT").String() ) , '/' );
	EGEngineFS_Init( egEngineFSInitParms( *RootDir , EngineParms.GameName , L"" , L"" )  );
	MainLoader->SetIsLoadOnOtherThreadOkay( true );
	EntDict_Init();
	MainLoader->SetIsLoadOnOtherThreadOkay( false );

	static const egWndAppInfo AppInfo( GetModuleHandleW( NULL ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , false , MAKEINTRESOURCEW(IDI_EGASSET) , MAKEINTRESOURCEW(IDI_EGASSET) , nullptr );
	EGWnd_RunApp( AppInfo , [&Filename](int nCmdShow)->EGWndAppWindow*{ return new EGDataAssetEditorWnd( nCmdShow , Filename ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } ); 

	EntDict_Deinit();
	EGEngineFS_Deinit();

	return 0;
}

#endif

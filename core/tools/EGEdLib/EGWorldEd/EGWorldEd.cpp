// (C) 2018 Beem Media

#include "EGWorldEd.h"
#include "EGWorldFile.h"
#include "EGEngineApp.h"
#include "EGWndAppWindow.h"
#include "../EGEdResLib/resource.h"
#include "EGWnd.h"
#include "EGWndHelpers.h"
#include "EGRenderer.h"
#include "EGWndPanel.h"
#include "EGEngine.h"
#include "EGWorldFile.h"
#include "EGEngineForTools.h"
#include "EGTimer.h"
#include "EGDebugText.h"
#include "EGEGWinCon.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGFileData.h"
#include "EGEngineMem.h"
#include "EGCrcDb.h"
#include "EGWndTabbedPanel.h"
#include "EGWndDebugLogPanel.h"
#include "EGResourceLib.h"
#include "EGEdUtility.h"
#include "EGEdLib.h"
#include "EGWndPanelPropEditor.h"
#include "EGLibFile.h"
#include "EGAssetPath.h"
#include "EGWorldEdPreviewPanel.h"
#include "EGCamera2.h"
#include "EGWorldEdObjLibPanel.h"
#include "EGWorldEdWorldObjsPanel.h"
#include "EGWorldObject.h"
#include "EGWorldEdPropEdPanel.h"
#include "EGEntWorld.h"
#include "EGWorldSceneGraph.h"
#include "EGWorldRenderPipe.h"
#include "EGWorldObjectEntity.h"

class EGWorldEdMainWnd : public EGWndAppWindow
{
	EG_DECL_SUPER( EGWndAppWindow )

public:

	static eg_cpstr16 AppName;
	static EGWorldEdMainWnd* GlobalInst;

private:

	EGWorldFile*              m_WorldFile = nullptr;
	EGEntWorld*               m_PreviewWorld = nullptr;
	EGWorldRenderPipe*        m_WorldRenderPipe = nullptr;
	EGWndPanel                m_MainPanel;
	EGWorldEdPropEdPanel*     m_BaseObjectPropertiesPanel = nullptr;
	EGWorldEdPropEdPanel*     m_EntityPropertiesPanel = nullptr;
	EGWorldEdPropEdPanel*     m_GlobalSettingsPanel = nullptr;
	EGWorldEdWorldObjsPanel*  m_WorldObjsPanel = nullptr;
	EGWorldEdObjLibPanel*     m_ObjLibPanel = nullptr;
	EGWorldEdPreviewPanel*    m_PreviewPanel = nullptr;
	eg_string_big             m_CurrentGameFilename;
	eg_char16                 m_LastSaveFilename[256];
	eg_transform              m_CameraPose = CT_Default;
	eg_bool                   m_bIsDirty:1;
	eg_bool                   m_bIsDone:1;
	eg_bool                   m_bResetEntityProperties = false;

public:

	EGWorldEdMainWnd( int nCmdShow , eg_cpstr16 InitialFilename )
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
			m_PreviewPanel = PreviewBg->AddChild<EGWorldEdPreviewPanel>();

			PreviewContainer->AddChild<EGWndPanelHeader>( "Log" );
			EGWndDebugLogPanel* DebugLog = PreviewContainer->AddChild<EGWndDebugLogPanel>();
			DebugLog->SetFillSize( eg_panel_size_t::Percent , 25 );
		}

		// Left Pane:
		{
			EGWndPanel* ComponentsContainer = CompLib->AddContainer( eg_panel_fill::Vertical );
			ComponentsContainer->AddChild<EGWndPanelHeader>( "World Objects" );
			m_WorldObjsPanel = ComponentsContainer->AddChild<EGWorldEdWorldObjsPanel>();

			EGWndPanel* LibContainer = CompLib->AddContainer( eg_panel_fill::Vertical );
			LibContainer->SetFillSize( eg_panel_size_t::Percent , 35 );
			LibContainer->AddChild<EGWndPanelHeader>( "Objects Library" );
			m_ObjLibPanel = LibContainer->AddChild<EGWorldEdObjLibPanel>();
		}

		// Right Pane:
		{
			m_BaseObjectPropertiesPanel = RightPane->AddTab<EGWorldEdPropEdPanel>( "Object Properties" );
			// m_EntityPropertiesPanel = RightPane->AddTab<EGWorldEdPropEdPanel>( "Entity Properties" );
			m_GlobalSettingsPanel = RightPane->AddTab<EGWorldEdPropEdPanel>( "World Settings" );
		}

		m_MainPanel.HandleResize();

		SetForegroundWindow( m_hwnd );

		UpdateHeaderText();

		SetMenu( GetWnd() , LoadMenuW( EGResourceLib_GetLibrary() , L"EGWORLDED_MAIN_MENU" ) );

		ShowWindow(m_hwnd, nCmdShow);
		UpdateWindow(m_hwnd);

		m_WorldFile = EGNewObject<EGWorldFile>();

		if( nullptr == m_WorldFile )
		{
			MessageBoxW( GetWnd() , L"Could not create the world file." , AppName , MB_OK|MB_ICONERROR );
			m_bIsDone = true;
			return;
		}

		m_PreviewWorld = EGNewObject<EGEntWorld>();

		if( nullptr == m_PreviewWorld )
		{
			MessageBoxW( GetWnd() , L"Could not create the preview world." , AppName , MB_OK|MB_ICONERROR );
			m_bIsDone = true;
			return;
		}

		m_PreviewWorld->InitWorld( eg_ent_world_role::EditorPreview , nullptr , nullptr , nullptr );

		EGClass* WorldRenderPipeClass = Engine_GetRenderPipeClass();
		m_WorldRenderPipe = EGNewObject<EGWorldRenderPipe>( WorldRenderPipeClass );

		SetWorldObjectToEdit( nullptr );
		m_BaseObjectPropertiesPanel->FullRedraw();
		if( m_EntityPropertiesPanel )
		{
			m_EntityPropertiesPanel->FullRedraw();
		}

		m_GlobalSettingsPanel->SetEditObject( m_WorldFile->GetGlobalsEditor() );

		if( InitialFilename )
		{
			LoadWorldFile( InitialFilename );
		}
	}

	~EGWorldEdMainWnd()
	{
		if( m_PreviewWorld )
		{
			EGDeleteObject( m_PreviewWorld );
			m_PreviewWorld = nullptr;
		}

		if( m_WorldFile )
		{
			EGDeleteObject( m_WorldFile );
			m_WorldFile = nullptr;
		}

		if( m_WorldRenderPipe )
		{
			EGDeleteObject( m_WorldRenderPipe );
			m_WorldRenderPipe = nullptr;
		}

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
	}

	virtual void OnAppUpdate( eg_real DeltaTime ) override final
	{
		EngineForTools_Update( DeltaTime );

		if( m_bResetEntityProperties )
		{
			m_bResetEntityProperties = false;

			if( m_EntityPropertiesPanel )
			{
				EGWorldObject* WorldObject = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
				if( WorldObject )
				{
					WorldObject->ResetPropertiesEditor();
					m_EntityPropertiesPanel->SetEditObject( nullptr );
					m_EntityPropertiesPanel->SetEditObject( WorldObject->GetPropertiesEditor() );
				}
			}
		}

		if( m_PreviewPanel )
		{
			m_PreviewPanel->HandleAppUpdate( DeltaTime );
		}
		if( m_PreviewWorld )
		{
			m_PreviewWorld->SetActivityPos( m_CameraPose.GetPosition() );
			m_PreviewWorld->Update( DeltaTime );
			EGLogToScreen::Get().Log( this , 1 , 1.f , EGString_Format( "Camera: %g %g %g" , m_CameraPose.GetTranslation().x , m_CameraPose.GetTranslation().y , m_CameraPose.GetTranslation().z ) );
		}
		if( m_WorldRenderPipe )
		{
			m_WorldRenderPipe->Update( DeltaTime );
		}
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "Tool FPS: %u", eg_uint( 1.f / DeltaTime ) ) );
		DrawPreview();
	}

	virtual eg_bool IsDone() const override final { return m_bIsDone; }

	void DoSave( eg_bool bForceUseFileSelector )
	{
		eg_bool bShouldSave = true;

		if( bForceUseFileSelector || m_LastSaveFilename[0] == '\0' )
		{
			bShouldSave = EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , true , L"EG World (*.egworld)\0*.egworld\0" );
		}

		if( bShouldSave )
		{
			eg_bool bFileOkay = true;

			if( bFileOkay )
			{
				if( !EGString_Contains( m_LastSaveFilename , L"." ) )
				{
					EGString_StrCat( m_LastSaveFilename , countof(m_LastSaveFilename) , L".egworld" );
				}
				EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
				m_WorldFile->Save( FileData );
				eg_bool bSaved = EGEdUtility_SaveFile( m_LastSaveFilename , FileData );
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
		if( EGWndHelper_GetFile( m_hwnd , m_LastSaveFilename , countof(m_LastSaveFilename) , false , L"EG World (*.egworld)\0*.egworld\0" ) )
		{
			LoadWorldFile( m_LastSaveFilename );
		}
	}

	void LoadWorldFile( eg_cpstr16 Filename )
	{
		if( m_WorldFile && CanOpenInCurrentGame( EGString_ToMultibyte(Filename) ) )
		{
			EGString_Copy( m_LastSaveFilename , Filename , countof(m_LastSaveFilename) );

			eg_string_big GamePath = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte(Filename) );
			eg_asset_path::SetCurrentFile( GamePath );

			m_GlobalSettingsPanel->SetEditObject( nullptr );

			EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
			EGLibFile_OpenFile( m_LastSaveFilename , eg_lib_file_t::OS , FileData );
			m_WorldFile->Load( FileData , GamePath , true );

			m_GlobalSettingsPanel->SetEditObject( m_WorldFile->GetGlobalsEditor() );
			m_WorldObjsPanel->RefreshPanel( -1 );
			m_PreviewPanel->SetCameraPose( m_WorldFile->GetMutableEditorConfig().CameraPose );

			m_bIsDirty = false;
			UpdateHeaderText();

			FullPreviewRebuild();
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
				// EGDefEdPreview::Get().ResetCamera();
				m_WorldFile->Reset();
				m_GlobalSettingsPanel->SetEditObject( nullptr );
				m_GlobalSettingsPanel->SetEditObject( m_WorldFile->GetGlobalsEditor() );
				m_WorldObjsPanel->RefreshPanel( -1 );
				m_LastSaveFilename[0] = '\0';
				m_bIsDirty = false;
				UpdateHeaderText();

				FullPreviewRebuild();
			}
		} break;
		case ID_FILE_QUIT:
			HandleQuitRequest();
			break;
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
		case ID_VIEW_REBUILDPREVIEW:
		{
			FullPreviewRebuild();
		} break;
		case ID_OBJECT_MOVEINFRONTOFCAMERA:
		{
			egWorldFileEditorSettings EdConfig = m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings();
			if( EdConfig.bMoveInFrontOfCameraTargetsGrid )
			{
				SnapSelectedToGridLookAt();
			}
			else
			{
				MoveSelectedInFrontOfCamera();
			}
		} break;
		case ID_OBJECT_MOVETOCAMERAPOS:
		{
			MoveSelectedToCameraPose();
		} break;
		case ID_OBJECT_MOVECAMERATOOBJECT:
		{
			MoveCameraToSelectedObject();
		} break;
		case ID_OBJECT_SNAPTOGRID:
		{
			SnapSelectedToGrid();
		} break;
		case ID_OBJECT_SPAWNATCAMERA:
		{
			DoSpawnAtCamera();
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
				m_WorldObjsPanel->OnToggleLocked();
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
		egPathParts2 PathParts = EGPath2_BreakPath( RelativeTo );
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

	void DrawPreview()
	{
		EGRenderer::Get().BeginFrame();

		if( m_PreviewPanel )
		{
			EGCamera2 Camera( CT_Default );
			Camera.SetAspectRatio( m_PreviewPanel->GetAspectRatio() );
			Camera.SetPose( m_CameraPose );

			MainDisplayList->SetViewTF( Camera.GetViewMat() );
			MainDisplayList->SetProjTF( Camera.GetProjMat() );
			MainDisplayList->SetVec4( eg_rv4_t::CAMERA_POS , Camera.GetPose().GetPosition() );

			MainDisplayList->ClearRT( eg_color(m_WorldFile ? m_WorldFile->GetEditorFileSettings().BackgroundColor : egWorldFileEditorSettings().BackgroundColor) );
			MainDisplayList->ClearDS( 1.f , 0 );

			if( m_PreviewWorld && m_WorldRenderPipe )
			{
				m_PreviewWorld->UpdateScenGraph( Camera );
				if( true )
				{
					egWorldRenderPipeDrawSpecs DrawSpecs;
					DrawSpecs.bIsSplitscreen = false;
					m_WorldRenderPipe->DrawSceneGraph( m_PreviewWorld->GetScenGraph() , Camera , DrawSpecs );
				}
				else
				{
					m_PreviewWorld->GetScenGraph()->DrawTerrains();
					MainDisplayList->PushDefaultShader( eg_defaultshader_t::MAP );
					MainDisplayList->DisableAllLights(); // Game callback may have set lights, we don't want them anymore.
					m_PreviewWorld->GetScenGraph()->DrawMapRegions();
					MainDisplayList->PopDefaultShader();
					MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_LIT );
					m_PreviewWorld->GetScenGraph()->DrawComponents( CT_Clear );
					MainDisplayList->PopDefaultShader();
					MainDisplayList->DisableAllLights();
					MainDisplayList->SetMaterial(EGV_MATERIAL_NULL);
					m_PreviewWorld->GetScenGraph()->DrawShadows( 100.f );
				}
			}

			DrawGrid( m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings() );
			// Draw_ApplyEditorConfig( EGDefEdFile::Get().GetBounds().GetCenter() );

			// EGDefEdFile::Get().Draw();

			EGLogToScreen::Get().Draw( Camera.GetAspectRatio() );
		}

		EGRenderer::Get().EndFrame();


		EngineForTools_Draw();
	}

	void DrawGrid( const egWorldFileEditorSettings& EditorConfig )
	{
		if( EditorConfig.bDrawGrid )
		{
			const eg_real GridSpacing = EditorConfig.GridSpacing;
			const eg_vec3 Starting = EditorConfig.GridOffset;
			const eg_color LineColor = eg_color(EditorConfig.GridColor);

			for( eg_int i=0; i<=EditorConfig.GridSize.x; i++ )
			{	
				MainDisplayList->DrawLine( eg_vec4(Starting.x + i*GridSpacing , Starting.y , Starting.z , 1.f ) , eg_vec4( Starting.x + i*GridSpacing , Starting.y , Starting.z + EditorConfig.GridSize.y*GridSpacing , 1.f ) , LineColor );
			}

			for( eg_int i=0; i<=EditorConfig.GridSize.y; i++ )
			{	
				MainDisplayList->DrawLine( eg_vec4( Starting.x , Starting.y , Starting.z + i*GridSpacing , 1.f ) , eg_vec4( Starting.x + EditorConfig.GridSize.x*GridSpacing , Starting.y , Starting.z + i*GridSpacing , 1.f ) , LineColor );
			}
		}
	}

	EGWndPanelPropEditor* GetGlobalSettingsPanel(){ return m_GlobalSettingsPanel; }
	EGWorldEdObjLibPanel* GetObjLibPanel() { return m_ObjLibPanel; }
	EGWorldEdWorldObjsPanel* GetWorldObjsPanel() { return m_WorldObjsPanel; }
	EGWorldEdPreviewPanel* GetPreviewPanel() { return m_PreviewPanel; }

	EGWorldFile* GetWorldFile() { return m_WorldFile; }
	eg_string_big GetCurrentGameFilename() const { return m_CurrentGameFilename; }

	void InsertNewWorldObject( EGClass* ObjectClass )
	{
		EGWorldObject* NewWorldObject = m_WorldFile->InsertNewWorldObject( ObjectClass );
		m_WorldObjsPanel->RefreshPanel( EG_To<eg_int>(m_WorldObjsPanel->GetNumItems()) );
		EGWorldEd_SetDirty();

		if( NewWorldObject )
		{
			NewWorldObject->AddToWorldPreview( m_PreviewWorld );
			m_WorldObjsPanel->SetSelectedWorldObject( NewWorldObject );
			SnapSelectedToGridLookAt();
		}
	}

	void DeleteWorldObject( EGWorldObject* Object )
	{
		if( Object )
		{
			m_WorldObjsPanel->SetSelectedIndex( -1 );
			m_BaseObjectPropertiesPanel->SetEditObject( nullptr );
			if( m_EntityPropertiesPanel )
			{
				m_EntityPropertiesPanel->SetEditObject( nullptr );
			}
			Object->RemoveFromWorldPreview( m_PreviewWorld );
			m_WorldFile->DeleteWorldObject( Object );
			m_WorldObjsPanel->RefreshPanel( -1 );
			EGWorldEd_SetDirty();
		}
	}

	void MoveObject( EGWorldObject* ObjToMove, EGWorldObject* ObjToMoveBefore, EGWorldObject* ObjParent )
	{
		m_WorldObjsPanel->SetSelectedIndex( -1 );
		m_BaseObjectPropertiesPanel->SetEditObject( nullptr );
		if( m_EntityPropertiesPanel )
		{
			m_EntityPropertiesPanel->SetEditObject( nullptr );
		}
		m_WorldFile->MoveWorldObject( ObjToMove , ObjToMoveBefore , ObjParent );
		m_WorldObjsPanel->RefreshPanel( -1 );
		m_WorldObjsPanel->SetSelectedWorldObject( ObjToMove );
		EGWorldEd_SetDirty();
	}

	void OnPropChanged( egRflEditor& BaseEditor , const egRflEditor& ChangedProperty )
	{
		const_cast<egRflEditor&>(ChangedProperty).ExecutePostLoad( GetCurrentGameFilename() , true );
		
		if( EGString_Equals( ChangedProperty.GetVarName() , "m_Id" ) )
		{
			m_WorldObjsPanel->RefreshPanel( m_WorldObjsPanel->GetSelectedIndex() );
		}

		if( EGString_Equals( BaseEditor.GetVarName() , "WorldObject" ) )
		{
			EGWorldObject* WorldObject = reinterpret_cast<EGWorldObject*>(BaseEditor.GetData());
			if( WorldObject )
			{
				WorldObject->RefreshInWorldPreview( m_PreviewWorld , ChangedProperty );
				if( EGString_Equals( ChangedProperty.GetVarName() , "m_EntityDefinition" ) )
				{
					m_bResetEntityProperties = true;
				}
			}
		}

		EGWorldEd_SetDirty();
	}

	void SetWorldObjectToEdit( EGWorldObject* ObjectToEdit )
	{
		if( ObjectToEdit )
		{
			m_BaseObjectPropertiesPanel->SetNoPropsMessage( EGWndPanelPropEditor::GetNoPropsMessage( false , ObjectToEdit->IsLockedInTool() ) );
			m_BaseObjectPropertiesPanel->SetEditObject( ObjectToEdit->IsLockedInTool() ? nullptr : ObjectToEdit->GetEditor() );

			if( m_EntityPropertiesPanel )
			{
				m_EntityPropertiesPanel->SetNoPropsMessage( EGWndPanelPropEditor::GetNoPropsMessage( false , ObjectToEdit->IsLockedInTool() ) );
				m_EntityPropertiesPanel->SetEditObject( ObjectToEdit->IsLockedInTool() ? nullptr : ObjectToEdit->GetPropertiesEditor() );
			}
		}
		else
		{
			m_BaseObjectPropertiesPanel->SetNoPropsMessage( EGWndPanelPropEditor::GetNoPropsMessage( true , false ) );
			m_BaseObjectPropertiesPanel->SetEditObject( nullptr );

			if( m_EntityPropertiesPanel )
			{
				m_EntityPropertiesPanel->SetNoPropsMessage( EGWndPanelPropEditor::GetNoPropsMessage( true , false ) );
				m_EntityPropertiesPanel->SetEditObject( nullptr );
			}
		}
	}

	void FullPreviewRebuild()
	{
		if( m_PreviewWorld )
		{
			m_PreviewWorld->ClearWorld();
			m_PreviewWorld->ResetGame();
			m_WorldFile->ClearAllFromPreview( m_PreviewWorld );
			m_WorldFile->ApplyToWorldPreview( m_PreviewWorld );
		}
	}

	void MoveSelectedInFrontOfCamera()
	{
		EGWorldObject* WorldObj = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
		if( WorldObj )
		{
			SetDirty();
			egWorldFileEditorSettings EdConfig = m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings();
			const eg_transform CurPose = WorldObj->GetWorldPose();
			eg_transform AdjPose = CurPose;
			eg_vec3 Offset(0.f,0.f,EdConfig.GridSpacing);
			eg_transform CameraAdjust = eg_transform::BuildTranslation(Offset)*m_CameraPose;
			AdjPose.SetTranslation( CameraAdjust.GetTranslation() );

			WorldObj->SetWorldPose( AdjPose );
			m_BaseObjectPropertiesPanel->RefreshAll();
			WorldObj->RefreshInWorldPreview( m_PreviewWorld , egRflEditor("m_Pose",nullptr,nullptr) ); // Don't need to give it the actual reflection data, the name will be fine.
		}
	}

	void MoveSelectedToCameraPose()
	{
		EGWorldObject* WorldObj = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
		if( WorldObj )
		{
			SetDirty();
			WorldObj->SetWorldPose( m_CameraPose );
			m_BaseObjectPropertiesPanel->RefreshAll();
			WorldObj->RefreshInWorldPreview( m_PreviewWorld , egRflEditor("m_Pose",nullptr,nullptr) ); // Don't need to give it the actual reflection data, the name will be fine.
		}
	}

	void SnapSelectedToGridLookAt()
	{
		EGWorldObject* WorldObj = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
		if( WorldObj )
		{
			SetDirty();
			egWorldFileEditorSettings EdConfig = m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings();

			// Standard plane intersect we can simplify the math because we know the grid plane is the XZ plane.

			const eg_real d = EdConfig.GridOffset.y;
			const eg_vec4 CameraPos = m_CameraPose.GetPosition();
			const eg_vec4 CameraLookVector = eg_vec4(0.f,0.f,1.f,0.f)*m_CameraPose;

			eg_transform NewPose = WorldObj->GetWorldPose();

			// EGLogf( eg_log_t::General , *EGSFormat8( "Camera Pos: {0} Look at: {1}" , EGVec4Formatter(CameraPos) , EGVec4Formatter(CameraLookVector) ) );
			const eg_real Den = CameraLookVector.y;
			if( EG_Abs(Den) > EG_SMALL_NUMBER )
			{
				// TODO: If we are looking really far away limit where it goes.
				
				const eg_real t = (EdConfig.GridOffset.y - CameraPos.y)/Den;
				// EGLogf( eg_log_t::General , *EGSFormat8( "Intersect Time: {0}" , t ) );
				if( t > 0.f )
				{
					// EGLogf( eg_log_t::General , "Looking at plane" );
					const eg_vec4 HitPoint = CameraPos + CameraLookVector*t;
					//EGLogf( eg_log_t::General , *EGSFormat8( "Hit: {0}" , EGVec4Formatter(HitPoint) ) );
					NewPose.SetTranslation( HitPoint.ToVec3() );
				}
				else
				{
					// EGLogf( eg_log_t::General , "Looking away from plane" );
					MoveSelectedInFrontOfCamera();
					return;
				}
			}
			else
			{
				// EGLogf( eg_log_t::General , "Look is parallel to plane" );
				MoveSelectedInFrontOfCamera();
				return;
			}

			WorldObj->SetWorldPose( NewPose );
			SnapSelectedToGrid();
		}
	}

	void SnapSelectedToGrid()
	{
		auto GetClosestAxis = []( const eg_transform& InPose ) -> eg_quat
		{
			eg_quat Out = CT_Default;

			const eg_vec4 Face = eg_vec4(0.f,0.f,1.f,0.f) * InPose;
			if( EG_Abs(Face.z) >= EG_Abs(Face.x) )
			{
				const eg_real Angle = Face.z >= 0.f ? 0.f : 180.f;
				Out = eg_quat::BuildRotationY( EG_Deg(Angle) );
			}
			else
			{
				const eg_real Angle = Face.x >= 0.f ? 90.f : -90.f;
				Out = eg_quat::BuildRotationY( EG_Deg(Angle) );
			}

			Out.NormalizeThis();
			return Out;
		};
		
		EGWorldObject* WorldObj = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
		if( WorldObj )
		{
			SetDirty();
			const eg_transform CurPose = WorldObj->GetWorldPose();
			eg_transform AdjPose = CurPose;

			egWorldFileEditorSettings EdConfig = m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings();
			
			if( EdConfig.bSnapToGridAlsoAlignsToAxis )
			{
				AdjPose.SetRotation( GetClosestAxis( CurPose ) );
			}
			eg_vec3 NewTrans = AdjPose.GetTranslation();
			if( EdConfig.GridSpacing > EG_SMALL_NUMBER )
			{		
				NewTrans.x = EGMath_round((CurPose.GetTranslation().x-EdConfig.GridOffset.x)/EdConfig.GridSpacing)*EdConfig.GridSpacing + EdConfig.GridOffset.x;
				NewTrans.z = EGMath_round((CurPose.GetTranslation().z-EdConfig.GridOffset.z)/EdConfig.GridSpacing)*EdConfig.GridSpacing + EdConfig.GridOffset.z;
			}
			NewTrans.y = EdConfig.GridOffset.y;
			AdjPose.SetTranslation( NewTrans );


			WorldObj->SetWorldPose( AdjPose );
			m_BaseObjectPropertiesPanel->RefreshAll();
			WorldObj->RefreshInWorldPreview( m_PreviewWorld , egRflEditor("m_Pose",nullptr,nullptr) ); // Don't need to give it the actual reflection data, the name will be fine.
		}
	}

	void DoSpawnAtCamera()
	{
		/*
		EGWorldObject* NewWorldObject = m_WorldFile->InsertNewWorldObject( ObjectClass );
		m_WorldObjsPanel->RefreshPanel( EG_To<eg_int>(m_WorldObjsPanel->GetNumItems()) );
		EGWorldEd_SetDirty();

		if( NewWorldObject )
		{
			NewWorldObject->AddToWorldPreview( m_PreviewWorld );
			m_WorldObjsPanel->SetSelectedWorldObject( NewWorldObject );
			SnapSelectedToGridLookAt();
		}
		*/

		// MessageBoxW( m_hwnd , L"No object specified." , AppName , MB_OK );

		EGClass* ObjectClass = &EGWorldObjectEntity::GetStaticClass();
		eg_d_string EntityId = CT_Clear;
		eg_d_string WorldObjectId = "UnformattedId";
		eg_d_string InitString = "InitString";
		eg_int SpawnIndex = 0;
		eg_world_file_spawn_ent_t SpawnMode = eg_world_file_spawn_ent_t::AtCamera;

		if( m_WorldFile )
		{
			egWorldFileEditorSettings& EditorSettings = m_WorldFile->GetEditorFileSettings();

			EntityId = EditorSettings.SpawnAtCamera.Ent.Path;
			SpawnMode = EditorSettings.SpawnAtCamera.SpawnMode;
			SpawnIndex = EditorSettings.SpawnAtCamera.NextIndex;
			WorldObjectId = EGSFormat8( *EditorSettings.SpawnAtCamera.LabelStringFmt , EditorSettings.SpawnAtCamera );
			InitString = EGSFormat8( *EditorSettings.SpawnAtCamera.InitStringFmt , EditorSettings.SpawnAtCamera );

			EditorSettings.SpawnAtCamera.NextIndex++;
			if( m_GlobalSettingsPanel )
			{
				m_GlobalSettingsPanel->SetEditObject( nullptr );
				m_GlobalSettingsPanel->SetEditObject( m_WorldFile->GetGlobalsEditor() );
			}
		}

		if( ObjectClass )
		{
			if( EGWorldObject* NewWorldObject = m_WorldFile->InsertNewWorldObject( ObjectClass ) )
			{
				NewWorldObject->SetId( *WorldObjectId );

				if( EGWorldObjectEntity* NewEntityObject = EGCast<EGWorldObjectEntity>(NewWorldObject) )
				{
					NewEntityObject->SetInitString( *InitString );
					NewEntityObject->SetEntDef( *EntityId );
				}

				if( m_WorldObjsPanel )
				{
					m_WorldObjsPanel->RefreshPanel( EG_To<eg_int>(m_WorldObjsPanel->GetNumItems()) );
				}

				if( m_WorldObjsPanel )
				{
					m_WorldObjsPanel->SetSelectedWorldObject( NewWorldObject );
					switch( SpawnMode )
					{
						case eg_world_file_spawn_ent_t::AtCamera:
							MoveSelectedToCameraPose();
							break;
						case eg_world_file_spawn_ent_t::InFrontOfCamera:
							MoveSelectedInFrontOfCamera();
							break;
						case eg_world_file_spawn_ent_t::InFrontOfCameraSnapToGrid:
							SnapSelectedToGridLookAt();
							break;
					}
				}

				NewWorldObject->AddToWorldPreview( m_PreviewWorld );
			}

			m_bResetEntityProperties = true;
			EGWorldEd_SetDirty();
		}
	}

	void MoveCameraToSelectedObject()
	{
		EGWorldObject* WorldObj = m_WorldObjsPanel ? m_WorldObjsPanel->GetSelectedWorldObject() : nullptr;
		if( WorldObj && m_PreviewPanel )
		{
			egWorldFileEditorSettings EdConfig = m_WorldFile ? m_WorldFile->GetEditorFileSettings() : egWorldFileEditorSettings();
			eg_vec3 Offset(0.f,0.f,EdConfig.GridSpacing*.5f);
			eg_transform NewPose = eg_transform::BuildRotationY( EG_Deg(180.f) ) * eg_transform::BuildTranslation( Offset ) * WorldObj->GetWorldPose();
			m_PreviewPanel->SetCameraPose( NewPose );
		}
	}

	void SetCameraPose( const eg_transform& NewPose )
	{
		m_CameraPose = NewPose;
		if( m_WorldFile )
		{
			m_WorldFile->GetMutableEditorConfig().CameraPose = NewPose;
		}
	}
};

eg_cpstr16 EGWorldEdMainWnd::AppName = L"EG World Editor";
EGWorldEdMainWnd* EGWorldEdMainWnd::GlobalInst = nullptr;

void EGWorldEd_SetDirty()
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->SetDirty();
	}
}

EGWorldEdObjLibPanel& EGWorldEd_GetObjLibPanel()
{
	return *EGWorldEdMainWnd::GlobalInst->GetObjLibPanel();
}

EGWorldEdWorldObjsPanel& EGWorldEd_GetWorldObjsPanel()
{
	return *EGWorldEdMainWnd::GlobalInst->GetWorldObjsPanel();
}

EGWorldEdPreviewPanel& EGWorldEd_GetPreviewPanel()
{
	return *EGWorldEdMainWnd::GlobalInst->GetPreviewPanel();
}

EGWorldFile& EGWorldEd_GetWorldFile()
{
	return *EGWorldEdMainWnd::GlobalInst->GetWorldFile();
}

class EGWndPanelPropEditor* EGWorldEd_GetGlobalSettingsPanel()
{
	return EGWorldEdMainWnd::GlobalInst ? EGWorldEdMainWnd::GlobalInst->GetGlobalSettingsPanel() : nullptr;
}

eg_string_big EGWorldEd_GetCurrentGameFilename()
{
	return EGWorldEdMainWnd::GlobalInst ? EGWorldEdMainWnd::GlobalInst->GetCurrentGameFilename() : "";
}

int EGWorldEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms )
{
	static const egWndAppInfo AppInfo( GetModuleHandleW( nullptr ) , EGResourceLib_GetLibrary() , SW_SHOWNORMAL , true , MAKEINTRESOURCEW(IDI_EGWORLD) , MAKEINTRESOURCEW(IDI_EGWORLD) , L"EGWORLDED_ACCELERATOR" );

	egSdkEngineInitParms InitParms( CT_Clear );
	InitParms.GameName = EngineParms.GameName;
	InitParms.WorldRenderPipeClass = EngineParms.WorldRenderPipeClass;
	InitParms.bAllowDedicatedServer = false;
	InitParms.bAllowCommandLineMap = false;
	EngineForTools_Init( InitParms , false );

	EGWnd_RunApp( AppInfo , [&InitialFilename](int nCmdShow)->EGWndAppWindow*{ return new EGWorldEdMainWnd( nCmdShow , InitialFilename ); } , []( EGWndAppWindow* AppWnd )->void{ delete AppWnd; } );

	EngineForTools_Deinit( false );

	return 0;
}

void EGWorldEd_InsertNewWorldObject( EGClass* ObjectClass )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->InsertNewWorldObject( ObjectClass );
	}
}

void EGWorldEd_SetWorldObjectToEdit( EGWorldObject* ObjectToEdit )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->SetWorldObjectToEdit( ObjectToEdit );
	}
}

void EGWorldEd_DeleteWorldObject( EGWorldObject* Object )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->DeleteWorldObject( Object );
	}
}

void EGWorldEd_MoveObject( EGWorldObject* ObjToMove , EGWorldObject* ObjToMoveBefore , EGWorldObject* ObjParent )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->MoveObject( ObjToMove , ObjToMoveBefore , ObjParent );
	}
}

void EGWorldEd_OnPropChanged( egRflEditor& BaseEditor , const egRflEditor& ChangedProperty )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->OnPropChanged( BaseEditor , ChangedProperty );
	}
}

void EGWorldEd_DrawPreview()
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->DrawPreview();
	}
}

void EGWorldEd_SetCameraPose( const eg_transform& CameraPose )
{
	if( EGWorldEdMainWnd::GlobalInst )
	{
		EGWorldEdMainWnd::GlobalInst->SetCameraPose( CameraPose );
	}
}

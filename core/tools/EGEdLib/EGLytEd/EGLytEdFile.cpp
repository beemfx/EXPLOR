// (c) 2016 Beem Media

#include "EGLytEdFile.h"
#include "EGMenu.h"
#include "EGRenderer.h"
#include "EGEntDict.h"
#include "EGFileData.h"
#include "EGUiLayout.h"
#include "EGLytEdLayoutPanel.h"
#include "EGAudio.h"
#include "EGLytEd.h"
#include "EGUiWidget.h"
#include "EGToolsHelper.h"
#include "EGLytEdPreviewPanel.h"
#include "EGEngineConfig.h"
#include "EGWndPanelPropEditor.h"
#include "EGLytEdSettingsPanel.h"
#include "EGEdUtility.h"
#include "EGEngineDataAssetLib.h"
#include "EGAssetPath.h"
#include "EGEdLib.h"
#include "EGGlobalConfig.h"

EGLytEdFile EGLytEdFile::GlobalLayoutFile;

class EGLytEdFileMenu : public EGMenu
{
	EG_CLASS_BODY( EGLytEdFileMenu , EGMenu )
};

EG_CLASS_DECL( EGLytEdFileMenu )

void EGLytEdFile::Init()
{
	m_LastMousePos = eg_vec2( 0.f, 0.f );
	m_NextMousePos = eg_vec2( 0.f, 0.f );
	m_MenuLayout.CreateNewForTool();
	InitMenu();

	DebugConfig_DrawGridMasks.SetValue( true );

	egUiWidgetInfo::QueryEvents = &StaticQueryEvents;
	egUiWidgetInfo::QueryTextNodes = &StaticQueryTextNodes;
	egUiWidgetInfo::QueryBones = &StaticQueryBones;
	egUiWidgetInfo::QueryPossibleScrollbars = &StaticQueryPossibleScrollbars;

	if( EGLytEdSettingsPanel::GetPanel() )
	{
		EGLytEdSettingsPanel::GetPanel()->SetEditObject( m_MenuLayout.GetEditor() );
	}
}

void EGLytEdFile::Deinit()
{
	if( EGLytEdSettingsPanel::GetPanel() )
	{
		EGLytEdSettingsPanel::GetPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
	}

	egUiWidgetInfo::QueryEvents = nullptr;
	egUiWidgetInfo::QueryTextNodes = nullptr;
	egUiWidgetInfo::QueryBones = nullptr;
	egUiWidgetInfo::QueryPossibleScrollbars = nullptr;

	DeinitMenu();
	EG_SafeDelete( m_Menu );
	m_MenuLayout.Clear();
}

void EGLytEdFile::RefreshSettingsPanel()
{
	if( EGLytEdSettingsPanel::GetPanel() )
	{
		EGLytEdSettingsPanel::GetPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
		EGLytEdSettingsPanel::GetPanel()->SetEditObject( m_MenuLayout.GetEditor() );
	}
}

void EGLytEdFile::SetPreviewAspectRatio( eg_real NewRatio )
{
	m_PreviewRatio = NewRatio;
	if( EGLytEdPreviewPanel::GetPanel() )
	{
		EGLytEdPreviewPanel::GetPanel()->SetPreviewPanelAspectRatio( NewRatio );
	}
}

egUiWidgetInfo* EGLytEdFile::GetObjByMousePos( eg_real x, eg_real y )
{
	egUiWidgetInfo* Out = nullptr;

	if( m_Menu )
	{
		EGUiWidget* FoundObj = m_Menu->FindObjectAt( x, y, nullptr, nullptr, false );
		if( FoundObj )
		{
			Out = const_cast<egUiWidgetInfo*>( FoundObj->GetInfo() );
		}
	}

	return Out;
}

void EGLytEdFile::Update( eg_real DeltaTime )
{
	EGAudio_BeginFrame();

	if( m_Menu )
	{
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::UPDATE, DeltaTime, GetPreviewAspectRatio() );

		eg_bool bMouseMoved = m_LastMousePos != m_NextMousePos;

		if( bMouseMoved )
		{
			m_Menu->HandleMouseEvent( eg_menumouse_e::MOVE, m_NextMousePos.x, m_NextMousePos.y );
			m_LastMousePos = m_NextMousePos;
		}
	}

	EGAudio_EndFrame();
}

void EGLytEdFile::Draw()
{
	if( m_Menu )
	{
		DebugConfig_DrawTextNodes.SetValue( m_bShowTextOutlines );
		DebugConfig_DrawMenuButtons.SetValue( false );
		GlobalConfig_IsUiLayoutTool.SetValue( true );

		MainDisplayList->SetMaterial( EGV_MATERIAL_NULL );

		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::PRE_DRAW , m_PreviewRatio );
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::DRAW , m_PreviewRatio );
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::POST_DRAW , m_PreviewRatio );

		DebugConfig_DrawMenuButtons.SetValue( false );
		DebugConfig_DrawTextNodes.SetValue( false );
		GlobalConfig_IsUiLayoutTool.SetValue( false );
	}
}

void EGLytEdFile::Open( eg_cpstr16 Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( Filename , MemFile );

	eg_string_big GamePath = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte( Filename ) );
	eg_asset_path::SetCurrentFile( GamePath );

	DeinitMenu();
	m_MenuLayout.Load( MemFile.GetData(), MemFile.GetSize(), EGLytEd_GetCurrentGameFilename() , true );
	RefreshSettingsPanel();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( -1 );
	}
}

eg_bool EGLytEdFile::Save( eg_cpstr16 Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	m_MenuLayout.SaveTo( MemFile );

	return EGEdUtility_SaveFile( Filename, MemFile );
}

void EGLytEdFile::InitMenu()
{
	m_Menu = EGNewObject<EGMenu>( &EGLytEdFileMenu::GetStaticClass() , eg_mem_pool::DefaultHi );
	if( m_Menu )
	{
		m_Menu->InitForTool( &m_MenuLayout );
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::INIT );
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::ACTIVATE );
	}
}

void EGLytEdFile::DeinitMenu()
{
	if( m_Menu )
	{
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		m_Menu->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
	}
	EGDeleteObject( m_Menu );
	m_Menu = nullptr;
}

void EGLytEdFile::InsertObject( eg_cpstr StrDef, eg_real x, eg_real y )
{
	EGLogf( eg_log_t::General, "Inserting %s at <%g,%g>", StrDef, x, y );

	m_MenuLayout.InsertObject( StrDef, x*MENU_ORTHO_SIZE, y*MENU_ORTHO_SIZE );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( static_cast<eg_int>( m_MenuLayout.m_Objects.Len() ) - 1 );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::InsertImage( struct egUiWidgetInfo* ObjToInsertBefore )
{
	eg_int NewSelection = -1;

	m_MenuLayout.InsertImage( ObjToInsertBefore );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewSelection );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::InsertClearZ( struct egUiWidgetInfo* ObjToInsertBefore )
{
	eg_int NewSelection = -1;

	m_MenuLayout.InsertClearZ( ObjToInsertBefore );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewSelection );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::InsertText( struct egUiWidgetInfo* ObjToInsertBefore )
{
	eg_int NewSelection = -1;

	m_MenuLayout.InsertText( ObjToInsertBefore );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewSelection );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::InsertLight( struct egUiWidgetInfo* ObjToInsertBefore )
{
	eg_int NewSelection = -1;

	m_MenuLayout.InsertLight( ObjToInsertBefore );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewSelection );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::InsertCamera( struct egUiWidgetInfo* ObjToInsertBefore )
{
	eg_int NewSelection = -1;

	m_MenuLayout.InsertCamera( ObjToInsertBefore );

	DeinitMenu();
	InitMenu();

	if( EGLytEdLayoutPanel::GetPanel() )
	{
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewSelection );
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::MoveObject( struct egUiWidgetInfo* ObjToMove, struct egUiWidgetInfo* ObjToMoveBefore )
{
	eg_bool bMoved = false;
	eg_int NewIndex = -1;

	bMoved = m_MenuLayout.MoveObject( ObjToMove, ObjToMoveBefore );

	for( eg_size_t i = 0; i < m_MenuLayout.m_Objects.Len(); i++ )
	{
		if( m_MenuLayout.m_Objects.GetByIndex( i ) == ObjToMove )
		{
			NewIndex = static_cast<eg_int>( i );
			break;
		}
	}

	if( bMoved )
	{
		DeinitMenu();
		InitMenu();
		EGLytEdLayoutPanel::GetPanel()->RefreshPanel( NewIndex );
		EGLytEd_SetDirty();
	}
}

void EGLytEdFile::DeleteObject( struct egUiWidgetInfo* ObjToMove )
{
	m_MenuLayout.DeleteObject( ObjToMove );
	DeinitMenu();
	InitMenu();
	EGLytEdLayoutPanel::GetPanel()->RefreshPanel( -1 );
	EGLytEd_SetDirty();
}

void EGLytEdFile::QueryEvents( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	if( ObjToQuery->Type == egUiWidgetInfo::eg_obj_t::MESH )
	{
		EGUiWidget* Obj = m_Menu->FindObjectFromLayout( ObjToQuery );
		if( Obj )
		{
			Obj->QueryEvents( Out );
		}
	}
}

void EGLytEdFile::QueryTextNodes( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	if( nullptr == ObjToQuery || nullptr == m_Menu )
	{
		return;
	}

	EGUiWidget* Obj = m_Menu->FindObjectFromLayout( ObjToQuery );

	if( nullptr == Obj )
	{
		return;
	}

	Obj->QueryTextNodes( Out );

	for( const egTextOverrideInfo& Info : ObjToQuery->TextOverrides )
	{
		Out.AppendUnique( EGCrcDb::CrcToString(Info.NodeId).String() );
	}
}

void EGLytEdFile::QueryBones( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	if( nullptr == ObjToQuery || nullptr == m_Menu )
	{
		return;
	}

	EGUiWidget* Obj = m_Menu->FindObjectFromLayout( ObjToQuery );

	if( nullptr == Obj )
	{
		return;
	}

	Obj->QueryBones( Out );

	for( const egBoneOverride& Info : ObjToQuery->BoneOverrides )
	{
		Out.AppendUnique( EGCrcDb::CrcToString(Info.BoneId).String() );
	}
}

void EGLytEdFile::QueryPossibleScrollbars( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	const eg_size_t NumObjs = GetNumObjects();
	for( eg_size_t i=0; i<NumObjs; i++ )
	{
		egUiWidgetInfo* Info = GetObjectInfoByIndex( i );
		eg_bool bIsValidType = Info && Info->Type == egUiWidgetInfo::eg_obj_t::MESH;
		eg_bool bIsValidWidgetType = bIsValidType && (Info->WidgetType == eg_widget_t::NONE);
		eg_bool bIsScrollbarOfThis = Info->WidgetType == eg_widget_t::SCROLLBAR && eg_string_crc(Info->GridIdOfThisScrollbar) == eg_string_crc(ObjToQuery->Id);
		if( (bIsValidWidgetType || bIsScrollbarOfThis) && eg_string_crc(Info->Id).IsNotNull() )
		{
			Out.Append( EGCrcDb::CrcToString(Info->Id).String() );
		}
	}
}

eg_size_t EGLytEdFile::GetNumObjects()
{
	return m_MenuLayout.m_Objects.Len();
}

struct egUiWidgetInfo* EGLytEdFile::GetObjectInfoByIndex( eg_size_t Index )
{
	return m_MenuLayout.m_Objects.IsValidIndex( Index ) ? m_MenuLayout.m_Objects.GetByIndex( Index ) : nullptr;
}

void EGLytEdFile::OnWasChanged( egUiWidgetInfo* Item , eg_bool bFullRebuild )
{
	unused( Item );

	if( bFullRebuild )
	{
		FullRebuild();
	}

	EGLytEd_SetDirty();
}

void EGLytEdFile::FullRebuild()
{
	EGLytEdLayoutPanel* LayoutPanel = EGLytEdLayoutPanel::GetPanel();
	eg_int LayoutSelectedIndex = LayoutPanel ? LayoutPanel->GetSelectedIndex() : -1;

	m_MenuLayout.OnToolFullRebuild();
	DeinitMenu();
	InitMenu();

	if( LayoutPanel )
	{
		LayoutPanel->RefreshPanel( LayoutSelectedIndex );
	}
}

void EGLytEdFile::SetAllTextToBWNoShadow()
{
	for( egUiWidgetInfo* Item : m_MenuLayout.m_Objects)
	{
		if( Item )
		{
			auto& Tni = Item->TextNodeInfo;
			Tni.Color = eg_color32::Black;
			Tni.bHasDropShadow = false;
		}
	}
	FullRebuild();
}

void EGLytEdFile::StaticQueryEvents( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	EGLytEdFile::Get().QueryEvents( ObjToQuery , Out );
}

void EGLytEdFile::StaticQueryTextNodes( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	EGLytEdFile::Get().QueryTextNodes( ObjToQuery , Out );
}

void EGLytEdFile::StaticQueryBones( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	EGLytEdFile::Get().QueryBones( ObjToQuery , Out );
}

void EGLytEdFile::StaticQueryPossibleScrollbars( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out )
{
	EGLytEdFile::Get().QueryPossibleScrollbars( ObjToQuery , Out );
}

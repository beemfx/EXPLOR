// (c) 2017 Beem Media

#include "EGUiLayout.h"
#include "EGLocalize.h"
#include "EGAlwaysLoaded.h"
#include "EGEntDict.h"
#include "EGEntDef.h"
#include "EGEntObj.h"
#include "EGFileData.h"
#include "EGLoader.h"
#include "EGEngine.h"
#include "EGCrcDb.h"
#include "EGMenu.h"
#include "EGUiWidgetInfo.h"
#include "EGReflectionDeserializer.h"
#include "EGTimer.h"

///////////////////////////////////////////////////////////////////////////////
//
// The Layout Manager - Keeps all layouts loaded so they can be grabbed when
// needed.
//
///////////////////////////////////////////////////////////////////////////////
EGUiLayoutMgr EGUiLayoutMgr::s_LayoutMgr;

void EGUiLayoutMgr::Init()
{
	m_DefaultLayout.Clear();

	eg_uint64 StartTime = Timer_GetRawTime();
	EGAlwaysLoadedFilenameList EnumList;
	AlwaysLoaded_GetFileList( &EnumList , "elyt" );
	m_LayoutMap.Init( EnumList.Len() );
	m_LayoutMap.SetNotFoundItem( &m_DefaultLayout );
	for( egAlwaysLoadedFilename* FnItem : EnumList )
	{
		Init_AddLayout( FnItem->Filename );
	}
	eg_uint64 EndTime = Timer_GetRawTime();
	eg_real LoadingSeconds = Timer_GetRawTimeElapsedSec( StartTime , EndTime );
	EGLogf( eg_log_t::Performance , "Layouts loaded in %g seconds." , LoadingSeconds );
}

void EGUiLayoutMgr::Init_AddLayout( eg_cpstr Filename )
{
	EGUiLayout* Layout = new ( eg_mem_pool::System ) EGUiLayout;
	if( nullptr == Layout )
	{
		assert( false ); //Menu system out of memory!
		EGLogf( eg_log_t::Error , __FUNCTION__": Menu system out of memory, couldn't load \"%s\".", Filename );
		return;
	}

	Layout->Load( Filename );

	eg_string_crc MenuCrc = EGCrcDb::StringToCrc(*Layout->m_Id);
	if( MenuCrc == eg_crc("") )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__": Menu %s had no id, using filename." , Filename );
		MenuCrc = FilenameToMenuCrc( Filename );
	}

	if( m_LayoutMap.Contains( MenuCrc ) )
	{
		assert( false ); //Menu with same Crc already exists, crc collision.
		EGLogf( eg_log_t::Error , __FUNCTION__": Menu name crc collision, couldn't load \"%s\".", Filename );
		delete Layout;
		return;
	}

	EGClass* Script = Layout->m_ClassName.Class;
	if( Script && Script->IsA( &EGMenu::GetStaticClass() ) )
	{
		m_LayoutMap.Insert( MenuCrc , Layout );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__": Layout \"%s\" did not have a valid proc associated with it, it will be associated with the default menu.", Filename );
		// assert( false || Engine_IsTool() ); // No proc associated with this layout, this layout cannot be used!
		Layout->m_ClassName.Class = &EGMenu::GetStaticClass();
		m_LayoutMap.Insert( MenuCrc , Layout );
	}
}

void EGUiLayoutMgr::Deinit()
{
	for( eg_uint i=0; i<m_LayoutMap.Len(); i++ )
	{
		EGUiLayout* Layout = m_LayoutMap.GetByIndex( i );
		delete Layout;
	}
	m_LayoutMap.Clear();
	m_LayoutMap.Deinit();
}

const EGUiLayout* EGUiLayoutMgr::GetLayout( eg_string_crc MenuId )
{
	return m_LayoutMap.Get( MenuId );
}

eg_string_crc EGUiLayoutMgr::FilenameToMenuCrc( eg_cpstr Filename )
{
	eg_string Str;
	Str.SetToFilenameFromPathNoExt( Filename );
	Str.ConvertToLower();
	return eg_string_crc(Str);
}

EGUiLayout::EGUiLayout() 
{
	Clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// The Menu Layout - Describes how a menu is drawn, but not the state of it.
//
///////////////////////////////////////////////////////////////////////////////

void EGUiLayout::Load( eg_cpstr Filename )
{
	EGFileData fileScript( eg_file_data_init_t::HasOwnMemory );
	MainLoader->LoadNowTo(Filename, fileScript);

	Load( fileScript.GetData() , fileScript.GetSize() , Filename , false );
}

void EGUiLayout::Load( const void* Data , eg_size_t DataSize , eg_cpstr RefName , eg_bool IsInTool )
{
	Clear();

	m_Editor = EGReflection_GetEditor( *this , "Layout" );
	m_IsInTool = IsInTool;

	XMLLoad( Data , DataSize , RefName );

	m_Editor.ExecutePostLoad( RefName , IsInTool );
	for( egUiWidgetInfo* Info : m_Objects )
	{
		Info->GetEditor()->ExecutePostLoad( RefName , IsInTool );
	}

	if( !IsInTool )
	{
		m_Editor = CT_Clear;
		for( egUiWidgetInfo* Info : m_Objects )
		{
			Info->DeinitEditor();
		}
	}

	FinalizeLoad();
}

void EGUiLayout::LoadForResave( EGFileData& FileData, eg_cpstr RefName )
{
	Clear();

	m_Editor = EGReflection_GetEditor( *this , "Layout" );
	m_IsInTool = true;

	XMLLoad( FileData.GetData() , FileData.GetSize() , RefName );

	m_Editor.ExecutePostLoad( RefName , m_IsInTool );
	for( egUiWidgetInfo* Info : m_Objects )
	{
		Info->GetEditor()->ExecutePostLoad( RefName , m_IsInTool );
	}
}

void EGUiLayout::SaveTo( class EGFileData& FileOut )
{
	auto Write = [&FileOut]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		FileOut.WriteStr8( Buffer );
	};

	eg_string FlagsStr;

	auto AddFlag = [&FlagsStr]( eg_cpstr Flag ) -> void
	{
		if( FlagsStr.Len() > 0 )
		{
			FlagsStr.Append( '|' );
		}

		FlagsStr.Append( Flag );
	};

	if( m_NoInput )
	{
		AddFlag( "NOINPUT" );
	}

	Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	Write( "<elyt id=\"%s\" script=\"%s\" flags=\"%s\">\r\n" , EGString_ToXmlFriendly(*m_Id).String() , EGString_ToXmlFriendly( m_ClassName.Class ? m_ClassName.Class->GetName() : "").String() , FlagsStr.String() );

	Write( "\r\n" );

	for( egUiWidgetInfo* Obj : m_Objects )
	{
		auto GetTagString = [&Obj]() -> eg_string_big
		{
			switch( Obj->Type )
			{
				case egUiWidgetInfo::eg_obj_t::CAMERA: return "camera";
				case egUiWidgetInfo::eg_obj_t::CLEARZ: return "clearz";
				case egUiWidgetInfo::eg_obj_t::MESH: return "object";
				case egUiWidgetInfo::eg_obj_t::TEXT_NODE: return "textnode";
				case egUiWidgetInfo::eg_obj_t::LIGHT: return "light";
				case egUiWidgetInfo::eg_obj_t::IMAGE: return "image";
			}

			return "unknown";
		};

		auto WriteProperties = [&FileOut,&Write]( const egUiWidgetInfo* Obj ) -> void
		{
			Write( "\t\t<properties>\r\n" );
			Obj->Serialize( FileOut , 3 );
			Write( "\t\t</properties>\r\n" );
		};

		eg_string_big TagString = GetTagString();

		Write( "\t<%s>\r\n" , TagString.String() );
		Obj->RefreshEditableProperties();
		WriteProperties( Obj );
		Write( "\t</%s>\r\n" , TagString.String() );
		Write( "\r\n" );
	}

	Write( "</elyt>\r\n" );
}

void EGUiLayout::Clear( void )
{
	m_Id = "";
	m_ClassName.Class = nullptr;
	m_NoInput = false;
	while( m_Objects.HasItems() )
	{
		DeleteObject( m_Objects.GetOne() );
	}
	m_IsInTool = false;
	m_Editor = CT_Clear;
}

void EGUiLayout::OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& AttGet)
{
	auto InsertNew = [this]( egUiWidgetInfo::eg_obj_t Type ) -> void
	{
		egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( Type );
		if( NewObj )
		{
			NewObj->InitEditor();
			m_Objects.InsertLast( NewObj );
		}
	};

	static const struct egTypeTable
	{
		eg_string_crc CrcId;
		egUiWidgetInfo::eg_obj_t Type;
	}
	TypeTable[] =
	{
		{ eg_crc("object") , egUiWidgetInfo::eg_obj_t::MESH },
		{ eg_crc("image") , egUiWidgetInfo::eg_obj_t::IMAGE },
		{ eg_crc("clearz") , egUiWidgetInfo::eg_obj_t::CLEARZ },
		{ eg_crc("camera") , egUiWidgetInfo::eg_obj_t::CAMERA },
		{ eg_crc("textnode") , egUiWidgetInfo::eg_obj_t::TEXT_NODE },
		{ eg_crc("light") , egUiWidgetInfo::eg_obj_t::LIGHT },
	};

	if( m_Deserializer )
	{
		m_Deserializer->OnXmlTagBegin( Tag , AttGet );
	}
	else if( Tag.Equals("elyt") )
	{
		OnTag_emenu( AttGet );
	}
	else if( Tag.EqualsI( "properties" ) )
	{
		assert( nullptr == m_Deserializer );
		m_Deserializer = new EGReflectionDeserializer;
		if( m_Deserializer )
		{
			m_Deserializer->Init( *m_Objects.GetLast()->GetEditor() , GetFilename() );
		}
	}
	else
	{
		eg_string_crc Type = eg_string_crc(Tag);

		eg_bool bFound = false;
		for( const egTypeTable& Item : TypeTable )
		{
			if( Item.CrcId == Type )
			{
				InsertNew( Item.Type );
				bFound = true;
				break;
			}
		}

		assert( bFound ); // Bad tag?
	}

}

void EGUiLayout::OnTagEnd( const eg_string_base& Tag )
{
	if( Tag.Equals("properties") )
	{
		assert( m_Deserializer );
		if( m_Deserializer )
		{
			m_Deserializer->Deinit();
			delete m_Deserializer;
			m_Deserializer = nullptr;
		}
	}
	else if( m_Deserializer )
	{
		m_Deserializer->OnXmlTagEnd( Tag );
	}
}

void EGUiLayout::CreateNewForTool()
{
	Clear();
	m_Editor = EGReflection_GetEditor( *this , "Layout" );
	m_IsInTool = true;
}

void EGUiLayout::InsertObject( eg_cpstr StrDef, eg_real x, eg_real y )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( egUiWidgetInfo::eg_obj_t::MESH );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	m_Objects.InsertLast( NewObj );

	NewObj->Id = CT_Clear;
	NewObj->EntDefCrc = EGCrcDb::StringToCrc(StrDef);
	NewObj->IsAlwaysLoaded = true;
	NewObj->Anchor.X = eg_anchor_t::CENTER;
	NewObj->Anchor.Y = eg_anchor_t::CENTER;
	NewObj->ScaleVec = eg_vec3(1.f,1.f,1.f);
	NewObj->StartupEvent.Crc = CT_Clear;
	NewObj->TrackExtend = 0.f;
	NewObj->Type = egUiWidgetInfo::eg_obj_t::MESH;
	NewObj->BasePose = eg_transform::BuildTranslation( eg_vec3( x , y , 0.f ) );
	NewObj->WidgetType = eg_widget_t::NONE;

	if( m_IsInTool )
	{
		const EGEntDef* Def = EntDict_GetDef( NewObj->EntDefCrc );
		if( Def )
		{
			Def->CreateAssets();
		}
	}
}

void EGUiLayout::InsertImage( struct egUiWidgetInfo* ObjToInsertBefore )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( egUiWidgetInfo::eg_obj_t::IMAGE );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	if( ObjToInsertBefore )
	{
		m_Objects.InsertBefore( ObjToInsertBefore , NewObj );
	}
	else
	{
		m_Objects.InsertLast( NewObj );
	}

	NewObj->Type = egUiWidgetInfo::eg_obj_t::IMAGE;
	EGMaterialDef DefMat( CT_Default );
	NewObj->ImageInfo.Properties = DefMat.m_Mtr;
	NewObj->ScaleVec = eg_vec3(100.f,100.f,1.f);
	NewObj->IsAlwaysLoaded = true;
}

void EGUiLayout::InsertClearZ( struct egUiWidgetInfo* ObjToInsertBefore )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( egUiWidgetInfo::eg_obj_t::CLEARZ );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	if( ObjToInsertBefore )
	{
		m_Objects.InsertBefore( ObjToInsertBefore , NewObj );
	}
	else
	{
		m_Objects.InsertLast( NewObj );
	}

	NewObj->Type = egUiWidgetInfo::eg_obj_t::CLEARZ;
}

void EGUiLayout::InsertText( struct egUiWidgetInfo* ObjToInsertBefore )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( egUiWidgetInfo::eg_obj_t::TEXT_NODE );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	if( ObjToInsertBefore )
	{
		m_Objects.InsertBefore( ObjToInsertBefore , NewObj );
	}
	else
	{
		m_Objects.InsertLast( NewObj );
	}

	NewObj->Type = egUiWidgetInfo::eg_obj_t::TEXT_NODE;
	NewObj->TextNodeInfo.LocText = EGCrcDb::StringToCrc("Sample Text");
	NewObj->TextNodeInfo.TextContext = eg_text_context::None;
	NewObj->TextNodeInfo.Width = MENU_ORTHO_SIZE;
	NewObj->TextNodeInfo.Height = MENU_ORTHO_SIZE/10.f;
	NewObj->TextNodeInfo.NumLines = 1;
	NewObj->TextNodeInfo.Color = eg_color32::White;
	NewObj->TextNodeInfo.bHasDropShadow = true;
	NewObj->TextNodeInfo.DropShadowColor = eg_color32::Black;
}

void EGUiLayout::InsertLight( struct egUiWidgetInfo* ObjToInsertBefore )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( egUiWidgetInfo::eg_obj_t::LIGHT );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	if( ObjToInsertBefore )
	{
		m_Objects.InsertBefore( ObjToInsertBefore , NewObj );
	}
	else
	{
		m_Objects.InsertLast( NewObj );
	}

	NewObj->Type = egUiWidgetInfo::eg_obj_t::LIGHT;
	NewObj->LightInfo.LightColor = eg_color32::White;
	NewObj->LightInfo.AmbientColor = eg_color32::Black;
	NewObj->LightInfo.Range = 10.f;
	NewObj->LightInfo.Falloff = 5.f;
}

void EGUiLayout::InsertCamera( struct egUiWidgetInfo* ObjToInsertBefore )
{
	if( !m_IsInTool )
	{
		assert( false ); // Tool only functionality.
		return;
	}

	egUiWidgetInfo* NewObj = new ( eg_mem_pool::System ) egUiWidgetInfo( eg_camera_t::PERSP );
	if( nullptr == NewObj ){ assert( false ); return; } //Out of memory?
	if( ObjToInsertBefore )
	{
		m_Objects.InsertBefore( ObjToInsertBefore , NewObj );
	}
	else
	{
		m_Objects.InsertLast( NewObj );
	}

	NewObj->Type = egUiWidgetInfo::eg_obj_t::CAMERA;
	NewObj->CameraInfo.Type = eg_camera_t_ed::PERSPECTIVE;
}

eg_bool EGUiLayout::MoveObject( egUiWidgetInfo* ObjToMove, egUiWidgetInfo* ObjToMoveBefore )
{
	eg_bool bMoved = true;

	if( ObjToMove == ObjToMoveBefore )
	{
		bMoved = false;
	}

	if( nullptr == ObjToMove && ObjToMove == m_Objects.GetLast() )
	{
		bMoved = false;
	}

	if( bMoved )
	{
		m_Objects.Remove( ObjToMove );
		if( ObjToMoveBefore )
		{
			m_Objects.InsertBefore( ObjToMoveBefore , ObjToMove );
		}
		else
		{
			m_Objects.InsertLast( ObjToMove );
		}
	}

	return bMoved;
}

void EGUiLayout::DeleteObject( egUiWidgetInfo* ObjToDelete )
{
	if( ObjToDelete )
	{
		m_Objects.Remove( ObjToDelete );

		if( ObjToDelete->IsAlwaysLoaded || m_IsInTool )
		{
			ObjToDelete->DeinitAlwaysLoaded();
		}

		EG_SafeDelete( ObjToDelete );
	}
}

void EGUiLayout::OnToolFullRebuild()
{
	FinalizeLoad( true );
}

void EGUiLayout::OnTag_emenu( const EGXmlAttrGetter& AttGet )
{
	m_Id = AttGet.GetString( "id" );
	m_ClassName.Class = EGClass::FindClass( AttGet.GetString( "script" ) );

	eg_string FlagsStr = AttGet.GetString( "flags" , "" );
	if( FlagsStr.Contains( "NOINPUT" ) )
	{
		m_NoInput = true;
	}

	EGCrcDb::AddAndSaveIfInTool( *m_Id );
}

void EGUiLayout::FinalizeLoad( eg_bool bForToolRebuild /*= false*/ )
{
	if( bForToolRebuild )
	{
		// Clear scrollbars because they may have changed.
		for( egUiWidgetInfo* Obj : m_Objects )
		{
			if( Obj->WidgetType == eg_widget_t::SCROLLBAR )
			{
				Obj->WidgetType = eg_widget_t::NONE;
				Obj->GridIdOfThisScrollbar = CT_Clear;
			}
		}
	}

	for( egUiWidgetInfo* Obj : m_Objects )
	{
		if( m_IsInTool )
		{
			Obj->RefreshEditableProperties();
		}

		// Load the Always Loaded objects (not if this is a tool rebuild tho)
		if( (Obj->IsAlwaysLoaded || m_IsInTool) && !bForToolRebuild )
		{
			Obj->InitAlwaysLoaded();
		}

		// If this is a grid, and has a scrollbar mutate the scrollbar into a scrollbar.
		if( (Obj->WidgetType == eg_widget_t::GRID) && (Obj->GridInfo.ScrollbarId.Crc.IsNotNull()) )
		{
			for( egUiWidgetInfo* SearchObjs : m_Objects )
			{
				eg_bool IsMatchingId = SearchObjs->Id == Obj->GridInfo.ScrollbarId;
				eg_bool IsAcceptable = IsMatchingId && (SearchObjs->Type == egUiWidgetInfo::eg_obj_t::MESH) && (SearchObjs->WidgetType == eg_widget_t::NONE || SearchObjs->WidgetType == eg_widget_t::SCROLLBAR);
				if( IsMatchingId )
				{
					assert( IsAcceptable ); // You cannot have a scrollbar be anything other than a standard mesh entity.
					if( IsAcceptable )
					{
						SearchObjs->WidgetType = eg_widget_t::SCROLLBAR;
						SearchObjs->GridIdOfThisScrollbar = Obj->Id;
					}
				}
			}
		}
	}
}

egUiWidgetInfo* EGUiLayout::FindObjById( eg_string_crc Id )
{
	egUiWidgetInfo* Out = nullptr;

	for( egUiWidgetInfo* Obj : m_Objects )
	{
		if( Obj->Id == Id && Id != eg_string_crc( CT_Clear ) )
		{
			Out = Obj;
			break;
		}
	}

	return Out;
}

eg_cpstr EGUiLayout::XMLObjName()const { return ( "Menu" ); }

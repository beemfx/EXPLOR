// (c) 2017 Beem Media

#include "EGUiWidgetInfo.h"
#include "EGXMLBase.h"
#include "EGCrcDb.h"
#include "EGEntDef.h"
#include "EGEntDict.h"
#include "EGRenderer.h"

egUiWidgetInfo::egFnQueryObject egUiWidgetInfo::QueryEvents = nullptr;
egUiWidgetInfo::egFnQueryObject egUiWidgetInfo::QueryTextNodes = nullptr;
egUiWidgetInfo::egFnQueryObject egUiWidgetInfo::QueryBones = nullptr;
egUiWidgetInfo::egFnQueryObject egUiWidgetInfo::QueryPossibleScrollbars = nullptr;

egUiWidgetInfo::egUiWidgetInfo( eg_camera_t CameraType ) 
: egUiWidgetInfo( eg_obj_t::CAMERA )
{
	Type = eg_obj_t::CAMERA;
	CameraInfo.Type = static_cast<eg_camera_t_ed>(CameraType);

	if( CameraType == eg_camera_t::ORTHO )
	{
		CameraInfo.Near = -MENU_ORTHO_SIZE;
		CameraInfo.Far = MENU_ORTHO_SIZE;
		CameraInfo.OrthoRadius = MENU_ORTHO_SIZE;

		BasePose = eg_transform::BuildIdentity();
	}
	else
	{
		CameraInfo.Near = .1f;
		CameraInfo.Far = 1000.f;
		CameraInfo.FovDeg = 180.f;

		BasePose = eg_transform::BuildTranslation( eg_vec3(0.f, 0.f, -100.f) );
	}
}

egUiWidgetInfo::egUiWidgetInfo( eg_obj_t InType ) 
: IListable()
, Type( InType )
{
	
}

egUiWidgetInfo::~egUiWidgetInfo()
{
	m_Editor = CT_Clear;
}

void egUiWidgetInfo::InitEditor()
{
	m_Editor = EGReflection_GetEditor( *this , "WidgetInfo" );
	RefreshEditableProperties();
}

void egUiWidgetInfo::DeinitEditor()
{
	m_Editor = CT_Clear;
}

eg_string egUiWidgetInfo::AnchorToString( eg_anchor_t Type )
{
	eg_string Out = "UNK";
	switch( Type )
	{
	case eg_anchor_t::CENTER: Out = "CENTER"; break;
	case eg_anchor_t::TOP: Out = "TOP"; break;
	case eg_anchor_t::RIGHT: Out = "RIGHT"; break;
	case eg_anchor_t::BOTTOM: Out = "BOTTOM"; break;
	case eg_anchor_t::LEFT: Out = "LEFT"; break;
	}

	return Out;
}

void egUiWidgetInfo::GetAnchorFromString( eg_cpstr Str, eg_anchor_t* OutX, eg_anchor_t* OutY )
{
	eg_string Anchor( Str );

	eg_anchor_t AnchorX = eg_anchor_t::CENTER;
	eg_anchor_t AnchorY = eg_anchor_t::CENTER;

	if( Anchor.Len() > 0 )
	{
		Anchor.ConvertToUpper();

		// X
		if( Anchor.Contains( "RIGHT" ) )
		{
			AnchorX = eg_anchor_t::RIGHT;
		}
		else if( Anchor.Contains( "LEFT" ) )
		{
			AnchorX = eg_anchor_t::LEFT;
		}

		// Y
		if( Anchor.Contains( "TOP" ) )
		{
			AnchorY = eg_anchor_t::TOP;
		}
		else if( Anchor.Contains( "BOTTOM" ) )
		{
			AnchorY = eg_anchor_t::BOTTOM;
		}
	}

	if( OutX ){ *OutX = AnchorX; }
	if( OutY ){ *OutY = AnchorY; }
}

eg_string egUiWidgetInfo::GetToolDesc() const
{
	eg_string Out( CT_Clear );

	eg_string IdAsString = EGCrcDb::CrcToString( Id );

	switch( Type )
	{
	case eg_obj_t::CLEARZ:
		Out = "Clear Z";
		break;
	case eg_obj_t::CAMERA:
	{
		if( IdAsString.Len() == 0 )
		{
			IdAsString = "**UNNAMED**";
		}
		Out = EGString_Format( "%s (%s)", IdAsString.String(), CameraInfo.Type == eg_camera_t_ed::ORTHOGRAPHIC ? "ORTHO_CAM" : "PERSP_CAM" );
	} break;
	case eg_obj_t::MESH:
		if( IdAsString.Len() == 0 )
		{
			IdAsString = "**UNNAMED**";
		}
		Out = EGString_Format( "%s (%s,%s)", IdAsString.String() , EGCrcDb::CrcToString(EntDefCrc).String() , ClassName.Class ? ClassName.Class->GetName() : "" );
		break;
	case eg_obj_t::TEXT_NODE:
	{
		if( IdAsString.Len() == 0 )
		{
			IdAsString = "**UNNAMED**";
		}
		Out = EGString_Format( "%s (TEXT_NODE)", IdAsString.String() );
	} break;
	case eg_obj_t::LIGHT:
	{
		if( IdAsString.Len() == 0 )
		{
			IdAsString = "**UNNAMED**";
		}
		Out = EGString_Format( "%s (LIGHT) %g", IdAsString.String(), eg_real(LightInfo.Range) );
	} break;
	case eg_obj_t::IMAGE:
	{
		if( IdAsString.Len() == 0 )
		{
			IdAsString = "**UNNAMED**";
		}
		Out = EGString_Format( "%s (IMAGE:%s)", IdAsString.String() , *ImageInfo.Texture.Path );
	} break;
	}

	if( bIsLocked )
	{
		Out = EGString_Format( "(L) %s", Out.String() );
	}

	if( IsGuide )
	{
		Out = EGString_Format( "(GUIDE) %s", Out.String() );
	}
	//;
	return Out;
}

void egUiWidgetInfo::InitCameraType( eg_camera_t InType )
{
	CameraInfo.Type = static_cast<eg_camera_t_ed>(InType);
	if( InType == eg_camera_t::PERSP )
	{
		// Set some different defaults maybe?
		CameraInfo.Near = .1f;
		CameraInfo.Far = 1000.f;
		CameraInfo.FovDeg = 180.f;
		CameraInfo.OrthoRadius = MENU_ORTHO_SIZE;
		BasePose = eg_transform::BuildTranslation( eg_vec3(0.f,0.f,-100.f) );
	}
	else if( InType == eg_camera_t::ORTHO )
	{
		// Set some different defaults maybe?
		CameraInfo.Near = -MENU_ORTHO_SIZE;
		CameraInfo.Far = MENU_ORTHO_SIZE;
		CameraInfo.FovDeg = 180.f;
		CameraInfo.OrthoRadius = MENU_ORTHO_SIZE;
		BasePose = eg_transform::BuildIdentity();
	}
	else
	{
		assert( false );
	}
}

egUiWidgetInfoEditNeeds egUiWidgetInfo::OnEditPropertyChanged( const egRflEditor* ChangedProperty )
{
	egUiWidgetInfoEditNeeds Out;

	Out.bRebuildPreview = true;

	if( EGString_Equals( ChangedProperty->GetVarName() , "WidgetType" ) )
	{
		if( WidgetType != eg_widget_t::NONE )
		{
			bDrawAsMask = false;
		}

		if( WidgetType == eg_widget_t::GRID )
		{
			GridInfo.bAutoMask = true;
			GridInfo.Rows = 5;
			GridInfo.Cols = 1;
			GridInfo.ScrollbarId = CT_Clear;
		}

		if( eg_widget_t(WidgetType) != eg_widget_t::NONE )
		{
			BoneOverrides.Clear();
		}

		RefreshEditableProperties();
		Out.bRebuildEditor = true;
	}

	if( ChangedProperty->GetData() == &BasePose )
	{
		Out.bRebuildPreview = false;
	}

	if( EGString_Equals( ChangedProperty->GetVarName() , "bDrawAsMask" ) )
	{
		if( Type != eg_obj_t::MESH || WidgetType != eg_widget_t::NONE )
		{
			bDrawAsMask = false;
		}
	}

	if( EGString_Equals( ChangedProperty->GetVarName() , "Type" ) )
	{
		RefreshEditableProperties();
		Out.bRebuildEditor = true;
	}

	if( EGString_Equals( ChangedProperty->GetVarName() , "TextOverrides" ) || EGString_Equals( ChangedProperty->GetVarName() , "BoneOverrides" ) )
	{
		RefreshEditableProperties();
		Out.bRebuildEditor = true;
	}

	#define LIMIT_PROP( _prop_ , _type_ , _min_ , _max_ ) { auto OldValue = _prop_; _prop_ = EG_Clamp<_type_>( _prop_ , _min_ , _max_ ); if( OldValue != _prop_ ){ Out.bRebuildEditor = true; } }
	#define LIMIT_PROP_MIN( _prop_ , _type_ , _min_ ) { auto OldValue = _prop_; _prop_ = EG_Max<_type_>( _prop_ , _min_ ); if( OldValue != _prop_ ){ Out.bRebuildEditor = true; } }

	LIMIT_PROP( GridInfo.Cols , eg_int , 1 , 100 )
	LIMIT_PROP( GridInfo.Rows , eg_int , 1 , 100 )
	LIMIT_PROP( TextNodeInfo.NumLines , eg_int , 1 , 20 )
	LIMIT_PROP_MIN( LightInfo.Range , eg_real , 1.f )
	LIMIT_PROP( LightInfo.Falloff , eg_real , 0.f , LightInfo.Range )
	LIMIT_PROP( CameraInfo.FovDeg , eg_real , 35.f , 325.f )

	#undef LIMIT_PROP
	#undef LIMIT_PROP_MIN

	return Out;
}

void egUiWidgetInfo::RefreshEditableProperties()
{
	if( m_Editor.GetData() != this )
	{
		assert( m_Editor.GetData() == nullptr );
		InitEditor();
	}

	auto SetEditable = [this]( eg_cpstr Name , eg_bool bNewValue ) -> void
	{
		egRflEditor* VarEd = m_Editor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetEditable( bNewValue );
		}
	};

	auto SetSerialized = [this]( eg_cpstr Name , eg_bool bNewValue ) -> void
	{
		egRflEditor* VarEd = m_Editor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetSerialized( bNewValue );
		}
	};

	auto SetEditableAndSerialized = [this]( eg_cpstr Name , eg_bool bNewValue ) -> void
	{
		egRflEditor* VarEd = m_Editor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetEditable( bNewValue );
			VarEd->SetSerialized( bNewValue );
		}
	};

	SetEditable( "bIsLocked" , false );
	SetEditable( "EntDefCrc" , false );
	
	SetEditableAndSerialized( "Id" , Type != eg_obj_t::CLEARZ );
	SetEditableAndSerialized( "WidgetType" , Type == eg_obj_t::MESH );
	SetSerialized( "EntDefCrc" , Type == eg_obj_t::MESH );
	SetEditableAndSerialized( "BasePose" , Type == eg_obj_t::MESH || Type == eg_obj_t::LIGHT || Type == eg_obj_t::TEXT_NODE || Type == eg_obj_t::CAMERA || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "StartupEvent" , Type == eg_obj_t::MESH && WidgetType != eg_widget_t::GRID );
	SetEditableAndSerialized( "ScaleVec" , Type == eg_obj_t::MESH || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "IsAlwaysLoaded" , Type == eg_obj_t::MESH || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "IsLit" , Type == eg_obj_t::MESH || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "bDrawAsMask" , (Type == eg_obj_t::MESH && WidgetType == eg_widget_t::NONE) || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "bDrawOnMaskOnly" , (Type == eg_obj_t::MESH && WidgetType != eg_widget_t::GRID) || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "Anchor" , Type == eg_obj_t::MESH || Type == eg_obj_t::LIGHT || Type == eg_obj_t::TEXT_NODE || Type == eg_obj_t::IMAGE );
	SetEditableAndSerialized( "Adjacency" , Type == eg_obj_t::MESH && ( WidgetType == eg_widget_t::BUTTON || WidgetType == eg_widget_t::GRID ) );
	SetEditableAndSerialized( "GridInfo" , Type == eg_obj_t::MESH && WidgetType == eg_widget_t::GRID );
	SetEditableAndSerialized( "TrackExtend" , Type == eg_obj_t::MESH && WidgetType == eg_widget_t::SCROLLBAR );
	SetEditableAndSerialized( "GridIdOfThisScrollbar" , Type == eg_obj_t::MESH && WidgetType == eg_widget_t::SCROLLBAR );
	SetEditableAndSerialized( "BoneOverrides" , Type == eg_obj_t::MESH && WidgetType == eg_widget_t::NONE );
	SetEditableAndSerialized( "TextOverrides" , Type == eg_obj_t::MESH && WidgetType != eg_widget_t::GRID );
	SetEditableAndSerialized( "LightInfo" , Type == eg_obj_t::LIGHT );
	SetEditableAndSerialized( "TextNodeInfo" , Type == eg_obj_t::TEXT_NODE );
	SetEditableAndSerialized( "CameraInfo" , Type == eg_obj_t::CAMERA );
	SetEditableAndSerialized( "ImageInfo" , Type == eg_obj_t::IMAGE );

	egComboBoxPopulateFn PopulateStartupEvents = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
	{
		bManualEditOut = true;
		Out.Append( "" );
		if( QueryEvents )
		{
			QueryEvents( this , Out );
		}
	};

	egComboBoxPopulateFn PopulateScrollBarIds = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
	{
		bManualEditOut = true;
		Out.Append( "" );
		if( QueryPossibleScrollbars )
		{
			QueryPossibleScrollbars( this , Out );
		}
	};

	egComboBoxPopulateFn PopulateTextNodes = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
	{
		bManualEditOut = true;
		Out.Append( "" );
		if( QueryTextNodes )
		{
			QueryTextNodes( this , Out );
		}
	};

	egComboBoxPopulateFn PopulateBones = [this]( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) -> void
	{
		bManualEditOut = true;
		Out.Append( "" );
		if( QueryBones )
		{
			QueryBones( this , Out );
		}
	};

	StartupEvent.PopulateCb = PopulateStartupEvents;
	GridInfo.ScrollbarId.PopulateCb = PopulateScrollBarIds;

	for( egTextOverrideInfo& Info : TextOverrides )
	{
		Info.NodeId.PopulateCb = PopulateTextNodes;
	}

	for( egBoneOverrideInfo& Info : BoneOverrides )
	{
		Info.BoneId.PopulateCb = PopulateBones;
	}
}

void egUiWidgetInfo::Serialize( EGFileData& File, eg_int Depth ) const
{
	assert( m_Editor.GetData() == this );
	m_Editor.Serialize( eg_rfl_serialize_fmt::XML , Depth , File );
}

void egUiWidgetInfo::InitAlwaysLoaded()
{
	if( Type == egUiWidgetInfo::eg_obj_t::MESH )
	{
		const EGEntDef* Def = EntDict_GetDef( EntDefCrc );
		if( Def )
		{
			Def->CreateAssets();
		}
	}

	if( Type == eg_obj_t::IMAGE )
	{
		ImageInfo.AlwaysLoadedMaterial = EGRenderer::Get().CreateMaterialFromTextureFile( *ImageInfo.Texture.FullPath );
	}
}

void egUiWidgetInfo::DeinitAlwaysLoaded()
{
	if( Type == egUiWidgetInfo::eg_obj_t::MESH )
	{
		const EGEntDef* Def = EntDict_GetDef( EntDefCrc );
		if( Def )
		{
			Def->DestroyAssets();
		}

	}

	if( Type == eg_obj_t::IMAGE )
	{
		if( ImageInfo.AlwaysLoadedMaterial != EGV_MATERIAL_NULL )
		{
			EGRenderer::Get().DestroyMaterial( ImageInfo.AlwaysLoadedMaterial );
			ImageInfo.AlwaysLoadedMaterial = EGV_MATERIAL_NULL;
		}
	}
}

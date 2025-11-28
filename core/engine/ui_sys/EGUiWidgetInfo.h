// (c) 2017 Beem Media

#pragma once

#include "EGUiTypes.h"
#include "EGRendererTypes.h"
#include "EGEntTypes.h"
#include "EGUiWidgetInfoTypes.h"
#include "EGEngineSerializeTypes.h"
#include "EGComboBoxEd.h"
#include "EGUiWidgetInfo.reflection.h"

class EGFileData;

extern EGClass& EGUiWidget_GetClass();

struct egUiWidgetInfoEditNeeds
{
	eg_bool bRebuildEditor = false;
	eg_bool bRebuildPreview = false;
};

egreflect struct egUiWidgetInfo : public IListable
{
	static const eg_uint MAX_TEXTS_PER_OBJECT=8;
	static const eg_uint MAX_BONES_PER_OBJECT=8;

	typedef void ( * egFnQueryObject)( const egUiWidgetInfo* WidgetInfo , EGArray<eg_d_string>& ListOut );

	enum class eg_obj_t
	{
		CAMERA,
		CLEARZ,
		MESH,
		TEXT_NODE,
		LIGHT,
		IMAGE,
	};
	
	eg_obj_t Type;

	egprop eg_string_crc       Id                    = CT_Clear;
	egprop eg_widget_t         WidgetType            = eg_widget_t::NONE;
	egprop eg_bool             bIsLocked             = false;
	egprop eg_class_name       ClassName             = &EGUiWidget_GetClass();
	egprop eg_string_crc       EntDefCrc             = CT_Clear;
	egprop eg_string_crc       GridIdOfThisScrollbar = CT_Clear;
	egprop eg_transform        BasePose              = CT_Default;
	egprop eg_combo_box_crc_ed StartupEvent          = CT_Clear;
	egprop eg_vec3             ScaleVec              = eg_vec3(1.f,1.f,.1f);
	egprop eg_bool             IsAlwaysLoaded        = true;
	egprop eg_bool             IsLit                 = false;
	egprop eg_bool             IsGuide               = false;
	egprop eg_bool             bDrawAsMask           = false;
	egprop eg_bool             bDrawOnMaskOnly       = false;
	egprop eg_bool             bUsePointFiltering    = false;
	egprop eg_bool             bUseTextureEdgeClamp  = true;
	egprop eg_real             TrackExtend           = 0.f;
	egprop eg_anchor           Anchor                = eg_anchor( CT_Default );
	egprop egUiWidgetAdjacency    Adjacency;
	egprop egUiWidgetGridInfo     GridInfo;
	egprop egUiWidgetTextNodeInfo TextNodeInfo;
	egprop egLightInfo            LightInfo;
	egprop egCameraInfo           CameraInfo;
	egprop egImageInfo            ImageInfo;
	egprop EGArray<egTextOverrideInfo> TextOverrides;
	egprop EGArray<egBoneOverrideInfo> BoneOverrides;

	egRflEditor m_Editor;

	static egFnQueryObject QueryEvents;
	static egFnQueryObject QueryTextNodes;
	static egFnQueryObject QueryBones;
	static egFnQueryObject QueryPossibleScrollbars;

	egUiWidgetInfo( const egUiWidgetInfo& rhs ) = delete;
	egUiWidgetInfo( eg_camera_t CameraType );
	egUiWidgetInfo( eg_obj_t InType );
	~egUiWidgetInfo();

	void InitEditor();
	void DeinitEditor();

	static eg_string AnchorToString( eg_anchor_t Type );
	static void GetAnchorFromString( eg_cpstr Str , eg_anchor_t* OutX , eg_anchor_t* OutY );

	eg_bool IsGrid() const { return eg_obj_t::MESH == Type &&  WidgetType == eg_widget_t::GRID; }
	eg_bool IsButton() const { return eg_obj_t::MESH == Type && WidgetType == eg_widget_t::BUTTON; }
	eg_bool IsFocusableWidget() const { return IsGrid() || IsButton(); }
	eg_string GetToolDesc() const;
	void InitCameraType( eg_camera_t InType );

	egUiWidgetInfoEditNeeds OnEditPropertyChanged( const egRflEditor* ChangedProperty );
	void RefreshEditableProperties();
	void Serialize( EGFileData& File , eg_int Depth ) const;
	egRflEditor* GetEditor() { return &m_Editor; }

	void InitAlwaysLoaded();
	void DeinitAlwaysLoaded();
};
// (c) 2017 Beem Media

#pragma once

#include "EGUiTypes.h"
#include "EGEntTypes.h"
#include "EGFoundationTypes.h"
#include "EGClassName.h"
#include "EGAssetPath.h"
#include "EGEngineConfig.h"
#include "EGComboBoxEd.h"
#include "EGUiWidgetInfoTypes.reflection.h"

extern EGClass& EGUiGridWidget_GetClass();

static const eg_real MENU_ORTHO_SIZE = 100.0f;

egreflect struct egUiWidgetGridInfo
{
	egprop eg_class_name       ObjectClassName = &EGUiGridWidget_GetClass();
	egprop eg_int              Cols            = 1;
	egprop eg_int              Rows            = 5;
	egprop eg_combo_box_crc_ed ScrollbarId     = CT_Clear;
	egprop eg_grid_wrap_t      WrapType        = eg_grid_wrap_t::WRAP;
	egprop eg_bool             bAutoMask       = true;
};

egreflect struct egUiWidgetTextNodeInfo
{
	egprop eg_text_context  TextContext      = eg_text_context::None;
	egprop eg_string_crc    LocText          = CT_Clear;
	egprop eg_d_string_ml   LocText_enus     = "";
	egprop eg_asset_path    Font             = eg_asset_path_special_t::Font;
	egprop eg_real          Width            = 100.f;
	egprop eg_real          Height           = 10.f;
	egprop eg_int           NumLines         = 1;
	egprop eg_color32       Color            = eg_color32::White;
	egprop eg_bool          bHasDropShadow   = true;
	egprop eg_color32       DropShadowColor  = eg_color32::Black;
	egprop eg_text_align_ed Alignment        = eg_text_align_ed::LEFT;
};

egreflect struct egTextOverrideInfo
{
	egprop eg_combo_box_crc_ed NodeId    = CT_Clear;
	egprop eg_string_crc       Text      = CT_Clear;
	egprop eg_d_string_ml      Text_enus = "";
};

egreflect struct egBoneOverrideInfo
{
	egprop eg_combo_box_crc_ed BoneId    = CT_Clear;
	egprop eg_transform        Transform = CT_Default;

	operator egBoneOverride() const
	{
		egBoneOverride Out;
		Out.BoneId = BoneId;
		Out.Transform = Transform;
		return Out;
	}
};

egreflect struct egLightInfo
{
	egprop eg_color32 LightColor   = eg_color32::White;
	egprop eg_color32 AmbientColor = eg_color32::Black;
	egprop eg_real    Range        = 10.f;
	egprop eg_real    Falloff      = 5.f;
};

egreflect struct egCameraInfo
{
	egprop eg_camera_t_ed Type        = eg_camera_t_ed::ORTHOGRAPHIC;
	egprop eg_real        Near        = -MENU_ORTHO_SIZE;
	egprop eg_real        Far         = MENU_ORTHO_SIZE;
	egprop eg_real        FovDeg      = 90.f;
	egprop eg_real        OrthoRadius = MENU_ORTHO_SIZE;
};

egreflect struct egImageInfo
{
	egprop eg_asset_path  Texture = EXT_TEX;
	egprop eg_asset_path  Shader  = "evs5";
	egprop egPropMaterial Properties;

	egv_material AlwaysLoadedMaterial = egv_material::EGV_MATERIAL_NULL;
};

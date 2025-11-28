// (c) 2018 Beem Media

#include "EGReflectionPrimitives.h"
#include "EGEditControlType.h"
#include "EGCrcDb.h"

#define EGRFL_DECL_PRIM( _type_ , _editor_ ) const __EGRfl_##_type_ Rfl_##_type_; const egRflArrayType Rfl_Array_##_type_( Rfl_##_type_ ); eg_edit_ctrl_t __EGRfl_##_type_::GetEditControlType() const { return _editor_; }

EGRFL_DECL_PRIM( eg_bool             , eg_edit_ctrl_t::Bool )
EGRFL_DECL_PRIM( eg_int              , eg_edit_ctrl_t::Int )
EGRFL_DECL_PRIM( eg_uint             , eg_edit_ctrl_t::Int )
EGRFL_DECL_PRIM( eg_real             , eg_edit_ctrl_t::Real )
EGRFL_DECL_PRIM( eg_angle            , eg_edit_ctrl_t::Angle )
EGRFL_DECL_PRIM( eg_vec2             , eg_edit_ctrl_t::Vec2 )
EGRFL_DECL_PRIM( eg_vec3             , eg_edit_ctrl_t::Vec3 )
EGRFL_DECL_PRIM( eg_vec4             , eg_edit_ctrl_t::Vec4 )
EGRFL_DECL_PRIM( eg_ivec2            , eg_edit_ctrl_t::IntVec2 )
EGRFL_DECL_PRIM( eg_ivec3            , eg_edit_ctrl_t::IntVec3 )
EGRFL_DECL_PRIM( eg_ivec4            , eg_edit_ctrl_t::IntVec4 )
EGRFL_DECL_PRIM( eg_transform        , eg_edit_ctrl_t::Transform )
EGRFL_DECL_PRIM( eg_d_string         , eg_edit_ctrl_t::Text )
EGRFL_DECL_PRIM( eg_string_crc       , eg_edit_ctrl_t::Text )
EGRFL_DECL_PRIM( eg_color32          , eg_edit_ctrl_t::Color )
EGRFL_DECL_PRIM( eg_asset_path       , eg_edit_ctrl_t::Combo )
EGRFL_DECL_PRIM( eg_class_name       , eg_edit_ctrl_t::Combo )
EGRFL_DECL_PRIM( eg_combo_box_str_ed , eg_edit_ctrl_t::Combo )
EGRFL_DECL_PRIM( eg_combo_box_crc_ed , eg_edit_ctrl_t::Combo )
EGRFL_DECL_PRIM( eg_anchor           , eg_edit_ctrl_t::Anchor )

#undef EGRFL_DECL_PRIM

#define EGRFL_DECL_PRIM( _type_ ) const __EGRfl_##_type_ Rfl_##_type_; const egRflArrayType Rfl_Array_##_type_( Rfl_##_type_ ); 

EGRFL_DECL_PRIM( eg_d_string_ml )
EGRFL_DECL_PRIM( eg_os_browse_file_ed )
EGRFL_DECL_PRIM( eg_os_browse_dir_ed )
EGRFL_DECL_PRIM( eg_button_ed )
EGRFL_DECL_PRIM( eg_label_ed )

#undef EGRFL_DECL_PRIM

// (c) 2018 Beem Media

#pragma once

#include "EGAssetPath.h"
#include "EGClassName.h"
#include "EGComboBoxEd.h"
#include "EGReflection.h"
#include "EGReflectionStrConv.h"

template<typename T>
struct egRflRawValueType : public egRflValueType
{
	egRflRawValueType(){ }
	egRflRawValueType( const egRflRawValueType& rhs ) = delete;
	const egRflRawValueType& operator = ( const egRflRawValueType& rhs ) = delete;

	virtual eg_rfl_value_t GetType() const override { return eg_rfl_value_t::Primitive; }
	virtual eg_size_t GetPrimitiveSize() const override { return sizeof(T); }
	virtual void SetFromString( void* Data , eg_cpstr Str ) const  override
	{
		EGRflConv_SetFromString( reinterpret_cast<T*>(Data) , Str ); 
	}
	virtual eg_d_string ToString( const void* Data ) const override
	{ 
		return EGRflConv_ToString( reinterpret_cast<const T*>(Data) ); 
	}
	virtual void GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const override
	{
		return EGRflConv_GetComboChoices<T>( Data , Out , bManualEditOut );
	}
	virtual void PostLoad( void* Data , eg_cpstr8 Filename , eg_bool bForEditor ) const override
	{
		return EGRflConv_PostLoad<T>( Data , Filename , bForEditor );
	}
	virtual eg_edit_ctrl_t GetEditControlType() const override;
};

#define EGRFL_DECL_PRIM( _type_ ) typedef egRflRawValueType<_type_> __EGRfl_##_type_; extern const __EGRfl_##_type_ Rfl_##_type_; extern const egRflArrayType Rfl_Array_##_type_; static inline egRflEditor EGReflection_GetEditor( _type_& Var ,  eg_cpstr Name ) { return egRflEditor( Name , &Var , &Rfl_##_type_ ); }

EGRFL_DECL_PRIM( eg_bool )
EGRFL_DECL_PRIM( eg_int )
EGRFL_DECL_PRIM( eg_uint )
EGRFL_DECL_PRIM( eg_real )
EGRFL_DECL_PRIM( eg_angle )
EGRFL_DECL_PRIM( eg_vec2 )
EGRFL_DECL_PRIM( eg_vec3 )
EGRFL_DECL_PRIM( eg_vec4 )
EGRFL_DECL_PRIM( eg_ivec2 )
EGRFL_DECL_PRIM( eg_ivec3 )
EGRFL_DECL_PRIM( eg_ivec4 )
EGRFL_DECL_PRIM( eg_transform )
EGRFL_DECL_PRIM( eg_color32 )
EGRFL_DECL_PRIM( eg_d_string )
EGRFL_DECL_PRIM( eg_string_crc )
EGRFL_DECL_PRIM( eg_asset_path )
EGRFL_DECL_PRIM( eg_class_name )
EGRFL_DECL_PRIM( eg_anchor )
EGRFL_DECL_PRIM( eg_combo_box_str_ed )
EGRFL_DECL_PRIM( eg_combo_box_crc_ed )

#undef EGRFL_DECL_PRIM

#define EGRFL_DECL_STR_PRIM( _type_ , _editor_ ) class __EGRfl_##_type_ : public egRflRawValueType<_type_> { public: virtual eg_edit_ctrl_t GetEditControlType() const override { return _editor_; } }; extern const __EGRfl_##_type_ Rfl_##_type_; extern const egRflArrayType Rfl_Array_##_type_;

EGRFL_DECL_STR_PRIM( eg_d_string_ml , eg_edit_ctrl_t::BigText )
EGRFL_DECL_STR_PRIM( eg_os_browse_file_ed , eg_edit_ctrl_t::BrowseOpen )
EGRFL_DECL_STR_PRIM( eg_os_browse_dir_ed , eg_edit_ctrl_t::BrowseDir )
EGRFL_DECL_STR_PRIM( eg_button_ed , eg_edit_ctrl_t::Button )
EGRFL_DECL_STR_PRIM( eg_label_ed , eg_edit_ctrl_t::StaticText )

#undef EGRFL_DECL_STR_PRIM

// (c) 2018 Beem Media

#pragma once

#define EGRFL_DECL_STRUCT( _type_ , _cpptype_ ) \
egRflEditor EGReflection_GetEditor( _cpptype_ _type_& InData , eg_cpstr Name );\
struct __EGRfl_##_type_ : public egRflStructType \
{ \
	__EGRfl_##_type_():  egRflStructType( #_type_ ) { }\
	__EGRfl_##_type_( eg_cpstr InParentName ): egRflStructType( InParentName ) { }\
	virtual void GetChildren( EGArray<egRflVar>& Out ) const override;\
};\
extern const __EGRfl_##_type_ Rfl_##_type_; \
extern const egRflArrayType Rfl_Array_##_type_;


#define EGRFL_DECL_CHILD_STRUCT( _type_ , _parenttype_ , _cpptype_ ) \
egRflEditor EGReflection_GetEditor( _cpptype_ _type_& InData , eg_cpstr Name );\
struct __EGRfl_##_type_ : public __EGRfl_##_parenttype_ \
{ \
	__EGRfl_##_type_() : __EGRfl_##_parenttype_( #_type_ ) { }\
	__EGRfl_##_type_( eg_cpstr InParentName ): __EGRfl_##_parenttype_( InParentName )  { }\
	virtual void GetChildren( EGArray<egRflVar>& Out ) const override;\
}; \
extern const __EGRfl_##_type_ Rfl_##_type_; \
extern const egRflArrayType Rfl_Array_##_type_;

#define EGRFL_DECL_ENUM( _type_ )\
struct __EGRfl_##_type_ : public egRflEnumType \
{ \
	virtual void SetFromString( void* Data , eg_cpstr Str ) const override final;\
	virtual eg_d_string ToString( const void* Data ) const override final;\
	virtual eg_edit_ctrl_t GetEditControlType() const override final { return eg_edit_ctrl_t::Enum; }\
	virtual void GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const override final;\
	virtual eg_size_t GetPrimitiveSize() const override final;\
};\
extern const __EGRfl_##_type_ Rfl_##_type_;

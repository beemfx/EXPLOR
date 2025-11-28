// (c) 2018 Beem Media

#define EGRFL_IMPL_STRUCT_BEGIN( _type_ ) \
const __EGRfl_##_type_ Rfl_##_type_;\
egRflEditor EGReflection_GetEditor( _type_& InData , eg_cpstr Name ) { return egRflEditor( Name , &InData , &Rfl_##_type_ ); }\
const egRflArrayType Rfl_Array_##_type_( Rfl_##_type_ );\
void __EGRfl_##_type_::GetChildren( EGArray<egRflVar>& Out ) const { unused( Out );

#define EGRFL_IMPL_STRUCT_ITEM( _type_ , _vartype_ , _name_ ) Out.Append( egRflVar( #_name_ , offsetof(_type_,_name_) , Rfl_##_vartype_ ) );
#define EGRFL_IMPL_STRUCT_ITEM_ARRAY( _type_ , _vartype_ , _name_ ) Out.Append( egRflVar( #_name_ , offsetof(_type_,_name_) , Rfl_Array_##_vartype_ ) );

#define EGRFL_IMPL_STRUCT_END( _type_ ) }

#define EGRFL_IMPL_CHILD_STRUCT_BEGIN( _type_ , _parenttype_ ) \
const __EGRfl_##_type_ Rfl_##_type_;\
egRflEditor EGReflection_GetEditor( _type_& InData , eg_cpstr Name ) { return egRflEditor( Name , &InData , &Rfl_##_type_ ); }\
const egRflArrayType Rfl_Array_##_type_( Rfl_##_type_ );\
void __EGRfl_##_type_::GetChildren( EGArray<egRflVar>& Out ) const {\
		EGArray<egRflVar> ParentChildren;\
		__EGRfl_##_parenttype_::GetChildren( ParentChildren );\
		eg_intptr_t AdjustedOffset = reinterpret_cast<eg_intptr_t>(static_cast<_parenttype_*>(reinterpret_cast<_type_*>(0xF))) - reinterpret_cast<eg_intptr_t>(reinterpret_cast<_type_*>(0xF));\
		for( const egRflVar& Child : ParentChildren ){ Out.Append( egRflVar( Child.GetName() , Child.GetOffset() + AdjustedOffset , *Child.GetType() ) ); }\

#define EGRFL_IMPL_CHILD_STRUCT_END( _type_ , _parenttype_ ) }

#define EGRFL_IMPL_ENUM_BEGIN( _type_ ) static const struct egEnumTable_##_type_ { _type_ Value; eg_cpstr Name; } RflEnumTable_##_type_[] = {
#define EGRFL_IMPL_ENUM_ITEM( _type_ , _name_ ) { _type_::_name_ , #_name_ },
#define EGRFL_IMPL_ENUM_END( _type_ )\
};\
void __EGRfl_##_type_::SetFromString( void* Data , eg_cpstr Str ) const {\
	_type_* AsEnum = reinterpret_cast<_type_*>(Data);\
	for( const egEnumTable_##_type_& Entry : RflEnumTable_##_type_ ){ if( EGString_Equals( Str , Entry.Name ) ){ *AsEnum = Entry.Value; } }\
}\
eg_d_string __EGRfl_##_type_::ToString( const void* Data ) const {\
	const _type_* AsEnum = reinterpret_cast<const _type_*>(Data);\
	for( const egEnumTable_##_type_& Entry : RflEnumTable_##_type_ ){ if( *AsEnum == Entry.Value ){ return Entry.Name; } }\
	return "-invalid-";\
}\
void __EGRfl_##_type_::GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const {\
	unused( Data );\
	bManualEditOut = false;\
	for( const egEnumTable_##_type_& Entry : RflEnumTable_##_type_ ){ Out.Append( Entry.Name ); }\
}\
eg_size_t __EGRfl_##_type_::GetPrimitiveSize() const { return sizeof( _type_ ); }\
const __EGRfl_##_type_ Rfl_##_type_;

// Second pass implements everything
#define EGRFL_INCLUDE_SOURCE_HEADER
#include "EGReflection.h"
#include "EGReflectionPrimitives.h"
#include EGRFL_SYSTEM_HEADER
#undef EGRFL_INCLUDE_SOURCE_HEADER

#undef EGRFL_IMPL_STRUCT_BEGIN
#undef EGRFL_IMPL_STRUCT_ITEM
#undef EGRFL_IMPL_STRUCT_ITEM_ARRAY
#undef EGRFL_IMPL_STRUCT_END
#undef EGRFL_IMPL_CHILD_STRUCT_BEGIN
#undef EGRFL_IMPL_CHILD_STRUCT_END
#undef EGRFL_IMPL_ENUM_BEGIN
#undef EGRFL_IMPL_ENUM_ITEM
#undef EGRFL_IMPL_ENUM_END

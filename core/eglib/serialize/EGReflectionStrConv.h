// (c) 2018 Beem Media

#pragma once

struct eg_asset_path;
struct eg_class_name;
struct eg_anchor;
struct eg_combo_box_str_ed;
struct eg_combo_box_crc_ed;

static inline void EGRflConv_SetFromString( eg_bool* Var , eg_cpstr Str )
{
	*Var = eg_string_small(Str).ToBool();
}

static inline eg_d_string EGRflConv_ToString( const eg_bool* Var )
{
	return *Var ? "true" : "false";
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_int* Var , eg_cpstr Str )
{
	*Var = eg_string_small(Str).ToInt();
}

static inline eg_d_string EGRflConv_ToString( const eg_int* Var )
{
	return EGString_Format( "%i" , *Var ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_uint* Var , eg_cpstr Str )
{
	*Var = eg_string_small(Str).ToUInt();
}

static inline eg_d_string EGRflConv_ToString( const eg_uint* Var )
{
	return EGString_Format( "%u" , *Var ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_real* Var , eg_cpstr Str )
{
	*Var = eg_string_small(Str).ToFloat();
}

static inline eg_d_string EGRflConv_ToString( const eg_real* Var )
{
	return EGString_Format( "%g" , *Var ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_angle* Var , eg_cpstr Str )
{
	*Var = EG_Rad(eg_string_small(Str).ToFloat());
}

static inline eg_d_string EGRflConv_ToString( const eg_angle* Var )
{
	return EGString_Format( "%g" , Var->ToRad() ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_vec2* Var , eg_cpstr Str )
{
	eg_real Read[] = { 0.f , 0.f };
	EGString_GetFloatList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
}

static inline eg_d_string EGRflConv_ToString( const eg_vec2* Var )
{
	return EGString_Format( "%g %g" , Var->x , Var->y ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_vec3* Var , eg_cpstr Str )
{
	eg_real Read[] = { 0.f , 0.f , 0.f };
	EGString_GetFloatList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
	Var->z = Read[2];
}

static inline eg_d_string EGRflConv_ToString( const eg_vec3* Var )
{
	return EGString_Format( "%g %g %g" , Var->x , Var->y , Var->z ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_vec4* Var , eg_cpstr Str )
{
	eg_real Read[] = { 0.f , 0.f , 0.f , 0.f };
	EGString_GetFloatList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
	Var->z = Read[2];
	Var->w = Read[3];
}

static inline eg_d_string EGRflConv_ToString( const eg_vec4* Var )
{
	return EGString_Format( "%g %g %g %g" , Var->x , Var->y , Var->z , Var->w ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_ivec2* Var , eg_cpstr Str )
{
	eg_int Read[] = { 0 , 0 };
	EGString_GetIntList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
}

static inline eg_d_string EGRflConv_ToString( const eg_ivec2* Var )
{
	return EGString_Format( "%d %d" , Var->x , Var->y ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_ivec3* Var , eg_cpstr Str )
{
	eg_int Read[] = { 0 , 0 , 0 };
	EGString_GetIntList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
	Var->z = Read[2];
}

static inline eg_d_string EGRflConv_ToString( const eg_ivec3* Var )
{
	return EGString_Format( "%d %d %d" , Var->x , Var->y , Var->z ).String();
}

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_ivec4* Var , eg_cpstr Str )
{
	eg_int Read[] = { 0 , 0 , 0 , 0 };
	EGString_GetIntList( Str , EGString_StrLen(Str) , &Read , countof(Read) );
	Var->x = Read[0];
	Var->y = Read[1];
	Var->z = Read[2];
	Var->w = Read[3];
}

static inline eg_d_string EGRflConv_ToString( const eg_ivec4* Var )
{
	return EGString_Format( "%d %d %d %d" , Var->x , Var->y , Var->z , Var->w ).String();
}

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_transform* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_transform* Var );

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_color32* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_color32* Var );

///////////////////////////////////////////////////////////////////////////////

static inline void EGRflConv_SetFromString( eg_d_string* Var , eg_cpstr Str )
{
	*Var = Str;
}
static inline eg_d_string EGRflConv_ToString( const eg_d_string* Var )
{
	return *Var;
}

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_string_crc* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_string_crc* Var );

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_asset_path* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_asset_path* Var );
void EGRflConv_SetFromString( eg_class_name* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_class_name* Var );

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void EGRflConv_GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )
{
	unused( Data , Out );
	bManualEditOut = true;
}

template<>
void EGRflConv_GetComboChoices<eg_asset_path>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut );

template<>
void EGRflConv_GetComboChoices<eg_class_name>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut );

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void EGRflConv_PostLoad( void* Data , eg_cpstr8 Filename , eg_bool bForEditor )
{
	unused( Data , Filename , bForEditor );
}

template<>
void EGRflConv_PostLoad<eg_asset_path>( void* Data , eg_cpstr8 Filename , eg_bool bForEditor );

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_anchor* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_anchor* Var );

///////////////////////////////////////////////////////////////////////////////

void EGRflConv_SetFromString( eg_combo_box_str_ed* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_combo_box_str_ed* Var );
template<> void EGRflConv_GetComboChoices<eg_combo_box_str_ed>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut );

void EGRflConv_SetFromString( eg_combo_box_crc_ed* Var , eg_cpstr Str );
eg_d_string EGRflConv_ToString( const eg_combo_box_crc_ed* Var );
template<> void EGRflConv_GetComboChoices<eg_combo_box_crc_ed>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut );

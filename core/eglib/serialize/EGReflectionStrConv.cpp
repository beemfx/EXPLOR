// (c) 2018 Beem Media

#include "EGReflectionStrConv.h"
#include "EGBase64.h"
#include "EGCrcDb.h"
#include "EGAssetPath.h"
#include "EGClassName.h"
#include "EGPath2.h"
#include "EGComboBoxEd.h"

void EGRflConv_SetFromString( eg_transform* Var, eg_cpstr Str )
{
	*Var = CT_Default;

	if( EGString_EqualsCount( Str , "RqTS1:" , 6 ) )
	{
		eg_real Read[] = { 0.f , 0.f , 0.f , 0.f , 0.f , 0.f , 0.f , 0.f };
		EGString_GetFloatList( &Str[6] , 0 , &Read , countof(Read) );
		Var->SetRotation( eg_quat( Read[0] , Read[1] , Read[2] , Read[3] ) );
		Var->SetTranslation( eg_vec3( Read[4] , Read[5] , Read[6] ) );
		Var->SetScale( Read[7] );
		Var->NormalizeRotationOfThis();
	}
	else if( EGString_EqualsCount( Str , "B64:" , 4 ) )
	{
		EGBase64_Decode( &Str[4] , Var , sizeof(*Var) );
	}
}

eg_d_string EGRflConv_ToString( const eg_transform* Var )
{
	eg_bool bUseHumanReadable = true;

	if( bUseHumanReadable )
	{
		return EGString_Format( "RqTS1:%g %g %g %g , %g %g %g , %g"
			, Var->GetRotation().x, Var->GetRotation().y, Var->GetRotation().z, Var->GetRotation().w
			, Var->GetTranslation().x, Var->GetTranslation().y, Var->GetTranslation().z
			, Var->GetScale() ).String();
	}

	return EGString_Format( "B64:%s", *EGBase64_Encode( Var, sizeof( *Var ) ) ).String();
}

void EGRflConv_SetFromString( eg_string_crc* Var, eg_cpstr Str )
{
	*Var = EGCrcDb::StringToCrc( Str );
}

eg_d_string EGRflConv_ToString( const eg_string_crc* Var )
{
	return EGCrcDb::CrcToString( *Var ).String();
}

void EGRflConv_SetFromString( eg_color32* Var, eg_cpstr Str )
{
	eg_color Out(0.f,0.f,0.f,1.f);

	if( EGString_EqualsCount( Str , "B64:" , 4 ) )
	{
		EGString_GetFloatList( &Str[4] , 0 , &Out , 4 , true );
	}
	else if( EGString_EqualsCount( Str , "RGBA:" , 5 ) )
	{
		EGString_GetFloatList( &Str[5] , 0 , &Out , 4 , false );
	}
	else if( EGString_EqualsCount( Str , "ARGB32:" , 7 ) )
	{
		eg_int Int32Color[4] = { 255 , 255 , 255 , 255 };
		EGString_GetIntList( &Str[7] , 0 , &Int32Color , 4 , false );
		Out.a = EG_Clamp( Int32Color[0]/255.f , 0.f , 1.f );
		Out.r = EG_Clamp( Int32Color[1]/255.f , 0.f , 1.f );
		Out.g = EG_Clamp( Int32Color[2]/255.f , 0.f , 1.f );
		Out.b = EG_Clamp( Int32Color[3]/255.f , 0.f , 1.f );
	}
	else
	{
		// If the type wasn't specified assume %g
		EGString_GetFloatList( Str , 0 , &Out , 4 , false );
	}

	*Var = eg_color32(Out);
}

eg_d_string EGRflConv_ToString( const eg_color32* Var )
{
	eg_d_string Out;

	eg_bool bBase64 = false;
	eg_bool bFloat = false;

	if( bBase64 )
	{
		Out = EGString_Format( "B64:%s" , *EGBase64_Encode( Var , sizeof(*Var) ) );
	}
	else if( bFloat )
	{
		Out = EGString_Format( "RGBA:%g %g %g %g" , Var->R/255.f , Var->G/255.f , Var->B/255.f , Var->A/255.f );
	}
	else
	{
		Out = EGString_Format( "ARGB32:%u %u %u %u" 	, Var->A , Var->R , Var->G , Var->B );
	}

	return Out;
}

void EGRflConv_SetFromString( eg_asset_path* Var, eg_cpstr Str )
{
	Var->Path = Str;
	Var->FullPath = "";
}

eg_d_string EGRflConv_ToString( const eg_asset_path* Var )
{
	return Var->Path;
}

void EGRflConv_SetFromString( eg_class_name* Var , eg_cpstr Str )
{
	Var->Class = EGClass::FindClass( Str );
}

eg_d_string EGRflConv_ToString( const eg_class_name* Var )
{
	return Var->Class ? Var->Class->GetName() : "";
}

template<>
void EGRflConv_GetComboChoices<eg_asset_path>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )
{
	bManualEditOut = true;
	const eg_asset_path* AssetPath = reinterpret_cast<const eg_asset_path*>(Data);
	AssetPath->GetComboChoices( Out );
}

template<>
void EGRflConv_GetComboChoices<eg_class_name>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )
{
	bManualEditOut = false;
	const eg_class_name* ClassName = reinterpret_cast<const eg_class_name*>(Data);
	ClassName->GetComboChoices( Out );
}

template<>
void EGRflConv_PostLoad<eg_asset_path>( void* Data , eg_cpstr8 Filename , eg_bool bForEditor )
{
	unused( bForEditor );

	eg_asset_path* AssetPath = reinterpret_cast<eg_asset_path*>(Data);
	AssetPath->FullPath = *EGPath2_GetFullPathRelativeTo( *AssetPath->Path , Filename );
}

void EGRflConv_SetFromString( eg_anchor* Var , eg_cpstr Str )
{
	Var->X = eg_anchor_t::CENTER;
	Var->Y = eg_anchor_t::CENTER;

	eg_string_big TempStr = Str;

	if( TempStr.Len() > 0 )
	{
		TempStr.ConvertToUpper();

		// X
		if( TempStr.Contains( "RIGHT" ) )
		{
			Var->X = eg_anchor_t::RIGHT;
		}
		else if( TempStr.Contains( "LEFT" ) )
		{
			Var->X = eg_anchor_t::LEFT;
		}

		// Y
		if( TempStr.Contains( "TOP" ) )
		{
			Var->Y = eg_anchor_t::TOP;
		}
		else if( TempStr.Contains( "BOTTOM" ) )
		{
			Var->Y = eg_anchor_t::BOTTOM;
		}
	}
}

eg_d_string EGRflConv_ToString( const eg_anchor* Var )
{
	auto AnchorToString = []( eg_anchor_t Type ) -> eg_string_small
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
	};

	return EGString_Format( "%s|%s" , AnchorToString( Var->X ).String() , AnchorToString( Var->Y ).String() ).String();
}

void EGRflConv_SetFromString( eg_combo_box_str_ed* Var , eg_cpstr Str )
{
	Var->String = Str;
}

eg_d_string EGRflConv_ToString( const eg_combo_box_str_ed* Var )
{
	return Var->String;
}

template<> void EGRflConv_GetComboChoices<eg_combo_box_str_ed>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )
{
	const eg_combo_box_str_ed* Var = reinterpret_cast<const eg_combo_box_str_ed*>(Data);
	if( Var->PopulateCb )
	{
		Var->PopulateCb( Out , bManualEditOut );
	}
	else
	{
		Out.Append( "" );
		bManualEditOut = true;
	}
}

void EGRflConv_SetFromString( eg_combo_box_crc_ed* Var , eg_cpstr Str )
{
	Var->Crc = EGCrcDb::StringToCrc( Str );
}

eg_d_string EGRflConv_ToString( const eg_combo_box_crc_ed* Var )
{
	return EGCrcDb::CrcToString( Var->Crc ).String();
}

template<> void EGRflConv_GetComboChoices<eg_combo_box_crc_ed>( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut )
{
	const eg_combo_box_crc_ed* Var = reinterpret_cast<const eg_combo_box_crc_ed*>(Data);
	if( Var->PopulateCb )
	{
		Var->PopulateCb( Out , bManualEditOut );
	}
	else
	{
		Out.Append( "" );
		bManualEditOut = true;
	}
}
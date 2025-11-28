// (c) 2018 Beem Media

#include "EGSettings2Types.h"
#include "EGTextFormat.h"

EGSettingsBool::EGSettingsBool( eg_cpstr VarName , eg_string_crc DisplayName , eg_bool DefaultValue , eg_flags VarFlags )
: EGSettingsVarType<eg_bool>( VarName , DisplayName , DefaultValue, VarFlags )
{

}

void EGSettingsBool::SetFromString( eg_cpstr NewValue )
{
	SetValue( EGString_ToBool( NewValue ) );
}

eg_d_string EGSettingsBool::ToString() const
{
	eg_bool Out = GetValueThreadSafe();
	return Out ? "true" : "false";
}

eg_loc_text EGSettingsBool::ToLocText() const
{
	return eg_loc_text(GetValueThreadSafe() ? eg_loc("EGSettings2True","On") : eg_loc("EGSettings2False","Off"));
}

void EGSettingsBool::Inc()
{
	SetValue( !GetValueThreadSafe() );
}

void EGSettingsBool::Dec()
{
	Inc();
}

EGSettingsInt::EGSettingsInt( eg_cpstr VarName, eg_string_crc DisplayName, eg_int DefaultValue, eg_flags VarFlags )
: EGSettingsVarType<eg_int>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void EGSettingsInt::SetFromString( eg_cpstr NewValue )
{
	SetValue( EGString_ToInt( NewValue ) );
}

eg_d_string EGSettingsInt::ToString() const
{
	eg_int Out = GetValueThreadSafe();
	return EGString_Format( "%d", Out ).String();
}

eg_loc_text EGSettingsInt::ToLocText() const
{
	return eg_loc_text( eg_d_string16( *ToString() ).AsCStr() );
}

void EGSettingsInt::Inc()
{
	SetValue( GetValueThreadSafe() + 1 );
}

void EGSettingsInt::Dec()
{
	SetValue( GetValueThreadSafe() - 1 );
}

EGSettingsReal::EGSettingsReal( eg_cpstr VarName, eg_string_crc DisplayName, eg_real DefaultValue, eg_flags VarFlags )
: EGSettingsVarType<eg_real>( VarName, DisplayName, DefaultValue, VarFlags )
{

}

void EGSettingsReal::SetFromString( eg_cpstr NewValue )
{
	SetValue( EGString_ToReal( NewValue ) );
}

eg_d_string EGSettingsReal::ToString() const
{
	eg_real Out = GetValueThreadSafe();
	return EGString_Format( "%g", Out ).String();
}

eg_loc_text EGSettingsReal::ToLocText() const
{
	return eg_loc_text( eg_d_string16( *ToString() ).AsCStr() );
}

void EGSettingsReal::Inc()
{
	SetValue( GetValueThreadSafe() + .1f );
}

void EGSettingsReal::Dec()
{
	SetValue( GetValueThreadSafe() - .1f );
}

EGSettingsString::EGSettingsString( eg_cpstr VarName, eg_string_crc DisplayName, eg_cpstr DefaultValue, eg_flags VarFlags )
: EGSettingsVarType<eg_s_string_sml8>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void EGSettingsString::SetFromString( eg_cpstr NewValue )
{
	SetValue( NewValue );
}

eg_d_string EGSettingsString::ToString() const
{
	eg_d_string Out = *GetValueThreadSafe();
	return Out;
}

eg_loc_text EGSettingsString::ToLocText() const
{
	return eg_loc_text( eg_d_string16( *ToString() ).AsCStr() );
}

void EGSettingsString::Inc()
{
	
}

void EGSettingsString::Dec()
{
	
}

EGSettingsVolume::EGSettingsVolume( eg_cpstr VarName, eg_string_crc DisplayName, eg_int DefaultValue, eg_flags VarFlags )
: EGSettingsInt( VarName , DisplayName , DefaultValue , VarFlags )
{

}

eg_loc_text EGSettingsVolume::ToLocText() const
{
	return EGFormat( L"{0}" , GetValueThreadSafe() );
}

void EGSettingsVolume::Inc()
{
	SetValue( EG_Clamp( GetValueThreadSafe() + m_Inc , m_Min , m_Max ) );
}

void EGSettingsVolume::Dec()
{
	SetValue( EG_Clamp( GetValueThreadSafe() - m_Inc , m_Min , m_Max ) );
}

eg_bool EGSettingsVolume::CanInc() const
{
	return GetValueThreadSafe() < m_Max;
}

eg_bool EGSettingsVolume::CanDec() const
{
	return GetValueThreadSafe() > m_Min;
}

eg_real EGSettingsVolume::GetAudioValue() const
{
	const eg_real LinearValue = EGMath_GetMappedRangeValue( static_cast<eg_real>(GetValueThreadSafe()) , eg_vec2(EG_To<eg_real>(m_Min),EG_To<eg_real>(m_Max)) , eg_vec2(0.f,1.f) );
	const eg_real VolumeValue = EGMath_VolumeFromLinear( LinearValue );
	return VolumeValue;
}

EGSettingsClampedReal::EGSettingsClampedReal( eg_cpstr VarName, eg_string_crc DisplayName, eg_real DefaultValue, eg_flags VarFlags, eg_real InMin, eg_real InMax, eg_real InInc )
: EGSettingsReal( VarName , DisplayName , DefaultValue , VarFlags )
, m_Min( InMin )
, m_Max( InMax )
, m_Inc( InInc )
{

}

void EGSettingsClampedReal::Inc()
{
	SetValue( EG_Clamp( GetValueThreadSafe() + m_Inc , m_Min , m_Max ) );
}

void EGSettingsClampedReal::Dec()
{
	SetValue( EG_Clamp( GetValueThreadSafe() - m_Inc , m_Min , m_Max ) );
}

EGSettingsClampedInt::EGSettingsClampedInt( eg_cpstr VarName, eg_string_crc DisplayName, eg_int DefaultValue, eg_flags VarFlags, eg_int InMin, eg_int InMax, eg_int InInc )
: EGSettingsInt( VarName , DisplayName , DefaultValue , VarFlags )
, m_Min( InMin )
, m_Max( InMax )
, m_Inc( InInc )
{

}

void EGSettingsClampedInt::Inc()
{
	SetValue( EG_Clamp( GetValueThreadSafe() + m_Inc , m_Min , m_Max ) );
}

void EGSettingsClampedInt::Dec()
{
	SetValue( EG_Clamp( GetValueThreadSafe() - m_Inc , m_Min , m_Max ) );
}

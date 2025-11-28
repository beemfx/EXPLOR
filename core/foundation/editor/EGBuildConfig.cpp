// (c) 2018 Beem Media

#include "EGBuildConfig.h"
#include "EGFileData.h"
#include "EGPath2.h"
#include "EGLibFile.h"

void EGBuildConfig::LoadConfig( eg_cpstr16 Filename )
{
	EGFileData ConfigFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bOpenedFile = EGLibFile_OpenFile( Filename , eg_lib_file_t::OS , ConfigFile );
	if( bOpenedFile )
	{
		m_bConfigWasLoaded = true;
		XMLLoad( ConfigFile.GetData() , ConfigFile.GetSize() , *EGPath2_GetFilename( EGString_ToMultibyte(Filename) , false ) );
	}
}

void EGBuildConfig::ClearConfig()
{
	m_Vars.Clear();
}

const eg_d_string& EGBuildConfig::GetConfigValue( eg_cpstr VarName ) const
{
	for( const egBuildConfigVar& Var : m_Vars )
	{
		if( Var.VarName.EqualsI( VarName ) )
		{
			return Var.ConfigValue;
		}
	}

	return m_NotFoundString;
}

void EGBuildConfig::SetConfigValue( eg_cpstr VarName, eg_cpstr VarValue )
{
	for( egBuildConfigVar& Var : m_Vars )
	{
		if( Var.VarName.EqualsI( VarName ) )
		{
			Var.ConfigValue = VarValue;
			return;
		}
	}

	// If we made it here it's not added yet.
	egBuildConfigVar NewVar;
	NewVar.VarName = VarName;
	NewVar.ConfigValue = VarValue;
	m_Vars.Append( NewVar );
}

void EGBuildConfig::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	unused( Tag , AttGet );
}

void EGBuildConfig::OnTagEnd( const eg_string_base& Tag )
{
	unused( Tag );
}

void EGBuildConfig::OnData( eg_cpstr DataStr , eg_uint DataLen )
{
	unused( DataLen );

	if( GetXmlTagLevels() == 2 )
	{
		SetConfigValue( GetXmlTagUp( 0 ) , DataStr );
	}
}

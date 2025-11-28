// (c) 2018 Beem Media

#pragma once

#include "EGXMLBase.h"

struct egBuildConfigVar
{
	eg_d_string VarName;
	eg_d_string ConfigValue;
};

class EGBuildConfig : public IXmlBase
{
private:

	EGArray<egBuildConfigVar> m_Vars;
	const eg_d_string m_NotFoundString = CT_Clear;
	eg_bool m_bConfigWasLoaded = false;

public:

	EGBuildConfig() = default;
	EGBuildConfig( eg_cpstr16 Filename )
	: EGBuildConfig()
	{
		LoadConfig( Filename );
	}
	EGBuildConfig( eg_cpstr8 Filename )
	: EGBuildConfig( EGString_ToWide(Filename) )
	{
	}

	void LoadConfig( eg_cpstr16 Filename );
	eg_bool HasConfig() const { return m_bConfigWasLoaded; }
	void ClearConfig();
	const eg_d_string& GetConfigValue( eg_cpstr VarName ) const;

private:

	void SetConfigValue( eg_cpstr VarName , eg_cpstr VarValue );

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) override;
	virtual void OnTagEnd( const eg_string_base& Tag ) override;
	virtual void OnData( eg_cpstr DataStr , eg_uint DataLen ) override;
	virtual eg_cpstr XMLObjName() const override { return "EGBuildConfig"; }
};

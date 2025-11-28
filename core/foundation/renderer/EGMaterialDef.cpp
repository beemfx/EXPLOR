#include "EGRendererTypes.h"
#include "EGXMLBase.h"

EGMaterialDef::EGMaterialDef()
{
	Reset();
}

EGMaterialDef::EGMaterialDef( eg_ctor_t Ct )
{
	if( Ct == CT_Default )
	{
		zero( this );
		Reset();
	}
	else if( Ct == CT_Clear )
	{
		zero( this );
	}
}

void EGMaterialDef::MakePathsRelativeTo(eg_cpstr strFile)
{
	const eg_string_small sF(strFile);

	for(eg_uint i=0; i<MAX_TEX; i++)
	{
		eg_string_small s(m_strTex[i]);
		s.MakeThisFilenameRelativeTo(sF);
		if(s.Len()>=EG_MAX_PATH)EGLogf( eg_log_t::Error , __FUNCTION__ ": \"%s\" is too long.", sF.String());
		zero( &m_strTex[i] );
		s.CopyTo(m_strTex[i], EG_MAX_PATH);
	}

	eg_string_small sV(m_strVS);
	eg_string_small sP(m_strPS);
	sV.MakeThisFilenameRelativeTo(sF);
	sP.MakeThisFilenameRelativeTo(sF);
	if(sV.Len()>=EG_MAX_PATH)EGLogf(eg_log_t::Error , __FUNCTION__ ": \"%s\" is too long.", sV.String());
	if(sP.Len()>=EG_MAX_PATH)EGLogf(eg_log_t::Error , __FUNCTION__ ": \"%s\" is too long.", sP.String());
	zero( &m_strVS );
	zero( &m_strPS );
	sV.CopyTo(m_strVS, EG_MAX_PATH);
	sP.CopyTo(m_strPS, EG_MAX_PATH);
}

void EGMaterialDef::SetShader( eg_cpstr Shader )
{
	zero( &m_strVS );
	zero( &m_strPS );
	EGString_Copy( m_strVS , Shader , countof(m_strVS) );
	EGString_Copy( m_strPS , Shader , countof(m_strPS) );
}

void EGMaterialDef::Reset()
{
	//Create  basic white material
	zero( &m_Mtr );
	m_Mtr.Diffuse = eg_color(1.f,1.f,1.f,1.f);
	m_Mtr.Ambient = eg_color(0,0,0,1.f);
	m_Mtr.Specular = eg_color(0,0,0,1.f);
	m_Mtr.Emissive = eg_color(0,0,0,1.f);
	m_Mtr.Power = 1.0f;

	for(eg_uint i=0; i<MAX_TEX; i++)
	{
		zero( &m_strTex[i] );
	}

	zero( &m_strVS );
	zero( &m_strPS );
}

eg_string_big EGMaterialDef::CreateXmlTag( eg_uint Id )
{
	eg_string_big StrOut = EGString_Format( "\t<material id=\"%u\"" , Id );

	for( eg_uint i=0; i<countof(m_strTex); i++ )
	{
		if( m_strTex[i][0] != '\0' )
		{
			StrOut.Append( EGString_Format( " tex%u=\"%s\"" , i , m_strTex[i] ) );
		}
	}

	static_assert( countof(m_strPS) == countof(m_strVS) , "Wrong array size" );


	if( EGString_EqualsI( m_strPS , m_strVS ) && m_strPS[0] != '\0' )
	{
		StrOut.Append( EGString_Format( " shader=\"%s\"" , m_strPS ) );
	}
	else if( m_strPS[0] == '\0' )
	{
		//Just skip over it if it wasn't set.
	}
	else
	{
		//Not sure what we should do here. Print a warning maybe.
	}

	#define APPEND_COLOR( type , value ) StrOut.Append( EGString_Format( " " type "=\"%g %g %g %g\"" , value.r , value.g , value.b , value.a ) );	
	APPEND_COLOR( "diffuse"  , m_Mtr.Diffuse );
	APPEND_COLOR( "specular" , m_Mtr.Specular );
	APPEND_COLOR( "emissive" , m_Mtr.Emissive );
	APPEND_COLOR( "ambient"  , m_Mtr.Ambient );
	#undef APPEND_COLOR

	StrOut.Append( EGString_Format( " power=\"%g\"" , m_Mtr.Power ) );
	
	StrOut.Append( " />\r\n" );

	return StrOut;
}

void EGMaterialDef::SetFromTag( const class EGXmlAttrGetter& Getter )
{
	Reset();
	
	Getter.GetString( "tex0" ).CopyTo( m_strTex[0] , countof(m_strTex[0]) );
	Getter.GetString( "tex1" ).CopyTo( m_strTex[1] , countof(m_strTex[1]) );
	Getter.GetString( "tex2" ).CopyTo( m_strTex[2] , countof(m_strTex[2]) );
	Getter.GetString( "tex3" ).CopyTo( m_strTex[3] , countof(m_strTex[3]) );

	if( Getter.DoesAttributeExist( "shader" ) )
	{
		eg_string_small val = Getter.GetString( "shader" );
		val.CopyTo(m_strVS, countof(m_strVS));
		val.CopyTo(m_strPS, countof(m_strPS));
	}

	if( Getter.DoesAttributeExist( "diffuse" )  )Getter.GetString( "diffuse"  , "1 1 1 1" ).ToRealArray( reinterpret_cast<eg_real*>(&m_Mtr.Diffuse)  , 4 );
	if( Getter.DoesAttributeExist( "ambient" )  )Getter.GetString( "ambient"  , "1 1 1 1" ).ToRealArray( reinterpret_cast<eg_real*>(&m_Mtr.Ambient)  , 4 );
	if( Getter.DoesAttributeExist( "specular" ) )Getter.GetString( "specular" , "1 1 1 1" ).ToRealArray( reinterpret_cast<eg_real*>(&m_Mtr.Specular) , 4 );
	if( Getter.DoesAttributeExist( "emissive" ) )Getter.GetString( "emissive" , "1 1 1 1" ).ToRealArray( reinterpret_cast<eg_real*>(&m_Mtr.Emissive) , 4 );
	
	m_Mtr.Power = Getter.GetString( "power" ).ToFloat();
}

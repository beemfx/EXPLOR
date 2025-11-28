// (c) 2018 Beem Media

#pragma once

#include "EGXMLBase.h"

class EGEdCubeMapDescFile : public IXmlBase
{
public:

	EGEdCubeMapDescFile( eg_cpstr Filename );

	eg_bool IsLoaded()const{ return m_IsLoaded; }

	eg_cpstr GetXPos(){ return *m_XPos; }
	eg_cpstr GetXNeg(){ return *m_XNeg; }
	eg_cpstr GetYPos(){ return *m_YPos; }
	eg_cpstr GetYNeg(){ return *m_YNeg; }
	eg_cpstr GetZPos(){ return *m_ZPos; }
	eg_cpstr GetZNeg(){ return *m_ZNeg; }
	eg_cpstr GetOpts(){ return *m_Opts; }

private:

	eg_d_string m_XPos;
	eg_d_string m_XNeg;
	eg_d_string m_YPos;
	eg_d_string m_YNeg;
	eg_d_string m_ZPos;
	eg_d_string m_ZNeg;
	eg_d_string m_Opts;

	eg_bool m_IsLoaded:1;

private:

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Getter ) override;
	virtual eg_cpstr XMLObjName()const override{ return "EGEdCubeMapDescFile"; }
	void OnTag_ecube( const EGXmlAttrGetter& AttGet );
};

eg_bool EGEdCubeMapBuild_Build( eg_cpstr SourceFile , eg_cpstr DestFile );
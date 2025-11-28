// (c) 2018 Beem Media

#pragma once

#include "EGGcxRegion.h"
#include "EGXMLBase.h"

struct gcxSaveProps
{
	eg_d_string DefaultTileLib = "";
	eg_d_string OutputDir = "";
	eg_d_string BuildConfigFile = "";
};

class EGGcxFile : private IXmlBase
{
public:

	EGArray<EGGcxRegion>     m_Regions;
	EGArray<gcxCustomTile>   m_CustomTiles;
	eg_bool                  m_Failed:1;
	eg_bool                  m_bReadingRegion:1;
	eg_int                   m_CurRegionIndex;
	eg_d_string              m_SourceDir;

public:

	EGGcxFile( eg_cpstr Filename , eg_cpstr SrcDir );
	~EGGcxFile();

	eg_bool SaveAsXmlEmap( EGGcxSaveCb& SaveFn , EGGcxWriteMtrlCb& WriteMtrlFn , EGGcxProcessPropsCb& ProcessPropsCb , const gcxSaveProps& SaveProps , eg_bool bCompress );
	void Clear();

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& Atts) override;
	virtual void OnTagEnd( const eg_string_base& Tag ) override final;
	virtual void OnData( eg_cpstr Data , eg_uint Len ) override;
	virtual eg_cpstr XMLObjName()const override{ return "EGGcxCompiler"; }
	eg_bool IsLoaded()const{ return !m_Failed; }
	void LoadBuildInfoForRegion( const EGGcxRegion& Region );

private:

	static gcx_terrain_t CustomTileFileToType( eg_cpstr File );
};


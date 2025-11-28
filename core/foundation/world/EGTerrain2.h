// (c) 2018 Beem Media

#pragma once

struct EGMaterialDef;
class EGFileData;

class EGTerrain2
{
public:

	static const eg_uint ID      = 0x52455445; //("ETER")
	static const eg_uint VERSION = 2;

	struct egHeader
	{
		eg_uint  Id        = 0;
		eg_uint  Version   = 0;
		eg_ivec2 HfDims    = CT_Clear;
		eg_vec2  WorldDims = CT_Clear;
		eg_real  WorldScale = 1.f;
	};

protected:

	egHeader             m_Header;
	EGArray<eg_byte>     m_Mem;
	const EGMaterialDef* m_Material    = nullptr;
	const eg_real*       m_HeightField = nullptr;
	LOAD_S               m_LoadState = LOAD_NOT_LOADED;

public:

	void LoadData( const EGFileData& MemFile , eg_cpstr RefFilename );
	eg_bool IsValid() const { return m_Header.Id == ID && m_Header.Version == VERSION; }
	LOAD_S GetLoadState() const { return m_LoadState; }

	// These functions are as if standing facing North (Positive z direction (LH)) and then looking down.
	eg_real GetXMin()const;
	eg_real GetZMin()const;
	eg_real GetXMax()const;
	eg_real GetZMax()const;
	egv_vert_terrain GetVertexAt( eg_real x , eg_real z ) const;
	const EGMaterialDef& GetMaterialDef() const { return *m_Material; }
	eg_real GetHeightAt( eg_real x , eg_real z ) const;

protected:

	eg_real GetSampleAtIndex( eg_int x , eg_int z ) const;
	static eg_string BuildFinalFilename( eg_cpstr Filename );
};

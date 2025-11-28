// (c) 2018 Beem Media

#include "EGTerrain2.h"
#include "EGFileData.h"
#include "EGRendererTypes.h"

void EGTerrain2::LoadData( const EGFileData& MemFile, eg_cpstr RefFilename )
{
	unused( RefFilename );

	m_Mem.Clear();
	m_Header = egHeader();
	m_Material = nullptr;
	m_HeightField = nullptr;

	if( MemFile.GetSize() >= (sizeof(egHeader)+sizeof(EGMaterialDef)) )
	{
		m_Mem.Append( MemFile.GetDataAs<eg_byte>() , MemFile.GetSize() );
		m_Header = *reinterpret_cast<egHeader*>(&m_Mem[0]);
		m_Material = reinterpret_cast<EGMaterialDef*>(&m_Mem[sizeof(egHeader)]);
		m_HeightField = reinterpret_cast<eg_real*>(&m_Mem[sizeof(egHeader)+sizeof(EGMaterialDef)]);

		const_cast<EGMaterialDef*>(m_Material)->MakePathsRelativeTo( RefFilename );
	}
}

eg_real EGTerrain2::GetXMin() const
{
	return -m_Header.WorldDims.x*.5f*m_Header.WorldScale;
}

eg_real EGTerrain2::GetZMin() const
{
	return -m_Header.WorldDims.y*.5f*m_Header.WorldScale;
}

eg_real EGTerrain2::GetXMax() const
{
	return m_Header.WorldDims.x*.5f*m_Header.WorldScale;
}

eg_real EGTerrain2::GetZMax() const
{
	return m_Header.WorldDims.y*.5f*m_Header.WorldScale;
}

egv_vert_terrain EGTerrain2::GetVertexAt( eg_real x , eg_real z ) const
{
	eg_real xRealIndex = EGMath_GetMappedRangeValue( x , eg_vec2(GetXMin(),GetXMax()), eg_vec2(0, static_cast<eg_real>(m_Header.HfDims.x-1)));
	eg_real zRealIndex = EGMath_GetMappedRangeValue( z , eg_vec2(GetZMin(),GetZMax()), eg_vec2(0, static_cast<eg_real>(m_Header.HfDims.y-1)));

	eg_real Height = GetHeightAt( x , z );

	egv_vert_terrain Out;
	zero(&Out);

	const eg_real REPEAT = 10.0f;

	const eg_vec2 tu
	(
		xRealIndex/(m_Header.HfDims.x-1),
		zRealIndex/(m_Header.HfDims.y-1)
	);

	Out.Pos = eg_vec4( x , Height , z , 1.f );
	Out.Norm = eg_vec4( 0 , 1.f , 0 , 0 );
	Out.Tan = eg_vec4( 1.f , 0 , 0 , 0 );
	Out.Tex0 = eg_vec2(tu.x*REPEAT,tu.y*REPEAT);
	Out.Tex1 = eg_vec2(tu.x,tu.y);
	// Out.Color0 = eg_color(1,1,1,1);
	// Out.Weight0 = 1.f;

	// read neighbor heights using an arbitrary small offset
	eg_real off = .5f;
	eg_real hL = GetHeightAt( x - off , z );
	eg_real hR = GetHeightAt( x + off , z );
	eg_real hD = GetHeightAt( x , z - off );
	eg_real hU = GetHeightAt( x , z + off );

	// deduce terrain normal
	Out.Norm = eg_vec4( hL - hR , off*2.f , hD - hU , 0.f );
	Out.Norm.NormalizeThisAsVec3();

	return Out;
}

eg_real EGTerrain2::GetHeightAt( eg_real x , eg_real z ) const
{
	eg_real xRealIndex = EGMath_GetMappedRangeValue( x , eg_vec2(GetXMin(),GetXMax()), eg_vec2(0, static_cast<eg_real>(m_Header.HfDims.x-1)));
	eg_real zRealIndex = EGMath_GetMappedRangeValue( z , eg_vec2(GetZMin(),GetZMax()), eg_vec2(0, static_cast<eg_real>(m_Header.HfDims.y-1)));
	xRealIndex = EG_Clamp( xRealIndex , 0.f , static_cast<eg_real>(m_Header.HfDims.x-1) );
	zRealIndex = EG_Clamp( zRealIndex , 0.f , static_cast<eg_real>(m_Header.HfDims.y-1) );

	//We'll just go with the closest sample for now, but we could potentially lerp between two.
	eg_int xIndex = EGMath_floor(xRealIndex);
	eg_int zIndex = EGMath_floor(zRealIndex);
	eg_real xLerp = xRealIndex - xIndex;
	eg_real zLerp = zRealIndex - zIndex;
	assert( EG_IsBetween( xLerp , 0.f , 1.f ) && EG_IsBetween( zLerp , 0.f , 1.f ) );
	eg_real y0 = GetSampleAtIndex(xIndex,zIndex)*(1.f-xLerp)+GetSampleAtIndex(xIndex,zIndex+1)*xLerp;
	eg_real y1 = GetSampleAtIndex(xIndex+1,zIndex)*(1.f-xLerp)+GetSampleAtIndex(xIndex+1,zIndex+1)*xLerp;
	return (y0*(1.f-zLerp)+y1*zLerp)*m_Header.WorldScale;

	/*
	xi = floor(x);
	yi = floor(y);
	xt = x - xi;
	yt = y - yi;
	y0 = F[xi][yi]*(1-xt)+F[xi][yi+1]*xt;
	y1 = F[xi+1][yi]*(1-xt)+F[xi+1][yi+1]*xt;
	return y0*(1-yt)+y1*yt;
	*/
}

eg_real EGTerrain2::GetSampleAtIndex( eg_int x, eg_int z ) const
{
	eg_real LLSample = 0.f;

	eg_int xIndex = EG_Clamp<eg_int>( x , 0 , m_Header.HfDims.x-1 );
	eg_int zIndex = EG_Clamp<eg_int>( z , 0 , m_Header.HfDims.y-1 );
	return m_HeightField[xIndex + zIndex*m_Header.HfDims.x];//(m_Header.HfDims.x-1-xIndex) + (m_Header.HfDims.y-1-zIndex)*m_Header.HfDims.x];
}

eg_string EGTerrain2::BuildFinalFilename( eg_cpstr Filename )
{
	return EGString_Format( "%s.egterrain", Filename );
}

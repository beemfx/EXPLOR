////////////////////////////////////////////////////////////////////////////////
//
// map.fx - Built in shaders for maps in the Emergence Engine.
//
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
These shaders are all intended for the same purpose only at different levels
of quality. The suffix of each shader 1,2,3,... indicates the quality level
where 1 is the highest quality and quality goes down from there.
*******************************************************************************/
#include <base.hlsli>

struct EGVert_Map
{
	float4 Pos  : EG_POSITION;
	float2 Tex0 : TEXCOORD0;
	float2 Tex1 : TEXCOORD1;
	float4 Color: COLOR0;
};

struct EGVert_MapLow
{
	float4 Pos  : EG_POSITION;
	float2 Tex0 : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
//
// Quality Level 1 - Base Texture and Lightmap (Todo: +cube map?)
//
////////////////////////////////////////////////////////////////////////////////

EGVert_Map VS_QualityHigh(egv_vert_mesh IN) __export( vs , "map_default_high" )
{
	EGVert_Map OUT;
	OUT.Pos   = mul(IN.Pos, g_mWVP); //Final position of vertex.
	OUT.Tex0  = IN.Tex0;             //Use original texture coord.
	OUT.Tex1  = IN.Tex1;
	OUT.Color = IN.Color0;
	return OUT;
}

float4 PS_QualityHigh(EGVert_Map IN) : EG_PS_OUTPUT __export( ps , "map_default_high" )
{
	return EG_Sample(0, IN.Tex0)*EG_Sample(1, IN.Tex1)*IN.Color;
}

////////////////////////////////////////////////////////////////////////////////
//
// Quality Level 4 - Base Texture Only
//
////////////////////////////////////////////////////////////////////////////////

EGVert_MapLow VS_QualityLow(egv_vert_mesh IN) __export( vs , "map_default_low" )
{
	EGVert_MapLow OUT;
	OUT.Pos   = mul(IN.Pos, g_mWVP);
	OUT.Tex0  = IN.Tex0;
	return OUT;
}

float4 PS_QualityLow(EGVert_MapLow IN) : EG_PS_OUTPUT __export( ps , "map_default_low" )
{
	return EG_Sample(0, IN.Tex0);
}
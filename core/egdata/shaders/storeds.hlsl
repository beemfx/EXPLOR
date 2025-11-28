#include <base.hlsli>

///////////////////////////////////////////////////////////////////////////////
//
// StoreDs - Copies depth stencil to a R32 texture.
//
///////////////////////////////////////////////////////////////////////////////

egv_vert_simple VS_StoreDs(egv_vert_simple IN) __export( vs , "storeds" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

float4 PS_StoreDs(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "storeds" )
{
	float c = EG_Sample(0, IN.Tex0).r;
	return float4(c,0,0,1.f);
}

///////////////////////////////////////////////////////////////////////////////
//
// DrawDepth - Used to visulize depth.
//
///////////////////////////////////////////////////////////////////////////////

struct EGVert_DrawDepth
{
	float4 Pos : EG_POSITION;
	float2 Tex : TEXCOORD0;
};

EGVert_DrawDepth VS_DrawDepth( egv_vert_simple IN ) __export( vs , "drawds" )
{
	EGVert_DrawDepth OUT;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	OUT.Tex = IN.Tex0;
	return OUT;
}

float4 PS_DrawDepth( EGVert_DrawDepth IN ) : EG_PS_OUTPUT __export( ps , "drawds" )
{
	float c = EG_SampleGlobal(0, IN.Tex).r;
	c = (c-g_ps_reg0.z)/(1.f-g_ps_reg0.z);
	return float4(c, c, c, 1);
}
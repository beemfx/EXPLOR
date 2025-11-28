#include <base.hlsli>
#define FXAA_PC 1
#if EGFXV == 3
#define FXAA_HLSL_3 1
#elif EGFXV == 5
#define FXAA_HLSL_5 1
#endif
#define FXAA_GREEN_AS_LUMA 1
#define FXAA_QUALITY__PRESET 20
#include "Fxaa3_11.h"

#if (EGFXV == 3)
#define EG_ADJUST 0.5
#elif (EGFXV == 5)
#define EG_ADJUST 0
#endif

//We just need a basic vertex shader (we don't even need to transform anything).

struct VS_OUT
{
	float4 Pos : EG_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUT VS( egv_vert_simple IN ) __export( vs , "fxaa" )
{
	VS_OUT OUT;
	OUT.Pos = IN.Pos;
	OUT.Tex = IN.Tex0;

	return OUT;
}

float4 PS(VS_OUT IN) : EG_PS_OUTPUT __export( ps , "fxaa" )
{
	//The 1/with and 1/height were put in reg0
	const float2 Adj = g_ps_reg0.xy;
	//FXAA Input parameters (PC ONLY)
	const FxaaFloat2 pos                         = float2(IN.Tex.x + EG_ADJUST*Adj.x, IN.Tex.y + EG_ADJUST*Adj.y);
	#if FXAA_HLSL_5 == 1
	const FxaaTex    tex                         = { g_sampx , g_tex2D[GLOBAL_SAMPLER_START] };
	#elif FXAA_HLSL_3 == 1
	const FxaaTex    tex                         = g_sampx;
	#endif
	const FxaaFloat2 fxaaQualityRcpFrame         = Adj.xy; 
	const FxaaFloat  fxaaQualitySubPix           = g_ps_reg0.z; //0.75
	const FxaaFloat  fxaaQualityEdgeThreshold    = g_ps_reg0.w; //0.166
	const FxaaFloat  fxaaQualityEdgeThresholdMin = 0;//0.0625;

	return float4(FxaaPixelShader(
		pos, 
		0,
		tex,
		tex, //Have to put something here.
		tex, //Have to put something here.
		fxaaQualityRcpFrame,
		0,
		0,
		0,
		fxaaQualitySubPix,
		fxaaQualityEdgeThreshold,
		fxaaQualityEdgeThresholdMin,
		0,
		0,
		0,
		0).rgb, 1);

	/*
	float4 t = EG_Sample(0, IN.Tex);
	return float4(t.r, t.r, t.r, 1.0);
	*/
}
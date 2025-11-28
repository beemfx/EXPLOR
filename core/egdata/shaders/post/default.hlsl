#include <base.hlsli>

#if (EGFXV == 3)
#define EG_ADJUST 0.5
#elif (EGFXV == 5)
#define EG_ADJUST 0
#endif

egv_vert_simple VS_Copy( egv_vert_simple IN ) __export( vs , "default_copy" )
{
	const float2 Adj = g_vs_reg0.xy;
	IN.Tex0.x =  IN.Tex0.x + EG_ADJUST*Adj.x;
	IN.Tex0.y =  IN.Tex0.y + EG_ADJUST*Adj.y;
	return IN;
}

float4 PS_Copy( egv_vert_simple IN ): EG_PS_OUTPUT __export( ps , "default_copy" )
{
	#if 0
	const float2 Adj = g_ps_reg0.xy;
	IN.Tex0.x =  IN.Tex0.x + EG_ADJUST*Adj.x;
	IN.Tex0.y =  IN.Tex0.y + EG_ADJUST*Adj.y;
	#endif
	return float4(EG_SampleGlobal( 0 , IN.Tex0 ).rgb,1.0);
}
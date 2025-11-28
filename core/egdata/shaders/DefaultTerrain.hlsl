#include <base.hlsli>
#include <bone.hlsli>
#include <lightf.hlsli>

////////////////////////////////////////////////////////////////////////////////
//
// Terrain 
//
////////////////////////////////////////////////////////////////////////////////

struct egv_vert_terrain_ps
{
	egv_vert_terrain Vert;
	float2 Tex3 : TEXCOORD2;
};

egv_vert_terrain_ps VS_Terrain(egv_vert_terrain IN) __export( vs , "default_terrain" )
{
#if 1
	egv_vert_terrain_ps OUT;
	OUT.Vert = IN;
	OUT.Vert.Pos = mul(IN.Pos, g_mWVP);
	OUT.Tex3 = IN.Tex1/4;
#else
	IN.Pos = mul(IN.Pos, g_mW);

	float3 vDist = IN.Pos.xyz - g_v3Camera;
	float L = length(vDist);

	const float MIN = 100.0;
	const float MAX = 150.0;

	if(MIN < L && L <=MAX)
	{
		IN.Tex1 = IN.Tex1/(1 + 3*(L-MIN)/(MAX-MIN));
	}
	else if(MAX < L)
	{
		IN.Tex1 = IN.Tex1/4;//(L - 50.0f)*100;
	}

	IN.Pos = mul(IN.Pos, g_mVP);
#endif

	//IN.Norm = mul(float4(IN.Norm,0), g_mWVP).xyz;
	return OUT;
}

float4 PS_Terrain(egv_vert_terrain_ps IN): EG_PS_OUTPUT __export( ps , "default_terrain" )
{
	return EG_Sample( 0 , IN.Vert.Tex0 ) * EG_Sample( 1 , IN.Vert.Tex1 );// * IN.Vert.Color0;
	/*
	float4 c0 = EG_Sample( 0 , IN.Vert.Tex0 );
	float4 c1 = EG_Sample( 1 , IN.Vert.Tex1 )*0.5f + EG_Sample( 1 , IN.Tex3.xy )*0.5f;
	float4 c2 = EG_Sample( 2 , IN.Vert.Tex1 )*0.5f + EG_Sample( 2 , IN.Tex3.xy )*0.5f;
	float4 c3 = EG_Sample( 3 , IN.Vert.Tex1 )*0.5f + EG_Sample( 3 , IN.Tex3.xy )*0.5f;

	float4 c = c1*c0.r + c2*c0.g + c3*c0.b;
	//float4 c = c3*IN.Color.a + c2*IN.Color.b + c1*IN.Color.g + c0*IN.Color.r;
	c.a = 1.0;
	//return c1;
	return c;
	//return EG_Sample(1, IN.Tex1)*IN.Color;
	//return IN.Color;
	//return float4(0.5, 0, 0, 1);
	*/
}
////////////////////////////////////////////////////////////////////////////////
//
// default.hlsl - Built in Emergence shaders.
//
////////////////////////////////////////////////////////////////////////////////

#include <base.hlsli>
#include <bone.hlsli>
#include <lightf.hlsli>

////////////////////////////////////////////////////////////////////////////////
//
// These shaders exist only for testing the semantics in-game.
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_mesh VS_EGVert(egv_vert_mesh IN) __export( vs , "default_egvert" )
{
	egv_vert_mesh OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

egv_vert_simple VS_EGVert_Simple(egv_vert_simple IN) __export( vs , "default_egvert_simple" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

egv_vert_terrain VS_EGVert_Terrain(egv_vert_terrain IN) __export( vs , "default_egvert_terrain" )
{
	egv_vert_terrain OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

////////////////////////////////////////////////////////////////////////////////
//
// Color - Only uses vertex color component (including alpha )
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_simple VS_Color(egv_vert_simple IN) __export( vs , "default_color" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

float4 PS_Color(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_color" )
{
	return IN.Color0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Texture - Uses texture color blended with vertex color.
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_simple VS_Texture(egv_vert_simple IN) __export( vs , "default_texture" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	return OUT;
}

float4 PS_Texture(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "default_texture" )
{
	//return IN.Color0;
	//return EG_Sample( 0 , IN.Tex0 );
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Terrain 
//
////////////////////////////////////////////////////////////////////////////////
struct EGVertTerrain
{
	egv_vert_mesh Vert;
	float2 Tex3 : TEXCOORD2;
};

EGVertTerrain VS_Terrain_OLD(egv_vert_mesh IN) __export( vs , "default_terrain_old" )
{
	#if 1
	EGVertTerrain OUT;
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

float4 PS_Terrain_OLD(EGVertTerrain IN): EG_PS_OUTPUT __export( ps , "default_terrain_old" )
{
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
}

////////////////////////////////////////////////////////////////////////////////
//
// Shadow
//
////////////////////////////////////////////////////////////////////////////////

//Position lights will create more of a flare shadow effect, whereas
//direction lights cast the shadow in a specific direction.
#define POSITION_LIGHT (g_vs_reg0.y != 0)


struct VS_OUTPUT
{
	float4 Pos  : EG_POSITION;
	float4 Diff : COLOR0;
};

VS_OUTPUT VS_Shadow(egv_vert_mesh IN) __export( vs , "default_shadow" )
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	//Transform the position (P) and norm (N) to world space.
	EG_BoneTransformThis( IN );
	float4 N = IN.Norm;
	float4 P = mul(IN.Pos,g_mW);
	N = normalize(mul(N, g_mW));


	float3 L;
	if(POSITION_LIGHT)
		L = normalize(P.xyz - g_Lights[0].Pos.xyz);
	else
		L = g_Lights[0].Dir.xyz;


	//Now see if this face is a shadow face or not.
	if(dot(N.xyz, L) > 0.0f)
	{
		P.xyz += L*g_vs_reg0.x;
		OUT.Pos = mul(P, g_mVP);
		OUT.Diff = float4(1,0,0,0.5f);
	}
	else
	{
		OUT.Pos = mul(P, g_mVP);
		OUT.Diff = float4(1,1,0,0.5f);
	}
	
	return OUT;
}

float4 PS_Shadow(VS_OUTPUT IN) : EG_PS_OUTPUT __export( ps , "default_shadow" )
{
	return IN.Diff;
}

////////////////////////////////////////////////////////////////////////////////
//
// Mask
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_mesh VS_Mask(egv_vert_mesh IN) __export( vs , "default_mask" )
{
	egv_vert_mesh OUT = EG_BoneTransform(IN);
	
	OUT.Pos = mul( OUT.Pos, g_mWVP );
	OUT.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);
	return OUT;
}

float4 PS_Mask(egv_vert_mesh IN): EG_PS_OUTPUT __export( ps , "default_mask" )
{
	return IN.Color0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Palette Texture
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_mesh VS_TextureWithPalette(egv_vert_mesh IN) __export( vs , "texture_with_palette" )
{
	EG_BoneTransformThis( IN );
	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);
	return IN;
}

float4 PS_TextureWithPalette(egv_vert_mesh IN): EG_PS_OUTPUT __export( ps , "texture_with_palette" )
{
	return EG_Sample( 0 , IN.Tex0 ) * IN.Color0;
}

// (c) 2016 Beem Media

///////////////////////////////////////////////////////////////////////////////
// Shaders for map tiles.
///////////////////////////////////////////////////////////////////////////////

#include "ExSharedShaders.hlsli"

struct EGVert_ExMap
{
	float4 Pos                 : EG_POSITION;
	float3 PointLightDir       : NORMAL0;
	float3 SunlightDir         : NORMAL1;
	float3 EyeDir              : NORMAL2;
	float2 Tex0                : TEXCOORD0;
	float2 Tex1                : TEXCOORD1;
	float2 FogBgTex            : TEXCOORD2;
	float4 AmbientColor        : COLOR0; // Combination of ambient light + vertex light
	float4 PointLightColor     : COLOR1;
	float4 SunlightColor       : COLOR2;
	float4 DiffuseColor        : COLOR3;
	float4 Fog                 : COLOR4;
};

///////////////////////////////////////////////////////////////////////////////
// Basic:
// Lit by vertex color, ambient light, single player point light, and a
// directional sun light.
///////////////////////////////////////////////////////////////////////////////

EGVert_ExMap ExMapVS_Basic(egv_vert_mesh IN) __export( vs , "ex_map_basic" )
{
	EGVert_ExMap OUT;
	float4 WorldPos = mul( IN.Pos , g_mW );
	float4 NormWorld = mul( IN.Norm , g_mW );

	OUT.Pos = mul(WorldPos, g_mVP); //Final position of vertex.

	float3x3 TSM = EG_ComputeTangentSpaceMatrix( IN );
	float3 LookAtDir = normalize(g_v4Camera.xyz - WorldPos.xyz);

	OUT.AmbientColor = EG_ComputeAmbient() + IN.Color0; // The vertex color is what we're actually using isntead of a lightmap for precomputed lighting.

	// Player Light
	float4 LightDir;
	float4 Specular;
	EG_ComputeDiffuseAndSpecularFromPointLights( WorldPos , OUT.PointLightColor , Specular , LightDir , 0 , 0 );
	OUT.PointLightDir = mul( LightDir.xyz , TSM );

	// Sun Light
	OUT.SunlightDir   = -mul( g_Lights[3].Dir.xyz , TSM );
	OUT.SunlightColor = g_Lights[3].Color;

	OUT.Tex0   = IN.Tex0;
	OUT.Tex1   = IN.Tex1;
	OUT.DiffuseColor = g_Material.Diffuse;
	
	OUT.EyeDir = mul( LookAtDir.xyz , TSM );

	// Fog
	ExShader_ComputeFog( WorldPos , OUT.Pos , OUT.Fog , OUT.FogBgTex );

	return OUT;
}

float4 ExMapPS_Basic(EGVert_ExMap IN) : EG_PS_OUTPUT __export( ps , "ex_map_basic" )
{
	float4 TextureDiffuse = EG_Sample( 0 , IN.Tex0 )*IN.DiffuseColor; // Texture, lightmap, and material color.
	float3 Normal = 2.f*(EG_Sample(1, IN.Tex0).xyz - .5f);   // Normal map

	float4 LightingColor = ExShader_ComputeLightColor( Normal , IN.SunlightDir , IN.SunlightColor , IN.PointLightDir , IN.PointLightColor , IN.AmbientColor ); 

	float4 FinalColor = TextureDiffuse*LightingColor;

	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	return OUT;
}

///////////////////////////////////////////////////////////////////////////////
// Lava:
///////////////////////////////////////////////////////////////////////////////

EGVert_ExMap ExMapVS_Lava(egv_vert_mesh IN) __export( vs , "ex_map_lava" )
{
	EGVert_ExMap OUT = ExMapVS_Basic( IN );
	
	// OUT.DiffuseColor *= (.5f + (1.f + sin(3.14159*2.*EG_GetOneMinuteCycleTime()))*.25f);
	OUT.Tex0 += EG_GetOneMinuteCycleTime();

	return OUT;
}

float4 ExMapPS_Lava(EGVert_ExMap IN) : EG_PS_OUTPUT __export( ps , "ex_map_lava" )
{
	float4 TextureDiffuse = EG_Sample( 0 , IN.Tex0 )*IN.DiffuseColor; // Texture, lightmap, and material color.
	float4 FinalColor = TextureDiffuse;
	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	return OUT;
}

///////////////////////////////////////////////////////////////////////////////
// Ooze:
///////////////////////////////////////////////////////////////////////////////

EGVert_ExMap ExMapVS_Ooze(egv_vert_mesh IN) __export( vs , "ex_map_ooze" )
{
	EGVert_ExMap OUT = ExMapVS_Basic( IN );

	// OUT.DiffuseColor *= (.5f + (1.f + sin(3.14159*2.*EG_GetOneMinuteCycleTime()))*.25f);
	OUT.Tex0 += EG_GetOneMinuteCycleTime();

	return OUT;
}

float4 ExMapPS_Ooze(EGVert_ExMap IN) : EG_PS_OUTPUT __export( ps , "ex_map_ooze" )
{
	return ExMapPS_Basic( IN );
}

///////////////////////////////////////////////////////////////////////////////
// Water:
///////////////////////////////////////////////////////////////////////////////

EGVert_ExMap ExMapVS_Water(egv_vert_mesh IN) __export( vs , "ex_map_water" )
{
	EGVert_ExMap OUT = ExMapVS_Basic( IN );

	// OUT.DiffuseColor *= (.5f + (1.f + sin(3.14159*2.*EG_GetOneMinuteCycleTime()))*.25f);
	OUT.Tex0 += EG_GetOneMinuteCycleTime();
	OUT.Tex1.x = .25;//EG_GetOneMinuteCycleTime();

	return OUT;
}

float4 ExMapPS_Water(EGVert_ExMap IN) : EG_PS_OUTPUT __export( ps , "ex_map_water" )
{
	return ExMapPS_Basic( IN ) + EG_Sample( 3 , IN.Tex0 )*IN.Tex1.x;
}

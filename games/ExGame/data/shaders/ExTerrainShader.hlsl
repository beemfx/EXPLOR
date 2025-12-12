// (c) 2016 Beem Media

#include "ExSharedShaders.hlsli"

///////////////////////////////////////////////////////////////////////////////
// Ground Terrain
///////////////////////////////////////////////////////////////////////////////

egv_vert_mesh ExMapVS_GroundEnt(egv_vert_mesh IN) __export( vs , "ex_ground_ent" )
{
	egv_vert_mesh OUT = IN;
	EG_BoneTransformThis( OUT );
	OUT.Pos = mul(OUT.Pos,g_mWVP);
	OUT.Tex0 *= 60.f;
	OUT.Color0 = g_AmbientLight;// g_Lights[3].Color;// g_vs_GamePalette2; // EG_ComputeAmbient();
	OUT.Color0.a = 1.f;
	return OUT;
}

float4 ExMapPS_GroundEnt(egv_vert_mesh IN) : EG_PS_OUTPUT __export( ps , "ex_ground_ent" )
{
	return EG_Sample(0,IN.Tex0)*IN.Color0;
}

///////////////////////////////////////////////////////////////////////////////
// Ground Terrain
///////////////////////////////////////////////////////////////////////////////

egv_vert_mesh ExMapVS_GroundSewersEnt(egv_vert_mesh IN) __export( vs , "ex_ground_sewers_ent" )
{
	egv_vert_mesh OUT = IN;
	EG_BoneTransformThis( OUT );
	OUT.Pos = mul(OUT.Pos,g_mWVP);
	OUT.Tex0 *= 15.f;
	OUT.Tex0.x -= EG_GetOneMinuteCycleTime();
	OUT.Tex0.y += EG_GetOneMinuteCycleTime();
	OUT.Tex1.x = .25;//EG_GetOneMinuteCycleTime();

	OUT.Color0 = g_AmbientLight;// g_Lights[3].Color;// g_vs_GamePalette2; // EG_ComputeAmbient();
	OUT.Color0.a = 1.f;
	return OUT;
}

float4 ExMapPS_GroundSewersEnt(egv_vert_mesh IN) : EG_PS_OUTPUT __export( ps , "ex_ground_sewers_ent" )
{
	return EG_Sample(0,IN.Tex0)*IN.Color0 + EG_Sample( 3 , IN.Tex0 )*IN.Tex1.x;
}

///////////////////////////////////////////////////////////////////////////////
// Ground Terrain
///////////////////////////////////////////////////////////////////////////////

egv_vert_mesh ExMapVS_GroundHellEnt(egv_vert_mesh IN) __export( vs , "ex_ground_hell_ent" )
{
	egv_vert_mesh OUT = IN;
	EG_BoneTransformThis( OUT );
	OUT.Pos = mul(OUT.Pos,g_mWVP);
	OUT.Tex0 *= 60.f;
	OUT.Tex0.x -= EG_GetOneMinuteCycleTime();
	OUT.Tex0.y += EG_GetOneMinuteCycleTime();
	OUT.Tex1.x = .25;//EG_GetOneMinuteCycleTime();

	OUT.Color0 = float4(1.f,1.f,1.f,1.f);//g_AmbientLight;// g_Lights[3].Color;// g_vs_GamePalette2; // EG_ComputeAmbient();
	OUT.Color0.a = 1.f;
	return OUT;
}

float4 ExMapPS_GroundHellEnt(egv_vert_mesh IN) : EG_PS_OUTPUT __export( ps , "ex_ground_hell_ent" )
{
	return EG_Sample(0,IN.Tex0)*IN.Color0 + EG_Sample( 3 , IN.Tex0 )*IN.Tex1.x;
}

///////////////////////////////////////////////////////////////////////////////
// Skybox
///////////////////////////////////////////////////////////////////////////////

struct EGVert_CubeTex
{
	float4 Pos  : EG_POSITION;
	float3 Tex0 : TEXCOORD0;
	float4 Color0 : COLOR0;
};

EGVert_CubeTex ExVS_Skybox( egv_vert_mesh IN ) __export( vs , "ex_skybox" )
{
	EGVert_CubeTex OUT;

	//The normal vector points to the actual vertex to properly select the right
	//spot in the cube texture.
	OUT.Tex0 = normalize(IN.Pos.xyz);
	//And we can compute the position easily.
	OUT.Pos = mul(float4(IN.Pos.xyz,0), g_mWVP);
	OUT.Pos.z = OUT.Pos.w*.9999f; // This will force the skybox to get drawn at maximum z
	OUT.Color0 = float4(1.f,1.f,1.f,g_AmbientLight.r)*g_Material.Diffuse;

	return OUT;
}

float4 ExPS_Skybox( EGVert_CubeTex IN ) : EG_PS_OUTPUT __export( ps , "ex_skybox" )
{
	const float3 Blend = IN.Color0.a*EG_SampleCube( 0 , IN.Tex0 ) + (1.f-IN.Color0.a)*EG_SampleCube(1,IN.Tex0);
	const float3 Colored = Blend*IN.Color0.rgb;
	return float4(Colored,1.f);
}

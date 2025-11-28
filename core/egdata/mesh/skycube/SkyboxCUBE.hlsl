#include <base.hlsli>

struct EGVert_CubeTex
{
	float4 Pos  : EG_POSITION;
	float3 Tex0 : TEXCOORD1;
};

EGVert_CubeTex VS( egv_vert_mesh IN ) __export( vs , "SkyboxCUBE" )
{
	EGVert_CubeTex OUT;

	//The normal vector points to the actual vertex to properly select the right
	//spot in the cube texture.
	OUT.Tex0 = normalize(IN.Pos.xyz);
	//And we can compute the position easily.
	OUT.Pos = mul(float4(IN.Pos.xyz,0), g_mWVP);
	OUT.Pos.z = OUT.Pos.w*.9999f; // This will force the skybox to get drawn at maximum z

	return OUT;
}

float4 PS( EGVert_CubeTex IN ) : EG_PS_OUTPUT __export( ps , "SkyboxCUBE" )
{
	return EG_SampleCube(3, IN.Tex0 );
}

#include <base.hlsli>
#include <bone.hlsli>

egv_vert_mesh DebugShapeVS( egv_vert_mesh IN ) __export( vs , "DebugShape" )
{
	EG_BoneTransformThis( IN );
	IN.Pos = mul( IN.Pos , g_mW );
	IN.Norm = mul( IN.Norm , g_mW );
	float LightIntensity = saturate( dot( IN.Norm.xyz , float3(0,0,-1) ) );
	const float MinI = .6f;
	LightIntensity = saturate((1.f - MinI)*LightIntensity + MinI);
	IN.Pos = mul( IN.Pos , g_mVP );
	IN.Color0 = float4( (IN.Color0 * g_Material.Diffuse * EG_GetPalette( 0 )).xyz * LightIntensity , IN.Color0.w );
	return IN;
}

float4 DebugShapePS( egv_vert_mesh IN ) : EG_PS_OUTPUT __export( ps , "DebugShape" )
{
	return EG_Sample( 0 , IN.Tex0 ) * IN.Color0;
}
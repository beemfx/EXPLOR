// (c) 2017 Beem Media

#include <base.hlsli>
#include <bone.hlsli>
#include <lightf.hlsli>
#include <EGUtilityLib.hlsli>

#pragma warning(disable:3206)

void ExShader_ComputeFog( in float4 WorldPos , in float4 ScreenPos , out float4 Fog , out float2 FogBgTex )
{
	// Base the distance off of being a box rather than the distance from the camera so an entire wall fades in at once.
	float VertexDistance = max(abs(WorldPos.z-g_v4Camera.z),abs(WorldPos.x-g_v4Camera.x));// From camera would be: length(WorldPos - g_v4Camera);
	float FogStart = g_vs_GamePalette0.x;
	float FogEnd   = g_vs_GamePalette0.y;
	//float FadeEnd  = g_vs_GamePalette0.z;
	float FogIntensity = saturate((FogEnd - VertexDistance)/(FogEnd - FogStart));
	// float FogAlpha = saturate((FadeEnd - VertexDistance)/(FadeEnd - FogEnd));

	Fog      = float4( g_vs_GamePalette1.rgb , FogIntensity );
	FogBgTex = (float2( ((ScreenPos.x/ScreenPos.w)+1.f)*.5f , (1.f-(ScreenPos.y/ScreenPos.w))*.5f ));
}

float4 ExShader_ComputeFogBlend( in float4 Fog , in float4 MapColor , in float2 FogBgTex )
{
	float4 FogColor = EG_SampleGlobal( 0 , FogBgTex );
	return float4( (1-Fog.a)*FogColor.rgb + (Fog.a)*MapColor.rgb , MapColor.a );
}

float4 ExShader_ComputeLightColor( in float3 Normal , in float3 SunlightDir , in float4 SunlightColor , in float3 PointLightDir , in float4 PointLightColor , in float4 AmbientColor )
{
	float SunlightIntensity = saturate(dot( Normal , SunlightDir));
	float PointLightIntensity = saturate(dot( Normal , PointLightDir));

	float3 LightingPercentages = g_ps_GamePalette2;

	float4 ColorFromSunlight = SunlightColor*SunlightIntensity*LightingPercentages.x;
	float4 ColorFromPointLights = PointLightColor*PointLightIntensity*LightingPercentages.y;
	float4 ColorFromAmbientColor = AmbientColor*LightingPercentages.z;

	float4 LightingColor = ( ColorFromSunlight + ColorFromPointLights + ColorFromAmbientColor );
	return LightingColor;
}

float4 ExShader_ComputeLightColorForBillboard( in float4 SunlightColor , in float4 PointLightColor , in float4 AmbientColor )
{
	// Unlike ExShader_ComputeLightColor, normals are ignored.
	const float SunlightIntensity = 1.f;
	const float PointLightIntensity = 1.f;

	float3 LightingPercentages = g_vs_GamePalette2;

	float4 ColorFromSunlight = SunlightColor*SunlightIntensity*LightingPercentages.x;
	float4 ColorFromPointLights = PointLightColor*PointLightIntensity*LightingPercentages.y;
	float4 ColorFromAmbientColor = AmbientColor*LightingPercentages.z;

	float4 LightingColor = ( ColorFromSunlight + ColorFromPointLights + ColorFromAmbientColor );
	return LightingColor;
}
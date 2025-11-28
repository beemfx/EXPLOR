/******************************************************************************
	base.hlsli - The basic HLSL declarations for the Emergence Engine graphics
	pipeline. This describes the structures used by the Emergence Engine and
	the variables that a shader may be used with the appropriate register
	offsets. All rendered items get all these variables, but they may not be
	meaningful for certain rendered objects. For example g_mBoneM is only
	valid when meshes are rendered.

	Any shader may get access to this file by specifing #include <base.hlsli>
	(triangle brackets are necessary) and then may use the variables
	described. This file is maintained internally with the game's compiler so
	it doesn't ened to be distributed with the game, though it will probably
	server as a helpful reference for modifying the shaders.

	(c) 2012 Beem Software
******************************************************************************/
#ifndef __BASE_FX__
#define __BASE_FX__

#if EGFXV == 3
#define EG_POSITION  POSITION
#define EG_PS_OUTPUT COLOR0
#elif EGFXV == 5
#define EG_POSITION  SV_POSITION
#define EG_PS_OUTPUT SV_TARGET
#endif

#define __export( type , filename )


/***********************************
*** Shader Registers (Generated) ***
***********************************/
#if EGFXV == 3
#include <ShaderConsts_3.hlsli>
#elif EGFXV == 5
#include <ShaderConsts_5.hlsli>
#endif

//The game uses up to four samplers:
static const uint GLOBAL_SAMPLER_START = 4;
static const uint GLOBAL_REFLECTION_SAMPLER_INDEX = 7;
sampler      g_samp[8]    : register( ps , s0 );
SamplerState g_sampx      : register( ps , s0 );
Texture2D    g_tex2D[8]   : register( ps , t0 );
TextureCube  g_texCube[8] : register( ps , t0 );


float4 EG_Sample( uint Index , float2 Tc )
{
	#if EGFXV == 3
	return tex2D( g_samp[Index] , Tc );
	#elif EGFXV == 5
	return g_tex2D[Index].Sample( g_sampx , Tc );
	#endif
}

float4 EG_SampleCube( uint Index , float3 Tc )
{
	#if EGFXV == 3
	return texCUBE( g_samp[Index] , Tc );
	#elif EGFXV == 5
	return g_texCube[Index].Sample( g_sampx , Tc );
	#endif
}

float4 EG_SampleGlobal( uint Index , float2 Tc )
{
#if EGFXV == 3
	return tex2D( g_samp[Index+GLOBAL_SAMPLER_START] , Tc );
#elif EGFXV == 5
	return g_tex2D[Index+GLOBAL_SAMPLER_START].Sample( g_sampx , Tc );
#endif
}

float4 EG_SampleGlobalCube( uint Index , float3 Tc )
{
#if EGFXV == 3
	return texCUBE( g_samp[Index+GLOBAL_SAMPLER_START] , Tc );
#elif EGFXV == 5
	return g_texCube[Index+GLOBAL_SAMPLER_START].Sample( g_sampx , Tc );
#endif
}

float4 EG_SampleReflection( float2 Tc )
{
	#if EGFXV == 3
	return float4( .2f , 2.f , .2f , 1.f );
	#elif EGFXV == 5
	return float4(g_tex2D[GLOBAL_REFLECTION_SAMPLER_INDEX].Sample( g_sampx , Tc ).rgb,1.f);
	#endif
}

float EG_GetTime()
{
	return g_fTimefOneMinuteCycleTime.x;
}

float EG_GetOneMinuteCycleTime()
{
	return g_fTimefOneMinuteCycleTime.y;
}

float4 EG_GetScaling()
{
	return g_ScaleVec;
}

float4 EG_GetPalette( uint Index )
{
	if( Index == 1 )
	{
		return g_vs_Palette1;
	}

	return g_vs_Palette0;
}

float4 EG_GetPsPalette( uint Index )
{
	if( Index == 1 )
	{
		return g_ps_Palette1;
	}

	return g_ps_Palette0;
}

bool EGLight_IsEnabled( EGLight Light )
{
	return Light.OnOff_RangeSq_Intensity_FallofRadiusSq.x > 0.f;
}

float EGLight_GetRangeSq( EGLight Light )
{
	return Light.OnOff_RangeSq_Intensity_FallofRadiusSq.y;
}

float EGLight_GetFalloffRadiusSq( EGLight Light )
{
	return Light.OnOff_RangeSq_Intensity_FallofRadiusSq.w;
}

float EGLight_GetIntensity( EGLight Light )
{
	return Light.OnOff_RangeSq_Intensity_FallofRadiusSq.z;
}

#endif //__BASE_FX__

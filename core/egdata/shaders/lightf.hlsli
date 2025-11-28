#ifndef __LIGHT_FX__
#define __LIGHT_FX__

#include <base.hlsli>

float EG_GetLightIntensity( uint Index )
{
	return EGLight_IsEnabled( g_Lights[Index] ) * EGLight_GetIntensity( g_Lights[Index] );
}

//P - Vertex position in world space.
//N - Vertex normal in world space.
//V - Vector pointing in direction of camera in world space.

float4 EG_ComputeDiffuseInWorldSpace( float4 P , float4 N )
{
	float4 Color = float4(0,0,0,0);

	for(uint i=0; i<4; i++)
	{
		float4 L = -g_Lights[i].Dir;
		Color += EG_GetLightIntensity(i)*g_Lights[i].Color*max(0,dot(N,L)); 
	}

	//Finally we multiply by the material's diffuse color.
	return g_Material.Diffuse*Color;
}

float4 EG_ComputeSpecularInWorldSpace( float4 P , float4 N , float4 V )
{
	float4 Color = float4(0,0,0,0);

	for(uint i=0; i<4; i++)
	{
		// By design all vectors should be normalized.
		float4 L = -g_Lights[i].Dir;
		float4 R = reflect(-L, N);
		Color += EG_GetLightIntensity(i)*g_Lights[i].Color*pow(max(dot(R, V), 0), max(1,g_Material.Power.x));
	}

	//clamp( Color , 0.f , 1.f );

	return g_Material.Specular*Color;
}


float4 EG_ComputeAmbient()
{
	return g_AmbientLight*g_Material.Ambient;
}

float4 EG_ComputeEmissive()
{
	return g_Material.Emissive;
}

float3x3 EG_ComputeTangentSpaceMatrix( const egv_vert_mesh V )
{
	// Don't need to normalize these since the world matrix rotation portion should be orthonormal
	float4 T = mul(float4(V.Tan.xyz,0), g_mW);
	float4 N = mul(float4(V.Norm.xyz,0), g_mW);

	//Compute the binormal vector.
	float3 B = cross(N.xyz, T.xyz)*V.Tan.w;

	//Form the tangent space transform
	return float3x3
		(
			T.x, B.x, N.x,
			T.y, B.y, N.y,
			T.z, B.z, N.z
			);
}

#pragma warning(push)
#pragma warning(disable:3557)

void EG_ComputeDiffuseAndSpecularFromPointLights( const float4 VertexPos , out float4 DiffuseOut , out float4 SpecularOut , out float4 LightDirOut , const uint MIN_LIGHT , const uint MAX_LIGHT )
{
	LightDirOut = float4(0,0,0,0); // Will hold the direction of the light
	DiffuseOut  = float4(0,0,0,0);
	SpecularOut = float4(0,0,0,0);
	float TotalScale = 0.f;
	for(uint i=MIN_LIGHT; i<=MAX_LIGHT; i++)
	{
		if( EGLight_IsEnabled( g_Lights[i] ) )
		{
			float4 LightDir = VertexPos - g_Lights[i].Pos;
			LightDir.w = 0; // Just in case.
			float DistSq     = dot(LightDir,LightDir);
			float RangeSq    = EGLight_GetRangeSq( g_Lights[i] );
			float FalloffRSq = EGLight_GetFalloffRadiusSq( g_Lights[i] );

#if 0
			// Using if statements...
			float Scale = 0.f;
			if( DistSq <= FalloffRSq )
			{
				Scale = 1.f;
			}
			else if( DistSq <= RangeSq )
			{
				Scale = (1.0 - abs((DistSq-FalloffRSq)/(RangeSq-FalloffRSq))) + .001;
			}
#elif 1
			// Using multiply in place of if statements.
			// Note that if RangeSq == FalloffRSq this won't work as the
			// divisor for ScaleIfOutOfFalloffR with be 0 and the expression will
			// be infinity. So as a rule falloff must be < RangeSq
			float ScaleIfInFalloffR = (DistSq<=FalloffRSq)*1.f;
			float ScaleIfOutOfFalloffR = (DistSq>FalloffRSq)*(1.0 - (DistSq-FalloffRSq)/(RangeSq-FalloffRSq));
			float Scale = (DistSq<RangeSq) * ( ScaleIfInFalloffR + ScaleIfOutOfFalloffR ) + .001;
#else
			// Falloff range of 0.
			float Scale = (DistSq<RangeSq)*(1.0 - DistSq/RangeSq);
#endif

			TotalScale += Scale;

			LightDirOut  += LightDir*Scale;
			DiffuseOut   += g_Lights[i].Color*Scale*g_Material.Diffuse * EGLight_GetIntensity( g_Lights[i] );
			SpecularOut  += g_Lights[i].Color*Scale*g_Material.Specular* EGLight_GetIntensity( g_Lights[i] );
		}
	}

	DiffuseOut = saturate( DiffuseOut );
	SpecularOut = saturate( SpecularOut );
	SpecularOut.a = max(1,g_Material.Power.x);

	LightDirOut = -normalize(LightDirOut*TotalScale);
}

#pragma warning(pop)

#endif //__LIGHT_FX__
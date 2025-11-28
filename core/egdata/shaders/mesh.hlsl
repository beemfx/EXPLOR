////////////////////////////////////////////////////////////////////////////////
//
// mesh.fx - Built in shaders for meshes in the Emergence Engine.
//
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
These shaders are all intended for the same purpose only at different levels
of quality. The suffix of each shader 1,2,3,... indicates the quality level
where 1 is the highest quality and quality goes down from there.
*******************************************************************************/

#include <base.hlsli>
#include <lightf.hlsli>
#include <bone.hlsli>

struct EGVert_Bump
{
	float4 Pos          : EG_POSITION;
	float2 Tex0         : TEXCOORD0;
	float3 LightDir     : NORMAL0;
	float3 EyeDir       : NORMAL1;
	float4 Diffuse      : COLOR0;
	float4 Ambient      : COLOR1;
	float4 Emissive     : COLOR2;
	float4 Specular     : COLOR3;
	float4 VertexColor  : COLOR4;
};

////////////////////////////////////////////////////////////////////////////////
//
// High Quality - Bump Map using light's position.
//
////////////////////////////////////////////////////////////////////////////////

EGVert_Bump VS_QualityHigh( egv_vert_mesh IN ) __export( vs , "mesh_default_high" ) 
{
	EGVert_Bump OUT;
	
	//All lighting is done in world space.
	EG_BoneTransformThis( IN );
	float3x3 TSM = EG_ComputeTangentSpaceMatrix( IN );
	float4 P = mul(IN.Pos, g_mW);
									
	float3 LookAtDir = normalize(g_v4Camera.xyz - P.xyz);

	OUT.Ambient = EG_ComputeAmbient();
	OUT.Emissive = EG_ComputeEmissive();
	float4 LightDir;
	EG_ComputeDiffuseAndSpecularFromPointLights( P , OUT.Diffuse , OUT.Specular , LightDir , 0 , 3 );

	//Transform light into normal space.
	OUT.LightDir     = mul(LightDir.xyz, TSM);
	OUT.EyeDir       = mul(LookAtDir.xyz, TSM);
	OUT.Pos          = mul(P, g_mVP);
	OUT.Tex0         = IN.Tex0;
	OUT.VertexColor  = IN.Color0;
		
	return OUT;
}


float4 PS_DiffusePerPixelSpecNone(EGVert_Bump IN) : EG_PS_OUTPUT __export( ps , "mesh_default_high" ) 
{
	float4 C = EG_Sample(0, IN.Tex0);       // Diffuse Texture
	float3 B = EG_Sample(1, IN.Tex0).xyz;   // Normal map
	B = (2.0*(B-0.5));
	float  I = saturate(dot( B , IN.LightDir)); //Transform normal

	return saturate(float4( (I*IN.Diffuse.rgb + IN.Ambient.rgb)*C.rgb + IN.Emissive.rgb , C.a*IN.VertexColor.a ));
}

//
// High quality with diffuse and specular per pixel
//
EGVert_Bump VS_NormPerPixelSpecPerPixel( egv_vert_mesh IN ) __export( vs , "mesh_diffuse_spec_high" )
{
	return VS_QualityHigh( IN );
}

float4 PS_NormPerPixelSpecPerPixel(EGVert_Bump IN) : EG_PS_OUTPUT __export( ps , "mesh_diffuse_spec_high" ) 
{
	float4 C = EG_Sample(0, IN.Tex0);       // Diffuse Texture
	float3 B = EG_Sample(1, IN.Tex0).xyz;   // Normal map
	float4 S = EG_Sample(2, IN.Tex0);
	B.xy = (2.0*(B.xy-0.5));
	float  I = saturate(dot(B,IN.LightDir)); //Transform normal
	float3 R = normalize(reflect(-IN.LightDir,B));
	float3 DiffColor = C.rgb*(I*IN.Diffuse.rgb + IN.Ambient.rgb);
	float3 SpecColor = IN.Specular.rgb*S.rgb*pow(saturate(dot(R,normalize(IN.EyeDir))),IN.Specular.a);

	//return float4( IN.LightDir.xyz , 1.f );
	//return float4( SpecColor , C.a );
	//return float4( I*float3(1,1,1) , C.a );
	return saturate(float4( DiffColor + IN.Emissive.rgb + SpecColor , C.a*IN.VertexColor.a ));
}


////////////////////////////////////////////////////////////////////////////////
//
// Low Quality Standard Lighting
//
////////////////////////////////////////////////////////////////////////////////

struct EGVert_LowQuality
{
	float4 Pos    : EG_POSITION;
	float2 Tex0   : TEXCOORD0;
	float4 Color0 : COLOR0;
	float4 Color1 : COLOR1;
};


EGVert_LowQuality VS_QualityLow(egv_vert_mesh IN) __export( vs , "mesh_default_low" ) 
{
	EG_BoneTransformThis( IN );
	
	float4 P = mul( IN.Pos , g_mW );
	float4 N = mul( IN.Norm, g_mW );
	//Normal should be normalized by the way the engine is set up.
	//N = normalize( N );

	EGVert_LowQuality OUT;
	OUT.Pos  = mul(IN.Pos , g_mWVP);

	//Now we compute various vectors that will be of use. (We want everything in world space).
	float4 V = float4(normalize(g_v4Camera.xyz - P.xyz),0);                  //Vector pointing towards camera.

	float4 Diffuse  = EG_ComputeDiffuseInWorldSpace( P , N )*IN.Color0;
	float4 Specular = EG_ComputeSpecularInWorldSpace( P , N , V );
	float4 Ambient  = EG_ComputeAmbient();
	float4 Emissive = EG_ComputeEmissive();

	// Color0 will be blended with the sampler (texture) color
	// Color1 will be added to it
	OUT.Color0 = Diffuse + Ambient;
	OUT.Color1 = Specular + Emissive;

	// We only want alpha from the sampler.
	OUT.Color0.a = 1.0f;
	OUT.Color1.a = 0;

	OUT.Tex0 = IN.Tex0;

	return OUT;
}

float4 PS_QualityLow(EGVert_LowQuality IN) : EG_PS_OUTPUT __export( ps , "mesh_default_low" ) 
{
	return EG_Sample( 0 , IN.Tex0 ) * IN.Color0 + IN.Color1;
}



////////////////////////////////////////////////////////////////////////////////
//
// Unlit - Does transform and uses texture 0 plus material color and vertex color
//
////////////////////////////////////////////////////////////////////////////////

egv_vert_mesh VS_Unlit(egv_vert_mesh IN) __export( vs , "mesh_default_unlit" )
{
	EG_BoneTransformThis( IN );
	
	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0;
	
	return IN;
}

float4 PS_Unlit(egv_vert_mesh IN): EG_PS_OUTPUT __export( ps , "mesh_default_unlit" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

egv_vert_mesh VS_WithPaletteUnlit(egv_vert_mesh IN) __export( vs , "mesh_with_palette_unlit" )
{
	EG_BoneTransformThis( IN );

	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);

	return IN;
}

float4 PS_WithPaletteUnlit(egv_vert_mesh IN): EG_PS_OUTPUT __export( ps , "mesh_with_palette_unlit" )
{
	return PS_Unlit( IN );
}

egv_vert_mesh VS_UnlitNFade(egv_vert_mesh IN) __export( vs , "mesh_unlit_faux_l" )
{
	EG_BoneTransformThis( IN );

	float4 P = mul( IN.Pos , g_mW );
	float4 N = mul( IN.Norm , g_mW );
	float4 L = float4(0,1,0,0);

	IN.Pos = mul( IN.Pos, g_mWVP );
	float Alpha = IN.Color0.a*g_Material.Diffuse.a;
	IN.Color0 = g_Material.Diffuse*saturate( IN.Color0*max(0,dot(N,L)) + .7 );
	IN.Color0.a = Alpha;
	
	return IN;
}

float4 PS_UnlitNFade(egv_vert_mesh IN): EG_PS_OUTPUT __export( ps , "mesh_unlit_faux_l" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}


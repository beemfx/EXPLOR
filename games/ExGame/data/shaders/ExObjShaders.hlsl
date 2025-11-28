// (c) 2017 Beem Media

#include "ExSharedShaders.hlsli"

struct EGVert_ExObj
{
	float4 Pos               : EG_POSITION;
	float3 PointLightDir     : NORMAL0;
	float3 SunlightDir       : NORMAL1;
	float3 EyeDir            : NORMAL2;
	float2 Tex0              : TEXCOORD0;
	float2 FogBgTex          : TEXCOORD1;
	float4 AmbientColor      : COLOR0;
	float4 PointLightColor   : COLOR1;
	float4 SunlightColor     : COLOR2;
	float4 DiffuseColor      : COLOR3;
	float4 EmissiveColor     : COLOR4;
	float4 SpecularColor     : COLOR5;
	float4 Fog               : COLOR6;
};

EGVert_ExObj ExObjVS_BasicCommon( in egv_vert_mesh IN , out float DistFromCamera )
{
	EGVert_ExObj OUT;

	//All lighting is done in world space.
	EG_BoneTransformThis( IN );
	float3x3 TSM = EG_ComputeTangentSpaceMatrix( IN );
	float4 WorldPos = mul(IN.Pos, g_mW);

	float3 LookAtVec = (g_v4Camera.xyz - WorldPos.xyz);
	float3 LookAtDir = normalize(LookAtVec);
	DistFromCamera = length(LookAtVec);

	OUT.AmbientColor = EG_ComputeAmbient();
	OUT.EmissiveColor = EG_ComputeEmissive();
	float4 LightDir;
	EG_ComputeDiffuseAndSpecularFromPointLights( WorldPos , OUT.PointLightColor , OUT.SpecularColor , LightDir , 0 , 2 );


	OUT.SunlightColor = g_Lights[3].Color;
	OUT.SunlightDir   = -mul(g_Lights[3].Dir.xyz,TSM);

	//Transform light into normal space.
	OUT.PointLightDir = mul(LightDir.xyz, TSM);
	OUT.EyeDir        = mul(LookAtDir.xyz, TSM);
	OUT.Pos           = mul(WorldPos, g_mVP);
	OUT.Tex0          = IN.Tex0;
	OUT.DiffuseColor  = IN.Color0*g_Material.Diffuse*EG_GetPalette(0);

	// Fog
	ExShader_ComputeFog( WorldPos , OUT.Pos , OUT.Fog , OUT.FogBgTex );

	return OUT;
}

EGVert_ExObj ExObjVS_Basic(egv_vert_mesh IN) __export(vs, "ex_mesh_basic")
{
	float DistFromCamera = 0.f;
	return ExObjVS_BasicCommon( IN , DistFromCamera );
}

float4 ExObjPS_Basic(EGVert_ExObj IN) : EG_PS_OUTPUT __export(ps, "ex_mesh_basic")
{
	float4 TextureDiffuse = EG_Sample(0, IN.Tex0)*IN.DiffuseColor; // Diffuse Texture
	float3 Normal = 2.f*(EG_Sample(1, IN.Tex0).xyz - .5f);         // Normal map
	
	float4 LightingColor = ExShader_ComputeLightColor( Normal , IN.SunlightDir , IN.SunlightColor , IN.PointLightDir , IN.PointLightColor , IN.AmbientColor ); 

	float4 FinalColor = TextureDiffuse*LightingColor + IN.EmissiveColor;

	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	OUT.a = IN.DiffuseColor.a*TextureDiffuse.a;
	return OUT;
}

EGVert_ExObj ExObjVS_BasicWithEmissive(egv_vert_mesh IN) __export(vs, "ex_mesh_basic_with_emissive")
{
	return ExObjVS_Basic( IN );
}

float4 ExObjPS_BasicWithEmissive(EGVert_ExObj IN) : EG_PS_OUTPUT __export(ps, "ex_mesh_basic_with_emissive")
{
	float4 TextureDiffuse = EG_Sample(0, IN.Tex0)*IN.DiffuseColor; // Diffuse Texture
	float3 Normal = 2.f*(EG_Sample(1, IN.Tex0).xyz - .5f);         // Normal map

	float4 LightingColor = ExShader_ComputeLightColor( Normal , IN.SunlightDir , IN.SunlightColor , IN.PointLightDir , IN.PointLightColor , IN.AmbientColor ); 

	float4 FinalColor = TextureDiffuse*LightingColor + IN.EmissiveColor*EG_Sample(3, IN.Tex0);

	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	OUT.a = IN.DiffuseColor.a*TextureDiffuse.a;
	return OUT;
}

EGVert_ExObj ExObjVS_BasicPortal(egv_vert_mesh IN) __export(vs, "ex_mesh_basic_portal")
{
	EGVert_ExObj OUT = ExObjVS_Basic( IN );

	static const int ModualteSpeed = 5000;
	static const float2 ModulateRange = float2(.9f , .97f );
	static const float ModulateAmount = ModulateRange.y - ModulateRange.x;

	float ModulateTime = (((uint)(abs(EG_GetTime())*1000))%ModualteSpeed)/float(ModualteSpeed);
	float fMod = ModulateRange.x + (abs(0.5f - ModulateTime)*2)*ModulateAmount;
	OUT.DiffuseColor.a *= fMod;

	static const float PI = 3.14159265f;

	static const int RotateSpeed = 2000;
	float RotateTime = (((uint)(abs(EG_GetTime())*1000))%RotateSpeed)/float(RotateSpeed);
	float sinX = sin( RotateTime*PI*2 );
	float cosX = cos( RotateTime*PI*2 );
	float2x2 RotMat = float2x2( cosX , -sinX , sinX , cosX );
	OUT.Tex0 = mul( (OUT.Tex0 - .5) , RotMat ) + .5;

	return OUT;
}

float4 ExObjPS_BasicPortal(EGVert_ExObj IN) : EG_PS_OUTPUT __export(ps, "ex_mesh_basic_portal")
{
	float4 TextureDiffuse = EG_Sample(0, IN.Tex0)*IN.DiffuseColor; // Diffuse Texture
	float4 FinalColor = TextureDiffuse*IN.EmissiveColor;;

	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	OUT.a = IN.DiffuseColor.a*TextureDiffuse.a;
	return OUT;
}

EGVert_ExObj ExObjVS_BasicEndOfGameObjectCore(egv_vert_mesh IN) __export(vs, "ex_mesh_end_of_game_object_core")
{
	float DistFromCamera = 0.f;
	EGVert_ExObj OUT = ExObjVS_BasicCommon( IN , DistFromCamera );

	static const int ModualteSpeed = 1500;
	static const float2 ModulateRange = float2(.8f , 1.f );
	static const float ModulateAmount = ModulateRange.y - ModulateRange.x;

	float ModulateTime = (((uint)(abs(EG_GetTime())*1000))%ModualteSpeed)/float(ModualteSpeed);
	float fMod = ModulateRange.x + (abs(0.5f - ModulateTime)*2)*ModulateAmount;
	OUT.EmissiveColor.rgb *= fMod;

	// We don't want the object too bright when fighting the final boss so we'll lighten it up
	// when we get close:
	const float StartDark = 12.f;
	const float FullDark = 15.f;
	const float DarkRange = FullDark - StartDark;
	float DistDarkenCurve = saturate((DistFromCamera-StartDark)/DarkRange);
	float DistDarken = .4f + .6f*(1.f-DistDarkenCurve);
	OUT.EmissiveColor.rgb *= DistDarken;

	return OUT;
}

float4 ExObjPS_BasicEndOfGameObjectCore(EGVert_ExObj IN) : EG_PS_OUTPUT __export(ps, "ex_mesh_end_of_game_object_core")
{
	float4 TextureDiffuse = EG_Sample(0, IN.Tex0)*IN.DiffuseColor; // Diffuse Texture
	float4 FinalColor = TextureDiffuse*IN.EmissiveColor;

	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	OUT.a = IN.DiffuseColor.a*TextureDiffuse.a;
	return OUT;
}

///////////////////////////////////////////////////////////////////////////////
//
// Billboard Shader
//
///////////////////////////////////////////////////////////////////////////////

struct EGVert_ExBillboard
{
	float4 Pos           : EG_POSITION;
	float2 Tex0          : TEXCOORD0;
	float2 FogBgTex      : TEXCOORD1;
	float4 LightColor    : COLOR0;
	float4 EmissiveColor : COLOR1;
	float4 Fog           : COLOR2;
};

EGVert_ExBillboard ExObjVS_Billboard( egv_vert_mesh IN ) __export( vs , "ex_mesh_billboard" )
{
	EG_BoneTransformThis( IN );

	EGVert_ExBillboard OUT;
	float4 FinalPos =  IN.Pos;
	float4x4 BbWV = g_mWV;

	float4 PosForLight = mul( IN.Pos , g_mW );

	// Column 0:
	BbWV[0][0] = 1;
	BbWV[0][1] = 0;
	BbWV[0][2] = 0;

	const bool CYL_BILLBOARD = true;

	if( !CYL_BILLBOARD )
	{
		// Column 1:
		BbWV[1][0] = 0;
		BbWV[1][1] = 1;
		BbWV[1][2] = 0;
	}

	// Column 2:
	BbWV[2][0] = 0;
	BbWV[2][1] = 0;
	BbWV[2][2] = 1;

	float4 LightDir;
	float4 PointLightColor;
	float4 SpecularColor;
	EG_ComputeDiffuseAndSpecularFromPointLights( PosForLight , PointLightColor , SpecularColor , LightDir , 0 , 2 );
	float4 SunlightColor = g_Lights[3].Color;
	float4 AmbientColor = EG_ComputeAmbient();

	OUT.LightColor = ExShader_ComputeLightColorForBillboard( SunlightColor , PointLightColor , AmbientColor );
	
	FinalPos = mul( FinalPos , BbWV );
	FinalPos = mul( FinalPos , g_mP );
	OUT.Pos = FinalPos;
	OUT.Tex0 = IN.Tex0;
	OUT.EmissiveColor = EG_ComputeEmissive();

	// Fog
	ExShader_ComputeFog( PosForLight , OUT.Pos , OUT.Fog , OUT.FogBgTex );

	return OUT;
}

float4 ExObjPS_Billboard( EGVert_ExBillboard IN ) __export( ps , "ex_mesh_billboard" ) : EG_PS_OUTPUT
{
	float4 TextureColor = EG_Sample( 0 , IN.Tex0 );
	float4 OUT = float4(TextureColor.rgb*IN.LightColor.rgb + IN.EmissiveColor.rgb , TextureColor.a*EG_GetPsPalette(0).a);
	OUT = ExShader_ComputeFogBlend( IN.Fog , OUT , IN.FogBgTex );
	return OUT;
}

///////////////////////////////////////////////////////////////////////////////
//
// Animated Billboard Shader
//
///////////////////////////////////////////////////////////////////////////////

EGVert_ExBillboard ExObjVS_BillboardAnimated( egv_vert_mesh IN ) __export( vs , "ex_mesh_billboard_animated" )
{
	EG_BoneTransformThis( IN );

	EGVert_ExBillboard OUT;
	float4 FinalPos =  IN.Pos;
	float4x4 BbWV = g_mWV;

	float4 PosForLight = mul( IN.Pos , g_mW );

	// Column 0:
	BbWV[0][0] = 1;
	BbWV[0][1] = 0;
	BbWV[0][2] = 0;

	const bool CYL_BILLBOARD = true;

	if( !CYL_BILLBOARD )
	{
		// Column 1:
		BbWV[1][0] = 0;
		BbWV[1][1] = 1;
		BbWV[1][2] = 0;
	}

	// Column 2:
	BbWV[2][0] = 0;
	BbWV[2][1] = 0;
	BbWV[2][2] = 1;

	float4 LightDir;
	float4 PointLightColor;
	float4 SpecularColor;
	EG_ComputeDiffuseAndSpecularFromPointLights( PosForLight , PointLightColor , SpecularColor , LightDir , 0 , 2 );
	float4 SunlightColor = g_Lights[3].Color;
	float4 AmbientColor = EG_ComputeAmbient();

	OUT.LightColor = ExShader_ComputeLightColorForBillboard( SunlightColor , PointLightColor , AmbientColor );

	FinalPos = mul( FinalPos , BbWV );
	FinalPos = mul( FinalPos , g_mP );
	OUT.Pos = FinalPos;
	OUT.Tex0 = IN.Tex0;
	OUT.EmissiveColor = EG_ComputeEmissive();

	uint nFrame = g_vs_EntInts0.x;

	EG_SelectAnimFrame( OUT.Tex0 , IN.Tex0 , nFrame , 4 , 4 );

	// Fog
	ExShader_ComputeFog( PosForLight , OUT.Pos , OUT.Fog , OUT.FogBgTex );

	return OUT;
}

float4 ExObjPS_BillboardAnimated( EGVert_ExBillboard IN ) __export( ps , "ex_mesh_billboard_animated" ) : EG_PS_OUTPUT
{
	float4 TextureColor = EG_Sample( 0 , IN.Tex0 );
	float4 OUT = float4(TextureColor.rgb*IN.LightColor.rgb + IN.EmissiveColor.rgb , TextureColor.a*EG_GetPsPalette(0).a);
	OUT = ExShader_ComputeFogBlend( IN.Fog , OUT , IN.FogBgTex );
	return OUT;
}


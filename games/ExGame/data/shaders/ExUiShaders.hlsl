// (c) 2017 Beem Media

#include "ExSharedShaders.hlsli"

egv_vert_mesh VSExUi( egv_vert_mesh IN ) __export( vs , "ex_ui" )
{
	EG_BoneTransformThis( IN );

	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);

	return IN;
}

float4 PSExUi(egv_vert_mesh IN) : EG_PS_OUTPUT __export( ps , "ex_ui" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

egv_vert_mesh VSExUiMap( egv_vert_mesh IN ) __export( vs , "ex_uimap" )
{
	EG_BoneTransformThis( IN );

	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);

	return IN;
}

float4 PSExUiMap(egv_vert_mesh IN) : EG_PS_OUTPUT __export( ps , "ex_uimap" )
{
	return EG_SampleGlobal( 0 , IN.Tex0 )*IN.Color0;
}


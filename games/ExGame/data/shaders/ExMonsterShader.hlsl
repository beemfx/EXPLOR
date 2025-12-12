// (c) 2017 Beem Media

#include "ExSharedShaders.hlsli"

egv_vert_mesh VSExUiMonster( egv_vert_mesh IN ) __export( vs , "SHD_VS_UI_Monster" )
{
	const uint SPRITE_COLUMNS=4;
	const uint SPRITE_ROWS=4;
	
	EG_BoneTransformThis( IN );

	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette(0);

	uint nFrame = g_vs_EntInts0.x;

	EG_SelectAnimFrame( IN.Tex0 , IN.Tex0 , nFrame , SPRITE_COLUMNS , SPRITE_ROWS );

	return IN;
}

float4 PSExUiMonster( egv_vert_mesh IN ) : EG_PS_OUTPUT __export( ps , "SHD_PS_UI_Monster" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

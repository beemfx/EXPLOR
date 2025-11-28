#include <base.hlsli>
#include <bone.hlsli>

egv_vert_mesh VSExplorButtonHighlighter( egv_vert_mesh IN ) __export( vs , "Explor_Button_Hl" )
{
	// Pretty standard transform:
	EG_BoneTransformThis( IN );
	IN.Pos = mul(IN.Pos , g_mWVP);

	// Color based on palette to make it transparent:
	IN.Color0 = IN.Color0*g_Material.Diffuse*EG_GetPalette( 0 );
	// Plus a little bit of pulsating:
	const float PULSE_AMOUNT = .08;
	IN.Color0.rgb *= (1.+PULSE_AMOUNT) + PULSE_AMOUNT*sin( .7 * 3.1415*EG_GetTime() + IN.Tex0.x );
	//IN.Color0.rgb += PULSE_AMOUNT + PULSE_AMOUNT*sin( .2 * 3.1415*EG_GetTime() + IN.Tex0.x );

	return IN;
}

float4 PSExplorButtonHighlighter( egv_vert_mesh IN ) : EG_PS_OUTPUT __export( ps , "Explor_Button_Hl" )
{
	float4 c = EG_Sample( 0 , IN.Tex0 ) * IN.Color0 ;
	return c;
}

egv_vert_mesh VSExUiFadeImage( egv_vert_mesh IN ) __export( vs , "ex_ui_fade_image" )
{
	EG_BoneTransformThis( IN );

	IN.Pos = mul( IN.Pos, g_mWVP );
	IN.Color0 = g_Material.Diffuse*IN.Color0*EG_GetPalette( 0 );

	return IN;
}

float4 PSExUiFadeImage( egv_vert_mesh IN ) : EG_PS_OUTPUT __export( ps , "ex_ui_fade_image" )
{
	return EG_Sample( 0 , IN.Tex0 )*IN.Color0;
}

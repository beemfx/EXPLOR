// (c) 2017 Beem Media

#include <base.hlsli>

egv_vert_simple VS_Texture(egv_vert_simple IN) __export( vs , "ex_world_bg" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	return OUT;
}

float4 PS_Texture(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "ex_world_bg" )
{
	// float c = EG_SampleGlobal(2, IN.Tex0 ).r;
	// c = (c-.95f)/(1.f-.95f);

	float4 Bg = EG_SampleGlobal( 0 , IN.Tex0 );
	float4 Fg = EG_SampleGlobal( 1 , IN.Tex0 );
	
	float Ratio = Fg.a;

	float4 OUT = (Fg*Ratio + Bg*(1.f - Ratio ))*IN.Color0;
	OUT.a = 1.f;
	return OUT;
}

egv_vert_simple VS_CombatPost(egv_vert_simple IN) __export( vs , "ex_combat_post" )
{
	egv_vert_simple OUT = IN;
	OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	return OUT;
}

float PS_CombatPost_Transform( in float yValue , in const int NUM_DISCRETE , in const float OFFSET_MAX )
{
	float ToSegment = yValue*NUM_DISCRETE;
	// We want to discretely break the vertical apart so convert to int
	int Segment = int(ToSegment);
	bool bEvenSegment = (Segment&1) == 0;
	float BaseSegmentOffset = ToSegment - Segment;
	float SegmentIntensity = bEvenSegment ? 1.f-BaseSegmentOffset : BaseSegmentOffset;

	float xOffset = g_ps_GamePalette0.y*OFFSET_MAX*SegmentIntensity;
	return xOffset;
}

float4 PS_BlurSampleX( float2 Tex0 , float Dist )
{
	float4 t = float4(0,0,0,0);
	for( int i=-4; i<=4; i++ )
	{
		float2 Tc = float2( Tex0.x + i*Dist , Tex0.y );
		t += EG_SampleGlobal(0, Tc);
	}
	t/=9.f;
	t.w = 1.f;
	return t;
}

float4 PS_BlurSampleY( float2 Tex0 , float Dist )
{
	float4 t = float4(0,0,0,0);
	for( int i=-4; i<=4; i++ )
	{
		float2 Tc = float2( Tex0.x , Tex0.y + i*Dist );
		t += EG_SampleGlobal(0, Tc);
	}
	t/=9.f;
	t.w = 1.f;
	return t;
}

float4 PS_CombatPost(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "ex_combat_post" )
{	
	// float xOffset1 = PS_CombatPost_Transform( IN.Tex0.y , 16 , .01f );

	float2 TexAdj = IN.Tex0;
	// TexAdj.x += (xOffset1);
	
	float4 Bg = PS_BlurSampleX( TexAdj , g_ps_GamePalette0.x*g_ps_GamePalette0.z );// EG_SampleGlobal( 0 , TexAdj );
	float4 OUT = float4(Bg.rgb,1);

	// Apply brightness.
	float Brightness = -.2f*g_ps_GamePalette0.x;
	OUT.rgb += Brightness;

	// Apply contrast.
	float Contrast = 1.0 + .1*g_ps_GamePalette0.x;
	OUT.rgb = saturate(((OUT.rgb - 0.5f) * max(Contrast, 0)) + 0.5f);

	OUT.a = 1.f;
	return OUT;
}

egv_vert_simple VS_Dof1Post(egv_vert_simple IN) __export( vs , "ex_dof1_post" )
{
	IN.Pos.z = g_vs_GamePalette0.x; // Set z value to the starting depth.
	return IN; // No transform needed.
	// egv_vert_simple OUT = IN;
	// OUT.Pos = mul(IN.Pos, g_mWVP); //Final position of vertex.
	// return OUT;
}

float4 PS_Dof1Post(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "ex_dof1_post" )
{
	float  fDepth = EG_SampleGlobal(1, IN.Tex0).r;

	if( /*g_ps_GamePalette0.x < fDepth &&*/ fDepth < g_ps_GamePalette0.y)
	{
		const float Dist = g_ps_GamePalette0.w*saturate((fDepth-g_ps_GamePalette0.x)/(g_ps_GamePalette0.z-g_ps_GamePalette0.x));
		return PS_BlurSampleX( IN.Tex0 , Dist );
	}
	float4 t = EG_SampleGlobal(0, IN.Tex0);
	return t;
}

egv_vert_simple VS_Dof2Post(egv_vert_simple IN) __export( vs , "ex_dof2_post" )
{
	return VS_Dof1Post( IN );
}

float4 PS_Dof2Post(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "ex_dof2_post" )
{	
	float  fDepth = EG_SampleGlobal(1, IN.Tex0).r;

	if( /*g_ps_GamePalette0.x < fDepth &&*/ fDepth < g_ps_GamePalette0.y)
	{
		const float Dist = g_ps_GamePalette0.w*saturate((fDepth-g_ps_GamePalette0.x)/(g_ps_GamePalette0.z-g_ps_GamePalette0.x));
		return PS_BlurSampleY( IN.Tex0 , Dist );
	}
	float4 t = EG_SampleGlobal(0, IN.Tex0);
	return t;
}

egv_vert_simple VS_ColorPost(egv_vert_simple IN) __export( vs , "ex_color_post" )
{
	return IN;
}

float4 PS_ColorPost(egv_vert_simple IN) : EG_PS_OUTPUT __export( ps , "ex_color_post" )
{
	float4 ColorOut = EG_SampleGlobal(0,IN.Tex0);
	ColorOut.rgb *= g_ps_GamePalette0.x;
	return ColorOut;
}


// (c) 2020 Beem Media

#include "ExSharedShaders.hlsli"

struct EGVert_Flame
{
	float4 Pos : EG_POSITION;
	float2 Tex0 : TEXCOORD0;
	float2 FogBgTex : TEXCOORD1;
	float4 Fog : COLOR0;
};


EGVert_Flame VS( egv_vert_mesh IN ) __export( vs , "SHD_VS_Flame" )
{
	const uint SPRITE_COLUMNS=5;
	const uint SPRITE_ROWS=4;
	const uint NUM_FRAMES=SPRITE_COLUMNS*SPRITE_ROWS;
	const int SPEED_FACTOR=60;
	const float MESH_SCALE=.2f; // TODO: Shouldn't be hard coded, should just respect original size, but the billboarding would have to be done in game and not in the shader.
	
	EGVert_Flame OUT;
	
	//Do a translation of the origin to get the location of the vertex
	//in view space.
	float4 WorldPos = mul(IN.Pos, g_mW);
	float4x4 mWV = mul(g_mW, g_mV);
	OUT.Pos = mul(float4(0, 0, 0, 1), mWV);
	//Then adjust by the actual vertex x,y position.
	OUT.Pos.xy += IN.Pos.xy*MESH_SCALE;
	//Finally multiply by the projection matrix.
	OUT.Pos = mul(OUT.Pos, g_mP);
	
	
	//There are 20 frames in this animation, so we decide which one we want.
	//We animate for 1 second.
	float FinalCycleTime = EG_GetOneMinuteCycleTime()*SPEED_FACTOR;
	uint nFrame = (uint)(NUM_FRAMES*abs(FinalCycleTime));

	EG_SelectAnimFrame( OUT.Tex0 , IN.Tex0 , nFrame , SPRITE_COLUMNS , SPRITE_ROWS );

	// Fog
	ExShader_ComputeFog( WorldPos , OUT.Pos , OUT.Fog , OUT.FogBgTex );

	return OUT;
}

float4 PS( EGVert_Flame IN ) : EG_PS_OUTPUT __export( ps , "SHD_PS_Flame" )
{
	float4 FinalColor = EG_Sample(0, IN.Tex0);
	float4 OUT = ExShader_ComputeFogBlend( IN.Fog , FinalColor , IN.FogBgTex );
	return OUT;
}

struct EGVert_Flame_UI
{
	float4 Pos : EG_POSITION;
	float2 Tex0 : TEXCOORD0;
};

EGVert_Flame_UI VS_UI( egv_vert_mesh IN ) __export( vs , "SHD_VS_Flame_UI" )
{
	const uint SPRITE_COLUMNS=5;
	const uint SPRITE_ROWS=4;
	const uint NUM_FRAMES=SPRITE_COLUMNS*SPRITE_ROWS;
	const int SPEED_FACTOR=60;

	EGVert_Flame_UI OUT;

	EG_BoneTransformThis( IN );
	OUT.Pos = mul(IN.Pos, g_mWVP);

	//There are 20 frames in this animation, so we decide which one we want.
	//We animate for 1 second.
	float FinalCycleTime = EG_GetOneMinuteCycleTime()*SPEED_FACTOR;
	uint nFrame = (uint)(NUM_FRAMES*abs(FinalCycleTime));

	EG_SelectAnimFrame( OUT.Tex0 , IN.Tex0 , nFrame , SPRITE_COLUMNS , SPRITE_ROWS );

	return OUT;
}

float4 PS_UI( EGVert_Flame_UI IN ) : EG_PS_OUTPUT __export( ps , "SHD_PS_Flame_UI" )
{
	float4 FinalColor = EG_Sample(0, IN.Tex0);
	return FinalColor;
}

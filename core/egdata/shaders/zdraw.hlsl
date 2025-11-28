#include <base.hlsli>
#include <bone.hlsli>

struct EGVert_ZDraw
{
	float4 Pos   : EG_POSITION;
	float4 Depth : FOG;
};

//The Z-Depth method just writes the z-depth in screen space. So the depth values are not relative to the eye.
EGVert_ZDraw VS_ByZDepth( egv_vert_mesh IN ) __export( vs , "drawtoz" )
{
	EGVert_ZDraw OUT;
	EG_BoneTransformThis(IN);
	OUT.Pos = mul(IN.Pos, g_mWVP);
	OUT.Depth = OUT.Pos;
	//OUT.Depth = float4(0,0,OUT.Pos.z/OUT.Pos.w, 1);
	return OUT;
}


float4 PS_ByZDepth( EGVert_ZDraw IN ) : EG_PS_OUTPUT __export( ps , "drawtoz" )
{
	float c = IN.Depth.z/IN.Depth.w;
	return float4(c, 0, 0, 1);
}


//We'll be getting both mesh and map vertices for this, but we're only concerned 
//with the position, so let us only write that.

struct EGVert_ZClear
{
	float4 Pos : EG_POSITION;
};

EGVert_ZClear VS( egv_vert_simple IN ) __export( vs , "clearz" )
{
	EGVert_ZClear OUT;
	OUT.Pos = mul(IN.Pos, g_mWVP);
	return OUT;
}

float4 PS( EGVert_ZClear IN ) : EG_PS_OUTPUT __export( ps , "clearz" )
{
	//return float4(0.f, 0, 0, 1);
	return float4(g_ps_reg0.x, 0, 0, 1);
}

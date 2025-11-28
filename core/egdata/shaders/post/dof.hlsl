#include <base.hlsli>

//We just need a basic vertex shader (we don't even need to transform anything).

struct VS_OUT
{
	float4 Pos : EG_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUT VS_p1(egv_vert_mesh IN) __export( vs , "dof_p1" )
{
	VS_OUT OUT;
	OUT.Pos = IN.Pos;
	OUT.Tex = IN.Tex0;

	return OUT;
}

VS_OUT VS_p2(egv_vert_mesh IN) __export( vs , "dof_p2" )
{
	VS_OUT OUT;
	OUT.Pos = IN.Pos;
	OUT.Tex = IN.Tex0;

	return OUT;
}

float4 PS_old(VS_OUT IN) : EG_PS_OUTPUT
{
	float4 t = EG_Sample(0, IN.Tex);
	float  fDepth = EG_Sample(1, IN.Tex).r;
	return (fDepth > g_ps_reg0.x) ? float4(t.r, t.r, 0, 1.0) : float4(t.r, t.g, t.b, 1.0);
}

float PerspectiveDepth(float d, float2 Tex)
{
	//This transformation should probably be done in the vertex shader of the z-renderer, so the z buffer texture
	//since this is a ridiculous amount of sqrts.
	
	//represents the distance from the camera.
	
	//We are going to assume a FOV of 90, so the actual x coordinate is given by..
	
	//Put texture coordinates in the range of [-1,-1] to [1,1]
	float2 ViewCoord=2*(Tex.xy-float2(0.5,0.5));
	//The DOF 90 transform (probably should pass in depth of field in g_ps_reg0)
	ViewCoord = ViewCoord*d;
	//Now just use the pythagorean theorem.
	return sqrt(ViewCoord.x*ViewCoord.x + ViewCoord.y*ViewCoord.y + d*d);
	
}

//Vertical Pass
float4 PS_p1(VS_OUT IN) : EG_PS_OUTPUT __export( ps , "dof_p1" )
{
	float  fDepth = EG_Sample(1, IN.Tex).r;
	fDepth = PerspectiveDepth(fDepth, IN.Tex);
	
	if(fDepth > g_ps_reg0.x)
	{
		IN.Tex.y = IN.Tex.y + (sin(IN.Tex.x*200)*0.01*sin(g_ps_reg0.y+0.2));
		IN.Tex.x = IN.Tex.x;// + (sin(IN.Tex.y*300)*0.02*sin(g_ps_reg0.y + 3.1415/5));
	}
	float4 t = EG_Sample(0, IN.Tex);
	return t;
}

//Horizontal pass:
float4 PS_p2(VS_OUT IN) : EG_PS_OUTPUT __export( ps , "dof_p2" )
{
	float  fDepth = EG_Sample(1, IN.Tex).r;
	fDepth = PerspectiveDepth(fDepth, IN.Tex);

	if(fDepth > g_ps_reg0.x)
	{
		IN.Tex.y = IN.Tex.y;// + (sin(IN.Tex.x*200)*0.01*sin(g_ps_reg0.y+3.1415/2));
		IN.Tex.x = IN.Tex.x+ (sin(IN.Tex.y*300)*0.01*sin(g_ps_reg0.y + 3.1415/2));
	}
	float4 t = EG_Sample(0, IN.Tex);
	return t;
}
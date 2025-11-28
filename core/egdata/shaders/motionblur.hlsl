//
// Motion Blur Post-Processing
// (c) 2016 Beem Media
#include <base.hlsli>
#include <post/default.hlsl>

struct EGVert_MotionBlur
{
	float4 Pos  : EG_POSITION;
	float2 Tex0 : TEXCOORD0;
};


EGVert_MotionBlur VS( egv_vert_simple IN ) __export( vs , "motionblur" )
{
	const float2 Adj = g_vs_reg0.xy;
	EGVert_MotionBlur OUT;
	OUT.Pos = IN.Pos;

	#if 0
	OUT.Tex0 = IN.Tex0;
	#else
	OUT.Tex0.x =  IN.Tex0.x + EG_ADJUST*Adj.x;
	OUT.Tex0.y =  IN.Tex0.y + EG_ADJUST*Adj.y;
	#endif
	return OUT;
}

float4 PS( EGVert_MotionBlur IN ) : EG_PS_OUTPUT __export( ps , "motionblur" )
{
	// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch27.html

	//
	// Part 1
	//

	// Get the depth buffer value at this pixel.  
	float zOverW = EG_SampleGlobal( 1 , IN.Tex0 ).r;  
	// H is the viewport position at this pixel in the range -1 to 1.  
	float4 H = float4(IN.Tex0.x * 2 - 1, (1 - IN.Tex0.y) * 2 - 1,  zOverW, 1);  
	// Transform by the view-projection inverse.  
	float4 D = mul(H, g_ps_InvVP);  
	// Divide by w to get the world position.  
	float4 worldPos = D / D.w; 

	//
	// Part 2
	//

	// Current viewport position  
	float4 currentPos = H;  
	// Use the world position, and transform by the previous view-  
	// projection matrix.  
	float4 previousPos = mul(worldPos, g_ps_PrevVP);  
	// Convert to nonhomogeneous points [-1,1] by dividing by w.  
	previousPos /= previousPos.w;  
	// Use this frame's position and last frame's to compute the pixel  
	// velocity.  
	float2 velocity = ((currentPos - previousPos)/2.f).xy;
	velocity.x = -velocity.x;

	//
	// Part 3
	//

	// Get the initial color at this pixel.  
	const int numSamples = 4;
	const int downScale = numSamples;
	float2 texCoord = IN.Tex0;
	float4 color = EG_SampleGlobal(0 , texCoord );
	texCoord += velocity/downScale;  
	for(int i = 1; i < numSamples; ++i, texCoord += velocity/downScale)  
	{  
		// Sample the color buffer along the velocity vector.  
		float4 currentColor = EG_SampleGlobal(0, texCoord);  
		// Add the current color to our color sum.  
		color += currentColor;
	}  
	// Average all of the samples to get the final blur color.  
	float4 finalColor = color / numSamples; 

	return finalColor;
}

// (c) 2018 Beem Media

#pragma once

struct egNetSmoothPose
{
	eg_transform Pose;
	eg_uint      TimeMs;

	egNetSmoothPose() = default;
	egNetSmoothPose( eg_ctor_t Ct ): Pose( Ct ){ if( Ct == CT_Default || Ct == CT_Clear ) { TimeMs = 0; } }
};

struct egNetSmoothTransform
{
	eg_transform    Pose;        // Pose that should be accessed
	egNetSmoothPose RecPose;     // The Pose received from the server. (client only)
	egNetSmoothPose SmoothPose0; // The pose previously received from the server. (client only)
	egNetSmoothPose SmoothPose1;
	eg_real         SmoothTime;
	eg_real         SmoothRate;

	egNetSmoothTransform() = default;
	egNetSmoothTransform( eg_ctor_t Ct );

	eg_bool Smooth( eg_real DeltaTime , eg_real UpdateRate );

	const eg_transform& operator = ( const eg_transform& rhs );
	operator const eg_transform& () const { return Pose; }
};

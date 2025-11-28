// (c) 2018 Beem Media

#include "EGNetSmoothTransform.h"

egNetSmoothTransform::egNetSmoothTransform( eg_ctor_t Ct )
: Pose( Ct )
, RecPose( Ct )
, SmoothPose0( Ct )
, SmoothPose1( Ct )
{
	if( Ct == CT_Clear || Ct == CT_Default )
	{
		SmoothTime = 0.f;
		SmoothRate = 0.f;
	}
}

eg_bool egNetSmoothTransform::Smooth( eg_real DeltaTime , eg_real UpdateRate )
{
	eg_bool bComplete = false;

	SmoothTime += DeltaTime;
	if( SmoothTime > SmoothRate )
	{
		SmoothTime = 0;
		SmoothPose0 = SmoothPose1;
		SmoothPose1 = RecPose;
		SmoothRate = EG_Min( UpdateRate , (SmoothPose1.TimeMs - SmoothPose0.TimeMs)/1000.f );
	}

	if( 0 == SmoothRate || 0 == SmoothPose1.TimeMs )
	{
		Pose = SmoothPose1.Pose;
		bComplete = true;
	}
	else
	{
		eg_real t = EGMath_GetMappedRangeValue( SmoothTime , eg_vec2(0,SmoothRate),eg_vec2(0,1.f));
		Pose = eg_transform::Lerp( SmoothPose0.Pose , SmoothPose1.Pose , t );
	}

	return bComplete;
}

const eg_transform& egNetSmoothTransform::operator = ( const eg_transform& rhs )
{
	Pose = rhs;

	RecPose.Pose = Pose;
	RecPose.TimeMs = 0;
	SmoothPose0 = RecPose;
	SmoothPose1 = RecPose;

	SmoothRate = 0;
	SmoothTime = 0;

	return Pose;
};

// (c) 2020 Beem Media. All Rights Reserved.

#include "EGTransform.h"

const eg_transform eg_transform::I = eg_transform::BuildRotationScalingTranslation( eg_quat(0.f,0.f,0.f,1.f) , 1.f , eg_vec3(0.f,0.f,0.f) );

eg_bool eg_transform::IsValid() const
{
	return EG_IsEqualEps( R.LenSq() , 1.f , EG_SMALL_EPS ) && s > 0.f;
}

eg_bool eg_transform::IsIdentity() const
{
	return *this == I;
}

eg_bool eg_transform::IsNormalized() const
{
	// What does a transform being normalized even mean?
	return R.IsNormalized() && s == 1.f;
}

eg_transform eg_transform::BuildInverse( const eg_transform& T )
{
	assert( T.IsValid() ); // Not going to work with a scale of 0

	eg_transform Out;

	Out.R.x = -T.R.x;
	Out.R.y = -T.R.y;
	Out.R.z = -T.R.z;
	Out.R.w = T.R.w;

	Out.s = 1.f/T.s;

	// Transform the position by the rotation portion,
	// then invert.
	Out.T = T.T*Out.R;
	Out.T.x = -Out.s*Out.T.x;
	Out.T.y = -Out.s*Out.T.y;
	Out.T.z = -Out.s*Out.T.z;

	Out.R.NormalizeThis();

	return Out;
}

eg_transform eg_transform::BuildFromMatrix( const eg_mat& M )
{
	assert( M.IsORT() ); // This formula is not even correct.
	eg_transform Out;

	eg_mat Temp( M );
	Temp._41 = 0.f;
	Temp._42 = 0.f;
	Temp._43 = 0.f;
	Out.R = eg_quat::BuildRotation( Temp );

	Out.T = eg_vec3(M._41,M._42,M._43);

	Out.s = 1.f; // Not decomposing scale?

	return Out;
}

eg_transform eg_transform::Lerp( const eg_transform& Lhs , const eg_transform& Rhs , eg_real t )
{
	eg_transform Out;
	// assert(0 <= t && t <= 1.0f);

	Out.R = Lhs.R.Slerp(Rhs.R, t);
	Out.R.NormalizeThis();

	Out.T.x = EGMath_Lerp( t , Lhs.T.x , Rhs.T.x );
	Out.T.y = EGMath_Lerp( t , Lhs.T.y , Rhs.T.y );
	Out.T.z = EGMath_Lerp( t , Lhs.T.z , Rhs.T.z );

	Out.s = EGMath_Lerp( t , Lhs.s , Rhs.s );

	return Out;
}

eg_transform eg_transform::CatmullRomLerp( const eg_transform& LhsC , const eg_transform& Lhs , const eg_transform& Rhs , const eg_transform& RhsC , eg_real t )
{
	using namespace DirectX;
	
	eg_transform Out;

	{
		XMVECTOR q1 = XMLoadFloat4(LhsC.R);
		XMVECTOR q2 = XMLoadFloat4(Lhs.R);
		XMVECTOR q3 = XMLoadFloat4(Rhs.R);
		XMVECTOR q4 = XMLoadFloat4(RhsC.R);

		XMVECTOR qs1,qs2,qs3;
		XMQuaternionSquadSetup( &qs1 , &qs2 , &qs3 , q1 , q2 , q3 , q4 );
		XMVECTOR CalcPos = XMQuaternionSquad( q2 , qs1 , qs2 , qs3 , t );
		XMStoreFloat4( Out.R , CalcPos );
	}

	{
		XMVECTOR v1 = XMLoadFloat4(eg_vec4(LhsC.T,LhsC.s));
		XMVECTOR v2 = XMLoadFloat4(eg_vec4(Lhs.T,Lhs.s));
		XMVECTOR v3 = XMLoadFloat4(eg_vec4(Rhs.T,Rhs.s));
		XMVECTOR v4 = XMLoadFloat4(eg_vec4(RhsC.T,RhsC.s));
		XMVECTOR CalcPos = XMVectorCatmullRom( v1 , v2 , v3 , v4 , t );
		XMStoreFloat4( *reinterpret_cast<eg_vec4*>(&Out.T) , CalcPos ); // This covers case of scaling as well that's why we reinterpret_cast
	}

	return Out;
}

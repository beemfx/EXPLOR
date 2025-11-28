// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

eg_mat EGMath_ClipProjectionMatrix( const eg_mat& matProj , const eg_plane& ClipPlaneViewSpace );
eg_mat EGMath_ClipProjectionMatrix( const eg_mat& matView , const eg_mat& matProj , const eg_plane& clip_plane );

eg_real EGMath_sqrt( eg_real f );
eg_real EGMath_exp( eg_real f );
eg_real EGMath_log( eg_real f );
eg_real EGMath_sin( eg_angle f );
eg_real EGMath_cos( eg_angle f );
eg_real EGMath_tan( eg_angle f );
eg_angle EGMath_atan( eg_real f );
eg_angle EGMath_atan2( eg_real y , eg_real x );
eg_int EGMath_floor( eg_real f );
eg_int64 EGMath_floor( eg_real64 f );
eg_int EGMath_round( eg_real f );
eg_int EGMath_round( eg_real64 f );
eg_int EGMath_ceil( eg_real f );
eg_real EGMath_pow( eg_real b , eg_real e );
eg_real64 EGMath_pow( eg_real64 b , eg_real64 e );

template<class T>
static inline T EGMath_Square( const T& In ) { return In * In; }

eg_real EGMath_Lerp( eg_real t , eg_real Min , eg_real Max );
eg_real EGMath_GetMappedRangeValue( eg_real t , const eg_vec2& FromSpace , const eg_vec2& ToSpace );

template< class T , class U >
static inline T EGMath_CubicInterp( const T& P0 , const T& T0 , const T& P1 , const T& T1 , const U& A )
{
	const float A2 = A * A;
	const float A3 = A2 * A;

	return (T)(((2 * A3) - (3 * A2) + 1) * P0) + ((A3 - (2 * A2) + A) * T0) + ((A3 - A2) * T1) + (((-2 * A3) + (3 * A2)) * P1);
}

static inline eg_real EGMath_GetMappedCubicRangeValueNormalized( eg_real t , const eg_vec2& FromSpace , const eg_vec2& ToSpace )
{
	return EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , EGMath_GetMappedRangeValue( t , FromSpace , ToSpace ) );
}

static inline eg_real EGMath_GetMappedCubicRangeValue( eg_real t , const eg_vec2& FromSpace , const eg_vec2& ToSpace )
{
	const eg_real NormalizedT = EGMath_GetMappedRangeValue( t , FromSpace , eg_vec2( 0.f , 1.f ) );
	const eg_real NormalizedTerp = EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , NormalizedT );
	const eg_real FinalTerp = EGMath_GetMappedRangeValue( NormalizedTerp , eg_vec2( 0.f , 1.f ) , ToSpace );
	return FinalTerp;
}

eg_vec4 EGMath_CatmullRomInterpolation( const eg_vec4 Vecs[4] , eg_real t );

template<class T> static inline T EG_Abs( T v1 )
{
	return v1 < 0 ? -v1 : v1;
}

static inline eg_bool EG_IsEqualEps( eg_real f1 , eg_real f2 , eg_real Eps = EG_SMALL_EPS )
{
	return EG_Abs( f1 - f2 ) < Eps;
}

/*********************************
*** Basic number manipulation. ***
*********************************/
template<class T> static inline T EG_Max( T v1 , T v2 )
{
	return ((v1) > (v2)) ? (v1) : (v2);
}

template<class T> static inline T EG_Min( T v1 , T v2 )
{
	return ((v1) < (v2)) ? (v1) : (v2);
}
template<class T> static inline T EG_Min( T v1 , T v2 , T v3 )
{
	return EG_Min( v1 , EG_Min( v2 , v3 ) );
}

template<class T> static inline T EG_Clamp( T v1 , T min , T max )
{
	return ((v1) > (max) ? (max) : (v1) < (min) ? (min) : (v1));
}

template<class T> static inline eg_bool EG_IsBetween( T v1 , T min , T max )
{
	return (v1 >= min) && (v1 <= max);
}

template<class T> static inline eg_bool EG_IsBetweenExclusive( T v1 , T min , T max )
{
	return (v1 > min) && (v1 < max);
}

template<class T> static inline void EG_Swap( T& v1 , T& v2 )
{
	T Temp = v1;
	v1 = v2;
	v2 = Temp;
}

eg_real EGMath_VolumeToLinear( eg_real f );
eg_real EGMath_VolumeFromLinear( eg_real f );

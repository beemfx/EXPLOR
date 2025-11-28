// (c) 2020 Beem Media. All Rights Reserved.

#include "EGMath.h"


#include "EGMath.h"

eg_real EGMath_Lerp( eg_real t , eg_real Min , eg_real Max )
{
	return Min + t * (Max - Min); //Shorthand for EGMath_Lerp( t , eg_vec2(0,1), eg_vec2(Min,Max) );
}

eg_real EGMath_GetMappedRangeValue( eg_real t , const eg_vec2& FromSpace , const eg_vec2& ToSpace )
{
	return ToSpace.x + (t - FromSpace.x) * (ToSpace.y - ToSpace.x) / (FromSpace.y - FromSpace.x);
}

eg_vec4 EGMath_CatmullRomInterpolation( const eg_vec4 Vecs[4] , eg_real t )
{
	DirectX::XMVECTOR v1 = DirectX::XMLoadFloat4( reinterpret_cast<const DirectX::XMFLOAT4*>(&Vecs[0]) );
	DirectX::XMVECTOR v2 = DirectX::XMLoadFloat4( reinterpret_cast<const DirectX::XMFLOAT4*>(&Vecs[1]) );
	DirectX::XMVECTOR v3 = DirectX::XMLoadFloat4( reinterpret_cast<const DirectX::XMFLOAT4*>(&Vecs[2]) );
	DirectX::XMVECTOR v4 = DirectX::XMLoadFloat4( reinterpret_cast<const DirectX::XMFLOAT4*>(&Vecs[3]) );
	DirectX::XMVECTOR CalcPos = DirectX::XMVectorCatmullRom( v1 , v2 , v3 , v4 , t );
	eg_vec4 ResultPos;
	DirectX::XMStoreFloat4( reinterpret_cast<DirectX::XMFLOAT4*>(&ResultPos) , CalcPos );
	return ResultPos;
}

eg_mat EGMath_ClipProjectionMatrix( const eg_mat& matProj , const eg_plane& ClipPlaneViewSpace )
{
	//http://www.nvidia.com/object/oblique_frustum_clipping.html
	// clip_plane is plane definition (view space)
	eg_plane clip_plane = ClipPlaneViewSpace;

	eg_vec4 clipPlane( clip_plane.a , clip_plane.b , clip_plane.c , clip_plane.d );

	// transform clip plane into projection space
	eg_vec4 projClipPlane = clipPlane;

	if( projClipPlane.w == 0 )  // or less than a really small value
	{
		// plane is perpendicular to the near plane
		return std::move( matProj );
	}


	if( projClipPlane.w > 0 )
	{
		// flip plane to point away from eye
		eg_vec4 clipPlane( -clip_plane.a , -clip_plane.b , -clip_plane.c , -clip_plane.d );

		// transform clip plane into projection space
		projClipPlane = clipPlane;

	}

	eg_mat matClipProj = CT_Default;
	// put projection space clip plane in Z column
	matClipProj._13 = projClipPlane.x;
	matClipProj._23 = projClipPlane.y;
	matClipProj._33 = projClipPlane.z;
	matClipProj._43 = projClipPlane.w;

	// multiply into projection matrix
	eg_mat projClipMatrix = matProj * matClipProj;
	return std::move( projClipMatrix );
}

eg_mat EGMath_ClipProjectionMatrix( const eg_mat& matView , const eg_mat& matProj , const eg_plane& _clip_plane )
{
	//http://www.nvidia.com/object/oblique_frustum_clipping.html
	eg_mat WorldToProjection = matView * matProj;

	// clip_plane is plane definition (world space)
	eg_plane clip_plane = _clip_plane;
	clip_plane.Normalize();
	WorldToProjection *= eg_mat::BuildInverse( WorldToProjection , nullptr );
	WorldToProjection *= eg_mat::BuildTranspose( WorldToProjection );

	eg_vec4 clipPlane( clip_plane.a , clip_plane.b , clip_plane.c , clip_plane.d );

	// transform clip plane into projection space
	eg_vec4 projClipPlane = clipPlane * WorldToProjection; //Reversed
	
	if( projClipPlane.w == 0 )  // or less than a really small value
	{
		// plane is perpendicular to the near plane
		return std::move( matProj );
	}


	if( projClipPlane.w > 0 )
	{
		// flip plane to point away from eye
		eg_vec4 clipPlane( -clip_plane.a , -clip_plane.b , -clip_plane.c , -clip_plane.d );

		// transform clip plane into projection space
		projClipPlane = clipPlane * WorldToProjection; //Reversed

	}

	eg_mat matClipProj = CT_Default;
	// put projection space clip plane in Z column
	matClipProj._13 = projClipPlane.x;
	matClipProj._23 = projClipPlane.y;
	matClipProj._33 = projClipPlane.z;
	matClipProj._43 = projClipPlane.w;

	// multiply into projection matrix
	eg_mat projClipMatrix = matProj * matClipProj;
	return std::move( projClipMatrix );
}

eg_real EGMath_sqrt( eg_real f )
{
	return sqrtf( f );
}

eg_real EGMath_exp( eg_real f )
{
	return expf( f );
}

eg_real EGMath_log( eg_real f )
{
	return logf( f );
}

eg_real EGMath_sin( eg_angle f )
{
	return sinf( f.ToRad() );
}

eg_real EGMath_cos( eg_angle f )
{
	return cosf( f.ToRad() );
}

eg_real EGMath_tan( eg_angle f )
{
	return tanf( f.ToRad() );
}

eg_angle EGMath_atan( eg_real f )
{
	return eg_angle::FromRad( atanf( f ) );
}

eg_angle EGMath_atan2( eg_real y , eg_real x )
{
	return eg_angle::FromRad( atan2f( y , x ) );
}

eg_int EGMath_floor( eg_real f )
{
	return static_cast<eg_int>(floorf( f ));
}

eg_int64 EGMath_floor( eg_real64 f )
{
	return static_cast<eg_int64>(floor( f ));
}

eg_int EGMath_round( eg_real f )
{
	return static_cast<eg_int>(roundf( f ));
}

eg_int EGMath_round( eg_real64 f )
{
	return static_cast<eg_int>(round( f ));
}

eg_int EGMath_ceil( eg_real f )
{
	return static_cast<eg_int>(ceilf( f ));
}

eg_real EGMath_pow( eg_real b , eg_real e )
{
	return powf( b , e );
}

eg_real64 EGMath_pow( eg_real64 b , eg_real64 e )
{
	return pow( b , e );
}

static const eg_real EG_AUDIO_VOLUME_START = 1.0f;
static const eg_real EG_AUDIO_VOLUME_STOP = 7.0f;

eg_real EGMath_VolumeToLinear( eg_real f )
{
	eg_real x = EGMath_log( f * (EGMath_exp( EG_AUDIO_VOLUME_STOP ) - EGMath_exp( EG_AUDIO_VOLUME_START )) + EGMath_exp( EG_AUDIO_VOLUME_START ) );
	eg_real Out = (x - EG_AUDIO_VOLUME_START) / (EG_AUDIO_VOLUME_STOP - EG_AUDIO_VOLUME_START);
	return EG_Max( 0.0f , Out );
}

eg_real EGMath_VolumeFromLinear( eg_real f )
{
	eg_real x = EG_AUDIO_VOLUME_START + f * (EG_AUDIO_VOLUME_STOP - EG_AUDIO_VOLUME_START);
	eg_real Out = (EGMath_exp( x ) - EGMath_exp( EG_AUDIO_VOLUME_START )) / (EGMath_exp( EG_AUDIO_VOLUME_STOP ) - EGMath_exp( EG_AUDIO_VOLUME_START ));
	return Out;
}
// (c) 2020 Beem Media. All Rights Reserved.

#include "EGPlane.h"

eg_real eg_plane::Dot( const eg_vec4& v )const
{
	using namespace DirectX;
	XMVECTOR p1 = XMLoadFloat4( *this );
	XMVECTOR v1 = XMLoadFloat4( v );
	return XMVectorGetX( XMPlaneDot( p1 , v1 ) );
}

void eg_plane::Normalize()
{
	using namespace DirectX;
	XMVECTOR p1 = XMLoadFloat4( *this );
	p1 = XMPlaneNormalize( p1 );
	XMStoreFloat4( *this , p1 );
}

eg_plane eg_plane::operator * ( const eg_mat& rhs ) const
{
	using namespace DirectX;
	//assert(rhs.IsORT());
	XMVECTOR p1 = XMLoadFloat4( *this );
	XMMATRIX m1 = XMLoadFloat4x4( rhs );
	p1 = XMPlaneTransform( p1 , m1 );
	eg_plane T;
	XMStoreFloat4( reinterpret_cast<XMFLOAT4*>(&T) , p1 );
	return T;
}

const eg_plane& eg_plane::operator *= ( const eg_mat& rhs )
{
	*this = *this * rhs;
	return *this;
}

eg_vec4 eg_plane::InteresectLine( const eg_vec4& A , const eg_vec4& B ) const
{
	using namespace DirectX;
	XMVECTOR p1 = XMLoadFloat4( *this );
	XMVECTOR v1 = XMLoadFloat4( A );
	XMVECTOR v2 = XMLoadFloat4( B );

	XMVECTOR Res = XMPlaneIntersectLine( p1 , v1 , v2 );

	eg_vec4 T;
	XMStoreFloat4( T , Res );
	return T;
}
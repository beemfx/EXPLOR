// (c) 2020 Beem Media. All Rights Reserved.

#include "EGQuaternion.h"

const eg_quat eg_quat::I( 0.f , 0.f , 0.f , 1.f );

const eg_quat& eg_quat::operator *= ( const eg_quat& rhs )
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR q2 = XMLoadFloat4( rhs );
	XMVECTOR qr = XMQuaternionMultiply( q1 , q2 );
	XMStoreFloat4( *this , qr );
	return *this;
}

eg_bool eg_quat::operator == ( const eg_quat& rhs ) const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR q2 = XMLoadFloat4( rhs );
	return XMQuaternionEqual( q1 , q2 );
}

eg_bool eg_quat::IsNearlyEqual( const eg_vec4& rhs , eg_real Eps /*= EG_SMALL_EPS */ ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR e1 = XMLoadFloat( &Eps );
	return XMVector4NearEqual( v1 , v2 , e1 );
}

eg_real eg_quat::Dot( const eg_quat& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMQuaternionDot( v1 , v2 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_quat eg_quat::Slerp( const eg_quat& rhs , eg_real t ) const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR q2 = XMLoadFloat4( rhs );
	XMVECTOR qr = XMQuaternionSlerp( q1 , q2 , t );
	eg_quat qT;
	XMStoreFloat4( qT , qr );
	return qT;
}

eg_quat eg_quat::GetConjugate() const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR qr = XMQuaternionConjugate( q1 );
	eg_quat qT;
	XMStoreFloat4( qT , qr );
	return qT;
}

eg_real eg_quat::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMQuaternionLength( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_real eg_quat::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMQuaternionLengthSq( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_bool eg_quat::IsNormalized() const
{
	return EG_IsEqualEps( LenSq() , 1.f , EG_SMALL_EPS );
}

eg_quat eg_quat::GetNormalized() const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR qr = XMQuaternionNormalize( q1 );
	eg_quat qT;
	XMStoreFloat4( qT , qr );
	return qT;
}

const eg_quat& eg_quat::NormalizeThis()
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR qr = XMQuaternionNormalize( q1 );
	XMStoreFloat4( *this , qr );
	return *this;
}

eg_angle eg_quat::GetRotationAngle() const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR Axis;
	eg_real Angle;
	XMQuaternionToAxisAngle( &Axis , &Angle , q1 );
	return eg_angle::FromRad( Angle );
}
eg_vec3 eg_quat::GetRotationAxis() const
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( *this );
	XMVECTOR Axis;
	eg_real Angle;
	XMQuaternionToAxisAngle( &Axis , &Angle , q1 );
	eg_vec3 Out;
	XMStoreFloat3( Out , Axis );
	return Out;
}

eg_quat eg_quat::BuildRotationAxis( const eg_vec3& V , eg_angle Angle )
{
	using namespace DirectX;
	assert( V.IsNormalized() );
	XMVECTOR v1 = XMLoadFloat3( V );
	XMVECTOR qr = XMQuaternionRotationNormal( v1 , Angle.ToRad() );
	eg_quat Out;
	XMStoreFloat4( reinterpret_cast<XMFLOAT4*>(&Out) , qr );
	return Out;
}

eg_quat eg_quat::BuildRotationX( eg_angle Angle )
{
	return BuildRotationAxis( eg_vec3(1.f,0.f,0.f) , Angle );
}

eg_quat eg_quat::BuildRotationY( eg_angle Angle )
{
	return BuildRotationAxis( eg_vec3(0.f,1.f,0.f) , Angle );
}

eg_quat eg_quat::BuildRotationZ( eg_angle Angle )
{
	return BuildRotationAxis( eg_vec3(0.f,0.f,1.f) , Angle );
}

eg_quat eg_quat::BuildRotation( const eg_mat& m )
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( m );
	XMVECTOR qr = XMQuaternionRotationMatrix( m1 );
	eg_quat Out;
	XMStoreFloat4( Out , qr );
	return Out;
}

// (c) 2020 Beem Media. All Rights Reserved.

#include "EGVector.h"

//
// eg_vec2
//

eg_vec2 eg_vec2::operator + ( const eg_vec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_vec2 T;
	XMStoreFloat2( T , vr );
	return T;
}

const eg_vec2& eg_vec2::operator += ( const eg_vec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreFloat2( *this , vr );
	return *this;
}

eg_vec2 eg_vec2::operator - ( const eg_vec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_vec2 T;
	XMStoreFloat2( T , vr );
	return T;
}

const eg_vec2& eg_vec2::operator -= ( const eg_vec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreFloat2( *this , vr );
	return *this;
}

eg_vec2 eg_vec2::operator * ( const eg_vec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_vec2 T;
	XMStoreFloat2( T , vr );
	return T;
}

const eg_vec2& eg_vec2::operator *= ( const eg_vec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreFloat2( *this , vr );
	return *this;
}

eg_vec2 eg_vec2::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_vec2 T;
	XMStoreFloat2( T , vr );
	return T;
}

eg_vec2 eg_vec2::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_vec2 T;
	XMStoreFloat2( T , vr );
	return T;
}

const eg_vec2& eg_vec2::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreFloat2( *this , vr );
	return *this;
}

eg_bool eg_vec2::operator == ( const eg_vec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	return XMVector2Equal( v1 , v2 );
}

eg_bool eg_vec2::IsNearlyEqual( const eg_vec2& rhs , eg_real Eps /* = EG_SMALL_EPS */ ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR e1 = XMLoadFloat( &Eps );
	return XMVector2NearEqual( v1 , v2 , e1 );
}

eg_real eg_vec2::Dot( const eg_vec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR v2 = XMLoadFloat2( rhs );
	XMVECTOR vr = XMVector2Dot( v1 , v2 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_vec2 eg_vec2::GetNormalized() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVector2Normalize( v1 );
	eg_vec2 Out;
	XMStoreFloat2( Out , vr );
	return Out;
}

const eg_vec2& eg_vec2::NormalizeThis()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVector2Normalize( v1 );
	XMStoreFloat2( *this , vr );
	return *this;
}

eg_bool eg_vec2::IsNormalized() const
{
	return EG_IsEqualEps( LenSq() , 1.f , EG_SMALL_EPS );
}

eg_real eg_vec2::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVector2Length( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_real eg_vec2::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat2( *this );
	XMVECTOR vr = XMVector2LengthSq( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

//
// eg_vec3
//

eg_vec3 eg_vec3::operator + ( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

const eg_vec3& eg_vec3::operator += ( const eg_vec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreFloat3( *this , vr );
	return *this;
}

eg_vec3 eg_vec3::operator - ( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

const eg_vec3& eg_vec3::operator -= ( const eg_vec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreFloat3( *this , vr );
	return *this;
}

eg_vec3 eg_vec3::operator * ( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

const eg_vec3& eg_vec3::operator *= ( const eg_vec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreFloat3( *this , vr );
	return *this;
}

eg_vec3 eg_vec3::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

eg_vec3 eg_vec3::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

const eg_vec3& eg_vec3::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreFloat3( *this , vr );
	return *this;
}

eg_bool eg_vec3::operator == ( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	return XMVector3Equal( v1 , v2 );
}

eg_bool eg_vec3::IsNearlyEqual( const eg_vec3& rhs , eg_real Eps /* = EG_SMALL_EPS */ ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR e1 = XMLoadFloat( &Eps );
	return XMVector3NearEqual( v1 , v2 , e1 );
}

eg_real eg_vec3::Dot( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVector3Dot( v1 , v2 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_vec3 eg_vec3::Cross( const eg_vec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVector3Cross( v1 , v2 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

eg_vec3 eg_vec3::GetNormalized() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVector3Normalize( v1 );
	eg_vec3 Out;
	XMStoreFloat3( Out , vr );
	return Out;
}

const eg_vec3& eg_vec3::NormalizeThis()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVector3Normalize( v1 );
	XMStoreFloat3( *this , vr );
	return *this;
}

eg_bool eg_vec3::IsNormalized() const
{
	return EG_IsEqualEps( LenSq() , 1.f , EG_SMALL_EPS );
}

eg_real eg_vec3::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVector3Length( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_real eg_vec3::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( *this );
	XMVECTOR vr = XMVector3LengthSq( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

//
// eg_vec4
//

eg_vec4 eg_vec4::operator + ( const eg_vec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

const eg_vec4& eg_vec4::operator += ( const eg_vec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreFloat4( *this , vr );
	return *this;
}

eg_vec4 eg_vec4::operator - ( const eg_vec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

const eg_vec4& eg_vec4::operator -= ( const eg_vec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreFloat4( *this , vr );
	return *this;
}

eg_vec4 eg_vec4::operator * ( const eg_vec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

const eg_vec4& eg_vec4::operator *= ( const eg_vec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreFloat4( *this , vr );
	return *this;
}

eg_vec4 eg_vec4::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

eg_vec4 eg_vec4::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

const eg_vec4& eg_vec4::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreFloat4( *this , vr );
	return *this;
}

const eg_vec4& eg_vec4::operator *= ( const eg_mat& M )
{
	*this = *this * M;
	return *this;
}

const eg_vec4& eg_vec4::operator *= ( const eg_transform& T )
{
	*this = *this * T;
	return *this;
}

eg_bool eg_vec4::operator == ( const eg_vec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	return XMVector4Equal( v1 , v2 );
}


eg_bool eg_vec4::IsNearlyEqual( const eg_vec4& rhs , eg_real Eps /* = EG_SMALL_EPS */ ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR e1 = XMLoadFloat( &Eps );
	return XMVector4NearEqual( v1 , v2 , e1 );
}

eg_real eg_vec4::Dot( const eg_vec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( rhs );
	XMVECTOR vr = XMVector4Dot( v1 , v2 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_vec4 eg_vec4::Cross( const eg_vec4& InV1 , const eg_vec4& InV2 ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR v2 = XMLoadFloat4( InV1 );
	XMVECTOR v3 = XMLoadFloat4( InV2 );
	XMVECTOR vr = XMVector4Cross( v1 , v2 , v3 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

eg_vec4 eg_vec4::GetNormalized() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVector4Normalize( v1 );
	eg_vec4 Out;
	XMStoreFloat4( Out , vr );
	return Out;
}

const eg_vec4& eg_vec4::NormalizeThis()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVector4Normalize( v1 );
	XMStoreFloat4( *this , vr );
	return *this;
}

eg_bool eg_vec4::IsNormalized() const
{
	return EG_IsEqualEps( LenSq() , 1.f , EG_SMALL_EPS );
}

eg_real eg_vec4::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVector4Length( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_real eg_vec4::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat4( *this );
	XMVECTOR vr = XMVector4LengthSq( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_vec3 eg_vec4::CrossAsVec3( const eg_vec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( ToVec3() );
	XMVECTOR v2 = XMLoadFloat3( rhs );
	XMVECTOR vr = XMVector3Cross( v1 , v2 );
	eg_vec3 T;
	XMStoreFloat3( T , vr );
	return T;
}

eg_vec3 eg_vec4::GetNormalizeAsVec3() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( ToVec3() );
	XMVECTOR vr = XMVector3Normalize( v1 );
	eg_vec3 Out;
	XMStoreFloat3( Out , vr );
	return Out;
}

const eg_vec4& eg_vec4::NormalizeThisAsVec3()
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( ToVec3() );
	XMVECTOR vr = XMVector3Normalize( v1 );
	eg_vec3 TempOut;
	XMStoreFloat3( TempOut, vr );
	*this = eg_vec4( TempOut , 0.f );
	return *this;
}

eg_bool eg_vec4::IsNormalizedAsVec3() const
{
	return EG_IsEqualEps( LenSq() , 1.f , EG_SMALL_EPS ) && EG_IsEqualEps( w , 0.f , EG_SMALL_EPS );
}

eg_real eg_vec4::LenAsVec3() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( ToVec3() );
	XMVECTOR vr = XMVector3Length( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

eg_real eg_vec4::LenSqAsVec3() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( ToVec3() );
	XMVECTOR vr = XMVector3LengthSq( v1 );
	eg_real Out;
	XMStoreFloat( &Out , vr );
	return Out;
}

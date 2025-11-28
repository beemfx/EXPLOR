// (c) 2020 Beem Media. All Rights Reserved.

#include "EGIntVector.h"

//
// eg_ivec2
//

eg_ivec2 eg_ivec2::operator + ( const eg_ivec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_ivec2 T;
	XMStoreSInt2( T , vr );
	return T;
}

const eg_ivec2& eg_ivec2::operator += ( const eg_ivec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreSInt2( *this , vr );
	return *this;
}

eg_ivec2 eg_ivec2::operator - ( const eg_ivec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_ivec2 T;
	XMStoreSInt2( T , vr );
	return T;
}

const eg_ivec2& eg_ivec2::operator -= ( const eg_ivec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreSInt2( *this , vr );
	return *this;
}

eg_ivec2 eg_ivec2::operator * ( const eg_ivec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_ivec2 T;
	XMStoreSInt2( T , vr );
	return T;
}

const eg_ivec2& eg_ivec2::operator *= ( const eg_ivec2& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreSInt2( *this , vr );
	return *this;
}

eg_ivec2 eg_ivec2::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_ivec2 T;
	XMStoreSInt2( T , vr );
	return T;
}

eg_ivec2 eg_ivec2::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_ivec2 T;
	XMStoreSInt2( T , vr );
	return T;
}

const eg_ivec2& eg_ivec2::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreSInt2( *this , vr );
	return *this;
}

eg_int eg_ivec2::Dot( const eg_ivec2& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR v2 = XMLoadSInt2( rhs );
	XMVECTOR vr = XMVector2Dot( v1 , v2 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_int eg_ivec2::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR vr = XMVector2Length( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_int eg_ivec2::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt2( *this );
	XMVECTOR vr = XMVector2LengthSq( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

//
// eg_ivec3
//

eg_ivec3 eg_ivec3::operator + ( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

const eg_ivec3& eg_ivec3::operator += ( const eg_ivec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreSInt3( *this , vr );
	return *this;
}

eg_ivec3 eg_ivec3::operator - ( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

const eg_ivec3& eg_ivec3::operator -= ( const eg_ivec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreSInt3( *this , vr );
	return *this;
}

eg_ivec3 eg_ivec3::operator * ( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

const eg_ivec3& eg_ivec3::operator *= ( const eg_ivec3& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreSInt3( *this , vr );
	return *this;
}

eg_ivec3 eg_ivec3::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

eg_ivec3 eg_ivec3::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

const eg_ivec3& eg_ivec3::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreSInt3( *this , vr );
	return *this;
}

eg_int eg_ivec3::Dot( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVector3Dot( v1 , v2 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_ivec3 eg_ivec3::Cross( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVector3Cross( v1 , v2 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

eg_int eg_ivec3::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR vr = XMVector3Length( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_int eg_ivec3::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( *this );
	XMVECTOR vr = XMVector3LengthSq( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

//
// eg_ivec4
//

eg_ivec4 eg_ivec4::operator + ( const eg_ivec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

const eg_ivec4& eg_ivec4::operator += ( const eg_ivec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorAdd( v1 , v2 );
	XMStoreSInt4( *this , vr );
	return *this;
}

eg_ivec4 eg_ivec4::operator - ( const eg_ivec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

const eg_ivec4& eg_ivec4::operator -= ( const eg_ivec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorSubtract( v1 , v2 );
	XMStoreSInt4( *this , vr );
	return *this;
}

eg_ivec4 eg_ivec4::operator * ( const eg_ivec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

const eg_ivec4& eg_ivec4::operator *= ( const eg_ivec4& rhs )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVectorMultiply( v1 , v2 );
	XMStoreSInt4( *this , vr );
	return *this;
}

eg_ivec4 eg_ivec4::operator - () const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR vr = XMVectorNegate( v1 );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

eg_ivec4 eg_ivec4::operator * ( eg_real s ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

const eg_ivec4& eg_ivec4::operator *= ( eg_real s )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR vr = XMVectorScale( v1 , s );
	XMStoreSInt4( *this , vr );
	return *this;
}

eg_int eg_ivec4::Dot( const eg_ivec4& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( rhs );
	XMVECTOR vr = XMVector4Dot( v1 , v2 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_ivec4 eg_ivec4::Cross( const eg_ivec4& InV1 , const eg_ivec4& InV2 ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR v2 = XMLoadSInt4( InV1 );
	XMVECTOR v3 = XMLoadSInt4( InV2 );
	XMVECTOR vr = XMVector4Cross( v1 , v2 , v3 );
	eg_ivec4 T;
	XMStoreSInt4( T , vr );
	return T;
}

eg_int eg_ivec4::Len() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR vr = XMVector4Length( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_int eg_ivec4::LenSq() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt4( *this );
	XMVECTOR vr = XMVector4LengthSq( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_ivec3 eg_ivec4::CrossAsVec3( const eg_ivec3& rhs ) const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( ToVec3() );
	XMVECTOR v2 = XMLoadSInt3( rhs );
	XMVECTOR vr = XMVector3Cross( v1 , v2 );
	eg_ivec3 T;
	XMStoreSInt3( T , vr );
	return T;
}

eg_int eg_ivec4::LenAsVec3() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( ToVec3() );
	XMVECTOR vr = XMVector3Length( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

eg_int eg_ivec4::LenSqAsVec3() const
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadSInt3( ToVec3() );
	XMVECTOR vr = XMVector3LengthSq( v1 );
	eg_ivec2 Out;
	XMStoreSInt2( Out , vr );
	return Out.x;
}

// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

struct eg_vec2;
struct eg_vec3;
struct eg_vec4;
struct eg_mat;
struct eg_quat;
struct eg_transform;

struct eg_vec2
{	
	eg_real x , y;

	eg_vec2() = default;
	eg_vec2( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_vec2( 0.f , 0.f ); } else if( Ct == CT_Default ) { *this = eg_vec2( 0.f , 0.f ); } }
	eg_vec2( eg_real InX , eg_real InY ) : x(InX) , y(InY) { }

	eg_vec2 operator + ( const eg_vec2& rhs ) const;
	const eg_vec2&  operator += ( const eg_vec2& rhs );
	eg_vec2 operator - ( const eg_vec2& rhs ) const;
	const eg_vec2& operator -= ( const eg_vec2& rhs );
	eg_vec2 operator * ( const eg_vec2& rhs ) const;
	const eg_vec2& operator *= ( const eg_vec2& rhs );
	eg_vec2 operator - () const;
	eg_vec2 operator * ( eg_real s ) const;
	const eg_vec2& operator *= ( eg_real s );
	eg_vec2 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_vec2& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	eg_bool operator == ( const eg_vec2& rhs ) const;
	eg_bool operator != ( const eg_vec2& rhs ) const { return !(*this == rhs); }
	eg_bool IsNearlyEqual( const eg_vec2& rhs , eg_real Eps = EG_SMALL_EPS ) const;

	eg_real Dot( const eg_vec2& rhs ) const;
	eg_vec2 GetNormalized() const;
	const eg_vec2& NormalizeThis();
	eg_bool IsNormalized() const;
	eg_real Len() const;
	eg_real LenSq() const;

	inline operator DirectX::XMFLOAT2* () { return reinterpret_cast<DirectX::XMFLOAT2*>(this); }
	inline operator const DirectX::XMFLOAT2* () const { return reinterpret_cast<const DirectX::XMFLOAT2*>(this); }
};

struct eg_vec3
{
	eg_real x , y , z;

	eg_vec3() = default;
	eg_vec3( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_vec3( 0.f , 0.f , 0.f ); } else if( Ct == CT_Default ) { *this = eg_vec3( 0.f , 0.f , 0.f ); } }
	eg_vec3( eg_real InX , eg_real InY , eg_real InZ ) : x(InX) , y(InY) , z(InZ) { }
	eg_vec3( const eg_vec2& InXY , eg_real InZ ) : x(InXY.x) , y(InXY.y) , z(InZ) { }

	eg_vec3 operator + ( const eg_vec3& rhs ) const;
	const eg_vec3&  operator += ( const eg_vec3& rhs );
	eg_vec3 operator - ( const eg_vec3& rhs ) const;
	const eg_vec3& operator -= ( const eg_vec3& rhs );
	eg_vec3 operator * ( const eg_vec3& rhs ) const;
	const eg_vec3& operator *= ( const eg_vec3& rhs );
	eg_vec3 operator - () const;
	eg_vec3 operator * ( eg_real s ) const;
	const eg_vec3& operator *= ( eg_real s );
	eg_vec3 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_vec3& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	eg_bool operator == ( const eg_vec3& rhs ) const;
	eg_bool operator != ( const eg_vec3& rhs ) const { return !(*this == rhs); }
	eg_bool IsNearlyEqual( const eg_vec3& rhs , eg_real Eps = EG_SMALL_EPS ) const;

	eg_real Dot( const eg_vec3& rhs ) const;
	eg_vec3 Cross( const eg_vec3& rhs ) const;
	eg_vec3 GetNormalized() const;
	const eg_vec3& NormalizeThis();
	eg_bool IsNormalized() const;
	eg_real Len() const;
	eg_real LenSq() const;

	eg_vec2 XY_ToVec2() const { return eg_vec2( x , y ); }
	eg_vec2 YZ_ToVec2() const { return eg_vec2( y , z ); }
	eg_vec2 XZ_ToVec2() const { return eg_vec2( x , z ); }

	inline operator DirectX::XMFLOAT3* () { return reinterpret_cast<DirectX::XMFLOAT3*>(this); }
	inline operator const DirectX::XMFLOAT3* () const { return reinterpret_cast<const DirectX::XMFLOAT3*>(this); }
};

struct eg_vec4
{
	eg_real x , y , z , w;

	eg_vec4() = default;
	eg_vec4( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_vec4( 0.f , 0.f , 0.f , 0.f ); } else if( Ct == CT_Default ) { *this = eg_vec4( 0.f , 0.f , 0.f , 1.f ); } }
	eg_vec4( eg_real InX , eg_real InY , eg_real InZ , eg_real InW ) : x(InX) , y(InY) , z(InZ) , w(InW) { }
	eg_vec4( const eg_vec2& InXY , eg_real InZ , eg_real InW ) : x(InXY.x) , y(InXY.y) , z(InZ) , w(InW) { }
	eg_vec4( const eg_vec3& InXYZ , eg_real InW ) : x(InXYZ.x) , y(InXYZ.y) , z(InXYZ.z) , w(InW) { }

	eg_vec4 operator + ( const eg_vec4& rhs ) const;
	const eg_vec4&  operator += ( const eg_vec4& rhs );
	eg_vec4 operator - ( const eg_vec4& rhs ) const;
	const eg_vec4& operator -= ( const eg_vec4& rhs );
	eg_vec4 operator * ( const eg_vec4& rhs ) const;
	const eg_vec4& operator *= ( const eg_vec4& rhs );
	eg_vec4 operator - () const;
	eg_vec4 operator * ( eg_real s ) const;
	const eg_vec4& operator *= ( eg_real s );
	eg_vec4 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_vec4& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	const eg_vec4& operator *= ( const eg_mat& M );
	const eg_vec4& operator *= ( const eg_transform& T );

	eg_bool operator == ( const eg_vec4& rhs ) const;
	eg_bool operator != ( const eg_vec4& rhs ) const { return !(*this == rhs); }
	eg_bool IsNearlyEqual( const eg_vec4& rhs , eg_real Eps = EG_SMALL_EPS ) const;

	eg_real Dot( const eg_vec4& rhs ) const;
	eg_vec4 Cross( const eg_vec4& InV1 , const eg_vec4& InV2 ) const;
	eg_vec4 GetNormalized() const;
	const eg_vec4& NormalizeThis();
	eg_bool IsNormalized() const;
	eg_real Len() const;
	eg_real LenSq() const;

	eg_vec3 CrossAsVec3( const eg_vec3& rhs );
	eg_vec3 GetNormalizeAsVec3() const;
	const eg_vec4& NormalizeThisAsVec3();
	eg_bool IsNormalizedAsVec3() const;
	eg_real LenAsVec3() const;
	eg_real LenSqAsVec3() const;

	eg_vec3 ToVec3() const { return eg_vec3( x , y , z ); }
	eg_vec3 XYZ_ToVec3() const { return eg_vec3( x , y , z ); }
	eg_vec2 XY_ToVec2() const { return eg_vec2( x , y ); }
	eg_vec2 YZ_ToVec2() const { return eg_vec2( y , z ); }
	eg_vec2 XZ_ToVec2() const { return eg_vec2( x , z ); }

	inline operator DirectX::XMFLOAT4* () { return reinterpret_cast<DirectX::XMFLOAT4*>(this); }
	inline operator const DirectX::XMFLOAT4* () const { return reinterpret_cast<const DirectX::XMFLOAT4*>(this); }
};

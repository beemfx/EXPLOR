// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

struct eg_ivec2;
struct eg_ivec3;
struct eg_ivec4;

struct eg_ivec2
{
	eg_int x , y;

	eg_ivec2() = default;
	eg_ivec2( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_ivec2( 0 , 0 ); } else if( Ct == CT_Default ) { *this = eg_ivec2( 0 , 0 ); } }
	eg_ivec2( eg_int InX , eg_int InY ) : x(InX) , y(InY) { }

	eg_ivec2 operator + ( const eg_ivec2& rhs ) const;
	const eg_ivec2&  operator += ( const eg_ivec2& rhs );
	eg_ivec2 operator - ( const eg_ivec2& rhs ) const;
	const eg_ivec2& operator -= ( const eg_ivec2& rhs );
	eg_ivec2 operator * ( const eg_ivec2& rhs ) const;
	const eg_ivec2& operator *= ( const eg_ivec2& rhs );
	eg_ivec2 operator - () const;
	eg_ivec2 operator * ( eg_real s ) const;
	const eg_ivec2& operator *= ( eg_real s );
	eg_ivec2 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_ivec2& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	eg_bool operator == ( const eg_ivec2& rhs ) const { return x == rhs.x && y == rhs.y; }
	eg_bool operator != ( const eg_ivec2& rhs ) const { return !(*this == rhs); }

	eg_int Dot( const eg_ivec2& rhs ) const;
	eg_int Len() const;
	eg_int LenSq() const;

	inline operator DirectX::XMINT2* () { return reinterpret_cast<DirectX::XMINT2*>(this); }
	inline operator const DirectX::XMINT2* () const { return reinterpret_cast<const DirectX::XMINT2*>(this); }
};

struct eg_ivec3
{
	eg_int x , y , z;

	eg_ivec3() = default;
	eg_ivec3( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_ivec3( 0 , 0 , 0 ); } else if( Ct == CT_Default ) { *this = eg_ivec3( 0 , 0 , 0 ); } }
	eg_ivec3( eg_int InX , eg_int InY , eg_int InZ ) : x(InX) , y(InY) , z(InZ) { }
	eg_ivec3( const eg_ivec2& InXY , eg_int InZ ) : x(InXY.x) , y(InXY.y) , z(InZ) { }

	eg_ivec3 operator + ( const eg_ivec3& rhs ) const;
	const eg_ivec3&  operator += ( const eg_ivec3& rhs );
	eg_ivec3 operator - ( const eg_ivec3& rhs ) const;
	const eg_ivec3& operator -= ( const eg_ivec3& rhs );
	eg_ivec3 operator * ( const eg_ivec3& rhs ) const;
	const eg_ivec3& operator *= ( const eg_ivec3& rhs );
	eg_ivec3 operator - () const;
	eg_ivec3 operator * ( eg_real s ) const;
	const eg_ivec3& operator *= ( eg_real s );
	eg_ivec3 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_ivec3& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	eg_bool operator == ( const eg_ivec3& rhs ) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	eg_bool operator != ( const eg_ivec3& rhs ) const { return !(*this == rhs); }

	eg_int Dot( const eg_ivec3& rhs ) const;
	eg_ivec3 Cross( const eg_ivec3& rhs ) const;
	eg_int Len() const;
	eg_int LenSq() const;

	eg_ivec2 XY_ToVec2() const { return eg_ivec2( x , y ); }
	eg_ivec2 YZ_ToVec2() const { return eg_ivec2( y , z ); }
	eg_ivec2 XZ_ToVec2() const { return eg_ivec2( x , z ); }

	inline operator DirectX::XMINT3* () { return reinterpret_cast<DirectX::XMINT3*>(this); }
	inline operator const DirectX::XMINT3* () const { return reinterpret_cast<const DirectX::XMINT3*>(this); }
};

struct eg_ivec4
{
	eg_int x , y , z , w;

	eg_ivec4() = default;
	eg_ivec4( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { *this = eg_ivec4( 0 , 0 , 0 , 0 ); } else if( Ct == CT_Default ) { *this = eg_ivec4( 0 , 0 , 0 , 1 ); } }
	eg_ivec4( eg_int InX , eg_int InY , eg_int InZ , eg_int InW ) : x(InX) , y(InY) , z(InZ) , w(InW) { }
	eg_ivec4( const eg_ivec2& InXY , eg_int InZ , eg_int InW ) : x(InXY.x) , y(InXY.y) , z(InZ) , w(InW) { }
	eg_ivec4( const eg_ivec3& InXYZ , eg_int InW ) : x(InXYZ.x) , y(InXYZ.y) , z(InXYZ.z) , w(InW) { }

	eg_ivec4 operator + ( const eg_ivec4& rhs ) const;
	const eg_ivec4&  operator += ( const eg_ivec4& rhs );
	eg_ivec4 operator - ( const eg_ivec4& rhs ) const;
	const eg_ivec4& operator -= ( const eg_ivec4& rhs );
	eg_ivec4 operator * ( const eg_ivec4& rhs ) const;
	const eg_ivec4& operator *= ( const eg_ivec4& rhs );
	eg_ivec4 operator - () const;
	eg_ivec4 operator * ( eg_real s ) const;
	const eg_ivec4& operator *= ( eg_real s );
	eg_ivec4 operator / ( eg_real s ) const { return (*this) * (1.f/s); }
	const eg_ivec4& operator /= ( eg_real s ) { return *this *= (1.f/s); }

	eg_bool operator == ( const eg_ivec4& rhs ) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	eg_bool operator != ( const eg_ivec4& rhs ) const { return !(*this == rhs); }

	eg_int Dot( const eg_ivec4& rhs ) const;
	eg_ivec4 Cross( const eg_ivec4& InV1 , const eg_ivec4& InV2 ) const;
	eg_int Len() const;
	eg_int LenSq() const;

	eg_ivec3 CrossAsVec3( const eg_ivec3& rhs ) const;
	eg_int LenAsVec3() const;
	eg_int LenSqAsVec3() const;

	eg_ivec3 ToVec3() const { return eg_ivec3( x , y , z ); }
	eg_ivec3 XYZ_ToVec3() const { return eg_ivec3( x , y , z ); }
	eg_ivec2 XY_ToVec2() const { return eg_ivec2( x , y ); }
	eg_ivec2 YZ_ToVec2() const { return eg_ivec2( y , z ); }
	eg_ivec2 XZ_ToVec2() const { return eg_ivec2( x , z ); }

	inline operator DirectX::XMINT4* () { return reinterpret_cast<DirectX::XMINT4*>(this); }
	inline operator const DirectX::XMINT4* () const { return reinterpret_cast<const DirectX::XMINT4*>(this); }
};

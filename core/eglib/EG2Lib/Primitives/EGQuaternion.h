// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

struct eg_mat;

struct eg_quat
{
	eg_real x , y , z , w;

	static const eg_quat I;

	eg_quat() = default;
	eg_quat( eg_real InX , eg_real InY , eg_real InZ , eg_real InW ) : x(InX) , y(InY) , z(InZ) , w(InW) { }
	eg_quat( eg_ctor_t Ct )	{ if( Ct == CT_Default ) { *this = eg_quat::BuildIdentity(); } else if( Ct == CT_Clear ) { *this = eg_quat( 0.f , 0.f , 0.f , 0.f ); } }
	eg_quat( const eg_vec3& V , eg_angle Angle ) { *this = eg_quat::BuildRotationAxis( V , Angle ); }

	friend eg_quat operator * ( const eg_quat& L , const eg_quat& R );
	friend eg_vec3 operator * ( const eg_vec3& L , const eg_quat& R );
	friend eg_vec3 operator * ( const eg_quat& L , const eg_vec3& R );
	const eg_quat& operator *= ( const eg_quat& rhs );
	eg_bool operator == ( const eg_quat& rhs ) const;
	eg_bool operator != ( const eg_quat& rhs ) const { return !(*this == rhs); }
	eg_bool IsNearlyEqual( const eg_vec4& rhs , eg_real Eps = EG_SMALL_EPS ) const;

	eg_real Dot( const eg_quat& rhs ) const;
	eg_quat Slerp( const eg_quat& rhs , eg_real t ) const;
	eg_quat GetConjugate() const;

	eg_real Len() const;
	eg_real LenSq() const;
	eg_bool IsNormalized() const;
	eg_quat GetNormalized() const;
	const eg_quat& NormalizeThis();

	eg_angle GetRotationAngle() const;
	eg_vec3 GetRotationAxis() const;

	static eg_quat BuildIdentity() { return I; }
	static eg_quat BuildRotationAxis( const eg_vec3& V , eg_angle Angle );
	static eg_quat BuildRotationX( eg_angle Angle );
	static eg_quat BuildRotationY( eg_angle Angle );
	static eg_quat BuildRotationZ( eg_angle Angle );
	static eg_quat BuildRotation( const eg_mat& m );

	inline operator DirectX::XMFLOAT4* () { return reinterpret_cast<DirectX::XMFLOAT4*>(this); }
	inline operator const DirectX::XMFLOAT4* () const { return reinterpret_cast<const DirectX::XMFLOAT4*>(this); }
};

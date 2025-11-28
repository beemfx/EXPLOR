// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

// 4x4 Transform Matrix in the form M(s)*M(R)*M(T)
// ( sR | 0 )
// ( ---|---)
// (  T | 1 )
// Vectors are post multiplied by the matrix for a meaningful transform. (i.e vp = v*M)
// Post multiplying a vector generally doesn't mean anything. And the order of
// transforms is from left to right.

struct eg_transform;
struct eg_quat;
struct eg_plane;

struct eg_mat
{
	union
	{
		struct
		{
			eg_real _11, _12, _13, _14;
			eg_real _21, _22, _23, _24;
			eg_real _31, _32, _33, _34;
			eg_real _41, _42, _43, _44;
		};

		struct
		{
			eg_real r11, r12, r13, x14;
			eg_real r21, r22, r23, x24;
			eg_real r31, r32, r33, x34;
			eg_real tx , ty , tz , x44;
		};
	};

	static const eg_mat I;
	static const eg_mat Zero;

	eg_mat() = default;

	eg_mat
	(
		eg_real f11 , eg_real f12 , eg_real f13 , eg_real f14 ,
		eg_real f21 , eg_real f22 , eg_real f23 , eg_real f24 ,
		eg_real f31 , eg_real f32 , eg_real f33 , eg_real f34 ,
		eg_real f41 , eg_real f42 , eg_real f43 , eg_real f44
	)
	{
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}

	eg_mat( const eg_vec4& e1 , const eg_vec4& e2 , const eg_vec4& e3 , const eg_vec4& e4 )
	{
		_11 = e1.x; _12 = e1.y; _13 = e1.z; _14 = e1.w;
		_21 = e2.x; _22 = e2.y; _23 = e2.z; _24 = e2.w;
		_31 = e3.x; _32 = e3.y; _33 = e3.z; _34 = e3.w;
		_41 = e4.x; _42 = e4.y; _43 = e4.z; _44 = e4.w;
	}

	explicit eg_mat( const eg_transform& T )
	{
		*this = BuildTransform( T );
	}

	explicit eg_mat( const eg_quat& InRot )
	{
		*this = BuildRotation( InRot );
	}

	eg_mat( eg_ctor_t Ct )
	{
		if( Ct == CT_Default )
		{
			*this = I;
		}
		else if( Ct == CT_Clear )
		{
			*this = Zero;
		}
	}

	friend eg_mat operator * ( const eg_mat& L , const eg_mat& R );
	const eg_mat& operator *= ( const eg_mat& rhs );
	friend eg_vec4 operator * ( const eg_vec4& L , const eg_mat& R );
	friend eg_vec4 operator * ( const eg_mat& L , const eg_vec4& R );

	eg_mat Lerp( const eg_mat& rhs , eg_real t ) const;

	eg_real GetDeterminant() const;
	eg_bool IsIdentity() const;
	eg_bool IsORT() const;

	const eg_mat& TranslateThis( const eg_vec3& t ) { tx += t.x; ty += t.y; tz += t.z; return *this; }
	const eg_mat& TranslateThis( eg_real x , eg_real y , eg_real z ) { TranslateThis( eg_vec3( x , y , z ) ); return *this; }

	const eg_mat& RotateAxisThis( const eg_vec3& Axis , eg_angle Angle ) { *this *= BuildRotationAxis( Axis , Angle ); return *this; }
	const eg_mat& RotateXThis( eg_angle Angle ) { *this *= BuildRotationX( Angle ); return *this; }
	const eg_mat& RotateYThis( eg_angle Angle ) { *this *= BuildRotationY( Angle ); return *this; }
	const eg_mat& RotateZThis( eg_angle Angle ) { *this *= BuildRotationZ( Angle ); return *this; }
	const eg_mat& ScaleThis( eg_real Scale ) { *this *= BuildScaling( Scale ); return *this; }
	const eg_mat& ScaleThis( const eg_vec3& Scale ) { *this *= BuildScaling( Scale ); return *this; }
	
	static eg_mat BuildIdentity() { return I; }
	static eg_mat BuildInverse( const eg_mat& M , eg_real* OutDet );
	static eg_mat BuildTranspose( const eg_mat& M );
	static eg_mat BuildORTInverse( const eg_mat& M );
	static eg_mat BuildTransform( const eg_transform& T );
	static eg_mat BuildTransformNoScale( const eg_transform& T );
	static eg_mat BuildRotation( const eg_quat& Q );
	static eg_mat BuildTranslation( const eg_vec3& V );
	static eg_mat BuildScaling( eg_real s );
	static eg_mat BuildScaling( const eg_vec3& V );
	static eg_mat BuildRotationAxis( const eg_vec3& V , eg_angle Angle );
	static eg_mat BuildRotationX( eg_angle Angle );
	static eg_mat BuildRotationY( eg_angle Angle );
	static eg_mat BuildRotationZ( eg_angle Angle );
	static eg_mat BuildLookAtLH( const eg_vec3& Pos , const eg_vec3& At , const eg_vec3& Up );
	static eg_mat BuildPerspectiveFovLH( eg_angle fovy , eg_real Aspect , eg_real zn , eg_real zf );
	static eg_mat BuildOrthographicLH( eg_real w , eg_real h , eg_real zn , eg_real zf );
	static eg_mat BuildLookAtRH( const eg_vec3& Pos , const eg_vec3& At , const eg_vec3& Up );
	static eg_mat BuildPerspectiveFovRH( eg_angle fovy , eg_real Aspect , eg_real zn , eg_real zf );
	static eg_mat BuildOrthographicRH( eg_real w , eg_real h , eg_real zn , eg_real zf );
	static eg_mat BuildReflection( const eg_plane& Plane );

	inline operator DirectX::XMFLOAT4X4* () { return reinterpret_cast<DirectX::XMFLOAT4X4*>(this); }
	inline operator const DirectX::XMFLOAT4X4* () const { return reinterpret_cast<const DirectX::XMFLOAT4X4*>(this); }
};

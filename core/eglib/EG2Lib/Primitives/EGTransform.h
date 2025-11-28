// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

// SRT-Transform (See Eberly 144-145) represents a transfrom in the form M(s)*M(R)*M(T)
// ( sR | 0 )
// ( ---|---)
// (  T | 1 )
// Vectors are post multiplied by the sRT for a meaningful transform. (i.e vp = v*SRT)
// Post multiplying a vector generally doesn't mean anything. And the order of
// transforms is from left to right.

struct eg_mat;

struct eg_transform
{
private:

	eg_quat R;
	eg_vec3 T;
	eg_real s;

public:

	static const eg_transform I;

	eg_transform() = default;
	eg_transform( eg_ctor_t Ct ) : R( Ct ) , T( Ct ) { if( Ct == CT_Default ) { s = 1.f; } else if( Ct == CT_Clear ) { s = 0.f; } }

	friend eg_transform operator * ( const eg_transform& l , const eg_transform& r );
	friend eg_vec4 operator * ( const eg_vec4& l , const eg_transform& r );
	friend eg_vec4 operator * ( const eg_transform& L , const eg_vec4& R );

	const eg_transform& operator *= ( const eg_transform& l ) { *this = *this * l; return *this; }

	eg_bool operator == ( const eg_transform& rhs ) const { return R == rhs.R && s == rhs.s && T == rhs.T; }
	eg_bool operator != ( const eg_transform& rhs ) const { return !(*this == rhs); }

	eg_bool IsValid() const;
	eg_bool IsIdentity() const;
	eg_bool IsNormalized() const;
	const eg_real& GetScale() const { return s; }
	const eg_quat& GetRotation() const { return R; }
	const eg_vec3& GetTranslation() const { return T; }
	eg_vec4 GetPosition() const { return eg_vec4( T , 1.f ); }
	eg_transform GetInverse() const { return BuildInverse( *this ); }
	void SetRotation( const eg_quat& In ) { R = In; }
	void SetTranslation( const eg_vec3& In ) { T = In; }
	void SetScale( eg_real In ) { s = In; }

	const eg_transform& TranslateThis( const eg_vec3& t ) { T += t; return *this; }
	const eg_transform& TranslateThis( eg_real x , eg_real y , eg_real z ) { TranslateThis( eg_vec3( x , y , z ) ); return *this; }
	const eg_transform& NormalizeThis() { R.NormalizeThis(); T *= s; s = 1.f; return *this; }
	const eg_transform& NormalizeRotationOfThis() { R.NormalizeThis(); return *this; }

	const eg_transform& RotateAxisThis( const eg_vec3& Axis , eg_angle Angle ) { *this *= BuildRotationAxis( Axis , Angle ); return *this; }
	const eg_transform& RotateXThis( eg_angle Angle ) { *this *= BuildRotationX( Angle ); return *this; }
	const eg_transform& RotateYThis( eg_angle Angle ) { *this *= BuildRotationY( Angle ); return *this; }
	const eg_transform& RotateZThis( eg_angle Angle ) { *this *= BuildRotationZ( Angle ); return *this; }
	const eg_transform& ScaleThis( eg_real Scale ) { s *= Scale; T*=Scale; return *this; }
	const eg_transform& ScaleTranslationOfThisNonUniformly( const eg_vec3& ScaleVec ) { T = T * ScaleVec; return *this; }

	static eg_transform BuildIdentity() { return I; }
	static eg_transform BuildInverse( const eg_transform& T );
	static eg_transform BuildFromMatrix( const eg_mat& M );
	static eg_transform BuildRotation( const eg_quat& R ) { eg_transform Out = I; Out.R = R; return Out; }
	static eg_transform BuildRotationAxis( const eg_vec3& Axis , eg_angle Angle ) { eg_transform Out = I; Out.R = eg_quat::BuildRotationAxis( Axis , Angle ); return Out; }
	static eg_transform BuildTranslation( const eg_vec3& t ) { eg_transform Out = I; Out.T = t; return Out; }
	static eg_transform BuildTranslation( eg_real x , eg_real y , eg_real z ) { return BuildTranslation( eg_vec3( x , y , z ) ); }
	static eg_transform BuildScaling( eg_real S ) { eg_transform Out = I; Out.s = S; return Out; }
	static eg_transform BuildRotationScalingTranslation( const eg_quat& Rot , eg_real S , const eg_vec3& Trans ) { eg_transform Out; Out.R = Rot; Out.s = S; Out.T = Trans; return Out; }
	static eg_transform BuildRotationX( eg_angle Angle ) { eg_transform Out = I; Out.R = eg_quat::BuildRotationX( Angle ); return Out; }
	static eg_transform BuildRotationY( eg_angle Angle ) { eg_transform Out = I; Out.R = eg_quat::BuildRotationY( Angle ); return Out; }
	static eg_transform BuildRotationZ( eg_angle Angle ) { eg_transform Out = I; Out.R = eg_quat::BuildRotationZ( Angle ); return Out; }

	static eg_transform Lerp( const eg_transform& Lhs , const eg_transform& Rhs , eg_real t );
	static eg_transform CatmullRomLerp( const eg_transform& LhsC , const eg_transform& Lhs , const eg_transform& Rhs , const eg_transform& RhsC , eg_real t );
};

static_assert(sizeof( eg_transform ) == 32 , "eg_transform was the wrong size!");

// (c) 2020 Beem Media. All Rights Reserved.

#include "EGMatrix.h"

const eg_mat eg_mat::I
(
	1.f,0.f,0.f,0.f,
	0.f,1.f,0.f,0.f,
	0.f,0.f,1.f,0.f,
	0.f,0.f,0.f,1.f
);

const eg_mat eg_mat::Zero
(
	0.f,0.f,0.f,0.f,
	0.f,0.f,0.f,0.f,
	0.f,0.f,0.f,0.f,
	0.f,0.f,0.f,0.f
);

const eg_mat& eg_mat::operator *= ( const eg_mat& rhs )
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( *this );
	XMMATRIX m2 = XMLoadFloat4x4( rhs );
	XMMATRIX mr = XMMatrixMultiply( m1 , m2 );
	XMStoreFloat4x4( *this , mr );
	return *this;
}

eg_mat eg_mat::Lerp( const eg_mat& rhs , eg_real t ) const
{
	eg_mat Out;
	const eg_mat* pM1 = this;
	// Just lerp each component, not as accurate as
	// using quaternions, but faster.
	Out._11 = pM1->_11 + t * (rhs._11 - pM1->_11);
	Out._12 = pM1->_12 + t * (rhs._12 - pM1->_12);
	Out._13 = pM1->_13 + t * (rhs._13 - pM1->_13);
	Out._14 = pM1->_14 + t * (rhs._14 - pM1->_14);

	Out._21 = pM1->_21 + t * (rhs._21 - pM1->_21);
	Out._22 = pM1->_22 + t * (rhs._22 - pM1->_22);
	Out._23 = pM1->_23 + t * (rhs._23 - pM1->_23);
	Out._24 = pM1->_24 + t * (rhs._24 - pM1->_24);

	Out._31 = pM1->_31 + t * (rhs._31 - pM1->_31);
	Out._32 = pM1->_32 + t * (rhs._32 - pM1->_32);
	Out._33 = pM1->_33 + t * (rhs._33 - pM1->_33);
	Out._34 = pM1->_34 + t * (rhs._34 - pM1->_34);

	Out._41 = pM1->_41 + t * (rhs._41 - pM1->_41);
	Out._42 = pM1->_42 + t * (rhs._42 - pM1->_42);
	Out._43 = pM1->_43 + t * (rhs._43 - pM1->_43);
	Out._44 = pM1->_44 + t * (rhs._44 - pM1->_44);

	return Out;
}

eg_real eg_mat::GetDeterminant() const
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( *this );
	XMVECTOR vr = XMMatrixDeterminant( m1 );
	return XMVectorGetX( vr );
}

eg_bool eg_mat::IsIdentity() const
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( *this );
	return XMMatrixIsIdentity( m1 );
}

eg_bool eg_mat::IsORT() const
{
	eg_mat T = *this;
	T._41 = T._42 = T._43 = 0;
	return EG_IsEqualEps( T.GetDeterminant() , 1.f , EG_SMALL_EPS );
}

eg_mat eg_mat::BuildInverse( const eg_mat& M , eg_real* OutDet )
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( M );
	XMVECTOR vDet;
	XMMATRIX mr = XMMatrixInverse( &vDet , m1 );
	if( nullptr != OutDet )
	{
		*OutDet = XMVectorGetX( vDet );
	}
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildTranspose( const eg_mat& M )
{
	using namespace DirectX;
	XMMATRIX m1 = XMLoadFloat4x4( M );
	XMMATRIX mr = XMMatrixTranspose( m1 );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildORTInverse( const eg_mat& M )
{
	assert( M.IsORT() );

	//this should be an orthogonal rotation and a translation only
	//the inverse by Lengyel 66.
	//Compute the inverted translation (just the negative of the translation)
	eg_mat mTInv( 1 , 0 , 0 , 0 ,
		0 , 1 , 0 , 0 ,
		0 , 0 , 1 , 0 ,
		-M._41 , -M._42 , -M._43 , 1 );
	//Compute the inverted rotation (the transpose of the translation)
	eg_mat mRInv( M._11 , M._21 , M._31 , 0 ,
		M._12 , M._22 , M._32 , 0 ,
		M._13 , M._23 , M._33 , 0 ,
		0 , 0 , 0 , 1 );

	//The inverse is then simply mTInv*mRInv ( = T^{-1}*R^{-1} = (RT)^{-1} )
	return mTInv * mRInv;
}
eg_mat eg_mat::BuildTransform( const eg_transform& T )
{
	using namespace DirectX;
	XMVECTOR RQ = XMLoadFloat4( T.GetRotation() );
	XMMATRIX RM = XMMatrixRotationQuaternion( RQ );
	XMMATRIX SM = XMMatrixScaling( T.GetScale() , T.GetScale() , T.GetScale() );
	XMMATRIX TM = XMMatrixTranslation( T.GetTranslation().x , T.GetTranslation().y , T.GetTranslation().z );
	XMMATRIX FM = RM * SM * TM;
	eg_mat Out;
	XMStoreFloat4x4( Out , FM );
	return Out;
}

eg_mat eg_mat::BuildTransformNoScale( const eg_transform& T )
{
	using namespace DirectX;
	XMVECTOR RQ = XMLoadFloat4( T.GetRotation() );
	XMMATRIX RM = XMMatrixRotationQuaternion( RQ );
	XMMATRIX TM = XMMatrixTranslation( T.GetTranslation().x , T.GetTranslation().y , T.GetTranslation().z );
	XMMATRIX FM = RM * TM;
	eg_mat Out;
	XMStoreFloat4x4( Out , FM );
	return Out;
}

eg_mat eg_mat::BuildRotation( const eg_quat& Q )
{
	using namespace DirectX;
	XMVECTOR q1 = XMLoadFloat4( Q );
	XMMATRIX mr = XMMatrixRotationQuaternion( q1 );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildTranslation( const eg_vec3& V )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixTranslation( V.x , V.y , V.z );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildScaling( eg_real s )
{
	return BuildScaling( eg_vec3( s , s , s ) );
}

eg_mat eg_mat::BuildScaling( const eg_vec3& V )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( V );
	XMMATRIX mr = XMMatrixScalingFromVector( v1 );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildRotationAxis( const eg_vec3& V , eg_angle Angle )
{
	using namespace DirectX;
	XMVECTOR v1 = XMLoadFloat3( V );
	XMMATRIX mr = XMMatrixRotationAxis( v1 , Angle.ToRad() );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildRotationX( eg_angle Angle )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixRotationX( Angle.ToRad() );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildRotationY( eg_angle Angle )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixRotationY( Angle.ToRad() );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildRotationZ( eg_angle Angle )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixRotationZ( Angle.ToRad() );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildLookAtLH( const eg_vec3& Pos , const eg_vec3& At , const eg_vec3& Up )
{
	using namespace DirectX;
	XMVECTOR XmPos = XMLoadFloat3( Pos );
	XMVECTOR XmAt = XMLoadFloat3( At );
	XMVECTOR XmUp = XMLoadFloat3( Up );
	XMMATRIX mr = XMMatrixLookAtLH( XmPos , XmAt , XmUp );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildPerspectiveFovLH( eg_angle fovy , eg_real Aspect , eg_real zn , eg_real zf )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixPerspectiveFovLH( fovy.ToRad() , Aspect , zn , zf );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildOrthographicLH( eg_real w , eg_real h , eg_real zn , eg_real zf )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixOrthographicLH( w , h , zn , zf );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildLookAtRH( const eg_vec3& Pos , const eg_vec3& At , const eg_vec3& Up )
{
	using namespace DirectX;
	XMVECTOR XmPos = XMLoadFloat3( Pos );
	XMVECTOR XmAt = XMLoadFloat3( At );
	XMVECTOR XmUp = XMLoadFloat3( Up );
	XMMATRIX mr = XMMatrixLookAtRH( XmPos , XmAt , XmUp );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildPerspectiveFovRH( eg_angle fovy , eg_real Aspect , eg_real zn , eg_real zf )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixPerspectiveFovRH( fovy.ToRad() , Aspect , zn , zf );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildOrthographicRH( eg_real w , eg_real h , eg_real zn , eg_real zf )
{
	using namespace DirectX;
	XMMATRIX mr = XMMatrixOrthographicRH( w , h , zn , zf );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

eg_mat eg_mat::BuildReflection( const eg_plane& Plane )
{
	using namespace DirectX;
	XMVECTOR PlnVec = XMLoadFloat4( Plane );
	XMMATRIX mr = XMMatrixReflect( PlnVec );
	eg_mat Out;
	XMStoreFloat4x4( Out , mr );
	return Out;
}

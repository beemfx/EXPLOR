// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

//
// eg_transform * eg_transform
//

static inline eg_transform operator * ( const eg_transform& L , const eg_transform& R )
{
	eg_transform Out;
	Out.s = L.s * R.s;
	Out.R = L.R * R.R;
	Out.T = ( (L.T * R.R) * R.s ) + R.T;
	return Out;
}

//
// eg_vec4 * eg_transform
//

static inline eg_vec4 operator * ( const eg_vec4& L , const eg_transform& R )
{
#if 1
 
	return eg_vec4( ( L.ToVec3() * R.GetRotation() ) * R.GetScale() + R.GetTranslation()*L.w , L.w );

#else
	// Seems to have slightly less perf...
	return L * eg_mat( R );

#endif
}

//
// eg_transform * eg_vec4 - Not a particularly meaningful multiplication...
//

static inline eg_vec4 operator * ( const eg_transform& L , const eg_vec4& R )
{
	#if 1

	return eg_vec4( ( L.GetRotation() * R.ToVec3() ) * L.GetScale() , L.GetTranslation().Dot(R.ToVec3()) + R.w );

	#else
	// Seems to have slightly less perf...
	return L * eg_mat( R );

	#endif
}

//
// eg_mat * eg_mat
//

static inline eg_mat operator * ( const eg_mat& L , const eg_mat& R )
{
	using namespace DirectX;

	XMMATRIX m1 = XMLoadFloat4x4( L );
	XMMATRIX m2 = XMLoadFloat4x4( R );
	XMMATRIX mr = XMMatrixMultiply( m1 , m2 );
	eg_mat T;
	XMStoreFloat4x4( reinterpret_cast<XMFLOAT4X4*>(&T) , mr );
	return T;
}

//
// eg_vec4 * eg_mat
//

static inline eg_vec4 operator * ( const eg_vec4& L , const eg_mat& R )
{
	using namespace DirectX;
	
	XMVECTOR v1 = XMLoadFloat4( L );
	XMMATRIX m1 = XMLoadFloat4x4( R );
	XMVECTOR vr = XMVector4Transform( v1 , m1 );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

//
// eg_mat * eg_vec4
//

static inline eg_vec4 operator * ( const eg_mat& L , const eg_vec4& R )
{
	using namespace DirectX;

	XMVECTOR v1 = XMLoadFloat4( R );
	XMMATRIX m1 = XMLoadFloat4x4( L );
	XMMATRIX mt = XMMatrixTranspose( m1 );
	XMVECTOR vr = XMVector4Transform( v1 , mt );
	eg_vec4 T;
	XMStoreFloat4( T , vr );
	return T;
}

//
// eg_quat * eg_quat
//

static inline eg_quat operator * ( const eg_quat& L , const eg_quat& R )
{
	using namespace DirectX;
	
	XMVECTOR q1 = XMLoadFloat4( L );
	XMVECTOR q2 = XMLoadFloat4( R );
	XMVECTOR qr = XMQuaternionMultiply( q1 , q2 );
	eg_quat qT;
	XMStoreFloat4( qT , qr );
	return qT;
}

//
// eg_vec3 * eg_quat
//

static inline eg_vec3 operator * ( const eg_vec3& L , const eg_quat& R )
{
	using namespace DirectX;

	XMVECTOR LeftV = XMLoadFloat3( L );
	XMVECTOR RightM = XMLoadFloat4( R );
	XMVECTOR InvM = XMQuaternionConjugate( RightM );
	XMVECTOR Final = XMQuaternionMultiply( InvM , LeftV );
	Final = XMQuaternionMultiply( Final , RightM );
	eg_vec3 Out;
	XMStoreFloat3( Out , Final );
	return Out;
}

//
// eg_quat * eg_vec3
//

static inline eg_vec3 operator * ( const eg_quat& L , const eg_vec3& R )
{
	using namespace DirectX;

	XMVECTOR LeftV = XMLoadFloat3( R );
	XMVECTOR RightM = XMLoadFloat4( L );
	XMVECTOR InvM = XMQuaternionConjugate( RightM );
	XMVECTOR Final = XMQuaternionMultiply( RightM , LeftV );
	Final = XMQuaternionMultiply( Final , InvM );
	eg_vec3 Out;
	XMStoreFloat3( Out , Final );
	return Out;
}

static inline void EGMatrix_TransformArray( eg_vec4* VecsOut , eg_int VecsOutStride , const eg_vec4* VecsIn , eg_int VecsInStride , const eg_mat& Matrix , eg_int VecsCount )
{ 
	using namespace DirectX;

	XMMATRIX M1 = XMLoadFloat4x4( Matrix );
	XMVector4TransformStream( reinterpret_cast<XMFLOAT4*>(VecsOut) , VecsOutStride , reinterpret_cast<const XMFLOAT4*>(VecsIn) , VecsInStride , VecsCount , M1 );
}

// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

struct eg_mat;

struct eg_plane
{
	eg_real a , b , c , d;

	eg_plane() = default;
	eg_plane( eg_ctor_t Ct ) { if( Ct == CT_Clear ) { a = b = c = d = 0.f; } else if( Ct == CT_Default ) { a = b = c = 0.f; d = 1.f; } }
	eg_plane( eg_real fa , eg_real fb , eg_real fc , eg_real fd ){ a = fa; b = fb; c = fc; d = fd; }

	eg_real Dot( const eg_vec4& v ) const;
	void Normalize();
	eg_plane operator * ( const eg_mat& rhs ) const;
	const eg_plane& operator *= ( const eg_mat& rhs );
	eg_vec4 InteresectLine( const eg_vec4& A , const eg_vec4& B ) const;

	inline operator DirectX::XMFLOAT4* () { return reinterpret_cast<DirectX::XMFLOAT4*>(this); }
	inline operator const DirectX::XMFLOAT4* () const { return reinterpret_cast<const DirectX::XMFLOAT4*>(this); }
};


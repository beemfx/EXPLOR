// (c) 2017 Beem Media

#pragma once

struct eg_sphere
{
	eg_real Radius;

	eg_sphere() = default;
	eg_sphere( eg_real InRadius ): Radius( InRadius ) { Validate(); }

	void SetupSphere( eg_real InRadius ) { Radius = InRadius; Validate(); }
	eg_real GetRadius() const { return Radius; }
	void Validate() { if( Radius < 0.f ){ Radius = 0.f; } }
};

struct eg_box
{
	eg_real XDim;
	eg_real YDim;
	eg_real ZDim;

	eg_box() = default;
	eg_box( eg_real InXDim , eg_real InYDim , eg_real InZDim ): XDim( InXDim ) , YDim( InYDim ) , ZDim( InZDim ){ }
	eg_box( const eg_vec3& rhs ): eg_box( rhs.x , rhs.y , rhs.z ){ }

	void SetupBox( eg_real InXDim , eg_real InYDim , eg_real InZDim ){ XDim = InXDim; YDim = InYDim; ZDim = InZDim; }
	void SetupBox( const eg_vec3& rhs ) { SetupBox( rhs.x , rhs.y , rhs.z ); }

	eg_real GetXDim() const { return XDim; }
	eg_real GetYDim() const { return YDim; }
	eg_real GetZDim() const { return ZDim; }

	eg_vec3 GetDims() const { return eg_vec3( XDim , YDim , ZDim ); }
	eg_vec3 GetMins() const { return eg_vec3( -XDim*.5f , -YDim*.5f , -ZDim*.5f ); }
	eg_vec3 GetMaxes() const { return eg_vec3( XDim*.5f , YDim*.5f , ZDim*.5f ); }

	eg_real GetWidth() const { return GetXDim(); }
	eg_real GetHeight() const { return GetYDim(); }
	eg_real GetDepth() const { return GetZDim(); }

	void Validate(){ if( XDim < 0.f ){ XDim = 0.f; } if( YDim < 0.f ){ YDim = 0.f; } if( ZDim < 0.f ){ ZDim = 0.f; } }
};

struct eg_capsule
{
	eg_real Height;
	eg_real Radius;

	eg_capsule() = default;
	eg_capsule( eg_real InHeight , eg_real InRadius ): Height( InHeight ) , Radius( InRadius ) { Validate(); }

	void SetupCapsule( eg_real InHeight , eg_real InRadius ){ Height = InHeight; Radius = InRadius; Validate(); }
	eg_real GetHeight() const { return Height; }
	eg_real GetRadius() const { return Radius; }

	void Validate() { if( Radius < 0.f ){ Radius = 0.f; } if( Height < (Radius*2.f) ) { Height = Radius*2.f; } }
};

struct eg_cylinder
{
	eg_real Height;
	eg_real Radius;

	eg_cylinder() = default;
	eg_cylinder( eg_real InHeight , eg_real InRadius ): Height( InHeight ) , Radius( InRadius ) { Validate(); }

	void SetupCylinder( eg_real InHeight , eg_real InRadius ){ Height = InHeight; Radius = InRadius; Validate(); }
	eg_real GetHeight() const { return Height; }
	eg_real GetRadius() const { return Radius; }

	void Validate() { if( Radius < 0.f ){ Radius = 0.f; } if( Height < 0.f ) { Height = 0.f; } }
};

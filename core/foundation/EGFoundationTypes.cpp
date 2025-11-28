// (c) 2018 Beem Media

#include "EGFoundationTypes.h"


eg_aabb eg_t_shape_union::GetAABB() const
{
	eg_aabb Out;

	switch( Type )
	{
	case eg_shape_t::Box:
	{
		Out.Min = eg_vec4( Box.GetMins() , 1.f );
		Out.Max = eg_vec4( Box.GetMaxes() , 1.f );
	} break;
	case eg_shape_t::Capsule:
	{
		Out.Min = eg_vec4( -Capsule.GetRadius() , -Capsule.GetHeight()*.5f , -Capsule.GetRadius() , 1.f );
		Out.Max = eg_vec4( Capsule.GetRadius() , Capsule.GetHeight()*.5f , Capsule.GetRadius() , 1.f );
	} break;
	case eg_shape_t::Cylinder:
	{
		Out.Min = eg_vec4( -Cylinder.GetRadius() , -Cylinder.GetHeight()*.5f , -Cylinder.GetRadius() , 1.f );
		Out.Max = eg_vec4( Cylinder.GetRadius() , Cylinder.GetHeight()*.5f , Cylinder.GetRadius() , 1.f );
	} break;
	case eg_shape_t::Sphere:
	{
		Out.Min = eg_vec4( -Sphere.GetRadius() , -Sphere.GetRadius() , -Sphere.GetRadius() , 1.f );
		Out.Max = eg_vec4( Sphere.GetRadius() , Sphere.GetRadius() , Sphere.GetRadius() , 1.f );
		// Spheres just need to be offset by the position.
		Out.Min += eg_vec4( Transform.GetTranslation() , 0.f );
		Out.Max += eg_vec4( Transform.GetTranslation() , 0.f );
		return Out;
	} break;
	}

	eg_vec4 Corners[8];
	Out.Get8Corners( Corners , countof(Corners) );
	for( eg_vec4& Corner : Corners )
	{
		Corner *= Transform;
	}
	Out.CreateFromVec4s( Corners , countof(Corners) );

	return Out;
}

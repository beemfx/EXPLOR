// (c) 2018 Beem Media

#pragma once

#include "EGFoundationTypes.reflection.h"

egreflect enum class eg_shape_t
{
	Sphere,
	Box,
	Capsule,
	Cylinder,
};

egreflect enum class eg_text_align_ed
{
	LEFT,
	CENTER,
	RIGHT,
};

struct eg_t_shape_union
{
	eg_shape_t   Type;
	eg_transform Transform;

	union
	{
		eg_sphere   Sphere;
		eg_box      Box;
		eg_capsule  Capsule;
		eg_cylinder Cylinder;
	};

	eg_t_shape_union() = default;
	eg_t_shape_union( const eg_sphere& InSphere ): Type( eg_shape_t::Sphere ), Transform( CT_Default ) { Sphere = InSphere; }
	eg_t_shape_union( const eg_box& InBox ): Type( eg_shape_t::Box ), Transform( CT_Default ) { Box = InBox; }
	eg_t_shape_union( const eg_capsule& InCapsule ): Type( eg_shape_t::Capsule ), Transform( CT_Default ) { Capsule = InCapsule; }
	eg_t_shape_union( const eg_cylinder& InCylinder ): Type( eg_shape_t::Cylinder ), Transform( CT_Default ) { Cylinder = InCylinder; }

	eg_aabb GetAABB() const;
};

struct eg_t_sphere
{
	eg_sphere    Sphere;
	eg_transform Transform;

	eg_t_sphere() = default;

	eg_t_sphere( const eg_sphere& InSphere , const eg_transform& InTransform )
		: Sphere( InSphere )
		, Transform( InTransform )
	{
	}

	eg_vec4 GetCenter() const { return eg_vec4( Transform.GetTranslation() , 1.f ); }
	eg_real GetRadius() const { return Sphere.GetRadius(); }
};


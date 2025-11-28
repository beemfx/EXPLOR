// (c) 2012 Beem Media

#pragma once

#include "EGShapes.h"
#include "EGFoundationTypes.h"

struct egPhysBodyDefFlags
{
	eg_bool bKinematicEnabled:1;
	eg_bool bCollisionEnabled:1;
	eg_bool bRaycastHitEnabled:1;
	eg_bool bIsTrigger:1;
	eg_bool bNoRotateX:1;
	eg_bool bNoRotateY:1;
	eg_bool bNoRotateZ:1;
	eg_bool bNoMoveX:1;
	eg_bool bNoMoveY:1;
	eg_bool bNoMoveZ:1;

	egPhysBodyDefFlags() = default;

	egPhysBodyDefFlags( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear )
		{
			zero( this );
		}

		if( Ct == CT_Default )
		{
			zero( this );
			bCollisionEnabled = true;
			bRaycastHitEnabled = true;
		}
	}
};

struct egPhysShapeDef
{
	eg_t_shape_union Shape;
	eg_real          Mass;
	eg_real          StaticFric;
	eg_real          DynamicFric;
	eg_real          Restitution;
};

struct egPhysBodyDef
{
	eg_string_small                ClassName;
	egPhysBodyDefFlags             Flags;
	eg_aabb                        BoundingBox; //A computed aabb that makes up the full body of the entity.
	EGFixedArray<egPhysShapeDef,4> Shapes; //A list of shape descriptions that make up this body:

	egPhysBodyDef();
	~egPhysBodyDef();

	void Reset();
	void Finalize();
};
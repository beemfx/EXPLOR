// (c) 2018 Beem Media

#pragma once

#include "EGPhysBodyDef.h"
#include "EGFoundationTypes.h"
#include "EGPhysBodyComponentTypes.reflection.h"

struct egRflEditor;

egreflect struct eg_phys_body_flags
{
	egprop eg_bool bKinematicEnabled  = false;
	egprop eg_bool bCollisionEnabled  = true;
	egprop eg_bool bRaycastHitEnabled = true;
	egprop eg_bool bIsTrigger         = false;
	egprop eg_bool bNoRotateX         = false;
	egprop eg_bool bNoRotateY         = false;
	egprop eg_bool bNoRotateZ         = false;
	egprop eg_bool bNoMoveX           = false;
	egprop eg_bool bNoMoveY           = false;
	egprop eg_bool bNoMoveZ           = false;

	const eg_phys_body_flags& operator = ( const egPhysBodyDefFlags& In );
	operator egPhysBodyDefFlags() const;
};

egreflect struct eg_phys_shape
{
	egprop eg_shape_t   Type = eg_shape_t::Box;
	egprop eg_transform Pose = CT_Default; //Pose of the shape in the entity's local space.
	egprop eg_real      Mass = 60.f;

	//Shape material:
	egprop eg_real StaticFriction  = 0.f;
	egprop eg_real DynamicFriction = 0.f;
	egprop eg_real Restitution     = 0.f;

	//Shared by sphere, cylinder, and capsule:
	egprop eg_real Radius = 1.f;
	egprop eg_real Height = 1.f;

	//Use by the box only:
	egprop eg_real XDim = 1.f;
	egprop eg_real YDim = 1.f;
	egprop eg_real ZDim = 1.f;

	eg_phys_shape() = default;

	eg_phys_shape( const egPhysShapeDef& InPhysShape );

	void RefreshEditableProperties( egRflEditor& ThisEditor );
	operator egPhysShapeDef() const;
	void Validate();
};

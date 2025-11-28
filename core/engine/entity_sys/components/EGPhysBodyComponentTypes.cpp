// (c) 2018 Beem Media

#include "EGPhysBodyComponentTypes.h"
#include "EGReflection.h"

const eg_phys_body_flags& eg_phys_body_flags::operator = ( const egPhysBodyDefFlags& In )
{
#define CPYFLG( a ) a = In.a
	CPYFLG( bKinematicEnabled );
	CPYFLG( bKinematicEnabled );
	CPYFLG( bCollisionEnabled );
	CPYFLG( bRaycastHitEnabled );
	CPYFLG( bIsTrigger );
	CPYFLG( bNoRotateX );
	CPYFLG( bNoRotateY );
	CPYFLG( bNoRotateZ );
	CPYFLG( bNoMoveX );
	CPYFLG( bNoMoveY );
	CPYFLG( bNoMoveZ );
#undef CPYFLG
	return *this;
}

eg_phys_body_flags::operator egPhysBodyDefFlags () const
{
	egPhysBodyDefFlags Out;
#define CPYFLG( a ) Out.a = a;
	CPYFLG( bKinematicEnabled );
	CPYFLG( bKinematicEnabled );
	CPYFLG( bCollisionEnabled );
	CPYFLG( bRaycastHitEnabled );
	CPYFLG( bIsTrigger );
	CPYFLG( bNoRotateX );
	CPYFLG( bNoRotateY );
	CPYFLG( bNoRotateZ );
	CPYFLG( bNoMoveX );
	CPYFLG( bNoMoveY );
	CPYFLG( bNoMoveZ );
#undef CPYFLG
	return Out;
}

eg_phys_shape::eg_phys_shape( const egPhysShapeDef& InPhysShape )
{
	Type = InPhysShape.Shape.Type;
	Pose = InPhysShape.Shape.Transform;
	Mass = InPhysShape.Mass;

	StaticFriction = InPhysShape.StaticFric;
	DynamicFriction = InPhysShape.DynamicFric;
	Restitution = InPhysShape.Restitution;

	if( Type == eg_shape_t::Sphere )
	{
		Radius = InPhysShape.Shape.Sphere.Radius;
	}

	if( Type == eg_shape_t::Box )
	{
		XDim = InPhysShape.Shape.Box.XDim;
		YDim = InPhysShape.Shape.Box.YDim;
		ZDim = InPhysShape.Shape.Box.ZDim;
	}

	if( Type == eg_shape_t::Cylinder )
	{
		Radius = InPhysShape.Shape.Cylinder.Radius;
		Height = InPhysShape.Shape.Cylinder.Height;
	}

	if( Type == eg_shape_t::Capsule )
	{
		Radius = InPhysShape.Shape.Capsule.Radius;
		Height = InPhysShape.Shape.Capsule.Height;
	}
}

void eg_phys_shape::RefreshEditableProperties( egRflEditor& ThisEditor )
{
	auto SetPropEditable = [&ThisEditor]( eg_cpstr Name , eg_bool bEditable ) -> void
	{
		egRflEditor* VarEd = ThisEditor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetEditable( bEditable );
		}
	};

	SetPropEditable( "Radius" , false );
	SetPropEditable( "Height" , false );
	SetPropEditable( "XDim" , false );
	SetPropEditable( "YDim" , false );
	SetPropEditable( "ZDim" , false );

	switch( Type )
	{
	case eg_shape_t::Sphere:
		SetPropEditable( "Radius" , true );
		break;
	case eg_shape_t::Capsule:
	case eg_shape_t::Cylinder:
		SetPropEditable( "Radius" , true );
		SetPropEditable( "Height" , true );
		break;
	case eg_shape_t::Box:
		SetPropEditable( "XDim" , true );
		SetPropEditable( "YDim" , true );
		SetPropEditable( "ZDim" , true );
		break;

	}
}

eg_phys_shape::operator egPhysShapeDef() const
{
	egPhysShapeDef Out;

	Out.Shape.Type = Type;
	Out.Shape.Transform = Pose;
	Out.Mass = Mass;

	Out.StaticFric = StaticFriction;
	Out.DynamicFric = DynamicFriction;
	Out.Restitution = Restitution;

	if( Type == eg_shape_t::Sphere )
	{
		Out.Shape.Sphere.Radius = Radius;
	}

	if( Type == eg_shape_t::Box )
	{
		Out.Shape.Box.XDim = XDim;
		Out.Shape.Box.YDim = YDim;
		Out.Shape.Box.ZDim = ZDim;
	}

	if( Type == eg_shape_t::Capsule )
	{
		Out.Shape.Capsule.Radius = Radius;
		Out.Shape.Capsule.Height = Height;
	}

	if( Type == eg_shape_t::Cylinder )
	{
		Out.Shape.Cylinder.Radius = Radius;
		Out.Shape.Cylinder.Height = Height;
	}

	return Out;
}

void eg_phys_shape::Validate()
{
	if( Radius < 0.f )
	{
		Radius = 0.f;
	}

	if( Height < 0.f )
	{
		Height = 0.f;
	}

	if( XDim < 0.f )
	{
		XDim = 0.f;
	}

	if( YDim < 0.f )
	{
		YDim = 0.f;
	}

	if( ZDim < 0.f )
	{
		ZDim = 0.f;
	}

	if( Mass < 0.f )
	{
		Mass = 0.f;
	}

	if( Type == eg_shape_t::Capsule )
	{
		if( Height < 2.f*Radius )
		{
			Height = 2.f*Radius;
		}
	}
}

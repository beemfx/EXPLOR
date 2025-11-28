// (c) 2012 Beem Media

#include "EGPhysBodyDef.h"
#include "EGFileData.h"

egPhysBodyDef::egPhysBodyDef()
: ClassName( CT_Clear )
, Flags( CT_Default )
, BoundingBox( CT_Default )
, Shapes( CT_Clear )
{

}

egPhysBodyDef::~egPhysBodyDef()
{

}

void egPhysBodyDef::Finalize()
{
	//If no shapes were added we create a default box
	if(0 == Shapes.Len())
	{
		if( !Flags.bCollisionEnabled && !Flags.bRaycastHitEnabled )
		{
			EGLogf( eg_log_t::Warning , __FUNCTION__ ": No shapes specified, creating default 1m box." );
		}
		egPhysShapeDef si;
		si.Shape = eg_t_shape_union( eg_box( 1.f , 1.f , 1.f ) );
		si.Mass = 10.0f;
		si.Shape.Transform  = eg_transform::BuildIdentity();
		si.DynamicFric = 0.5f;
		si.StaticFric  = 0.5f;
		si.Restitution = 0.1f;
				
		Shapes.Push(si);
	}

	//We will now compute a base aabb based on the shapes.
	for( eg_uint i=0; i<Shapes.Len(); i++ )
	{
		const egPhysShapeDef& Shape = Shapes[i];

		if( 0 == i )
		{
			BoundingBox = Shape.Shape.GetAABB();
		}
		else
		{
			BoundingBox.AddBox( Shape.Shape.GetAABB() );
		}
	}
}

void egPhysBodyDef::Reset()
{
	Shapes.Clear();
	ClassName = CT_Clear;
	BoundingBox = eg_aabb( CT_Default );
	Flags = egPhysBodyDefFlags( CT_Default );
}

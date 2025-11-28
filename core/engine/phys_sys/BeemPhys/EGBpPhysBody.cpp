// (c) 2025 Beem Media. All rights reserved.

#include "EGBpPhysBody.h"
#include "EGPhysBodyDef.h"
#include "EGBpPhysSim.h"

EG_CLASS_DECL( EGBpPhysBody )
EG_CLASS_DECL( EGPxPhysBody )
EG_CLASS_DECL( EGPxControlPhysBody )

void EGBpPhysBody::InitBp( EGBpPhysSim* OwnerSim )
{
	m_SimOwner = OwnerSim;

	if (m_SimOwner)
	{
		m_SimOwner->RegisterBody(*this);
	}
}

void EGBpPhysBody::SetLinearVelocity( const eg_vec4& LinVel )
{
	Super::SetLinearVelocity( LinVel );
}

void EGBpPhysBody::SetAngularVelocity( const eg_vec4& AngVel )
{
	Super::SetAngularVelocity( AngVel );
}

void EGBpPhysBody::SetPose( const eg_transform& Pose )
{
	Super::SetPose( Pose );
	
	if( IsKinematicEnabled() )
	{
		RefreshFromSimulation();
	}
}

void EGBpPhysBody::MoveToPose( const eg_transform& Pose )
{
	Super::MoveToPose( Pose );

	if( IsKinematicEnabled() )
	{
		RefreshFromSimulation();
	}
}

void EGBpPhysBody::AddForce( const eg_vec4& ForceVector )
{
	Super::AddForce( ForceVector );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::AddImpulse( const eg_vec4& ForceVector )
{
	Super::AddImpulse( ForceVector );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::AddTorque( const eg_vec4& TorqueVector )
{
	Super::AddTorque( TorqueVector );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::SetKinematicEnabled( eg_bool bEnabled )
{
	Super::SetKinematicEnabled( bEnabled );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::SetCollisionEnabled( eg_bool bEnabled )
{
	Super::SetCollisionEnabled( bEnabled );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::SetRaycastHitEnabled( eg_bool bEnabled )
{
	Super::SetRaycastHitEnabled( bEnabled );

	// Not implemented in BeemPhys.
}

void EGBpPhysBody::RefreshFromSimulation()
{
	Super::RefreshFromSimulation();
}

void EGBpPhysBody::OnDestruct()
{
	Super::OnDestruct();

	if (m_SimOwner)
	{
		m_SimOwner->UnregisterBody(*this);
	}

	if( m_Owner )
	{
		m_Owner->PhysBodySetPhysBody( nullptr );
	}
}

void EGBpPhysBody::InitFromDef( IEGPhysBodyOwner* Owner, const egPhysBodyDef& PhysBodyDef, const eg_transform& Pose )
{
	Super::InitFromDef( Owner , PhysBodyDef , Pose );

	BuildShapes( PhysBodyDef );
}

eg_bool EGBpPhysBody::DoesRayIntersect( const eg_vec3& Org , const eg_vec3& Dir , eg_vec3* OutHit ) const
{
	if (!IsRaycastHitEnabled())
	{
		return false;
	}

	eg_vec3 ClosestHit = CT_Clear;
	eg_bool bHasClosestHit = false;
	eg_real ClosestHitDist = 0.f;

	for (auto& Item : m_Shapes)
	{
		const eg_transform ShapeTransform = Item.LocalPose * GetPose();
		const eg_transform ShapeSpaceTransform = ShapeTransform.GetInverse();
		const eg_vec3 OrgInShapeSpace = (eg_vec4(Org, 1.f) * ShapeSpaceTransform).XYZ_ToVec3();
		const eg_vec3 DirInShapeSpace = (eg_vec4(Dir, 0.f) * ShapeSpaceTransform).XYZ_ToVec3();

		const auto HitRes = Item.Box.GetRayIntersection(OrgInShapeSpace, DirInShapeSpace);
		if (HitRes.IsHit())
		{
			if (bHasClosestHit)
			{
				const eg_vec3 HitWorldPos = (eg_vec4(HitRes.HitPos, 1.f) * ShapeTransform).XYZ_ToVec3();
				const eg_real HitDist = (HitWorldPos - Org).Len();
				if (HitDist < ClosestHitDist)
				{
					ClosestHit = HitWorldPos;
					ClosestHitDist = HitDist;
				}
			}
			else
			{
				ClosestHit = (eg_vec4(HitRes.HitPos, 1.f) * ShapeTransform).XYZ_ToVec3();
				ClosestHitDist = (ClosestHit - Org).Len();
				bHasClosestHit = true;
			}
		}
	}

	if (OutHit)
	{
		*OutHit = bHasClosestHit ? ClosestHit : CT_Clear;
	}

	return bHasClosestHit;
}

void EGBpPhysBody::BuildShapes( const egPhysBodyDef& Def )
{
	for( const egPhysShapeDef& Shape : Def.Shapes )
	{
		egBodyShape BodyShape;

		switch( Shape.Shape.Type )
		{
		case eg_shape_t::Sphere:
		{
			const eg_real Radius = Shape.Shape.Sphere.GetRadius();
			const eg_vec3 Ext( Radius , Radius , Radius );
			BodyShape.Box = eg_aabb( eg_vec4( -Ext , 1.f ) , eg_vec4( Ext , 1.f ) );
			BodyShape.LocalPose = Shape.Shape.Transform;
			if( !m_Shapes.IsFull() )
			{
				m_Shapes.Append( BodyShape );
			}
		} break;
		case eg_shape_t::Box:
		{
			const eg_vec3 Ext( Shape.Shape.Box.GetXDim() * .5f , Shape.Shape.Box.GetYDim() * .5f , Shape.Shape.Box.GetZDim() * .5f );
			BodyShape.Box = eg_aabb( eg_vec4( -Ext , 1.f ) , eg_vec4( Ext , 1.f ) );
			BodyShape.LocalPose = Shape.Shape.Transform;
			if( !m_Shapes.IsFull() )
			{
				m_Shapes.Append( BodyShape );
			}
		} break;
		case eg_shape_t::Capsule:
		{
			// Our capsules our defined as a radius and height where the height
			// is from tip to so effectively our dimensions are already a box.
			const eg_real Radius = Shape.Shape.Capsule.GetRadius();
			const eg_real Height = Shape.Shape.Capsule.GetHeight();
			const eg_vec3 Ext( Radius , Radius , Height * .5f );
			BodyShape.Box = eg_aabb( eg_vec4( -Ext , 1.f ) , eg_vec4( Ext , 1.f ) );
			BodyShape.LocalPose = Shape.Shape.Transform;
			if( !m_Shapes.IsFull() )
			{
				m_Shapes.Append( BodyShape );
			}
		} break;
		case eg_shape_t::Cylinder:
			const eg_real Radius = Shape.Shape.Cylinder.GetRadius();
			const eg_real Height = Shape.Shape.Cylinder.GetHeight();
			const eg_vec3 Ext( Radius , Radius , Height * .5f );
			BodyShape.Box = eg_aabb( eg_vec4( -Ext , 1.f ) , eg_vec4( Ext , 1.f ) );
			BodyShape.LocalPose = Shape.Shape.Transform;
			if( !m_Shapes.IsFull() )
			{
				m_Shapes.Append( BodyShape );
			}
			break;
		}
	}
}

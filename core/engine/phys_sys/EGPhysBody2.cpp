// (c) 2017 Beem Media

#include "EGPhysBody2.h"
#include "EGPhysBodyDef.h"

EG_CLASS_DECL( EGPhysBody2 )

void EGPhysBody2::InitFromDef( IEGPhysBodyOwner* Owner , const egPhysBodyDef& PhysBodyDef , const eg_transform& Pose )
{
	unused( PhysBodyDef , Pose );

	m_BaseBoundingBox = PhysBodyDef.BoundingBox;
	m_State.Pose = Pose;
	m_State.LinearVelocity = eg_vec4( CT_Clear );
	m_State.AngularVelocity = eg_vec4( CT_Clear );
	m_State.BoundingBox.Min = m_BaseBoundingBox.Min + m_State.Pose.GetPosition();
	m_State.BoundingBox.Max = m_BaseBoundingBox.Max + m_State.Pose.GetPosition();
	m_State.bKinematicEnabled = PhysBodyDef.Flags.bKinematicEnabled;
	m_State.bCollisionsEnabled = PhysBodyDef.Flags.bCollisionEnabled;
	m_State.bRaycastHitEnabled = PhysBodyDef.Flags.bRaycastHitEnabled;
	m_State.bIsTrigger = PhysBodyDef.Flags.bIsTrigger;

	m_Owner = Owner;

	if( m_Owner )
	{
		m_Owner->PhysBodySetPhysBody( this );
	}
}

void EGPhysBody2::RefreshFromSimulation()
{
	m_State.BoundingBox.Min = m_BaseBoundingBox.Min + m_State.Pose.GetPosition();
	m_State.BoundingBox.Max = m_BaseBoundingBox.Max + m_State.Pose.GetPosition();
	m_State.BoundingBox.Min.w = 1.f;
	m_State.BoundingBox.Max.w = 1.f;
	if( m_Owner ) 
	{
		m_Owner->PhysBodyOnUpdateState( m_State );
	}
}

void EGPhysBody2::UpdateState( const egPhysBodyState& NewState )
{
	m_State = NewState;
	if( m_Owner )
	{
		m_Owner->PhysBodyOnUpdateState( m_State );
	}
}

void EGPhysBody2::SetState( const egPhysBodyState& State , eg_bool bIgnorePose )
{
	SetKinematicEnabled( State.bKinematicEnabled );
	SetCollisionEnabled( State.bCollisionsEnabled );
	SetRaycastHitEnabled( State.bRaycastHitEnabled );

	if( !State.bKinematicEnabled )
	{
		SetLinearVelocity( State.LinearVelocity );
		SetAngularVelocity( State.AngularVelocity );
	}
	
	if( !bIgnorePose )
	{
		SetPose( State.Pose );
	}
}

void EGPhysBody2::HandleContact( eg_phys_body_contact_t ContactType , eg_phys_body_contact_event ContactEvent , eg_ent_id ContactEntId )
{
	if( m_Owner )
	{
		m_Owner->PhysOnContact( ContactType , ContactEvent , ContactEntId );
	}
}

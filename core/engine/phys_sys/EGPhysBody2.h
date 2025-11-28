// (c) 2017 Beem Media

#pragma once

struct egPhysBodyDef;

enum class eg_phys_body_contact_t
{
	Unknown,
	Trigger,
};

enum class eg_phys_body_contact_event
{
	Unknown,
	BeginContact,
	EndContact,
};

struct egPhysBodyState
{
	eg_transform Pose;
	eg_vec4      LinearVelocity;
	eg_vec4      AngularVelocity;
	eg_aabb      BoundingBox;
	eg_bool      bKinematicEnabled:1;
	eg_bool      bCollisionsEnabled:1;
	eg_bool      bRaycastHitEnabled:1;
	eg_bool      bIsTrigger:1;

	egPhysBodyState() = default;
	egPhysBodyState( eg_ctor_t Ct )
	: Pose( Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			LinearVelocity = eg_vec4(0.f,0.f,0.f,0.f);
			AngularVelocity = eg_vec4(0.f,0.f,0.f,0.f);
			BoundingBox.Min = eg_vec4(0.f,0.f,0.f,1.f);
			BoundingBox.Max = eg_vec4(0.f,0.f,0.f,1.f);
			bKinematicEnabled = false;
			bCollisionsEnabled = Ct == CT_Default;
			bRaycastHitEnabled = Ct == CT_Default;
			bIsTrigger = false;
		}
	}
};

class IEGPhysBodyOwner
{
public:
	
	virtual void PhysBodySetPhysBody( class EGPhysBody2* PhysBody ) = 0;
	virtual void PhysBodyOnUpdateState( const egPhysBodyState& NewState ) = 0; 
	virtual eg_ent_id PhysBodyGetEntId() const = 0;
	virtual void PhysOnContact( eg_phys_body_contact_t ContactType , eg_phys_body_contact_event ContactEvent , eg_ent_id ContactEntId ) { unused( ContactType , ContactEvent , ContactEntId ); }
};

class EGPhysBody2 : public EGObject
{
	EG_CLASS_BODY( EGPhysBody2 , EGObject )

protected:

	IEGPhysBodyOwner* m_Owner = nullptr;
	egPhysBodyState   m_State;
	eg_aabb           m_BaseBoundingBox;

public:

	virtual void InitFromDef( IEGPhysBodyOwner* Owner , const egPhysBodyDef& PhysBodyDef , const eg_transform& Pose ); 
	virtual void SetLinearVelocity( const eg_vec4& LinVel ){ m_State.LinearVelocity = LinVel; }
	virtual void SetAngularVelocity( const eg_vec4& AngVel ){ m_State.AngularVelocity = AngVel; }
	virtual void SetPose( const eg_transform& Pose ){ m_State.Pose = Pose; }
	virtual void MoveToPose( const eg_transform& Pose ){ m_State.Pose = Pose; }
	virtual void AddForce( const eg_vec4& ForceVector ){ unused( ForceVector ); }
	virtual void AddImpulse( const eg_vec4& ForceVector ){ unused( ForceVector ); }
	virtual void AddTorque( const eg_vec4& TorqueVector ){ unused( TorqueVector ); }
	virtual void SetKinematicEnabled( eg_bool bEnabled ){ m_State.bKinematicEnabled = bEnabled; }
	virtual void SetCollisionEnabled( eg_bool bEnabled ){ m_State.bCollisionsEnabled = bEnabled; }
	virtual void SetRaycastHitEnabled( eg_bool bEnabled ){ m_State.bRaycastHitEnabled = bEnabled; }
	virtual void RefreshFromSimulation();

	void UpdateState( const egPhysBodyState& NewState );
	eg_ent_id GetEntId() const { return m_Owner ? m_Owner->PhysBodyGetEntId() : INVALID_ID; }
	const egPhysBodyState& GetState() { return m_State; }
	void SetState( const egPhysBodyState& State , eg_bool bIgnorePose );
	eg_transform GetPose() const { return m_State.Pose; }
	eg_bool IsKinematicEnabled() const { return m_State.bKinematicEnabled; }
	eg_bool IsCollisionEnabled() const { return m_State.bCollisionsEnabled; }
	eg_bool IsRaycastHitEnabled() const { return m_State.bRaycastHitEnabled; }
	eg_vec4 GetLinearVelocity() const { return m_State.LinearVelocity; }
	eg_vec4 GetAngularVelocity() const { return m_State.AngularVelocity; }
	void SetRelativeLinearVelocity( const eg_vec4& RelLinVel ){ SetLinearVelocity( RelLinVel * m_State.Pose ); }
	void SetRelativeAngularVelocity( const eg_vec4& RelAngVel ){ SetAngularVelocity( RelAngVel * m_State.Pose ); }
	void AddLocalForce( const eg_vec4& ForceVector ){ AddForce( ForceVector * m_State.Pose ); }
	void AddLocalImpulse( const eg_vec4& ForceVector ){ AddImpulse( ForceVector * m_State.Pose ); }
	void HandleContact( eg_phys_body_contact_t ContactType , eg_phys_body_contact_event ContactEvent , eg_ent_id ContactEntId );
};

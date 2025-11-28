// (c) 2025 Beem Media. All rights reserved.

#pragma once

#include "EGPhysBody2.h"
#include "EGBpPhysLib.h"
#include "EGWeakPtr.h"

class EGBpPhysSim;

class EGBpPhysBody : public EGPhysBody2
{
	EG_CLASS_BODY( EGBpPhysBody , EGPhysBody2 )

protected:

friend class EGBpPhysSim;

	EGWeakPtr<EGBpPhysSim> m_SimOwner;

	static const eg_size_t MAX_SHAPES_PER_ACTOR = 5;

	EGBpPhysBodyListHook m_MasterListHook;
	eg_bool m_bRegisteredInMasterList = false;

	// EX-Release: In our simple ray-traceable only sim everything is a box.
	struct egBodyShape
	{
		eg_aabb Box = {{0.f,0.f,0.f,1.f},{0.f,0.f,0.f,1.f}};
		eg_transform LocalPose = eg_transform::I;
	};

	EGFixedArray<egBodyShape,MAX_SHAPES_PER_ACTOR> m_Shapes;

public:

	void InitBp( EGBpPhysSim* OwnerSim );

	// BEGIN EGPhysBody2
	virtual void SetLinearVelocity( const eg_vec4& LinVel ) override;
	virtual void SetAngularVelocity( const eg_vec4& AngVel ) override;
	virtual void SetPose( const eg_transform& Pose ) override;
	virtual void MoveToPose( const eg_transform& Pose ) override;
	virtual void AddForce( const eg_vec4& ForceVector ) override;
	virtual void AddImpulse( const eg_vec4& ForceVector ) override;
	virtual void AddTorque( const eg_vec4& TorqueVector ) override;
	virtual void SetKinematicEnabled( eg_bool bEnabled ) override;
	virtual void SetCollisionEnabled( eg_bool bEnabled ) override;
	virtual void SetRaycastHitEnabled( eg_bool bEnabled ) override;
	virtual void RefreshFromSimulation() override;
	virtual void OnDestruct() override;
	virtual void InitFromDef( IEGPhysBodyOwner* Owner , const egPhysBodyDef& PhysBodyDef , const eg_transform& Pose ) override;
	// END EGPhysBody2

	eg_bool DoesRayIntersect( const eg_vec3& Org , const eg_vec3& Dir , eg_vec3* OutHit ) const;

protected:
	
	void BuildShapes( const egPhysBodyDef& Def );
};

// EX-Release: Everything is a EGBpPhysBody, which is basically a kinematic
// non-colliding, ray-traceable body, but lots of game data refers to other
// types of bodies, so create instances of them to fall back to EGBpPhysBody.
class EGPxPhysBody : public EGBpPhysBody
{
	EG_CLASS_BODY( EGPxPhysBody , EGBpPhysBody )
};

class EGPxControlPhysBody : public EGBpPhysBody
{
	EG_CLASS_BODY( EGPxControlPhysBody , EGBpPhysBody )
};

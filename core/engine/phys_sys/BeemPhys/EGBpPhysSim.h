// (c) 2025 Beem Media. All rights reserved.

#pragma once

#include "EGPhysSim2.h"
#include "EGBpPhysLib.h"
#include "EGPhysBodyDef.h"
#include "EGHeap2.h"
#include "EGPhysMemMgr.h"

class EGGameMap;
class EGTerrain2;
class EGBpPhysBody;

// EX-Release: EGBpPhysSim is almost the simplest physics simulation you can
// imagine. All it does is allow the creation of box based shapes and
// ray-casting them. This serves the needs of EXPLOR since all that game needs
// is to ray-cast for doors. This simulation is not even particularly optimized
// for that (see RaycastClosestShape), but it's a simple game so it works well
// enough and we don't have all the baggage associated with a larger system such
// as PhysX.
class EGBpPhysSim 
	: public EGPhysSim2 
{
	EG_CLASS_BODY( EGBpPhysSim , EGPhysSim2 )

public:

	// BEGIN EGPhysSim2
	virtual void OnConstruct() override;
	virtual void InitScene() override;
	virtual void OnDestruct() override;
	virtual void AddGameMap( const EGGameMap* GameMap ) override;
	virtual void AddTerrain( const EGTerrain2* Terrain , const eg_transform& Pose ) override;
	virtual void SetGravity( const eg_vec4& GravityVec ) override;
	virtual void Simulate( eg_real DeltaTime ) override;
	virtual void CreatePhysBody( IEGPhysBodyOwner* PhysBodyOwner , const egPhysBodyDef& PhysBodyDef , const eg_transform& Pose ) override;
	virtual eg_phys_group RaycastClosestShape( const eg_vec4& vOrg , const eg_vec4& vDir , eg_vec4* pvHitOut , eg_ent_id* pnEntOut ) override;
	// END EGPhysSim2

	void RegisterBody( EGBpPhysBody& InBody );
	void UnregisterBody( EGBpPhysBody& InBody );

private:

	EGBpPhysBodyList m_MasterBodyList;

private:

	void CreateScene();
	void ClearScene();
};

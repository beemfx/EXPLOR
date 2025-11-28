// (c) 2017 Beem Media

#pragma once

struct egPhysBodyDef;
class EGGameMap;
class EGTerrain2;
class IEGPhysBodyOwner;
class EGPhysBody2;

class EGPhysSim2 : public EGObject
{
	EG_CLASS_BODY( EGPhysSim2 , EGObject )

public:

	virtual void InitScene();
	virtual void AddGameMap( const EGGameMap* GameMap ){ unused( GameMap ); }
	virtual void AddTerrain( const EGTerrain2* Terrain , const eg_transform& Pose ){ unused( Terrain , Pose ); }
	virtual void SetGravity( const eg_vec4& GravityVec ){ unused( GravityVec ); }
	virtual void Simulate( eg_real DeltaTime );
	virtual void CreatePhysBody( IEGPhysBodyOwner* PhysBodyOwner , const egPhysBodyDef& PhysBodyDef , const eg_transform& Pose );
	virtual eg_phys_group RaycastClosestShape( const eg_vec4& Origin , const eg_vec4& NormalizedDir , eg_vec4* HitOut , eg_ent_id* EntIdOut );
};
// (c) 2017 Beem Media

#include "EGPhysSim2.h"
#include "EGPhysBody2.h"

EG_CLASS_DECL( EGPhysSim2 )

void EGPhysSim2::InitScene()
{

}

void EGPhysSim2::Simulate( eg_real DeltaTime )
{
	unused( DeltaTime );
}

void EGPhysSim2::CreatePhysBody( IEGPhysBodyOwner* PhysBodyOwner, const egPhysBodyDef& PhysBodyDef, const eg_transform& Pose )
{
	unused( PhysBodyOwner , PhysBodyDef , Pose );
	if( PhysBodyOwner )
	{
		EGPhysBody2* NewPhysBody = EGNewObject<EGPhysBody2>( eg_mem_pool::DefaultHi );
		if( NewPhysBody )
		{
			NewPhysBody->InitFromDef( PhysBodyOwner , PhysBodyDef , Pose );
			NewPhysBody->RefreshFromSimulation();
		}
	}
}

eg_phys_group EGPhysSim2::RaycastClosestShape( const eg_vec4& Origin , const eg_vec4& NormalizedDir , eg_vec4* HitOut , eg_ent_id* EntIdOut )
{
	unused( Origin , NormalizedDir );
	
	if( HitOut )
	{
		*HitOut = eg_vec4( CT_Default );
	}

	if( EntIdOut )
	{
		*EntIdOut = INVALID_ID;
	}
	return eg_phys_group::Unknown;
}

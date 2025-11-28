// (c) 2025 Beem Media. All rights reserved.

#include "EGBpPhysSim.h"
#include "EGGameMap.h"
#include "EGTerrain2.h"
#include "EGMemAlloc.h"
#include "EGBpPhysBody.h"
#include "EGDebugText.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGBpPhysSim )

void EGBpPhysSim::OnConstruct()
{
	Super::OnConstruct();
}

void EGBpPhysSim::InitScene()
{
	EGLogf( eg_log_t::Physics , "BeemPhys Initializing Scene" );
	CreateScene();
}

void EGBpPhysSim::OnDestruct()
{
	ClearScene();
}

void EGBpPhysSim::AddGameMap( const EGGameMap* GameMap )
{	
	unused( GameMap );

	// EX-Release: Game Maps are not supported in the simulation and cannot be queried.
}

void EGBpPhysSim::AddTerrain( const EGTerrain2* Terrain , const eg_transform& Pose )
{
	unused( Terrain , Pose );

	// EX-Release: Terrains are not supported in the simulation and cannot be queried.

}

void EGBpPhysSim::SetGravity( const eg_vec4& GravityVec )
{
	unused( GravityVec );

	// EX-Release: Gravity is not supported in the simulation.
}

void EGBpPhysSim::Simulate( eg_real DeltaTime )
{	
	Super::Simulate( DeltaTime );

	// EX-Release: Our physics sim only does ray tracing no actual simulation.
}

void EGBpPhysSim::CreatePhysBody( IEGPhysBodyOwner* PhysBodyOwner, const egPhysBodyDef& PhysBodyDef, const eg_transform& Pose )
{
	EGPxControlPhysBody::GetStaticClass(); // Force link to EGPxControlPhysBody
	EGPxPhysBody::GetStaticClass(); // Force link to EGPxPhysBody

	EGClass* PhysBodyClass = EGClass::FindClass( PhysBodyDef.ClassName );
	if( !(PhysBodyClass && PhysBodyClass->IsA( &EGBpPhysBody::GetStaticClass() )) )
	{
		PhysBodyClass = &EGBpPhysBody::GetStaticClass();
		// assert( PhysBodyDef.ClassName.Len() == 0 ); // Class not found.
	}

	EGBpPhysBody* NewPhysBody = EGCast<EGBpPhysBody>(EGNewObject( PhysBodyClass , eg_mem_pool::DefaultHi ));
	if( NewPhysBody )
	{
		NewPhysBody->InitBp( this );
		NewPhysBody->InitFromDef( PhysBodyOwner , PhysBodyDef , Pose );
		NewPhysBody->RefreshFromSimulation();
	}
}

eg_phys_group EGBpPhysSim::RaycastClosestShape(const eg_vec4& vOrg, const eg_vec4& vDir, eg_vec4* pvHitOut, eg_ent_id* pnEntOut)
{
	assert(vOrg.w == 1 && vDir.w == 0);

	*pnEntOut = INVALID_ID;

	const eg_vec3 Start = vOrg.XYZ_ToVec3();
	const eg_vec3 Dir = vDir.XYZ_ToVec3();

	EGBpPhysBody* ClosestHit = nullptr;
	eg_real ClosestHitDist = 0.f;
	eg_vec3 CLosestHitPos = CT_Clear;

	eg_vec3 HitPos = CT_Clear;

	// EX-Release: Not the most efficient algorithm since we are just checking
	// every single physics body in the world, but there generally are less than
	// 100 such things and we don't do it that often for the game so this simple
	// algorithm suffices.

	for (auto* Item : m_MasterBodyList)
	{
		if (Item->IsRaycastHitEnabled())
		{
			if (Item->DoesRayIntersect(Start, Dir, &HitPos))
			{
				const eg_real HitDist = (HitPos - Start).Len();

				if (ClosestHit)
				{
					if (HitDist < ClosestHitDist)
					{
						ClosestHit = Item;
						ClosestHitDist = HitDist;
						CLosestHitPos = HitPos;
					}
				}
				else
				{
					ClosestHit = Item;
					ClosestHitDist = HitDist;
					CLosestHitPos = HitPos;

				}
			}
		}
	}

	if (ClosestHit)
	{
		*pnEntOut = ClosestHit->GetEntId();
		*pvHitOut = eg_vec4(CLosestHitPos, 1.f);
		return eg_phys_group::Entity;
	}

	return eg_phys_group::Unknown;
}

void EGBpPhysSim::RegisterBody( EGBpPhysBody& InBody )
{
	assert(!InBody.m_bRegisteredInMasterList);
	InBody.m_MasterListHook = m_MasterBodyList.insert(m_MasterBodyList.end(), &InBody);
	InBody.m_bRegisteredInMasterList = true;
}

void EGBpPhysSim::UnregisterBody( EGBpPhysBody& InBody )
{
	if (InBody.m_bRegisteredInMasterList)
	{
		m_MasterBodyList.erase(InBody.m_MasterListHook);
		InBody.m_bRegisteredInMasterList = false;
	}
}

void EGBpPhysSim::CreateScene()
{
	EGLogf( eg_log_t::Physics , "Resetting scene." );

	ClearScene();
}

void EGBpPhysSim::ClearScene()
{
	// EX-Release: Everything is managed in the EGBpPhysBody and m_MasterList.
}

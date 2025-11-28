// (c) 2017 Beem Media

#include "EGEntWorldTerrain.h"
#include "EGEntWorld.h"
#include "EGGameTerrain2.h"
#include "EGTerrainMesh.h"

EG_CLASS_DECL( EGEntWorldTerrain )

void EGEntWorldTerrain::InitTerrain( eg_cpstr Filename, EGEntWorld* Owner , eg_spawn_reason SpawnReason , const eg_transform& InitPose )
{
	m_Filename = Filename;
	m_OwnerWorld = Owner;
	m_SpawnReason = SpawnReason;
	m_Pose = InitPose;

	if( m_OwnerWorld.IsValid() )
	{
		m_Terrain = new EGGameTerrain2;
		if( m_Terrain )
		{
			m_Terrain->TerrainLoadedDelegate.AddUnique( this , &ThisClass::OnTerrainLoaded );
			m_Terrain->Load( Filename , m_OwnerWorld->GetRole() == eg_ent_world_role::Server );
		}
	}
}

void EGEntWorldTerrain::OnDestruct()
{
	EG_SafeDelete( m_Mesh );

	if( m_Terrain )
	{
		m_Terrain->Unload();
	}
	EG_SafeDelete( m_Terrain );

	Super::OnDestruct();
}

eg_bool EGEntWorldTerrain::IsLoaded() const
{
	return m_Terrain && m_Terrain->IsValid() && m_Terrain->GetLoadState() == LOAD_LOADED;
}

eg_bool EGEntWorldTerrain::IsLoading() const
{
	return m_Terrain && m_Terrain->GetLoadState() == LOAD_LOADING;
}

eg_bool EGEntWorldTerrain::DidLoadFail() const
{
	if( nullptr == m_Terrain )
	{
		return true;
	}

	return m_Terrain->GetLoadState() == LOAD_LOADED_WITH_ERROR;
}

void EGEntWorldTerrain::SetPose( const eg_transform& NewPose )
{
	m_Pose = NewPose;
}

void EGEntWorldTerrain::UpdateLOD( eg_real DeltaTime, const eg_vec4& CameraPos )
{
	if( m_Mesh )
	{
		m_Mesh->UpdateLOD( DeltaTime , CameraPos , GetPose() );
	}
}

void EGEntWorldTerrain::OnTerrainLoaded( EGTerrain2* Terrain )
{
	unused( Terrain );

	assert( m_Terrain == Terrain );

	if( m_OwnerWorld.IsValid() )
	{
		if( m_Terrain && m_Terrain->IsValid() && (m_OwnerWorld->GetRole() == eg_ent_world_role::Client || m_OwnerWorld->GetRole() == eg_ent_world_role::EditorPreview ) )
		{
			m_Mesh = new EGTerrainMesh( m_Terrain );
		}

		m_OwnerWorld->HandleTerrainLoaded( this );
	}
}

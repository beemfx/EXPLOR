// (c) 2017 Beem Media

#include "EGEntWorldMap.h"
#include "EGEntWorld.h"
#include "EGGameMap.h"

EG_CLASS_DECL( EGEntWorldMap )

void EGEntWorldMap::InitMap( eg_cpstr Filename, EGEntWorld* Owner , eg_spawn_reason SpawnReason )
{
	m_Filename = Filename;
	m_OwnerWorld = Owner;
	m_SpawnReason = SpawnReason;

	if( m_OwnerWorld.IsValid() )
	{
		m_Map = new EGGameMap( m_OwnerWorld->GetRole() == eg_ent_world_role::Client || m_OwnerWorld->GetRole() == eg_ent_world_role::EditorPreview );
		if( m_Map )
		{
			m_Map->MapLoadedDelegate.AddUnique( this , &ThisClass::OnMapLoaded );
			m_Map->LoadOnLoadingThread( Filename , m_OwnerWorld->GetRole() == eg_ent_world_role::Server );
		}
	}
}

void EGEntWorldMap::OnDestruct()
{
	EG_SafeDelete( m_Map );

	Super::OnDestruct();
}

eg_bool EGEntWorldMap::IsLoaded() const
{
	return m_Map && m_Map->IsLoaded() && m_Map->GetLoadState() == LOAD_LOADED;
}

eg_bool EGEntWorldMap::IsLoading() const
{
	return m_Map && m_Map->GetLoadState() == LOAD_LOADING;
}

eg_bool EGEntWorldMap::DidLoadFail() const
{
	if( nullptr == m_Map )
	{
		return true;
	}

	return m_Map->GetLoadState() == LOAD_LOADED_WITH_ERROR;
}

void EGEntWorldMap::OnMapLoaded( EGGameMap* GameMap )
{
	unused( GameMap );

	assert( m_Map == GameMap );
	if( m_OwnerWorld.IsValid() )
	{
		m_OwnerWorld->HandleMapLoaded( this );
	}
}

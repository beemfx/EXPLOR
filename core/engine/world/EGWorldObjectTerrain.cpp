// (c) 2018 Beem Media

#include "EGWorldObjectTerrain.h"
#include "EGEntWorld.h"

EG_CLASS_DECL( EGWorldObjectTerrain )

void EGWorldObjectTerrain::ApplyToWorldPreload( EGEntWorld* World ) const
{
	m_LoadedTerrain = World->SpawnTerrain( *m_TerrainFile.FullPath , m_Pose );
}

void EGWorldObjectTerrain::ApplyToWorldPostLoad( EGEntWorld* World ) const
{
	unused( World );

	m_LoadedTerrain = nullptr;
}

eg_bool EGWorldObjectTerrain::IsPreLoadComplete( const EGEntWorld* World ) const
{
	return !World->IsTerrainStillLoading( m_LoadedTerrain );
}

void EGWorldObjectTerrain::AddToWorldPreview( EGEntWorld* World ) const
{
	assert( m_LoadedTerrain == nullptr );
	if( m_TerrainFile.FullPath.Len() > 0 )
	{
		m_LoadedTerrain = World->SpawnTerrain( *m_TerrainFile.FullPath , m_Pose );
	}
}

void EGWorldObjectTerrain::RemoveFromWorldPreview( EGEntWorld* World ) const
{
	if( m_LoadedTerrain )
	{
		World->DestroyTerrain( m_LoadedTerrain );
		m_LoadedTerrain = nullptr;
	}
}

void EGWorldObjectTerrain::RefreshInWorldPreview( EGEntWorld* World, const egRflEditor& ChangedProperty ) const
{
	if( EGString_Equals( ChangedProperty.GetVarName() , "m_TerrainFile" ) )
	{
		RemoveFromWorldPreview( World );
		AddToWorldPreview( World );
	}

	if( m_LoadedTerrain )
	{
		if( EGString_Equals( ChangedProperty.GetVarName() , "m_Pose" ) )
		{
			Super::RefreshInWorldPreview( World , ChangedProperty );
			World->MoveTerrain( m_LoadedTerrain , GetWorldPose() );
		}
	}
}

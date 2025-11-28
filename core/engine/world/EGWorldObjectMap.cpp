// (c) 2018 Beem Media

#include "EGWorldObjectMap.h"
#include "EGEntWorld.h"
#include "EGGameMap.h"

EG_CLASS_DECL( EGWorldObjectMap )

void EGWorldObjectMap::RefreshEditableProperties()
{
	Super::RefreshEditableProperties();

	egRflEditor* PoseEd = m_Editor.GetChildPtr( "m_Pose" );
	if( PoseEd )
	{
		PoseEd->SetEditable( false );
	}
}

void EGWorldObjectMap::ApplyToWorldPreload( EGEntWorld* World ) const
{
	m_LoadedMap = World->SpawnMap( *m_MapFile.FullPath );
}

void EGWorldObjectMap::ApplyToWorldPostLoad(EGEntWorld* World) const 
{
	if( m_bSpawnMapEntities )
	{
		World->SpawnMapEnts( m_LoadedMap );
	}
	m_LoadedMap = nullptr;
}

eg_bool EGWorldObjectMap::IsPreLoadComplete( const EGEntWorld* World ) const
{
	return !World->IsMapStillLoading( m_LoadedMap );
}

void EGWorldObjectMap::AddToWorldPreview( EGEntWorld* World ) const
{
	assert( m_LoadedMap == nullptr );
	if( m_MapFile.FullPath.Len() > 0 )
	{
		m_LoadedMap = World->SpawnMap( *m_MapFile.FullPath );
	}
}

void EGWorldObjectMap::RemoveFromWorldPreview( EGEntWorld* World ) const
{
	if( m_LoadedMap )
	{
		World->DestroyMap( m_LoadedMap );
		m_LoadedMap = nullptr;
	}
}

void EGWorldObjectMap::RefreshInWorldPreview( EGEntWorld* World, const egRflEditor& ChangedProperty ) const
{
	if( EGString_Equals( ChangedProperty.GetVarName() , "m_MapFile" ) )
	{
		RemoveFromWorldPreview( World );
		AddToWorldPreview( World );
	}
}

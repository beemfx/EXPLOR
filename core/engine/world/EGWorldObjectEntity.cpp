// (c) 2018 Beem Media

#include "EGWorldObjectEntity.h"
#include "EGEntWorld.h"
#include "EGEnt.h"
#include "EGEntDict.h"

EG_CLASS_DECL( EGWorldObjectEntity )

void EGWorldObjectEntity::OnDestruct()
{
	Super::OnDestruct();
}

void EGWorldObjectEntity::ApplyToWorldPostLoad( EGEntWorld* World ) const
{
	World->SpawnEnt( eg_string_crc(*m_EntityDefinition.Path) , GetWorldPose() , *m_InitializationString );
}

void EGWorldObjectEntity::AddToWorldPreview( EGEntWorld* World ) const
{
	assert( nullptr == m_SpawnedEnt );
	if( m_EntityDefinition.Path.Len() > 0 )
	{
		m_SpawnedEnt = World->SpawnEnt( eg_string_crc(*m_EntityDefinition.Path) , GetWorldPose() , *m_InitializationString );
	}
}

void EGWorldObjectEntity::RemoveFromWorldPreview( EGEntWorld* World ) const
{
	if( m_SpawnedEnt )
	{
		World->DestroyEnt( m_SpawnedEnt->GetID() );
		m_SpawnedEnt = nullptr;
	}
}

void EGWorldObjectEntity::RefreshInWorldPreview( EGEntWorld* World , const egRflEditor& ChangedProperty ) const
{
	if( m_SpawnedEnt )
	{
		if( EGString_Equals( ChangedProperty.GetVarName() , "m_Pose" ) )
		{
			Super::RefreshInWorldPreview( World , ChangedProperty );
			m_SpawnedEnt->SetPose( GetWorldPose() );
		}
	}

	if( EGString_Equals( ChangedProperty.GetVarName() , "m_EntityDefinition" ) || EGString_Equals( ChangedProperty.GetVarName() , "m_InitializationString" ) )
	{
		RemoveFromWorldPreview( World );
		AddToWorldPreview( World );
	}
}

void EGWorldObjectEntity::ResetPropertiesEditor()
{

}

egRflEditor* EGWorldObjectEntity::GetPropertiesEditor()
{
	return nullptr;
}

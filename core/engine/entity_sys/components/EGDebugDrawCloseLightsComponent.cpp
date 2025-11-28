// (c) 2021 Beem Media. All Rights Reserved.

#include "EGDebugDrawCloseLightsComponent.h"
#include "EGEnt.h"
#include "EGEntWorld.h"
#include "EGDebugShapes.h"

EG_CLASS_DECL( EGDebugDrawCloseLightsComponent )

void EGDebugDrawCloseLightsComponent::OnConstruct()
{
	Super::OnConstruct();
	m_DbLight = EGNewObject<EGDebugSphere>( eg_mem_pool::System );

}

void EGDebugDrawCloseLightsComponent::OnDestruct()
{
	EG_SafeRelease( m_DbLight );
	Super::OnDestruct();
}

void EGDebugDrawCloseLightsComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	if( m_DbLight )
	{
		m_DbLight->Update( DeltaTime );
	}
}

void EGDebugDrawCloseLightsComponent::Draw( const eg_transform& ParentPose ) const
{
	unused( ParentPose );

	if( m_DbLight )
	{
		if( const EGEnt* EntOwner = GetOwner<EGEnt>() )
		{
			if( const EGEntWorld* EntWorld = EntOwner->GetWorld() )
			{
				EntWorld->DrawLights( EntOwner->GetID() , m_DbLight );
			}
		}
	}
}

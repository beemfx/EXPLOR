// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMapObjectComponent.h"
#include "ExGame.h"
#include "ExMapNavBase.h"
#include "EGVisualComponent.h"
#include "EGSettings2Types.h"

EG_CLASS_DECL( ExMapObjectComponent )

static EGSettingsBool ExMapObjectComponent_DebugShowMapObjects( "ExMapObjectComponent.DebugShowMapObjects" , eg_loc("SettingsDebugShowMapObjects","DEBUG: Draw Map Objects") , false , EGS_F_USER_SAVED|EGS_F_DEBUG_EDITABLE );

void ExMapObjectComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	ExEnt* EntOwner = GetOwner<ExEnt>();
	if( EntOwner )
	{
		if( IsClient() && GetEntRole() != eg_ent_role::EditorPreview && !( EX_CHEATS_ENABLED && ExMapObjectComponent_DebugShowMapObjects.GetValue()) )
		{
			EGArray<EGVisualComponent*> Visuals = EntOwner->FindAllComponentsByClass<EGVisualComponent>();
			for( EGVisualComponent* Visual : Visuals )
			{
				Visual->SetHidden( true );
			}
		}

		if( IsServer() )
		{
			m_OwnerGame = EGCast<ExGame>(EntOwner->SDK_GetGame());

			if( ExMapNavBase* MapNav = EGCast<ExMapNavBase>(EntOwner) )
			{
				// Move the ent to the nearest map graph.
				MapNav->Initialize();
			}
		}

		EntOwner->SetActive( false );
	}
	else
	{
		EGLogf( eg_log_t::Warning , "An ExMapObjectComponent (%s) was added to an entity that was not a ExMapNavBase (%s,%s)." , *GetName() , *GetEntOwnerDefId() , *GetOwnerFilename() );
	}
}

void ExMapObjectComponent::OnLeaveWorld()
{
	m_OwnerGame = nullptr;

	Super::OnLeaveWorld();
}

// (c) 2020 Beem Media. All Rights Reserved.

#include "ExNpcComponent.h"

EG_CLASS_DECL( ExNpcComponent )

void ExNpcComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );
	const ExNpcComponent* DefAsNpc = EGCast<ExNpcComponent>(InitData.Def);
	m_ScriptId = DefAsNpc->m_ScriptId;
	m_EncounterPriority = DefAsNpc->m_EncounterPriority;
}

void ExNpcComponent::OnEnterWorld()
{
	Super::OnEnterWorld();
}

void ExNpcComponent::OnLeaveWorld()
{
	Super::OnLeaveWorld();
}

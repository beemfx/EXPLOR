// (c) 2020 Beem Media. All Rights Reserved.

#include "ExScriptExecComponent.h"

EG_CLASS_DECL( ExScriptExecComponent )

void ExScriptExecComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );
	const ExScriptExecComponent* DefAsScriptExec = EGCast<ExScriptExecComponent>(InitData.Def);
	m_bExecInAnyDirection = DefAsScriptExec->m_bExecInAnyDirection;
	m_EncounterPriority = DefAsScriptExec->m_EncounterPriority;
}

void ExScriptExecComponent::OnEnterWorld()
{
	Super::OnEnterWorld();
}

void ExScriptExecComponent::OnLeaveWorld()
{
	Super::OnLeaveWorld();
}

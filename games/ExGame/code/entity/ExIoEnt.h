// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMapNavBase.h"
#include "ExIoEnt.reflection.h"

egreflect class ExIoEnt : public egprop ExMapNavBase
{
	EG_CLASS_BODY( ExIoEnt , ExMapNavBase )
	EG_FRIEND_RFL( ExIoEnt )

protected:

	eg_string_crc m_InitId;
	eg_string_crc m_ScriptId;
	eg_string_crc m_ScriptEntry;
	eg_int m_EncounterPriority;
	eg_bool m_bEncountersEnabled = true;
	eg_bool m_bDespawnQueued = false;
	eg_real m_DespawnCountdown = 0.f;

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual void OnUpdate( eg_real DeltaTime ) override;

	virtual void OnEncounter();
	virtual void HandleNpcDespawn( eg_real Duration , eg_string_crc Event );
	eg_int GetEncounterPriority() const { return m_EncounterPriority; }
	eg_bool AreEncountersEnabled() const { return m_bEncountersEnabled; }

protected:

	virtual void ParseInitString( const eg_d_string& InitString );
};

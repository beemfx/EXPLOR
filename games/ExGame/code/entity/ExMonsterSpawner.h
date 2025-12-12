// (c) 2021 Beem Media. All Rights Reserved.

#pragma once

#include "ExMapNavBase.h"
#include "ExMapObjectComponent.h"
#include "ExMonsterSpawner.reflection.h"

class ExMonsterSpawner : public ExMapNavBase
{
	EG_CLASS_BODY( ExMonsterSpawner , ExEnt )

protected:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
};

egreflect enum class ex_monster_spawner_t
{
	None ,
	Monster ,
	Boss ,
	Custom ,
};

egreflect class ExMonsterSpawnerComponent : public egprop ExMapObjectComponent
{
	EG_CLASS_BODY( ExMonsterSpawnerComponent , ExMapObjectComponent )
	EG_FRIEND_RFL( ExMonsterSpawnerComponent )

protected:

	egprop ex_monster_spawner_t m_SpawnType = ex_monster_spawner_t::None;
	egprop eg_string_crc m_CustomSpawnType = CT_Clear;
	egprop eg_bool m_bMonsterStaysDead = true;

public:
	
	ex_monster_spawner_t GetSpawnType() const { return m_SpawnType; }
	eg_string_crc GetCustomSpawnType() const { return m_CustomSpawnType; }
	eg_bool GetMosnterStaysDead() const { return m_bMonsterStaysDead; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};

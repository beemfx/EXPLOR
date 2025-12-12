// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMapObjectComponent.h"
#include "EGAssetPath.h"
#include "ExNpcSpawnerComponent.reflection.h"

struct egsm_var;

egreflect struct exNpcSpawnerData
{
	egprop eg_string_crc SpawnerEntityInitId = CT_Clear;
	egprop eg_asset_path EntId = eg_asset_path_special_t::EntityDefinition;
	egprop eg_d_string_ml EntInitString = CT_Clear;
	egprop eg_string_crc ControlVariable = CT_Clear;
	egprop eg_ivec2 ExistenceRange = eg_ivec2(0,100);
	egprop eg_real DespawnDuration = 0.f;
	egprop eg_string_crc SpawnEvent = CT_Clear;
	egprop eg_string_crc DespawnEvent = CT_Clear;
	mutable EGWeakPtr<EGEnt> SpawnedEntity;
};

egreflect class ExNpcSpawnerComponent : public egprop ExMapObjectComponent
{
	EG_CLASS_BODY( ExNpcSpawnerComponent , ExMapObjectComponent )
	EG_FRIEND_RFL( ExNpcSpawnerComponent )

protected:

	egprop EGArray<exNpcSpawnerData> m_SpawnerData;
	eg_string_crc m_OwnerInitId = CT_Clear;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }

	void RefreshFromGameState( eg_bool bBecauseVarChanged );
	void HandleGameState( const exNpcSpawnerData& SpawnerData , eg_bool bExists , eg_bool bBecauseVarChanged );

	void OnGameStateVarChanged( eg_string_crc VarId , const egsm_var& NewValue );
};

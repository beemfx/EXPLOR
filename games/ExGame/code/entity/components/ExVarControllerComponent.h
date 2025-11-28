// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "ExVarControllerComponent.reflection.h"

struct egsm_var;

egreflect struct exVarControllerState
{
	egprop eg_int Value = 0;
	egprop eg_string_crc SpawnedWithValueEvent = CT_Clear;
	egprop eg_string_crc ChangedToValueEvent = CT_Clear;
};

egreflect struct exVarControllerData
{
	egprop eg_string_crc ControlVarId = CT_Clear;
	egprop EGArray<exVarControllerState> States;
};

egreflect class ExVarControllerComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExVarControllerComponent , EGComponent )
	EG_FRIEND_RFL( ExVarControllerComponent )

private:

	egprop exVarControllerData m_ControlData;

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;

	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;

	void OnGameVarChanged( eg_string_crc VarId ,const egsm_var& VarValue );
	void HandleVarValue( eg_int VarValue , eg_bool bChangedTo );
};

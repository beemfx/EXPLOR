// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "ExScriptExecComponent.reflection.h"

egreflect class ExScriptExecComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExScriptExecComponent , EGComponent )
	EG_FRIEND_RFL( ExScriptExecComponent )

protected:

	egprop eg_bool m_bExecInAnyDirection = false;
	egprop eg_int m_EncounterPriority = 0;

public:

	eg_bool IsInteractionInAnyDirection() const { return m_bExecInAnyDirection; }
	eg_int GetEncounterPriority() const { return m_EncounterPriority; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};

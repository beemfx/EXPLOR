// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "ExNpcComponent.reflection.h"

egreflect class ExNpcComponent : public egprop EGComponent
{
	EG_CLASS_BODY( ExNpcComponent , EGComponent )
	EG_FRIEND_RFL( ExNpcComponent )

protected:

	egprop eg_d_string m_ScriptId;
	egprop eg_int m_EncounterPriority;

public:

	const eg_d_string& GetScriptIdStr() const { return m_ScriptId; }
	eg_string_crc GetScriptId() const { return eg_string_crc(*m_ScriptId); }
	eg_int GetEncounterPriority() const { return m_EncounterPriority; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual eg_bool CanHaveMoreThanOne() const { return false; }
};

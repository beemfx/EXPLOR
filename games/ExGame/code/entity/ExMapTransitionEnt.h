// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExEnt.h"

class ExPlayerController;

class ExMapTransitionEnt : public ExEnt
{
	EG_CLASS_BODY( ExMapTransitionEnt , ExEnt )

protected:

	eg_string_crc m_TransitionId;
	eg_string_crc m_ScriptId;
	eg_string_crc m_ScriptEntry;

public:

	eg_string_crc GetId() const { return m_TransitionId; }
	eg_string_crc GetScriptId() const { return m_ScriptId; }
	eg_string_crc GetScriptEntry() const { return m_ScriptEntry; }

protected:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;

	void ParseInitString( const eg_d_string& InitString );
};

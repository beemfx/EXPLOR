// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExIoEnt.h"

egreflect class ExGenericIoEnt : public egprop ExIoEnt
{
	EG_CLASS_BODY( ExGenericIoEnt , ExIoEnt )
	EG_FRIEND_RFL( ExGenericIoEnt )

protected:

	eg_string_crc m_ControlVar = CT_Preserve;

public:

	eg_string_crc GetControlVar() const { return m_ControlVar; }

protected:

	virtual void ParseInitString( const eg_d_string& InitString ) override;
};

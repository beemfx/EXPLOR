// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExEnt.h"

class EGMenuStack;

class ExCosmeticEnt : public ExEnt
{
	EG_CLASS_BODY( ExCosmeticEnt , ExEnt )

protected:

	eg_string_crc m_MenuOpenedEvent;
	eg_string_crc m_MenuClosedEvent;
	eg_int        m_MenuCount;

protected:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;

	void ParseInitString( const eg_d_string& InitString );

	void OnMenuStackChanged( EGMenuStack* MenuStack );
	eg_int GetAudioDisablingMenus( EGMenuStack* MenuStack );
};

// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGAmbientSoundComponent.h"
#include "ExAmbientSoundControllerComponent.reflection.h"

class EGMenuStack;

egreflect class ExAmbientSoundControllerComponent : public egprop EGAmbientSoundComponent
{
	EG_CLASS_BODY( ExAmbientSoundControllerComponent , EGAmbientSoundComponent )
	EG_FRIEND_RFL( ExAmbientSoundControllerComponent )

protected:

	egprop eg_bool m_bSetVolumeByLoadingOpacity = false;
	egprop eg_bool m_bDisableForMenus = false;

	eg_int m_MenuCount = 0;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;

	void OnLoadingOpacityChanged( eg_real LoadingOpacity );

	void OnMenuStackChanged( EGMenuStack* MenuStack );
	eg_int GetAudioDisablingMenus( EGMenuStack* MenuStack );
};

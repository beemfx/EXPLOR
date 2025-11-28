// (c) 2017 Beem Media

#pragma once

#include "EGComponent.h"
#include "EGAudioTypes.h"
#include "EGAssetPath.h"
#include "EGSoundPlayerComponent.reflection.h"

class EGEnt;

egreflect struct egSoundPlayerSound
{
	egprop eg_string_crc Id       = CT_Clear;
	egprop eg_asset_path Filename = eg_asset_path_special_t::Sound;
};

egreflect class EGSoundPlayerComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGSoundPlayerComponent , EGComponent )
	EG_FRIEND_RFL( EGSoundPlayerComponent )

protected:

	struct egNamedSound
	{
		eg_string_crc Id;
		egs_sound     Sound;
	};

protected:

	egprop EGArray<egSoundPlayerSound> m_Sounds;

protected:

	EGEnt*                 m_EntOwner;
	EGArray<egNamedSound>  m_CreatedSounds;
	EGArray<eg_string_crc> m_QueuedSounds;
	EGArray<eg_string_crc> m_QueuedStopSounds;

public:

	void QueueSound( eg_string_crc Id );
	void QueueStopSound( eg_string_crc Id );

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnDestruct() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;

	virtual void RefreshEditableProperties();

	void PlaySound( eg_string_crc Name , const eg_transform& WorldPose ) const;
	void StopSound( eg_string_crc Name ) const;
};

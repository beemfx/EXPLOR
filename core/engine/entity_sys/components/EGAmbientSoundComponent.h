// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGComponent.h"
#include "EGAudioTypes.h"
#include "EGAssetPath.h"
#include "EGItemMap.h"
#include "EGWeakPtr.h"
#include "EGAmbientSoundComponent.reflection.h"

class EGAudioList;

egreflect class EGAmbientSoundComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGAmbientSoundComponent , EGComponent )
	EG_FRIEND_RFL( EGAmbientSoundComponent )

protected:

	egprop eg_asset_path m_SoundEffect = eg_asset_path_special_t::Sound;
	egprop eg_bool       m_bPlayingByDefault = false;

protected:

	EGEnt*                 m_EntOwner;
	egs_sound              m_CreatedSound = egs_sound::Null;
	eg_bool                m_bIsPlaying = false;
	eg_real                m_Volume = 1.f;

public:

	void SetIsPlaying( eg_bool bNewValue );
	eg_bool IsPlaying() const { return m_bIsPlaying; }
	egs_sound GetSound() const { return m_CreatedSound; }
	eg_vec3 GetSoundPos() const;
	void SetVolume( eg_real NewValue ) { m_Volume = NewValue; }
	eg_real GetVolume() const { return m_Volume; }

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnDestruct() override;
	virtual void OnEnterWorld() override;
	virtual void OnLeaveWorld() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;
};

class EGAmbientSoundManager : public EGObject
{
	EG_CLASS_BODY( EGAmbientSoundManager , EGObject )

private:

	static EGAmbientSoundManager* s_Inst;

private:

	struct egSoundData
	{
		EGArray<EGWeakPtr<EGAmbientSoundComponent>> SoundComps;
	};

private:

	EGItemMap<egs_sound,egSoundData> m_SoundMap = egSoundData();
	
public:

	virtual ~EGAmbientSoundManager();

	static EGAmbientSoundManager* Create();
	static EGAmbientSoundManager* Get();

	void HandleCreateAmbience( EGAmbientSoundComponent* SoundComp );
	void HandleDestroyAmbience( EGAmbientSoundComponent* SoundComp );

	void Update( eg_real DeltaTime , EGAudioList* AudioList , const eg_transform& CameraPose );
};

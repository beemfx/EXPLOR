// (c) 2017 Beem Media

#pragma once

#include "EGAudioDevice.h"
#include "EGAudioList.h"

class EGNoAudioDevice : public EGAudioDevice
{
	EG_CLASS_BODY( EGNoAudioDevice , EGAudioDevice )

private:

	EGNoAudioDevice();
	~EGNoAudioDevice();

	// BEGIN EGAudioDevice
	virtual egs_sound CreateSound( eg_cpstr SoundId ) override;
	virtual void DestroySound( egs_sound Sound ) override;
	virtual EGAudioList* BeginFrame_MainThread() override;
	virtual void EndFrame_MainThread( EGAudioList* Al ) override;
	virtual void AddCB( ISoundCB* Cb ) override;
	virtual void RemoveCB( ISoundCB* Cb ) override;
	virtual void ProcessCBs() override;
	virtual void QueryAllSounds( EGArray<eg_string_small>& SoundsOut ) override;
	// END EGAudioDevice

private:

	EGAudioList m_AlList;
	void*       m_AlMem;
	eg_size_t   m_AlMemSize;
};

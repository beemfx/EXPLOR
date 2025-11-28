// (c) 2017 Beem Media

#pragma once

#include "EGAudioTypes.h"

class EGAudioList;

class EGAudioDevice : public EGObject
{
	EG_ABSTRACT_CLASS_BODY( EGAudioDevice , EGObject )

public:

	virtual egs_sound CreateSound( eg_cpstr SoundId ) = 0;
	virtual void DestroySound(  egs_sound Sound ) = 0;

	virtual EGAudioList* BeginFrame_MainThread() = 0;
	virtual void EndFrame_MainThread( EGAudioList* Al ) = 0;

	virtual void AddCB( ISoundCB* Cb ) = 0;
	virtual void RemoveCB( ISoundCB* Cb ) = 0;
	virtual void ProcessCBs() = 0;
	virtual void QueryAllSounds( EGArray<eg_string_small>& SoundsOut ) = 0;
};

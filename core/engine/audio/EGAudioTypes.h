// (c) 2015 Beem Media

#pragma once

enum class egs_sound : eg_uint32 { Null };

struct egAudioListenerInfo
{
	eg_vec4 vPos;
	eg_vec4 vVel;
	eg_vec4 vAt;
	eg_vec4 vUp;
};


enum class eg_sound_event_t
{
	PLAY,
	COMPLETE,
	STOP,
};

struct egSoundEvent
{
	eg_sound_event_t Event;
	egs_sound        Sound;
};

class ISoundCB
{
public:
	virtual void OnSoundEvent( const egSoundEvent& Info )=0;
};

static const eg_uint UPDATE_3D_OP_SET  = 1;
static const eg_uint PLAY_SOUND_OP_SET = 2;
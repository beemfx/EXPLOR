// (c) 2016 Beem Media
#pragma once

#include "EGAudioTypes.h"

class EGSoundScape;

class EGSoundTrack : public EGObject , public IListable
{
	EG_CLASS_BODY( EGSoundTrack , EGObject )

private:

	enum class eg_fade_t
	{
		NONE,
		FADE_IN,
		FADE_OUT,
	};

private:

	egs_sound     m_Sound;
	EGSoundScape* m_Owner;
	eg_string_crc m_Id;
	eg_flags      m_PlaySoundFlags;
	eg_fade_t     m_FadeType;
	eg_real       m_FadeDuration;
	eg_real       m_FadeTime;

public:
	
	EG_DECLARE_FLAG( F_LOOPING  , 1 );
	EG_DECLARE_FLAG( F_BG_MUSIC , 2 );

	void Init( EGSoundScape* Owner, eg_cpstr Filename );
	void Deinit();
	void Play( eg_flags PlayFlags );
	void ResetTrack();
	void FadeIn( eg_real Duration , eg_flags PlayFlags );
	void FadeToFullVolume();
	void FadeOut( eg_real Duration );
	void Update( eg_real DeltaTime );
	void OnSoundEvent( eg_sound_event_t Event );
	eg_bool IsBgMusic() const { return m_PlaySoundFlags.IsSet( F_BG_MUSIC ); }
	eg_string_crc GetId() const { return m_Id; }
	static eg_string_crc FilenameToId( eg_cpstr Filename );

	egs_sound GetSound() const { return m_Sound; }
};

// (c) 2016 Beem Media
#include "EGSoundTrack.h"
#include "EGAudio.h"
#include "EGAudioList.h"
#include "EGSoundScape.h"

EG_CLASS_DECL( EGSoundTrack )

void EGSoundTrack::Init( EGSoundScape* Owner, eg_cpstr Filename )
{
	assert( nullptr == m_Owner );
	m_Owner = Owner;
	m_Sound = EGAudio_CreateSound( Filename );
	m_Id = FilenameToId( Filename );
	if( m_Sound == egs_sound::Null )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": No such sound %s." , Filename );
	}
}

void EGSoundTrack::Deinit()
{
	if( egs_sound::Null != m_Sound )
	{
		EGAudio_DestroySound( m_Sound );
		m_Sound = egs_sound::Null;
	}
}

void EGSoundTrack::Play( eg_flags PlayFlags )
{
	m_FadeType = eg_fade_t::NONE;

	if( egs_sound::Null != m_Sound )
	{
		m_PlaySoundFlags = PlayFlags;
		MainAudioList->PlaySound( m_Sound );
	}
}

void EGSoundTrack::ResetTrack()
{
	MainAudioList->StopSound( m_Sound );
	MainAudioList->PlaySound( m_Sound );
}

void EGSoundTrack::FadeIn( eg_real Duration, eg_flags PlayFlags )
{
	m_FadeTime = 0;
	m_FadeDuration = Duration;
	m_FadeType = eg_fade_t::FADE_IN;
	m_PlaySoundFlags = PlayFlags;
	if( egs_sound::Null != m_Sound && MainAudioList )
	{
		MainAudioList->SetVolume( m_Sound , 0.f ); // Set volume first to avoid pop
		MainAudioList->PlaySound( m_Sound );
	}
}

void EGSoundTrack::FadeToFullVolume()
{
	if( m_FadeType == eg_fade_t::FADE_IN || m_FadeType == eg_fade_t::NONE )
	{
		return;
	}

	m_FadeTime = m_FadeDuration - m_FadeTime;
	assert( m_FadeTime > 0 );
	m_FadeType = eg_fade_t::FADE_IN;
}

void EGSoundTrack::FadeOut( eg_real Duration )
{
	m_FadeTime = 0;
	m_FadeDuration = Duration;
	m_FadeType = eg_fade_t::FADE_OUT;
}

void EGSoundTrack::Update( eg_real DeltaTime )
{
	if( m_Sound == egs_sound::Null )
	{
		return;
	}

	auto GetFadePct = [this]( eg_real From , eg_real To ) -> eg_real
	{
		eg_real FadePct = EGMath_CubicInterp( 0.f, 0.f, 1.f, 0.f, EGMath_GetMappedRangeValue( m_FadeTime , eg_vec2( 0.f, m_FadeDuration ), eg_vec2( From, To ) ) );
		FadePct = EG_Clamp( FadePct , 0.f , 1.f );
		return FadePct;
	};

	switch( m_FadeType )
	{
		case eg_fade_t::NONE:
		{
		} break;
		case eg_fade_t::FADE_IN:
		{
			eg_real fVolume = GetFadePct( 0.f , 1.f );
			MainAudioList->SetVolume( m_Sound , fVolume );
			m_FadeTime += DeltaTime;
			if( m_FadeTime >= m_FadeDuration )
			{
				MainAudioList->SetVolume( m_Sound , 1.f );
				m_FadeType = eg_fade_t::NONE;
			}
		} break;
		case eg_fade_t::FADE_OUT:
		{
			eg_real fVolume = GetFadePct( 1.f, 0.f );
			MainAudioList->SetVolume( m_Sound, fVolume );
			m_FadeTime += DeltaTime;
			if( m_FadeTime >= m_FadeDuration )
			{
				MainAudioList->SetVolume( m_Sound , 0.f );
				m_FadeType = eg_fade_t::NONE;
				m_Owner->DestroyTrack( this );
			}
		} break;
		
	}
}

void EGSoundTrack::OnSoundEvent( eg_sound_event_t Event )
{
	switch( Event )
	{
		case eg_sound_event_t::PLAY:
		{
		} break;
		case eg_sound_event_t::COMPLETE:
		{
			if( m_PlaySoundFlags.IsSet( F_LOOPING ) )
			{
				Play( m_PlaySoundFlags );
			}
			else
			{
				m_Owner->DestroyTrack( this );
			}
		} break;
		case eg_sound_event_t::STOP:
		{
		} break;
	}
}

eg_string_crc EGSoundTrack::FilenameToId( eg_cpstr Filename )
{
	eg_string AsString = Filename;
	AsString.ConvertToLower();
	return eg_string_crc(AsString);
}

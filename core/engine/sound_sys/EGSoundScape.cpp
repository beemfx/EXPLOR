#include "EGSoundScape.h"
#include "EGAudio.h"
#include "EGAudioList.h"

EG_CLASS_DECL( EGSoundScape )

void EGSoundScape::Init()
{
	assert( 0 == m_Tracks.Len() );

	EGAudio_AddCB( this );
}

void EGSoundScape::Deinit()
{
	EGAudio_RemoveCB( this );
	ClearTracks();
	CullDeltedTracks();
}

void EGSoundScape::ClearTracks()
{
	m_BgMusicNextTrack.Clear();
	for( EGSoundTrack* Track : m_Tracks )
	{
		DestroyTrack( Track );
	}
}

void EGSoundScape::StartBgMusic( eg_cpstr Filename )
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Clear();

	for( EGSoundTrack* Track : m_Tracks )
	{
		if( Track->IsBgMusic() )
		{
			DestroyTrack( Track );
		}
	}

	EGSoundTrack* NewSoundTrack = CreateNewTrack( Filename );
	if( NewSoundTrack )
	{
		NewSoundTrack->Play( EGSoundTrack::F_LOOPING|EGSoundTrack::F_BG_MUSIC );
	}
}

void EGSoundScape::FadeToBgMusic( eg_cpstr Filename, eg_real FadeInTime, eg_real FadeOutTime )
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Clear();
	m_BgMusicStack.Push( Filename );

	FadeToBgMusicInternal( Filename , FadeInTime , FadeOutTime );
}

void EGSoundScape::FadeAllBgMusic( eg_real FadeOutTime )
{
	m_BgMusicNextTrack.Clear();

	for( EGSoundTrack* Track : m_Tracks )
	{
		if( Track->IsBgMusic() )
		{
			Track->FadeOut( FadeOutTime );
		}
	}
}

void EGSoundScape::Stop()
{
	m_BgMusicNextTrack.Clear();
	ClearTracks();
}

void EGSoundScape::ClearBgMusicStack()
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Clear();
	ClearTracks();
}

void EGSoundScape::DestroyTrack( EGSoundTrack* Track )
{
	// We'll actually cull these later.
	assert( Track->GetListId() == m_Tracks.GetId() );
	m_TrackCullList.AppendUnique( Track );
}

void EGSoundScape::Update( eg_real DeltaTime )
{
	// DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format("EGSoundScape: Tracks %u Culls %u" , m_Tracks.Len() , m_TrackCullList.Len() ) );
	
	for( EGSoundTrack* Track : m_Tracks )
	{
		Track->Update( DeltaTime );
	}

	CullDeltedTracks();
}

void EGSoundScape::PushBgMusic( eg_cpstr Filename , eg_real FadeInTime , eg_real FadeOutTime )
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Push( Filename );
	FadeToBgMusicInternal( m_BgMusicStack.Top() , FadeInTime , FadeOutTime );
}

void EGSoundScape::SwitchBgMusic( eg_cpstr Filename , eg_real FadeInTime , eg_real FadeOutTime )
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Pop();
	m_BgMusicStack.Push( Filename );
	FadeToBgMusicInternal( m_BgMusicStack.Top() , FadeInTime , FadeOutTime );
}

void EGSoundScape::SetNextBgMusicTrack( eg_cpstr Filename )
{
	m_BgMusicNextTrack = Filename;
}

void EGSoundScape::PopBgMusic( eg_real FadeInTime , eg_real FadeOutTime )
{
	m_BgMusicNextTrack.Clear();
	m_BgMusicStack.Pop();
	if( m_BgMusicStack.HasItems() )
	{
		FadeToBgMusicInternal( m_BgMusicStack.Top() , FadeInTime , FadeOutTime );
	}
	else
	{
		FadeAllBgMusic( FadeOutTime );
	}
}

void EGSoundScape::ResetBgMusicTrack()
{
	if( m_BgMusicStack.HasItems() )
	{
		EGSoundTrack* PlayingTrack = FindTrackById(EGSoundTrack::FilenameToId(m_BgMusicStack.Top()));
		if( PlayingTrack )
		{
			// TODO: Reseting music tracks is causing audio popping so the resume ends up being more tolerable.
			// PlayingTrack->ResetTrack();
		}
	}
}

void EGSoundScape::FadeToBgMusicInternal( eg_cpstr Filename , eg_real FadeInTime , eg_real FadeOutTime )
{
	EGSoundTrack* PlayingTrack = FindTrackById( EGSoundTrack::FilenameToId( Filename ) );
	if( PlayingTrack )
	{
		EGLogf( eg_log_t::Audio , "%s was already playing, fading to full volume..." , Filename );

		PlayingTrack->FadeToFullVolume();
	}

	for( EGSoundTrack* Track : m_Tracks )
	{
		if( Track->IsBgMusic() && Track != PlayingTrack )
		{
			Track->FadeOut( FadeOutTime );
		}
	}

	if( nullptr == PlayingTrack && Filename && EGString_StrLen( Filename ) > 0 )
	{
		PlayingTrack = CreateNewTrack( Filename );
		if( PlayingTrack )
		{
			PlayingTrack->FadeIn( FadeInTime , EGSoundTrack::F_LOOPING|EGSoundTrack::F_BG_MUSIC );
		}
	}
}

EGSoundTrack* EGSoundScape::CreateNewTrack( eg_cpstr Filename )
{
	EGSoundTrack* NewSoundTrack = EGNewObject<EGSoundTrack>( eg_mem_pool::Audio );
	if( NewSoundTrack )
	{
		m_Tracks.Insert( NewSoundTrack );
		NewSoundTrack->Init( this, Filename );
	}
	else
	{
		assert( false );
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": Couldn't create track, out of memory?" );
	}
	return NewSoundTrack;
}

EGSoundTrack* EGSoundScape::FindTrackById( eg_string_crc Id )
{
	for( EGSoundTrack* Track : m_Tracks )
	{
		if( Track->GetId() == Id )
		{
			return Track;
		}
	}

	return nullptr;
}

void EGSoundScape::CullDeltedTracks()
{
	for( EGSoundTrack* Track : m_TrackCullList )
	{
		m_Tracks.Remove( Track );
		Track->Deinit();
		EGDeleteObject( Track );
	}

	m_TrackCullList.Clear();
}

void EGSoundScape::OnSoundEvent( const egSoundEvent& Info )
{
	for( EGSoundTrack* Track : m_Tracks )
	{
		if( Track->GetSound() == Info.Sound )
		{
			if( Info.Event == eg_sound_event_t::COMPLETE && m_BgMusicNextTrack.Len() > 0 )
			{
				DestroyTrack( Track );
				FadeToBgMusicInternal( m_BgMusicNextTrack , 0.f , 0.f );
				m_BgMusicNextTrack.Clear();
			}
			else
			{
				Track->OnSoundEvent( Info.Event );
			}
		}
	}
}
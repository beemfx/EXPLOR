/******************************************************************************
	EGSoundScape - A class for managing scripted audio sequences.

	(c) 2015 Beem Software
******************************************************************************/
#pragma once

#include "EGAudioTypes.h"
#include "EGSoundTrack.h"

class EGSoundScape : public EGObject , public ISoundCB
{
	EG_CLASS_BODY( EGSoundScape , EGObject )

private:
	
	typedef EGList<EGSoundTrack>   EGTrackList;
	typedef EGArray<EGSoundTrack*> EGCullList;

public:
	EGSoundScape()
	: m_Tracks( EGTrackList::DEFAULT_ID )
	, m_TrackCullList()
	{ 
 
	}

	void Init();
	void Deinit();

	void Update( eg_real DeltaTime );

	void PushBgMusic( eg_cpstr Filename , eg_real FadeInTime = .5f , eg_real FadeOutTime = 2.f );
	void SwitchBgMusic( eg_cpstr Filename ,  eg_real FadeInTime = .5f , eg_real FadeOutTime = 2.f );
	void SetNextBgMusicTrack( eg_cpstr Filename );
	void PopBgMusic( eg_real FadeInTime = .5f , eg_real FadeOutTime = 2.f );
	void ResetBgMusicTrack();

	void StartBgMusic( eg_cpstr Filename ); // Stops all tracks, starts a new one.
	void FadeToBgMusic( eg_cpstr Filename , eg_real FadeInTime = .5f , eg_real FadeOutTime = 2.f ); // Fades all running tracks, fades in the new one.
	void FadeAllBgMusic( eg_real FadeOutTime = 2.f );
	void Stop();
	void ClearBgMusicStack();

	void DestroyTrack( EGSoundTrack* Track ); // Meant to be called from the tracks themselves so they can delete themselves.

private:

	EGTrackList        m_Tracks;
	EGCullList         m_TrackCullList;
	EGArray<eg_string> m_BgMusicStack;
	eg_string          m_BgMusicNextTrack;

protected:
	
	void ClearTracks();

private:
	
	void FadeToBgMusicInternal( eg_cpstr Filename , eg_real FadeInTime, eg_real FadeOutTime ); // Fades all running tracks, fades in the new one.

	EGSoundTrack* CreateNewTrack( eg_cpstr Filename );
	EGSoundTrack* FindTrackById( eg_string_crc Id );
	void CullDeltedTracks();
	virtual void OnSoundEvent( const egSoundEvent& Info ) override;
};
// (c) 2017 Beem Media

#include "EGSoundPlayerComponent.h"
#include "EGAudio.h"
#include "EGAudioList.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGSoundPlayerComponent )

void EGSoundPlayerComponent::QueueSound( eg_string_crc Id )
{
	m_QueuedSounds.Append( Id );
}

void EGSoundPlayerComponent::QueueStopSound( eg_string_crc Id )
{
	m_QueuedStopSounds.Append( Id );
}

void EGSoundPlayerComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	m_EntOwner = EGCast<EGEnt>( InitData.Owner );

	const EGSoundPlayerComponent* AsSoundPlayerDef = EGCast<EGSoundPlayerComponent>( InitData.Def );

	if( InitData.bIsClient )
	{
		if( AsSoundPlayerDef )
		{
			for( const egSoundPlayerSound& SoundInfo : AsSoundPlayerDef->m_Sounds )
			{
				egNamedSound NewSound;
				NewSound.Id = SoundInfo.Id;
				NewSound.Sound = EGAudio_CreateSound( *SoundInfo.Filename.Path );
				m_CreatedSounds.Append( NewSound );
			}
		}
	}
}

void EGSoundPlayerComponent::OnDestruct()
{
	for( egNamedSound& Sound : m_CreatedSounds )
	{
		EGAudio_DestroySound( Sound.Sound );
	}
	m_Sounds.Clear();

	Super::OnDestruct();
}

void EGSoundPlayerComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	for( eg_string_crc QueuedSound : m_QueuedSounds )
	{
		if( !IsAudioMuted() )
		{
			eg_transform SoundPos( CT_Default );
			if( m_EntOwner )
			{
				SoundPos *= m_EntOwner->GetPose();
			}
			PlaySound( QueuedSound, SoundPos );
		}
	}
	m_QueuedSounds.Clear();

	for( eg_string_crc QueuedSound : m_QueuedStopSounds )
	{
		StopSound( QueuedSound );
	}
	m_QueuedStopSounds.Clear();
}

void EGSoundPlayerComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("PlaySound"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				if( !IsAudioMuted() )
				{
					QueueSound( Action.StrCrcParm(0) );
				}
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for PlaySound." );
			}
		} break;
		case_crc("StopSound"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				QueueStopSound( Action.StrCrcParm(0) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for PlaySound." );
			}
		} break;
		default:
		{
			EGLogf( eg_log_t::Error , "EGEntSoundPlayerComponent: Invalid script \"%s\"." , Action.FnCall.FunctionName );
		} break;
	}
}

void EGSoundPlayerComponent::RefreshEditableProperties()
{
	Super::RefreshEditableProperties();

	SetPropEditable( "m_ScaleVector" , false );
}

void EGSoundPlayerComponent::PlaySound( eg_string_crc Name , const eg_transform& WorldPose ) const
{
	for( const egNamedSound& Sound : m_CreatedSounds )
	{
		if( Sound.Id == Name )
		{
			eg_vec4 Pos = GetWorldPose( WorldPose ).GetPosition();
			MainAudioList->PlaySoundAt( Sound.Sound, &Pos );
			return;
		}
	}
}

void EGSoundPlayerComponent::StopSound( eg_string_crc Name ) const
{
	for( const egNamedSound& Sound : m_CreatedSounds )
	{
		if( Sound.Id == Name )
		{
			MainAudioList->StopSound( Sound.Sound );
			return;
		}
	}
}

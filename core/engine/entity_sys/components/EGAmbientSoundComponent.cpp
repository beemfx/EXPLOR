// (c) 2020 Beem Media. All Rights Reserved.

#include "EGAmbientSoundComponent.h"
#include "EGEnt.h"
#include "EGAudio.h"
#include "EGAudioList.h"
#include "EGWorkerThreads.h"

EG_CLASS_DECL( EGAmbientSoundComponent )

void EGAmbientSoundComponent::SetIsPlaying( eg_bool bNewValue )
{
	assert( IsClient() ); // Only has an effect on client.
	if( IsClient() )
	{
		m_bIsPlaying = bNewValue;
	}
}

eg_vec3 EGAmbientSoundComponent::GetSoundPos() const
{
	return m_EntOwner ? GetWorldPose( m_EntOwner->GetPose() ).GetTranslation() : CT_Clear;
}

void EGAmbientSoundComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	m_EntOwner = EGCast<EGEnt>( InitData.Owner );

	const EGAmbientSoundComponent* AsAmbientSoundDef = EGCast<EGAmbientSoundComponent>( InitData.Def );

	if( IsClient() )
	{
		if( AsAmbientSoundDef )
		{
			m_SoundEffect = AsAmbientSoundDef->m_SoundEffect;
			m_bPlayingByDefault = AsAmbientSoundDef->m_bPlayingByDefault;

			m_CreatedSound = EGAudio_CreateSound( *m_SoundEffect.Path );
		}
	}
}

void EGAmbientSoundComponent::OnDestruct()
{
	if( IsClient() )
	{
		EGAudio_DestroySound( m_CreatedSound );
	}

	Super::OnDestruct();
}

void EGAmbientSoundComponent::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsClient() )
	{
		SetIsPlaying( m_bPlayingByDefault );

		if( EGAmbientSoundManager* AmbMgr = EGAmbientSoundManager::Get() )
		{
			AmbMgr->HandleCreateAmbience( this );
		}
	}
}

void EGAmbientSoundComponent::OnLeaveWorld()
{
	if( IsClient() )
	{
		if( EGAmbientSoundManager* AmbMgr = EGAmbientSoundManager::Get() )
		{
			AmbMgr->HandleDestroyAmbience( this );
		}
	}

	Super::OnLeaveWorld();
}

void EGAmbientSoundComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );
}

void EGAmbientSoundComponent::ScriptExec( const struct egTimelineAction& Action )
{
	assert( IsClient() ); // Only has effect on client.

	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("SetIsEnabled"):
		case_crc("SetIsPlaying"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				SetIsPlaying( Action.BoolParm( 0 ) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetIsPlaying." );
			}
		} break;
		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

EG_CLASS_DECL( EGAmbientSoundManager )

EGAmbientSoundManager* EGAmbientSoundManager::s_Inst = nullptr;

EGAmbientSoundManager::~EGAmbientSoundManager()
{
	assert( this == s_Inst );
	s_Inst = nullptr;
}

EGAmbientSoundManager* EGAmbientSoundManager::Create()
{
	assert( EGWorkerThreads_IsThisMainThread() );

	assert( nullptr == s_Inst );

	if( nullptr == s_Inst )
	{
		s_Inst = EGNewObject<EGAmbientSoundManager>();
	}

	return s_Inst;
}

EGAmbientSoundManager* EGAmbientSoundManager::Get()
{
	assert( EGWorkerThreads_IsThisMainThread() );

	return s_Inst;
}

void EGAmbientSoundManager::HandleCreateAmbience( EGAmbientSoundComponent* SoundComp )
{
	assert( EGWorkerThreads_IsThisMainThread() );

	if( SoundComp && SoundComp->GetSound() != egs_sound::Null )
	{
		if( m_SoundMap.Contains( SoundComp->GetSound() ) )
		{
			egSoundData& Data = m_SoundMap[SoundComp->GetSound()];
			Data.SoundComps.Append( SoundComp );
		}
		else
		{
			egSoundData NewSoundData;
			NewSoundData.SoundComps.Append( SoundComp );
			m_SoundMap.Insert( SoundComp->GetSound() , NewSoundData );
		}
	}
}

void EGAmbientSoundManager::HandleDestroyAmbience( EGAmbientSoundComponent* SoundComp )
{
	assert( EGWorkerThreads_IsThisMainThread() );

	if( SoundComp && SoundComp->GetSound() != egs_sound::Null )
	{
		if( m_SoundMap.Contains( SoundComp->GetSound() ) )
		{
			egSoundData& Data = m_SoundMap[SoundComp->GetSound()];
			Data.SoundComps.DeleteByItem( SoundComp );
			if( Data.SoundComps.IsEmpty() )
			{
				m_SoundMap.Delete( SoundComp->GetSound() );
			}
		}
		else
		{
			assert( false ); // Bad flow?
		}
	}
}

void EGAmbientSoundManager::Update( eg_real DeltaTime , EGAudioList* AudioList , const eg_transform& CameraPose )
{
	unused( DeltaTime );
	
	assert( EGWorkerThreads_IsThisMainThread() );

	if( AudioList )
	{
		const eg_vec3 CamPos = CameraPose.GetTranslation();

		// For each sound effect, find the active emitter that is
		// closest to the camera and use it. Ideally we'd want more than
		// one instance at a time, for example an emitter on left and 
		// right side, but EXPLOR doesn't feature any situation like that
		// so for now we call this good.

		for( eg_size_t MapIdx = 0; MapIdx < m_SoundMap.Len(); MapIdx++ )
		{
			const egs_sound& Sound = m_SoundMap.GetKeyByIndex( MapIdx );
			egSoundData& Data = m_SoundMap.GetByIndex( MapIdx );

			EGAmbientSoundComponent* Closest = nullptr;
			eg_real ClosestDistSq = 0.f;
			eg_vec3 ClosestPos = CT_Clear;
			eg_real ClosestVolume = 1.f;
			for( EGWeakPtr<EGAmbientSoundComponent>& Comp : Data.SoundComps )
			{
				if( Comp && Comp->IsPlaying() )
				{
					const eg_vec3 Pos = Comp->GetSoundPos();
					const eg_real DistSq = (Pos - CamPos).LenSq();
					if( nullptr == Closest || DistSq < ClosestDistSq  )
					{
						Closest = Comp.GetObject();
						ClosestDistSq = DistSq;
						ClosestPos = Pos;
						ClosestVolume = Comp->GetVolume();
					}
				}
			}

			if( nullptr != Closest )
			{
				AudioList->SetVolume( Sound , ClosestVolume );
			}
			AudioList->UpdateAmbientSound( Sound , ClosestPos , nullptr != Closest );
		}
	}
}

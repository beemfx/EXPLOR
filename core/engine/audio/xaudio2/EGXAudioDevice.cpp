// (c) 2015 Beem Media

#include "EGXSound.h"
#include "EGXAudioDevice.h"
#include "EGAudioFile.h"
#include "EGTimer.h"
#include "EGAudioFileMgr.h"
#include "EGEngineConfig.h"
#include "EGEngine.h"
#include "EGXAudioDeviceEnumerator.h"
#include "EGSettings2Types.h"

EG_CLASS_DECL( EGXAudioDevice )

static EGSettingsInt EGXAudioDevice_AudioChannels( "EGXAudioDevice.AudioChannels" , eg_loc("EGXAudioDeviceChannels","Audio Channels") , 0 , EGS_F_SYS_SAVED );

/********************************
*** Creation and destruction. ***
********************************/
EGXAudioDevice::EGXAudioDevice()
: m_pAudio(nullptr)
, m_pMasterVoice(nullptr)
, m_Sounds()
, m_nLastUpdated(egs_sound::Null)
, m_AudioThread( EGThread::egInitParms( "XAudio" , EGThread::eg_init_t::GamePriority ) )
{
	zero(&m_X3DAudio);
	{
		HRESULT nRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	}

	InitAudioDevice();

	m_DeviceEnumerator = new EGXAudioDeviceEnumerator( this );
	
	m_AudioFileMgr = new ( eg_mem_pool::Audio ) EGAudioFileMgr;
	if( nullptr == m_AudioFileMgr )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to create sound manager." );
		DeinitAudioDevice();
		return;
	}
	m_AudioFileMgr->Init();

	EGXSound::InitMgr( m_AudioFileMgr );

	for( eg_uint i=0; i<countof(m_AlData); i++ )
	{
		m_AlData[i].Mem = EGMem2_Alloc( AUDIO_LIST_BUFFER_SIZE , eg_mem_pool::System );
		m_AlData[i].MemSize = AUDIO_LIST_BUFFER_SIZE;
	}

	//The pre-update and unplayed lists are always initialized (except breifly when they are purged).
	m_AlData[static_cast<eg_size_t>(AL_T::PRE_UPDATE)].InitList( 0 );
	m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].InitList( 0 );

	//We create the update thread:
	m_AudioThread.RegisterProc( this );
	m_AudioThread.Start();
}

EGXAudioDevice::~EGXAudioDevice()
{
	{
		EGFunctionLock FnLockData( &m_LockData );

		PurgeDeletedSounds();
	}
	
	//Stop the update thread:
	m_AudioThread.Stop();
	m_AudioThread.UnregisterProc( this );

	m_AlData[static_cast<eg_size_t>(AL_T::PRE_UPDATE)].DeinitList();
	m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].DeinitList();

	for( eg_uint i=0; i<countof(m_AlData); i++ )
	{
		if( m_AlData[i].Mem )
		{
			EGMem2_Free( m_AlData[i].Mem );
		}
		m_AlData[i].Mem = nullptr;
		m_AlData[i].MemSize = 0;
	}

	assert( !m_CBs.HasItems() ); //Not all callbacks were unregistered.

	m_AudioFileMgr->Deinit();
	EG_SafeDelete( m_AudioFileMgr );
	for( EGXSound*& Sound : m_Sounds )
	{
		assert( nullptr == Sound ); // Should have been released previously.
		EG_SafeRelease( Sound );
	}
	m_Sounds.Clear();

	DeinitAudioDevice();

	if( m_DeviceEnumerator )
	{
		delete m_DeviceEnumerator;
		m_DeviceEnumerator = nullptr;
	}

	CoUninitialize();

	EGLogf(eg_log_t::Audio , "XAduio2 device was shut down." );
}

void EGXAudioDevice::OnThreadMsg(eg_cpstr sMsg)
{
	unused( sMsg );
}

void EGXAudioDevice::OnProcessingPassStart()
{

}

void EGXAudioDevice::OnProcessingPassEnd()
{

}

void EGXAudioDevice::OnCriticalError( HRESULT Error )
{
	EGFunctionLock Lock( &m_LockData );

	m_CriticalError = Error;

	EGLogf( eg_log_t::Error , "XAudio2 had a critical error." );
}

void EGXAudioDevice::InitAudioDevice()
{
	HRESULT nRes = 0;

	EGLogf( eg_log_t::Audio , "Initializing XAudio2 device." );

	UINT32 flags = 0;

	nRes = ::XAudio2Create(&m_pAudio, flags);

	if(FAILED(nRes))
	{
		::EGLogf( eg_log_t::Error , "Failed to create XAudio2 Interface.");
		m_pAudio = nullptr;
		CoUninitialize();
		m_CriticalError = E_FAIL;
		return;
	}

	m_pAudio->RegisterForCallbacks( this );

	/*
	//Let us enumerate the devices
	UINT32 nDevices = 0;
	nRes = m_pAudio->GetDeviceCount(&nDevices);
	::EGLogf( eg_log_t::Audio , "   %u audio devices available.", nDevices);
	for(UINT32 i=0; i<nDevices; i++)
	{
		XAUDIO2_DEVICE_DETAILS dd;
		zero(&dd);
		m_pAudio->GetDeviceDetails(i, &dd);
		::EGLogf( eg_log_t::Audio , "   Device %u:", i);
		::EGLogf( eg_log_t::Audio , "   Name: %s", eg_string(dd.DisplayName).String());
		::EGLogf( eg_log_t::Audio , "   ID:   %s", eg_string(dd.DeviceID).String());
	}

	const eg_uint nDeviceID = 0;
	m_pAudio->GetDeviceDetails(nDeviceID, &m_dd);
	*/

	::EGLogf( eg_log_t::Audio , "Creating mastering voice...");
	UINT32 Flags = 0;
	// Flags = XAUDIO2_NO_VIRTUAL_AUDIO_CLIENT;
	const UINT32 NumChannels = EGXAudioDevice_AudioChannels.GetValueThreadSafe(); // XAUDIO2_DEFAULT_CHANNELS;
	nRes = m_pAudio->CreateMasteringVoice(	&m_pMasterVoice , NumChannels , XAUDIO2_DEFAULT_SAMPLERATE , Flags , NULL , NULL , AudioCategory_GameEffects );

	if(FAILED(nRes))
	{
		::EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to create mastering voice...");
		::EG_SafeRelease(m_pAudio);
		CoUninitialize();
		m_CriticalError = E_FAIL;
		return;
	}

	m_pMasterVoice->GetVoiceDetails(&m_mvd);;

	::EGLogf( eg_log_t::Audio , "Mastering voice created at 0x%08X with %u channels.", reinterpret_cast<eg_uintptr_t>(m_pMasterVoice), m_mvd.InputChannels);

	::EGLogf( eg_log_t::Audio , "Initializing XAudio3D...");
	DWORD dwChannelMask = 0;  
	if( NumChannels == 0 )
	{
		m_pMasterVoice->GetChannelMask( &dwChannelMask );
	}
	else
	{
		dwChannelMask = SPEAKER_STEREO;
	}
	::X3DAudioInitialize( dwChannelMask , X3DAUDIO_SPEED_OF_SOUND , m_X3DAudio );

	//Set the basic properties for the listener and emitter.
	//Listener:
	m_L.Position    = CreateVector(0,0,0);
	m_L.OrientFront = CreateVector(0,0,1);
	m_L.OrientTop   = CreateVector(0,1,0);
	m_L.Velocity    = CreateVector(0,0,0);
	m_L.pCone       = nullptr;

	EGXSound::s_EGXAudio = this;
	EGXSound::s_pAudio = this->m_pAudio;
}

void EGXAudioDevice::DeinitAudioDevice()
{
	EGXSound::s_pAudio = nullptr;
	EGXSound::s_EGXAudio = nullptr;

	if( m_pMasterVoice )
	{
		m_pMasterVoice->DestroyVoice();
		m_pMasterVoice = nullptr;
	}

	if( m_pAudio )
	{
		m_pAudio->UnregisterForCallbacks( this );
		ULONG n = m_pAudio->Release();
		::EGLogf( eg_log_t::Audio , "Released XAudio2 Interface at 0x%08X with %u references remaining.", reinterpret_cast<eg_uintptr_t>(m_pAudio), n);
		m_pAudio = nullptr;
	}
}

void EGXAudioDevice::TryToRestartAudio()
{
	// MessageBoxW( NULL, L"XAudio2 Failed, attempting to restart it." , *Engine_GetGameTitle() , MB_OK );
	if( m_bAudioDeviceChanged )
	{
		EGLogf( eg_log_t::Error , "Restarting XAudio2 due to device change." );
	}
	
	if( FAILED( m_CriticalError ) )
	{
		EGLogf( eg_log_t::Error , "XAudio2 Failed, attempting to restart it..." );
	}
	
	for( EGXSound* Sound : m_Sounds )
	{
		if( Sound )
		{
			Sound->HandleAudioFailedPreReboot();
		}
	}

	DeinitAudioDevice();
	
	InitAudioDevice();

	if( m_pAudio )
	{
		m_CriticalError = S_OK;
		m_bAudioDeviceChanged = false;

		for( EGXSound* Sound : m_Sounds )
		{
			if( Sound )
			{
				Sound->HandleAudioFailedPostReboot();
			}
		}
	}
}

void EGXAudioDevice::Update( eg_real DeltaTime )
{
	EGFunctionLock FnLockData( &m_LockData );

	if( FAILED( m_CriticalError ) || m_bAudioDeviceChanged )
	{
		TryToRestartAudio();
		return;
	}

	//Update every sound every frame...?
	for( EGXSound* Sound : m_Sounds )
	{
		if( Sound )
		{
			Sound->Update( m_mvd.InputChannels , m_X3DAudio , m_L );
		}
	}

	m_pAudio->CommitChanges( PLAY_SOUND_OP_SET );
	m_pAudio->CommitChanges( UPDATE_3D_OP_SET );
	m_AudioFileMgr->Update( DeltaTime );

	//Copy the pre-update and unplayed audio lists into the update list.
	egAlData* AlUpdate    = &m_AlData[static_cast<eg_size_t>(AL_T::UPDATE)];
	egAlData* AlPreUpdate = &m_AlData[static_cast<eg_size_t>(AL_T::PRE_UPDATE)];
	egAlData* AlUnplayed  = &m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)];

	AlUpdate->InitList( 0 );

	//copy in unplayed list
	AlUpdate->List.Append( &AlUnplayed->List );
	//Reset the unplayed list.
	AlUnplayed->DeinitList();
	AlUnplayed->InitList( 0 );

	//Copy in pre-update list
	m_AlLock.Lock(); //We only need to lock the copy of the pre-update into the update as only pre-update is modified on the main thread.
	AlUpdate->List.Append( &AlPreUpdate->List );	
	//Reset the pre-update list.
	AlPreUpdate->DeinitList();
	AlPreUpdate->InitList( 0 );
	m_AlLock.Unlock();

	ProcessAudioList( &AlUpdate->List );

	AlUpdate->DeinitList();

	PurgeDeletedSounds();
}

void EGXAudioDevice::ProcessAudioList( EGAudioList* Al )
{
	for( EGAudioList::egCmd* Cmd : Al->List )
	{
		switch( Cmd->Cmd )
		{
			case eg_audio_list_cmd::SET_LISTENER:
			{
				egAudioListenerInfo Info;
				Cmd->Data.SetListener.vAt.CopyTo( &Info.vAt );
				Cmd->Data.SetListener.vPos.CopyTo( &Info.vPos );;
				Cmd->Data.SetListener.vUp.CopyTo( &Info.vUp );
				Cmd->Data.SetListener.vVel.CopyTo( &Info.vVel );
				SetListener( &Info );
			} break;

			case eg_audio_list_cmd::STOP_SOUND:
			{
				StopSound( Cmd->Data.StopSound.Sound );
			} break;

			case eg_audio_list_cmd::PLAY_SOUND_3D:
			{
				eg_vec4 Position;
				Cmd->Data.PlaySound3D.Position.CopyTo( &Position );
				PlaySoundAt( Cmd->Data.PlaySound3D.Sound , &Position );
			} break;

			case eg_audio_list_cmd::UPDATE_AMBIENT_SOUND:
			{
				eg_vec4 Position;
				Cmd->Data.PlaySound3D.Position.CopyTo( &Position );
				const eg_bool bIsPlaying = Position.w > EG_SMALL_NUMBER;
				UpdateAmbientSound( Cmd->Data.PlaySound3D.Sound , Position.XYZ_ToVec3() , bIsPlaying );
			} break;

			case eg_audio_list_cmd::PLAY_SOUND:
			{
				PlaySound( Cmd->Data.PlaySound.Sound );
			} break;

			case eg_audio_list_cmd::SET_VOLUME:
			{
				SetVolume( Cmd->Data.SetVolume.Sound , Cmd->Data.SetVolume.Volume );
			} break;
		}
	}
}

EGAudioList* EGXAudioDevice::BeginFrame_MainThread()
{
	EGFunctionLock Lock( &m_AlLock );

	egAlData* Data = &m_AlData[static_cast<eg_size_t>(AL_T::MAIN_THREAD)];
	assert( 0 == Data->List.List.Len() );

	Data->List.InitAudioList( Data->Mem , Data->MemSize , 0 );
	return &Data->List;
}

void EGXAudioDevice::EndFrame_MainThread( EGAudioList* Al )
{
	EGFunctionLock Lock( &m_AlLock );

	//Copy the main thread audio list to the pre-update list.
	assert( Al == &m_AlData[static_cast<eg_size_t>(AL_T::MAIN_THREAD)].List );
	EGAudioList* To   = &m_AlData[static_cast<eg_size_t>(AL_T::PRE_UPDATE)].List;
	To->Append( Al );
	Al->DeinitAudioList();
}

egs_sound EGXAudioDevice::CreateSound( eg_cpstr SoundId )
{
	EGFunctionLock Lock( &m_LockData );

	eg_string SoundIdStr( SoundId );
	SoundIdStr.ConvertToLower();
	eg_string_crc SoundIdAsCrc( SoundIdStr );

	for( EGXSound* Sound : m_Sounds )
	{
		if( Sound && Sound->GetUniqueId() == SoundIdAsCrc )
		{
			Sound->AddRef();
			return Sound->GetHandle();
		}
	}

	EGXSound* NewSound = EGNewObject<EGXSound>( eg_mem_pool::Audio );
	if( NewSound )
	{
		NewSound->InitSound( SoundId , SoundIdAsCrc );

		// Find an empty slot or add a new one
		for( eg_size_t i=0; i<m_Sounds.Len(); i++ )
		{
			if( m_Sounds[i] == nullptr )
			{
				m_Sounds[i] = NewSound;
				NewSound->SetHandle( static_cast<egs_sound>(i + 1) );
				return static_cast<egs_sound>( i + 1 );
			}
		}

		// If we're here there was no slot so
		m_Sounds.Append( NewSound );
		egs_sound SoundHandle = static_cast<egs_sound>( m_Sounds.Len() );
		NewSound->SetHandle( SoundHandle );
		return SoundHandle;
	}

	return egs_sound::Null;
}

void EGXAudioDevice::DestroySound( egs_sound Sound )
{
	EGFunctionLock Lock( &m_DeleteListLock );
	m_DeleteSoundList.Append( Sound );
}

void EGXAudioDevice::PlaySound(egs_sound sound)
{
	EGFunctionLock Lock( &m_LockData );

	EGXSound* pSound = IdToSound( sound );
	if(pSound)
	{
		//If the sound is ready play it, if not queue it in the unplayed list.
		if( pSound->IsReady() ) 
		{
			pSound->Play();
			egSoundEvent Event = { eg_sound_event_t::PLAY , sound };
			QueueSoundEvent( Event );
		}
		else
		{
			m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].List.PlaySound( sound );
		}
	}
}

void EGXAudioDevice::PlaySoundAt(egs_sound sound, const eg_vec4* pvPos)
{
	EGXSound* pSound = IdToSound( sound );
	if(pSound)
	{
		if( pSound->IsReady() )
		{
			if(pSound->m_SoundDef->bCentered )
			{
				PlaySound( sound );
				return;
			}
			//If the sound is too far away we won't play it.
			eg_vec4 v3L;
			eg_vec4 vListenerPos(m_L.Position.x, m_L.Position.y, m_L.Position.z, 1);
			v3L = *pvPos - vListenerPos;
			eg_real DistSq = v3L.LenSqAsVec3(); //Distance of sound from listener.
			eg_real SoundDistSq = pSound->GetRange()*pSound->GetRange();
			if( DistSq < 1 || (DistSq<=SoundDistSq) )
			{
				pSound->SetPosition(pvPos);
				pSound->Play();
			}
		}
		else
		{
			m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].List.PlaySoundAt( sound , pvPos );
		}
	}
}

void EGXAudioDevice::UpdateAmbientSound( egs_sound sound , const eg_vec3& Pos , eg_bool bIsPlaying )
{
	EGXSound* pSound = IdToSound( sound );
	if(pSound)
	{
		if( pSound->IsReady() && pSound->m_SoundDef->Type == eg_sound_class_t::Ambience )
		{
			//If the sound is too far away we won't play it.
			const eg_vec3 vListenerPos(m_L.Position.x, m_L.Position.y, m_L.Position.z);
			const eg_real DistSq = (vListenerPos - Pos).LenSq();
			const eg_real SoundDistSq = pSound->GetRange()*pSound->GetRange();
			if( (DistSq <= SoundDistSq || pSound->m_SoundDef->bCentered) && bIsPlaying )
			{
				if( !pSound->m_IsPlaying )
				{
					pSound->Play();
				}

				if( !pSound->m_SoundDef->bCentered )
				{
					pSound->SetPosition(&eg_vec4(Pos,1.f));
				}
			}
			else
			{
				if( pSound->m_IsPlaying )
				{
					pSound->Stop();
				}
			}
		}
		else
		{
			assert( pSound->m_SoundDef->Type == eg_sound_class_t::Ambience ); // Tried to play non ambient sound as ambience.
			// Called every frame so we don't need this anyway.
			// m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].List.UpdateAmbientSound( sound , Pos );
		}
	}
}

void EGXAudioDevice::StopSound(egs_sound sound)
{
	EGXSound* pSound = IdToSound( sound );
	if(pSound)
	{
		//If the sound is ready play it, if not que it in the unplayed list.
		if( pSound->IsReady() ) 
		{
			pSound->Stop();
		}
		else
		{
			//It might seem like it's possibly that a Play could have been queued
			//then the audio engine processed the load, so Play ended up queued
			//but Stop got called, and so the Play gets called afterwards, but
			//note that completed loads are processed in in the sound manager
			//update and the audio list is processed afterwards so if a sound had
			//both a Play and Stop queued they would always get processed on the
			//same frame in the correct order.
			m_AlData[static_cast<eg_size_t>(AL_T::UNPLAYED)].List.StopSound( sound );
		}
	}
}

void EGXAudioDevice::SetVolume( egs_sound Sound, eg_real Volume )
{
	EGXSound* pSound = IdToSound( Sound );
	if( pSound )
	{
		if( pSound->IsReady() )
		{
			pSound->SetVolume( Volume );
		}
		else
		{
			// Process this later if the sound isn't available yet.
			m_AlData[static_cast<eg_size_t>( AL_T::UNPLAYED )].List.SetVolume( Sound , Volume );
		}
	}
}

void EGXAudioDevice::SetListener(const egAudioListenerInfo* pLI)
{
	assert(pLI->vPos.w == 1 && pLI->vAt.w == 0 && pLI->vUp.w == 0 && pLI->vVel.w == 0);
	//We use Lock(LISTENER_LOCK) so that when we update the listener
	//it will only stall if we are currently reading the listener
	m_L.Position    = CreateVector(pLI->vPos.x, pLI->vPos.y, pLI->vPos.z);
	m_L.OrientFront = CreateVector(pLI->vAt.x, pLI->vAt.y, pLI->vAt.z);
	m_L.OrientTop   = CreateVector(pLI->vUp.x, pLI->vUp.y, pLI->vUp.z);
	m_L.Velocity    = CreateVector(pLI->vVel.x, pLI->vVel.y, pLI->vVel.z);
	m_L.pCone       = nullptr;
}


void EGXAudioDevice::PurgeDeletedSounds()
{
	EGFunctionLock FnDeleteListLock( &m_DeleteListLock );

	for( egs_sound SoundToDelete : m_DeleteSoundList )
	{
		eg_size_t SoundIndex = static_cast<eg_size_t>( SoundToDelete ) - 1;
		if( m_Sounds.IsValidIndex( SoundIndex ) )
		{
			if( m_Sounds[SoundIndex] )
			{
				eg_int NumLeft = m_Sounds[SoundIndex]->Release();
				if( 0 == NumLeft )
				{
					m_Sounds[SoundIndex] = nullptr;
				}
			}
		}
	}
	m_DeleteSoundList.Clear();
}

EGXSound* EGXAudioDevice::IdToSound( egs_sound Sound )
{
	eg_size_t SoundIndex = static_cast<eg_size_t>(Sound)-1;
	if( m_Sounds.IsValidIndex( SoundIndex ) )
	{
		return m_Sounds[SoundIndex];
	}

	return nullptr;
}

void EGXAudioDevice::AddCB( ISoundCB* Cb )
{
	EGFunctionLock Lock( &m_LockCb );

	//Make sure the cb isn't already in the list.
	for( eg_uint i=0; i<m_CBs.Len(); i++ )
	{
		if( Cb == m_CBs[i] )
		{
			assert( false ); //Callback already registered.
			return;
		}
	}

	m_CBs.Push( Cb );
}

void EGXAudioDevice::RemoveCB( ISoundCB* Cb )
{
	EGFunctionLock Lock( &m_LockCb );

	//Make sure the cb isn't already in the list.
	for( eg_uint i=0; i<m_CBs.Len(); i++ )
	{
		if( Cb == m_CBs[i] )
		{
			m_CBs.DeleteByIndex(i);
			return;
		}
	}
	assert( false ); //Callback was not registered.
	return;
}

void EGXAudioDevice::ProcessCBs()
{
	EGFunctionLock Lock( &m_LockCb );

	for( eg_uint InfoId=0; InfoId<m_CBInfos.Len(); InfoId++ )
	{
		const egSoundEvent& Info = m_CBInfos[InfoId];

		for( eg_uint j=0; j<m_CBs.Len(); j++ )
		{ 
			m_CBs[j]->OnSoundEvent( Info ); 
		}

	}

	m_CBInfos.Clear();
}

void EGXAudioDevice::QueryAllSounds( EGArray<eg_string_small>& SoundsOut )
{
	EGFunctionLock Lock( &m_LockData );
	m_AudioFileMgr->QueryAllSounds( SoundsOut );
}

void EGXAudioDevice::HandleDefaultDeviceChanged()
{
	EGFunctionLock Lock( &m_LockData );

	m_bAudioDeviceChanged = true;
}

void EGXAudioDevice::QueueSoundEvent( const egSoundEvent& Event )
{
	EGFunctionLock Lock( &m_LockCb );
	m_CBInfos.Push( Event );
}
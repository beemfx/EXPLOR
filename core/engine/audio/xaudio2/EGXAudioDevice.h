// (c) 2015 Beem Software

#pragma once

#include "EGAudioDevice.h"
#include "EGThread.h"
#include "EGAudioList.h"
#include <XAudio2.h>
#include <X3DAudio.h>

class EGXSoundMgr;
class EGXAudioDeviceEnumerator;

class EGXAudioDevice : public EGAudioDevice , private IXAudio2EngineCallback , private IThreadProc
{
	EG_CLASS_BODY( EGXAudioDevice , EGAudioDevice )

public:

	static const X3DAUDIO_VECTOR CreateVector( eg_real x , eg_real y , eg_real z )
	{
		X3DAUDIO_VECTOR Out = { x , y , z };
		return Out;
	}

	static const X3DAUDIO_VECTOR CreateVector( const eg_vec3& Vec )
	{
		X3DAUDIO_VECTOR Out = { Vec.x , Vec.y , Vec.z };
		return Out;
	}
	
public:

	EGXAudioDevice();
	~EGXAudioDevice();

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

	void HandleDefaultDeviceChanged();

public:

	void QueueSoundEvent( const egSoundEvent& Event );

private:

	struct egAlData //Audio List Data
	{
		EGAudioList List;
		void*       Mem;
		eg_size_t   MemSize;

		egAlData(): List(), Mem(nullptr), MemSize(0){ }

		void InitList( eg_uint AssetState )
		{
			assert( nullptr != Mem );
			assert( 0 != MemSize );
			List.InitAudioList( Mem , MemSize , AssetState );
		}

		void DeinitList()
		{
			List.DeinitAudioList();
		}
	};

	enum class AL_T
	{
		MAIN_THREAD,
		PRE_UPDATE,
		UPDATE,
		UNPLAYED,

		COUNT,
	};

	static const eg_size_t AUDIO_LIST_BUFFER_SIZE = 1*1024*1024; //May need this bigger, may need it bigger only for everything but main thread.

private:

	void InitAudioDevice();
	void DeinitAudioDevice();
	void TryToRestartAudio();

	// BEGIN IThreadProc
	virtual void Update( eg_real DeltaTime );
	virtual void OnThreadMsg(eg_cpstr sMsg);
	// END IThreadProc

	// BEGIN IXAudio2EngineCallback
	virtual void OnProcessingPassStart() override;
	virtual void OnProcessingPassEnd() override;
	virtual void OnCriticalError( HRESULT Error ) override;
	// END IXAudio2EngineCallback

	void ProcessAudioList( EGAudioList* Al );

	void PlaySound(egs_sound sound);
	void PlaySoundAt(egs_sound sound, const eg_vec4* pv3Pos);
	void UpdateAmbientSound(egs_sound sound, const eg_vec3& Pos , eg_bool bIsPlaying );
	void StopSound(egs_sound sound);
	void SetVolume( egs_sound Sound , eg_real Volume );
	void SetListener(const egAudioListenerInfo* pLisInfo);

	void PurgeDeletedSounds();

	class EGXSound* IdToSound( egs_sound Sound );

private:

	EGThread  m_AudioThread;
	IXAudio2* m_pAudio;
	IXAudio2MasteringVoice* m_pMasterVoice;
	// XAUDIO2_DEVICE_DETAILS m_dd;
	XAUDIO2_VOICE_DETAILS  m_mvd; //Mastering voice details.

	X3DAUDIO_HANDLE       m_X3DAudio;
	X3DAUDIO_DSP_SETTINGS m_DSP;
	X3DAUDIO_EMITTER      m_Emitter;
	X3DAUDIO_LISTENER     m_L;

	EGArray<EGXSound*>    m_Sounds;
	class EGAudioFileMgr* m_AudioFileMgr;
	egs_sound             m_nLastUpdated;

	EGFixedArray<ISoundCB*,5> m_CBs;
	EGArray<egSoundEvent>     m_CBInfos;

	EGArray<egs_sound> m_DeleteSoundList;
	HRESULT m_CriticalError = S_OK;
	eg_bool m_bAudioDeviceChanged = false;

	EGXAudioDeviceEnumerator* m_DeviceEnumerator = nullptr;

	mutable EGMutex m_LockData;
	mutable EGMutex m_LockCb;
	mutable EGMutex m_AssetStateLock;
	mutable EGMutex m_DeleteListLock;

	/***************************************************************************
	AUDIO LIST DATA

	The Audio List is based off the display list idea, but executed quite a bit
	differently. The way it works is that every frame the main thread creates
	an audio list for that frame. That is submitted to the audio library. The
	audio library then stores than in a pre-update buffer that is built up over
	time. In an update loop the the audio library copies the pre-update buffer
	into the update buffer (under a lock). The update loop then processes the
	update buffer, and once the loops goes around again it checks the pre-update
	buffer for more commands. There is also an un-played buffer which retains
	any commands that couldn't executed due to the fact that the data didn't
	exist yet. This is also copied into the update buffer every frame.
	***************************************************************************/
	egAlData  m_AlData[static_cast<eg_size_t>(AL_T::COUNT)];
	EGMutex   m_AlLock;      //Lock for moving around audio lists.
};

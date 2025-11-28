/******************************************************************************
	EGXSound - An instance of a sound in the Xaudio2 engine.

	(c) 2015 Beem Software
******************************************************************************/
#pragma once

#include "EGAudioTypes.h"
#include <XAudio2.h>
#include <X3DAudio.h>
#include "EGSoundFile.h"

class EGAudioFile;

class EGXSound : public EGObject
{
	EG_CLASS_BODY( EGXSound , EGObject )

public:

	static void InitMgr( class EGAudioFileMgr* SoundMgr ){ s_AudioFileMgr = SoundMgr; }

public:

	EGXSound();
	void InitSound( eg_cpstr SoundId , eg_string_crc CrcIdentifier );
	void SetHandle( egs_sound SoundHandle ){ m_SoundHandle = SoundHandle; }
	eg_string_crc GetUniqueId() const { return m_UniqueId; }
	egs_sound GetHandle() const { return m_SoundHandle; }
	virtual ~EGXSound() override;

	void HandleAudioFailedPreReboot();
	void HandleAudioFailedPostReboot();

public:

	//Audio control functionality, should work according to specifications of the
	//data acquired from the load.
	void Play();
	void Stop();
	void SetVolume(eg_real fVol);
	void SetPosition(const eg_vec4* vPos);
	eg_real GetRange()const{ return m_SoundDef ? m_SoundDef->Range : 0.f; }
	eg_real GetRangeFalloffPct() { return m_SoundDef ? m_SoundDef->RangeFalloffPct : 0.f; }
	//Update: This is called every few frames.
	void Update(const eg_uint nC,X3DAUDIO_HANDLE& H,X3DAUDIO_LISTENER& L);
	eg_bool IsReady(); //Not const because this will actually make the sound ready if it's not.

	static const eg_uint MAX_CHANNELS = 9;

private:

	enum FINALIZE_S
	{
		NOT_LOADED,
		LOADING,
		READY,
		FAILED,
	};

private:

	//PostLoad: Called after Load has acquired all the data from the XML file,
	//should load the file according to the specifications.
	void PostLoad( class EGAudioFileMgr* SndMgr );
	void Unload();

	FINALIZE_S EnsureSoundIsFinalized();

	eg_real GetVolumeForType() const;

private:

	const egSoundDef*    m_SoundDef;
	eg_string_crc        m_UniqueId;
	egs_sound            m_SoundHandle;
	EGAudioFileData*     m_RawSoundBuffer;
	eg_uint              m_nNextInst; //The next voice to get played next time an effect is played.
	IXAudio2SourceVoice* m_apV[egSoundDef::MAX_INSTANCES];
	eg_vec4              m_avVPos[egSoundDef::MAX_INSTANCES];

	//Data for streaming file:
	EGAudioFile* m_pStreamFile;

	eg_uint     m_nBufferSize;
	BYTE*       m_pBuffer1; //The sound buffer.
	//BYTE*       m_pBuffer2; //2nd sound buffer (for streaming only).

	static const eg_uint NUM_STREAM_BUFFERS = 3;
	BYTE* m_apStBufs[NUM_STREAM_BUFFERS];
	eg_uint m_nNextBuffer; //Indicates the next buffer that should be played.
	eg_real m_Volume;
	eg_real m_SoundTypeVolume;

	//::IXAudio2SourceVoice* m_pVoice; //voice.
	WAVEFORMATEX m_wf;
	XAUDIO2_BUFFER m_bf;

	eg_bool    m_bUpdateStream:1; //true if the stream should be updated.
	eg_bool    m_IsStream:1;
	eg_bool    m_IsPaused:1;
	eg_bool    m_IsPlaying:1;
	eg_bool    m_IsFinalized:1;
	eg_bool    m_bPreResetWasPlaying:1;
	eg_bool    m_bShouldPlayOnLoad:1;

private:

	eg_bool LoadEffect( void* Data , eg_size_t DataSize , eg_uint nInstances);
	eg_bool LoadStream( void* Data , eg_size_t DataSize );

	EG_INLINE void Play_Effect();
	EG_INLINE void Play_Stream();

	EG_INLINE void Update_3D(const eg_uint nC, X3DAUDIO_HANDLE& H, X3DAUDIO_LISTENER& L);

	EG_INLINE void Update_Stream();
	EG_INLINE eg_bool Update_Buffer();

	void* AllocBuffer( eg_size_t Size );
	void  FreeBuffer( void* Buffer );

	eg_real GetRawVolumeInternal() const;

private:

	friend class EGXAudioDevice;
	static IXAudio2* s_pAudio;
	static class EGXAudioDevice* s_EGXAudio;

private:

	class EGVoiceStreamCB: public IXAudio2VoiceCallback
	{
		virtual void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired);
		virtual void __stdcall OnVoiceProcessingPassEnd();
		virtual void __stdcall OnStreamEnd();
		virtual void __stdcall OnBufferStart(void* pBufferContext);
		virtual void __stdcall OnBufferEnd(void* pBufferContext);
		virtual void __stdcall OnLoopEnd(void* pBufferContext);
		virtual void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error);
	};

	static EGVoiceStreamCB s_VoiceStreamCB;
	static EGAudioFileMgr*     s_AudioFileMgr;
};

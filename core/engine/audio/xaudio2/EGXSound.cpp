// (c) 2017 Beem Media

#include "EGXSound.h"
#include "EGAudioFileMgr.h"
#include "EGXAudioDevice.h"
#include "EGAudioFile.h"
#include "EGAudioTypes.h"
#include "EGDebugText.h"
#include "EGGlobalConfig.h"
#include "EGSettings2Types.h"

EG_CLASS_DECL( EGXSound )

static EGSettingsBool EGXSound_Show3DMatrix( "EGXSound.Show3DMatrix" , eg_loc("EGXSoundShow3DMatrix","DEBUG: Show Sound Matrix") , false , EGS_F_DEBUG_EDITABLE|EGS_F_SYS_SAVED );

IXAudio2* EGXSound::s_pAudio = nullptr;
EGXAudioDevice* EGXSound::s_EGXAudio = nullptr;

EGXSound::EGVoiceStreamCB EGXSound::s_VoiceStreamCB;
EGAudioFileMgr* EGXSound::s_AudioFileMgr = nullptr;

EGXSound::EGXSound()
: m_pStreamFile(nullptr)
, m_nNextBuffer(0)
, m_nBufferSize(0)
, m_pBuffer1(nullptr)
, m_nNextInst(0)
, m_bUpdateStream(false)
, m_IsStream( false )
, m_IsPaused( false )
, m_IsPlaying( false )
, m_bPreResetWasPlaying( false )
, m_bShouldPlayOnLoad( false )
, m_RawSoundBuffer( nullptr )
, m_IsFinalized( false )
, m_SoundDef( nullptr )
, m_Volume( 1.f )
, m_SoundTypeVolume( 1.f )
{
	zero(&m_wf);
	zero(&m_bf);
	for(eg_uint i=0; i<NUM_STREAM_BUFFERS; i++)
	{
		m_apStBufs[i] = 0;
	}

	for(eg_uint i=0; i<egSoundDef::MAX_INSTANCES; i++)
	{
		m_apV[i] = nullptr;
	}

	zero(&m_avVPos);

	
}

void EGXSound::InitSound( eg_cpstr SoundId , eg_string_crc CrcIdentifier )
{
	m_UniqueId = CrcIdentifier;
	m_SoundDef = s_AudioFileMgr->GetSoundDef( SoundId );
	assert( nullptr != m_SoundDef ); //Even if the file doesn't exist the manager should still be returning a default.
	PostLoad( s_AudioFileMgr );
}

EGXSound::~EGXSound()
{
	Unload();
}

void EGXSound::HandleAudioFailedPreReboot()
{
	m_bPreResetWasPlaying = m_IsPlaying;
	Unload();
}

void EGXSound::HandleAudioFailedPostReboot()
{
	assert( nullptr != m_SoundDef ); //Even if the file doesn't exist the manager should still be returning a default.
	PostLoad( s_AudioFileMgr );

	if( m_bPreResetWasPlaying )
	{
		Play();
	}
}

void EGXSound::PostLoad( class EGAudioFileMgr* SndMgr )
{
	eg_bool bRes=false;

	m_nNextInst = 0;

	m_RawSoundBuffer = SndMgr->GetAudioFile( m_SoundDef ? m_SoundDef->SoundFile : "" );
	if( nullptr != m_RawSoundBuffer )
	{
		m_RawSoundBuffer->AddRef();
	}
}

void EGXSound::Unload()
{
	if(m_IsFinalized)
	{
		Stop();
		for(eg_uint i=0; i<m_SoundDef->Instances; i++)
		{
			m_apV[i]->DestroyVoice();
		}
		if(m_pBuffer1)
		{
			FreeBuffer(m_pBuffer1);
			m_pBuffer1 = nullptr;
		}

		for(eg_uint i=0; i<NUM_STREAM_BUFFERS; i++)
		{
			if(m_apStBufs[i])
			{
				FreeBuffer(m_apStBufs[i]);
				m_apStBufs[i] = nullptr;
			}
		}

		//Close streaming file in case we are streaming, if not this doesn't hurt.
		EGAudioFile::CloseAF(m_pStreamFile);
		m_pStreamFile=nullptr;

		m_nNextInst=0;
	}

	EG_SafeRelease( m_RawSoundBuffer );
	m_IsFinalized = false;
}

EGXSound::FINALIZE_S EGXSound::EnsureSoundIsFinalized()
{
	FINALIZE_S State = NOT_LOADED;

	if( m_IsFinalized )
	{
		//If we are initialzied it means we're good to go.
		State = READY;
	}
	else if( nullptr == m_RawSoundBuffer || nullptr == m_SoundDef )
	{
		//If the raw data doesn't even exist, we failed.
		State = FAILED;
	}
	else
	{
		//Finally, if the data is done loading, make it ready.
		eg_bool bRes=false;

		switch( m_RawSoundBuffer->State )
		{
			case EGAudioFileData::eg_loaded_s::NOT_LOADED : assert( false ); State = NOT_LOADED; break;
			case EGAudioFileData::eg_loaded_s::LOADING    : State = LOADING; break;
			case EGAudioFileData::eg_loaded_s::LOADED     :
			{
				switch(m_SoundDef->Type)
				{
				case eg_sound_class_t::Effect:
				case eg_sound_class_t::Ambience:
					bRes=LoadEffect( m_RawSoundBuffer->Buffer , m_RawSoundBuffer->BufferSize , m_SoundDef->Instances );
					break;
				case eg_sound_class_t::MusicTrack:
					assert( 1 == m_SoundDef->Instances ); //A music track may only have one instance.
					bRes=LoadStream( m_RawSoundBuffer->Buffer , m_RawSoundBuffer->BufferSize );
					break;
				case eg_sound_class_t::Dialog:
					assert( 1 == m_SoundDef->Instances );
					bRes=LoadStream( m_RawSoundBuffer->Buffer , m_RawSoundBuffer->BufferSize );
					break;
				default:
					assert(false); //Do we need to handle this?
					break;
				};

				if( !bRes )
				{
					EG_SafeRelease( m_RawSoundBuffer );
					m_RawSoundBuffer = nullptr;
				}
				m_IsFinalized = bRes;
				SetVolume( 1.f );
				if( m_bShouldPlayOnLoad && m_IsFinalized )
				{
					m_bShouldPlayOnLoad = false;
					Play();
				}
			} break;
		}
	}

	return State;
}

eg_real EGXSound::GetVolumeForType() const
{
	auto GetVolumeForSoundClass = []( eg_sound_class_t SoundClass ) -> eg_real
	{
		switch( SoundClass )
		{
		case eg_sound_class_t::Unknown:
		case eg_sound_class_t::Effect:
		case eg_sound_class_t::Ambience:
			return AudioConfig_EffectVolume.GetAudioValue();
		case eg_sound_class_t::Dialog:
			return AudioConfig_SpeechVolume.GetAudioValue();
		case eg_sound_class_t::MusicTrack:
			return AudioConfig_MusicVolume.GetAudioValue();
		}

		return AudioConfig_EffectVolume.GetAudioValue();
	};

	eg_real Out = 1.f;

	if( m_SoundDef )
	{
		if( m_SoundDef->VolumeOverrideType != eg_sound_class_t::Unknown )
		{
			Out = GetVolumeForSoundClass( m_SoundDef->VolumeOverrideType );
		}
		else
		{
			Out = GetVolumeForSoundClass( m_SoundDef->Type );
		}
	}

	return Out;
}

void* EGXSound::AllocBuffer( eg_size_t Size )
{
	return EGMem2_Alloc( Size , eg_mem_pool::Audio );
}

void EGXSound::FreeBuffer( void* Buffer )
{
	EGMem2_Free( Buffer );
}

eg_real EGXSound::GetRawVolumeInternal() const
{
	return m_Volume*m_SoundTypeVolume*EGMath_VolumeFromLinear( m_SoundDef->Volume );
}

eg_bool EGXSound::LoadStream( void* Data , eg_size_t DataSize )
{
	m_pStreamFile = EGAudioFile::OpenAF( Data , DataSize );
	
	if(!m_pStreamFile)
	{
		Unload();
		return false;
	}

	//2: Create the source voice...
	HRESULT nRes = 0;

	zero(&m_wf);
	m_wf.wFormatTag      = WAVE_FORMAT_PCM;
	m_wf.nChannels       = (WORD)m_pStreamFile->GetNumChannels();
	m_wf.nSamplesPerSec  = (WORD)m_pStreamFile->GetSamplesPerSecond();
	m_wf.wBitsPerSample  = (WORD)m_pStreamFile->GetBitsPerSample();
	m_wf.nBlockAlign     = m_wf.nChannels*m_wf.wBitsPerSample/8;
	m_wf.nAvgBytesPerSec = m_wf.nSamplesPerSec*m_wf.nBlockAlign;
	m_wf.cbSize          = 0;

	nRes = s_pAudio->CreateSourceVoice(&m_apV[0], &m_wf, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &s_VoiceStreamCB);

	if(FAILED(nRes))
	{
		::EGLogf( eg_log_t::Error , __FUNCTION__ ": Could not create the source voice." );
		EGAudioFile::CloseAF(m_pStreamFile);
		return false;
	}

	//3: Allocate memory for the buffers (just a short sample, about a second)...
	m_nBufferSize = m_pStreamFile->GetBitsPerSample()*m_pStreamFile->GetSamplesPerSecond()/8;
	nRes = S_OK;
	for(eg_uint i=0; i<NUM_STREAM_BUFFERS; i++)
	{
		m_apStBufs[i] = (BYTE*)AllocBuffer(sizeof(BYTE)*m_nBufferSize);
		if(!m_apStBufs[i])
			nRes = -1;
	}

	if(FAILED(nRes))
	{
		::EGLogf( eg_log_t::Error , __FUNCTION__ ": Could not allocate %u bytes." , m_nBufferSize );
		for(eg_uint i=0; i<NUM_STREAM_BUFFERS; i++)
		{
			if(m_apStBufs[i])
			{
				FreeBuffer(m_apStBufs[i]);
				m_apStBufs[i] = nullptr;
			}
		}
		m_apV[0]->DestroyVoice();
		m_apV[0] = nullptr;
		EGAudioFile::CloseAF(m_pStreamFile);
		return false;
	}

	zero(&m_bf);
	m_bf.Flags      = XAUDIO2_END_OF_STREAM;
	m_bf.AudioBytes = m_nBufferSize;
	m_bf.pAudioData = m_pBuffer1;
	m_bf.PlayBegin  = 0;
	m_bf.LoopBegin  = 0;
	m_bf.LoopCount  = 0;
	m_bf.PlayLength = 0;
	m_bf.pContext   = reinterpret_cast<void*>(this);

	m_IsStream = true;

	return true;
}

eg_bool EGXSound::LoadEffect( void* Data , eg_size_t DataSize , eg_uint nInstances)
{
	unused( nInstances );
	assert( m_SoundDef->Instances == nInstances );
	//1: Load the audio file...
	EGAudioFile* pFile = EGAudioFile::OpenAF(Data , DataSize );

	if(!pFile)
	{
		return false;
	}

	//2: Create the source voice...
	HRESULT nRes = 0;

	zero(&m_wf);
	m_wf.wFormatTag      = WAVE_FORMAT_PCM;
	m_wf.nChannels       = (WORD)pFile->GetNumChannels();
	m_wf.nSamplesPerSec  = (WORD)pFile->GetSamplesPerSecond();
	m_wf.wBitsPerSample  = (WORD)pFile->GetBitsPerSample();
	m_wf.nBlockAlign     = m_wf.nChannels*m_wf.wBitsPerSample/8;
	m_wf.nAvgBytesPerSec = m_wf.nSamplesPerSec*m_wf.nBlockAlign;
	m_wf.cbSize          = 0;

	eg_bool bRes = true;
	for(eg_uint i=0; i<m_SoundDef->Instances; i++)
	{
		nRes = s_pAudio->CreateSourceVoice(&m_apV[i], &m_wf, 0 , XAUDIO2_DEFAULT_FREQ_RATIO, &s_VoiceStreamCB );
		if(FAILED(nRes))m_apV[i] = nullptr;
		bRes = bRes && SUCCEEDED(nRes);
	}

	if(!bRes)
	{
		for(eg_uint i=0; i<m_SoundDef->Instances; i++)
		{
			if(m_apV[i]){m_apV[i]->DestroyVoice(); m_apV[i]=nullptr;}
		}
		assert( false );
		EGAudioFile::CloseAF(pFile);
		return false;
	}

	//3: Allocate memory for the buffer...
	m_nBufferSize = pFile->GetDataSize();
	m_pBuffer1 = (BYTE*)AllocBuffer(sizeof(BYTE)*m_nBufferSize);
	if(!m_pBuffer1)
	{
		assert( false );
		for(eg_uint i=0; i<m_SoundDef->Instances; i++)
		{
			if(m_apV[i]){m_apV[i]->DestroyVoice(); m_apV[i]=nullptr;}
		}
		EGAudioFile::CloseAF(pFile);
		return false;
	}

	m_nBufferSize = pFile->Read(m_pBuffer1, m_nBufferSize);

	EGAudioFile::CloseAF(pFile);

	zero(&m_bf);
	m_bf.Flags      = XAUDIO2_END_OF_STREAM;
	m_bf.AudioBytes = m_nBufferSize;
	m_bf.pAudioData = m_pBuffer1;
	m_bf.PlayBegin  = 0;
	m_bf.LoopBegin  = 0;
	m_bf.LoopCount  = 0;
	m_bf.PlayLength = 0;
	m_bf.pContext   = reinterpret_cast<void*>(this);

	return true;
}

eg_bool EGXSound::IsReady()
{
	FINALIZE_S SoundState = EnsureSoundIsFinalized();
	eg_bool Ready;
	switch( SoundState )
	{
		case NOT_LOADED:
		case READY:
		case FAILED:
			Ready = true;
			break;
		case LOADING:
			Ready = false;
			break;
	}

	return Ready;
}

void EGXSound::Play()
{
	FINALIZE_S SoundState = EnsureSoundIsFinalized();
	if( READY != SoundState )
	{
		m_bShouldPlayOnLoad = true;
		return;
	}

	//We assume that if play was called, the file is loaded.
	switch(m_SoundDef->Type)
	{
	case eg_sound_class_t::Effect      : Play_Effect(); break;
	case eg_sound_class_t::MusicTrack : Play_Stream(); break;
	case eg_sound_class_t::Ambience : Play_Effect(); break;
	default: assert(false); break; //Do we need to handle this?
	}

	eg_uintptr_t SoundAddr = reinterpret_cast<eg_uintptr_t>(this);
	egSoundEvent Event = { eg_sound_event_t::PLAY , m_SoundHandle };
	s_EGXAudio->QueueSoundEvent( Event );

	m_IsPaused = false;
}

void EGXSound::Play_Effect()
{
	m_IsPlaying = false;
	m_SoundTypeVolume = GetVolumeForType();
	m_apV[m_nNextInst]->Stop();
	m_apV[m_nNextInst]->FlushSourceBuffers();
	m_apV[m_nNextInst]->SubmitSourceBuffer(&m_bf);
	m_apV[m_nNextInst]->SetVolume(GetRawVolumeInternal());
	m_apV[m_nNextInst]->Start(0, PLAY_SOUND_OP_SET);

	m_nNextInst = (m_nNextInst+1) % m_SoundDef->Instances;
	m_IsPlaying = true;
}

void EGXSound::Play_Stream()
{
	m_IsPlaying = true;

	m_SoundTypeVolume = GetVolumeForType();
	m_pStreamFile->Reset();
	m_apV[0]->Stop();
	m_apV[0]->FlushSourceBuffers();
	m_nNextBuffer = 0;
	m_bf.pAudioData = nullptr;
	m_bUpdateStream = false;
	//Queue all of the buffers, that way we should never end up
	//with an empty voice and a slight pause.
	for(eg_uint i=0; i<NUM_STREAM_BUFFERS; i++)
	{
		Update_Buffer();
	}
	m_bUpdateStream = false;
	m_apV[0]->SetVolume(GetRawVolumeInternal());
	m_apV[0]->Start();
}

void EGXSound::Stop()
{
	FINALIZE_S SoundState = EnsureSoundIsFinalized();
	if( READY != SoundState )return;

	if( m_apV[0] )
	{
		m_apV[0]->Stop();
	}
	HRESULT Res = m_apV[0]->FlushSourceBuffers();
	assert( SUCCEEDED(Res) );

	if( m_IsPlaying )
	{
		egSoundEvent Event = { eg_sound_event_t::STOP , m_SoundHandle };
		s_EGXAudio->QueueSoundEvent( Event );
	}

	m_IsPlaying = false;
	m_IsPaused = false;
}

void EGXSound::SetPosition(const eg_vec4* pvPos)
{
	m_avVPos[m_nNextInst] = *pvPos;
}

void EGXSound::Update(const eg_uint nC, X3DAUDIO_HANDLE& H, X3DAUDIO_LISTENER& L)
{
	FINALIZE_S SoundState = EnsureSoundIsFinalized();
	if( READY != SoundState )return;

	if( !m_IsPlaying || m_IsPaused )
		return;
	
	if( m_IsStream )
	{
		Update_Stream();
	}
	
	if((eg_sound_class_t::Effect == m_SoundDef->Type) || (eg_sound_class_t::Dialog == m_SoundDef->Type) ||(eg_sound_class_t::Ambience == m_SoundDef->Type))
	{
		Update_3D(nC, H, L);
	}
}

void EGXSound::Update_3D(const eg_uint nC, X3DAUDIO_HANDLE& H, X3DAUDIO_LISTENER& L)
{
	if( !m_SoundDef->bCentered )
	{
		for(eg_uint i=0; i<m_SoundDef->Instances; i++)
		{
			//We only change the position of the emitter.
			// X3DAUDIO_CONE Cone;
			// zero(&Cone);
			// Cone.InnerAngle = 0.f;
			// Cone.OuterAngle = X3DAUDIO_2PI;
			// Cone.InnerVolume = 1.f;
			// Cone.OuterVolume = 1.f;

			const eg_real Range = GetRange();
			const eg_real RangeFalloffPct = EG_Clamp( GetRangeFalloffPct() , FLT_MIN , 1.f );

			X3DAUDIO_DISTANCE_CURVE_POINT CurvePoints[] = { {0.f , 1.f } , { RangeFalloffPct , 1.f } , { 1.f , 0.f } };
			X3DAUDIO_DISTANCE_CURVE VolumeCurve { CurvePoints , countof(CurvePoints) };

			//Emitter:
			X3DAUDIO_EMITTER E;
			//We currently only change the position vector.
			E.pCone = nullptr;// &Cone;
			E.OrientFront = EGXAudioDevice::CreateVector(0,0,1);
			E.OrientTop   = EGXAudioDevice::CreateVector(0,1,0);
			E.Velocity    = EGXAudioDevice::CreateVector(0,0,0);
			E.InnerRadius = 0;
			E.InnerRadiusAngle = 0;
			E.ChannelCount = 1;
			E.ChannelRadius = 0;
			E.pChannelAzimuths = nullptr;
			E.pVolumeCurve = &VolumeCurve;
			E.pLFECurve = nullptr;
			E.pLPFDirectCurve = nullptr;
			E.pLPFReverbCurve = nullptr;
			E.CurveDistanceScaler = Range;
			E.DopplerScaler = 0;
			E.Position = EGXAudioDevice::CreateVector( m_avVPos[i].ToVec3() );

			X3DAUDIO_DSP_SETTINGS DSP;
			zero(&DSP);
			FLOAT32 m[MAX_CHANNELS];
			zero(&m);
			DSP.pMatrixCoefficients = m;
			DSP.SrcChannelCount = 1;
			DSP.DstChannelCount = EG_Min( nC , MAX_CHANNELS );

			if( 1 == m_wf.nChannels && (countof(m) >= (DSP.SrcChannelCount*DSP.DstChannelCount)) )
			{
				::X3DAudioCalculate(H, &L, &E, X3DAUDIO_CALCULATE_MATRIX, &DSP);
				m_apV[i]->SetOutputMatrix(nullptr, 1, DSP.DstChannelCount, m, UPDATE_3D_OP_SET);
			}

			if( EGXSound_Show3DMatrix.GetValue() && 0 == i)
			{
				const FLOAT32* pm = DSP.pMatrixCoefficients;
				EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Distance: {0}" , DSP.EmitterToListenerDistance ) );
				EGLogToScreen::Get().Log( this , __LINE__ , 1.f , ("Matrix:") );
				for( eg_uint i=0; i<DSP.DstChannelCount; i++ )
				{
					EGLogToScreen::Get().Log( this , __LINE__+i , 1.f , EGString_Format( "%.6f", pm[i] ) );
				}
			}

			E.pCone = nullptr;
		}
	}
}

void EGXSound::Update_Stream()
{	
	if(m_bUpdateStream)
	{
		Update_Buffer();
		//We should probably check to see that voice is queued for all of the
		//stream buffers and fill it to capacity if it's not.
		m_bUpdateStream = false;
	}
}

eg_bool EGXSound::Update_Buffer()
{	
	if( !m_IsPlaying )return false;

	if( eg_sound_class_t::MusicTrack == m_SoundDef->Type )
	{
		m_SoundTypeVolume = GetVolumeForType();
		m_apV[0]->SetVolume(GetRawVolumeInternal());
	}

	eg_uint nRead=0;
	BYTE* pBuffer = m_apStBufs[m_nNextBuffer];
	//We technically want to read until we've read all the data that we
	//want, but in case we end up not reading anything we limit the number
	//of times that we loop. Realistically this should only go through
	//one time and only loop if we reach the end of the file. The read size
	//is meant to be an integer number of samples so the read should work
	//but if the audio file format isn't happy with the read size, there
	//can be problems here. So far the ogg vorbis format hasn't ever
	//had problems here, and the reads seem to be pretty much exact.
	const eg_uint MAX_LOOPS = 20;
	for(eg_uint l=0; nRead<m_nBufferSize && l< MAX_LOOPS; l++)
	{
		nRead+=m_pStreamFile->Read(pBuffer+nRead, m_nBufferSize-nRead);
		if(m_pStreamFile->IsEOF())
		{
			egSoundEvent CompleteEvent = { eg_sound_event_t::COMPLETE , m_SoundHandle };
			s_EGXAudio->QueueSoundEvent( CompleteEvent );

			if(m_SoundDef->bLooping)
			{
				m_pStreamFile->Reset();
				continue;
			}

			Stop();
			return true;
		}
	}
	m_bf.pAudioData = pBuffer;
	HRESULT Res = m_apV[0]->SubmitSourceBuffer(&m_bf);
	assert(SUCCEEDED(Res) );

	m_nNextBuffer = (m_nNextBuffer+1)%NUM_STREAM_BUFFERS;
	return nRead==m_nBufferSize?true:false;
}



void EGXSound::SetVolume(eg_real fVol)
{
	m_Volume = EGMath_VolumeFromLinear( fVol );
	FINALIZE_S SoundState = EnsureSoundIsFinalized();
	if( READY != SoundState )return;

	m_Volume=EG_Clamp<eg_real>(m_Volume, 0, 1.f);

	for(eg_uint i=0; i<m_SoundDef->Instances; i++)
	{
		m_apV[i]->SetVolume( GetRawVolumeInternal() );
	}
}

void EGXSound::EGVoiceStreamCB::OnVoiceProcessingPassStart(UINT32 /*BytesRequired*/){}
void EGXSound::EGVoiceStreamCB::OnVoiceProcessingPassEnd(){}
void EGXSound::EGVoiceStreamCB::OnStreamEnd(){}
void EGXSound::EGVoiceStreamCB::OnBufferStart(void* /*pBufferContext*/){}

void EGXSound::EGVoiceStreamCB::OnBufferEnd(void* pBufferContext)
{
	EGXSound* _this = reinterpret_cast<EGXSound*>(pBufferContext);

	if(_this->m_IsStream)
	{
		_this->m_bUpdateStream = true;
	}
	else
	{
		if( _this->m_IsPlaying )
		{
			egSoundEvent CompleteEvent = { eg_sound_event_t::COMPLETE , _this->m_SoundHandle };
			s_EGXAudio->QueueSoundEvent( CompleteEvent );
			egSoundEvent StopEvent = { eg_sound_event_t::STOP , _this->m_SoundHandle };
			s_EGXAudio->QueueSoundEvent( StopEvent );
		}
		_this->m_IsPlaying = false;
	}
}

void EGXSound::EGVoiceStreamCB::OnLoopEnd(void* /*pBufferContext*/){}
void EGXSound::EGVoiceStreamCB::OnVoiceError(void* /*pBufferContext*/, HRESULT /*Error*/){ assert(false); }

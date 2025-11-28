// (c) 2011 Beem Media

#include "EGAudio.h"
#include "EGAudioDevice.h"
#include "EGAudioList.h"

static EGAudioDevice* Audio_Lib = nullptr;
class EGAudioList* MainAudioList = nullptr;

static EGAudioDevice* EGAudio_CreateDevice( eg_string_crc SystemId );

eg_bool EGAudio_Init( eg_string_crc DriverId )
{
	Audio_Lib = EGAudio_CreateDevice( DriverId );

	return nullptr != Audio_Lib;
}

void EGAudio_Deinit( void )
{
	EG_SafeRelease( Audio_Lib );
}

void EGAudio_BeginFrame( void )
{
	assert( nullptr == MainAudioList );
	MainAudioList = Audio_Lib->BeginFrame_MainThread();
}

void EGAudio_EndFrame( void )
{
	Audio_Lib->ProcessCBs();
	assert( nullptr != MainAudioList );
	Audio_Lib->EndFrame_MainThread( MainAudioList );
	MainAudioList = nullptr;
}

egs_sound EGAudio_CreateSound( eg_cpstr File )
{
	return Audio_Lib->CreateSound( File );
}

void EGAudio_DestroySound( egs_sound Sound )
{
	Audio_Lib->DestroySound( Sound );
}

void EGAudio_AddCB( ISoundCB* Cb )
{
	Audio_Lib->AddCB( Cb );
}

void EGAudio_RemoveCB( ISoundCB* Cb )
{
	Audio_Lib->RemoveCB( Cb );
}

void EGAudio_QueryAllSounds( EGArray<eg_string_small>& SoundsOut )
{
	if( Audio_Lib )
	{
		Audio_Lib->QueryAllSounds( SoundsOut );
	}
}

#include "EGNoAudioDevice.h"

static EGAudioDevice* EGAudio_CreateDevice( eg_string_crc SystemId )
{
	EGAudioDevice* pAudio = nullptr;
	EGClass* Class = nullptr;

	if( eg_crc("XAUDIO2") == SystemId || eg_crc("DEFAULT") == SystemId )
	{
		EGLogf( eg_log_t::Audio, "Creating XAudio2 sound driver..." );
		Class = EGClass::FindClassWithBackup( "EGXAudioDevice" , EGNoAudioDevice::GetStaticClass() );
	}
	else if( eg_crc("NULL") == SystemId )
	{
		EGLogf( eg_log_t::Audio , "Creating NoAudio sound driver..." );
		Class = &EGNoAudioDevice::GetStaticClass();
	}
	else
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": No valid audio driver specified, creating NoAudio sound driver..." );
		Class = &EGNoAudioDevice::GetStaticClass();
	}

	pAudio = EGNewObject<EGAudioDevice>( Class , eg_mem_pool::Audio );

	return pAudio;
}

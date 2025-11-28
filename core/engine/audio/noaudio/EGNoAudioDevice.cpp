// (c) 2017 Beem Media

#include "EGNoAudioDevice.h"

EG_CLASS_DECL( EGNoAudioDevice )

EGNoAudioDevice::EGNoAudioDevice()
{
	m_AlMemSize = 10 * 1024;
	m_AlMem = EGMem2_Alloc( m_AlMemSize, eg_mem_pool::System );
}

EGNoAudioDevice::~EGNoAudioDevice()
{
	EGMem2_Free( m_AlMem );
}

egs_sound EGNoAudioDevice::CreateSound( eg_cpstr SoundId )
{
	unused( SoundId ); 
	return egs_sound::Null;
}

void EGNoAudioDevice::DestroySound( egs_sound Sound )
{
	unused( Sound );
}

EGAudioList* EGNoAudioDevice::BeginFrame_MainThread()
{

	m_AlList.InitAudioList( m_AlMem, m_AlMemSize, 0 );
	return &m_AlList;
}

void EGNoAudioDevice::EndFrame_MainThread( EGAudioList* Al )
{
	unused( Al );
	assert( Al == &m_AlList );
	m_AlList.DeinitAudioList();
}

void EGNoAudioDevice::AddCB( ISoundCB* Cb )
{
	unused( Cb );
}

void EGNoAudioDevice::RemoveCB( ISoundCB* Cb )
{
	unused( Cb );
}

void EGNoAudioDevice::ProcessCBs()
{

}

void EGNoAudioDevice::QueryAllSounds( EGArray<eg_string_small>& SoundsOut )
{
	unused( SoundsOut );
}

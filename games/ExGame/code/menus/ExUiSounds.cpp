// (c) 2016 Beem Media

#include "ExUiSounds.h"
#include "EGAudio.h"
#include "EGAudioList.h"

ExUiSounds ExUiSounds::s_Instance;

void ExUiSounds::Init()
{
	m_Sounds[static_cast<eg_uint>(ex_sound_e::DIALOG_OPEN)] = EGAudio_CreateSound( "uisound:DialogOpen" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::SELECTION_CHANGED)] = EGAudio_CreateSound( "uisound:ButtonHighlight" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::INC_OR_DEC_ITEM)] = EGAudio_CreateSound( "uisound:ButtonHighlight" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::LEVEL_UP)] = EGAudio_CreateSound( "uisound:LevelUp" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::PURCHASE)] = EGAudio_CreateSound( "uisound:Purchase" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::NOT_ALLOWED)] = EGAudio_CreateSound( "uisound:NotAllowed" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::MenuSlide)] = EGAudio_CreateSound( "uisound:MenuSlide" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::HealParty)] = EGAudio_CreateSound( "uisound:HealParty" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::TeleportParty)] = EGAudio_CreateSound( "uisound:TeleportParty" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::TranslatorSlide)] = EGAudio_CreateSound( "uisound:TranslatorSlide" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::InventoryEquip)] = EGAudio_CreateSound( "uisound:InventoryEquip" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::Ambush)] = EGAudio_CreateSound( "uisound:Ambush" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::ExchangePartyMember)] = EGAudio_CreateSound( "uisound:ExchangePartyMember" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::ConfirmDialogChoice)] = EGAudio_CreateSound( "uisound:ConfirmDialogChoice" );
	m_Sounds[static_cast<eg_uint>(ex_sound_e::BuyFood)] = EGAudio_CreateSound( "uisound:BuyFood" );

	m_TextRevealSound = EGAudio_CreateSound( "uisound:DialogTextReveal" );
}

void ExUiSounds::Deinit()
{
	for( eg_uint i=0; i<countof(m_Sounds); i++ )
	{
		EGAudio_DestroySound( m_Sounds[i] );
		m_Sounds[i] = egs_sound::Null;
	}

	EGAudio_DestroySound( m_TextRevealSound );
	m_TextRevealSound = egs_sound::Null;
}

void ExUiSounds::PlaySoundEvent( ex_sound_e Event )
{
	MainAudioList->PlaySound( m_Sounds[static_cast<eg_uint>(Event)] );
}

void ExUiSounds::Update( eg_real DeltaTime )
{
	unused( DeltaTime );

	if( MainAudioList )
	{
		MainAudioList->UpdateAmbientSound( m_TextRevealSound , CT_Clear , m_TextRevealCount > 0 );
	}
}

void ExUiSounds::SetTextRevealPlaying( eg_bool bPlaying )
{
	if( bPlaying )
	{
		m_TextRevealCount++;
	}
	else
	{
		m_TextRevealCount--;
	}
}

// (c) 2016 Beem Media

#pragma once

#include "EGAudioTypes.h"

class ExUiSounds
{
public:
	
	enum class ex_sound_e
	{
		DIALOG_OPEN,
		INC_OR_DEC_ITEM,
		SELECTION_CHANGED,
		LEVEL_UP,
		PURCHASE,
		NOT_ALLOWED,
		MenuSlide,
		HealParty,
		TranslatorSlide,
		InventoryEquip,
		Ambush,
		ExchangePartyMember,
		TeleportParty ,
		ConfirmDialogChoice ,
		BuyFood ,

		COUNT,
	};

public:
	
	static ExUiSounds& Get(){ return s_Instance; }

	ExUiSounds()
	{
		zero( &m_Sounds );
	}

	void Init();
	void Deinit();

	void PlaySoundEvent( ex_sound_e Event );

	void Update( eg_real DeltaTime );
	void SetTextRevealPlaying( eg_bool bPlaying );

private:
	
	egs_sound m_Sounds[static_cast<eg_uint>(ex_sound_e::COUNT)];
	egs_sound m_TextRevealSound = egs_sound::Null;
	eg_int m_TextRevealCount = 0;

private:
	
	static ExUiSounds s_Instance;
};
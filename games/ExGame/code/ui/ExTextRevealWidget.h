// (c) 2017 Beem Media

#pragma once

#include "EGUiTextWidget.h"
#include "ExGameTypes.h"

class ExTextRevealWidget : public EGUiTextWidget
{
	EG_CLASS_BODY( ExTextRevealWidget , EGUiTextWidget )

private:

	eg_loc_char m_FinalText[1024];
	eg_size_t   m_EffectiveLength; // Length of string not including formatting
	eg_real     m_RevealDuration;
	eg_real     m_RevealTime;
	eg_bool     m_bIsFullyRevealed;

public:

	// BEGIN EGUiTextWidget
	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override final;
	virtual void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText ) override final;
	// END EGUiTextWidget

	void RevealText( const eg_loc_text& NewText , ex_reveal_speed RevealSpeed );
	void ForceFullReveal();
	eg_bool IsFullyRevealed() const { return m_bIsFullyRevealed; }
	eg_bool IsRevealing() const { return !m_bIsFullyRevealed; }

public:

	static eg_real GetRevealSeconds( ex_reveal_speed Speed , eg_size_t StrLen );
	static eg_size_t GetEffectiveStringLen( const eg_loc_char* Str );
	static eg_size_t GetStringEndByPos( const eg_loc_char* Str , eg_size_t Pos );
	static void StrCatNonFormatCharacters( eg_loc_char* Str, eg_size_t StrSize , const eg_loc_char* AppendStr );
	static void StrCatNonFormatCharacters( eg_d_string16& Str , const eg_loc_char* AppendStr );
};

// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGTextNodeComponent.h"
#include "ExGameTypes.h"
#include "EGAudioTypes.h"
#include "ExTextRevealComponent.reflection.h"

egreflect class ExTextRevealComponent : public egprop EGTextNodeComponent
{
	EG_CLASS_BODY( ExTextRevealComponent , EGTextNodeComponent )
	EG_FRIEND_RFL( ExTextRevealComponent )

protected:

	eg_loc_text m_FinalText;
	eg_size_t   m_EffectiveLength; // Length of string not including formatting
	eg_real     m_RevealDuration;
	eg_real     m_RevealTime;
	eg_bool     m_bIsFullyRevealed = true;
	eg_bool     m_bIsRevealSoundPlaying = false;

	egprop eg_bool m_bPlaySound = false;
	egprop ex_reveal_speed m_ScriptRevealSpeed = ex_reveal_speed::Normal;
	egprop EGArray<exLocTextMultiline> m_TextDatabase;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;
	virtual void RefreshFromDef() override;

public:

	virtual void OnDestruct() override;
	virtual void ActiveUpdate( eg_real DeltaTime ) override final;
	virtual void SetText( const class eg_loc_text& NewText ) override;

	void RevealText( const eg_loc_text& NewText , ex_reveal_speed RevealSpeed );
	void ForceFullReveal();
	eg_bool IsFullyRevealed() const { return m_bIsFullyRevealed; }
};

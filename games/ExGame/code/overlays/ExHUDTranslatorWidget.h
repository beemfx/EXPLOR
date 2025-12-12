// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGUiMeshWidget.h"

class ExTextRevealComponent;

class ExHUDTranslatorWidget : public EGUiMeshWidget
{
	EG_CLASS_BODY( ExHUDTranslatorWidget , EGUiMeshWidget )

protected:

	ExTextRevealComponent* m_TextReveal;
	eg_string_crc          m_CurrentText;
	eg_uint                m_CurrentPartyMember;

	eg_bool m_bTranslatorActive = false;
	eg_bool m_bIsMoving = false;
	eg_real m_MoveTimeElapsed = 0.f;
	const eg_real m_MoveDuration = .25f;
	const eg_real m_HiddenFinalOffset = 39.f;
	eg_bool m_bUseTextScrolling = false;

protected:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	virtual void OnDestruct() override;
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio ) override;

	void SetTranslatorActive( eg_bool bActive , eg_bool bImmediate );
	void UpdateTranslatorPose( eg_real DeltaTime );
	void ApplyTranslatorPos( eg_real Offset );

	void OnTranslationChangedCallback();
};

// (c) 2016 Beem Media

#pragma once

#include "ExUiSounds.h"
#include "EGUiMeshWidget.h"

class ExBgComponent : public EGUiMeshWidget
{
	EG_CLASS_BODY( ExBgComponent , EGUiMeshWidget )

public:
	class ICb
	{
	public:
		virtual void OnBgComponentAnimComplete( ExBgComponent* Bg ) = 0;
	};

private:
	ICb*    m_Cb;
	eg_real m_AnimDuration;
	eg_vec2 m_WidthFromTo;
	eg_vec2 m_HeightFromTo;
	eg_vec2 m_AdditionalOffset;
	eg_real m_RevealTime;
	eg_bool m_bIsRevealing:1;
	eg_bool m_bAudioMuted:1;

public:

	void InitWidget( ICb* Cb );

	void SetAudioMuted( eg_bool bNewValue ) { m_bAudioMuted = bNewValue; }
	void PlayReveal( eg_uint NumChoices = 0 );
	void PlayFullScreenInAnim();
	void PlayFullScreenSmallInAnim();
	void PlayQuestLogInAnim();
	void PlayDialogInAnim( eg_uint NumChoices );
	void PlayChoiceWidgetIntroAnim( eg_int NumChoices );
	void PlayKeyBindDialogBoxInAnim();

	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override;

	eg_bool IsRevealing() const { return m_bIsRevealing; }

private:

	void AnimateReveal( eg_real DeltaTime );
	
};

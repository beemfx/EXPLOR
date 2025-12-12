// (c) 2020 Beem Media. All Rights Reserved.

#include "ExHudTextWidget.h"

EG_CLASS_DECL( ExHudTextWidget )

void ExHudTextWidget::HudPlayShow()
{
	SetVisible( true );
	m_AlphaTween.bActive = true;
	m_AlphaTween.TimeElapsed = 0.f;
	m_AlphaTween.Duration = .25f;
	m_AlphaTween.StartValue = m_TextColor.a;
	m_AlphaTween.EndValue = 1.f;
}

void ExHudTextWidget::HudPlayHide()
{
	m_AlphaTween.bActive = true;
	m_AlphaTween.TimeElapsed = 0.f;
	m_AlphaTween.Duration = .25f;
	m_AlphaTween.StartValue = m_TextColor.a;
	m_AlphaTween.EndValue = 0.f;
}

void ExHudTextWidget::HudShowNow()
{
	SetVisible( true );
	m_AlphaTween.bActive = false;
	m_TextColor.a = 1.f;
}

void ExHudTextWidget::HudHideNow()
{
	SetVisible( false );
	m_AlphaTween.bActive = false;
	m_TextColor.a = 0.f;
}

void ExHudTextWidget::Update( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	if( m_AlphaTween.bActive && m_AlphaTween.Duration > 0.f )
	{
		m_AlphaTween.TimeElapsed += DeltaTime;
		if( m_AlphaTween.TimeElapsed >= m_AlphaTween.Duration )
		{
			m_TextColor.a = m_AlphaTween.EndValue;
			m_AlphaTween.bActive = false;
			if( EG_IsEqualEps( m_TextColor.a , 0.f ) )
			{
				SetVisible( false );
			}
		}
		else
		{
			eg_real NormalizedTransition = m_AlphaTween.TimeElapsed / m_AlphaTween.Duration;
			NormalizedTransition = EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , NormalizedTransition );
			m_TextColor.a = m_AlphaTween.StartValue*(1.f - NormalizedTransition) + m_AlphaTween.EndValue*NormalizedTransition;
		}
	}
}

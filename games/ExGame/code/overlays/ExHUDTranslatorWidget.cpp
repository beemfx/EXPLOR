// (c) 2020 Beem Media. All Rights Reserved.

#include "ExHUDTranslatorWidget.h"
#include "ExTextRevealComponent.h"
#include "ExMenu.h"
#include "ExGameSettings.h"

EG_CLASS_DECL( ExHUDTranslatorWidget )

void ExHUDTranslatorWidget::Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	m_TextReveal = m_EntObj.GetComponentTree().FindComponentByClass<ExTextRevealComponent>();

	if( !IsToolPreview() )
	{
		ExMenu* Owner = EGCast<ExMenu>(GetOwnerMenu());
		ExGame* Game = Owner ? EGCast<ExGame>(Owner->GetClientOwner()->SDK_GetGame()) : nullptr;
		if( Game )
		{
			Game->OnClientTranslationChangedDelegate.AddUnique( this , &ThisClass::OnTranslationChangedCallback );
		}

		SetTranslatorActive( false , true );
		OnTranslationChangedCallback();
	}
	else
	{
		SetTranslatorActive( true , true );
	}
}

void ExHUDTranslatorWidget::OnDestruct()
{
	if( !IsToolPreview() )
	{
		ExMenu* Owner = EGCast<ExMenu>(GetOwnerMenu());
		ExGame* Game = Owner ? EGCast<ExGame>(Owner->GetClientOwner()->SDK_GetGame()) : nullptr;
		if( Game )
		{
			Game->OnClientTranslationChangedDelegate.RemoveAll( this );
		}
	}

	Super::OnDestruct();
}

void ExHUDTranslatorWidget::Update( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	UpdateTranslatorPose( DeltaTime );
}

void ExHUDTranslatorWidget::SetTranslatorActive( eg_bool bActive , eg_bool bImmediate )
{
	if( m_bTranslatorActive != bActive || bImmediate )
	{
		ExMenu* Owner = EGCast<ExMenu>(GetOwnerMenu());
		ExGame* Game = Owner ? EGCast<ExGame>(Owner->GetClientOwner()->SDK_GetGame()) : nullptr;
		const ExFighter* RelevantPartyMember = Game ? Game->GetPartyMemberByIndex( m_CurrentPartyMember ) : nullptr;
		
		m_bTranslatorActive = bActive;

		if( m_bTranslatorActive )
		{
			if( m_TextReveal )
			{
				m_TextReveal->SetText( CT_Clear );
			}
		}

		if( bImmediate )
		{
			m_bIsMoving = false;
			ApplyTranslatorPos( m_bTranslatorActive ? 0.f : m_HiddenFinalOffset );
			if( m_TextReveal )
			{
				if( RelevantPartyMember )
				{
					m_TextReveal->SetText( EGFormat( m_CurrentText , *RelevantPartyMember ) );
				}
				else
				{
					m_TextReveal->SetText( eg_loc_text(m_CurrentText) );
				}
			}
			SetVisible( bActive );
		}
		else
		{
			if( bActive )
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::TranslatorSlide );
				if( !m_bUseTextScrolling )
				{
					if( m_TextReveal )
					{
						if( RelevantPartyMember )
						{
							m_TextReveal->SetText( EGFormat( m_CurrentText , *RelevantPartyMember ) );
						}
						else
						{
							m_TextReveal->SetText( eg_loc_text(m_CurrentText) );
						}
					}
				}
			}
			SetVisible( true );
			if( !m_bIsMoving )
			{
				m_bIsMoving = true;
				m_MoveTimeElapsed = 0.f;
			}
			else
			{
				m_MoveTimeElapsed = m_MoveDuration - m_MoveTimeElapsed;
			}
		}
	}
}

void ExHUDTranslatorWidget::UpdateTranslatorPose( eg_real DeltaTime )
{
	if( m_bIsMoving )
	{
		m_MoveTimeElapsed += DeltaTime;

		if( m_MoveTimeElapsed >= m_MoveDuration )
		{
			// Done moving...

			m_bIsMoving = false;
			SetVisible( m_bTranslatorActive );

			if( m_bTranslatorActive )
			{
				if( m_bUseTextScrolling )
				{
					if( m_TextReveal )
					{
						m_TextReveal->RevealText( eg_loc_text(m_CurrentText) , ExGameSettings_DialogTextSpeed.GetValue() );
					}
				}

				ApplyTranslatorPos( m_bTranslatorActive ? 0.f : m_HiddenFinalOffset );
			}
		}
		else
		{
			eg_real MoveOffset = 0.f;
			if( m_bTranslatorActive )
			{
				MoveOffset = EGMath_GetMappedCubicRangeValue( m_MoveTimeElapsed , eg_vec2(0.f,m_MoveDuration) , eg_vec2(m_HiddenFinalOffset,0.f) );
			}
			else
			{
				MoveOffset = EGMath_GetMappedCubicRangeValue( m_MoveTimeElapsed , eg_vec2(0.f,m_MoveDuration) , eg_vec2(0.f,m_HiddenFinalOffset) );
			}

			ApplyTranslatorPos( MoveOffset );
		}
	}
}

void ExHUDTranslatorWidget::ApplyTranslatorPos( eg_real Offset )
{
	SetOffset( EGUiWidget::eg_offset_t::POST , eg_transform::BuildTranslation( 0.f , Offset , 0.f ) );
}

void ExHUDTranslatorWidget::OnTranslationChangedCallback()
{
	ExMenu* Owner = EGCast<ExMenu>(GetOwnerMenu());
	ExGame* Game = Owner ? EGCast<ExGame>(Owner->GetClientOwner()->SDK_GetGame()) : nullptr;
	const eg_string_crc OldText = m_CurrentText;
	m_CurrentText = Game ? Game->GetActiveTranslationText().Message : CT_Clear;
	m_CurrentPartyMember = Game ? Game->GetActiveTranslationText().PartyMemberIndex : CT_Clear;

	if( m_bTranslatorActive && OldText != m_CurrentText && m_CurrentText.IsNotNull() )
	{
		SetTranslatorActive( false , true );
	}
	SetTranslatorActive( m_CurrentText.IsNotNull() , false );
}

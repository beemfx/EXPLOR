// (c) 2020 Beem Media. All Rights Reserved.

#include "ExTextRevealComponent.h"
#include "ExTextRevealWidget.h"
#include "ExUiSounds.h"

EG_CLASS_DECL( ExTextRevealComponent )

void ExTextRevealComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const ExTextRevealComponent* DefAsReveal = EGCast<ExTextRevealComponent>(InitData.Def);
	if( DefAsReveal )
	{
		// Technically we don't really need to copy the text database at it really just exists so that the localization tool can pick up the texts.
		m_TextDatabase = DefAsReveal->m_TextDatabase;
	}
}

void ExTextRevealComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("SetText"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				SetText( eg_loc_text(Action.StrCrcParm(0)) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetText." );
			}
		} break;

		case_crc("RevealText"):
		{
			if( Action.FnCall.NumParms >= 1 )
			{
				RevealText( eg_loc_text(Action.StrCrcParm(0)) , m_ScriptRevealSpeed );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for RevealText." );
			}
		} break;

		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

void ExTextRevealComponent::RefreshFromDef()
{
	Super::RefreshFromDef();

	if( const ExTextRevealComponent* RevDef = EGCast<ExTextRevealComponent>(m_TextNodeDef) )
	{
		m_ScriptRevealSpeed = RevDef->m_ScriptRevealSpeed;
		m_bPlaySound = RevDef->m_bPlaySound;
	}
}

void ExTextRevealComponent::OnDestruct()
{
	if( m_bIsRevealSoundPlaying )
	{
		m_bIsRevealSoundPlaying = false;
		ExUiSounds::Get().SetTextRevealPlaying( false );
	}
	Super::OnDestruct();
}

void ExTextRevealComponent::ActiveUpdate( eg_real DeltaTime )
{
	Super::ActiveUpdate( DeltaTime );

	eg_bool bShouldPlayAudio = false;

	if( !m_bIsFullyRevealed )
	{
		m_RevealTime += DeltaTime;

		bShouldPlayAudio = m_bPlaySound;

		if( m_RevealTime >= m_RevealDuration || m_EffectiveLength == 0 )
		{
			m_bIsFullyRevealed = true;
			m_CreatedLocText = m_FinalText;
		}
		else
		{
			eg_d_string16 CurrentText;
			eg_real RevealCharsAsFloat = EGMath_GetMappedRangeValue( m_RevealTime , eg_vec2(0.f,m_RevealDuration) , eg_vec2(0,static_cast<eg_real>(m_EffectiveLength)) );
			eg_size_t RevealChars = ExTextRevealWidget::GetStringEndByPos( m_FinalText.GetString() , static_cast<eg_size_t>(RevealCharsAsFloat));
			CurrentText = m_FinalText.GetString();
			CurrentText.Shrink( RevealChars );
			CurrentText.Append( L"|SetColor(0,0,0,0)|" );
			ExTextRevealWidget::StrCatNonFormatCharacters( CurrentText , &m_FinalText.GetString()[RevealChars] );
			m_CreatedLocText = eg_loc_text( *CurrentText );
		}
	}

	if( bShouldPlayAudio )
	{
		if( !m_bIsRevealSoundPlaying )
		{
			m_bIsRevealSoundPlaying = true;
			ExUiSounds::Get().SetTextRevealPlaying( true );
		}
	}
	else
	{
		if( m_bIsRevealSoundPlaying )
		{
			m_bIsRevealSoundPlaying = false;
			ExUiSounds::Get().SetTextRevealPlaying( false );
		}
	}
}

void ExTextRevealComponent::SetText( const class eg_loc_text& NewText )
{
	Super::SetText( NewText );
	m_bIsFullyRevealed = true;
}

void ExTextRevealComponent::RevealText( const eg_loc_text& NewText , ex_reveal_speed RevealSpeed )
{
	m_FinalText = NewText;
	m_EffectiveLength = ExTextRevealWidget::GetEffectiveStringLen(m_FinalText.GetString());
	m_RevealDuration = ExTextRevealWidget::GetRevealSeconds( RevealSpeed , m_EffectiveLength );
	m_RevealTime = 0.f;
	m_bIsFullyRevealed = false;
	m_LocText = CT_Clear;
}

void ExTextRevealComponent::ForceFullReveal()
{
	m_RevealTime = m_RevealDuration + 1.f;
}

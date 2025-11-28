// (c) 2017 Beem Media

#include "ExTextRevealWidget.h"

EG_CLASS_DECL( ExTextRevealWidget )

void ExTextRevealWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	if( !m_bIsFullyRevealed )
	{
		m_RevealTime += DeltaTime;

		if( m_RevealTime >= m_RevealDuration || m_EffectiveLength == 0 )
		{
			m_bIsFullyRevealed = true;
			m_LocText = eg_loc_text(m_FinalText);
		}
		else
		{
			eg_loc_char CurrentText[1024];
			eg_real RevealCharsAsFloat = EGMath_GetMappedRangeValue( m_RevealTime , eg_vec2(0.f,m_RevealDuration) , eg_vec2(0,static_cast<eg_real>(m_EffectiveLength)) );

			eg_size_t RevealChars = GetStringEndByPos( m_FinalText , static_cast<eg_size_t>(RevealCharsAsFloat));
			EGString_Copy( CurrentText , m_FinalText , EG_Min(RevealChars+1,countof(CurrentText)) );

			EGString_StrCat( CurrentText , countof(m_FinalText) , L"|SetColor(0,0,0,0)|" );
			StrCatNonFormatCharacters( CurrentText , countof(CurrentText) , &m_FinalText[RevealChars] );

			m_LocText = eg_loc_text( CurrentText );
		}
	}
}

void ExTextRevealWidget::SetText( eg_string_crc TextNodeCrcId, const eg_loc_text& NewText )
{
	Super::SetText( TextNodeCrcId , NewText );
	m_bIsFullyRevealed = true;
}

void ExTextRevealWidget::RevealText( const eg_loc_text& NewText, ex_reveal_speed RevealSpeed )
{
	assert( m_Info->TextNodeInfo.TextContext == eg_text_context::None ); // Won't work with dynamic text context updates...

	EGString_Copy( m_FinalText , NewText.GetString() , countof(m_FinalText) );
	m_EffectiveLength = GetEffectiveStringLen(m_FinalText);
	m_RevealDuration = GetRevealSeconds( RevealSpeed , m_EffectiveLength );
	m_RevealTime = 0.f;
	m_bIsFullyRevealed = false;
	m_LocText = CT_Clear;
}

void ExTextRevealWidget::ForceFullReveal()
{
	m_RevealTime = m_RevealDuration + 1.f;
}

eg_real ExTextRevealWidget::GetRevealSeconds( ex_reveal_speed Speed, eg_size_t StrLen )
{
	eg_real Out = 1.f;

	switch( Speed )
	{
	case ex_reveal_speed::Slow:
		Out = StrLen*.05f;
		break;
	case ex_reveal_speed::Normal:
		Out = StrLen*.035f;
		break;
	case ex_reveal_speed::Fast:
		Out = StrLen*.02f;
		break;
	case ex_reveal_speed::SuperFast:
		Out = EG_Min( .75f, StrLen*.02f );
		break;
	}

	return Out;
}

eg_size_t ExTextRevealWidget::GetEffectiveStringLen( const eg_loc_char* Str )
{
	eg_size_t Count = 0;
	eg_bool bInFormat = false;
	while( *Str )
	{
		if( '|' == *Str )
		{
			bInFormat = !bInFormat;
		}
		else
		{
			if( !bInFormat )
			{
				Count++;
			}
		}
		Str++;
	}
	return Count;
}

eg_size_t ExTextRevealWidget::GetStringEndByPos( const eg_loc_char* Str, eg_size_t Pos )
{
	eg_size_t i = 0;
	eg_size_t CharsPassed = 0;
	eg_bool bInFormat = false;
	while( *Str && i <= Pos )
	{
		if( *Str == '|' )
		{
			bInFormat = !bInFormat;
		}
		else
		{
			if( !bInFormat )
			{
				i++;
			}
		}

		Str++;
		CharsPassed++;
	}
	return CharsPassed;
}

void ExTextRevealWidget::StrCatNonFormatCharacters( eg_loc_char* OutStr, eg_size_t StrSize, const eg_loc_char* AppendStr )
{
	eg_size_t WritePos = EGString_StrLen( OutStr );

	auto AppendChar = [&WritePos,&StrSize,&OutStr]( eg_loc_char Char ) -> void
	{
		if( WritePos >= (StrSize-1) )
		{
			OutStr[StrSize-1] = '\0';
		}
		else
		{
			OutStr[WritePos] = Char;
			OutStr[WritePos+1] = '\0';
			WritePos++;
		}
	};

	eg_bool bInFormat = false;
	while( *AppendStr )
	{
		if( *AppendStr == '|' )
		{
			bInFormat = !bInFormat;
		}
		else
		{
			if( !bInFormat )
			{
				AppendChar( *AppendStr );
			}
		}

		AppendStr++;
	}
}

void ExTextRevealWidget::StrCatNonFormatCharacters( eg_d_string16& Str , const eg_loc_char* AppendStr )
{
	eg_bool bInFormat = false;
	while( *AppendStr )
	{
		if( *AppendStr == '|' )
		{
			bInFormat = !bInFormat;
		}
		else
		{
			if( !bInFormat )
			{
				Str.Append( *AppendStr );
			}
		}

		AppendStr++;
	}
}

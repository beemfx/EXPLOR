#include "ExTextEditWidget.h"
#include "EGTextFormat.h"
#include "ExUiSounds.h"
#include "ExKeyboardMenu.h"
#include "EGMenu.h"

static const eg_loc_char EX_TEXT_EDIT_DEFAULT_ALLOWED_CHARS[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789_";
static const eg_uint EX_TEXT_EDIT_DEFAULT_MAX_LENGTH = 32;

EG_CLASS_DECL( ExTextEditWidget )

ExTextEditWidget::ExTextEditWidget()
{
	InitWidget( EX_TEXT_EDIT_DEFAULT_ALLOWED_CHARS , EX_TEXT_EDIT_DEFAULT_MAX_LENGTH );
	SetText( L"" );
}

void ExTextEditWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime, AspectRatio );
	
	m_BlinkTime += DeltaTime;
	if( m_BlinkTime > .5f )
	{
		m_BlinkTime = 0.f;
		m_bBlinkerOn = !m_bBlinkerOn;
		RefreshView();
	}
}

void ExTextEditWidget::OnFocusGained( eg_bool FromMouse, const eg_vec2& WidgetHitPoint )
{
	Super::OnFocusGained( FromMouse, WidgetHitPoint );

	if( !m_bIsFocused )
	{
		EGKbCharHandler::Get().AddListener( this , true );
	}

	RunEvent( eg_crc( "SelectBox" ) );

	m_bIsFocused = true;
	RefreshView();
}

void ExTextEditWidget::OnFocusLost()
{
	Super::OnFocusLost();

	if( m_bIsFocused )
	{
		EGKbCharHandler::Get().RemoveListener( this );
	}

	RunEvent( eg_crc( "Deselect" ) );

	m_bIsFocused = false;
	RefreshView();
}

eg_bool ExTextEditWidget::HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint, eg_bool bFromMouse )
{
	if( Event == eg_menuinput_t::BUTTON_PRIMARY )
	{
		EGArray<eg_loc_char> AllowCharsWithTerminal;
		AllowCharsWithTerminal = m_AllowedChars;
		AllowCharsWithTerminal.Append( '\0' );
		ExKeyboardMenu_PushMenu( GetOwnerMenu()->GetClientOwner(), AllowCharsWithTerminal.GetArray(), this, m_MaxLength, m_Text.GetArray() );
		return true;
	}

	return Super::HandleInput( Event, WidgetHitPoint, bFromMouse );
}

void ExTextEditWidget::InitWidget( const eg_loc_char* AllowedChars, eg_uint MaxLength )
{
	m_MaxLength = MaxLength;
	SetText( L"" );

	m_AllowedChars.Clear();
	const eg_loc_char* Itr = AllowedChars;
	while( *Itr )
	{
		m_AllowedChars.Append( *Itr );
		Itr++;
	}

	RefreshView();
}

void ExTextEditWidget::SetText( const eg_loc_char* NewText )
{
	m_Text.Clear();

	const eg_loc_char* Itr = NewText;
	while( *Itr )
	{
		m_Text.Append( *Itr );
		Itr++;
	}

	m_Text.Append( '\0' );

	RefreshView();
}

void ExTextEditWidget::GetText( eg_loc_char* Out, eg_size_t OutSize )
{
	if( m_Text.Len() > 0 && m_Text[m_Text.Len() - 1] == '\0' )
	{
		EGString_Copy( Out, m_Text.GetArray(), OutSize );
	}
	else
	{
		assert( false ); // Somehow the text became invalid
	}
}

eg_bool ExTextEditWidget::HasText() const
{
	return m_Text.Len() > 0 && m_Text[0] != '\0';
}

eg_bool ExTextEditWidget::HandleTypedChar( eg_char16 Char )
{
	assert( m_Text.Len() > 0 && m_Text[m_Text.Len() - 1] == '\0' );

	auto IsCharAllowed = [this]( eg_char16 Char ) -> eg_bool
	{
		for( eg_size_t i = 0; i<m_AllowedChars.Len(); i++ )
		{
			if( Char == m_AllowedChars[i] )
			{
				return true;
			}
		}

		return false;
	};

	eg_bool bHandled = false;

	eg_size_t NameLen = m_Text.Len() - 1;

	if( '\b' == Char )
	{
		if( NameLen > 0 )
		{
			m_Text.DeleteByIndex( NameLen );
			m_Text[NameLen - 1] = '\0';
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
			RefreshView();
		}
		bHandled = true;
	}
	else if( IsCharAllowed( Char ) )
	{
		if( NameLen < m_MaxLength )
		{
			m_Text[NameLen] = Char;
			m_Text.Append( '\0' );
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::INC_OR_DEC_ITEM );
		}
		RefreshView();
		bHandled = true;
	}

	return bHandled;
}

void ExTextEditWidget::OnTextFromKeyboardMenu( const eg_loc_char* String )
{
	SetText( String );
}

void ExTextEditWidget::RefreshView()
{
	assert( m_Text.Len() > 0 && m_Text[m_Text.Len() - 1] == '\0' );

	eg_bool bShowBlinker = m_bBlinkerOn && m_bIsFocused;

	Super::SetText( eg_crc( "Text" ), EGFormat( bShowBlinker ? L"{0}_" : L"{0}|SC(0,0,0,0)|_|RC()|", m_Text.GetArray() ) );
}

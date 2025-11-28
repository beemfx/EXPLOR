// (c) 2016 Beem Media

#include "EGLocText.h"

eg_loc_text::EGLocalizeFn eg_loc_text::s_PfnLocalize = nullptr;

eg_loc_text::eg_loc_text()
{
	m_LocText[0] = '\0';
	m_LocTextLen = 0;
}

eg_loc_text::eg_loc_text( eg_ctor_t Ct )
{
	if( Ct == CT_Preserve )
	{
		// If the text was in the dynamic array it is not preservable
		assert( !EGString_Equals( m_LocText , L"-D-" ) );
	}
	else
	{
		m_LocText[0] = '\0';
		m_LocTextLen = 0;
	}
}


eg_loc_text::eg_loc_text( const eg_loc_char* Str )
: eg_loc_text() 
{
	SetTextInternal( Str , EGString_StrLen( Str ) );
}

eg_loc_text::eg_loc_text( const eg_loc_char* Str, eg_size_t StrLen )
{
	SetTextInternal( Str , StrLen );
}

eg_loc_text::eg_loc_text( const eg_string_crc& StrKeyCrc )
{
	if( s_PfnLocalize )
	{
		*this = s_PfnLocalize( StrKeyCrc );
	}
	else
	{
		assert( false ); // Can't loclize text, function not set.
	}
}

eg_loc_text::~eg_loc_text()
{ 
	
}

void eg_loc_text::SetTextInternal( const eg_loc_char* Str, eg_size_t Length )
{
	m_LongTextArray.Clear();
	m_LocText[0] = L'\0';
	m_LocTextLen = 0;

	if( Length < countof(m_LocText) )
	{
		EGString_Copy( m_LocText , Str , Length+1 );
		m_LocText[Length] = L'\0';
		m_LocTextLen = Length;
	}
	else
	{
		EGString_Copy( m_LocText , L"-D-" , countof(m_LocText) );
		m_LongTextArray.Reserve( Length+1 );
		for( eg_size_t i=0; i<Length; i++ )
		{
			m_LongTextArray.Append( Str[i] );
		}
		m_LongTextArray.Append( L'\0' );
	}
}

// (c) 2017 Beem Media

#include "EGDynamicString.h"

//
// 8
//

template<>
void eg_d_string8::Set( eg_cpstr8 Str )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		m_Data.Resize( StrLen + 1 );
		EGString_Copy( m_Data.GetArray() , Str , m_Data.Len() );
	}
	else
	{
		m_Data.Clear( true );
	}
	assert( IsValid() );
}

template<>
void eg_d_string8::Set( eg_cpstr16 Str )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		m_Data.Resize( StrLen + 1 );
		EGString_Copy( m_Data.GetArray() , Str , m_Data.Len() );
	}
	else
	{
		m_Data.Clear( true );
	}
	assert( IsValid() );
}

template<>
void eg_d_string8::Append( const eg_char8* Str )
{
	const eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		if( m_Data.IsEmpty() )
		{
			m_Data.Append( '\0' );
		}
		m_Data.Resize( Len() + StrLen + 1 );
		EGString_StrCat( m_Data.GetArray() , m_Data.Len() , Str );
	}
	assert( IsValid() );
}

template<>
eg_bool eg_d_string8::operator < ( const eg_d_string8& rhs ) const
{
	return EGString_Compare( AsCStr() , rhs.AsCStr() ) < 0;
}

template<>
eg_bool eg_d_string8::Equals( eg_cpstr8 rhs ) const
{
	return EGString_Equals( AsCStr() , rhs );
}

template<>
eg_bool eg_d_string8::EqualsI( eg_cpstr8 rhs ) const
{
	return EGString_EqualsI( AsCStr() , rhs );
}

template<>
const eg_char8* eg_d_string8::AsCStr() const { assert(IsValid()); return m_Data.IsEmpty() ? "" : m_Data.GetArray(); }

template<>
eg_bool eg_d_string8::IsValid() const
{
	if( m_Data.IsEmpty() )
	{
		return true;
	}

	eg_bool bLastIsNull = m_Data.Len() > 0 && m_Data[m_Data.Len() - 1] == '\0';
	eg_bool bLengthIsCorrect = EGString_StrLen( m_Data.GetArray() ) == m_Data.Len()-1;
	return bLastIsNull && bLengthIsCorrect;
}

//
// 16
//

template<>
void eg_d_string16::Set( eg_cpstr8 Str )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		m_Data.Resize( StrLen + 1 );
		EGString_Copy( m_Data.GetArray() , Str , m_Data.Len() );
	}
	else
	{
		m_Data.Clear( true );
	}
	assert( IsValid() );
}

template<>
void eg_d_string16::Set( eg_cpstr16 Str )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		m_Data.Resize( StrLen + 1 );
		EGString_Copy( m_Data.GetArray() , Str , m_Data.Len() );
	}
	else
	{
		m_Data.Clear( true );
	}
	assert( IsValid() );
}

template<>
void eg_d_string16::Append( const eg_char16* Str )
{
	const eg_size_t StrLen = EGString_StrLen( Str );
	if( StrLen > 0 )
	{
		if( m_Data.IsEmpty() )
		{
			m_Data.Append( '\0' );
		}
		m_Data.Resize( Len() + StrLen + 1 );
		EGString_StrCat( m_Data.GetArray() , m_Data.Len() , Str );
	}
	assert( IsValid() );
}

template<>
eg_bool eg_d_string16::operator < ( const eg_d_string16& rhs ) const
{
	return EGString_Compare( AsCStr() , rhs.AsCStr() ) < 0;
}

template<>
eg_bool eg_d_string16::Equals( const eg_char16* rhs ) const
{
	return EGString_Equals( AsCStr() , rhs );
}

template<>
eg_bool eg_d_string16::EqualsI( const eg_char16* rhs ) const
{
	return EGString_EqualsI( AsCStr() , rhs );
}

template<>
const eg_char16* eg_d_string16::AsCStr() const { assert(IsValid()); return m_Data.IsEmpty() ? L"" : m_Data.GetArray(); }

template<>
eg_bool eg_d_string16::IsValid() const
{
	if( m_Data.IsEmpty() )
	{
		return true;
	}

	eg_bool bLastIsNull = m_Data.Len() > 0 && m_Data[m_Data.Len() - 1] == '\0';
	eg_bool bLengthIsCorrect = EGString_StrLen( m_Data.GetArray() ) == m_Data.Len()-1;
	return bLastIsNull && bLengthIsCorrect;
}

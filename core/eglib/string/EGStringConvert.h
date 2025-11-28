#pragma once

template< typename FROM , typename TO , eg_size_t BUFFER_SIZE = 1024>
class EGStringConvert
{
private:
	TO m_Internal[BUFFER_SIZE];
public:
	EGStringConvert( const FROM* In )
	{
		EGString_Copy( m_Internal , In , countof(m_Internal) );
	}

	~EGStringConvert()
	{
		#if defined( __DEBUG__ )
		m_Internal[0] = '\0';
		#endif
	}

	operator const TO*(){ return m_Internal; }
};

#define EGString_ToWide( _str_ ) EGStringConvert<eg_char8,eg_char16>( _str_ )
#define EGString_ToMultibyte( _str_ ) EGStringConvert<eg_char16,eg_char8>( _str_ )
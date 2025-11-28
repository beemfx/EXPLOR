// (c) 2017 Beem Media

#pragma once

template<typename CharType>
class eg_d_string_base
{
private:

	EGArray<CharType> m_Data;

public:

	eg_d_string_base(): m_Data( eg_mem_pool::String ) { Clear(); }
	eg_d_string_base( eg_ctor_t Ct ): eg_d_string_base() { unused( Ct ); assert( Ct == CT_Clear || Ct == CT_Default ); /* Cannot be preserved.*/ Clear(); }
	eg_d_string_base( const eg_d_string_base<CharType>& rhs ): eg_d_string_base() { m_Data = rhs.m_Data; }
	eg_d_string_base( eg_d_string_base<CharType>&& rhs ): eg_d_string_base() { m_Data = std::move(rhs.m_Data); assert( IsValid() ); }
	eg_d_string_base( eg_cpstr8 rhs ): eg_d_string_base() { Set( rhs ); }
	eg_d_string_base( eg_cpstr16 rhs ): eg_d_string_base() { Set( rhs ); }
	~eg_d_string_base() { Clear(); }

	const eg_d_string_base<CharType>& operator = ( const eg_d_string_base<CharType>& rhs ) { if( this == &rhs ) { return * this; } Set( *rhs ); return *this; }
	const eg_d_string_base<CharType>& operator = ( eg_d_string_base<CharType>&& rhs ) { if( this == &rhs ) { return *this; } m_Data = std::move(rhs.m_Data); assert( IsValid() ); return *this; }

	const eg_d_string_base<CharType>& operator = ( eg_cpstr8 rhs ) { Set( rhs ); return *this; }
	const eg_d_string_base<CharType>& operator = ( eg_cpstr16 rhs ) { Set( rhs ); return *this; }

	eg_bool operator == ( const eg_d_string_base<CharType>& rhs ) const { return Equals( *rhs ); }
	eg_bool operator != ( const eg_d_string_base<CharType>& rhs ) const { return !Equals( *rhs ); }
	eg_bool operator < ( const eg_d_string_base<CharType>& rhs ) const;
	CharType& operator [] ( eg_size_t Index ) { return m_Data[Index]; }
	const CharType& operator [] ( eg_size_t Index ) const { return m_Data[Index]; }
	
	eg_d_string_base<CharType> operator + ( const eg_d_string_base<CharType>& rhs ) const { eg_d_string_base<CharType> Out = **this; Out.Append( *rhs ); return Out; }
	eg_d_string_base<CharType> operator + ( const eg_cpstr8 rhs ) const { eg_d_string_base<CharType> Out = **this; Out.Append( eg_d_string_base<CharType>(rhs) ); return Out; }
	eg_d_string_base<CharType> operator + ( const eg_cpstr16 rhs ) const { eg_d_string_base<CharType> Out = **this; Out.Append( eg_d_string_base<CharType>(rhs) ); return Out; }
	const eg_d_string_base<CharType>& operator += ( const eg_d_string_base<CharType>& rhs ) { Append( rhs ); return *this; }
	const eg_d_string_base<CharType>& operator += ( const eg_cpstr8 rhs ) { Append( eg_d_string_base<CharType>(rhs) ); return *this; }
	const eg_d_string_base<CharType>& operator += ( const eg_cpstr16 rhs ) { Append( eg_d_string_base<CharType>(rhs) ); return *this; }

	eg_bool Equals( const CharType* rhs ) const;
	eg_bool EqualsI( const CharType* rhs ) const;

	void Clear() { m_Data.Clear(); }
	void Reserve( eg_size_t SizeToReserve ) { m_Data.Reserve( SizeToReserve ); }
	void Set( const eg_cpstr8 Str );
	void Set( const eg_cpstr16 Str );
	void Append( const eg_d_string_base<CharType>& rhs ) { Append( *rhs ); }
	void Append( const CharType* Str );
	void Append( const CharType Char ) { CharType AsStr[] = { Char , '\0' }; Append( AsStr ); }
	void ClampEnd( eg_size_t Count ){ if( Count >= m_Data.Len() ) { Clear(); } else { m_Data.Resize( m_Data.Len() - Count ); m_Data[m_Data.Len()-1] = '\0'; } }
	void Shrink( eg_size_t NewLength ) { if( NewLength < m_Data.Len() ) { m_Data.Resize( NewLength + 1 ); m_Data[NewLength] = '\0'; } }

	eg_size_t Len() const { assert( IsValid() ); return m_Data.IsEmpty() ? 0 : m_Data.Len() - 1; }
	template<typename RetType>	RetType LenAs() const { return static_cast<RetType>(Len()); }
	eg_bool IsValid() const;
	const CharType* AsCStr() const;
	const CharType* operator*() const { return AsCStr(); }
};

typedef eg_d_string_base<eg_char8> eg_d_string8;
typedef eg_d_string_base<eg_char16> eg_d_string16;
typedef eg_d_string8 eg_d_string;

// Special types for property editor to do different things with:
typedef eg_d_string eg_d_string_ml;       // Multiline editor
typedef eg_d_string eg_os_browse_file_ed; // Browse for OS file
typedef eg_d_string eg_os_browse_dir_ed;  // Browse for OS directory
typedef eg_d_string eg_button_ed;         // Button
typedef eg_d_string eg_label_ed;          // Label

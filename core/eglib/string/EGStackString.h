// (c) 2017 Beem Media

#pragma once

template<typename CharType,eg_size_t DataSize>
class eg_s_string_base
{
private:

	eg_size_t m_Len;
	CharType  m_Data[DataSize];

public:

	eg_s_string_base() { Clear(); }
	eg_s_string_base( eg_ctor_t Ct ){ if( Ct == CT_Clear || Ct == CT_Default ) { Clear(); } }
	eg_s_string_base( const eg_s_string_base<CharType,DataSize>& rhs ): eg_s_string_base() { Set( *rhs ); }
	eg_s_string_base( eg_s_string_base<CharType,DataSize>&& rhs ): eg_s_string_base() { Set( *rhs ); }
	eg_s_string_base( eg_cpstr8 rhs ): eg_s_string_base() { Set( rhs ); }
	eg_s_string_base( eg_cpstr16 rhs ): eg_s_string_base() { Set( rhs ); }
	~eg_s_string_base() { }

	const eg_s_string_base<CharType,DataSize>& operator = ( const eg_s_string_base<CharType,DataSize>& rhs ) { if( this == &rhs ) { return *this; } Set( *rhs ); return *this; }
	const eg_s_string_base<CharType,DataSize>& operator = ( eg_s_string_base<CharType,DataSize>&& rhs ) { if( this == &rhs ) { return *this; } Set( *rhs ); return *this; }

	const eg_s_string_base<CharType,DataSize>& operator = ( eg_cpstr8 rhs ) { Set( rhs ); return *this; }
	const eg_s_string_base<CharType,DataSize>& operator = ( eg_cpstr16 rhs ) { Set( rhs ); return *this; }

	eg_bool operator == ( const eg_s_string_base<CharType,DataSize>& rhs ) const { return Equals( *rhs ); }
	eg_bool operator != ( const eg_s_string_base<CharType,DataSize>& rhs ) const { return !Equals( *rhs ); }
	eg_bool operator < ( const eg_s_string_base<CharType,DataSize>& rhs ) const { return EGString_Compare( AsCStr() , *rhs ) < 0; }
	CharType& operator [] ( eg_size_t Index ) { return m_Data[Index]; }
	const CharType& operator [] ( eg_size_t Index ) const { return m_Data[Index]; }

	eg_s_string_base<CharType,DataSize> operator + ( const eg_s_string_base<CharType,DataSize>& rhs ) const { eg_s_string_base<CharType,DataSize> Out = AsCStr(); Out.Append( *rhs ); return Out; }
	eg_s_string_base<CharType,DataSize> operator + ( const eg_cpstr8 rhs ) const { eg_s_string_base<CharType,DataSize> Out = **this; Out.Append( eg_s_string_base<CharType,DataSize>( rhs ) ); return Out; }
	eg_s_string_base<CharType,DataSize> operator + ( const eg_cpstr16 rhs ) const { eg_s_string_base<CharType,DataSize> Out = **this; Out.Append( eg_s_string_base<CharType,DataSize>( rhs ) ); return Out; }
	const eg_s_string_base<CharType,DataSize>& operator += ( const eg_s_string_base<CharType,DataSize>& rhs ) { Append( rhs ); return *this; }
	const eg_s_string_base<CharType,DataSize>& operator += ( const eg_cpstr8 rhs ) { Append( eg_s_string_base<CharType,DataSize>( rhs ) ); return *this; }
	const eg_s_string_base<CharType,DataSize>& operator += ( const eg_cpstr16 rhs ) { Append( eg_s_string_base<CharType,DataSize>( rhs ) ); return *this; }

	eg_bool Equals( const CharType* rhs ) const { return EGString_Equals( AsCStr() , rhs ); }
	eg_bool EqualsI( const CharType* rhs ) const { return EGString_EqualsI( AsCStr() , rhs ); }

	void Clear() { m_Data[0] = '\0'; m_Len = 0; }
	void Set( const eg_cpstr8 Str ) { eg_size_t SrcLen = EGString_StrLen( Str ); assert( SrcLen < DataSize  ); /* SourceString bigger, will be clamped. */ m_Len = EG_Min( SrcLen , DataSize-1 ); EGString_Copy( m_Data , Str , countof(m_Data) ); assert( IsValid() ); }
	void Set( const eg_cpstr16 Str ) { eg_size_t SrcLen = EGString_StrLen( Str ); assert( SrcLen < DataSize  ); /* SourceString bigger, will be clamped. */ m_Len = EG_Min( SrcLen , DataSize-1 ); EGString_Copy( m_Data , Str , countof(m_Data) ); assert( IsValid() ); }
	void Append( const eg_s_string_base<CharType,DataSize>& rhs ) { Append( *rhs ); }
	void Append( const CharType* Str ) { eg_size_t SrcLen = EGString_StrLen( Str ); assert( (m_Len+SrcLen) < DataSize ); /* Makes string too big, will be clamped. */ EGString_StrCatCount( m_Data , countof(m_Data) , Str , EG_Min( SrcLen , DataSize-1-m_Len ) ); m_Len = EG_Min( m_Len + SrcLen , DataSize-1 ); assert( IsValid() ); }
	void Append( const CharType Char ) { CharType AsStr[] = { Char , '\0' }; Append( AsStr ); }

	eg_size_t Len() const { assert( IsValid() ); return m_Len; }
	template<typename RetType>	RetType LenAs() const { return static_cast<RetType>(Len()); }
	eg_bool IsValid() const { return m_Len == EGString_StrLen( m_Data ); }
	const CharType* AsCStr() const { return &m_Data[0]; }
	const CharType* operator*() const { return AsCStr(); }

	eg_size_t GetMaxLen() const { return DataSize - 1; }
	eg_size_t GetDataSize() const { return DataSize; }
};

typedef eg_s_string_base<eg_char8,1024> eg_s_string_big8;
typedef eg_s_string_base<eg_char8,128> eg_s_string_sml8;
typedef eg_s_string_base<eg_char16,1024> eg_s_string_big16;
typedef eg_s_string_base<eg_char16,128> eg_s_string_sml16;

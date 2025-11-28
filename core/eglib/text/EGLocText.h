// (c) 2016 Beem Media

#pragma once

class eg_loc_text
{
private:

	eg_loc_char          m_LocText[64];
	eg_size_t            m_LocTextLen;
	EGArray<eg_loc_char> m_LongTextArray;

	typedef eg_loc_text ( * EGLocalizeFn )( const eg_string_crc& );
	static EGLocalizeFn s_PfnLocalize;

public:

	static void SetLocFunction( EGLocalizeFn NewFn ){ s_PfnLocalize = NewFn; }

	eg_loc_text();
	eg_loc_text( eg_ctor_t Ct );
	explicit eg_loc_text( const eg_string_crc& StrKeyCrc );
	explicit eg_loc_text( const eg_loc_char* Str );
	explicit eg_loc_text( const eg_loc_char* Str , eg_size_t StrLen );
	~eg_loc_text();

	// const eg_loc_text& operator=( const eg_loc_text& rhs );

	const eg_loc_char* GetString() const { return m_LongTextArray.Len() > 0 ? m_LongTextArray.GetArray() : m_LocText; }
	eg_size_t GetLen() const { return m_LongTextArray.Len() > 0 ? m_LongTextArray.Len()-1 : m_LocTextLen; }

private:
	
	void SetTextInternal( const eg_loc_char* Str , eg_size_t Length );
};
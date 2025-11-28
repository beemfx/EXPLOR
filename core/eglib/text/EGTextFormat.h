// (c) 2014 Beem Media

#pragma once

#include "EGLocText.h"

class EGTextParmFormatter;

class IEGCustomFormatHandler
{
public:

	virtual void FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const = 0;
};

#include "EGTextFormatHelper.hpp"

eg_loc_text EGTextFormat_Format( eg_string_crc Format , EGTextParmHandler& Handler );
eg_loc_text EGTextFormat_Format( const eg_loc_char* Format , EGTextParmHandler& Handler );

static inline eg_loc_text EGFormat( eg_string_crc Format ){ return EGTextFormat_Format( Format , EGTextParmHandler() ); }
static inline eg_loc_text EGFormat( eg_string_crc Format , eg_loc_parm p1 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 ) ); }
static inline eg_loc_text EGFormat( eg_string_crc Format , eg_loc_parm p1 , eg_loc_parm p2 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 ) ); }
static inline eg_loc_text EGFormat( eg_string_crc Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 ) ); }
static inline eg_loc_text EGFormat( eg_string_crc Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 , p4 ) ); }
static inline eg_loc_text EGFormat( eg_string_crc Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 , eg_loc_parm p5 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 , p4 , p5 ) ); }
static inline eg_loc_text EGFormat( const eg_loc_char* Format , eg_loc_parm p1 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 ) ); }
static inline eg_loc_text EGFormat( const eg_loc_char* Format , eg_loc_parm p1 , eg_loc_parm p2 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 ) ); }
static inline eg_loc_text EGFormat( const eg_loc_char* Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 ) ); }
static inline eg_loc_text EGFormat( const eg_loc_char* Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 , p4 ) ); }
static inline eg_loc_text EGFormat( const eg_loc_char* Format , eg_loc_parm p1 , eg_loc_parm p2 , eg_loc_parm p3 , eg_loc_parm p4 , eg_loc_parm p5 ){ return EGTextFormat_Format( Format , EGTextParmHandler( p1 , p2 , p3 , p4 , p5 ) ); }

class EGTextParmFormatter
{
private:

	eg_loc_text m_RawText2;

public:

	EGTextParmFormatter();
	void SetText( eg_cpstr8 StrText );
	void SetText( eg_cpstr16 StrText );
	void SetNumber( eg_real RealNumber , eg_cpstr FormatFlags = nullptr );
	void SetNumber( eg_uint UIntNumber , eg_cpstr FormatFlags = nullptr );
	void SetNumber( eg_int IntNumber , eg_cpstr FormatFlags = nullptr );
	void SetBigNumber( eg_int64 IntNumber , eg_cpstr FormatFlags = nullptr );
	void CopyTo( eg_loc_text& Target );
	static eg_string_crc GetNextFlag( eg_cpstr* Flags );
	static eg_int GetNextFlagAsInt( eg_cpstr* Flags );

private:

	static void printfcommai( eg_int64 n , eg_string& Out );
	static void printfcommau( eg_uint64 n , eg_string& Out );
};
// (c) 2014 Beem Media

#include "EGTextFormat.h"

EGTextParmFormatter::EGTextParmFormatter()
: m_RawText2( CT_Clear )
{
}

void EGTextParmFormatter::SetText( eg_cpstr8 StrText )
{
	m_RawText2 = eg_loc_text( *eg_d_string16( StrText ) );
}

void EGTextParmFormatter::SetText( eg_cpstr16 StrText )
{
	m_RawText2 = eg_loc_text( StrText );
}

void EGTextParmFormatter::SetNumber( eg_real RealNumber , eg_cpstr FormatFlags )
{
	unused( FormatFlags );

	eg_real Sign = RealNumber >= 0 ? 1.f : -1.f;
	eg_real AbsNumber = EG_Abs(RealNumber);
	eg_int NumberPart = EGMath_floor( AbsNumber );
	eg_real DecimalPart = AbsNumber - NumberPart;
	eg_string Str( CT_Clear );
	if( Sign < 0.f )
	{
		Str.Append( '-' );
	}
	printfcommai(NumberPart,Str);
	eg_string DecimalStr = EGString_Format( "%0.2f" , DecimalPart );
	Str.Append( &(DecimalStr.String()[1]) );
	SetText( Str );
}

void EGTextParmFormatter::SetNumber( eg_uint UIntNumber , eg_cpstr FormatFlags )
{
	eg_string Str( CT_Clear );
	if( eg_crc("nocomma") == GetNextFlag(&FormatFlags) )
	{
		Str = EGString_Format( "%d" , UIntNumber );
	}
	else
	{
		printfcommau( UIntNumber, Str );
	}
	SetText( Str );
}

void EGTextParmFormatter::SetNumber( eg_int IntNumber , eg_cpstr FormatFlags )
{
	eg_string Str( CT_Clear );
	if( eg_crc( "nocomma" ) == GetNextFlag( &FormatFlags ) )
	{
		Str = EGString_Format( "%d" , IntNumber );
	}
	else
	{
		printfcommai( IntNumber, Str );
	}
	SetText( Str );
}

void EGTextParmFormatter::SetBigNumber( eg_int64 IntNumber, eg_cpstr FormatFlags /*= nullptr */ )
{
	unused( FormatFlags );

	eg_string Str( CT_Clear );
	printfcommai( IntNumber , Str );
	SetText( Str );
}

void EGTextParmFormatter::printfcommai( eg_int64 n, eg_string& Out )
{
	if (n < 0) 
	{
		Out.Append( '-' );
		n  = -n;
	}
	printfcommau( static_cast<eg_uint64>(n) , Out );
}

void EGTextParmFormatter::printfcommau( eg_uint64 n , eg_string& Out )
{
	if (n < 1000) 
	{
		Out.Append( EGString_Format("%d", n) );
		return;
	}

	printfcommau( n/1000 , Out );
	Out.Append( EGString_Format(",%03d", n%1000) );
}

void EGTextParmFormatter::CopyTo( eg_loc_text& Target )
{
	Target = m_RawText2;
}

eg_string_crc EGTextParmFormatter::GetNextFlag( eg_cpstr* Flags )
{
	eg_s_string_sml8 Flag;
	while( Flags && *Flags && 0 != **Flags )
	{
		eg_char c = **Flags;
		(*Flags)++;
		if( c == ':' )
		{
			return eg_string_crc(*Flag);
		}
		else
		{
			Flag.Append( c );
		}
	}
	return eg_string_crc(*Flag);
}

eg_int EGTextParmFormatter::GetNextFlagAsInt( eg_cpstr* Flags )
{
	eg_s_string_sml8 Flag;
	while( Flags && *Flags && 0 != **Flags )
	{
		eg_char c = **Flags;
		(*Flags)++;
		if( c == ':' )
		{
			return EGString_ToInt(*Flag);
		}
		else
		{
			Flag.Append( c );
		}
	}
	return EGString_ToInt(*Flag);
}

static void EGTextFormat_FormatString( eg_loc_text& OutText , const eg_loc_char* Format , eg_size_t FormatLen , EGTextParmHandler& Handler )
{
	eg_size_t ReadPos = 0;

	eg_d_string16 OutTextRaw; // TODO: Some optimization so we don't allocate here.
	OutTextRaw.Reserve( 256 );

	auto ReadFormatString = [&Format,&FormatLen]( eg_size_t ReadPos , eg_string_base& FormatString ) -> eg_size_t
	{
		for( eg_size_t i=ReadPos; i<FormatLen; i++ )
		{
			if( Format[i] == '}' )
			{
				return FormatString.Len()+1;
			}
			else
			{
				FormatString.Append( static_cast<eg_char>(Format[i]) );
			}
		}
		assert( false ); // Text formatter missing '}'.
		return 0;
	};

	auto ProcessFormatString = [&OutTextRaw,&Handler]( const eg_string& InFormatString ) -> void
	{
		eg_string NumberPart( CT_Clear );
		eg_cpstr Flags = "";
		for( eg_size_t i=0; i<InFormatString.Len(); i++ )
		{
			if( InFormatString[i] == ':' )
			{
				Flags = &InFormatString[i+1];
				break;
			}
			else
			{
				NumberPart += InFormatString[i];
			}
		}
		eg_uint ParmIndex = InFormatString.ToUInt();
		EGTextParmFormatter Formatter;
		Handler.FormatParm( ParmIndex , Flags , &Formatter ); 
		eg_loc_text LocText;
		Formatter.CopyTo( LocText );
		OutTextRaw.Append( LocText.GetString() );
	};

	for( ; ReadPos<FormatLen; ReadPos++ )
	{
		eg_loc_char c = Format[ReadPos];
		if( c == '{' )
		{
			eg_string FormatString( CT_Clear );
			ReadPos += ReadFormatString( ReadPos+1 , FormatString );
			if( FormatString.Len() > 0 )
			{
				ProcessFormatString( FormatString );			
				continue;
			}
		}

		OutTextRaw.Append( c );
	}

	// Add zero to play nice with things that want null-terminated text.
	OutText = eg_loc_text( *OutTextRaw );

	return;
}

static void EGTextFormat_FormatString( eg_loc_text& OutText , eg_string_crc Format , EGTextParmHandler& Handler )
{
	eg_loc_text LocalizedFormat( Format );
	EGTextFormat_FormatString(  OutText , LocalizedFormat.GetString() , LocalizedFormat.GetLen() , Handler );
}

eg_loc_text EGTextFormat_Format( eg_string_crc Format , EGTextParmHandler& Handler )
{
	eg_loc_text OutText( CT_Clear );
	EGTextFormat_FormatString( OutText , Format , Handler );
	return OutText;
}

eg_loc_text EGTextFormat_Format( const eg_loc_char* Format , EGTextParmHandler& Handler )
{
	eg_loc_text LocalizedFormat( CT_Clear );
	EGTextFormat_FormatString( LocalizedFormat , Format ,  EGString_StrLen(Format) , Handler );
	return LocalizedFormat;
}

void EGTextParmHandler::FormatParm( eg_uint Index , eg_cpstr Flags , class EGTextParmFormatter* Formatter )
{
	if( 0 <= Index && Index < countof( m_Parms ) )
	{
		switch( m_Parms[Index].GetType() )
		{
		case eg_loc_parm::eg_p_t::T_CRC:
		{
			Formatter->SetText( eg_loc_text( m_Parms[Index].as_crc() ).GetString() );
		} break;
		case eg_loc_parm::eg_p_t::T_INT:
		{
			Formatter->SetNumber( m_Parms[Index].as_int() , Flags );
		} break;
		case eg_loc_parm::eg_p_t::T_REAL:
		{
			Formatter->SetNumber( m_Parms[Index].as_real() , Flags );
		} break;
		case eg_loc_parm::eg_p_t::T_UINT:
		{	
			Formatter->SetNumber( m_Parms[Index].as_uint() , Flags );
		} break;
		case eg_loc_parm::eg_p_t::T_BOOL:
		{
			Formatter->SetText( eg_loc_text( m_Parms[Index].as_bool() ? eg_loc("EGTextTrue","True") : eg_loc("EGTextFalse","False") ).GetString() );
		} break;
		case eg_loc_parm::eg_p_t::T_INT64:
		{
			Formatter->SetBigNumber( m_Parms[Index].as_int64() , Flags );
		} break;
		case eg_loc_parm::eg_p_t::T_UINT64:
		{
			Formatter->SetBigNumber( m_Parms[Index].as_uint64() , Flags );
		} break;
		case eg_loc_parm::eg_p_t::T_LOCSTR:
		{
			Formatter->SetText( m_Parms[Index].as_str() );
		} break;
		case eg_loc_parm::eg_p_t::T_HANDLER:
		{
			const IEGCustomFormatHandler* Handler = m_Parms[Index].as_handler();
			if( Handler )
			{
				Handler->FormatText( Flags , Formatter );
			}
			else
			{
				Formatter->SetText( L"{HANDLED}" );
			}
		} break;
		}
	}
	else
	{
		assert( false ); // Need to add support for more parms.
	}
}

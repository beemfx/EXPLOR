// (c) 2019 Beem Media

#include "EGStringFormat.h"

struct egsformat_parm_collection;

static void EGSFormat_FormatString( eg_d_string16& OutText , eg_cpstr16 Format , eg_size_t FormatLen , const egsformat_parm_collection& Parms );

//////////////////////////////////////////////////////////////////////////

struct egsformat_parm_collection
{
	const egsformat_parm& p0;
	const egsformat_parm& p1;
	const egsformat_parm& p2;
	const egsformat_parm& p3;
	const egsformat_parm& p4;
	const egsformat_parm& p5;
	const egsformat_parm& p6;
	const egsformat_parm& p7;
	const egsformat_parm NullParm;

	egsformat_parm_collection( const egsformat_parm& InP0 , const egsformat_parm& InP1 , const egsformat_parm& InP2 , const egsformat_parm& InP3 , const egsformat_parm& InP4 , const egsformat_parm& InP5 , const egsformat_parm& InP6 , const egsformat_parm& InP7 )
	: p0( InP0 )
	, p1( InP1 )
	, p2( InP2 )
	, p3( InP3 )
	, p4( InP4 )
	, p5( InP5 )
	, p6( InP6 )
	, p7( InP7 )
	, NullParm()
	{
	}

	void FormatParm( eg_uint Index , eg_cpstr16 Flags , EGSParmFormatter& Formatter ) const
	{
		const egsformat_parm& Parm = GetParmByIndex( Index );
		switch( Parm.GetType() )
		{
		case egsformat_parm::eg_t::T_Null:
		{
			Formatter.SetText( L"(null)" , Flags );
		} break;
		case egsformat_parm::eg_t::T_Int:
		{
			Formatter.SetNumber( Parm.as_int(), Flags );
		} break;
		case egsformat_parm::eg_t::T_UInt:
		{
			Formatter.SetNumber( Parm.as_uint(), Flags );
		} break;
		case egsformat_parm::eg_t::T_Int64:
		{
			Formatter.SetBigNumber( Parm.as_int64(), Flags );
		} break;
		case egsformat_parm::eg_t::T_UInt64:
		{
			Formatter.SetBigNumber( Parm.as_uint64(), Flags );
		} break;
		case egsformat_parm::eg_t::T_Real:
		{
			Formatter.SetNumber( Parm.as_real(), Flags );
		} break;
		case egsformat_parm::eg_t::T_Bool:
		{
			Formatter.SetBool( Parm.as_bool() , Flags );
		} break;
		case egsformat_parm::eg_t::T_StringCrc:
		{
			Formatter.SetCrc( Parm.as_crc() , Flags );		
		} break;
		case egsformat_parm::eg_t::T_PtrStr8:
		{
			Formatter.SetText( Parm.as_pstr8() , Flags );
		} break;
		case egsformat_parm::eg_t::T_PtrStr16:
		{
			Formatter.SetText( Parm.as_pstr16() , Flags );
		} break;
		case egsformat_parm::eg_t::T_Custom:
		{
			const IEGSFormatHandler* Handler = Parm.as_CustomHandler();
			if( Handler )
			{
				Handler->FormatText( Flags, Formatter );
			}
			else
			{
				Formatter.SetText( L"{HANDLED}" , Flags );
			}
		} break;
		}
	}

private:

	const egsformat_parm& GetParmByIndex( eg_uint Index ) const
	{
		switch( Index )
		{
		case 0: return p0;
		case 1: return p1;
		case 2: return p2;
		case 3: return p3;
		case 4: return p4;
		case 5: return p5;
		case 6: return p6;
		case 7: return p7;
		default:
			break;
		}

		assert( false ); // Need to add support for more parms.
		return NullParm;
	}
};

//////////////////////////////////////////////////////////////////////////

eg_d_string8 EGSFormat8( eg_cpstr8 Format, const egsformat_parm& p0 /*= egsformat_parm()*/, const egsformat_parm& p1 /*= egsformat_parm()*/, const egsformat_parm& p2 /*= egsformat_parm()*/, const egsformat_parm& p3 /*= egsformat_parm()*/, const egsformat_parm& p4 /*= egsformat_parm()*/, const egsformat_parm& p5 /*= egsformat_parm() */, const egsformat_parm& p6 /*= egsformat_parm() */ , const egsformat_parm& p7 /*= egsformat_parm() */  )
{
	eg_d_string16 Out = EGSFormat16( *eg_d_string16(Format) , p0 , p1 , p2 , p3 , p4 , p5 , p6 , p7 );
	return *Out;
}

eg_d_string16 EGSFormat16( eg_cpstr16 Format, const egsformat_parm& p0 /*= egsformat_parm()*/, const egsformat_parm& p1 /*= egsformat_parm()*/, const egsformat_parm& p2 /*= egsformat_parm()*/, const egsformat_parm& p3 /*= egsformat_parm()*/, const egsformat_parm& p4 /*= egsformat_parm()*/, const egsformat_parm& p5 /*= egsformat_parm() */, const egsformat_parm& p6 /*= egsformat_parm() */ , const egsformat_parm& p7 /*= egsformat_parm() */  )
{
	eg_d_string16 Out;
	EGSFormat_FormatString( Out , Format , EGString_StrLen( Format ) , egsformat_parm_collection(p0,p1,p2,p3,p4,p5,p6,p7) );
	return Out;
}

//////////////////////////////////////////////////////////////////////////

static void EGSFormat_FormatString( eg_d_string16& OutText , eg_cpstr16 Format , eg_size_t FormatLen , const egsformat_parm_collection& Parms )
{
	eg_size_t ReadPos = 0;

	auto ReadFormatString = [&Format,&FormatLen]( eg_size_t ReadPos , eg_s_string_sml16& FormatString ) -> eg_size_t
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

	auto ProcessFormatString = [&OutText,&Parms]( const eg_s_string_sml16& InFormatString ) -> void
	{
		eg_s_string_sml16 NumberPart( CT_Clear );
		eg_cpstr16 Flags = L"";
		for( eg_size_t i=0; i<InFormatString.Len(); i++ )
		{
			if( InFormatString[i] == ':' )
			{
				Flags = &InFormatString[i+1];
				break;
			}
			else
			{
				NumberPart.Append( InFormatString[i] );
			}
		}
		const eg_uint ParmIndex = EGString_ToUInt( *InFormatString );
		EGSParmFormatter Formatter;
		Parms.FormatParm( ParmIndex , Flags , Formatter );
		OutText.Append( *Formatter.GetString() );
	};

	eg_bool bNextIsEscape = false;
	for( ; ReadPos<FormatLen; ReadPos++ )
	{
		const eg_char16 c = Format[ReadPos];
		if( bNextIsEscape )
		{
			OutText.Append( c );
			bNextIsEscape = false;
		}
		else if( c == '\\' )
		{
			bNextIsEscape = true;
		}
		else
		{
			if( c == '{' )
			{
				eg_s_string_sml16 FormatString( CT_Clear );
				ReadPos += ReadFormatString( ReadPos+1 , FormatString );
				if( FormatString.Len() > 0 )
				{
					ProcessFormatString( FormatString );			
					continue;
				}
			}

			OutText.Append( c );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void EGSParmFormatter::SetText( eg_cpstr8 InValue , eg_cpstr16 FormatFlags )
{
	unused( FormatFlags );
	m_String = InValue;
}

void EGSParmFormatter::SetText( eg_cpstr16 InValue , eg_cpstr16 FormatFlags )
{
	unused( FormatFlags );
	m_String = InValue;
}

void EGSParmFormatter::SetNumber( eg_real InValue, eg_cpstr16 FormatFlags )
{
	eg_string_crc SpecialFormat = GetNextFlag( &FormatFlags );
	if( SpecialFormat == eg_crc("PRETTY") )
	{
		eg_real Sign = InValue >= 0 ? 1.f : -1.f;
		eg_real AbsNumber = EG_Abs( InValue );
		eg_int NumberPart = EGMath_floor( AbsNumber );
		eg_real DecimalPart = AbsNumber - NumberPart;
		eg_s_string_sml8 Str( CT_Clear );
		if( Sign < 0.f )
		{
			Str.Append( '-' );
		}
		printfcommai( NumberPart, Str );
		eg_s_string_sml8 DecimalStr = SysNumberToStr( DecimalPart , L"%0.2g" );
		Str.Append( &( (*DecimalStr)[1] ) );
		SetText( *Str , nullptr );
	}
	else
	{
		SetText( *SysNumberToStr(InValue,L"%g") , nullptr );
	}
}

void EGSParmFormatter::SetNumber( eg_uint InValue , eg_cpstr16 FormatFlags )
{
	eg_string_crc SpecialFormat = GetNextFlag( &FormatFlags );
	if( SpecialFormat == eg_crc("PRETTY") )
	{
		eg_s_string_sml8 Str;
		printfcommau( InValue, Str );
		SetText( *Str , nullptr );
	}
	else
	{
		SetText( *SysNumberToStr(InValue,L"%u") , nullptr );
	}
}

void EGSParmFormatter::SetNumber( eg_int InValue, eg_cpstr16 FormatFlags )
{
	eg_string_crc SpecialFormat = GetNextFlag( &FormatFlags );
	if( SpecialFormat == eg_crc("PRETTY") )
	{
		eg_s_string_sml8 Str;
		printfcommai( InValue, Str );
		SetText( *Str, nullptr );
	}
	else
	{
		SetText( *SysNumberToStr( InValue, L"%d" ), nullptr );
	}
}

void EGSParmFormatter::SetBigNumber( eg_int64 InValue, eg_cpstr16 FormatFlags )
{
	eg_string_crc SpecialFormat = GetNextFlag( &FormatFlags );
	if( SpecialFormat == eg_crc( "BYTESIZE" ) )
	{
		eg_int64 AdjustedValue = InValue;
		eg_cpstr8 Suffix = "bytes";
		eg_s_string_sml8 Str;

		if( InValue >= 1024*1024*1024 )
		{
			AdjustedValue = InValue/(1024*1024*1024);
			Suffix = "GB";
		}
		else if( InValue >= 1024*1024 )
		{
			AdjustedValue = InValue / (1024*1024);
			Suffix = "MB";
		}
		else if( InValue >= 1024 )
		{
			AdjustedValue = InValue / (1024);
			Suffix = "KB";
		}

		printfcommai( AdjustedValue , Str );
		Str.Append( " " );
		Str.Append( Suffix );
		SetText( *Str, nullptr );
	}
	else
	{
		eg_s_string_sml8 Str;
		printfcommai( InValue, Str );
		SetText( *Str, nullptr );
	}
}

void EGSParmFormatter::SetCrc( const eg_string_crc& InValue, eg_cpstr16 FormatFlags )
{
	unused( FormatFlags );
	SetText( *SysNumberToStr( InValue.ToUint32() , L"CRC:%08X" ) , nullptr );
}

void EGSParmFormatter::SetBool( eg_bool InValue, eg_cpstr16 FormatFlags )
{
	unused( FormatFlags );
	SetText( InValue ? L"true" : L"false" , nullptr );
}

eg_string_crc EGSParmFormatter::GetNextFlag( eg_cpstr16* Flags )
{
	eg_s_string_sml16 Flag;
	while( Flags && *Flags && 0 != **Flags )
	{
		eg_char16 c = **Flags;
		( *Flags )++;
		if( c == ':' )
		{
			return eg_string_crc( *Flag );
		}
		else
		{
			Flag.Append( c );
		}
	}
	return eg_string_crc( *Flag );
}

void EGSParmFormatter::printfcommai( eg_int64 n , eg_s_string_sml8& Out )
{
	if( n < 0 )
	{
		Out.Append( '-' );
		n = -n;
	}
	printfcommau( static_cast<eg_uint64>( n ), Out );
}

void EGSParmFormatter::printfcommau( eg_uint64 n , eg_s_string_sml8& Out )
{
	if( n < 1000 )
	{
		Out.Append( *SysNumberToStr( static_cast<eg_uint>(n) , L"%u" ) );
		return;
	}

	printfcommau( n/1000 , Out );
	Out.Append( *SysNumberToStr( static_cast<eg_uint>(n%1000) , L",%03d" ) );
}

eg_s_string_sml8 EGSParmFormatter::SysNumberToStr( eg_real InValue , eg_cpstr16 Format )
{
	eg_char16 ValueBuffer[128];
	swprintf_s( ValueBuffer, countof( ValueBuffer ), Format, InValue );
	return ValueBuffer;
}

eg_s_string_sml8 EGSParmFormatter::SysNumberToStr( eg_uint InValue , eg_cpstr16 Format )
{
	eg_char16 ValueBuffer[128];
	swprintf_s( ValueBuffer, countof( ValueBuffer ), Format , InValue  );
	return ValueBuffer;
}

eg_s_string_sml8 EGSParmFormatter::SysNumberToStr( eg_int InValue , eg_cpstr16 Format )
{
	eg_char16 ValueBuffer[128];
	swprintf_s( ValueBuffer, countof( ValueBuffer ), Format , InValue );
	return ValueBuffer;
}

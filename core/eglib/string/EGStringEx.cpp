// (c) 2015 Beem Media

#include "EGStringEx.h"
#include "EGBase64.h"
#include "EGStdLibAPI.h"

eg_string_big EGString_Format( eg_cpstr8 strFormat , ... )
{
	eg_string_big sTemp;
	va_list arglist = nullptr;
	va_start(arglist,strFormat);
	eg_char8 Temp[1024];
	_vsnprintf_s(Temp, countof(Temp), _TRUNCATE, strFormat, arglist);
	Temp[countof(Temp)-1] = 0;
	va_end(arglist);
	sTemp = Temp;
	return sTemp;
}

eg_string_big EGString_Format( eg_cpstr16 strFormat , ... )
{
	eg_string_big sTemp;
	va_list arglist = nullptr;
	va_start(arglist,strFormat);
	eg_char16 Temp[1024];
	_vsnwprintf_s(Temp, countof(Temp), _TRUNCATE, strFormat, arglist);
	Temp[countof(Temp)-1] = 0;
	va_end(arglist);
	sTemp = Temp;
	return sTemp;
}

void EGString_FormatToBuffer( eg_pstr8 Out , eg_size_t OutSize , eg_cpstr8 strFormat , ... )
{
	va_list arglist = nullptr;
	va_start(arglist,strFormat);
	_vsnprintf_s(Out, OutSize, _TRUNCATE, strFormat, arglist);
	Out[OutSize-1] = 0;
	va_end(arglist);
}

void EGString_FormatToBuffer( eg_pstr16 Out , eg_size_t OutSize , eg_cpstr16 strFormat , ... )
{
	va_list arglist = nullptr;
	va_start(arglist,strFormat);
	_vsnwprintf_s(Out, OutSize, _TRUNCATE, strFormat, arglist);
	Out[OutSize-1] = 0;
	va_end(arglist);
}

eg_uint EGString_GetOccurencesOfSubstr( eg_cpstr String , eg_cpstr SubStr )
{
	eg_size_t Len = EGString_StrLen( String );
	eg_size_t SubStrLen = EGString_StrLen( SubStr );

	eg_uint Count = 0;

	for( eg_uint i=0; i<Len; i++ )
	{
		if( !EGString_CompareCount(&String[i] , SubStr , SubStrLen ) )
		{
			Count++;	
		}
	}

	return Count;
}

eg_string_big EGString_ToFloatList( const void* Floats, eg_size_t NumFloats, eg_bool bBase64 /*= false */ )
{
	eg_string_big Out = CT_Clear;

	const eg_real* AsRealArray = reinterpret_cast<const eg_real*>(Floats);

	if( AsRealArray )
	{
		if( bBase64 )
		{
			return *EGBase64_Encode( AsRealArray, sizeof( eg_real )*NumFloats);
		}
		else
		{
			for( eg_size_t i=0; i<NumFloats;  i++ )
			{
				Out.Append( EGString_Format("%g" , AsRealArray[i] ) );
				if( i != (NumFloats-1) )
				{
					Out.Append( ' ' );
				}
			}
		}
	}

	return Out;
}

void EGString_GetFloatList(eg_cpstr str, eg_size_t nStrLen, void* pDestV, eg_uint nNumFloats, eg_bool bBase64)
{
	//if nStrLen was specified as zero, find it out.
	if(!nStrLen)
		nStrLen = EGString_StrLen(str);

	eg_real* pDest = reinterpret_cast<eg_real*>(pDestV);

	if(bBase64)
	{
		//For base64 just use the helper function.
		EGBase64_Decode( str, pDest, sizeof(eg_real)*nNumFloats);
	}
	else
	{
		//For text, it is necessary to tokenize:
		eg_char strDel[]=(" \n\r\t,;");

		eg_char strTemp[1024];
		EGString_Copy(strTemp, str, countof(strTemp));
		//Just tokenize the string, and insert it in the matrix
		//going row by row.
		eg_string_big strToken;
		eg_char* ct;
		eg_char* tok = EGString_StrTok(strTemp, strDel, &ct);
		eg_uint nPos=0;
		while(tok && nPos<nNumFloats)
		{
			strToken=tok;
			pDest[nPos++]=strToken.ToFloat();
			tok=EGString_StrTok(nullptr, strDel, &ct);
		}
	}
}

void EGString_GetIntList(eg_cpstr str, eg_size_t nStrLen, void* pDestV, eg_uint nNumFloats, eg_bool bBase64)
{
	//if nStrLen was specified as zero, find it out.
	if(!nStrLen)
		nStrLen = EGString_StrLen(str);

	eg_int* pDest = reinterpret_cast<eg_int*>(pDestV);

	if(bBase64)
	{
		//For base64 just use the helper function.
		EGBase64_Decode( str, pDest, sizeof(eg_int)*nNumFloats);
	}
	else
	{
		//For text, it is necessary to tokenize:
		eg_char strDel[]=(" \n\r\t,;");

		eg_char strTemp[1024];
		EGString_Copy(strTemp, str, countof(strTemp));
		//Just tokenize the string, and insert it in the matrix
		//going row by row.
		eg_string_big strToken;
		eg_char* ct;
		eg_char* tok = EGString_StrTok(strTemp, strDel, &ct);
		eg_uint nPos=0;
		while(tok && nPos<nNumFloats)
		{
			strToken=tok;
			pDest[nPos++]=strToken.ToInt();
			tok=EGString_StrTok(nullptr, strDel, &ct);
		}
	}
}

eg_string_big EGString_ToXmlFriendly( eg_cpstr In )
{
	eg_string_big Out;
	eg_string_big InAsStr( In );
	eg_string_base::ToXmlFriendly( InAsStr , Out );
	return Out;
}

void EGStringEx_ReplaceAll( eg_d_string8& Target , eg_cpstr8 SearchValue , eg_cpstr8 NewValue )
{
	eg_d_string8 FinalStr;

	eg_cpstr8 SourceStr = *Target;
	const eg_size_t SourceLen = Target.Len();
	const eg_size_t SearchValueLen = EGString_StrLen( SearchValue );
	const eg_size_t NewValueLen = EGString_StrLen( NewValue );

	eg_d_string8 CurrentBuffer;

	for( eg_size_t i=0; i<SourceLen; i++ )
	{
		if( EGString_EqualsCount( &SourceStr[i] , SearchValue , SearchValueLen ) )
		{
			// Found a match!
			FinalStr += CurrentBuffer;
			CurrentBuffer = "";
			FinalStr += NewValue;
			
			i += (SearchValueLen-1);
		}
		else
		{
			CurrentBuffer.Append( SourceStr[i] );
		}
	}

	FinalStr += CurrentBuffer;
	CurrentBuffer = "";

	Target = std::move( FinalStr );
}

void EGStringEx_ReplaceAll( eg_d_string16& Target , eg_cpstr16 SearchValue , eg_cpstr16 NewValue )
{
	eg_d_string16 FinalStr;

	eg_cpstr16 SourceStr = *Target;
	const eg_size_t SourceLen = Target.Len();
	const eg_size_t SearchValueLen = EGString_StrLen( SearchValue );
	const eg_size_t NewValueLen = EGString_StrLen( NewValue );

	eg_d_string16 CurrentBuffer;

	for( eg_size_t i=0; i<SourceLen; i++ )
	{
		if( EGString_EqualsCount( &SourceStr[i] , SearchValue , SearchValueLen ) )
		{
			// Found a match!
			FinalStr += CurrentBuffer;
			CurrentBuffer = "";
			FinalStr += NewValue;
			
			i += (SearchValueLen-1);
		}
		else
		{
			CurrentBuffer.Append( SourceStr[i] );
		}
	}

	FinalStr += CurrentBuffer;
	CurrentBuffer = "";

	Target = std::move( FinalStr );
}

void EGStringEx_TrimPadding( eg_d_string8& Target )
{
	eg_d_string8 FinalStr;
	eg_d_string8 WhiteTempStr;

	FinalStr.Reserve( Target.Len() );

	eg_bool bFoundFirstNonWhitespace = false;
	eg_size_t LastWhitespaceFound = -1;

	for( eg_size_t i=0; i<Target.Len(); i++ )
	{
		const eg_char8 c = Target[i];
		const eg_bool bIsWhiteSpace = EGStringEx_IsWhiteSpace( c );

		if( bIsWhiteSpace )
		{
			LastWhitespaceFound = i;
		}
		
		if( !bFoundFirstNonWhitespace && !bIsWhiteSpace )
		{
			bFoundFirstNonWhitespace = true;
		}

		if( bFoundFirstNonWhitespace )
		{
			if( !bIsWhiteSpace )
			{
				FinalStr.Append( WhiteTempStr );
				WhiteTempStr.Clear();
				FinalStr.Append( c );
			}
			else
			{
				WhiteTempStr.Append( c );
			}
		}
	}

	Target = std::move( FinalStr );
}

void EGStringEx_TrimPadding( eg_d_string16& Target )
{
	eg_d_string16 FinalStr;
	eg_d_string16 WhiteTempStr;

	FinalStr.Reserve( Target.Len() );

	eg_bool bFoundFirstNonWhitespace = false;
	eg_size_t LastWhitespaceFound = -1;

	for( eg_size_t i=0; i<Target.Len(); i++ )
	{
		const eg_char16 c = Target[i];
		const eg_bool bIsWhiteSpace = EGStringEx_IsWhiteSpace( c );

		if( bIsWhiteSpace )
		{
			LastWhitespaceFound = i;
		}

		if( !bFoundFirstNonWhitespace && !bIsWhiteSpace )
		{
			bFoundFirstNonWhitespace = true;
		}

		if( bFoundFirstNonWhitespace )
		{
			if( !bIsWhiteSpace )
			{
				FinalStr.Append( WhiteTempStr );
				WhiteTempStr.Clear();
				FinalStr.Append( c );
			}
			else
			{
				WhiteTempStr.Append( c );
			}
		}
	}

	Target = std::move( FinalStr );
}

eg_d_string8 EGStringEx_ToXmlFriendly( eg_cpstr String )
{
	eg_d_string8 Out;

	static const struct egMapping
	{
		eg_char c;
		eg_cpstr Value;
	}
	Mapping[] =
	{
		{ '\"' , "&quot;" } ,
		{ '\xA9' , "&#xA9;" } ,
		{ '\n' , "&#xA;" } ,
		{ '\r' , "&#xD;" } ,
		{ '<' , "&lt;" } ,
		{ '>' , "&gt;" } ,
		{ '&' , "&amp;" } ,
		{ '\'' , "&apos;" } ,
		{ '\t' , "&#x9;" } ,
		// { ' '  , "&#x20;" },
	};

	auto AppendChar = [&Out]( eg_char c ) -> void
	{
		eg_bool bSpecial = false;

		for( const egMapping& Item : Mapping )
		{
			if( Item.c == c )
			{
				Out.Append( Item.Value );
				bSpecial = true;
				break;
			}
		}

		if( !bSpecial )
		{
			Out.Append( c );
		}
	};

	const eg_size_t StrLen = EGString_StrLen( String );

	for( eg_size_t i = 0; i < StrLen; i++ )
	{
		AppendChar( String[i] );
	}

	return Out;
}

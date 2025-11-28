/******************************************************************************
EG String Extension Library - Additional functionality for strings.
(c) 2015 Beem Media
******************************************************************************/
#pragma once

eg_string_big EGString_Format( eg_cpstr8 strFormat , ... );
eg_string_big EGString_Format( eg_cpstr16 strFormat , ... );
void EGString_FormatToBuffer( eg_pstr8 Out , eg_size_t OutSize , eg_cpstr8 strFormat , ... );
void EGString_FormatToBuffer( eg_pstr16 Out , eg_size_t OutSize , eg_cpstr16 strFormat , ... );

eg_uint EGString_GetOccurencesOfSubstr( eg_cpstr String , eg_cpstr SubStr );
eg_string_big EGString_ToFloatList( const void* Floats , eg_size_t NumFloats , eg_bool bBase64 = false );
void EGString_GetFloatList(eg_cpstr str, eg_size_t nStrLen, void* pDestV, eg_uint nNumFloats, eg_bool bBase64=false);
void EGString_GetIntList(eg_cpstr str, eg_size_t nStrLen, void* pDestV, eg_uint nNumFloats, eg_bool bBase64=false);
eg_string_big EGString_ToXmlFriendly( eg_cpstr In );

template<typename StrType,typename CharType>
static inline EGArray<StrType> EGString_Split( const CharType* StringIn , CharType Separator , eg_size_t MaxSplits = 0 )
{
	EGArray<StrType> Out;

	if( StringIn )
	{
		StrType CurrentString;

		const eg_size_t StringInLen = EGString_StrLen( StringIn );
		for( eg_size_t i = 0; i < StringInLen; i++ )
		{
			const CharType c = StringIn[i];

			if( c == Separator && (MaxSplits == 0 || Out.Len() < MaxSplits) )
			{
				if( CurrentString.Len() > 0 )
				{
					Out.Append( CurrentString );
					CurrentString = "";
				}
			}
			else
			{
				CurrentString.Append( c );
			}
		}

		if( CurrentString.Len() > 0 )
		{
			Out.Append( CurrentString );
			CurrentString = "";
		}
	}

	return Out;
}

static inline eg_bool EGStringEx_IsWhiteSpace( eg_char16 c ) { return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'); }

void EGStringEx_ReplaceAll( eg_d_string8& Target , eg_cpstr8 SearchValue , eg_cpstr8 NewValue );
void EGStringEx_ReplaceAll( eg_d_string16& Target , eg_cpstr16 SearchValue , eg_cpstr16 NewValue );

void EGStringEx_TrimPadding( eg_d_string8& Target );
void EGStringEx_TrimPadding( eg_d_string16& Target );

eg_d_string8 EGStringEx_ToXmlFriendly( eg_cpstr String );

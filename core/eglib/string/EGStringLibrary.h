/******************************************************************************
EGStringFunctions
String manipulation library

(c) 2015 Beem Media
******************************************************************************/
#pragma once

#define EGSTRING_FUNCTION 

EGSTRING_FUNCTION void EGString_Copy(eg_pstr8 Dest, eg_cpstr8 Src, eg_size_t DestSize);
EGSTRING_FUNCTION void EGString_Copy(eg_pstr8 Dest, eg_cpstr16 Src, eg_size_t DestSize);
EGSTRING_FUNCTION void EGString_Copy(eg_pstr16 Dest, eg_cpstr8 Src, eg_size_t DestSize);
EGSTRING_FUNCTION void EGString_Copy(eg_pstr16 Dest, eg_cpstr16 Src, eg_size_t DestSize);
EGSTRING_FUNCTION eg_bool EGString_Contains(eg_cpstr8 Str, eg_cpstr8 SubStr);
EGSTRING_FUNCTION eg_bool EGString_Contains(eg_cpstr16 Str, eg_cpstr16 SubStr);
EGSTRING_FUNCTION eg_size_t EGString_StrLen(eg_cpstr8 Str);
EGSTRING_FUNCTION eg_size_t EGString_StrLen(eg_cpstr16 Str);
EGSTRING_FUNCTION eg_int EGString_Compare(eg_cpstr8 s1, eg_cpstr8 s2);
EGSTRING_FUNCTION eg_int EGString_Compare(eg_cpstr16 s1, eg_cpstr16 s2);
EGSTRING_FUNCTION eg_int EGString_CompareCount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_int EGString_CompareCount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_int EGString_CompareI(eg_cpstr8 s1, eg_cpstr8 s2);
EGSTRING_FUNCTION eg_int EGString_CompareI(eg_cpstr16 s1, eg_cpstr16 s2);
EGSTRING_FUNCTION eg_int EGString_CompareICount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_int EGString_CompareICount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_bool EGString_Equals(eg_cpstr8 s1, eg_cpstr8 s2);
EGSTRING_FUNCTION eg_bool EGString_Equals(eg_cpstr16 s1, eg_cpstr16 s2);
EGSTRING_FUNCTION eg_bool EGString_EqualsI(eg_cpstr8 s1, eg_cpstr8 s2);
EGSTRING_FUNCTION eg_bool EGString_EqualsI(eg_cpstr16 s1, eg_cpstr16 s2);
EGSTRING_FUNCTION eg_bool EGString_EqualsCount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_bool EGString_EqualsCount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_bool EGString_EqualsICount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count);
EGSTRING_FUNCTION eg_bool EGString_EqualsICount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count);
EGSTRING_FUNCTION void EGString_ToUpper(eg_pstr8 Str, eg_size_t Count);
EGSTRING_FUNCTION void EGString_ToUpper(eg_pstr16 Str, eg_size_t Count);
EGSTRING_FUNCTION void EGString_ToLower(eg_pstr8 Str, eg_size_t Count);
EGSTRING_FUNCTION void EGString_ToLower(eg_pstr16 Str, eg_size_t Count);
EGSTRING_FUNCTION eg_real EGString_ToReal( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_real EGString_ToReal( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_int EGString_ToInt( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_int EGString_ToInt( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_uint EGString_ToUInt( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_uint EGString_ToUInt( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_uint EGString_ToUIntFromHex( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_uint EGString_ToUIntFromHex( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_pstr8 EGString_StrTok( eg_pstr8 Str , eg_cpstr8 Deliminators , eg_pstr8* Context );
EGSTRING_FUNCTION eg_pstr16 EGString_StrTok( eg_pstr16 Str , eg_cpstr16 Deliminators , eg_pstr16* Context );
EGSTRING_FUNCTION eg_cpstr8 EGString_StrStr( eg_cpstr8 Str , eg_cpstr8 Search );
EGSTRING_FUNCTION eg_cpstr16 EGString_StrStr( eg_cpstr16 Str , eg_cpstr16 Search );
EGSTRING_FUNCTION const eg_char8* EGString_StrChr( eg_cpstr8 Str , eg_char8 Search );
EGSTRING_FUNCTION const eg_char16* EGString_StrChr( eg_cpstr16 Str , eg_char16 Search );
EGSTRING_FUNCTION void EGString_StrCat( eg_pstr8 Str , eg_size_t StrSize , eg_cpstr8 AppendStr );
EGSTRING_FUNCTION void EGString_StrCat( eg_pstr16 Str, eg_size_t StrSize , eg_cpstr16 AppendStr );
EGSTRING_FUNCTION void EGString_StrCatCount( eg_pstr8 Str , eg_size_t StrSize , eg_cpstr8 AppendStr , eg_size_t Count );
EGSTRING_FUNCTION void EGString_StrCatCount( eg_pstr16 Str, eg_size_t StrSize , eg_cpstr16 AppendStr , eg_size_t Count );
EGSTRING_FUNCTION eg_bool EGString_EndsWith( eg_cpstr8 Str , eg_cpstr8 End );
EGSTRING_FUNCTION eg_bool EGString_EndsWith( eg_cpstr16 Str , eg_cpstr16 End );
EGSTRING_FUNCTION eg_bool EGString_EndsWithI( eg_cpstr8 Str , eg_cpstr8 End );
EGSTRING_FUNCTION eg_bool EGString_EndsWithI( eg_cpstr16 Str , eg_cpstr16 End );
EGSTRING_FUNCTION eg_bool EGString_BeginsWith( eg_cpstr8 Str , eg_cpstr8 Begin );
EGSTRING_FUNCTION eg_bool EGString_BeginsWith( eg_cpstr16 Str , eg_cpstr16 Begin );
EGSTRING_FUNCTION eg_bool EGString_BeginsWithI( eg_cpstr8 Str , eg_cpstr8 Begin );
EGSTRING_FUNCTION eg_bool EGString_BeginsWithI( eg_cpstr16 Str , eg_cpstr16 Begin );
EGSTRING_FUNCTION eg_int EGString_CountOccurencesOf( eg_cpstr8 Str , eg_cpstr8 c );
EGSTRING_FUNCTION eg_int EGString_CountOccurencesOf( eg_cpstr16 Str , eg_cpstr16 c );
EGSTRING_FUNCTION eg_bool EGString_IsInteger( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_bool EGString_IsInteger( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_bool EGString_IsNumber( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_bool EGString_IsNumber( eg_cpstr16 Str );
EGSTRING_FUNCTION eg_bool EGString_ToBool( eg_cpstr8 Str );
EGSTRING_FUNCTION eg_bool EGString_ToBool( eg_cpstr16 Str );

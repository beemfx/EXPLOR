#include "EGStringLibrary.h"
#include <stdlib.h>
#include <string.h>

static const eg_char8 UNICODE_DOWNCAST_CHAR = ' ';

void EGString_Copy(eg_pstr8 Dest, eg_cpstr8 Src, eg_size_t DestSize)
{
	if(0 == DestSize)return;

	for(eg_size_t i=0; i<DestSize; i++)
	{
		Dest[i]=Src[i];
		if(Dest[i]==0)
			break;
	}
	Dest[DestSize-1]=0;
}

void EGString_Copy(eg_pstr8 Dest, eg_cpstr16 Src, eg_size_t DestSize)
{
	const eg_size_t MAX = 256;
	if(nullptr == Dest || nullptr == Src || 0 == DestSize)return;

	for(eg_size_t i=0; i<DestSize; i++, Src++)
	{
		eg_char16 c = *Src;
		if( c>= MAX )
		{
			c = UNICODE_DOWNCAST_CHAR;
		}
		Dest[i] = static_cast<eg_char8>(c);

		if( '\0' == Dest[i])
			break;
	}
	Dest[DestSize-1] = '\0';
}

void EGString_Copy(eg_pstr16 Dest, eg_cpstr8 Src, eg_size_t DestSize)
{
	if(nullptr == Dest || nullptr == Src || 0 == DestSize)return;

	for(eg_size_t i=0; i<DestSize; i++, Src++)
	{
		unsigned char c = static_cast<unsigned char>(*Src);
		Dest[i] = static_cast<eg_char16>(c);

		if( '\0' == Dest[i])
			break;
	}
	Dest[DestSize-1] = '\0';
}

void EGString_Copy(eg_pstr16 Dest, eg_cpstr16 Src, eg_size_t DestSize)
{
	if(nullptr == Dest || nullptr == Src || 0 == DestSize)return;

	for(eg_size_t i=0; i<DestSize; i++)
	{
		Dest[i]=Src[i];
		if(Dest[i]==0)
			break;
	}
	Dest[DestSize-1]=0;
}

eg_bool EGString_Contains(eg_cpstr8 Str, eg_cpstr8 SubStr)
{
	return nullptr != EGString_StrStr( Str , SubStr );
}

eg_bool EGString_Contains(eg_cpstr16 Str, eg_cpstr16 SubStr)
{
	return nullptr != EGString_StrStr( Str , SubStr );
}

eg_size_t EGString_StrLen(eg_cpstr8 Str)
{
	return Str ? strlen(Str) : 0;
}

eg_size_t EGString_StrLen(eg_cpstr16 Str)
{
	return Str ? wcslen(Str) : 0;
}

eg_int EGString_Compare(eg_cpstr8 s1, eg_cpstr8 s2)
{
	return strcmp(s1,s2);
}

eg_int EGString_Compare(eg_cpstr16 s1, eg_cpstr16 s2)
{
	return wcscmp(s1,s2);
}

eg_int EGString_CompareCount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count)
{
	return strncmp(s1,s2,Count);
}

eg_int EGString_CompareCount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count)
{
	return wcsncmp(s1,s2,Count);
}

eg_int EGString_CompareI(eg_cpstr8 s1, eg_cpstr8 s2)
{
	return _stricmp(s1,s2);
}

eg_int EGString_CompareI(eg_cpstr16 s1, eg_cpstr16 s2)
{
	return _wcsicmp(s1,s2);
}

eg_int EGString_CompareICount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count)
{
	return _strnicmp(s1,s2,Count);
}

eg_int EGString_CompareICount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count)
{
	return _wcsnicmp(s1,s2,Count);
}

eg_bool EGString_Equals(eg_cpstr8 s1, eg_cpstr8 s2)
{
	return !EGString_Compare(s1, s2);
}

eg_bool EGString_Equals(eg_cpstr16 s1, eg_cpstr16 s2)
{
	return !EGString_Compare(s1, s2);
}

eg_bool EGString_EqualsI(eg_cpstr8 s1, eg_cpstr8 s2)
{
	return !EGString_CompareI(s1, s2);
}

eg_bool EGString_EqualsI(eg_cpstr16 s1, eg_cpstr16 s2)
{
	return !EGString_CompareI(s1, s2);
}

eg_bool EGString_EqualsCount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count)
{
	return !EGString_CompareCount(s1, s2, Count);
}

eg_bool EGString_EqualsCount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count)
{
	return !EGString_CompareCount(s1, s2, Count);
}

eg_bool EGString_EqualsICount(eg_cpstr8 s1, eg_cpstr8 s2, eg_size_t Count)
{
	return !EGString_CompareICount(s1, s2, Count);
}

eg_bool EGString_EqualsICount(eg_cpstr16 s1, eg_cpstr16 s2, eg_size_t Count)
{
	return !EGString_CompareICount(s1, s2, Count);
}

void EGString_ToUpper(eg_pstr8 Str, eg_size_t Count)
{
	_strupr_s(Str, Count);
}

void EGString_ToUpper(eg_pstr16 Str, eg_size_t Count)
{
	_wcsupr_s(Str, Count);
}

void EGString_ToLower(eg_pstr8 Str, eg_size_t Count)
{
	_strlwr_s(Str, Count);
}

void EGString_ToLower(eg_pstr16 Str, eg_size_t Count)
{
	_wcslwr_s(Str, Count);
}

eg_real EGString_ToReal( eg_cpstr8 Str )
{
	return static_cast<eg_real>(atof(Str));
}

eg_real EGString_ToReal( eg_cpstr16 Str )
{
	return static_cast<eg_real>(_wtof(Str));
}

eg_int EGString_ToInt( eg_cpstr8 Str )
{
	return atol(Str);
}

eg_int EGString_ToInt( eg_cpstr16 Str )
{
	return _wtol(Str);
}

eg_uint EGString_ToUInt( eg_cpstr8 Str )
{
	return static_cast<eg_uint>(strtoul(Str,nullptr,10));
}

eg_uint EGString_ToUInt( eg_cpstr16 Str )
{
	return static_cast<eg_uint>(wcstoul(Str,nullptr,10));
}

eg_uint EGString_ToUIntFromHex( eg_cpstr8 Str )
{
	eg_char8 c = 0;
	eg_uint nHexValue = 0;
	eg_uint SkipChars = 2;


	if(Str[0]!='0' || (Str[1]!='x' && Str[1]!='X'))
	{
		SkipChars=0;
	}

	// Keep adding to value, until we reach an invalid character, or the end of the string.
	for(eg_uint i=SkipChars; Str[i]!='\0'; i++)
	{
		c=Str[i];

		if(c>='0' && c<='9')
			nHexValue=(nHexValue<<4)|(c-'0');
		else if(c>='a' && c<='f')
			nHexValue=(nHexValue<<4)|(c-'a'+10);
		else if(c>='A' && c<='F')
			nHexValue=(nHexValue<<4)|(c-'A'+10);
		else
			break;
	}

	return nHexValue;
}

eg_uint EGString_ToUIntFromHex( eg_cpstr16 Str )
{
	eg_char16 c = 0;
	eg_uint nHexValue = 0;
	eg_uint SkipChars = 2;


	if(Str[0]!='0' || (Str[1]!='x' && Str[1]!='X'))
	{
		SkipChars=0;
	}

	// Keep adding to value, until we reach an invalid character, or the end of the string.
	for(eg_uint i=SkipChars; Str[i]!='\0'; i++)
	{
		c=Str[i];

		if(c>='0' && c<='9')
			nHexValue=(nHexValue<<4)|(c-'0');
		else if(c>='a' && c<='f')
			nHexValue=(nHexValue<<4)|(c-'a'+10);
		else if(c>='A' && c<='F')
			nHexValue=(nHexValue<<4)|(c-'A'+10);
		else
			break;
	}

	return nHexValue;
}

eg_pstr8 EGString_StrTok( eg_pstr8 Str , eg_cpstr8 Deliminators , eg_pstr8* Context )
{
	return strtok_s( Str , Deliminators , Context );
}

eg_pstr16 EGString_StrTok( eg_pstr16 Str , eg_cpstr16 Deliminators , eg_pstr16* Context )
{
	return wcstok_s( Str , Deliminators , Context );
}

eg_cpstr8 EGString_StrStr( eg_cpstr8 Str , eg_cpstr8 Search )
{
	return strstr( Str , Search );
}

eg_cpstr16 EGString_StrStr( eg_cpstr16 Str , eg_cpstr16 Search )
{
	return wcsstr( Str , Search );
}

const eg_char8* EGString_StrChr( eg_cpstr8 Str , eg_char8 Search )
{
	return strchr( Str , Search );
}

const eg_char16* EGString_StrChr( eg_cpstr16 Str , eg_char16 Search )
{
	return wcschr( Str , Search );
}

void EGString_StrCat( eg_pstr8 Str, eg_size_t StrSize , eg_cpstr8 AppendStr )
{
	strcat_s( Str , StrSize , AppendStr );
}

void EGString_StrCat( eg_pstr16 Str, eg_size_t StrSize , eg_cpstr16 AppendStr )
{
	wcscat_s( Str , StrSize , AppendStr );
}

void EGString_StrCatCount( eg_pstr8 Str , eg_size_t StrSize , eg_cpstr8 AppendStr , eg_size_t Count )
{
	strncat_s( Str , StrSize , AppendStr , Count );
}

void EGString_StrCatCount( eg_pstr16 Str, eg_size_t StrSize , eg_cpstr16 AppendStr , eg_size_t Count )
{
	wcsncat_s( Str , StrSize , AppendStr , Count );
}

eg_bool EGString_EndsWith( eg_cpstr8 Str , eg_cpstr8 End )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	eg_size_t EndLen = EGString_StrLen( End );

	if( EndLen > StrLen )
	{
		return false;
	}

	return EGString_Equals( &Str[StrLen-EndLen] , End );
}

eg_bool EGString_EndsWith( eg_cpstr16 Str , eg_cpstr16 End )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	eg_size_t EndLen = EGString_StrLen( End );

	if( EndLen > StrLen )
	{
		return false;
	}

	return EGString_Equals( &Str[StrLen-EndLen] , End );
}

eg_bool EGString_EndsWithI( eg_cpstr8 Str , eg_cpstr8 End )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	eg_size_t EndLen = EGString_StrLen( End );

	if( EndLen > StrLen )
	{
		return false;
	}

	return EGString_EqualsI( &Str[StrLen-EndLen] , End );
}

eg_bool EGString_EndsWithI( eg_cpstr16 Str , eg_cpstr16 End )
{
	eg_size_t StrLen = EGString_StrLen( Str );
	eg_size_t EndLen = EGString_StrLen( End );

	if( EndLen > StrLen )
	{
		return false;
	}

	return EGString_EqualsI( &Str[StrLen-EndLen] , End );
}


eg_bool EGString_BeginsWith( eg_cpstr8 Str , eg_cpstr8 Begin )
{
	return EGString_EqualsCount( Str , Begin , EGString_StrLen(Begin) );
}

eg_bool EGString_BeginsWith( eg_cpstr16 Str , eg_cpstr16 Begin )
{
	return EGString_EqualsCount( Str , Begin , EGString_StrLen(Begin) );
}

eg_bool EGString_BeginsWithI( eg_cpstr8 Str , eg_cpstr8 Begin )
{
	return EGString_EqualsICount( Str , Begin , EGString_StrLen(Begin) );
}

eg_bool EGString_BeginsWithI( eg_cpstr16 Str , eg_cpstr16 Begin )
{
	return EGString_EqualsICount( Str , Begin , EGString_StrLen(Begin) );
}

eg_int EGString_CountOccurencesOf( eg_cpstr8 Str , eg_cpstr8 c )
{
	eg_int Out = 0;

	eg_size_t CompareLen = EGString_StrLen( c );

	for( eg_uint i=0; Str[i] != '\0'; i++ )
	{
		if( EGString_EqualsCount( &Str[i] ,  c , CompareLen ) )
		{
			Out++;
		}
	}

	return Out;
}

eg_int EGString_CountOccurencesOf( eg_cpstr16 Str , eg_cpstr16 c )
{
	eg_int Out = 0;

	eg_size_t CompareLen = EGString_StrLen( c );

	for( eg_uint i=0; Str[i] != '\0'; i++ )
	{
		if( EGString_EqualsCount( &Str[i] ,  c , CompareLen ) )
		{
			Out++;
		}
	}

	return Out;
}

eg_bool EGString_IsInteger( eg_cpstr8 Str )
{
	const eg_char OkayList[] = ("0123456789");
	eg_uint nPos = 0;
	if( Str == nullptr || Str[0] == '\0' ) return false;

	if(Str[nPos] == '-' && Str[nPos+1] != '\0' )
	{
		nPos++;
	}

	for( ; ; nPos++)
	{
		eg_bool bOkay = false;
		for(eg_uint i=0; i<countof(OkayList); i++)
		{
			if(OkayList[i] == Str[nPos])
			{
				bOkay = true;
				break;
			}
		}

		if( Str[nPos] == '\0' )
		{
			break;
		}

		if(!bOkay)return false;
	}

	return true;
}

eg_bool EGString_IsInteger( eg_cpstr16 Str )
{
	const eg_char OkayList[] = ("0123456789");
	eg_uint nPos = 0;
	if( Str == nullptr || Str[0] == '\0' ) return false;

	if(Str[nPos] == '-' && Str[nPos+1] != '\0' )
	{
		nPos++;
	}

	for( ; ; nPos++)
	{
		eg_bool bOkay = false;
		for(eg_uint i=0; i<countof(OkayList); i++)
		{
			if(OkayList[i] == Str[nPos])
			{
				bOkay = true;
				break;
			}
		}

		if( Str[nPos] == '\0' )
		{
			break;
		}

		if(!bOkay)return false;
	}

	return true;
}

eg_bool EGString_IsNumber( eg_cpstr8 Str )
{
	const eg_char OkayList[] = ("0123456789.");
	eg_uint nPos = 0;
	if( nullptr == Str || Str[0] == '\0' )return false;

	if( Str[nPos] == '-' && Str[nPos+1] != '\0' )
	{
		nPos++;
	}

	for( ; ; nPos++ )
	{
		eg_bool bOkay = false;

		for(eg_uint i=0; i<countof(OkayList); i++)
		{
			if(OkayList[i] == Str[nPos])
			{
				bOkay = true;
				break;
			}
		}

		if( Str[nPos] == '\0' )
		{
			break;
		}

		if(!bOkay)return false;
	}

	return true;
}

eg_bool EGString_IsNumber( eg_cpstr16 Str )
{
	const eg_char OkayList[] = ("0123456789.");
	eg_uint nPos = 0;
	if( nullptr == Str || Str[0] == '\0' )return false;

	if( Str[nPos] == '-' && Str[nPos+1] != '\0' )
	{
		nPos++;
	}

	for( ; ; nPos++ )
	{
		eg_bool bOkay = false;

		for(eg_uint i=0; i<countof(OkayList); i++)
		{
			if(OkayList[i] == Str[nPos])
			{
				bOkay = true;
				break;
			}
		}

		if( Str[nPos] == '\0' )
		{
			break;
		}

		if(!bOkay)return false;
	}

	return true;
}

eg_bool EGString_ToBool( eg_cpstr8 Str )
{
	eg_bool Out = false;

	if( EGString_EqualsI( Str , "true" ) || EGString_EqualsI( Str , "1" ) )
	{
		Out = true;
	}
	else if( EGString_EqualsI( Str , "false" ) || EGString_EqualsI( Str , "0" ) )
	{
		Out = false;
	}
	else if( EGString_IsNumber( Str ) )
	{
		Out = EGString_ToInt( Str ) != 0;
	}

	return Out;
}

eg_bool EGString_ToBool( eg_cpstr16 Str )
{
	eg_bool Out = false;

	if( EGString_EqualsI( Str , L"true" ) || EGString_EqualsI( Str , L"1" ) )
	{
		Out = true;
	}
	else if( EGString_EqualsI( Str , L"false" ) || EGString_EqualsI( Str , L"0" ) )
	{
		Out = false;
	}
	else if( EGString_IsNumber( Str ) )
	{
		Out = EGString_ToInt( Str ) != 0;
	}

	return Out;
}
/******************************************************************************
eg_string_base, eg_string_fixed_size, and eg_string
(c) 2011 Beem Media
******************************************************************************/
#include "EGString.h"
#include "EGPath2.h"

#define __EGSTRING_GETDEBUGINFO__ false

#if __EGSTRING_GETDEBUGINFO__
#include "EGMutex.h"
#endif


#if __EGSTRING_GETDEBUGINFO__
eg_int  eg_string_base::s_StringCount    = 0;
eg_int  eg_string_base::s_MaxStringCount = 0;
eg_uint eg_string_base::s_MaxStringLen   = 0;
static EGMutex* g_pStrLock = nullptr;
#endif

EG_INLINE void eg_string_base::CommonConstruct()
{
#if __EGSTRING_GETDEBUGINFO__
	static EGMutex StrLock;
	if( nullptr == g_pStrLock )
	{
		g_pStrLock = &StrLock;
	}
	g_pStrLock->Lock();
	s_StringCount++;
	s_MaxStringCount = EG_Max(s_StringCount, s_MaxStringCount);
	g_pStrLock->Unlock();
#endif
}

EG_INLINE void eg_string_base::CommonDestruct()
{
#if __EGSTRING_GETDEBUGINFO__
	assert( g_pStrLock );
	if( g_pStrLock )
	{
		g_pStrLock->Lock();
		s_StringCount--;
		assert(s_StringCount >= 0);
		//We'll go with the string length at the end since usually that is when
		//a string reaches it's maximum.
		s_MaxStringLen = EG_Max(s_MaxStringLen, m_nLength);
		g_pStrLock->Unlock();
	}
#endif
}


void eg_string_base::UpdateLength()
{
	m_nLength=static_cast<eg_uint>(EGString_StrLen(m_strString));
}

void eg_string_base::Set( const eg_color& rhs )
{
	Set( EGString_Format( "%g %g %g %g" , rhs.r , rhs.g , rhs.b , rhs.a ) );
}

void eg_string_base::Append( eg_cpstr str )
{
	if(str)
	{
		while( *str && (m_nLength < m_strStringSize) )
		{
			Append( *str );
			str++;
		}

		assert( '\0' == *str ); //Couldn't fully append string.
	}
}

void eg_string_base::Append(const eg_string_base& str)
{
	Append( str.String() );
}

void eg_string_base::Append(const eg_char c)
{
	if(m_nLength>=(m_strStringSize-1))
	{
		assert( false ); //String cannot be this long.
		return;
	}
		
	m_strString[m_nLength]=c;
	m_nLength++;
	m_strString[m_nLength]=0;
}

void eg_string_base::AppendSpaces(const eg_uint count)
{
	for(eg_uint i=0; i<count; i++)
	{
		Append(' ');
	}
}

void eg_string_base::ClampEnd(eg_uint nCount)
{
	nCount=((nCount)<(m_nLength))?(nCount):(m_nLength);//EG_Min(nCount, m_nLength);
	m_strString[m_nLength-nCount]=0;
	m_nLength=m_nLength-nCount;
}

void eg_string_base::ClampTo(eg_uint NewLength)
{
	if( NewLength <= m_nLength && NewLength < m_strStringSize )
	{
		m_strString[NewLength]=0;
		m_nLength = NewLength;
	}
}

void eg_string_base::AddSlashes()
{
	static const eg_char eg_string_base_SLASH_CHARS[] = { '"' , '\\' };

	//Add any slashes necessary so that this can be a string parameter of EGPars.
	eg_char TempStringData[1024];
	eg_string_base TempString( TempStringData , countof(TempStringData) );

	for( eg_size_t i=0; i<m_nLength; i++ )
	{
		for( eg_size_t s=0; s<countof(eg_string_base_SLASH_CHARS); s++ )
		{
			if( m_strString[i] == eg_string_base_SLASH_CHARS[s] )
			{
				TempString.Append( '\\' );
				if( m_strString[i] == '\\' ) // Add another set of slashes for the parse if this is a pars.
				{
					TempString.Append( '\\' );
					TempString.Append( '\\' );
				}
			}
		}

		TempString.Append( m_strString[i] );
	}

	*this = TempString;
}

void eg_string_base::RemoveSlashes()
{
	//Remove any slashes since we got this as a string parameter of egparse
	eg_char TempStringData[1024];
	eg_string_base TempString( TempStringData , countof(TempStringData) );

	for( eg_size_t i=0; i<m_nLength; i++ )
	{
		eg_bool NeedsSlash = false;

		if( m_strString[i] == '\\' && m_strString[i+1] != '\0' )
		{
			i++;
		}

		TempString.Append( m_strString[i] );
	}

	*this = TempString;
}

eg_bool eg_string_base::IsNumber()const
{
	return EGString_IsNumber( m_strString );
}

eg_bool eg_string_base::IsInteger() const
{
	return EGString_IsInteger( m_strString );
}

eg_bool eg_string_base::ToBool() const
{
	return EGString_ToBool( m_strString );
}

eg_uint eg_string_base::ToUIntArray( eg_uint* anOut, const eg_uint nMaxNums )const
{
	//For text, it is necessary to tokenize:
	eg_char strDel[]=(" \n\r\t,;");
				
	eg_char strTemp[2048];
	CopyTo( strTemp , countof(strTemp) );
	//Just tokenize the string, and insert into the list
	eg_char strTokenMem[512];
	eg_string_base strToken( strTokenMem , countof(strTokenMem) );
	eg_char* ct;
	eg_char* tok = EGString_StrTok(strTemp, strDel, &ct);
	eg_uint* pDest = anOut;
	eg_uint nPos=0;
	while(tok && nPos<nMaxNums)
	{
		strToken.Set(tok);
		pDest[nPos++]=strToken.ToUInt();
		tok=EGString_StrTok(nullptr, strDel, &ct);
	}

	return nPos;
}
eg_uint eg_string_base::ToRealArray(eg_real* Out , eg_uint MaxOut )const
{
	//For text, it is necessary to tokenize:
	eg_char strDel[]=(" \n\r\t,;");

	eg_char strTemp[2048];
	CopyTo( strTemp , countof(strTemp) );
	//Just tokenize the string, and insert into the list
	eg_char strTokenMem[512];
	eg_string_base strToken( strTokenMem , countof(strTokenMem) );
	eg_char* ct;
	eg_char* tok = EGString_StrTok(strTemp, strDel, &ct);
	eg_real* pDest = Out;
	eg_uint nPos=0;
	while(tok && nPos<MaxOut)
	{
		strToken.Set(tok);
		pDest[nPos++]=strToken.ToFloat();
		tok=EGString_StrTok(nullptr, strDel, &ct);
	}

	return nPos;
}

void eg_string_base::SetToFilenameFromPathNoExt( eg_cpstr Path )
{
	eg_string_big OutTemp = Path; //Use temp variable so both inputs can be the same thing!
	eg_uint Len = OutTemp.Len();

	for( eg_uint i=0; i<Len; i++ )
	{
		if( Path[i] == '/' || Path[i] =='\\' )
		{
			OutTemp = &Path[i+1];
		}
	}

	//Remove the file extension
	Len = OutTemp.Len();
	for( eg_uint i=0; i<Len; i++ )
	{
		if( OutTemp[i] == '.' )
		{
			OutTemp.ClampEnd( Len-i );
			break;
		}
	}


	*this = OutTemp;
}

void eg_string_base::SetToDirectoryFromPath( eg_cpstr strPath )
{
	//1. Start by setting the out to the path.
	eg_string_big strOut;
	strOut = strPath;

	//If the string is length zero we bail out so that the following loop will
	//not have problems.
	if(0 == strOut.Len())return;

	//2. Serach backwards until we find a \ or / to indicate
	//a directory change.
	eg_int i=0;
	for(i=static_cast<eg_int>(strOut.Len())-1; i>=0; i--)
	{
		eg_char c = strOut[static_cast<eg_uint>(i)];
		if(c == '\\' || c == '/')
		{
			break;
		}
		strOut.ClampEnd(1);
	}
	*this = strOut;
}

void eg_string_base::MakeThisFilenameRelativeTo( eg_cpstr strParent )
{
	*this = *EGPath2_GetFullPathRelativeTo( String() , strParent );
}

void eg_string_base::Replace( eg_cpstr RepString , eg_cpstr WithString )
{
	eg_char strTemp[2048];
	this->CopyTo( strTemp , countof(strTemp) );
	eg_size_t RepStringLen = EGString_StrLen( RepString );
	eg_string_big StrFinal = strTemp;

	if( 0 == RepStringLen )
	{
		return;
	}

	for( eg_cpstr LocStr = EGString_StrStr( strTemp , RepString ); nullptr != LocStr; LocStr = EGString_StrStr( strTemp , RepString ) )
	{
		eg_string_big PostString = LocStr + RepStringLen;
		eg_uintptr_t EndOfPreString = reinterpret_cast<eg_uintptr_t>(&LocStr[0]) - reinterpret_cast<eg_uintptr_t>(&strTemp[0]);
		strTemp[EndOfPreString] = '\0';
		eg_string_big PreString = strTemp;
		StrFinal = PreString;
		StrFinal.Append(WithString);
		StrFinal.Append(PostString);
		StrFinal.CopyTo( strTemp , countof(strTemp) );
	}
	*this = StrFinal;
}

void eg_string_base::ToXmlFriendly( const eg_string_base& String , eg_string_base& Out )
{
	Out.Clear();

	static const struct egMapping
	{
		eg_char c;
		eg_cpstr Value;
	}
	Mapping[]=
	{
		{ '\"' , "&quot;" },
		{ '\xA9' , "&#xA9;" },
		{ '\n' , "&#xA;"  },
		{ '\r' , "&#xD;"  },
		{ '<'  , "&lt;"   },
		{ '>'  , "&gt;"   },
		{ '&'  , "&amp;"  },
		{ '\'' , "&apos;" },
		{ '\t' , "&#x9;"  },
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

	for( eg_size_t i = 0; i < String.Len(); i++ )
	{
		AppendChar( String[i] );
	}
}

#include "EGStringLibrary.hpp"

#include "EGParse.h"

//For the string character escape table we'll use the standard C table
static const struct
{
	eg_char EscapeCode;
	eg_char Char;
}
EGParse_EscapeCodes[] =
{
	{ 'n' , '\n' },
	{ 'r' , '\r' },
	{ 'a' , '\a' },
	{ 'b' , '\b' },
	{ 'f' , '\f' },
	{ 't' , '\t' },
	{ 'v' , '\v' },
	{ '?' , '\?' },
	{ 'x' , 'x'  }, //Special case
};

eg_bool EGParse_IsWhiteSpace( eg_char c )
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

enum EGPARSE_NAME_RES
{
	NAME_OKAY,
	NAME_FUNCTION_START,
	NAME_FUNCTION_DONE,
	NAME_ERROR,
};

static eg_bool EGParse_ParseFunction_IsFirstCharForNameAllowed( eg_char c )
{
	if( 'a' <= c && c <= 'z' )return true;
	if( 'A' <= c && c <= 'Z' )return true;
	if( '_' == c ) return true;

	return false;
}

static eg_bool EGParse_IsNameFirstCharValid( eg_cpstr Str )
{
	return Str && Str[0] != '\0' ? EGParse_ParseFunction_IsFirstCharForNameAllowed( Str[0] ) : true;
}


static EGPARSE_NAME_RES EGParse_ParseFunction_NameChar( eg_char c )
{
	//Valid characters:
	if('a' <= c && c <= 'z')return NAME_OKAY;
	if('A' <= c && c <= 'Z')return NAME_OKAY;
	if('0' <= c && c <= '9')return NAME_OKAY;
	if('_' == c)return NAME_OKAY;
	//Switching to either system or the function call:
	if('.' == c)return NAME_FUNCTION_START;
	if('(' == c)return NAME_FUNCTION_DONE;
	//Invalid character (handles null termination).
	return NAME_ERROR;
}

enum EGPARSE_PARM_RES
{
	PARM_OKAY,
	PARM_BEGINSTRING,
	PARM_ENDPARM,
	PARM_ENDALLPARMS,
	PARM_E_NULLTERMINATE,
	PARM_BEGINWHITESPACE,
	PARM_E_BADCHAR,
};


static EGPARSE_PARM_RES EGParse_ParseFunction_ParmChar( eg_char c )
{
	//Valid characters:
	if('"' == c)return PARM_BEGINSTRING;
	if(',' == c)return PARM_ENDPARM;
	if(')' == c)return PARM_ENDALLPARMS;
	if('(' == c)return PARM_E_BADCHAR;
	if(';' == c)return PARM_E_BADCHAR;
	if(0 == c)return PARM_E_NULLTERMINATE;
	if(EGParse_IsWhiteSpace(c))return PARM_BEGINWHITESPACE;

#if 1
	return PARM_OKAY; //The calling function will need to decide if the parms are okay.
#else
	if('a' <= c && c <= 'z')return PARM_OKAY;
	if('A' <= c && c <= 'Z')return PARM_OKAY;
	if('0' <= c && c <= '9')return PARM_OKAY;
	if('_' == c || '.' == c || '-' == c)return PARM_OKAY;

	return PARM_E_BADCHAR;
#endif
}

static eg_size_t EGParse_ParseFunction_IgnoreWhiteSpace( eg_cpstr sLine , eg_size_t nPos )
{
	while(EGParse_IsWhiteSpace(sLine[nPos]))
	{
		nPos++;
	}
	return nPos;
}

static eg_char EGParse_EscapeCodeToChar( eg_char EscapeCode )
{
	eg_char CharOut = EscapeCode; //In many cases the char will be the escape code itself e.g. \" -> "

	for( eg_uint i=0; i<countof(EGParse_EscapeCodes); i++ )
	{
		if( EscapeCode == EGParse_EscapeCodes[i].EscapeCode )
		{
			CharOut = EGParse_EscapeCodes[i].Char;
			break;
		}
	}

	return CharOut;
}

static eg_uint8 EGParse_HexNumberCharToValue( eg_char Char )
{
	if( '0' <= Char && Char <= '9' )
	{
		return static_cast<eg_uint8>(Char-'0');
	}
	if( 'A' <= Char && Char <= 'F' )
	{
		return static_cast<eg_uint8>( (Char-'A') + 0x0A );
	}
	if( 'a' <= Char && Char <= 'f' )
	{
		return static_cast<eg_uint8>( (Char-'a') + 0x0A );
	}
	return 0;
}

static eg_bool EGParse_AppendCharToStorage( eg_char Char , eg_char* Storage , eg_size_t StorageSize , eg_size_t* StoragePosInOut )
{
	if( *StoragePosInOut < StorageSize-1 )
	{
		Storage[*StoragePosInOut] = Char;
		(*StoragePosInOut)++;
		return true;
	}
	return false;
}


static eg_size_t EGParse_ParseFunction_ReadString( eg_cpstr sLine , eg_size_t nPos , eg_char* Storage , eg_size_t StorageSize , eg_size_t* StoragePosInOut , EGPARSE_RESULT* ErrorOut )
{
	if( ErrorOut ){ *ErrorOut = EGPARSE_OKAY; }

	//We assume that nPos is right after the starting quote.
	while(true)
	{
		eg_char c = sLine[nPos];
		nPos++;
		if(0 == c)
		{
			if( ErrorOut ){ *ErrorOut = EGPARSE_E_INCOMPLETEPARMS; } // Missing closing quote.
			return nPos - 1;
		}

		if('"' == c)
		{
			return nPos;
		}

		if('\\' == c)
		{
			//Skip ahead a character:
			c = EGParse_EscapeCodeToChar(sLine[nPos]);
			nPos++;
			if(0 == c)return nPos - 1;

			//x is a special case where we want to get a hex code.
			if('x' == c )
			{
				if( sLine[nPos+0] != 0 && sLine[nPos+1] != 0 && sLine[nPos+0] != '\"' && sLine[nPos+1] != '\"' )
				{
					eg_char c1 = sLine[nPos+0];
					eg_char c2 = sLine[nPos+1];
					
					c = (EGParse_HexNumberCharToValue(c1)<<4) | (EGParse_HexNumberCharToValue(c2)<<0);
					nPos+=2;
				}
				else
				{
					//String format error...
				}
			}
		}
		if( EGParse_AppendCharToStorage( c , Storage , StorageSize , StoragePosInOut ) )
		{
		}
		else
		{
			if( ErrorOut ){ *ErrorOut = EGPARSE_E_OUTOFMEM; }
		}
	}
	return nPos;
}

EGPARSE_RESULT EGParse_ParseFunction( eg_cpstr sLine , egParseFuncBase* pOut )
{
	if( nullptr == sLine || nullptr == pOut )
	{
		return EGPARSE_E_OUTOFMEM;
	}
	//Setup some default stuff:
	pOut->SystemName = nullptr;
	pOut->FunctionName = nullptr;
	for( eg_uint i=0; i<pOut->ParmsSize; i++ )
	{
		pOut->Parms[i] = nullptr;
	}
	EGMem_Set( pOut->Storage , 0 , pOut->StorageSize );

	if( nullptr == pOut->Storage || nullptr == pOut->Parms || 0 == pOut->StorageSize || 0 == pOut->ParmsSize )
	{
		return EGPARSE_E_OUTOFMEM;
	}

	eg_size_t nPos = 0;
	eg_size_t nStoragePos = 0;

	//Ignore leading whitespace.
	//Technically ignoring whitespace breaks the NFA rules since the last
	//character is considered twice, but ohs wells.
	nPos = EGParse_ParseFunction_IgnoreWhiteSpace(sLine, nPos);

	if( sLine[nPos] == '\0' )
	{
		return EGPARSE_E_WASONLYWHITESPACE;
	}

	//Get the system name and function:
	enum NAME_STATE
	{
		SYSTEM_OR_FUNCTION,
		FUNCTION,
	};

	pOut->FunctionName = pOut->Storage;

	NAME_STATE NameState = SYSTEM_OR_FUNCTION;

	eg_bool StillSearching = true;
	while( StillSearching )
	{
		eg_char c = sLine[nPos];
		nPos++;

		EGPARSE_NAME_RES r = EGParse_ParseFunction_NameChar(c);

		switch (r)
		{
		case NAME_OKAY: 
			if( EGParse_AppendCharToStorage( c , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
			{
			}
			else
			{
				return EGPARSE_E_OUTOFMEM;
			}
			break;
		case NAME_FUNCTION_START:
		{
			if(SYSTEM_OR_FUNCTION == NameState)
			{
				if(pOut->FunctionName[0] == '\0')return EGPARSE_E_DOTWITHNOSYSTEM;

				pOut->SystemName = pOut->FunctionName;
				if( EGParse_AppendCharToStorage( '\0' , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
				{
				}
				else
				{
					return EGPARSE_E_OUTOFMEM;
				}
				pOut->FunctionName = &pOut->Storage[nStoragePos];
				NameState = FUNCTION;
			}
			else
			{
				return EGPARSE_E_MULTIPLESYSTEMSEPARATORS;
			}
		} break;
		case NAME_FUNCTION_DONE:
		{
			if( pOut->FunctionName[0] == '\0')return EGPARSE_E_NOFUNCTION;

			StillSearching = false;
		} break;
		case NAME_ERROR:
		{
			if(0 == c)return EGPARSE_E_NOPARMS;
			else return EGPARSE_E_INVALIDNAME;
		} break;
		}
	}

	//Append an end of string to the storage, since we will now be filling out the parameters.
	if( EGParse_AppendCharToStorage( '\0' , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
	{
	}
	else
	{
		return EGPARSE_E_OUTOFMEM;
	}

	//Now we get the parameters:
	eg_uint nParm = 0;
	pOut->Parms[0] = &pOut->Storage[nStoragePos];
	eg_bool IsString = false;
	StillSearching = true;
	nPos = EGParse_ParseFunction_IgnoreWhiteSpace(sLine, nPos);
	//If the first character we encountered was a ')' then there were no parms
	//this breaks the NFA rule, since we'll consider this character twice if
	//it wasn't.
	if(PARM_ENDALLPARMS == EGParse_ParseFunction_ParmChar(sLine[nPos]))
	{
		nPos++;
		StillSearching = false;
	}

	while(StillSearching)
	{
		eg_char c = sLine[nPos];
		nPos++;

		EGPARSE_PARM_RES r = EGParse_ParseFunction_ParmChar(c);
		switch (r)
		{
		case PARM_OKAY:
			if(IsString)return EGPARSE_E_STRINGWITHIDENTIFIER;
			if( EGParse_AppendCharToStorage( c , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
			{
			}
			else
			{
				return EGPARSE_E_OUTOFMEM;
			}
			break;
		case PARM_BEGINSTRING:
		{
			IsString = true;
			if(pOut->Parms[nParm][0] != '\0')return EGPARSE_E_STRINGWITHIDENTIFIER;

			EGPARSE_RESULT ParseStringRes = EGPARSE_OKAY;
			nPos = EGParse_ParseFunction_ReadString(sLine, nPos, pOut->Storage , pOut->StorageSize , &nStoragePos , &ParseStringRes );
			if( ParseStringRes != EGPARSE_OKAY )
			{
				return ParseStringRes;
			}
		} break;
		case PARM_ENDPARM:
			if(!IsString && pOut->Parms[nParm][0] == '\0')return EGPARSE_E_EMPTYPARM;

			if( EGParse_AppendCharToStorage( '\0' , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
			{
				nParm++;
				pOut->Parms[nParm] = &pOut->Storage[nStoragePos];
				IsString = false;
				if(pOut->ParmsSize == nParm)return EGPARSE_E_TOOMANYPARMS;

				nPos = EGParse_ParseFunction_IgnoreWhiteSpace(sLine, nPos);
			}
			else
			{
				return EGPARSE_E_OUTOFMEM;
			}
			break;
		case PARM_ENDALLPARMS:
			if(!IsString && pOut->Parms[nParm][0] == '\0')return EGPARSE_E_EMPTYPARM;
			if( EGParse_AppendCharToStorage( '\0' , pOut->Storage , pOut->StorageSize , &nStoragePos ) )
			{
				nParm++;
				IsString = false;
				StillSearching = false;
			}
			else
			{
				return EGPARSE_E_OUTOFMEM;
			}
			break;
		case PARM_E_NULLTERMINATE:
			return EGPARSE_E_INCOMPLETEPARMS;
		case PARM_E_BADCHAR:
			return EGPARSE_E_BADIDENTIFIER;
		case PARM_BEGINWHITESPACE:
		{
			//We ignore all whitespace and we better encounter a ENDPARM
			//or ENDALLPARMS afterwards.
			nPos = EGParse_ParseFunction_IgnoreWhiteSpace(sLine, nPos);
			EGPARSE_PARM_RES r = EGParse_ParseFunction_ParmChar(sLine[nPos]);
			if(PARM_ENDPARM != r && PARM_ENDALLPARMS != r && PARM_E_NULLTERMINATE != r)return EGPARSE_E_WHITESPACE;
		} break;
		}
	}

	pOut->NumParms = nParm;

	//Read any trailing whitespace.
	nPos = EGParse_ParseFunction_IgnoreWhiteSpace(sLine, nPos);

	//We better be at the end of the string:
	if(0 != sLine[nPos])return EGPARSE_E_TRAILINGCHARS;

	//We don't want any null pointers, so assigning any remaining pars to the last character.
	for( eg_uint i = nParm; i<pOut->ParmsSize; i++ )
	{
		pOut->Parms[i] = &pOut->Storage[nStoragePos];
	}
	if( nullptr == pOut->SystemName )
	{
		pOut->SystemName = &pOut->Storage[nStoragePos];
	}

	// Check for invalid names
	if( !EGParse_IsNameFirstCharValid( pOut->SystemName ) || !EGParse_IsNameFirstCharValid(pOut->FunctionName) )
	{
		return EGPARSE_E_INVALIDNAME_FIRSTCHAR;
	}

	return EGPARSE_OKAY;
}

EGPARSE_RESULT EGParse_ParseCSV( eg_cpstr sLine , egParseFuncBase* pOut )
{
	eg_string_big sT = EGString_Format("F(%s)", sLine);
	EGPARSE_RESULT r = EGParse_ParseFunction(sT, pOut);
	if( EGPARSE_OKAY == r )
	{
		pOut->Storage[0] = '\0'; //This will make the function name 0.
	}
	return r;
}

static EGPARSE_RESULT EGParse_GetAttValue_GetValue( eg_cpstr sLineAfterName , eg_string_base* sOutValue )
{
	eg_uint nPos = 0;
	enum STATE
	{
		LOOKING_FOR_EQUALS,
		LOOKING_FOR_STARTQUOTE,
		READING_CHARS,
	};

	STATE State = LOOKING_FOR_EQUALS;

	while(0 != sLineAfterName[nPos])
	{
		eg_char c = sLineAfterName[nPos];

		if(LOOKING_FOR_EQUALS == State)
		{
			if('=' == c)
			{
				State = LOOKING_FOR_STARTQUOTE;
			}
			else if(!EGParse_IsWhiteSpace(c))
			{
				return EGPARSE_E_NOTFOUND;
			}
		}
		else if(LOOKING_FOR_STARTQUOTE == State)
		{
			if('\"' == c)
			{
				State = READING_CHARS;
			}
			else if(!EGParse_IsWhiteSpace(c))
			{
				return EGPARSE_E_NOTFOUND;
			}
		}
		else if(READING_CHARS == State)
		{
			if('\"' == c)
			{
				return EGPARSE_OKAY;
			}
			else if('\\' == c)
			{
				if(0 != sLineAfterName[nPos + 1])
				{
					*sOutValue += sLineAfterName[nPos + 1];
					nPos++;
				}
				else
				{
					sOutValue->Clear();
					return EGPARSE_E_NOTFOUND;
				}
			}
			else
			{
				*sOutValue += c;
			}
		}

		nPos++;
	}
	sOutValue->Clear();
	return EGPARSE_E_NOTFOUND;
}

static EGPARSE_RESULT EGParse_GetAttValue_GetValue( eg_cpstr sLineAfterName , eg_d_string& sOutValue )
{
	eg_uint nPos = 0;
	enum STATE
	{
		LOOKING_FOR_EQUALS,
		LOOKING_FOR_STARTQUOTE,
		READING_CHARS,
	};

	STATE State = LOOKING_FOR_EQUALS;

	while(0 != sLineAfterName[nPos])
	{
		eg_char c = sLineAfterName[nPos];

		if(LOOKING_FOR_EQUALS == State)
		{
			if('=' == c)
			{
				State = LOOKING_FOR_STARTQUOTE;
			}
			else if(!EGParse_IsWhiteSpace(c))
			{
				return EGPARSE_E_NOTFOUND;
			}
		}
		else if(LOOKING_FOR_STARTQUOTE == State)
		{
			if('\"' == c)
			{
				State = READING_CHARS;
			}
			else if(!EGParse_IsWhiteSpace(c))
			{
				return EGPARSE_E_NOTFOUND;
			}
		}
		else if(READING_CHARS == State)
		{
			if('\"' == c)
			{
				return EGPARSE_OKAY;
			}
			else if('\\' == c)
			{
				if(0 != sLineAfterName[nPos + 1])
				{
					sOutValue.Append( sLineAfterName[nPos + 1] );
					nPos++;
				}
				else
				{
					sOutValue = "";
					return EGPARSE_E_NOTFOUND;
				}
			}
			else
			{
				sOutValue.Append( c );
			}
		}

		nPos++;
	}
	sOutValue = "";
	return EGPARSE_E_NOTFOUND;
}

EGPARSE_RESULT EGParse_GetAttValue( eg_cpstr sLine , eg_cpstr sName , eg_string_base* sOutValue )
{
	sOutValue->Clear();
	eg_uint nPos = 0;
	eg_string_fixed_size<2048> sAtt;
	eg_bool bWordEndFound = false;
	while(0 != sLine[nPos])
	{
		eg_char c = sLine[nPos];
		if('=' == c || EGParse_IsWhiteSpace(c))
		{
			bWordEndFound = true;
			//See if the word was correct.
			if(!sAtt.Compare(sName))
				return EGParse_GetAttValue_GetValue(&sLine[nPos], sOutValue);
		}
		else
		{
			if(bWordEndFound)
			{
				sAtt.Clear();
				bWordEndFound = false;
			}
			sAtt += c;
		}

		nPos++;
	}
	return EGPARSE_E_NOTFOUND;
}

eg_d_string EGParse_GetAttValue( eg_cpstr sLine , eg_cpstr sName )
{
	eg_uint nPos = 0;
	eg_string_fixed_size<2048> sAtt;
	eg_bool bWordEndFound = false;
	while(0 != sLine[nPos])
	{
		eg_char c = sLine[nPos];
		if('=' == c || EGParse_IsWhiteSpace(c))
		{
			bWordEndFound = true;
			//See if the word was correct.
			if(!sAtt.Compare(sName))
			{
				eg_d_string OutValue;
				EGPARSE_RESULT Res = EGParse_GetAttValue_GetValue( &sLine[nPos] , OutValue );
				if( Res != EGPARSE_OKAY )
				{
					OutValue = "";
				}
				return OutValue;
			}
		}
		else
		{
			if(bWordEndFound)
			{
				sAtt.Clear();
				bWordEndFound = false;
			}
			sAtt += c;
		}

		nPos++;
	}
	return "";
}

eg_cpstr EGParse_GetParseResultString( EGPARSE_RESULT r )
{
	static const struct
	{
		EGPARSE_RESULT Res;
		eg_cpstr       Str;
	}
	Table[] =
	{
		{ EGPARSE_OKAY                       , ("Okay") } ,
		{ EGPARSE_E_MULTIPLESYSTEMSEPARATORS , ("Unexpected '.', at most one system is allowed") } ,
		{ EGPARSE_E_INVALIDNAME              , ("Unexpected character found in name") } ,
		{ EGPARSE_E_NOPARMS                  , ("No '(' encountered, parms must be specified") } ,
		{ EGPARSE_E_TOOMANYPARMS             , ("Too many parms specified") } ,
		{ EGPARSE_E_INCOMPLETEPARMS          , ("Missing ')' or '\"', parm list was incomplete") } ,
		{ EGPARSE_E_TRAILINGCHARS            , ("Characters encountered after closing ')', end of line expected") } ,
		{ EGPARSE_E_WHITESPACE               , ("White space was found in the middle of an identifier") } ,
		{ EGPARSE_E_STRINGWITHIDENTIFIER     , ("A parm must be a string or identifier, not both") } ,
		{ EGPARSE_E_BADIDENTIFIER            , ("A parm contained an invalid character") } ,
		{ EGPARSE_E_EMPTYPARM                , ("A parm contained no identifier or string") } ,
		{ EGPARSE_E_DOTWITHNOSYSTEM          , ("No system was specified but '.' was used") } ,
		{ EGPARSE_E_NOTFOUND                 , ("The desired attribute was not found") } ,
		{ EGPARSE_E_NOFUNCTION               , ("No function name specified") },
		{ EGPARSE_E_OUTOFMEM                 , ("Insufficient storage") },
		{ EGPARSE_E_BADVARDECL               , ("Bad var declaration") },
		{ EGPARSE_E_WASONLYWHITESPACE        , ("Was only whitespace") },
	};

	for( eg_uint i=0; i<countof(Table); i++ )
	{
		if( r == Table[i].Res )return Table[i].Str;
	}

	assert( false );
	return "Unknown";
}

static eg_cpstr EGParse_ParseVarDecl_ReservedWords[] =
{
	"void",
	"terminal",
	"int",
	"real",
	"bool",
	"float",
	"const",
	"number",
};

static eg_bool EGParse_ParseVarDecl_ValidateNames( const egParseVarDecl& Decl )
{
	auto IsReservedWord = []( eg_cpstr Str ) -> eg_bool
	{
		for( eg_cpstr Word : EGParse_ParseVarDecl_ReservedWords )
		{
			if( EGString_EqualsI( Word , Str ) )
			{
				return true;
			}
		}

		return false;
	};

	if( IsReservedWord( Decl.Name ) )
	{
		return false;
	}

	if( Decl.bIsFunctionDecl )
	{
		for( eg_size_t i=0; i<Decl.NumParms; i++ )
		{
			if( IsReservedWord( Decl.Parms[i].ParmName ) )
			{
				return false;
			}

			if( !EGParse_IsNameFirstCharValid( Decl.Parms[i].ParmName ) || !EGParse_IsNameFirstCharValid( Decl.Parms[i].ParmType ) )
			{
				return false;
			}
		}
	}

	return true;
}

egParseVarDecl EGParse_ParseVarDecl( eg_cpstr sLine )
{
	egParseVarDecl Out;
	Out.Result = EGPARSE_OKAY;
	Out.bIsFunctionDecl = false;
	Out.NumParms = 0;

	eg_size_t ReadPos = 0;
	EGPARSE_RESULT LambdaError = EGPARSE_OKAY;

	auto ReadWhiteSpace = [&ReadPos,&sLine]() -> void
	{
		while( EGParse_IsWhiteSpace( sLine[ReadPos] ) )
		{
			ReadPos++;
		}
	};

	auto ReadVarTypeConst = [&ReadPos,&sLine,&ReadWhiteSpace]() -> eg_bool
	{
		eg_bool bIsConstOut = false;

		if( EGString_EqualsCount( &sLine[ReadPos] , "const" , 5 ) && EGParse_IsWhiteSpace( sLine[ReadPos+5] ) )
		{
			bIsConstOut = true;
			ReadPos += 5;
			ReadWhiteSpace();
		}

		return bIsConstOut;
	};

	auto ReadVarTypeWord = [&ReadPos,&sLine]() -> eg_string
	{
		eg_string TypeWord;

		while( EGParse_ParseFunction_NameChar( sLine[ReadPos] ) == NAME_OKAY )
		{
			TypeWord.Append( sLine[ReadPos] );
			ReadPos++;
		}

		return TypeWord;
	};

	auto ReadVarNameWord = [&ReadPos, &sLine]() -> eg_string
	{
		eg_string NameWord;

		while( EGParse_ParseFunction_NameChar( sLine[ReadPos] ) == NAME_OKAY )
		{
			NameWord.Append( sLine[ReadPos] );
			ReadPos++;
		}

		return NameWord;
	};

	auto ReadDefaultValue = [&ReadPos,&sLine,&LambdaError]() -> eg_string
	{
		eg_string Out( CT_Clear );

		eg_bool bIsString = EGParse_ParseFunction_ParmChar( sLine[ReadPos] ) == PARM_BEGINSTRING;
		eg_bool bIsStringClosed = false;

		if( bIsString )
		{
			ReadPos++;
			eg_char DefaultAsStr[eg_string_big::STR_SIZE] = { '\0' };
			eg_size_t StorePos = 0;
			EGPARSE_RESULT Res = EGPARSE_OKAY;
			ReadPos = EGParse_ParseFunction_ReadString( sLine , ReadPos , DefaultAsStr , countof( DefaultAsStr ) , &StorePos , &Res );
			if( Res == EGPARSE_OKAY )
			{
				EGParse_AppendCharToStorage( '\0' , DefaultAsStr , countof( DefaultAsStr ) , &StorePos );
				DefaultAsStr[countof( DefaultAsStr ) - 1] = '\0';
				Out = DefaultAsStr;
			}
			else
			{
				// Bad string...
				LambdaError = EGPARSE_E_BADVARDECL;
				return Out;
			}
		}
		else
		{
			auto ShouldContinue = [&sLine , &ReadPos]() -> eg_bool
			{
				eg_char c = sLine[ReadPos];
				if( c == '\0' )
				{
					return false;
				}
				if( EGParse_IsWhiteSpace( c ) )
				{
					return false;
				}
				if( c == ';' )
				{
					return false;
				}

				return true;
			};

			while( ShouldContinue() )
			{
				EGPARSE_PARM_RES r = EGParse_ParseFunction_ParmChar( sLine[ReadPos] );

				if( PARM_OKAY == r )
				{
					if( EGParse_ParseFunction_ParmChar( sLine[ReadPos] ) == PARM_OKAY )
					{
						Out.Append( sLine[ReadPos] );
					}
					else
					{
						// Not a valid identifier for default
						LambdaError = EGPARSE_E_BADVARDECL;
						return Out;
					}
				}
				else if( PARM_E_NULLTERMINATE == r )
				{
					assert( false ); // Should have got caught in while check
				}
				else
				{
					// Bad assignment
					LambdaError = EGPARSE_E_BADVARDECL;
					return Out;
				}

				ReadPos++;
			}
		}

		if( Out.Len() == 0 && !bIsString )
		{
			// Nothing assigned
			LambdaError = EGPARSE_E_BADVARDECL;
			return Out;
		}

		return Out;
	};

	ReadWhiteSpace(); // Consume any whitespace at the beginning.

	if( '\0' == sLine[ReadPos] )
	{
		// No actual statement
		Out.Result = EGPARSE_E_BADVARDECL;
		return Out;
	}

	// Get type
	Out.Type = ReadVarTypeWord();

	if( !EGParse_IsWhiteSpace( sLine[ReadPos] ) )
	{
		// Badly named type (type must be followed by whitespace)
		Out.Result = EGPARSE_E_BADVARDECL;
		return Out;
	}

	ReadWhiteSpace();

	// Get Name
	Out.Name = ReadVarNameWord();

	// A few things may happen here, we may be at the end of the statement, or we might have an =, in any case we can read whitespace.
	// or a ( in which case we are a function declaration.
	ReadWhiteSpace();

	if( sLine[ReadPos] == '=' )
	{
		ReadPos++;
		ReadWhiteSpace();
		Out.Default = ReadDefaultValue();
		if( LambdaError != EGPARSE_OKAY )
		{
			Out.Result = LambdaError;
			return Out;
		}
	}
	else if( sLine[ReadPos] == '(' )
	{
		ReadPos++;
		// We are a function declaration.
		Out.bIsFunctionDecl = true;
		eg_bool bFindingParms = true;
		eg_bool bTooManyParms = false;

		auto CommitParm = [&Out,&bTooManyParms]( const egParseVarDecl::egParmDecl& ParmDecl ) -> void
		{
			if( Out.NumParms < countof(Out.Parms) )
			{
				Out.Parms[Out.NumParms] = ParmDecl;
				Out.NumParms++;
			}
			else
			{
				bTooManyParms = true;
			}
		};

		while( bFindingParms )
		{
			ReadWhiteSpace();

			if( sLine[ReadPos] == '\0' )
			{
				Out.Result = EGPARSE_E_BADVARDECL;
				return Out;
			}

			if( sLine[ReadPos] == ')' )
			{
				ReadPos++;
				bFindingParms = false;
			}
			else
			{

				egParseVarDecl::egParmDecl ParmDecl( CT_Clear );

				ParmDecl.bIsConst = ReadVarTypeConst();
				ParmDecl.ParmType = ReadVarTypeWord();
				ReadWhiteSpace();

				if( sLine[ReadPos] == '&' )
				{
					ParmDecl.bIsRef = true;
					ReadPos++;
					ReadWhiteSpace();
				}

				if( ParmDecl.ParmType.Len() == 0 )
				{
					Out.Result = EGPARSE_E_BADVARDECL;
					return Out;
				}
				else if( sLine[ReadPos] == ',' )
				{
					CommitParm( ParmDecl );
					ReadPos++;
				}
				else if( sLine[ReadPos] == ')' )
				{
					CommitParm( ParmDecl );
					ReadPos++;
					bFindingParms = false;
				}
				else
				{
					ParmDecl.ParmName = ReadVarNameWord();
					ReadWhiteSpace();
					if( sLine[ReadPos] == '=' ) // Default Value
					{
						ReadPos++;
						ReadWhiteSpace();
						ParmDecl.Default = ReadDefaultValue();
						if( LambdaError != EGPARSE_OKAY )
						{
							Out.Result = LambdaError;
							return Out;
						}
						if( ParmDecl.ParmName.Len() == 0 )
						{
							Out.Result = EGPARSE_E_INVALIDNAME; // If there is a default value there needs to be a name.
							return Out;
						}
					}
					CommitParm( ParmDecl ); // We may not have read a name, but that's okay.
					ReadWhiteSpace();


					if( sLine[ReadPos] == ',' )
					{
						ReadPos++;  // We'll be getting the next parm (or end)
					}
				}
			}
		}

		if( bTooManyParms )
		{
			Out.Result = EGPARSE_E_TOOMANYPARMS;
			return Out;
		}
	}

	ReadWhiteSpace();
	if( sLine[ReadPos] == ';' )
	{
		ReadPos++;
	}
	ReadWhiteSpace();

	if( sLine[ReadPos] != '\0' )
	{
		// Invalid declaration after statement
		Out.Result = EGPARSE_E_BADVARDECL;
		return Out;
	}

	if( Out.Type.Len() == 0 || Out.Name.Len() == 0 )
	{
		// Name or type missing
		Out.Result = EGPARSE_E_BADVARDECL;
		return Out;
	}

	if( !EGParse_IsNameFirstCharValid( Out.Type ) || !EGParse_IsNameFirstCharValid( Out.Name ) )
	{
		Out.Result = EGPARSE_E_INVALIDNAME_FIRSTCHAR;
		return Out;
	}

	eg_bool bNamesAreValid = EGParse_ParseVarDecl_ValidateNames( Out );

	if( !bNamesAreValid )
	{
		Out.Result = EGPARSE_E_INVALIDNAME;
	}

	return Out;
}

void egParseFuncBase::CopyFrom( const egParseFuncBase& rhs )
{
	SystemName = nullptr;
	FunctionName = nullptr;
	NumParms = 0;
	for( eg_uint i = 0; i < ParmsSize; i++ )
	{
		Parms[i] = nullptr;
	}

	eg_size_t AllocPos = 0;
	auto AllocString = [&AllocPos,this]( eg_size_t Size ) -> eg_pstr
	{
		eg_pstr Out = nullptr;

		if( AllocPos + Size <= StorageSize )
		{
			Out = &Storage[AllocPos];
			AllocPos += Size;
		}

		return Out;
	};

	auto DuplicateString = [&AllocString]( eg_cpstr* Target , eg_cpstr Src ) -> eg_bool
	{
		const eg_size_t StrSize = EGString_StrLen( Src );
		eg_pstr NewString = AllocString( StrSize + 1 );
		if( NewString )
		{
			EGString_Copy( NewString , Src , StrSize + 1 );
			*Target = NewString;
			return true;
		}
		return false;
	};

	DuplicateString( &SystemName , rhs.SystemName );
	DuplicateString( &FunctionName , rhs.FunctionName );
	
	NumParms = EG_Min<eg_uint>( rhs.NumParms , static_cast<eg_uint>(ParmsSize) );
	for( eg_uint i=0; i<NumParms;  i++ )
	{
		DuplicateString( &Parms[i] , rhs.Parms[i] );
	}
}

eg_bool egParseFuncBase::IsValid() const
{
	eg_bool bValid = true;

	bValid = SystemName && FunctionName;

	for( eg_uint i=0; bValid && i<NumParms; i++ )
	{
		bValid = bValid && Parms[i];
	}

	return bValid;
}

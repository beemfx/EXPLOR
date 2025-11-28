// (c) 2015 Beem Media

#pragma once

/*******************************************************************************
EGParse_ParseFunction, parses an expression as follows:

Let F represent valid function name parameters (A-Z, a-z, 0-9, _)
Let P represent a valid parm. A Parm may be in one of the following forms:
"S{0,}"
I{1,}

Where S represents any character allowed in a string, which is anything except
the null terminating character and other quotes. However, a quote may be
included in the string if it is preceded by a \ (\"), and if a \ is desired it
must be preceded by a \ as well (\\). (Note when in C++ the usual rules for
slashes apply so a quote would actually (be \\\"). In the game console things
are a little cleaner.

And where I represents any character allowed in an identifier. Being that
identifiers can be almost anything, anything other than whitespace parenthesis
and commas is allowed. However, whitespace is allowed on either side of the
identifier.

Let W be whitespace:

The regular expression would then be:

W*(F+.)?F+\((W*PW*,){0,MAX_PARMS-1}W*P?W*\)W*

Basically it is a C style function that can be preceded by a namespace where
the namespace is identified with a . as opposed to a :: and there can either be
0 or 1 namespaces.

Parms may either be strings or identifiers, see above.

Examples:

server.run("/bt.lua")
quit()
server.spawn("Missile", 0, -2, 3)
game.Rename( "This is \\\"Jack's\\\" Game" )
client.connect()

The output is stored in a egParseFuncInfo struct. All input is stored as
strings, but may then be converted to numbers or commands as desired.

Notes:
Run-time is O(n) where n is the number of characters in the string. Other than
a minimal amount of stack space and the memory passed in for pOut, no memory
is used and no allocations are made.

The O(n) run-time is because the parser is an NFA, so each character is only
considered once and the appropriate action is taken (there are a few exceptions
to the NFA rules, but they can only happen a fixed number of times (MAX_PARMS+1)
so they do not change the complexity).

After parsing is completed, strings do not contain their surrounding quotes.

The function only parses the input, it does no verification that the function
is an actual function, or that the parameter names are the correct type or mean
anything useful.
*******************************************************************************/

enum EGPARSE_RESULT
{
	EGPARSE_OKAY,
	EGPARSE_E_MULTIPLESYSTEMSEPARATORS,
	EGPARSE_E_INVALIDNAME,
	EGPARSE_E_INVALIDNAME_FIRSTCHAR,
	EGPARSE_E_NOPARMS,
	EGPARSE_E_TOOMANYPARMS,
	EGPARSE_E_INCOMPLETEPARMS,
	EGPARSE_E_TRAILINGCHARS,
	EGPARSE_E_WHITESPACE,
	EGPARSE_E_STRINGWITHIDENTIFIER,
	EGPARSE_E_BADIDENTIFIER,
	EGPARSE_E_EMPTYPARM,
	EGPARSE_E_DOTWITHNOSYSTEM,
	EGPARSE_E_NOTFOUND,
	EGPARSE_E_NOFUNCTION,
	EGPARSE_E_OUTOFMEM,
	EGPARSE_E_BADVARDECL,
	EGPARSE_E_WASONLYWHITESPACE,
};

struct egParseFuncBase
{
	eg_cpstr  SystemName;
	eg_cpstr  FunctionName;
	eg_cpstr* Parms;
	eg_size_t ParmsSize;
	eg_uint   NumParms;
	eg_char*  Storage;
	eg_size_t StorageSize;

	egParseFuncBase() = default;
	egParseFuncBase( const egParseFuncBase& rhs ) = delete;
	egParseFuncBase& operator = ( const egParseFuncBase& rhs ) = delete;

	void CopyFrom( const egParseFuncBase& rhs );
	eg_bool IsValid() const;
};

template<eg_size_t STORAGE_SIZE,eg_size_t PARM_SIZE>
struct egParseFuncT : public egParseFuncBase
{
	static const eg_size_t MAX_PARMS = PARM_SIZE;

	eg_cpstr InternalParms[PARM_SIZE];
	eg_char  InternalStorage[STORAGE_SIZE];

	egParseFuncT()
	{
		Parms = InternalParms;
		ParmsSize = countof(InternalParms);
		Storage = InternalStorage;
		StorageSize = countof(InternalStorage);
	}

	egParseFuncT( const egParseFuncT& rhs ) = delete;
	egParseFuncT( const egParseFuncT&& rhs )
	{
		CopyFrom( rhs );
	}
	egParseFuncT& operator = ( const egParseFuncT& rhs )
	{
		CopyFrom( rhs );
		return *this;
	}
	const egParseFuncT& operator = ( const egParseFuncBase& rhs )
	{
		CopyFrom( rhs );
		return *this;
	}
};

struct egParseFuncInfo : public egParseFuncBase
{
	static const eg_uint MAX_PARMS = 10;

	eg_cpstr IntParms[MAX_PARMS];
	eg_char  IntStorage[eg_string_big::STR_SIZE]; //We're probably parsing an eg_string, so we don't need any more storage than what originally fit in it.

	egParseFuncInfo()
	{
		Parms = IntParms;
		ParmsSize = countof(IntParms);
		Storage = IntStorage;
		StorageSize = countof(IntStorage);
	}
};

EGPARSE_RESULT EGParse_ParseFunction( eg_cpstr sLine , egParseFuncBase* pOut );
EGPARSE_RESULT EGParse_ParseCSV( eg_cpstr sLIne , egParseFuncBase* pOut );
EGPARSE_RESULT EGParse_GetAttValue( eg_cpstr sLine , eg_cpstr sName , eg_string_base* sOutValue );
eg_d_string EGParse_GetAttValue( eg_cpstr sLine , eg_cpstr sName );
eg_cpstr       EGParse_GetParseResultString( EGPARSE_RESULT r );
eg_bool        EGParse_IsWhiteSpace( eg_char c );

struct egParseVarDecl
{
	EGPARSE_RESULT  Result;
	eg_bool         bIsFunctionDecl;
	eg_string_small Type;
	eg_string_small Name;
	eg_string_small Default;
	eg_size_t       NumParms;
	struct egParmDecl
	{
		eg_string_small ParmType;
		eg_string_small ParmName;
		eg_string_small Default;
		eg_bool         bIsRef:1;
		eg_bool         bIsConst:1;

		egParmDecl() = default;
		egParmDecl( eg_ctor_t Ct )
		{
			if( Ct == CT_Clear || Ct == CT_Default )
			{
				ParmType = "";
				ParmName = "";
				Default = "";
				bIsRef = false;
				bIsConst = false;
			}
		}
	} Parms[egParseFuncInfo::MAX_PARMS];
};

egParseVarDecl EGParse_ParseVarDecl( eg_cpstr sLine );

//
// Bonus struct so that we can use parse results as collection of eg_strings if wanted.
//
struct egParseFuncInfoAsEgStrings
{
	egParseFuncInfoAsEgStrings( const egParseFuncBase& rhs )
	{
		SystemName = rhs.SystemName;
		FunctionName = rhs.FunctionName;
		NumParms = EG_Min<eg_uint>(rhs.NumParms,countof(Parms));
		for( eg_uint i=0; i<NumParms; i++ )
		{
			Parms[i] = rhs.Parms[i];
		}
	}

	eg_string_big SystemName;
	eg_string_big FunctionName;
	eg_string_big Parms[egParseFuncInfo::MAX_PARMS];
	eg_uint       NumParms;
};

template<typename L> static void EGParse_ProcessFnCallScript( eg_cpstr Script , eg_size_t ScriptLen , L& HandleCall )
{
	auto ReadLines = [&Script,&ScriptLen]( EGArray<eg_string_big>& Out ) -> void
	{
		// Treat either \n or ; as a new line.
		eg_string_big CurLine( CT_Clear );

		for( eg_size_t i = 0; i < ScriptLen; i++ )
		{
			eg_char c = Script[i];
			if( c == ';' || c == '\n' )
			{
				if( CurLine.Len() > 0 )
				{
					Out.Append( CurLine );
					CurLine.Clear();
				}
			}
			else if( c == '\0' )
			{
				break; // Shouldn't happen but just in case.
			}
			else
			{
				CurLine.Append( c );
			}
		}

		if( CurLine.Len() > 0 )
		{
			Out.Append( CurLine );
			CurLine.Clear();
		}
	};

	EGArray<eg_string_big> Lines;
	ReadLines( Lines );

	for( const eg_string_big& Line : Lines )
	{
		egParseFuncInfo InfoStr;
		EGPARSE_RESULT Res = EGParse_ParseFunction( Line , &InfoStr );
		if( EGPARSE_OKAY == Res )
		{
			HandleCall( InfoStr );
		}
		else
		{
			if( EGPARSE_E_WASONLYWHITESPACE != Res )
			{
				EGLogf( eg_log_t::Error , "Bad EGParse statement \"%s\": %s" , Line.String() , EGParse_GetParseResultString( Res ) );
			}
		}
	}
}
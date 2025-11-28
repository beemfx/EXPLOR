// (c) 2017 Beem Media

#include "EGSmVars.h"
#include "EGParse.h"

void EGSmVars_LoadVarDecl( const eg_char8* FileAsString , const eg_size_t FileAsStringLen , const egsmVarDeclCallback& Callback )
{
	eg_size_t Pos = 0;

	auto GetStatement = [&Pos, &FileAsString, &FileAsStringLen]( eg_string_base& Out ) -> eg_bool
	{
		eg_bool bGotStatment = false;
		eg_bool bInComment = false;
		eg_bool bInCStyleComment = false;

		Out.Clear();

		while( !bGotStatment && Pos < FileAsStringLen )
		{
			if( bInCStyleComment )
			{
				if( Pos < (FileAsStringLen-1) && FileAsString[Pos] == '*' && FileAsString[Pos+1] == '/' )
				{
					bInCStyleComment = false;
					Pos++; // So we consume both the * and the /
				}
			}
			else if( bInComment )
			{
				if( FileAsString[Pos] == '\n' )
				{
					bInComment = false;
				}
			}
			else if( FileAsString[Pos] == ';' )
			{
				bGotStatment = true;
			}
			else if( Pos < (FileAsStringLen-1) && FileAsString[Pos] == '/' && FileAsString[Pos+1] == '/' )
			{
				bInComment = true;
			}
			else if( Pos < (FileAsStringLen-1) && FileAsString[Pos] == '/' && FileAsString[Pos+1] == '*' )
			{
				bInCStyleComment = true;
			}
			else
			{
				if( !bInComment )
				{
					Out.Append( FileAsString[Pos] );
				}
			}
			Pos++;
		}

		if( bInCStyleComment )
		{
			Out.Append( "/*" ); // Force junk in for compiler error.
		}

		return bGotStatment;
	};

	auto StringToVarType = []( eg_cpstr InString ) -> egsm_var_t
	{
		egsm_var_t Out = egsm_var_t::UNK;
		eg_string_crc TypeAsCrc = eg_string_crc( InString );

		switch_crc( TypeAsCrc )
		{
			case_crc( "bool" ) :
			case_crc( "eg_bool" ) :
				Out = egsm_var_t::BOOL;
				break;
			case_crc( "int" ) :
			case_crc( "eg_int" ) :
				Out = egsm_var_t::INT;
				break;
			case_crc( "real" ) :
			case_crc( "float" ) :
			case_crc( "eg_real" ) :
				Out = egsm_var_t::REAL;
				break;
			case_crc( "string_crc" ) :
			case_crc( "crc" ) :
			case_crc( "eg_string_crc" ) :
				Out = egsm_var_t::CRC;
				break;
			case_crc( "loc_text" ) :
				Out = egsm_var_t::LOC_TEXT;
				break;
			case_crc( "terminal" ):
				Out = egsm_var_t::TERMINAL;
				break;
			case_crc( "void" ) :
				Out = egsm_var_t::RETURN_VOID;
				break;
			case_crc( "number" ):
				Out = egsm_var_t::NUMBER;
				break;
			default:
				Out = egsm_var_t::UNK;
				break;
		}

		return Out;
	};

	eg_string_big Statement;
	while( GetStatement( Statement ) )
	{
		egParseVarDecl VarDecl = EGParse_ParseVarDecl( Statement );
		if( VarDecl.Result == EGPARSE_OKAY )
		{
			//EGLogf( eg_log_t::GameLibFromGame , "%s %s = %s" , VarDecl.Type.String() , VarDecl.Name.String() , VarDecl.Default.String() );
			eg_string_small VarName = VarDecl.Name;
			egsm_var_t VarType = StringToVarType( VarDecl.Type );
			egsm_var VarValue( CT_Clear );
			EGFixedArray<egsmVarDeclParmScr,egParseFuncInfo::MAX_PARMS> FnParms;

			eg_bool bSupported = true;
			switch( VarType )
			{
				case egsm_var_t::BOOL:
					VarValue = VarDecl.Default.ToBool();
					break;
				case egsm_var_t::INT:
					VarValue = VarDecl.Default.ToInt();
					break;
				case egsm_var_t::NUMBER:
				case egsm_var_t::REAL:
					VarValue = VarDecl.Default.ToFloat();
					break;
				case egsm_var_t::CRC:
				case egsm_var_t::LOC_TEXT:
					VarValue = eg_string_crc( VarDecl.Default );
					break;
				case egsm_var_t::RETURN_VOID:
					VarValue = egsm_var( CT_Clear );
					bSupported = VarDecl.bIsFunctionDecl;
					break;
				case egsm_var_t::TERMINAL:
					VarValue = egsm_var( CT_Clear );
					bSupported = VarDecl.bIsFunctionDecl;
					break;
				default:
					bSupported = false;
					break;
			}

			if( VarDecl.bIsFunctionDecl )
			{
				for( eg_size_t i=0; i<VarDecl.NumParms; i++ )
				{
					if( !FnParms.IsFull() )
					{
						egsmVarDeclParmScr NewParm;
						NewParm.Type = StringToVarType( VarDecl.Parms[i].ParmType );
						NewParm.Name = VarDecl.Parms[i].ParmName;
						NewParm.DefaultValue = VarDecl.Parms[i].Default;
						NewParm.bIsRef = VarDecl.Parms[i].bIsRef;
						NewParm.bIsConst = VarDecl.Parms[i].bIsConst;
						FnParms.Append( NewParm );
					}
				}
			}

			if( bSupported )
			{
				egsmVarDeclScr VarDeclSrc;
				VarDeclSrc.DeclType = VarDecl.bIsFunctionDecl ? egsm_var_decl_t::FUNCTION : egsm_var_decl_t::VAR;
				VarDeclSrc.VarType = VarType;
				VarDeclSrc.Name = VarName;
				VarDeclSrc.DefaultValue = VarValue;

				for( eg_size_t i=0; i<countof(VarDeclSrc.ParmInfo); i++ )
				{
					VarDeclSrc.ParmInfo[i].Type = egsm_var_t::UNK;
				}

				for( eg_size_t i=0; i<FnParms.Len(); i++ )
				{
					if( i < countof(VarDeclSrc.ParmInfo) )
					{
						VarDeclSrc.ParmInfo[i] = FnParms[i];
					}
				}

				Callback( VarDeclSrc );
			}
			else
			{
				EGLogf( eg_log_t::Error, __FUNCTION__ ": \"%s\" is not a supported type.", VarDecl.Type.String() );
				assert( false );
			}
		}
		else
		{
			// Might be better to show this error only if the statement contained some
			// non whitespace character.
			EGLogf( eg_log_t::Error, __FUNCTION__ ": Invalid statement \"%s\".", Statement.String() );
			assert( false );
		}
	}

	eg_bool bHadJunkAtEnd = false;
	for( eg_size_t i = 0; i < Statement.Len(); i++ )
	{
		if( !EGParse_IsWhiteSpace( Statement[i] ) )
		{
			bHadJunkAtEnd = true;
		}
	}

	if( bHadJunkAtEnd )
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": Invalid statement \"%s\".", Statement.String() );
		assert( false );
	}
}
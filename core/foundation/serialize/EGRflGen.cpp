// (c) 2018 Beem Media

#include "EGRflGen.h"
#include "EGPath2.h"
#include "EGToolsHelper.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGOsFile.h"
#include "EGParse.h"
#include "EGWnd.h"
#include "EGEdUtility.h"

static eg_d_string EGRflGen_GetEGSRC()
{
	eg_char Buffer[MAX_PATH];
	eg_bool bGot = GetEnvironmentVariableA( "EGSRC" , Buffer , countof(Buffer) ) < countof(Buffer);
	return bGot ? Buffer : ""; // EGToolsHelper_GetEnvVar("EGSRC").String();
}

static eg_d_string EGRflGen_GetEGOUT()
{
	eg_char Buffer[MAX_PATH];
	eg_bool bGot = GetEnvironmentVariableA( "EGOUT" , Buffer , countof(Buffer) ) < countof(Buffer);
	return bGot ? Buffer : ""; // EGToolsHelper_GetEnvVar("EGOUT").String();
}

static eg_d_string EGRflGen_ReadComment( const eg_char8* FileStr , const eg_size_t FileLen , eg_size_t& Pos )
{
	eg_d_string Out;

	Pos++;
	eg_bool bIsCpp = FileStr[Pos] == '/';
	Pos++;

	eg_bool bIgnoringLineBreak = false;

	for( ; Pos < FileLen; Pos++ )
	{
		eg_char Char = FileStr[Pos];

		if( bIsCpp )
		{
			if( Char == '\\' )
			{
				bIgnoringLineBreak = true;
				continue;
			}
			else if( Char == '\r' )
			{
				continue;
			}
			else if( Char == '\n' )
			{
				if( bIgnoringLineBreak )
				{
					bIgnoringLineBreak = false;
					continue;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			if( Char == '*' && FileStr[Pos+1] == '/' )
			{
				Pos++;
				break;
			}
		}

		Out.Append( Char );
	}

	return Out;
}

static eg_d_string EGRflGen_ReadPragma( const eg_char8* FileStr , const eg_size_t FileLen , eg_size_t& Pos )
{
	eg_d_string Out;

	eg_bool bIgnoringLineBreak = false;

	Pos++;
	for( ; Pos < FileLen; Pos++ )
	{
		eg_char Char = FileStr[Pos];

		if( Char == '\\' )
		{
			bIgnoringLineBreak = true;
			continue;
		}
		else if( Char == '\r' )
		{
			continue;
		}
		else if( Char == '\n' )
		{
			// This is a little more forgiving than a C++ compiler, it allows stuff to be written after the backslack
			// but it's file for this to work even on invalid C++ code. (The same is true for parsing comments.)
			if( bIgnoringLineBreak )
			{
				bIgnoringLineBreak = false;
				continue;
			}
			else
			{
				break;
			}
		}
		Out.Append( Char );
	}

	return Out;
}

static void EGRflGen_CleanWhitespaceAndComments( const eg_char8* FileStr , const eg_size_t FileLen , eg_d_string& CleanedFile )
{
	volatile eg_bool bLastWasWhitespace = true; // MSVCC BUG: Introduced 16.4.0 should not need volatile here
	eg_bool bInQuotes = false;
	eg_bool bNextIsSpecialChar = false;

	eg_int NextStringIndex = 0;

	for( eg_size_t i=0; i<FileLen; i++ )
	{
		eg_char8 Char = FileStr[i];
		eg_bool bIsWhitespace = EGParse_IsWhiteSpace( Char );

		if( bInQuotes )
		{
			if( bNextIsSpecialChar )
			{
				bNextIsSpecialChar = false;
			}
			else if( Char == '\\' )
			{
				bNextIsSpecialChar = true;
			}
			else if( Char == '\"' )
			{
				bInQuotes = false;
				CleanedFile.Append( EGString_Format( "STRING_%d" , NextStringIndex ) );
				NextStringIndex++;
			}
		}
		else if( bIsWhitespace )
		{
			if( !bLastWasWhitespace )
			{
				CleanedFile.Append( ' ' );
			}
		}
		else if( Char == '/' && (FileStr[i+1] == '*' || FileStr[i+1] == '/' ) )
		{
			eg_d_string Comment = EGRflGen_ReadComment( FileStr , FileLen , i );
			// EGLogf( eg_log_t::General , "Comment: %s" , *Comment );
		}
		else if( Char == '#' )
		{
			eg_d_string Pragma = EGRflGen_ReadPragma( FileStr , FileLen , i );
			// EGLogf( eg_log_t::General , "Pragma: %s" , *Pragma );
		}
		else
		{
			if( Char == '\"' )
			{
				bInQuotes = true;
			}
			else
			{
				CleanedFile.Append( Char );
			}
		}

		bLastWasWhitespace = bIsWhitespace;
	}
}

static void EGRflGen_CleanFunctionBodies( eg_d_string& FileStr )
{
	eg_d_string CleanStr;

	const eg_size_t StrLen = FileStr.Len();
	eg_bool bLastWasWhitespace = true; // We'll clean whitespace again.
	eg_int BracketDepth = 0;

	for( eg_size_t i=0; i<StrLen; i++ )
	{
		eg_char Char = FileStr[i];

		eg_bool bIsWhitespace = EGParse_IsWhiteSpace( Char );

		if( Char == '{' )
		{
			BracketDepth++;
		}
		else if( Char == '}' )
		{
			BracketDepth--;
			if( BracketDepth == 1 )
			{
				CleanStr.Append(";");
				continue;
			}
		}

		if( BracketDepth <= 1 )
		{
			if( bIsWhitespace && bLastWasWhitespace )
			{

			}
			else
			{
				CleanStr.Append( Char );
			}
		}

		bLastWasWhitespace = bIsWhitespace;
	}

	FileStr = std::move( CleanStr );
}

static void EGRflGen_BreakWords( const eg_d_string& Line , EGArray<eg_d_string>& Words )
{
	const eg_size_t StrLen = Line.Len();

	

	auto IsWordBreak = []( eg_char Char ) -> eg_bool
	{
		const eg_char WordBreaks[] = " =,{}:;[]<>";

		for( const eg_char& CompareChar : WordBreaks )
		{
			if( Char == CompareChar )
			{
				return true;
			}
		}

		return false;
	};

	eg_d_string CurrentWord;
	for( eg_size_t i=0; i<StrLen; i++ )
	{
		eg_char Char = Line[i];

		if( IsWordBreak( Char )  )
		{
			if( CurrentWord.Len() > 0 )
			{
				Words.Append(CurrentWord);
			}
			CurrentWord.Clear();
			if( Char != ' ' )
			{
				CurrentWord.Append( Char );
				Words.Append( CurrentWord );
				CurrentWord.Clear();
			}
		}
		else
		{
			CurrentWord.Append( Char );
		}
	}

	if( CurrentWord.Len() > 0 )
	{
		Words.Append( CurrentWord );
	}
}

static void EGRflGen_GetTypeInfo( const eg_d_string& StructDecl , egRflGenStruct& Out )
{
	EGArray<eg_d_string> DeclWords;
	EGRflGen_BreakWords( StructDecl , DeclWords );

	if( DeclWords.IsValidIndex(1) )
	{
		if( DeclWords[1].Equals( "class" ) || DeclWords[1].Equals( "struct" ) )
		{
			Out.Type = DeclWords[1].Equals( "class" ) ? eg_rfl_gen_t::Class : eg_rfl_gen_t::Struct;
			if( DeclWords.IsValidIndex(2) )
			{
				Out.Name = DeclWords[2];
			}

			// Check for inheritance
			eg_bool bNextIsInherited = false;
			for( eg_size_t i=3; i<DeclWords.Len(); i++ )
			{
				if( DeclWords[i].Equals( "egprop" ) )
				{
					bNextIsInherited = true;
				}
				else if( bNextIsInherited )
				{
					bNextIsInherited = false;
					Out.ParentClasses.Append( DeclWords[i] );
				}
			}
		}
		else if( DeclWords[1].Equals( "enum" ) )
		{
			Out.Type = eg_rfl_gen_t::Enum;
			if( DeclWords.IsValidIndex(2) )
			{
				if( DeclWords[2].Equals( "class" ) )
				{
					if( DeclWords.IsValidIndex(3) )
					{
						Out.Name = DeclWords[3];
					}
				}
				else
				{
					Out.Name = DeclWords[2];
				}
			}
		}
	}
}

static void EGRflGen_GetEnumInfo( const eg_d_string& StructDecl , egRflGenStruct& Out )
{
	// For enums we basically want to get through = signs and assignemnts, just get the names
	// which will be the firs thing after { and anything after a comma that isn't the last one.
	EGArray<eg_d_string> Words;
	EGRflGen_BreakWords( StructDecl , Words );
	eg_bool bNextIsValue = false; // First word after a comma or { is gonna be a value.
	for( const eg_d_string& Word : Words )
	{
		if( Word.Equals(",") || Word.Equals("{") )
		{
			bNextIsValue = true;
		}
		else if( Word.Equals( "}"  ) )
		{
			break;
		}
		else if( bNextIsValue )
		{
			bNextIsValue = false;

			egRflGenProp Prop;
			Prop.VarType = Out.Name;
			Prop.VarName = Word;
			Out.Properties.Append( Prop );
		}
	}
}

static void EGRflGen_GetStructInfo( const eg_d_string& StructDecl , egRflGenStruct& Out )
{
	// For a struct we basically search for the egprop keyword. Then the next thing we find
	// will be the type and the next thing after that will be the variable name.
	// types get a little complicated because arrays use triangle brackets which throw things
	// off.
	EGArray<eg_d_string> Words;
	EGRflGen_BreakWords( StructDecl , Words );

	eg_bool bNextIsPropType = false;
	eg_bool bNextIsVarName = false;
	eg_int TemplateDepth = 0;
	egRflGenProp NextProp;
	for( const eg_d_string& Word : Words )
	{
		if( Word.EqualsI( ";" ) )
		{
			// Reset everything if we hit a semicolon.
			NextProp.VarType = "";
			NextProp.VarName = "";
			bNextIsPropType = false;
			bNextIsVarName = false;
			TemplateDepth = 0;
		}
		else if( TemplateDepth > 0 )
		{
			NextProp.VarType.Append( *Word );
			if( Word.EqualsI("<") )
			{
				TemplateDepth++;
			}
			else if( Word.Equals(">") )
			{
				TemplateDepth--;
			}

			if( TemplateDepth == 0 )
			{
				bNextIsVarName = true;
			}
		}
		else if( Word.Equals( "egprop" ) )
		{
			bNextIsPropType = true;
		}
		else if( bNextIsPropType )
		{
			bNextIsPropType = false;
			bNextIsVarName = true;
			NextProp.VarType = Word;
		}
		else if( bNextIsVarName )
		{
			if( Word.Equals( "<" ) )
			{
				NextProp.VarType.Append( *Word );
				TemplateDepth++;
			}
			else
			{
				bNextIsVarName = false;
				NextProp.VarName = Word;
				Out.Properties.Append( NextProp );
				NextProp.VarType = "";
				NextProp.VarName = "";
			}
		}
	}
}

static egRflGenStruct EGRflGen_ReadStruct( const eg_d_string& FileStr , eg_size_t& Pos )
{
	const eg_size_t StrLen = FileStr.Len();

	egRflGenStruct Out;

	// At this point FileStr is clean of everything, so we just read until the next closing brace
	eg_d_string StructDecl;
	eg_d_string StructProps;

	eg_bool bFoundProps = false;

	for( ; Pos < StrLen; Pos++ )
	{
		eg_char Char = FileStr[Pos];

		if( Char == '{' )
		{
			bFoundProps = true;
		}

		if( bFoundProps )
		{
			StructProps.Append( Char );
		}
		else
		{
			StructDecl.Append( Char );
		}
		
		if( Char == '}' )
		{
			break;
		}
	}

	// EGLogf( eg_log_t::General , "RFL: %s" , *StructDecl );
	// EGLogf( eg_log_t::General , "RFL: %s" , *StructProps );

	EGRflGen_GetTypeInfo( StructDecl , Out );

	switch( Out.Type )
	{
		case eg_rfl_gen_t::Unknown:
			break;
		case eg_rfl_gen_t::Class:
		case eg_rfl_gen_t::Struct:
			EGRflGen_GetStructInfo( StructProps , Out );
			break;
		case eg_rfl_gen_t::Enum:
			EGRflGen_GetEnumInfo( StructProps , Out );
			break;		
	}

	return Out;
} 

static void EGRflGen_ParseFile( const eg_d_string& FileStr , egRflGenInfo& Out )
{
	const eg_size_t StrLen = FileStr.Len();

	eg_d_string ReflectName = "egreflect ";

	for( eg_size_t Pos = 0; Pos < StrLen; Pos++ )
	{
		if( EGString_EqualsCount(* ReflectName , &FileStr[Pos] , ReflectName.Len() ) )
		{
			egRflGenStruct RflStruct = EGRflGen_ReadStruct( FileStr , Pos );
			if( RflStruct.Name.Len() > 0 && RflStruct.Type != eg_rfl_gen_t::Unknown )// RflStruct.ParentClasses.HasItems() || RflStruct.Properties.HasItems() )
			{
				Out.Structs.Append( RflStruct );
			}
		}
	}
}

static void EGRflGen_GetOutputFilenames( eg_cpstr16 Filename , eg_d_string& SourceFilename , eg_d_string& H_Filename , eg_d_string& HPP_Filename , eg_d_string& LINK_Filename )
{
	SourceFilename = EGPath2_CleanPath( EGString_ToMultibyte(Filename) , '/' );
	eg_d_string RelativeFilename = EGPath2_GetRelativePathTo( *SourceFilename , *EGRflGen_GetEGSRC() );
	egPathParts2 RelativeParts = EGPath2_BreakPath( *RelativeFilename );
	eg_d_string ProjectName;
	if( RelativeParts.Folders.IsValidIndex(0) && RelativeParts.Folders[0].EqualsI(L"core") )
	{
		if( RelativeParts.Folders.IsValidIndex(1) && RelativeParts.Folders[1].EqualsI(L"tools") )
		{
			if( RelativeParts.Folders.IsValidIndex(2) && RelativeParts.Folders[2].Equals(L"EGEdLib") )
			{
				ProjectName = "edlib";
			}
			else
			{
				ProjectName = "tools";
			}
		}
		else if( RelativeParts.Folders.IsValidIndex(1) && RelativeParts.Folders[1].EqualsI(L"foundation") )
		{
			ProjectName = "foundation";
		}
		else
		{
			ProjectName = "core";
		}
	}
	else if( RelativeParts.Folders.IsValidIndex(0) && RelativeParts.Folders[0].EqualsI(L"games") )
	{
		ProjectName = RelativeParts.Folders.IsValidIndex(1) ? *RelativeParts.Folders[1] : L"-unknown-";
	}
	else
	{
		ProjectName = "-unknown-";
	}

	H_Filename= EGRflGen_GetEGOUT();
	H_Filename.Append( "/generated_code/reflection/");
	H_Filename = EGPath2_CleanPath( *H_Filename , '/' );
	H_Filename.Append( *ProjectName );
	H_Filename.Append( "/" );
	EGOsFile_CreateDirectory( *H_Filename );
	LINK_Filename = H_Filename;
	LINK_Filename.Append( *ProjectName );
	LINK_Filename.Append( ".link.reflection.hpp" );
	H_Filename.Append( *RelativeParts.GetFilename8( false ) );
	HPP_Filename = H_Filename;
	HPP_Filename.Append(".reflection.hpp");
	H_Filename.Append(".reflection.h");
}

static eg_bool EGRflGen_WriteH( const eg_d_string& H_Filename , const eg_d_string& SourceFilename , const egRflGenInfo& GenInfo )
{
	EGFileData OutputFile( eg_file_data_init_t::HasOwnMemory );
	OutputFile.WriteStr8( EGString_Format( "// (c) Beem Media - Generated Code Header for \"%s\"\r\n\r\n#pragma once\r\n\r\n" , *EGPath2_GetFilename(*SourceFilename) ) );
	OutputFile.WriteStr8( "#include \"EGReflection.h\"\r\n\r\n" );

	for( const egRflGenStruct& Struct : GenInfo.Structs )
	{
		switch( Struct.Type )
		{
			case eg_rfl_gen_t::Unknown:
				break;
			case eg_rfl_gen_t::Class:
			case eg_rfl_gen_t::Struct:
			{
				eg_d_string ClassDecl = Struct.Type == eg_rfl_gen_t::Class ? "class" : "struct";
				if( Struct.ParentClasses.Len() > 0 )
				{
					if( Struct.ParentClasses.Len() == 1 )
					{
						OutputFile.WriteStr8( EGString_Format( "EGRFL_DECL_CHILD_STRUCT( %s , %s , %s )\r\n" , *Struct.Name , *Struct.ParentClasses[0] , *ClassDecl ) );
					}
					else
					{
						EGLogf( eg_log_t::Error , "Cannot reflect %s, only one parent egprop is supported." , *Struct.Name );
					}
				}
				else
				{
					OutputFile.WriteStr8( EGString_Format( "EGRFL_DECL_STRUCT( %s , %s )\r\n" ,  *Struct.Name , *ClassDecl ) );
				}
			} break;
			case eg_rfl_gen_t::Enum:
				OutputFile.WriteStr8( EGString_Format( "EGRFL_DECL_ENUM( %s )\r\n" , *Struct.Name ) );
				break;
		}
	}

	eg_bool bWroteFile = EGEdUtility_SaveIfUnchanged( *H_Filename , OutputFile );
	return bWroteFile;
}

static eg_bool EGRflGen_WriteHPP( const eg_d_string& HPP_Filename , const eg_d_string& H_Filename , const eg_d_string& SourceFilename , const egRflGenInfo& GenInfo , eg_bool* bWasNew )
{
	if( bWasNew )
	{
		*bWasNew = !EGOsFile_DoesFileExist( *HPP_Filename );
	}

	EGFileData OutputFile( eg_file_data_init_t::HasOwnMemory );
	OutputFile.WriteStr8( EGString_Format( "// (c) Beem Media - Generated Code Definition for \"%s\"\r\n\r\n" , *EGPath2_GetFilename(*SourceFilename) ) );
	OutputFile.WriteStr8( "#if defined( EGRFL_INCLUDE_SOURCE_HEADER )\r\n" );
	OutputFile.WriteStr8( EGString_Format( "#include \"%s\"\r\n" , *EGPath2_GetFilename(*SourceFilename) ) );
	OutputFile.WriteStr8( EGString_Format( "#include \"%s\"\r\n" , *EGPath2_GetFilename(*H_Filename) ) );
	OutputFile.WriteStr8( "#endif // defined( EGRFL_INCLUDE_SOURCE_HEADER )\r\n" );
	OutputFile.WriteStr8( "\r\n" );

	for( const egRflGenStruct& Struct : GenInfo.Structs )
	{
		switch( Struct.Type )
		{
		case eg_rfl_gen_t::Unknown:
			break;
		case eg_rfl_gen_t::Class:
		case eg_rfl_gen_t::Struct:
		{
			if( Struct.ParentClasses.Len() > 0 )
			{
				if( Struct.ParentClasses.Len() == 1 )
				{
					OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_CHILD_STRUCT_BEGIN( %s , %s )\r\n" , *Struct.Name , *Struct.ParentClasses[0] ) );
				}
			}
			else
			{
				OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_STRUCT_BEGIN( %s )\r\n" , *Struct.Name ) );
			}
			for( const egRflGenProp& Prop : Struct.Properties )
			{
				eg_bool bIsTemplate = false;
				eg_bool bIsNestedTemplate = false;
				eg_int TemplateDepth = 0;
				eg_d_string TemplateType;
				eg_d_string TemplateVarType;

				for( eg_size_t i=0; i<Prop.VarType.Len(); i++ )
				{
					eg_char Char = Prop.VarType[i];
					if( Char == '<' )
					{
						TemplateDepth++;
						bIsTemplate = true;
						if( TemplateDepth > 1 )
						{
							bIsNestedTemplate = true;
							break;
						}
					}
					else if( Char == '>' )
					{
						TemplateDepth--;
					}
					else if( TemplateDepth == 1 )
					{
						TemplateVarType.Append( Char );
					}
					else
					{
						TemplateType.Append( Char );
					}
				}

				if( !bIsTemplate )
				{
					OutputFile.WriteStr8( EGString_Format( "\tEGRFL_IMPL_STRUCT_ITEM( %s , %s , %s )\r\n" , *Struct.Name , *Prop.VarType , *Prop.VarName ) );
				}
				else if( bIsNestedTemplate )
				{
					EGLogf( eg_log_t::Error , "Cannot reflect %s, nested templates are not supported." , *Prop.VarType );
				}
				else if( TemplateType.Equals("EGArray") )
				{
					OutputFile.WriteStr8( EGString_Format( "\tEGRFL_IMPL_STRUCT_ITEM_ARRAY( %s , %s , %s )\r\n" , *Struct.Name , *TemplateVarType , *Prop.VarName ) );
				}
				else
				{
					EGLogf( eg_log_t::Error , "Cannot reflect %s, templates of type %s are not supported." , *Prop.VarType , *TemplateType );
				}
			}
			if( Struct.ParentClasses.Len() > 0 )
			{
				if( Struct.ParentClasses.Len() == 1 )
				{
					OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_CHILD_STRUCT_END( %s , %s )\r\n\r\n" , *Struct.Name , *Struct.ParentClasses[0] ) );
				}
			}
			else
			{
				OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_STRUCT_END( %s )\r\n\r\n" , *Struct.Name ) );
			}
		} break;
		case eg_rfl_gen_t::Enum:
			OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_ENUM_BEGIN( %s )\r\n" , *Struct.Name ) );
			for( const egRflGenProp& Prop : Struct.Properties )
			{
				OutputFile.WriteStr8( EGString_Format( "\tEGRFL_IMPL_ENUM_ITEM( %s , %s )\r\n" , *Struct.Name , *Prop.VarName ) );
			}
			OutputFile.WriteStr8( EGString_Format( "EGRFL_IMPL_ENUM_END( %s )\r\n\r\n" , *Struct.Name ) );
			break;
		}
	}

	eg_bool bWroteFile = EGEdUtility_SaveIfUnchanged( *HPP_Filename , OutputFile );
	return bWroteFile;
}

static eg_bool EGRflGen_WriteLINK( const eg_d_string& LINK_Filename , const eg_d_string& HPP_Filename )
{
	eg_d_string AppendData = EGString_Format( "#include \"%s\"\r\n" , *EGPath2_GetFilename( *HPP_Filename ) );
	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	if( EGOsFile_DoesFileExist( *LINK_Filename ) )
	{
		EGLibFile_OpenFile( *LINK_Filename , eg_lib_file_t::OS , FileData );
		FileData.Seek( eg_file_data_seek_t::End , 0 );
	}
	else
	{
		FileData.WriteStr8( "// (c) Beem Media - Generated LINK Code\r\n\r\n" );
	}
	FileData.WriteStr8( *AppendData );
	eg_bool bWroteFile = EGLibFile_SaveFile( *LINK_Filename , eg_lib_file_t::OS , FileData );
	return bWroteFile;
}

eg_bool EGRflGen_ProcessFile( eg_cpstr16 Filename )
{
	eg_d_string SourceFilename;
	eg_d_string H_Filename;
	eg_d_string HPP_Filename;
	eg_d_string LINK_Filename;
	EGRflGen_GetOutputFilenames( Filename , SourceFilename , H_Filename , HPP_Filename , LINK_Filename );

	EGLogf( eg_log_t::Verbose , "Generating Reflection for \"%s\"" , *SourceFilename );

	EGFileData SourceFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bOpenedFile = EGLibFile_OpenFile( *SourceFilename , eg_lib_file_t::OS , SourceFile );
	if( !bOpenedFile )
	{
		EGLogf( eg_log_t::Error , "Could not open \"%s\"." , *SourceFilename );
		return false;
	}

	SourceFile.Seek( eg_file_data_seek_t::End  , 0 );
	eg_char8 NullTerm = '\0';
	SourceFile.Write( &NullTerm , sizeof(NullTerm) );
	SourceFile.Seek( eg_file_data_seek_t::Begin , 0 );

	eg_d_string CleanFile;
	EGRflGen_CleanWhitespaceAndComments( SourceFile.GetDataAs<eg_char8>() , SourceFile.GetSize() - 1 , CleanFile );
	EGRflGen_CleanFunctionBodies( CleanFile );
	egRflGenInfo RflInfo;
	EGRflGen_ParseFile( CleanFile , RflInfo );

	eg_bool bSuccess = true;
	if( RflInfo.HasInfo() )
	{
		RflInfo.PrintToLog( eg_log_t::Verbose );

		eg_bool bWasHPPNew = false;
		eg_bool bWroteH = EGRflGen_WriteH( H_Filename , SourceFilename , RflInfo );
		eg_bool bWroteHPP = EGRflGen_WriteHPP( HPP_Filename , H_Filename , SourceFilename , RflInfo , &bWasHPPNew );
		if( bWasHPPNew )
		{
			eg_bool bWroteLINK = EGRflGen_WriteLINK( LINK_Filename , HPP_Filename );
		}
		else
		{
			EGLogf( eg_log_t::Verbose , "\"%s\" already added to \"%s\"." , *HPP_Filename , *LINK_Filename );
		}
	}
	else
	{
		EGLogf( eg_log_t::Verbose , "No reflection data." );
	}
	return bSuccess;
}

static eg_bool EGRflGen_CopyFoundation()
{
	// We copy the foundation classes right back into the foundation directory.
	EGEdUtility_Init();

	eg_bool bSavedAll = true;

	eg_d_string SourceRoot = EGPath2_CleanPath( EGString_Format( "%s/generated_code/reflection/foundation/" , *EGRflGen_GetEGOUT() ) , '/' );
	eg_d_string DestRoot = EGPath2_CleanPath( EGString_Format( "%s/core/foundation/reflection/" , *EGRflGen_GetEGSRC() ) , '/' );

	EGLogf( eg_log_t::Verbose , "Copying \"%s\" -> \"%s\"" , *SourceRoot , *DestRoot );
	EGOsFile_CreateDirectory( *DestRoot );

	EGArray<eg_d_string> FoundationFiles;
	EGOsFile_FindAllFiles( *SourceRoot , nullptr , FoundationFiles );

	for( const eg_d_string& File : FoundationFiles )
	{
		eg_d_string FullSource = SourceRoot + File;
		eg_d_string FullDest = DestRoot + File;
		EGLogf( eg_log_t::Verbose , "Copying \"%s\" -> \"%s\"..." , *FullSource , *FullDest );
		EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
		eg_bool bOpened = EGToolsHelper_OpenFile( *FullSource , FileData );
		if( bOpened )
		{
			bSavedAll = EGEdUtility_SaveIfUnchanged( *FullDest , FileData , true ) && bSavedAll;
		}
	}

	EGEdUtility_Deinit();
	return bSavedAll;
}

static eg_bool EGRflGen_ProcessAll()
{
	EGOsFileWantsFileFn WantsFile = []( eg_cpstr Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
	{
		unused( FileAttr );

		egPathParts2 PathParts = EGPath2_BreakPath( Filename );
		if( PathParts.Ext.Equals(L"h") )
		{
			return true;
		}

		return false;
	};

	auto GetHeaderForPath = [&WantsFile]( eg_cpstr Path , EGArray<eg_d_string>& Out ) -> void
	{
		EGArray<eg_d_string> PathHeaderFiles;
		EGOsFile_FindAllFiles( Path , WantsFile , PathHeaderFiles );
		for( eg_d_string& Filename : PathHeaderFiles )
		{
			Out.Append( EGPath2_CleanPath( EGString_Format( "%s/%s" , Path , *Filename ) , '/' ) );
		}
	};

	EGLogf( eg_log_t::General , "Generating reflection data..." );

	EGArray<eg_d_string> AllHeaders;
	auto AddRoot = [&AllHeaders,&GetHeaderForPath]( eg_cpstr Src ) -> void
	{
		eg_d_string Root = EGString_Format("%s/%s" , *EGRflGen_GetEGSRC() , Src );
		GetHeaderForPath( *Root , AllHeaders );
	};
	AddRoot( "core/foundation/" );
	AddRoot( "core/engine/" );
	AddRoot( "core/audio/" );
	AddRoot( "games/" );
	AddRoot( "core/tools/EGEdLib/" );
	AddRoot( "core/tools/EGWndTest/" );

	eg_bool bAllSucceeded = true;
	for( const eg_d_string& Filename : AllHeaders )
	{
		eg_bool bSucceeded = EGRflGen_ProcessFile( EGString_ToWide(*Filename) );\
		if( !bSucceeded )
		{
			bAllSucceeded = false;
		}
	}

	eg_bool bCopiedFoundation = EGRflGen_CopyFoundation();

	return bAllSucceeded && bCopiedFoundation;
}

int EGRflGen_main( int argc , char* argv[] )
{
	unused( argc , argv );

	eg_bool bNextIsFile = false;
	eg_d_string Filename;

	EGWndAppParms AppParms;
	EGWnd_GetAppParmsFromCommandLine( AppParms );

	eg_bool bProcessedFile = false;

	if( AppParms.ContainsType("-file") )
	{
		bProcessedFile = EGRflGen_ProcessFile( EGString_ToWide(*AppParms.GetParmValue("-file")) );
	}
	else if( AppParms.ContainsType("-all") )
	{
		bProcessedFile = EGRflGen_ProcessAll();
	}

	return bProcessedFile ? 0 : -1;
}

void egRflGenInfo::PrintToLog( eg_log_channel LogChannel )
{
	EGLogf( LogChannel , "Reflection Info: " );
	for( const egRflGenStruct& Struct : Structs )
	{
		EGLogf( LogChannel , "\t%s" , *Struct.Name );
		for( const eg_d_string& Parent : Struct.ParentClasses )
		{
			EGLogf( LogChannel , "\t\tpublic: %s" , *Parent );
		}
		EGLogf( LogChannel , "\t{" );
		for( const egRflGenProp& Prop : Struct.Properties )
		{
			if( Struct.Type == eg_rfl_gen_t::Enum )
			{
				EGLogf( LogChannel , "\t\t%s ," , *Prop.VarName );
			}
			else
			{
				EGLogf( LogChannel , "\t\t%s %s;" , *Prop.VarType , *Prop.VarName );
			}
		}
		EGLogf( LogChannel , "\t}" );
	}
}

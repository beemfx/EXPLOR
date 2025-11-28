// (c) 2017 Beem Media

#include "EGMakeCodeLoc.h"
#include "EGMake.h"
#include "EGWnd.h"
#include "EGFileData.h"
#include "EGLocCompiler.h"
#include "EGParse.h"
#include "EGWindowsAPI.h"
#include "EGSmEdVarMgr.h"
#include "EGSmFile2.h"

enum class eg_codeloc_process_t
{
	SourceCode,
	XmlAsset,
	EGSM,
};

static const eg_char CodeLocCall[] = "eg_loc";
static const eg_size_t CodeLocCallSize = countof(CodeLocCall)-1;

class EGXmlAssetLoc : public IXmlBase
{
private:

	struct egSerializePair
	{
		eg_string_big Name;
		eg_string_big Value;
	};

	struct egSerializeStruct
	{
		EGArray<egSerializePair> Values;
	};
	
private:
	
	EGLocCompiler& m_LocCompiler;
	EGArray<egSerializeStruct> m_StructStack;

public:

	EGXmlAssetLoc( const EGFileData& SourceFile , EGLocCompiler& LocCompiler )
	: m_LocCompiler( LocCompiler )
	{
		XMLLoad( SourceFile.GetData() , SourceFile.GetSize() , "XML Asset" );
	}

private:

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) override final
	{
		unused( Tag );

		if( Tag == "struct" )
		{
			m_StructStack.Push( egSerializeStruct() );
		}
		else if( Tag == "property" )
		{
			egSerializePair Pair;
			Pair.Name = AttGet.GetString( "name" );
			Pair.Value = AttGet.GetString( "value" );
			if( m_StructStack.HasItems() )
			{
				m_StructStack.Top().Values.Append( Pair );
			}
		}

		const eg_size_t NumAtts = AttGet.Num();
		for( eg_size_t i=0; i<NumAtts; i++ )
		{
			eg_string_big AttName = AttGet.GetAttributeNameByIndex( i );
			if( EGString_EndsWithI( AttName , "_enus" ) )
			{
				eg_string_big AttValue = AttGet.GetAttributeValueByIndex( i );
				if( AttValue.Len() > 0 )
				{
					eg_string_big KeyName = AttName;
					KeyName.ClampEnd( 5 );
					eg_string_big KeyValue = AttGet.GetString( KeyName );
					if( KeyValue.Len() > 0 )
					{
						AddEntry( KeyValue , AttValue );
					}
				}
			}
		}
	}

	virtual void OnTagEnd( const eg_string_base& Tag )
	{
		if( Tag == "struct" )
		{
			EGArray<egSerializePair> NamesWanted;

			const egSerializeStruct& Struct = m_StructStack.Top();

			// Get all properties ending with _enus
			for( const egSerializePair& Property : Struct.Values )
			{
				if( EGString_EndsWithI( Property.Name , "_enus" ) )
				{
					NamesWanted.Append( Property );
				}
			}

			for( egSerializePair& Wanted : NamesWanted )
			{
				eg_string_big PropName = Wanted.Name;
				PropName.ClampEnd( 5 );

				for( const egSerializePair& Property : Struct.Values )
				{
					if( PropName == Property.Name )
					{
						AddEntry( Property.Value , Wanted.Value );
					}
				}
			}

			m_StructStack.Pop();
		}
	}

	virtual eg_cpstr XMLObjName() const override final { return "EGXmlAssetLoc"; }

	void AddEntry( const eg_string_big& Key , const eg_string_big& Value )
	{
		if( Key.Len() > 0 && Value.Len() > 0 )
		{
			EGLogf( eg_log_t::General , "%s = \"%s\"" , Key.String() , Value.String() );

			m_LocCompiler.AddOrReplaceEntry( Key , EGString_ToWide( Value ) );
		}
	}
};

static eg_bool EGMakeCodeLoc_LocalizeXmlAssetFile( EGLocCompiler& LocCompiler , const EGFileData& SourceFile )
{
	EGXmlAssetLoc AssetLoc( SourceFile , LocCompiler );

	return true;
}


static eg_bool EGMakeCodeLoc_LocalizeEGSMFile( EGLocCompiler& LocCompiler , const EGFileData& SourceFile , eg_cpstr Filename )
{
	auto PrepareForEnExport =[]( EGSmFile2& EgSmFile ) -> void
	{
		for( eg_size_t i=0; i<EgSmFile.GetNodeCount(); i++ )
		{
			egsmNodeScr& Node = EgSmFile.GetNode( i );
			if( Node.Type == egsm_node_t::NATIVE_EVENT )
			{
				egsmVarDeclScr FnDecl = EGSmEdVarMgr_GetFunctionInfo( eg_string_crc(Node.Parms[0]) );
				if( FnDecl.DeclType == egsm_var_decl_t::FUNCTION )
				{
					for( eg_size_t i=0; i<countof(FnDecl.ParmInfo); i++ )
					{
						if( FnDecl.ParmInfo[i].Type == egsm_var_t::LOC_TEXT )
						{
							Node.EnParmIndex = i+1; // Plus one because the first parm in the state is the event name.
							break;
						}
					}
				}
			}
		}
	};

	EGSmEdVarMgr_InitForFile( EGString_ToWide( Filename ) );

	EGSmFile2 EgSmFile;
	EgSmFile.LoadFromMemFile( SourceFile );
	PrepareForEnExport( EgSmFile );
	EgSmFile.SaveEnLoc( LocCompiler );

	EGSmEdVarMgr_Deinit();
	return true;
}

static eg_bool EGMakeCodeLoc_LocalizeSourceFile( EGLocCompiler& LocCompiler , const EGFileData& SourceCode )
{
	struct egPairing
	{
		eg_string_small Key;
		eg_string_big   Text;
	};

	EGArray<egPairing> ToExport;

	const eg_char* Code = reinterpret_cast<const eg_char*>(SourceCode.GetData());
	const eg_size_t Size = SourceCode.GetSize();

	auto FindEnd = [&Code,&Size]( eg_size_t Start ) -> eg_size_t
	{
		eg_size_t Out = Start+CodeLocCallSize;

		eg_bool bInQuotes = false;
		eg_bool bFoundOpenParenthesis = false;

		for( eg_size_t Search = Start+CodeLocCallSize; Code[Search] != '\0'; Search++ )
		{
			const eg_char c = Code[Search];

			if( c == '(' )
			{
				bFoundOpenParenthesis = true;
			}

			if( !bFoundOpenParenthesis )
			{
				if( !EGParse_IsWhiteSpace( c ) )
				{
					break;
				}
			}

			if( c == '\"' && Code[Search-1] != '\\' )
			{
				bInQuotes = !bInQuotes;
			}
			
			if( c == ')' && !bInQuotes )
			{
				Out = Search;
				break;
			}
		}

		return Out;
	};

	auto HandleLine = [&ToExport]( eg_cpstr Line ) -> void
	{
		// EGLogf( eg_log_t::Warning , "Line: %s" , Line );

		egParseFuncInfo ParseInfo;
		EGPARSE_RESULT Res = EGParse_ParseFunction( Line , &ParseInfo );
		if( EGPARSE_OKAY == Res )
		{
			egPairing Pair = { ParseInfo.Parms[0] , ParseInfo.Parms[1] };
			if( Pair.Key != "key" && Pair.Text != "en" ) // Weird edge case of the define.
			{
				eg_string_big LogFriendly = CT_Clear;
				eg_string_base::ToXmlFriendly( Pair.Text , LogFriendly );
				EGLogf( eg_log_t::General , "%s = \"%s\"" , Pair.Key.String() , LogFriendly.String() );
				ToExport.Append( Pair );
			}
		}
	};

	for( eg_size_t i=0; i<Size; i++ )
	{
		if( EGString_EqualsCount( &Code[i] , CodeLocCall , CodeLocCallSize ) )
		{
			eg_size_t Start = i;
			eg_size_t End = FindEnd( Start );

			if( End > (Start+CodeLocCallSize) )
			{
				eg_char Line[1024];
				eg_size_t LineLen = (End - Start) + 1;
				LineLen = EG_Min( countof(Line)-2 , LineLen );
				EGMem_Copy( Line , &Code[Start] , LineLen );
				Line[LineLen] = '\0';
				HandleLine( Line );
			}
			i = End;
		}
	}


	for( const egPairing& Pair : ToExport )
	{
		LocCompiler.AddOrReplaceEntry( Pair.Key , EGString_ToWide(Pair.Text) );
	}

	return true;
}

static eg_bool EGMakeCodeLoc_ProcessFile( eg_cpstr InFilename , eg_cpstr OutFilename , eg_codeloc_process_t ProcessType )
{
	EGFileData SourceFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bSuccess = EGMake_ReadInputFile( InFilename , SourceFile );
	
	if( ProcessType == eg_codeloc_process_t::SourceCode )
	{
		// Add a null terminating zero so we can treat this as a string when parsing.
		eg_char Nullz = 0;
		SourceFile.Write( &Nullz , sizeof(Nullz) );
	}

	if( bSuccess )
	{
		EGLocCompiler LocCompiler;
		EGFileData CurFile( eg_file_data_init_t::HasOwnMemory );
		EGMake_ReadInputFile( OutFilename , CurFile );

		if( CurFile.GetSize() == 0 )
		{
			LocCompiler.InitAsEnUS();
		}
		else
		{
			LocCompiler.LoadLocFile( CurFile , ProcessType == eg_codeloc_process_t::SourceCode ? "CODE-LOC Text" : "ASSET-LOC Text" );
		}

		switch( ProcessType )
		{
		case eg_codeloc_process_t::SourceCode:
			bSuccess = EGMakeCodeLoc_LocalizeSourceFile( LocCompiler , SourceFile );
			break;
		case eg_codeloc_process_t::XmlAsset:
			bSuccess = EGMakeCodeLoc_LocalizeXmlAssetFile( LocCompiler , SourceFile );
			break;
		case eg_codeloc_process_t::EGSM:
			bSuccess = EGMakeCodeLoc_LocalizeEGSMFile( LocCompiler , SourceFile , InFilename );
			break;
		}

		if( bSuccess )
		{
			EGFileData Out( eg_file_data_init_t::HasOwnMemory );
			LocCompiler.SaveXml( Out );
			bSuccess = EGMake_WriteOutputFile( OutFilename , Out );
		}
	}

	return bSuccess;
}

int EGMakeCodeLoc_main( int argc , char *argv[] )
{
	unused( argc , argv );

	enum class eg_file_t
	{
		None,
		File,
		Directory,
	};

	auto GetFileType = []( eg_cpstr Path ) -> eg_file_t
	{
		DWORD dwAttrib = GetFileAttributesA( Path );

		if( dwAttrib == INVALID_FILE_ATTRIBUTES )
		{
			return eg_file_t::None;
		}
		else if( (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0 )
		{
			return eg_file_t::Directory;
		}
		
		return eg_file_t::File;
	};
	
	EGWndAppParms AppParms;
	EGWnd_GetAppParmsFromCommandLine( AppParms );

	auto GetProcessType = [&AppParms]() -> eg_codeloc_process_t
	{
		eg_codeloc_process_t Out = eg_codeloc_process_t::SourceCode;
		if( AppParms.GetParmValue( "-type" ).EqualsI( "XMLASSET" ) )
		{
			Out = eg_codeloc_process_t::XmlAsset;
		}
		else if( AppParms.GetParmValue( "-type" ).EqualsI( "EGSM" ) )
		{
			Out = eg_codeloc_process_t::EGSM;
		}
		return Out;
	};

	eg_string_big InFilename = AppParms.GetParmValue( "-in" );
	eg_string_big OutFilename = AppParms.GetParmValue( "-out" );

	eg_file_t InFileType = GetFileType( InFilename );
	eg_codeloc_process_t ProcessType = GetProcessType();

	eg_bool bSuccess = false;
	
	if( InFileType == eg_file_t::File )
	{
		bSuccess = EGMakeCodeLoc_ProcessFile( InFilename , OutFilename , ProcessType );
	}

	return bSuccess ? 0 : -1;
}

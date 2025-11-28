/******************************************************************************
egfx.cpp - Emergence Shader compiler.

The process goes like this. The file is searched for all functions beginning
with VS. These are compiled as vertex shaders, and output as
{{outputfilename}}{{functionname}}.evs.
Where functionname is the name of the function following VS.

So if the file were basicshader.fx then VS_Pass1 -> basicshader_Pass1.evs.

Similarly all functions starting with PS are searched for, and they are output
with the eps extions.

(c) 2012 Beem Software
******************************************************************************/

#define EGMAKE_HLSL_USE_D3DCOMPILE 1

#include "EGMake.h"
#include "EGParse.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGExtProcess.h"
#include "EGToolsHelper.h"
#include "EGOsFile.h"
#include "EGPath2.h"

#if EGMAKE_HLSL_USE_D3DCOMPILE
#pragma warning( push )
#include <d3dcompiler.h>
#pragma warning( pop )
#endif

#include <regex>

static eg_bool EGR_FX_CompileInternal( eg_cpstr16 SourceFile , eg_cpstr16 OutputFile , eg_cpstr8 Entry , eg_cpstr8 Target , const EGArray<eg_d_string16>& IncludeRoots , const D3D_SHADER_MACRO* Macros );

eg_bool EGR_FX_Matches(const eg_string_base& _this, eg_cpstr16 strRegEx);
void EGR_FX_GetMatch(const eg_string_base& _this, eg_string_base& strOut, eg_cpstr16 strRegEx, eg_uint nMatch);
eg_bool EGR_FX_Matches(const eg_string_base& _this, eg_cpstr8 strRegEx);
void EGR_FX_GetMatch(const eg_string_base& _this, eg_string_base& strOut, eg_cpstr8 strRegEx, eg_uint nMatch);

struct egShaderExpInfo
{
	eg_string_big ExpFn;
	eg_string_big Filename;
};

typedef EGFixedArray<egShaderExpInfo,32> EGShaderNameStack;

eg_bool EGR_FX_Matches(const eg_string_base& _this, eg_cpstr8 strRegEx)
{
	eg_char8 Temp[1024];
	_this.CopyTo(Temp,countof(Temp));
	std::regex rx(strRegEx);
	return std::regex_match(Temp, rx);
}

void EGR_FX_GetMatch(const eg_string_base& _this, eg_string_base& strOut, eg_cpstr8 strRegEx, eg_uint nMatch)
{
	eg_char8 Temp[1024];
	_this.CopyTo(Temp,countof(Temp));
	std::cmatch res;
	std::regex rx(strRegEx);
	std::regex_search(Temp, res, rx);
	strOut.Clear();
	strOut.Append(res[nMatch].str().c_str());
}

eg_bool EGR_FX_Matches(const eg_string_base& _this, eg_cpstr16 strRegEx)
{
	eg_char16 Temp[1024];
	_this.CopyTo(Temp,countof(Temp));
	std::wregex rx(strRegEx);
	return std::regex_match(Temp, rx);
}

void EGR_FX_GetMatch(const eg_string_base& _this, eg_string_base& strOut, eg_cpstr16 strRegEx, eg_uint nMatch)
{
	eg_char16 Temp[1024];
	_this.CopyTo(Temp,countof(Temp));
	std::wcmatch res;
	std::wregex rx(strRegEx);
	std::regex_search(Temp, res, rx);
	strOut.Clear();
	strOut.Append(eg_string_big(res[nMatch].str().c_str()));
}

enum SHADER_T
{
	SHADER_VS,
	SHADER_PS,
	
	SHADER_VS_5,
	SHADER_PS_5,

	SHADER_T_COUNT,
};

static const struct
{
	eg_cpstr  Profile;
	eg_cpstr  Ext;
}
g_ShaderInfo[SHADER_T_COUNT] =
{
	{("vs_3_0"), ("evs3")},
	{("ps_3_0"), ("eps3")},
	
	{("vs_5_0"), ("evs5")},
	{("ps_5_0"), ("eps5")},
};

static eg_bool EGR_FX_CompileExternal( eg_cpstr strIn , eg_cpstr strOut , const egShaderExpInfo& ExpInfo , SHADER_T t , eg_cpstr DefaultOutFile )
{
	eg_string_big strFinalName;

	if( ExpInfo.Filename.Len() == 0 )
	{
		eg_cpstr strRX = ("[VP]S(.*)");
		eg_string_big strShortName( ("") );
		EGR_FX_GetMatch(ExpInfo.ExpFn, strShortName, strRX, 1);
		strFinalName = EGString_Format( ("%s%s%s.%s"), strOut, DefaultOutFile , strShortName.String(), g_ShaderInfo[t].Ext );
	}
	else
	{
		strFinalName = EGString_Format( "%s%s.%s" , strOut , ExpInfo.Filename.String() , g_ShaderInfo[t].Ext );
	}

	eg_cpstr Version = "";
	switch( t )
	{
	case SHADER_VS:
	case SHADER_PS:
		Version = "3";
		break;
	case SHADER_VS_5:
	case SHADER_PS_5:
		Version = "5";
		break;
	case SHADER_T_COUNT: break;
	}

#if EGMAKE_HLSL_USE_D3DCOMPILE
	const D3D_SHADER_MACRO Macros[] =
	{
		{ "EGFXV" , Version },
		{ NULL , NULL },
	};

	EGArray<eg_d_string16> IncludeDirs;
	IncludeDirs.Append(EGSFormat16( L"{0}{1}" , ::EGMake_GetEGSRC() , "/core/egdata/shaders/" ));
	IncludeDirs.Append(EGSFormat16( L"{0}{1}" , ::EGMake_GetEGOUT() , "/generated_code/" ));
	IncludeDirs.Append(*EGPath2_BreakPath(strIn).GetDirectory());

	const eg_bool bRes = EGR_FX_CompileInternal(
		*eg_d_string16(strIn),
		*eg_d_string16(strFinalName),
		ExpInfo.ExpFn.String(),
		g_ShaderInfo[t].Profile,
		IncludeDirs,
		Macros);

	return bRes;
#else
	eg_d_string8 StrCmd = "";

	StrCmd.Append( EGSFormat8( "fxc.exe /nologo /T {0} /E {1} /D EGFXV={2} "  , g_ShaderInfo[t].Profile , ExpInfo.ExpFn.String() , Version ) );
	StrCmd.Append( EGSFormat8( "/I \"{0}{1}\" " , ::EGMake_GetEGSRC() , "/core/egdata/shaders" ) );
	StrCmd.Append( EGSFormat8( "/I \"{0}{1}\" "  , ::EGMake_GetEGOUT() , "/generated_code/" ) );
	eg_bool bGenerateDebugData = EGString_ToBool( *EGToolsHelper_GetBuildVar( "EGSHADERDEBUG" ) );
	if( bGenerateDebugData )
	{
		StrCmd.Append( EGSFormat8( "/Zi /Fd \"{0}.pdb\" " , strFinalName.String() ) );
	}
	StrCmd.Append( EGSFormat8( "/Fo \"{0}\" \"{1}\" " , strFinalName.String() , strIn ) );


	//EGLogf( ("%s") , strCmd );

	eg_bool bRes = EGExtProcess_Run( *StrCmd , nullptr );
	if(bRes)
		EGLogf( eg_log_t::General , ("   (%s:%s) -> \"%s\""), g_ShaderInfo[t].Profile, ExpInfo.ExpFn.String(), strFinalName.String());

	return bRes;
#endif
}

static eg_bool EGR_FX_Compile(const eg_byte* pData, eg_uint Size, eg_cpstr strIn, eg_cpstr strOut, const egShaderExpInfo& ExpInfo , SHADER_T t, eg_cpstr DefaultOutFile )
{
	unused(Size);
	unused(pData);
	return EGR_FX_CompileExternal(strIn, strOut, ExpInfo, t, DefaultOutFile );
}

static eg_string_big EGR_FX_GetExportDirective( const eg_string_base& Line )
{
	eg_cpstr LineStr  = Line.String();
	eg_string_big Out;
	eg_bool FoundStart = false;
	for( eg_uint i=0; i<Line.Len(); i++ )
	{
		if( !FoundStart && EGString_EqualsCount( &LineStr[i] , "__export" , countof("__export")-1 ) )
		{
			FoundStart = true;
		}

		if( FoundStart )
		{
			Out.Append( LineStr[i] );
			//The end of it is the first encountered )
			if( LineStr[i] == ')' )
				break;
		}
	}
	return Out;
}

static eg_bool EGR_FX_IsWhitespace( eg_char c )
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static eg_string_big EGR_FX_GetFunctionName( const eg_string_base& Line )
{
	//We'll use a simple algorithm, and just build up a name within whitespace until a ( is found, then
	//if name is not __export we'll use it.

	eg_string_big OutName;
	eg_bool WhitespaceFound = false;

	for( eg_uint i=0; i<Line.Len(); i++ )
	{
		if( '(' == Line[i] )
		{
			if( OutName != "__export" )
				break;

			OutName = "";
		}
		else if( EGR_FX_IsWhitespace( Line[i] ) )
		{
			WhitespaceFound = true;
		}
		else
		{
			if( WhitespaceFound )
			{
				OutName = "";
				WhitespaceFound = false;
			}
			OutName.Append( Line[i] );
		}
	}

	return OutName;
}

static void EGR_FX_ProcessExportLine( const eg_string_base& Line , EGShaderNameStack& VSStack , EGShaderNameStack& PSStack )
{
	eg_string_big Directive = EGR_FX_GetExportDirective( Line );

	egParseFuncInfo ParseInfoRaw;
	EGPARSE_RESULT ParseRes = EGParse_ParseFunction( Directive , &ParseInfoRaw );
	egParseFuncInfoAsEgStrings ParseInfo( ParseInfoRaw );
	if( EGPARSE_OKAY != ParseRes || ParseInfo.FunctionName != "__export" )
	{
		EGLogf( eg_log_t::Error , "ERROR: Badly formed export directive \"%s\" (%s)." , Directive.String() , EGParse_GetParseResultString(ParseRes));
		return;
	}

	eg_string_big ExpFnName = EGR_FX_GetFunctionName( Line );
	if( ExpFnName.Len() == 0 )
	{
		EGLogf( eg_log_t::Error , "ERROR: Could not find function to export \"%s\"." , Line.String() );
		return;
	}

	enum EType
	{
		UNK,
		PS,
		VS,
	};

	EType Type = UNK;
	if( ParseInfo.Parms[0] == "vs" )
	{
		Type = VS;
	}
	else if( ParseInfo.Parms[0] == "ps" )
	{
		Type = PS;
	}

	egShaderExpInfo NewInfo = { ExpFnName , ParseInfo.Parms[1] };

	switch( Type )
	{
		case UNK: EGLogf( eg_log_t::Error , "ERROR: Invalid export type: %s." , ParseInfo.Parms[0] ); break;
		case PS : PSStack.Push( NewInfo ); break;
		case VS : VSStack.Push( NewInfo ); break;
	}
}

static void EGR_FX_FindExportFunctions( eg_cpstr8 strShader , eg_size_t strShaderLength , EGShaderNameStack& VSStack , EGShaderNameStack& PSStack )
{
	eg_uint nPos = 0;

	eg_string_big strLine((""));
	eg_string_big strTemp((""));
	for(eg_uint i=0; i<strShaderLength; i++)
	{
		strLine.Append(strShader[i]);
		if('\n' == strShader[i] || i == (strShaderLength-1))
		{
			if(strLine.Contains( "__export" ) )
			{
				EGR_FX_ProcessExportLine( strLine , VSStack , PSStack );
			}

			//Clear the line.
			strLine = "";
		}
	}
}

static void EGR_FX_ShowExports( const EGShaderNameStack& VSStack , const EGShaderNameStack& PSStack )
{
	if( VSStack.Len() > 0 )
	{
		eg_string_big OutputString;
		OutputString.Append( "vs->" );
		for( eg_uint i=0; i<VSStack.Len(); i++ )
		{
			OutputString.Append( EGString_Format( " %s ," , VSStack[i].ExpFn.String() ) );
		}
		EGLogf( eg_log_t::General , "%s" , OutputString.String() );
	}

	if( PSStack.Len() > 0 )
	{
		eg_string_big OutputString;
		OutputString.Append( "ps->" );
		for( eg_uint i=0; i<PSStack.Len(); i++ )
		{
			OutputString.Append( EGString_Format( " %s ," , PSStack[i].ExpFn.String() ) );
		}
		EGLogf( eg_log_t::General , "%s" , OutputString.String() );
	}
}


eg_bool EGMakeRes_hlsl()
{
	EGFileData ShaderFile( eg_file_data_init_t::HasOwnMemory );
	eg_bool bShaderFileLoaded = EGLibFile_OpenFile( EGString_ToWide(EGMake_GetInputPath()) , eg_lib_file_t::OS , ShaderFile );

	if( !bShaderFileLoaded )
	{
		return false;
	}

	//The first thing we will do is search for all of the vertex and pixel
	//shaders.
	EGShaderNameStack VSStack;
	EGShaderNameStack PSStack;
	
	eg_cpstr8 strShader = ShaderFile.GetDataAs<eg_char8>();

	EGR_FX_FindExportFunctions( strShader , ShaderFile.GetSize() , VSStack , PSStack );
	
	if( 0 == VSStack.Len() && 0 == PSStack.Len() )
	{
		EGLogf( eg_log_t::General , "[EGHLSL] No exports found." );
	}

	EGR_FX_ShowExports( VSStack , PSStack );

	bool bSuccess = true;

	#if 1
	for(eg_uint i=0; i<VSStack.Len(); i++)
	{
		// bSuccess = bSuccess && EGR_FX_Compile( pData, Size, EGR_GetInputPath(), EGR_GetOutputPath(FINFO_DIR), VSStack[i], SHADER_VS , EGR_GetOutputPath(FINFO_SHORT_NOEXT) );
		bSuccess = bSuccess && EGR_FX_Compile( ShaderFile.GetDataAs<eg_byte>() , static_cast<eg_uint>(ShaderFile.GetSize()) , EGMake_GetInputPath(), EGMake_GetOutputPath(FINFO_DIR), VSStack[i], SHADER_VS_5 , EGMake_GetOutputPath(FINFO_SHORT_NOEXT) );
	}
	for(eg_uint i=0; i<PSStack.Len(); i++)
	{
		// bSuccess = bSuccess && EGR_FX_Compile( pData, Size, EGR_GetInputPath(), EGR_GetOutputPath(FINFO_DIR), PSStack[i], SHADER_PS , EGR_GetOutputPath(FINFO_SHORT_NOEXT) );
		bSuccess = bSuccess && EGR_FX_Compile( ShaderFile.GetDataAs<eg_byte>(), static_cast<eg_uint>(ShaderFile.GetSize()) , EGMake_GetInputPath(), EGMake_GetOutputPath(FINFO_DIR), PSStack[i], SHADER_PS_5 , EGMake_GetOutputPath(FINFO_SHORT_NOEXT) );
	}
	#endif

	return bSuccess;
}

#if EGMAKE_HLSL_USE_D3DCOMPILE

class EGShaderCompilerIncluder : public ID3DInclude
{
protected:

	eg_d_string16 m_SourceFile;
	eg_d_string16 m_LocalRoot;
	eg_d_string16 m_SystemRoot;
	EGArray<eg_d_string16> m_SearchFolders;

public:

	EGShaderCompilerIncluder( eg_cpstr16 SourceFile , const EGArray<eg_d_string16>& IncludeRoots )
		: m_SourceFile( SourceFile )
	{
		m_LocalRoot = EGPath2_BreakPath( *EGPath2_GetRelativePathTo( *m_SourceFile , *EGOsFile_GetCWD() ) ).GetDirectory();
		m_SystemRoot = L"";
		m_SearchFolders.AppendUnique( m_LocalRoot );
		m_SearchFolders.Append( m_SystemRoot );
		m_SearchFolders.Append( IncludeRoots );
	}

protected:

	eg_d_string16 SearchForFile( eg_cpstr8 Filename , eg_bool bLocal )
	{
		unused( bLocal );

		EGArray<eg_d_string16>& SearchFolders = m_SearchFolders;

		eg_d_string16 Out = Filename;

		for( eg_d_string16& Folder : SearchFolders )
		{
			eg_d_string16 TempPath = EGSFormat16( L"{0}{1}" , *Folder , Filename );
			if( EGOsFile_DoesFileExist( *TempPath ) )
			{
				Out = TempPath;
				// Add the new path to the search folders (if it is new).
				SearchFolders.AppendUnique( *EGPath2_BreakPath( *EGPath2_GetRelativePathTo( *Out , *EGOsFile_GetCWD() ) ).GetDirectory() );
				break;
			}
		}

		return Out;
	}

	virtual HRESULT Open( D3D_INCLUDE_TYPE IncludeType , LPCSTR pFileName , LPCVOID pParentData , LPCVOID* ppData , UINT* pBytes ) override
	{
		unused( pParentData );

		eg_bool bSuccess = false;

		const eg_d_string16 FullPath = SearchForFile( pFileName , IncludeType == D3D_INCLUDE_TYPE::D3D_INCLUDE_LOCAL );

		if( EGOsFile_DoesFileExist( *FullPath ) )
		{
			EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
			EGToolsHelper_OpenFile( *FullPath , FileData);

			if( FileData.GetSize() > 0 )
			{
				void* ShaderData = EGMem2_Alloc( FileData.GetSize() , eg_mem_pool::DefaultHi );
				if( ShaderData )
				{
					EGMem_Copy( ShaderData , FileData.GetData() , FileData.GetSize() );
					*ppData = ShaderData;
					*pBytes = static_cast<UINT>(FileData.GetSize());
					bSuccess = true;
				}
			}
			else
			{
				*ppData = nullptr;
				*pBytes = 0;
			}
		}

		return bSuccess ? S_OK : E_FAIL;
	}

	virtual HRESULT Close( LPCVOID pData ) override
	{
		void* ShaderData = const_cast<void*>(pData);
		EGMem2_Free( ShaderData );
		return S_OK;
	}
};

static eg_bool EGR_FX_CompileInternal( eg_cpstr16 SourceFileRef , const EGFileData& Input , EGFileData& Output , eg_cpstr8 Entry , eg_cpstr8 Target , const EGArray<eg_d_string16>& IncludeRoots , const D3D_SHADER_MACRO* Macros )
{
	eg_bool bSuccess = false;
	eg_cpstr16 CompilerDllFilename = L"d3dcompiler_47.dll";
	HMODULE CompilerDll = LoadLibraryW( CompilerDllFilename );
	if( CompilerDll )
	{
		if( pD3DCompile D3DCompileFn = CompilerDll ? reinterpret_cast<pD3DCompile>(GetProcAddress( CompilerDll , "D3DCompile" )) : nullptr )
		{
			ID3DBlob* CodeBlob = nullptr;
			ID3DBlob* ErrorBlob = nullptr;

			EGShaderCompilerIncluder Includer( SourceFileRef , IncludeRoots );

			UINT CompileFlags = 0;

			HRESULT CompileRes = D3DCompileFn( Input.GetData() , Input.GetSize() , "" , Macros , &Includer , Entry , Target , CompileFlags , 0 , &CodeBlob , &ErrorBlob );
			if( FAILED( CompileRes ) )
			{
				EGLogf( eg_log_t::General , "Failed to compile." );
			}
			if( ErrorBlob )
			{
				eg_cpstr Error = reinterpret_cast<eg_cpstr>(ErrorBlob->GetBufferPointer());
				EGLogf( eg_log_t::General , "Error: %s" , Error );
				EG_SafeRelease( ErrorBlob );
			}

			if( CodeBlob )
			{
				const eg_size_t SizeWritten = Output.Write( CodeBlob->GetBufferPointer() , EG_To<eg_int>( CodeBlob->GetBufferSize() ) );
				bSuccess = (SizeWritten == CodeBlob->GetBufferSize());
				EG_SafeRelease( CodeBlob );
			}
		}
		else
		{
			EGLogf( eg_log_t::General , "Couldn't find D3DCompile function." );
		}

		FreeLibrary( CompilerDll );
	}
	else
	{
		EGLogf( eg_log_t::General , "Couldn't load the compiler library. %s is required for compiling shaders." , CompilerDllFilename );
	}

	return bSuccess;
}

static eg_bool EGR_FX_CompileInternal( eg_cpstr16 SourceFile , EGFileData& Output , eg_cpstr8 Entry , eg_cpstr8 Target , const EGArray<eg_d_string16>& IncludeRoots , const D3D_SHADER_MACRO* Macros )
{

	EGFileData SourceFileData( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( SourceFile , SourceFileData );
	return EGR_FX_CompileInternal( SourceFile , SourceFileData , Output , Entry , Target , IncludeRoots , Macros);
}


static eg_bool EGR_FX_CompileInternal( eg_cpstr16 SourceFile , eg_cpstr16 OutputFile , eg_cpstr8 Entry , eg_cpstr8 Target , const EGArray<eg_d_string16>& IncludeRoots , const D3D_SHADER_MACRO* Macros )
{
	eg_bool bSuccess = false;
	EGFileData DestFileData( eg_file_data_init_t::HasOwnMemory );
	const eg_bool bCompiled = EGR_FX_CompileInternal( SourceFile , DestFileData , Entry , Target , IncludeRoots , Macros);
	if( bCompiled )
	{
		EGLogf( eg_log_t::General , "Saving to %s..." , OutputFile );
		const eg_bool bSaved = EGToolsHelper_SaveFile( OutputFile , DestFileData );
		bSuccess = bSaved;
	}

	return bSuccess;
}

#endif

#undef EGMAKE_HLSL_USE_D3DCOMPILE
